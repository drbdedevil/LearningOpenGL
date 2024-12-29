#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#pragma comment (lib, "opengl32.lib")

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) 
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
    {
        std::cout << "Left mouse button pressed" << std::endl;
    }
}

// Шейдеры
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    uniform vec4 color;

    void main() {
        FragColor = color;
    }
)";
// FragColor = vec4(0.0, 0.0, 0.2, 1.0); // Красный цвет

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

    // Настройка области просмотра
    glViewport(0, 0, 800, 600);

    // Компиляция шейдеров
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Создание шейдерной программы
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Удаление шейдеров (они больше не нужны после линковки программы)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Вершинные данные
    // Вершинные данные
    float vertices[] = {
          0.0f,  0.5f, -0.5f,
          0.5f, -0.5f, 0.0f,

          -0.5f, -0.5f, 0.0f,
          0.f, -0.5f, -1.f,

          0.f, -0.5f, -1.f,
          0.0f,  0.5f, -0.5f,

          0.5f, -0.5f, 0.0f,
          -0.5f, -0.5f, 0.0f,

          -0.5f, -0.5f, 0.0f,
          0.f, 0.5f, -0.5f,

          0.f, -0.5f, -1.f,
          0.5f, -0.5f, 0.0f
    };
    /*float vertices[] = {
          0.0f,  0.5f, 0.0f,  // Верхний
          0.5f, -0.5f, 0.0f,  // Левый
          -0.5f, -0.5f, 0.0f,//,   // Правый

          0.f, -0.5f, -1.f,
          0.5f, -0.5f, 0.0f,
          -0.5f, -0.5f, 0.0f,

          0.f, -0.5f, -1.f,
          0.0f,  0.5f, 0.0f,
          -0.5f, -0.5f, 0.0f,

          0.f, -0.5f, -1.f,
          0.0f,  0.5f, 0.0f,
          0.5f, -0.5f, 0.0f
    };*/

    // Создание VBO и VAO
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Привязка VAO
    glBindVertexArray(VAO);

    // Привязка VBO и передача данных
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Настройка атрибутов вершин
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Отвязываем VBO и VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Настройка размера.
    // glPointSize(10.f);
    glLineWidth(3.5f);

    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    // Главный цикл
    while (!glfwWindowShouldClose(window)) {
        // Очистка экрана
        glClearColor(0.075f, 0.075f, 0.075f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Создаём матрицу трансформации
        glm::mat4 transform = glm::mat4(1.0f); // Единичная матрица
        float time = glfwGetTime(); // Время с момента запуска
        float offset = sin(time * 0.5f) * 0.5f; // Двигаем треугольник синусоидально
        // transform = glm::translate(transform, glm::vec3(offset, 0.f, 0.f)); // Смещение по X
        transform = glm::rotate(transform, time * 0.3f, glm::vec3(0.f, -0.4f, 0.f));

        // Матрица вида (положение камеры)
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.f, 0.f, 3.f), // Позиция камеры
            glm::vec3(0.f, 0.f, 0.f), // Точка, на которую смотрит камера
            glm::vec3(0.f, 1.f, 0.f) // Направление "Вверх"
        );

        // Матрица проекции (перспективная проекция)
        glm::mat4 projection = glm::perspective(
            glm::radians(45.f), // Угол обзора
            800.f / 600.f, // Соотношение сторон экрана
            0.1f, // Ближняя плоскость отсечения
            100.f // Дальная плоскость отсечения
        );
        
        if (glm::epsilonEqual(offset, 0.f, 0.001f))
        {
            std::cout << "CATCH!" << std::endl;
        }
        
        // Передаём матрицу в шейдер
        GLint transformLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Динамический цвет
        // float red = (sin(time) + 1.0f) / 2.0f;  // Значение от 0 до 1
        // float green = (cos(time) + 1.0f) / 2.0f;
        // float blue = (cos(time * 2.f) + 1.0f) / 8.0f;
        // float alpha = (cos(time * 3.f) + 1.0f) / 2.0f; // Прозрачность (1.0 = полностью непрозрачный)

        // Передача цвета в шейдер
        GLint colorLoc = glGetUniformLocation(shaderProgram, "color");
        glUniform4f(colorLoc, 1.f, 1.f, 1.f, 1.f);

        // Рендеринг треугольника
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, 12);

        // Смена буферов
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Освобождение ресурсов
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}