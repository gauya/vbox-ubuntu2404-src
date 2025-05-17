#include "stl_loader.h"
#include <iostream>
#include <fstream>

std::vector<Triangle> LoadSTL(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    std::vector<Triangle> triangles;

    if (!file) {
        std::cerr << "파일을 열 수 없습니다!" << std::endl;
        return triangles;
    }

    file.seekg(80); // STL 헤더 건너뛰기
    uint32_t numTriangles;
    file.read(reinterpret_cast<char*>(&numTriangles), sizeof(numTriangles));

    for (uint32_t i = 0; i < numTriangles; ++i) {
        Triangle tri;
        file.read(reinterpret_cast<char*>(&tri), sizeof(Triangle));
        triangles.push_back(tri);
        file.ignore(2);
    }

    return triangles;
}

