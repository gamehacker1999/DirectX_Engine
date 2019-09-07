#pragma once
#include<d3d11.h>
#include<DirectXMath.h>
#include"Mesh.h"
#include<memory>
#include"SimpleShader.h"
#include"DDSTextureLoader.h"
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

	//texture and samplestate
	ID3D11ShaderResourceView* textureSRV;
	ID3D11SamplerState* sampleState;

	//number of indices
	unsigned int numIndices;

public:
	Skybox();
	~Skybox();

	//method to load skybox
	void LoadSkybox(std::wstring fileName,ID3D11Device* device, ID3D11DeviceContext* context,ID3D11SamplerState* sampleState);
	
	//method to draw skybox
	void PrepareSkybox(XMFLOAT4X4 view, XMFLOAT4X4 projection,XMFLOAT3 cameraPos);

	//getters
	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();
	unsigned int GetIndexCount();
	unsigned int GetStride();

	std::shared_ptr<Mesh> cubeMesh;
};

