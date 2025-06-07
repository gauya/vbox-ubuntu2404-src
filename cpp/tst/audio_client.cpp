#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread> // for std::this_thread::sleep_for
#include <chrono> // for std::chrono::milliseconds
#include <cstdint> // for uint8_t
#include <cstring>

// OS에 따른 소켓 라이브러리 포함
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h> // for close()
#endif

// WAV 파일 헤더 구조체 (간단화된 버전, 실제 WAV 파일 파싱은 더 복잡함)
// float32 WAV 파일 가정
struct WavHeader {
    char chunkID[4];       // "RIFF"
    uint32_t chunkSize;
    char format[4];        // "WAVE"
    char subchunk1ID[4];   // "fmt "
    uint32_t subchunk1Size; // 16 for PCM
    uint16_t audioFormat;  // 3 for IEEE float
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample; // 32 for float
    char subchunk2ID[4];   // "data"
    uint32_t subchunk2Size; // data size
};


void initialize_socket_library() {
#ifdef _WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        exit(1);
    }
#endif
}

void cleanup_socket_library() {
#ifdef _WIN32
    WSACleanup();
#endif
}

int main() {
    initialize_socket_library();

    std::string server_ip = "127.0.0.1";
    int server_port = 3001;
    std::string audio_file_path = "sample_audio.wav"; // Assuming this is a float32 WAV file

    // 1. WAV 파일 읽기 (데이터 부분만)
    std::ifstream file(audio_file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error opening audio file: " << audio_file_path << std::endl;
        cleanup_socket_library();
        return 1;
    }

    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read WAV header (simplified, assumes a standard float32 WAV)
    WavHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));

    if (std::string(header.chunkID, 4) != "RIFF" || std::string(header.format, 4) != "WAVE" ||
        std::string(header.subchunk2ID, 4) != "data" || header.audioFormat != 3 || header.bitsPerSample != 32) {
        std::cerr << "Error: Not a standard float32 WAV file or unsupported format." << std::endl;

        std::cout << "id(RIFF)=" << std::string(header.chunkID, 4) << ", head(WAVE)=" << std::string(header.format, 4);
        std::cout << ", subchunk(data)=" << std::string(header.subchunk2ID, 4);
        std::cout << ", format(3)=" << std::string(header.format, 4);
        std::cout << ", sample(32)=" << header.bitsPerSample << std::endl;
        
        file.close();
        cleanup_socket_library();
        return 1;
    }

    std::vector<char> audio_data(header.subchunk2Size); // Allocate memory for audio data
    file.read(audio_data.data(), header.subchunk2Size); // Read raw audio data
    file.close();

    std::cout << "Loaded audio file: " << audio_file_path << std::endl;
    std::cout << "Sample Rate: " << header.sampleRate << std::endl;
    std::cout << "Channels: " << header.numChannels << std::endl;
    std::cout << "Bits Per Sample: " << header.bitsPerSample << std::endl;
    std::cout << "Audio Data Size: " << audio_data.size() << " bytes" << std::endl;

    // 2. TCP 소켓 생성
#ifdef _WIN32
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        cleanup_socket_library();
        return 1;
    }
#else
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        cleanup_socket_library();
        return 1;
    }
#endif

    // 3. 서버 주소 설정
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
#ifdef _WIN32
    inet_pton(AF_INET, server_ip.c_str(), &(server_addr.sin_addr));
#else
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);
#endif

    // 4. 서버에 연결
    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Error connecting to server: ";
#ifdef _WIN32
        std::cerr << WSAGetLastError() << std::endl;
#else
        std::cerr << strerror(errno) << std::endl;
#endif
        cleanup_socket_library();
        return 1;
    }
    std::cout << "Connected to " << server_ip << ":" << server_port << std::endl;

    // --- Optional: Send metadata as JSON (similar to Python example) ---
    std::string metadata_json = "{ \"type\": \"audio_stream_start\", ";
    metadata_json += "\"samplerate\": " + std::to_string(header.sampleRate) + ", ";
    metadata_json += "\"channels\": " + std::to_string(header.numChannels) + ", ";
    metadata_json += "\"bits_per_sample\": " + std::to_string(header.bitsPerSample) + ", ";
    metadata_json += "\"dtype\": \"float32\" }\n"; // Add newline for server parsing

    send(client_socket, metadata_json.c_str(), metadata_json.length(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Give server time to process

    // 5. 오디오 데이터를 청크로 나누어 전송
    const size_t CHUNK_SIZE_BYTES = 1024 * 32; // 32KB chunks for example
    size_t total_sent_bytes = 0;

    while (total_sent_bytes < audio_data.size()) {
        size_t bytes_to_send = std::min(CHUNK_SIZE_BYTES, audio_data.size() - total_sent_bytes);
        
        int sent_bytes = send(client_socket, audio_data.data() + total_sent_bytes, bytes_to_send, 0);
        if (sent_bytes == -1) {
            std::cerr << "Error sending data: ";
#ifdef _WIN32
            std::cerr << WSAGetLastError() << std::endl;
#else
            std::cerr << strerror(errno) << std::endl;
#endif
            break;
        }
        total_sent_bytes += sent_bytes;
        std::cout << "Sent " << total_sent_bytes << "/" << audio_data.size() << " bytes" << std::endl;

        // Simulate real-time streaming: sleep for a duration proportional to the sent data
        // (bytes_to_send / (header.byteRate / 1000.0)) for milliseconds
        std::this_thread::sleep_for(std::chrono::milliseconds(
            static_cast<long long>(bytes_to_send * 1000.0 / header.byteRate)
        ));
    }

    std::cout << "Finished sending audio data." << std::endl;

    // 6. 소켓 닫기
#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif
    std::cout << "Socket closed." << std::endl;

    cleanup_socket_library();
    return 0;
}
