#pragma once
#include "Entity.h"
#include<DirectXMath.h>
using namespace DirectX;
class Ship :
	public Entity
{
protected:
	XMFLOAT4 originalRotation;
	float health;

public:
	Ship(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	~Ship();

	void Update(float deltaTime) override;

	void GetInput(float deltaTime) override;

	float GetHealth();

	bool IsColliding(std::shared_ptr<Entity> other) override;

	void SetOriginalRotation(XMFLOAT4 originalRotation);

};

