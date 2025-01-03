#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include "MinesweeperGame.h"
#include <Engine/CameraManager.h>

PrimitiveShape::PrimitiveShape(int id)
{
	ID = id;
	FramebufferColor = glm::vec3(
		(ID & 0x000000FF) / 255.0f,
		((ID & 0x0000FF00) >> 8) / 255.0f,
		((ID & 0x00FF0000) >> 16) / 255.0f
	);

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
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glDeleteProgram(shaderProgram);
}

Cube::Cube(int id) : PrimitiveShape(id)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
}

Cube::~Cube()
{
	
}

void Cube::Update(glm::vec3 Location)
{
	// ��������� ���� �������
	
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glm::mat4 transform = glm::mat4(1.f);
	CameraManager* CameraMan = CameraManager::GetCameraManager();

	transform = glm::translate(transform, Location);
	
	// ������� ���� (��������� ������)
	glm::mat4 view = glm::lookAt(
		CameraMan->GetPosition(), // ������� ������
		CameraMan->GetPoint(),    // �����, �� ������� ������� ������
		CameraMan->GetDirection() // ����������� "�����"
	);

	// ������� �������� (������������� ��������)
	glm::mat4 projection = glm::perspective(
		glm::radians(45.f), // ���� ������
		800.f / 600.f,      // ����������� ������ ������
		0.1f,               // ������� ��������� ���������
		100.f               // ������� ��������� ���������
	);

	// ������� ������� � ������
	GLint transformLoc = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// �������� ����� � ������
	GLint colorLoc = glGetUniformLocation(shaderProgram, "color");
	glUniform4f(colorLoc, 0.f, 0.f, 0.f, 1.f);

	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	glLineWidth(2.f);
	glDrawArrays(GL_LINES, 0, 24);

	// ��������� ������ ��������������

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleVertices), TriangleVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLint colorCell = glGetUniformLocation(shaderProgram, "color");
	// glUniform4f(colorCell, 0.8f, 0.8f, 0.8f, 1.f);
	glUniform4f(colorCell, FramebufferColor.x, FramebufferColor.y, FramebufferColor.z, 1.f);

	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// ��������� ���������� ������

	glBindFramebuffer(GL_FRAMEBUFFER, CameraMan->fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgram);

	glUniform4f(colorCell, FramebufferColor.x, FramebufferColor.y, FramebufferColor.z, 1.f);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}