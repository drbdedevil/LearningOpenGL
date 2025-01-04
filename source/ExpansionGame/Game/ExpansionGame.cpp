#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include "ExpansionGame.h"
#include <Engine/CameraManager.h>
#include <iostream>

// --------------------------------------- Game ----------------------------------------

int ExpansionGame::NumberOfObjects = 1;
ExpansionGame* ExpansionGame::Game = nullptr;
std::vector<std::vector<Cube*>> ExpansionGame::Map = std::vector<std::vector<Cube*>>();
std::unordered_map<int, Cube*> ExpansionGame::idMap = std::unordered_map<int, Cube*>();
std::vector<std::vector<glm::vec3>> ExpansionGame::CubePositions = std::vector<std::vector<glm::vec3>>();

ExpansionGame* ExpansionGame::CreateInstance(int Width, int Height)
{
	if (!Game)
	{
		Game = new ExpansionGame();
		Game->Map.clear();
		if (Width < 0 || Height < 0)
		{
			return nullptr;
		}
		Game->Map.resize(Width);
		Game->CubePositions.resize(Width);

		float XPos = 0.f;
		float YPos = 0.f;
		float Offset = 1.05f;

		for (size_t i = 0; i < Width; ++i)
		{
			Game->Map[i] = std::vector<Cube*>();
			Game->Map[i].resize(Height);
			Game->CubePositions[i].resize(Height);

			for (size_t j = 0; j < Height; ++j)
			{
				Cube* NewCube = new Cube(Game->NumberOfObjects++);
				Game->Map[i][j] = NewCube;
				Game->CubePositions[i][j] = glm::vec3(XPos, YPos, 0.f);

				Game->idMap[Game->Map[i][j]->GetID()] = NewCube;
				// std::cout << Game->Map[i][j]->GetID() << " - " << NewCube << std::endl;

				YPos = YPos + Offset;
			}
			XPos = XPos + Offset;
			YPos = 0.f;
		}
	}

	return Game;
}

void ExpansionGame::Update()
{
	if (Game)
	{
		for (size_t i = 0; i < Game->Map.size(); ++i)
		{
			for (size_t j = 0; j < Game->Map[i].size(); ++j)
			{
				if (Game->Map[i][j])
				{
					Game->Map[i][j]->Update(Game->CubePositions[i][j]);
				}
			}
		}
	}
}

Cube* ExpansionGame::GetCubeByID(int ID) const
{
	if (Game)
	{
		/*for (const auto& [key, value] : Game->idMap)
		{
			if (key == ID)
			{
				std::cout << "Key: " << key << ", Address: " << value->GetID() << "\n";
			}
			
		}*/

		auto it = Game->idMap.find(ID);
		if (it != Game->idMap.end())
		{
			std::cout << it->first << ", Address: "<< it->second << std::endl;
			return it->second;
		}
	}

	return nullptr;
}

ExpansionGame::~ExpansionGame()
{
	if (!Game) return;

	for (size_t i = 0; i < Game->Map.size(); ++i)
	{
		for (size_t j = 0; j < Game->Map[i].size(); ++j)
		{
			if (Game->Map[i][j])
			{
				delete Game->Map[i][j];
			}
		}
		Game->Map[i].shrink_to_fit();
		Game->CubePositions[i].clear();
	}
	Game->Map.shrink_to_fit();
	Game->idMap.clear();
	Game->CubePositions.clear();

	delete Game;
}

// -------------------------------------- Объекты --------------------------------------

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
	glEnable(GL_DEPTH_TEST);
	// Отрисовка куба линиями
	
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
	
	// Матрица вида (положение камеры)
	glm::mat4 view = glm::lookAt(
		CameraMan->GetPosition(), // Позиция камеры
		CameraMan->GetPoint(),    // Точка, на которую смотрит камера
		CameraMan->GetDirection() // Направление "Вверх"
	);

	// Матрица проекции (перспективная проекция)
	glm::mat4 projection = glm::perspective(
		glm::radians(45.f), // Угол обзора
		800.f / 600.f,      // Соотношение сторон экрана
		0.1f,               // Ближняя плоскость отсечения
		100.f               // Дальная плоскость отсечения
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
	glUniform4f(colorLoc, 0.f, 0.f, 0.f, 1.f);

	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	glLineWidth(2.f);
	glDrawArrays(GL_LINES, 0, 24);

	// Отрисовка плейна треугольниками

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleVertices), TriangleVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLint colorCell = glGetUniformLocation(shaderProgram, "color");
	glUniform4f(colorCell, Color.x, Color.y, Color.z, 1.f);
	// glUniform4f(colorCell, FramebufferColor.x, FramebufferColor.y, FramebufferColor.z, 1.f);

	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Рендеринг уникальных цветов
	glDisable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, CameraMan->fbo);
	// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgram);

	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glUniform4f(colorCell, FramebufferColor.x, FramebufferColor.y, FramebufferColor.z, 1.f);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}