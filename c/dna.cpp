#include <string>
#include <iostream>
#include <algorithm> // std::random_shuffle
#include "dna.h"

// 외부에서 사용할 함수 (extern "C" 로 C linkage 설정)
extern "C" {

    DnaData combine_dna(int id1, const char* seq1, int id2, const char* seq2) {
        std::string combined_seq = std::string(seq1) + std::string(seq2);

        // DNA 조합 로직 (예시 - 랜덤하게 섞기)
        std::random_shuffle(combined_seq.begin(), combined_seq.end());

        DnaData result;
        result.id = id1 + id2; // ID 합산
        result.str = combined_seq.c_str();
        return result;
    }

    DnaData combine_dna_struct(DnaData dna1, DnaData dna2) {
      std::string combined_seq = dna1.sequence + dna2.sequence;
      std::random_shuffle(combined_seq.begin(), combined_seq.end());

      DnaData result;
      result.id = dna1.id * dna2.id; // ID 곱셈 (구분 위한 예시)
      result.sequence = combined_seq;
      return result;

    }

}
//g++ -fPIC -shared -o libdna.so dna.cpp -std=c++11
