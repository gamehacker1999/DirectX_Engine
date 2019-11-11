#include "FollowCamera.h"

FollowCamera::FollowCamera(XMFLOAT3 pos, XMFLOAT3 dir, XMFLOAT3 upward):Camera(pos,dir,upward)
{
	horizontalDistance = 300;
	verticalDistance = 300;
	targetDistance = 100;
	velocity = XMFLOAT3(0, 0, 0);
	springConstant = 64;
	SnapToIdeal();
}

FollowCamera::~FollowCamera()
{
}

void FollowCamera::SetOwner(std::shared_ptr<Entity> owner)
{
	this->owner = owner;

}

void FollowCamera::Update(float deltaTime)
{
	//compute dampning from spring constant
	float dampning = 2.0f * sqrt(springConstant);

	//getting the ideal position of the camera
	XMFLOAT3 idealPosition = ComputeCameraPosition();

	XMFLOAT3 diff;
	XMStoreFloat3(&diff,XMLoadFloat3(&actualPosition) - XMLoadFloat3(&idealPosition));

	XMFLOAT3 acel;

	XMVECTOR tempAcel = springConstant * XMLoadFloat3(&diff) - dampning * XMLoadFloat3(&velocity);
	XMStoreFloat3(&acel, tempAcel);

	velocity.x += acel.x * deltaTime;
	velocity.y += acel.y * deltaTime;
	velocity.z += acel.z * deltaTime;


	actualPosition.x += velocity.x *= deltaTime;
	actualPosition.y += velocity.y *= deltaTime;
	actualPosition.z += velocity.y *= deltaTime;


	//calculating target that is slighly in from of the actor
	XMFLOAT3 target; 
	XMFLOAT3 ownerPos = owner->GetPosition();
	XMFLOAT3 ownerForward = owner->GetForward();
	XMStoreFloat3(&target, XMLoadFloat3(&ownerPos) + XMLoadFloat3(&ownerForward) * targetDistance);

	XMFLOAT3 lookDir;
	XMStoreFloat3(&lookDir, XMVector3Normalize(XMLoadFloat3(&target) - XMLoadFloat3(&actualPosition)));
	//MFLOAT3 up = XMFLOAT3::UnitZ;
	//XMFLOAT3 position = ComputeCameraPosition();

	SetPositionTargetAndUp(actualPosition, lookDir, up);
}

void FollowCamera::SnapToIdeal()
{
	actualPosition = ComputeCameraPosition();
	velocity = XMFLOAT3(0, 0, 0);
	XMFLOAT3 target;
	XMFLOAT3 ownerPos = owner->GetPosition();
	XMFLOAT3 ownerForward = owner->GetForward();

	XMStoreFloat3(&target, XMLoadFloat3(&ownerPos)+XMLoadFloat3(&ownerForward)*targetDistance);

	XMFLOAT3 lookDir;
	XMStoreFloat3(&lookDir, XMVector3Normalize(XMLoadFloat3(&target) - XMLoadFloat3(&actualPosition)));

	SetPositionTargetAndUp(actualPosition, lookDir, up);
}

XMFLOAT3 FollowCamera::ComputeCameraPosition()
{
	XMFLOAT3 camPos = XMFLOAT3(0,0,0);
	XMFLOAT3 ownerPos = owner->GetPosition();
	XMFLOAT3 ownerForward = owner->GetForward();

	XMStoreFloat3(&camPos, XMLoadFloat3(&ownerPos) - horizontalDistance * XMLoadFloat3(&ownerForward) +
		verticalDistance * XMVectorSet(0, 1, 0, 0));

	return camPos;
}
