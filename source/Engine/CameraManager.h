#include <glm/glm.hpp>

class CameraManager
{
public:
	static CameraManager* GetCameraManager();
	static void DeleteCamera();
	CameraManager() {}
	~CameraManager() {}

	void SetPosition(const glm::vec3& NewPos) { Position = NewPos; }
	void SetPoint(const glm::vec3& NewPoint) { Point = NewPoint; }
	void SetDirection(const glm::vec3& NewDirection) { Direction = NewDirection; }

	void AddPosition(const glm::vec3& AddPosition) { Position = Position + AddPosition; }
	void AddDirection(const glm::vec3& AddDirection) { Direction = Direction + AddDirection; }

	void AddCameraPosition(const glm::vec3& AddPosition);

	glm::vec3 GetPosition() const { return Position; }
	glm::vec3 GetPoint() const { return Point; }
	glm::vec3 GetDirection() const { return Direction; }

	// TODO: ”¡–¿“‹ ›“Œ   ’”ﬂÃ Œ“—ﬁƒ¿
	unsigned int fbo = 0;
private:
	static CameraManager* Camera;

	glm::vec3 Position;
	glm::vec3 Point;
	glm::vec3 Direction;
};