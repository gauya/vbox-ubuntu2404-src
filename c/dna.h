struct DnaData {
    int id;
    char *str;
};

//extern "C" {
DnaData combine_dna(int id1, const char* seq1, int id2, const char* seq2);
DnaData combine_dna_struct(DnaData dna1, DnaData dna2);
//}

