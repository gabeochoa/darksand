
#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
//
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <streambuf>
#include <string>

#include "shader.h"
#include "util.h"

enum Material {
    // This order should match the shader
    Empty = 0,
    Sand = 1,
    Wood = 2,
};
Material selectedMaterial = Sand;

float g_xpos = 0.0;
float g_ypos = 0.0;
int left_mouse_button = 0;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action,
                           int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mods);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window =
        glfwCreateWindow(window_width, window_height, "Sand", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // setting glViewport and callback functions for user input
    glViewport(0, 0, window_width, window_height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    unsigned int indices[] = {
        0, 1, 2, 0, 2, 3,
    };  // EBO data

    float vertices[] = {
        // pos info         //tex info
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f,  1.0f,  0.0f, 1.0f, 1.0f, -1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
    };

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);

    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    Shader shader("../src/glsl/vertex.vert", "../src/glsl/fragment.frag");

    // gen textures (2 for ping pong buffering)
    unsigned int texture[2];
    glGenTextures(2, (GLuint*)&texture);
    float borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // gen framebuffer
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // binding the textures to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           texture[0], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                           texture[1], 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
                  << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int activeBuffer = 0;
    int src, dest;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT);

        float timeValue = glfwGetTime();

        shader.use();
        // set most of the uniform variables for the fragment shader (except the
        // textures)
        shader.setVec2("mouse_pos_u", g_xpos / window_width,
                       1.0 - g_ypos / window_height);
        shader.setInt("mouse_leftButton_u", left_mouse_button);
        shader.setInt("selected_material", static_cast<int>(selectedMaterial));

        // swap src and dest values
        src = activeBuffer;
        dest = 1 - activeBuffer;
        activeBuffer = 1 - activeBuffer;

        // bind vertex data
        glBindVertexArray(VAO);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        // draw on back buffer
        GLuint buffs[] = {GL_COLOR_ATTACHMENT0 + (GLuint)dest};
        glDrawBuffers(1, buffs);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture[src]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // draw on std buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture[src]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // unbind vertex data
        glBindVertexArray(0);

        // update glfw
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // clean
    glDeleteFramebuffers(1, &fbo);
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    g_xpos = xpos;
    g_ypos = ypos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action,
                           int mods) {
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    left_mouse_button = (int)state == GLFW_PRESS;
}

inline bool pressed(GLFWwindow* window, int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mods) {
    if (pressed(window, GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

    if (pressed(window, GLFW_KEY_S)) selectedMaterial = Sand;
    if (pressed(window, GLFW_KEY_T)) selectedMaterial = Wood;
    if (pressed(window, GLFW_KEY_E)) selectedMaterial = Empty;
}
