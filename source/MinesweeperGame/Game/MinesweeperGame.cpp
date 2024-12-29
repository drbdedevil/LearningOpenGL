#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include "MinesweeperGame.h"
#include <Engine/CameraManager.h>

PrimitiveShape::PrimitiveShape()
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
	glCompileShader(fragmentShader);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

PrimitiveShape::~PrimitiveShape()
{
	glDeleteProgram(shaderProgram);
}

Cube::Cube()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Cube::~Cube()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void Cube::Update(glm::vec3 Location)
{
	glm::mat4 transform = glm::mat4(1.f);
	CameraManager* CameraMan = CameraManager::GetCameraManager();

	transform = glm::translate(transform, Location);

	// Матрица вида (положение камеры)
	glm::mat4 view = glm::lookAt(
		CameraMan->GetPosition(), // Позиция камеры
		CameraMan->GetPoint(), // Точка, на которую смотрит камера
		CameraMan->GetDirection() // Направление "Вверх"
	);

	// Матрица проекции (перспективная проекция)
	glm::mat4 projection = glm::perspective(
		glm::radians(45.f), // Угол обзора
		800.f / 600.f, // Соотношение сторон экрана
		0.1f, // Ближняя плоскость отсечения
		100.f // Дальная плоскость отсечения
	);

	// Передаём матрицу в шейдер
	GLint transformLoc = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Передача цвета в шейдер
	GLint colorLoc = glGetUniformLocation(shaderProgram, "color");
	glUniform4f(colorLoc, 1.f, 1.f, 1.f, 1.f);

	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 24);
}