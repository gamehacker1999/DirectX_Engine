#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
{
	this->mesh = mesh;
	this->material = material;

	XMStoreFloat4x4(&modelMatrix, XMMatrixIdentity()); //setting model matrix as identity

	//initializing position scale and rotation
	position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scale = XMFLOAT3(1.0f, 1.0f,1.0f);

	XMStoreFloat4(&rotation, XMQuaternionIdentity()); //identity quaternion

	//don't need to recalculate matrix now
	recalculateMatrix = false;
}

Entity::~Entity()
{
}

void Entity::SetPosition(XMFLOAT3 position)
{
	this->position = position;
	recalculateMatrix = true; //need to recalculate model matrix now
}

void Entity::SetRotation(XMFLOAT4 rotation)
{
	this->rotation = rotation;
	recalculateMatrix = true; //need to recalculate model matrix now
}

void Entity::SetScale(XMFLOAT3 scale)
{
	this->scale = scale;
	recalculateMatrix = true; //need to recalculate model matrix now
}

void Entity::SetModelMatrix(XMFLOAT4X4 matrix)
{
	this->modelMatrix = matrix;
}

XMFLOAT3 Entity::GetPosition()
{
	return position;
}

XMFLOAT3 Entity::GetScale()
{
	return scale;
}

XMFLOAT4 Entity::GetRotation()
{
	return rotation;
}

XMFLOAT4X4 Entity::GetModelMatrix()
{
	//check if matrix has to be recalculated
	//if yes then calculate it 
	if (recalculateMatrix)
	{
		//getting the translation, scale, and rotation matrices
		XMMATRIX translate = XMMatrixTranslationFromVector(XMLoadFloat3(&position));
		XMMATRIX scaleMat = XMMatrixScalingFromVector(XMLoadFloat3(&scale));
		XMMATRIX rotationMat = XMMatrixRotationQuaternion(XMLoadFloat4(&rotation));

		//calculating the model matrix from these three matrices and storing it
		//we transpose it before storing the matrix
		 XMStoreFloat4x4(&modelMatrix,XMMatrixTranspose(scaleMat*rotationMat*translate));

		recalculateMatrix = false;
	}

	//returning the model matrix
	return modelMatrix;
}

std::shared_ptr<Mesh> Entity::GetMesh()
{
	return mesh;
}

std::shared_ptr<Material> Entity::GetMaterial()
{
	return material;
}

void Entity::PrepareMaterial(XMFLOAT4X4 view, XMFLOAT4X4 projection)
{
	//setting the appropriate data for the shader
	material->GetVertexShader()->SetMatrix4x4("world", GetModelMatrix());
	material->GetVertexShader()->SetMatrix4x4("view", view);
	material->GetVertexShader()->SetMatrix4x4("projection", projection);

	//setting the shaders as active
	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();

	//copying data to gpu
	material->GetVertexShader()->CopyAllBufferData();
	//material->GetPixelShader()->CopyAllBufferData();
}
