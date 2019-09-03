#pragma once
#include"Mesh.h"
#include<d3d11.h>
#include"Vertex.h"
#include<memory>
#include"Material.h"
using namespace DirectX;
class Entity
{
	//vectors for scale and position
	XMFLOAT3 position;
	XMFLOAT3 scale;

	//quaternion for rotation
	XMFLOAT4 rotation;

	//model matrix of the entity
	XMFLOAT4X4 modelMatrix;

	bool recalculateMatrix; // boolean to check if any transform has changed

	std::shared_ptr<Mesh> mesh; //mesh associated with this entity

	std::shared_ptr<Material> material; //material of this entity

public:

	//constructor which accepts a mesh
	Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	~Entity();

	//getters and setters
	void SetPosition(XMFLOAT3 position);
	void SetRotation(XMFLOAT4 rotation);
	void SetScale(XMFLOAT3 scale);
	void SetModelMatrix(XMFLOAT4X4 matrix);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetScale();
	XMFLOAT4 GetRotation();
	XMFLOAT4X4 GetModelMatrix();

	std::shared_ptr<Mesh> GetMesh();

	//method that prepares the material and sends it to the gpu
	void PrepareMaterial(XMFLOAT4X4 view, XMFLOAT4X4 projection);
};

