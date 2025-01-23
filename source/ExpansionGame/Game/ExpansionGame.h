#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>

// --------------------------------------- Game ----------------------------------------
class Cube;

class ExpansionGame
{
public:
    static ExpansionGame* CreateInstance(int Width, int Height);
    static ExpansionGame* Instance() { return Game; }
    void Update();

    Cube* GetCubeByID(int ID) const;
    
    ~ExpansionGame();
private:
    ExpansionGame() {};

    static ExpansionGame* Game;

    static std::vector<std::vector<Cube*>> Map;
    static std::unordered_map<int, Cube*> idMap;
    static std::vector<std::vector<glm::vec3>> CubePositions;

    static int NumberOfObjects;
};

// -------------------------------------- Objects --------------------------------------

class PrimitiveShape
{
public:
    PrimitiveShape(int id);
    ~PrimitiveShape();

    virtual void Update(glm::vec3 Location) = 0;

    int GetID() const { return ID; }
    void SetColor(const glm::vec3& NewColor) { Color = NewColor; }
protected:
    unsigned int shaderProgram;

    unsigned int VAO;
    unsigned int VBO;

    unsigned int VAOPlane;
    unsigned int VBOPlane;

    int ID;
    glm::vec3 Color = glm::vec3(0.7f, 0.7f, 0.65f);
    glm::vec3 FramebufferColor;
private:
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
};

class Cube : public PrimitiveShape
{
public:
    Cube(int id);
    ~Cube();

    virtual void Update(glm::vec3 Location) override;
private:
    float Vertices[72] = {
        // ѕередн€€ грань
        -0.5f, 0.5f, -0.5f, // A
        -0.5f, 0.5f, 0.5f,  // B

        -0.5f, 0.5f, 0.5f,  // B
        0.5f, 0.5f, 0.5f,   // C

        0.5f, 0.5f, 0.5f,   // C
        0.5f, 0.5f, -0.5f,  // D

        0.5f, 0.5f, -0.5f,  // D
        -0.5f, 0.5f, -0.5f, // A

        // «адн€€ грань
        -0.5f, -0.5f, -0.5f,// E
        -0.5f, -0.5f, 0.5f, // F

        -0.5f, -0.5f, 0.5f, // F
        0.5f, -0.5f, 0.5f,  // G

        0.5f, -0.5f, 0.5f,  // G
        0.5f, -0.5f, -0.5f, // H

        0.5f, -0.5f, -0.5f, // H
        -0.5f, -0.5f, -0.5f,// E

        // —оединени€ граней
        -0.5f, 0.5f, -0.5f, // A
        -0.5f, -0.5f, -0.5f,// E

        -0.5f, 0.5f, 0.5f,  // B
        -0.5f, -0.5f, 0.5f, // F

        0.5f, 0.5f, 0.5f,   // C
        0.5f, -0.5f, 0.5f,  // G

        0.5f, 0.5f, -0.5f,  // D
        0.5f, -0.5f, -0.5f  // H
    };
    float TriangleVertices[18] = {
        -0.5f, -0.5f, 0.5f, // F
        0.5f, -0.5f, 0.5f,   // G
        0.5f, 0.5f, 0.5f,   // C

        -0.5f, -0.5f, 0.5f, // F
        0.5f, 0.5f, 0.5f,   // C
        -0.5f, 0.5f, 0.5f   // B
    };
};