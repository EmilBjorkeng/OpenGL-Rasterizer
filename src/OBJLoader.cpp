#include "OBJLoader.h"
#include "Material.h"
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <vector>

struct FaceVertex {
    int v = 0, vt = 0, vn = 0;
};

// Triangulate a polygonal face using ear clipping
static std::vector<Vertex> triangulateFace(std::vector<Vertex>& face) {
    std::vector<Vertex> result;
    int n = face.size();
    if (n < 3) return result;

    // Create Plane for the face
    glm::vec3 edge1 = face[1].point - face[0].point;
    glm::vec3 edge2 = face[2].point - face[0].point;
    glm::vec3 N = glm::normalize(glm::cross(edge1, edge2));
    // Use U and V basis for the plane
    glm::vec3 U = glm::normalize(edge1);
    glm::vec3 V = glm::cross(N, U);

    // Project vertices onto the 2D plane
    std::vector<glm::vec2> projected;
    projected.reserve(face.size());
    for (const auto &p : face) {
        glm::vec3 vec = p.point - face[0].point;
        projected.emplace_back(glm::dot(vec, U), glm::dot(vec, V));
    }

    // Check orientation
    float totalArea = 0.0f;
    for (int i = 0; i < (int)projected.size(); i++) {
        int j = (i + 1) % projected.size();
        totalArea += (projected[j].x - projected[i].x) * (projected[j].y + projected[i].y);
    }
    bool ccw = (totalArea < 0.0f);

    // Ensure counter-clockwise orientation
    if (!ccw) {
        std::reverse(face.begin(), face.end());
        projected.clear();
        projected.reserve(face.size());

        // recompute projection for reversed vertex order
        for (const auto &p : face) {
            glm::vec3 vec = p.point - face[0].point;
            projected.emplace_back(glm::dot(vec, U), glm::dot(vec, V));
        }
    }

    auto pointInTriangle = [&](const glm::vec2& P, const glm::vec2& A, const glm::vec2& B, const glm::vec2& C)
    {
        float c1 = (B.x - A.x) * (P.y - A.y) - (B.y - A.y) * (P.x - A.x);
        float c2 = (C.x - B.x) * (P.y - B.y) - (C.y - B.y) * (P.x - B.x);
        float c3 = (A.x - C.x) * (P.y - C.y) - (A.y - C.y) * (P.x - C.x);

        bool hasNeg = (c1 < 0) || (c2 < 0) || (c3 < 0);
        bool hasPos = (c1 > 0) || (c2 > 0) || (c3 > 0);

        return !(hasNeg && hasPos);
    };

    std::vector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;

    while (indices.size() > 3) {
        bool earFound = false;

        for (int i = 0; i < (int)indices.size(); ++i) {
            int prev = indices[(i + indices.size() - 1) % indices.size()];
            int curr = indices[i];
            int next = indices[(i + 1) % indices.size()];

            glm::vec2 &a = projected[prev];
            glm::vec2 &b = projected[curr];
            glm::vec2 &c = projected[next];

            // Convex check
            float check = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
            if (check <= 0.0f) continue;

            // Check for vertices inside triangle
            bool hasPointInside = false;
            for (int j = 0; j < (int)indices.size(); ++j) {
                int vi = indices[j];
                if (vi == prev || vi == curr || vi == next) continue;

                if (pointInTriangle(projected[vi], a, b, c)) {
                    hasPointInside = true;
                    break;
                }
            }
            if (hasPointInside) continue;

            result.push_back(face[prev]);
            result.push_back(face[curr]);
            result.push_back(face[next]);
            indices.erase(indices.begin() + i);
            earFound = true;
            break;
        }

        if (!earFound) break; // No valid ear
    }

    // Final triangle
    if (indices.size() == 3) {
        result.push_back(face[indices[0]]);
        result.push_back(face[indices[1]]);
        result.push_back(face[indices[2]]);
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
            std::vector<glm::vec3> facePositions; // Face positions to calculate normals if any are missing
            std::string vert;

            while (ss >> vert) {
                FaceVertex fv;
                std::replace(vert.begin(), vert.end(), '/', ' ');
                std::stringstream tok(vert);
                tok >> fv.v >> fv.vt >> fv.vn;

                // Positions
                glm::vec3 pos = (fv.v > 0 && fv.v <= (int)positions.size()) ? positions[fv.v - 1] : glm::vec3(0.0f);
                facePositions.push_back(pos);

                // Normal
                glm::vec3 normal(0.0f);
                if (fv.vn > 0 && fv.vn <= (int)normals.size()) {
                    normal = normals[fv.vn - 1];
                }

                // Texture
                glm::vec2 uv = (fv.vt > 0 && fv.vt <= (int)texcoords.size()) ? texcoords[fv.vt - 1] : glm::vec2(0.0f);

                faceVertices.push_back({pos, normal, uv});
            }

            // Compute fallback face normal if any vertex has missing normal
            bool needFallback = false;
            for (const auto &v : faceVertices) {
                if (glm::length(v.normal) < 1e-6f) {
                    needFallback = true;
                    break;
                }
            }
            if (needFallback && facePositions.size() >= 3) {
                glm::vec3 edge1 = facePositions[1] - facePositions[0];
                glm::vec3 edge2 = facePositions[2] - facePositions[0];
                glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

                for (auto &v : faceVertices) {
                    if (glm::length(v.normal) < 1e-6f) {
                        v.normal = faceNormal;
                    }
                }
            }

            // Triangulate face
            auto tris = triangulateFace(faceVertices);
            faces.push_back({tris, currentMaterial});
        }
    }

    return faces;
}
