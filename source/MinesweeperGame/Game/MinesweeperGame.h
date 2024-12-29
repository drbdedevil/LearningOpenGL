class PrimitiveShape
{
public:
    PrimitiveShape();
    ~PrimitiveShape();

    virtual void Update(glm::vec3 Location) = 0;
protected:
    unsigned int shaderProgram;

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
    Cube();
    ~Cube();

    virtual void Update(glm::vec3 Location) override;
private:
    unsigned int VAO;
    unsigned int VBO;

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
};