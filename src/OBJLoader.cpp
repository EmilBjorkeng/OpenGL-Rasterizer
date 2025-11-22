#include "OBJLoader.h"
#include "Material.h"
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

struct FaceVertex {
    int v = 0, vt = 0, vn = 0;
};

static Vertex buildVertex(const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv) {
    Vertex vertex;
    vertex.point = pos;
    vertex.normal = normal;
    vertex.texture = uv;
    return vertex;
}

static std::vector<Vertex> triangulateFace(const std::vector<Vertex>& face) {
    std::vector<Vertex> result;
    for (size_t i = 1; i + 1 < face.size(); ++i) {
        result.push_back(face[0]);
        result.push_back(face[i]);
        result.push_back(face[i + 1]);
    }
    return result;
}

std::vector<Face> OBJLoader::loadOBJ(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Failed to open OBJ file: " << path << "\n";
        return {};
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;

    std::vector<Face> faces;
    Material currentMaterial;

    std::string line, mtlFile, useMat;
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") {
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            positions.push_back(v);
        }
        else if (type == "vn") {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (type == "vt") {
            glm::vec2 t;
            ss >> t.x >> t.y;
            texcoords.push_back(t);
        }
        else if (type == "mtllib") {
            ss >> mtlFile;
        }
        else if (type == "usemtl") {
            ss >> useMat;
            if (!mtlFile.empty()) {
                std::filesystem::path mtlPath = std::filesystem::path(path).parent_path() / mtlFile;
                currentMaterial.loadMTL(mtlPath.string(), useMat);
            }
        }
        else if (type == "f") {
            std::vector<Vertex> faceVertices;
            std::string vert;
            while (ss >> vert) {
                FaceVertex fv;
                std::replace(vert.begin(), vert.end(), '/', ' ');
                std::stringstream tok(vert);
                tok >> fv.v >> fv.vt >> fv.vn;

                glm::vec3 pos = positions[fv.v - 1];
                glm::vec3 normal = fv.vn > 0 ? normals[fv.vn - 1] : glm::vec3(0, 1, 0);
                glm::vec2 uv = fv.vt > 0 ? texcoords[fv.vt - 1] : glm::normalize(
                    glm::cross(
                        positions[fv.v - 1] - positions[0],
                        positions[(fv.v % positions.size())] - positions[0]
                    )
                );

                faceVertices.push_back(buildVertex(pos, normal, uv));
            }
            auto tris = triangulateFace(faceVertices);
            faces.push_back(Face(tris, currentMaterial));
        }
    }

    return faces;
}