#include "Object.h"
#include "OBJLoader.h"
#include <algorithm>

Object::Object(const char* path, const Shader* shader) {
    this->shader = shader;

    std::vector<Face> faces = OBJLoader::loadOBJ(path);

    // Combine faces
    for (auto& face : faces) {
        int texIndex = 0;
        auto it = std::find(textures.begin(), textures.end(), face.material.diffuseTexture);
        if (it == textures.end()) {
            textures.push_back(face.material.diffuseTexture);
            texIndex = textures.size() - 1;
        } else {
            texIndex = it - textures.begin();
        }

        for (auto& v : face.vertices) {
            vertices.push_back(v.point.x);
            vertices.push_back(v.point.y);
            vertices.push_back(v.point.z);

            vertices.push_back(v.normal.x);
            vertices.push_back(v.normal.y);
            vertices.push_back(v.normal.z);

            vertices.push_back(v.texture.x);
            vertices.push_back(v.texture.y);

            vertices.push_back((float)texIndex);

            vertices.push_back(face.material.diffuseColor.x);
            vertices.push_back(face.material.diffuseColor.y);
            vertices.push_back(face.material.diffuseColor.z);

            vertices.push_back(face.material.opacity);
        }
    }

    // Generate VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 13*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 13*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture Coord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 13*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    // Texture ID
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 13*sizeof(float), (void*)(8*sizeof(float)));
    glEnableVertexAttribArray(3);
    // Diffuse Color
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 13*sizeof(float), (void*)(9*sizeof(float)));
    glEnableVertexAttribArray(4);
    // Opacity
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 13*sizeof(float), (void*)(12*sizeof(float)));
    glEnableVertexAttribArray(5);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Object::draw(const glm::mat4 view, const glm::mat4 projection) {
    // Construct model matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);

    // Apply all matrices
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setMat4("model", model);

    // Bind all textures
    for (int i = 0; i < (int)textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
    }

    // Activates shader and maps each texture to their corresponding unit
    shader->use();
    std::vector<int> texUnits(textures.size());
    for (int i = 0; i < (int)textures.size(); ++i)
        texUnits[i] = i;
    shader->setIntArray("textures", texUnits);

    // Bind VAO, draw call, unbind VAB
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 13);
    glBindVertexArray(0);
}