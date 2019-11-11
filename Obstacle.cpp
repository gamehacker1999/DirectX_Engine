#include "Obstacle.h"

Obstacle::Obstacle(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material):Entity(mesh, material)
{
	tag = "Obstacle";
}

Obstacle::~Obstacle()
{
}

bool Obstacle::IsColliding(std::shared_ptr<Entity> other)
{
	return false;
}

void Obstacle::Update(float deltaTime)
{
}
