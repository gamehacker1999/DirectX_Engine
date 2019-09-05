#pragma once
#include<d3d11.h>
#include<DirectXMath.h>
#include"Mesh.h"
#include<memory>
#include"SimpleShader.h"
using namespace DirectX;
class Skybox
{
	struct SkyboxData
	{
		XMFLOAT3 position;
	};

	//model for the skybox
	std::shared_ptr<Mesh> cube;

	//vertex and indexbuffer for the skybox
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	//shaders for the skybox
	SimplePixelShader* pixelShader;
	SimpleVertexShader* vertexShader;

public:
	Skybox();
	~Skybox();

	//method to load skybox
	void LoadSkybox(std::shared_ptr<Mesh> skyboxMesh);
	
	//method to draw skybox
	void DrawSkybox(XMFLOAT4X4 view, XMFLOAT4X4 projection);
};

