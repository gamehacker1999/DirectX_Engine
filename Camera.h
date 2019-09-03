#pragma once
#include<d3d11.h>
#include<DirectXMath.h>
using namespace DirectX;

//class to represent the a movable camera
class Camera
{
	//fields for the class
	//view and projection matrices
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;

	//vectors to describe position and direction
	XMFLOAT3 position;
	XMFLOAT3 direction;
	XMFLOAT3 up;

	//roation angles around x and y axis
	float xRotation;
	float yRotation;

public:
	Camera(XMFLOAT3 position, XMFLOAT3 direction, XMFLOAT3 up = XMFLOAT3(0.0f,1.0f,0.0f));

	//getters and setters
	XMFLOAT4X4 GetViewMatrix();
	XMFLOAT4X4 GetProjectionMatrix();

	//method to create projection matrix
	void CreateProjectionMatrix(float aspectRatio);

	//function to get keyboard input
	void ManageKeyboard(float deltaTime);

	//changing the x and y of the mouse to rotate the camera
	void ChangeYawAndPitch(float deltaX, float deltaY);

	//method to update the camera
	void Update(float deltaTime);
};

