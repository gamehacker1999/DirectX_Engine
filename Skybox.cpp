#include "Skybox.h"

Skybox::Skybox()
{
	//chosing to load cube as the mesh for skybox
	cube = nullptr;
	vertexBuffer = nullptr;
	indexBuffer = nullptr;
	numIndices = 0;
	cubeMesh = nullptr;
}

Skybox::~Skybox()
{
	if (vertexBuffer)
		vertexBuffer->Release();
	if (indexBuffer)
		indexBuffer->Release();
	if (textureSRV)
		textureSRV->Release();
	if (sampleState)
		sampleState->Release();

	delete vertexShader;
	delete pixelShader;
}

void Skybox::LoadSkybox(std::wstring fileName, ID3D11Device* device, ID3D11DeviceContext* context, ID3D11SamplerState* sampleState)
{
	//loading the skybox shader
	pixelShader = new SimplePixelShader(device, context);
	pixelShader->LoadShaderFile(L"SkyboxPS.cso");
	vertexShader = new SimpleVertexShader(device, context);
	vertexShader->LoadShaderFile(L"SkyboxVS.cso");

	//loading the dds file
	CreateDDSTextureFromFile(device, context, fileName.c_str(), 0, &textureSRV);

	this->sampleState = sampleState;

	cube = std::make_shared<Mesh>("../../Assets/Models/skyboxCube.obj",device);
}

void Skybox::PrepareSkybox(XMFLOAT4X4 view, XMFLOAT4X4 projection,XMFLOAT3 cameraPos)
{

	//setting the vertex and pixel shaders and drawing them
	vertexShader->SetShader();
	pixelShader->SetShader();

	//adding all the external data
	vertexShader->SetMatrix4x4("view", view);
	vertexShader->SetMatrix4x4("projection", projection);

	vertexShader->SetFloat3("cameraPos",cameraPos);

	//sending all buffer data
	vertexShader->CopyAllBufferData();

	//setting the data in the pixel shader
	pixelShader->SetSamplerState("basicSampler", sampleState);
	pixelShader->SetShaderResourceView("skyboxTexture", textureSRV);
	pixelShader->CopyAllBufferData();
}

ID3D11Buffer* Skybox::GetVertexBuffer()
{
	//return vertexBuffer;
	return cube->GetVertexBuffer();
}

ID3D11Buffer* Skybox::GetIndexBuffer()
{
	//return indexBuffer;
	return cube->GetIndexBuffer();
}

ID3D11ShaderResourceView* Skybox::GetSkyboxTexture()
{
	return textureSRV;
}

unsigned int Skybox::GetIndexCount()
{
	//return numIndices;
	return cube->GetIndexCount();
}

unsigned int Skybox::GetStride()
{
	return sizeof(SkyboxData);
}

SimplePixelShader* Skybox::GetPixelShader()
{
	return pixelShader;
}

