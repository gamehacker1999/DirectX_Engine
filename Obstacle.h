#pragma once
#include "Entity.h"
class Obstacle :
	public Entity
{
public:
	Obstacle(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	~Obstacle();
	bool IsColliding(std::shared_ptr<Entity> other) override;
	void Update(float deltaTime) override;
};

