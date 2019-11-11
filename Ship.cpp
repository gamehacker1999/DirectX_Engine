#include "Ship.h"

Ship::Ship(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material):Entity(mesh,material)
{
	XMStoreFloat4(&originalRotation, XMQuaternionIdentity());
	health = 5;
	tag = "Player";
}

Ship::~Ship()
{
}

void Ship::Update(float deltaTime)
{

}

void Ship::GetInput(float deltaTime)
{


}

bool Ship::IsColliding(std::shared_ptr<Entity> other)
{
	//checking if it collided with the obstacle
	if (other->GetTag() == "Obstacle"&&useRigidBody
		&& GetRigidBody()->SATCollision(other->GetRigidBody()))
	{
		health -= 1;
		if (health <= 0)
		{
			Die();
		}
		other->Die();
		return true;

	}

	return false;
}

void Ship::SetOriginalRotation(XMFLOAT4 originalRotation)
{
	this->originalRotation = originalRotation;
}
