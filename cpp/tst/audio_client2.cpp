#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm> // For std::min, std::find
#include <thread> // for std::this_thread::sleep_for
#include <chrono> // for std::chrono::milliseconds
#include <cstdint> // for uint8_t
#include <cstring>

// ... (기존 include)

// Chunk header (general for RIFF files)
struct ChunkHeader {
    char id[4];
    uint32_t size;
};

// Modified WavHeader (only the fmt part for initial read)
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
    // ... (초기화 코드)

    std::ifstream file(audio_file_path, std::ios::binary | std::ios::ate);
    // ... (파일 열기 및 에러 체크)

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
    FmtChunk fmt_data;
    uint32_t data_chunk_size = 0;
    long long data_chunk_pos = -1;

    while (file.tellg() < file_size) {
        ChunkHeader chunk_header;
        file.read(reinterpret_cast<char*>(&chunk_header), sizeof(ChunkHeader));

        if (std::string(chunk_header.id, 4) == "fmt ") {
            if (chunk_header.size < sizeof(FmtChunk)) {
                std::cerr << "Error: Malformed fmt chunk size." << std::endl;
                file.close(); cleanup_socket_library(); return 1;
            }
            file.read(reinterpret_cast<char*>(&fmt_data), sizeof(FmtChunk));
            // If fmt chunk has extension, skip it
            if (chunk_header.size > sizeof(FmtChunk)) {
                file.seekg(chunk_header.size - sizeof(FmtChunk), std::ios::cur);
            }
        } else if (std::string(chunk_header.id, 4) == "data") {
            data_chunk_size = chunk_header.size;
            data_chunk_pos = file.tellg();
            // We found data, break (or continue parsing if other chunks are relevant)
            break;
        } else {
            // Skip unknown chunks
            file.seekg(chunk_header.size, std::ios::cur);
        }
        // Ensure proper alignment (RIFF chunks are often word-aligned)
        if (chunk_header.size % 2 != 0) {
            file.seekg(1, std::ios::cur);
        }
    }

    if (data_chunk_pos == -1) {
        std::cerr << "Error: Data chunk not found in WAV file." << std::endl;
        file.close(); cleanup_socket_library(); return 1;
    }

    // 검증 로직 (이전과 동일하게)
    if (fmt_data.audioFormat != 3 || fmt_data.bitsPerSample != 32) {
        std::cerr << "Error: Not a standard float32 WAV file or unsupported format. AudioFormat: "
                  << fmt_data.audioFormat << ", BitsPerSample: " << fmt_data.bitsPerSample << std::endl;
        file.close(); cleanup_socket_library(); return 1;
    }

    // 실제 오디오 데이터 위치로 이동 및 읽기
    file.seekg(data_chunk_pos, std::ios::beg);
    std::vector<char> audio_data(data_chunk_size);
    file.read(audio_data.data(), data_chunk_size);
    file.close();

    // ... (나머지 소켓 통신 로직은 동일)
}
