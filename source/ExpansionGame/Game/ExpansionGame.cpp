#include <vulkan/vulkan.h>

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

// -------------------------------------- Îáúåêòû --------------------------------------

PrimitiveShape::PrimitiveShape(int id)
{
	ID = id;
	FramebufferColor = glm::vec3(
		(ID & 0x000000FF) / 255.0f,
		((ID & 0x0000FF00) >> 8) / 255.0f,
		((ID & 0x00FF0000) >> 16) / 255.0f
	);

	
}

PrimitiveShape::~PrimitiveShape()
{
	
}

Cube::Cube(int id) : PrimitiveShape(id)
{
	

	// ----------------------------------------------------------------------------

}

Cube::~Cube()
{
	
}

void Cube::Update(glm::vec3 Location)
{

}