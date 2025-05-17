#ifndef STL_LOADER_H
#define STL_LOADER_H

#include <vector>
#include <glm/glm.hpp>
#include <string>

struct Triangle {
    glm::vec3 vertices[3];
};

std::vector<Triangle> LoadSTL(const std::string& filename);

#endif
