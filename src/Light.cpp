#include "Light.h"
#include "Object.h"
#include <glad/gl.h>

Light::Light(glm::vec3 position, glm::vec3 color, float intensity, const Shader *shader)
: position(position), color(color), intensity(intensity), shader(shader) {
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Attach depth texture as FBO's depth buffer
    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shadowFarPlane = calculateFarPlane();
}

Light::~Light() {
    if (depthMapFBO != 0) {
        glDeleteFramebuffers(1, &depthMapFBO);
    }
    if (depthCubemap != 0) {
        glDeleteTextures(1, &depthCubemap);
    }
}

Light::Light(Light&& other) noexcept
    : position(other.position), color(other.color), intensity(other.intensity),
        shader(other.shader), depthMapFBO(other.depthMapFBO),
        depthCubemap(other.depthCubemap) {
    other.depthMapFBO = 0;
    other.depthCubemap = 0;
}

Light& Light::operator=(Light&& other) noexcept {
    if (this != &other) {
        if (depthMapFBO != 0) glDeleteFramebuffers(1, &depthMapFBO);
        if (depthCubemap != 0) glDeleteTextures(1, &depthCubemap);

        position = other.position;
        color = other.color;
        intensity = other.intensity;
        shader = other.shader;
        depthMapFBO = other.depthMapFBO;
        depthCubemap = other.depthCubemap;

        other.depthMapFBO = 0;
        other.depthCubemap = 0;
    }
    return *this;
}

float Light::calculateFarPlane() const {
    if (intensity < 1e-6f) {
        return MIN_FAR_PLANE;
    }

    const float minIntensity = 1.0f / 256.0f;
    float maxDistance = std::sqrt(intensity / minIntensity);

    return glm::clamp(maxDistance, MIN_FAR_PLANE, MAX_FAR_PLANE);
}

void Light::renderShadowMap(const std::vector<Object*> &objects) {
    // Create depth cubemap transformation matrices
    glm::mat4 shadowProjection = glm::perspective(glm::radians(90.0f),
        (float)SHADOW_MAP_SIZE / (float)SHADOW_MAP_SIZE, shadowNearPlane, shadowFarPlane);

    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3( 1.0f,  0.0f,  0.0f) * 0.1f, glm::vec3(0.0f, -1.0f,  0.0f) * 0.1f));
    shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3(-1.0f,  0.0f,  0.0f) * 0.1f, glm::vec3(0.0f, -1.0f,  0.0f) * 0.1f));
    shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3( 0.0f,  1.0f,  0.0f) * 0.1f, glm::vec3(0.0f,  0.0f,  1.0f) * 0.1f));
    shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3( 0.0f, -1.0f,  0.0f) * 0.1f, glm::vec3(0.0f,  0.0f, -1.0f) * 0.1f));
    shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3( 0.0f,  0.0f,  1.0f) * 0.1f, glm::vec3(0.0f, -1.0f,  0.0f) * 0.1f));
    shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3( 0.0f,  0.0f, -1.0f) * 0.1f, glm::vec3(0.0f, -1.0f,  0.0f) * 0.1f));

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    glClear(GL_DEPTH_BUFFER_BIT);

    shader->use();
    for (unsigned int i = 0; i < 6; ++i)
        shader->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
    shader->setFloat("far_plane", shadowFarPlane);
    shader->setVec3("lightPos", position);

    // Render scene to depth cubemap
    for (Object *object : objects) {
        shader->setMat4("model", object->GetModelMatrix());

        glBindVertexArray(object->VAO);
        glDrawArrays(GL_TRIANGLES, 0, object->vertices.size() / OBJECT_STRIDE);
        glBindVertexArray(0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}