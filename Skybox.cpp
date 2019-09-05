#include "Skybox.h"

Skybox::Skybox()
{
	//chosing to load cube as the mesh for skybox
	cube = nullptr;
	vertexBuffer = nullptr;
	indexBuffer = nullptr;
}

Skybox::~Skybox()
{
	if (vertexBuffer)
		vertexBuffer->Release();
	if (indexBuffer)
		indexBuffer->Release();
}

void Skybox::LoadSkybox(std::shared_ptr<Mesh> skyboxMesh)
{
	this->cube = skyboxMesh;

	D3D11_BUFFER_DESC vbd;
	memset(&vbd, 0, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	
}

void Skybox::DrawSkybox(XMFLOAT4X4 view, XMFLOAT4X4 projection)
{
}
