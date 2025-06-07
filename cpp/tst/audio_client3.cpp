#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>      // for std::this_thread::sleep_for
#include <chrono>      // for std::chrono::milliseconds
#include <cstdint>     // For uint8_t, uint16_t, uint32_t
#include <cstring>     // For strerror
#include <algorithm>   // For std::min

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
    #include <errno.h>  // For errno
#endif

// 소켓 라이브러리 초기화 및 정리 함수 프로토타입 선언 (main 함수보다 먼저 선언)
void initialize_socket_library();
void cleanup_socket_library();

// Chunk header (general for RIFF files)
// 'size' 멤버가 누락되지 않도록 주의
struct ChunkHeader {
    char id[4];
    uint32_t size; // uint32_t 타입이 <cstdint>에 정의되어 있어야 함
};

// Modified FmtChunk structure
// 모든 uint16_t, uint32_t 타입이 <cstdint>에 정의되어 있어야 함
struct FmtChunk {
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    // Optional: uint16_t cbSize; // for extended fmt chunks (if subchunk1Size > 16)
};

int main() {
    // 변수 선언을 main 함수 초반에 모아둡니다.
    std::string server_ip = "127.0.0.1";
    int server_port = 3001;
    std::string audio_file_path = "sample_audio.wav"; // Assuming this is a float32 WAV file

    // 파일 크기 변수 선언
    std::streamsize file_size = 0;
    
    // 데이터 청크 관련 변수 선언
    uint32_t data_chunk_size = 0;
    long long data_chunk_pos = -1;

    // 소켓 라이브러리 초기화
    initialize_socket_library();

    // 1. WAV 파일 읽기 (데이터 부분만)
    std::ifstream file(audio_file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error opening audio file: " << audio_file_path << std::endl;
        cleanup_socket_library();
        return 1;
    }

    file_size = file.tellg(); // 파일 크기 저장
    file.seekg(0, std::ios::beg);

    // RIFF Chunk 읽기
    ChunkHeader riff_header;
    file.read(reinterpret_cast<char*>(&riff_header), sizeof(ChunkHeader));
    char wave_format[4];
    file.read(wave_format, 4);

    if (std::string(riff_header.id, 4) != "RIFF" || std::string(wave_format, 4) != "WAVE") {
        std::cerr << "Error: Not a RIFF WAVE file." << std::endl;
        file.close(); cleanup_socket_library(); return 1;
    }

    // 청크들을 순회하며 fmt와 data 청크 찾기
    FmtChunk fmt_data; // fmt_data 선언
    bool fmt_found = false; // fmt 청크를 찾았는지 확인하는 플래그

    while (file.tellg() < file_size) { // file_size 변수 사용
        ChunkHeader chunk_header;
        // 파일 끝에 도달했는지 확인 후 읽기
        if (file_size - file.tellg() < sizeof(ChunkHeader)) {
            std::cerr << "Error: Unexpected end of file while reading chunk header." << std::endl;
            file.close(); cleanup_socket_library(); return 1;
        }
        file.read(reinterpret_cast<char*>(&chunk_header), sizeof(ChunkHeader));

        // 청크 데이터 크기가 너무 크거나 음수일 경우 (오류 방지)
        if (chunk_header.size > (file_size - file.tellg()) + 1 || chunk_header.size < 0) { // +1 for alignment
            std::cerr << "Error: Chunk size seems invalid. Aborting." << std::endl;
            file.close(); cleanup_socket_library(); return 1;
        }


        if (std::string(chunk_header.id, 4) == "fmt ") {
            if (chunk_header.size < sizeof(FmtChunk)) {
                std::cerr << "Error: Malformed fmt chunk size." << std::endl;
                file.close(); cleanup_socket_library(); return 1;
            }
            file.read(reinterpret_cast<char*>(&fmt_data), sizeof(FmtChunk));
            fmt_found = true;
            // If fmt chunk has extension, skip it
            if (chunk_header.size > sizeof(FmtChunk)) {
                file.seekg(chunk_header.size - sizeof(FmtChunk), std::ios::cur);
            }
        } else if (std::string(chunk_header.id, 4) == "data") {
            data_chunk_size = chunk_header.size;
            data_chunk_pos = file.tellg();
            // We found data, break (or continue parsing if other chunks are relevant)
            break; // data 청크를 찾았으면 루프 종료
        } else {
            // Skip unknown chunks
            file.seekg(chunk_header.size, std::ios::cur);
        }
        // Ensure proper alignment (RIFF chunks are often word-aligned)
        // 청크 크기가 홀수이면 1바이트 더 스킵하여 다음 청크가 짝수 주소에서 시작하도록 함
        if (chunk_header.size % 2 != 0) {
            file.seekg(1, std::ios::cur);
        }
    }

    if (!fmt_found) {
        std::cerr << "Error: Format (fmt) chunk not found in WAV file." << std::endl;
        file.close(); cleanup_socket_library(); return 1;
    }

    if (data_chunk_pos == -1) {
        std::cerr << "Error: Data chunk not found in WAV file." << std::endl;
        file.close(); cleanup_socket_library(); return 1;
    }

    // 검증 로직 (이제 fmt_data 멤버에 직접 접근)
    if (fmt_data.audioFormat != 3 || fmt_data.bitsPerSample != 32) {
        std::cerr << "Error: Not a standard float32 WAV file or unsupported format. AudioFormat: "
                  << fmt_data.audioFormat << ", BitsPerSample: " << fmt_data.bitsPerSample << std::endl;
        file.close(); cleanup_socket_library(); return 1;
    }

    // 실제 오디오 데이터 위치로 이동 및 읽기
    file.seekg(data_chunk_pos, std::ios::beg);
    std::vector<char> audio_data(data_chunk_size); // data_chunk_size 변수 사용
    file.read(audio_data.data(), data_chunk_size);
    file.close();

    std::cout << "Loaded audio file: " << audio_file_path << std::endl;
    std::cout << "Sample Rate: " << fmt_data.sampleRate << std::endl;     // fmt_data.sampleRate 사용
    std::cout << "Channels: " << fmt_data.numChannels << std::endl;       // fmt_data.numChannels 사용
    std::cout << "Bits Per Sample: " << fmt_data.bitsPerSample << std::endl; // fmt_data.bitsPerSample 사용
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
        // 오류 발생 시 소켓 정리 후 종료
        #ifdef _WIN32
            closesocket(client_socket);
        #else
            close(client_socket);
        #endif
        cleanup_socket_library();
        return 1;
    }
    std::cout << "Connected to " << server_ip << ":" << server_port << std::endl;

    // --- Optional: Send metadata as JSON (similar to Python example) ---
    std::string metadata_json = "{ \"type\": \"audio_stream_start\", ";
    metadata_json += "\"samplerate\": " + std::to_string(fmt_data.sampleRate) + ", ";
    metadata_json += "\"channels\": " + std::to_string(fmt_data.numChannels) + ", ";
    metadata_json += "\"bits_per_sample\": " + std::to_string(fmt_data.bitsPerSample) + ", ";
    metadata_json += "\"dtype\": \"float32\" }\n"; // Add newline for server parsing

    send(client_socket, metadata_json.c_str(), metadata_json.length(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Give server time to process

    // 5. 오디오 데이터를 청크로 나누어 전송
    const size_t CHUNK_SIZE_BYTES = 1024 * 32; // 32KB chunks for example
    size_t total_sent_bytes = 0;

    // byteRate 계산 (fmt_data에서 가져옴)
    uint32_t byte_rate = fmt_data.byteRate;
    if (byte_rate == 0) { // 0으로 나누는 것 방지 (안전 장치)
        std::cerr << "Warning: ByteRate is 0. Cannot simulate real-time streaming accurately." << std::endl;
        byte_rate = fmt_data.sampleRate * fmt_data.numChannels * (fmt_data.bitsPerSample / 8);
        if (byte_rate == 0) byte_rate = 1; // 최소값 설정
    }

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
        // byteRate가 0이 아닌지 확인하여 0으로 나누는 오류 방지
        if (byte_rate > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(
                static_cast<long long>(bytes_to_send * 1000.0 / byte_rate)
            ));
        }
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

// 소켓 라이브러리 초기화 함수 정의
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

// 소켓 라이브러리 정리 함수 정의
void cleanup_socket_library() {
#ifdef _WIN32
    WSACleanup();
#endif
}
