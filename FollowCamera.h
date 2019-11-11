#pragma once
#include "Camera.h"
#include "Entity.h"
class FollowCamera :
	public Camera
{
	//horizontal distance behind actor
	float horizontalDistance;
	//vertical distance abobe camera actor
	float verticalDistance;
	//target position, a little in front of the actor
	float targetDistance;

	//adding springiness to the camera
	float springConstant;
	XMFLOAT3 velocity;
	XMFLOAT3 actualPosition;

	std::shared_ptr<Entity> owner;


public:
	FollowCamera(XMFLOAT3 position, XMFLOAT3 direction, XMFLOAT3 up);
	~FollowCamera();
	void SetOwner(std::shared_ptr<Entity> owner);
	void Update(float deltaTime) override;
	void SnapToIdeal();
	XMFLOAT3 ComputeCameraPosition();
};

