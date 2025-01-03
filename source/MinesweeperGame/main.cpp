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

static GLuint fbo = 0;

void MouseCallback(GLFWwindow* window, int button, int action, int mods) 
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        std::cout << "(" << xpos << ", " << ypos << ")" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        GLubyte pixel[3];
        glReadPixels(xpos, 600 - ypos, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Конвертируем цвет в ID
        int clickedObjectID = pixel[0] + (pixel[1] << 8) + (pixel[2] << 16);

        if (clickedObjectID > 0) 
        {
            std::cout << "Объект с ID: " << clickedObjectID << " выбран." << std::endl;
        }
        else 
        {
            std::cout << "Объект не найден - " << clickedObjectID << std::endl;
        }
    }
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    if (key == GLFW_KEY_W/* && action == GLFW_PRESS*/) 
    {
        std::cout << "W" << std::endl;
        CameraManager* CameraMan = CameraManager::GetCameraManager();
        if (CameraMan)
        {
            CameraMan->AddCameraPosition(glm::vec3(0.f, 0.25f, 0.f));
        }
    }
    if (key == GLFW_KEY_S)
    {
        std::cout << "S" << std::endl;
        CameraManager* CameraMan = CameraManager::GetCameraManager();
        if (CameraMan)
        {
            CameraMan->AddCameraPosition(glm::vec3(0.f, -0.25f, 0.f));
        }
    }
    if (key == GLFW_KEY_A)
    {
        std::cout << "A" << std::endl;
        CameraManager* CameraMan = CameraManager::GetCameraManager();
        if (CameraMan)
        {
            CameraMan->AddCameraPosition(glm::vec3(-0.25f, 0.f, 0.f));
        }
    }
    if (key == GLFW_KEY_D)
    {
        std::cout << "D" << std::endl;
        CameraManager* CameraMan = CameraManager::GetCameraManager();
        if (CameraMan)
        {
            CameraMan->AddCameraPosition(glm::vec3(0.25f, 0.f, 0.f));
        }
    }
}

void CreateFBO()
{
    GLuint colorAttachment, rbo;

    // Создаём FBO
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Создаём текстуру для хранения цветов (идентификаторов)
    glGenTextures(1, &colorAttachment);
    glBindTexture(GL_TEXTURE_2D, colorAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment, 0);

    // Создаём буфер глубины
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // Проверяем статус FBO
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Ошибка: Framebuffer не завершён!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main() 
{
    setlocale(LC_ALL, "rus");

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
    CreateFBO();

    Cube cube(1);
    Cube cube2(2000);
    Cube cube3(3000000);

    Cube cube4(8778);
    Cube cube5(57568568);
    Cube cube6(63252);
    Cube cube7(24234);
    Cube cube8(2347668);
    Cube cube9(9565656);

    CameraManager* CameraMan = CameraManager::GetCameraManager();
    if (CameraMan)
    {
        CameraMan->SetPosition(glm::vec3(0.f, -5.f, 10.f));
        CameraMan->SetPoint(glm::vec3(0.f, 0.f, 0.f));
        CameraMan->SetDirection(glm::vec3(0.f, 0.f, 1.f));

        CameraMan->fbo = fbo;
    }

    // Настройка области просмотра
    glViewport(0, 0, 800, 600);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseCallback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    while (!glfwWindowShouldClose(window)) 
    {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        float time = glfwGetTime();
        float zCube = sin(time);
        float zCube2 = cos(time);

        cube.Update(glm::vec3(1.05f, 0.f, 0.f));
        cube2.Update(glm::vec3(-1.05f, 0.f, 0.f));
        cube3.Update(glm::vec3(0.f, 0.f, 0.f));

        cube4.Update(glm::vec3(1.05f, 1.05f, 0.f));
        cube5.Update(glm::vec3(-1.05f, 1.05f, 0.f));
        cube6.Update(glm::vec3(0.f, 1.05f, 0.f));
        cube7.Update(glm::vec3(1.05f, -1.05f, 0.f));
        cube8.Update(glm::vec3(-1.05f, -1.05f, 0.f));
        cube9.Update(glm::vec3(0.f, -1.05f, 0.f));

        if (CameraMan)
        {
            float offset = 5.f * (sin(time) - 1.f) - 5.f * ((sin(time) + 1.f) / 2.f);
            // CameraMan->SetPosition(glm::vec3(0.f, offset, 0.f));
            // std::cout << offset << std::endl;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    CameraManager::DeleteCamera();
    return 0;
}