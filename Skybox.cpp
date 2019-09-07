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

	//creating a vertex buffer for the vertices
	SkyboxData vertices[] =
	{
		// Front Face
		{XMFLOAT3(-0.5f, -0.5f, -0.5f)},
		{XMFLOAT3(-0.5f,  0.5f, -0.5f)},
		{XMFLOAT3(0.5f,  0.5f, -0.5f)},
		{XMFLOAT3(0.5f, -0.5f, -0.5f)},

		// Back Face            
		{XMFLOAT3(-0.5f, -0.5f, 0.5f)},
		{XMFLOAT3(0.5f, -0.5f, 0.5f)},
		{XMFLOAT3(0.5f,  0.5f, 0.5f)},
		{XMFLOAT3(-0.5f,  0.5f, 0.5f)},

		// Top Face                
		{XMFLOAT3(-0.5f, 0.5f, -0.5f)},
		{XMFLOAT3(-0.5f, 0.5f,  0.5f)},
		{XMFLOAT3(0.5f, 0.5f,  0.5f)},
		{XMFLOAT3(0.5f, 0.5f, -0.5f)},

		// Bottom Face            
		{XMFLOAT3(-0.5f, -0.5f, -0.5f)},
		{XMFLOAT3(0.5f, -0.5f, -0.5f)},
		{XMFLOAT3(0.5f, -0.5f,  0.5f)},
		{XMFLOAT3(-0.5f, -0.5f,  0.5f)},

		// Left Face            
		{XMFLOAT3(-0.5f, -0.5f,  0.5f)},
		{XMFLOAT3(-0.5f,  0.5f,  0.5f)},
		{XMFLOAT3(-0.5f,  0.5f, -0.5f)},
		{XMFLOAT3(-0.5f, -0.5f, -0.5f)},

		// Right Face            
		{XMFLOAT3(0.5f, -0.5f, -0.5f)},
		{XMFLOAT3(0.5f,  0.5f, -0.5f)},
		{XMFLOAT3(0.5f,  0.5f,  0.5f)},
		{XMFLOAT3(0.5f, -0.5f,  0.5f)},
	};

	unsigned int indices[] =
	{
		// Front Face
		0,  1,  2,
		0,  2,  3,

		// Back Face
		4,  5,  6,
		4,  6,  7,

		// Top Face
		8,  9, 10,
		8, 10, 11,

		// Bottom Face
		12, 13, 14,
		12, 14, 15,

		// Left Face
		16, 17, 18,
		16, 18, 19,

		// Right Face
		20, 21, 22,
		20, 22, 23
	};

	numIndices = 3 * 2 * 6; //number of indices in the index buffer

	//creating vertex buffer
	D3D11_BUFFER_DESC vbd;
	memset(&vbd, 0, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(vertices);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initialVbd;
	initialVbd.pSysMem = reinterpret_cast<void*>(&vertices[0]);

	//describing the vertex buffer
	device->CreateBuffer(&vbd, &initialVbd, &vertexBuffer);

	//creating index buffer
	D3D11_BUFFER_DESC ibd;
	memset(&ibd, 0, sizeof(ibd));

	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.ByteWidth = sizeof(indices);

	D3D11_SUBRESOURCE_DATA initialIbd;
	initialIbd.pSysMem = reinterpret_cast<void*>(&indices[0]);

	device->CreateBuffer(&ibd, &initialIbd, &indexBuffer);

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

unsigned int Skybox::GetIndexCount()
{
	//return numIndices;
	return cube->GetIndexCount();
}

unsigned int Skybox::GetStride()
{
	return sizeof(SkyboxData);
}

