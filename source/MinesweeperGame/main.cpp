#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <MinesweeperGame/Game/MinesweeperGame.h>
#include <Engine/CameraManager.h>

#pragma comment (lib, "opengl32.lib")

int main() {
    // Инициализация GLFW
    if (!glfwInit()) {
        std::cerr << "Не удалось инициализировать GLFW" << std::endl;
        return -1;
    }

    // Настройка GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    // Создание окна
    GLFWwindow* window = glfwCreateWindow(800, 600, "MaxOpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Не удалось создать окно" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Инициализация GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Не удалось инициализировать GLEW" << std::endl;
        return -1;
    }

    Cube cube;
    Cube cube2;

    CameraManager* CameraMan = CameraManager::GetCameraManager();
    if (CameraMan)
    {
        CameraMan->SetPosition(glm::vec3(0.f, -5.f, 0.f));
        CameraMan->SetPoint(glm::vec3(0.f, 0.f, 0.f));
        CameraMan->SetDirection(glm::vec3(0.f, 0.f, 1.f));
    }

    // Настройка области просмотра
    glViewport(0, 0, 800, 600);

    while (!glfwWindowShouldClose(window)) 
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float time = glfwGetTime();
        float zCube = sin(time);
        float zCube2 = cos(time);

        cube.Update(glm::vec3(1.f, 0.f, zCube));
        cube2.Update(glm::vec3(-1.f, 0.f, zCube2));

        if (CameraMan)
        {
            float offset = 5.f * (sin(time) - 1.f) - 5.f * ((sin(time) + 1.f) / 2.f);
            CameraMan->SetPosition(glm::vec3(0.f, offset, 0.f));
            std::cout << offset << std::endl;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}