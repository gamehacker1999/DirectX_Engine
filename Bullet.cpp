#include "Bullet.h"

Bullet::Bullet(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material):Entity(mesh, material)
{
	lifeTime = 0;
	tag = "bullet";
	isActive = false;
}

Bullet::~Bullet()
{
}

bool Bullet::IsColliding(std::shared_ptr<Entity> other)
{
	if (other->GetTag() == "Obstacle"&&useRigidBody
		&& GetRigidBody()->SATCollision(other->GetRigidBody()))
	{
		this->isAlive = false;
		other->Die();
		return true;
	}

	return false;
}

void Bullet::Update(float deltaTime)
{
	isActive = true;
	position.z += 40*deltaTime;
	SetPosition(position);
	lifeTime += deltaTime;
	isAlive = lifeTime < MAX_LIFETIME;
	if (!isAlive)
	{
		Reset();
	}
}

void Bullet::Reset()
{
	lifeTime = 0;
	position.z = 0;
	isActive = false;
	isAlive = false;
}
