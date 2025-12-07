#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Object.h"
#include "TextureManager.h"
#include "Camera.h"
#include "Light.h"
#include "MapLoader.h"

#include <iostream>
#include <algorithm>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

#define WINDOW_TITLE "Title"
int window_width = 1920;
int window_height = 1080;

int main() {
    // Initialize and configure (glfw)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window (glfw)
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, WINDOW_TITLE, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load all OpenGL function pointers (glad)
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);

    // Calculate center position
    int xpos = (mode->width - window_width) / 2;
    int ypos = (mode->height - window_height) / 2;

    glfwSetWindowPos(window, xpos, ypos);
    glfwMakeContextCurrent(window);

    // Configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    Camera camera(glm::vec3(0.5f, 0.5f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    camera.projectionMatrix = glm::perspective(
            glm::radians(45.0f), (float)window_width / (float)window_height,
            camera.nearPlane, camera.farPlane);
    glfwSetWindowUserPointer(window, &camera);

    Shader shader("shaders/Shader.vs", "shaders/Shader.fs");
    Shader shadowShader("shaders/Shadow.vs", "shaders/Shadow.fs", "shaders/Shadow.gs");

    Scene map = MAPLoader::loadMAP("maps/map.map", shader, shadowShader);

    std::vector<Object*> &sceneObjects       = map.sceneObjects;
    std::vector<Light*>  &sceneLights        = map.sceneLights;
    std::vector<Object*> &opaqueObjects      = map.opaqueObjects;
    std::vector<Object*> &transparentObjects = map.transparentObjects;

    double lastTime = glfwGetTime();
    double DeltaTime = 0.0;

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        DeltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Input
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Speed boost
        float moveSpeed = camera.moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            moveSpeed *= 3.0f;

        // Movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.position += camera.Forward() * static_cast<float>(moveSpeed * DeltaTime);

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.position -= camera.Forward() * static_cast<float>(moveSpeed * DeltaTime);

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.position -= camera.Right() * static_cast<float>(moveSpeed * DeltaTime);

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.position += camera.Right() * static_cast<float>(moveSpeed * DeltaTime);

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.position.y += moveSpeed * DeltaTime;

        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            camera.position.y -= moveSpeed * DeltaTime;

        float lerpSpeed = 5.0f;
        glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
        float angle = glm::radians(camera.turnSpeed * DeltaTime);

        // Pitch
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            camera.rotation = glm::angleAxis(angle, glm::normalize(camera.Right())) * camera.rotation;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            camera.rotation = glm::angleAxis(-angle, glm::normalize(camera.Right())) * camera.rotation;

        // Yaw
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera.rotation = glm::angleAxis(angle, worldUp) * camera.rotation;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera.rotation = glm::angleAxis(-angle, worldUp) * camera.rotation;

        // Roll
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera.rotation = glm::angleAxis(-angle, glm::normalize(camera.Forward())) * camera.rotation;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera.rotation = glm::angleAxis(angle, glm::normalize(camera.Forward())) * camera.rotation;

        // Reset Roll
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            glm::mat4 lookAtMat = glm::lookAt(glm::vec3(0.0f), camera.Forward(), worldUp);
            glm::quat targetRotation = glm::quat_cast(glm::inverse(lookAtMat));
            camera.rotation = glm::slerp(camera.rotation, targetRotation, static_cast<float>(lerpSpeed * DeltaTime));
        }

        camera.rotation = glm::normalize(camera.rotation);

        // Logic

        // Draw
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Shadow map
        glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
        glCullFace(GL_FRONT);
        for (Light *light : sceneLights) {
            light->renderShadowMap(sceneObjects);
        }
        glCullFace(GL_BACK);

        glViewport(0, 0, window_width, window_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = camera.projectionMatrix;

        // Draw opaque objects
        for (Object *object : opaqueObjects) {
            object->draw(view, projection, sceneLights);
        }

        // Sort translucent objects back to front
        glm::vec3 camPos = camera.position;
        std::sort(transparentObjects.begin(), transparentObjects.end(),
            [camPos](Object *a, Object *b) {
                float distA = glm::length(camPos - a->position);
                float distB = glm::length(camPos - b->position);
                return distA > distB; // Farthest first
            });


        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        // Draw translucent objects
        for (Object *object : transparentObjects) {
            object->draw(view, projection, sceneLights);
        }

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

// Callback for whenever the window size changed (by OS or user resize)
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);

    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    camera->projectionMatrix = glm::perspective(
        glm::radians(45.0f), (float)width / (float)height,
        camera->nearPlane, camera->farPlane);

    window_width = width;
    window_height = height;
}

// Callback for whenever the mouse is moved
bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    static double lastX = xpos, lastY = ypos;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    // Apply rotation
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    camera->rotation = glm::angleAxis((float)-xoffset * camera->sensitivity, worldUp) * camera->rotation;
    camera->rotation = glm::angleAxis((float)yoffset * camera->sensitivity, glm::normalize(camera->Right())) * camera->rotation;
}
