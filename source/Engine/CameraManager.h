#include <glm/glm.hpp>

class CameraManager
{
public:
	static CameraManager* GetCameraManager();
	CameraManager() {}
	~CameraManager() { delete Camera; }

	void SetPosition(const glm::vec3& NewPos) { Position = NewPos; }
	void SetPoint(const glm::vec3& NewPoint) { Point = NewPoint; }
	void SetDirection(const glm::vec3& NewDirection) { Direction = NewDirection; }

	glm::vec3 GetPosition() const { return Position; }
	glm::vec3 GetPoint() const { return Point; }
	glm::vec3 GetDirection() const { return Direction; }
private:
	static CameraManager* Camera;

	glm::vec3 Position;
	glm::vec3 Point;
	glm::vec3 Direction;
};