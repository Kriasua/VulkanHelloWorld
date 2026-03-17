#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//定义移动方向枚举
enum CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT };

const float YAW = 225.0f;
const float PITCH = -30.0f; // 初始俯仰角，微微向下看
const float SPEED = 2.5f;   // 移动速度
const float SENSITIVITY = 0.1f;   // 鼠标灵敏度
const float ZOOM = 45.0f;  // 视野 (FOV)

class Camera
{
public:
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	bool active = true;

	// 欧拉角
	float Yaw;
	float Pitch;

	// 选项
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// 构造函数：默认把相机放在坐标 (2, 2, 2) 也就是你之前看木屋的位置，Z轴朝上
	Camera(glm::vec3 position = glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f), float yaw = YAW, float pitch = PITCH)
		: Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// 获取 View 矩阵 
	glm::mat4 getViewMatrix() const {
		return glm::lookAt(Position, Position + Front, Up);
	}

	// 获取 Projection 矩阵 
	glm::mat4 getProjectionMatrix(float aspect) const {
		glm::mat4 proj = glm::perspective(glm::radians(Zoom), aspect, 0.1f, 100.0f);
		proj[1][1] *= -1;
		return proj;
	}

	void processKeyboard(CameraMovement move, float deltaTime)
	{
		float vel = MovementSpeed * deltaTime;
		if (move == FORWARD) Position += Front * vel;
		if(move == BACKWARD) Position -= Front * vel;
		if(move == LEFT) Position -= Right * vel;
		if(move == RIGHT) Position += Right * vel;
	}

	// 处理鼠标移动
	void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
		if (active)
		{
			xoffset *= MouseSensitivity;
			yoffset *= MouseSensitivity;

			Yaw += xoffset;
			Pitch += yoffset;

			// 防止视角发生“翻转”的死锁
			if (constrainPitch) {
				if (Pitch > 89.0f)  Pitch = 89.0f;
				if (Pitch < -89.0f) Pitch = -89.0f;
			}
			updateCameraVectors();
		}
	}

	void reset()
	{
		Position = glm::vec3(2.0f, 2.0f, 2.0f);
		Front = glm::vec3(0.0f, 0.0f, -1.0f);
		Pitch = PITCH;
		Yaw = YAW;
		updateCameraVectors();
	}

private:
	void updateCameraVectors() {

		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.z = sin(glm::radians(Pitch));
		Front = glm::normalize(front);

		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
	}
};


