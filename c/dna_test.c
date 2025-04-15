#include <stdio.h>
#include <dlfcn.h> // dlopen, dlsym
#include "dna.h" // 구조체 정의 포함

int main() {
    void* handle = dlopen("./libdna.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        return 1;
    }

    DnaData (*combine_dna)(int, const char*, int, const char*) =
        dlsym(handle, "combine_dna");

    DnaData (*combine_dna_struct)(DnaData, DnaData) = dlsym(handle, "combine_dna_struct");

    if (!combine_dna || !combine_dna_struct) {
        fprintf(stderr, "Error getting function address: %s\n", dlerror());
        return 1;
    }

    DnaData result = combine_dna(1, "ATGC", 2, "GCTA");
    printf("Combined ID: %d, Sequence: %s\n", result.id, result.sequence.c_str());

    DnaData dna1 = {1, "ATGC"};
    DnaData dna2 = {2, "GCTA"};
    DnaData result2 = combine_dna_struct(dna1, dna2);
    printf("Combined ID (struct): %d, Sequence: %s\n", result2.id, result2.sequence.c_str());


    dlclose(handle);
    return 0;
}
//gcc -o dna_test dna_test.c -ldl -L. -ldna
