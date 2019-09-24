#include "Game.h"
#include "Vertex.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	vertexShader = 0;
	pixelShader = 0;
	shadowVertexShader = nullptr;
	shadowPixelShader = nullptr;
	pbrPixelShader = nullptr;

	cubemapViews.reserve(6); //6 view matrices for 6 faces of the cube

	irradianceMapTexture = nullptr;
	irradienceDepthStencil = nullptr;
	irradienceSRV = nullptr;
	irradiancePS = nullptr;
	irradianceVS = nullptr;

	prefilteredSRV = nullptr;
	prefileteredMapTexture = nullptr;
	prefilteredMapPS = nullptr;

	integrationBRDFPS = nullptr;

	fullScreenTriangleVS = nullptr;

	prevMousePos = { 0,0 };	

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
	
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;
	
	if (shadowVertexShader)
		delete shadowVertexShader;

	if (shadowPixelShader)
		delete shadowPixelShader;

	if (pbrPixelShader)
		delete pbrPixelShader;

	if (samplerState)
		samplerState->Release();

	if (irradianceMapTexture)
		irradianceMapTexture->Release();

	if (irradienceDepthStencil)
		irradienceDepthStencil->Release();

	if (irradienceSRV)
		irradienceSRV->Release();

	//releasing the render targets
	for (size_t i = 0; i < 6; i++)
	{
		if (irradienceRTV[i])
			irradienceRTV[i]->Release();
	}

	if (prefilteredSRV)
		prefilteredSRV->Release();

	if (prefileteredMapTexture)
		prefileteredMapTexture->Release();

	if (irradiancePS)
		delete irradiancePS;

	if (irradianceVS)
		delete irradianceVS;

	if (prefilteredMapPS)
		delete prefilteredMapPS;

	if (integrationBRDFPS)
		delete integrationBRDFPS;

	if (fullScreenTriangleVS)
		delete fullScreenTriangleVS;

	//releasing depth stencil
	dssLessEqual->Release();

}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateMatrices();
	CreateBasicGeometry();

	//initalizing camera
	camera = std::make_shared<Camera>(XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f));

	camera->CreateProjectionMatrix((float)width / height); //creating the camera projection matrix

	//specifying the directional light
	directionalLight.ambientColor = XMFLOAT4(0.3f, 0.3f ,0.3f,1.f);
	directionalLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLight.direction = XMFLOAT3(1.0f, -1.0f, 0.0f);

	//second light
	directionalLight2.ambientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	directionalLight2.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLight2.direction = XMFLOAT3(-1.0f, -1.0f, 1.0f);

	//setting depth stencil for skybox;
		//depth stencil state for skybox
	D3D11_DEPTH_STENCIL_DESC dssDesc;
	memset(&dssDesc, 0, sizeof(dssDesc));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	device->CreateDepthStencilState(&dssDesc, &dssLessEqual);

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CreateIrradianceMaps();

	CreatePrefilteredMaps();

	CreateEnvironmentLUTs();

}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{

	vertexShader = new SimpleVertexShader(device, context);
	vertexShader->LoadShaderFile(L"VertexShader.cso");

	pixelShader = new SimplePixelShader(device, context);
	pixelShader->LoadShaderFile(L"PixelShader.cso");

	shadowVertexShader = new SimpleVertexShader(device, context);
	shadowVertexShader->LoadShaderFile(L"ShadowsVS.cso");

	shadowPixelShader = new SimplePixelShader(device, context);
	shadowPixelShader->LoadShaderFile(L"ShadowsPS.cso");

	pbrPixelShader = new SimplePixelShader(device, context);
	pbrPixelShader->LoadShaderFile(L"PBRPixelShader.cso");

	irradiancePS = new SimplePixelShader(device, context);
	irradiancePS->LoadShaderFile(L"IrradianceMapPS.cso");

	irradianceVS = new SimpleVertexShader(device, context);
	irradianceVS->LoadShaderFile(L"IrradianceMapVS.cso");

	prefilteredMapPS = new SimplePixelShader(device, context);
	prefilteredMapPS->LoadShaderFile(L"PrefilteredMapPS.cso");

	integrationBRDFPS = new SimplePixelShader(device, context);
	integrationBRDFPS->LoadShaderFile(L"IntegrationBRDFPixelShader.cso");

	fullScreenTriangleVS = new SimpleVertexShader(device, context);
	fullScreenTriangleVS->LoadShaderFile(L"FullScreenTriangleVS.cso");
}



// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void Game::CreateMatrices()
{
	// Set up world matrix
	// - In an actual game, each object will need one of these and they should
	//    update when/if the object moves (every frame)
	// - You'll notice a "transpose" happening below, which is redundant for
	//    an identity matrix.  This is just to show that HLSL expects a different
	//    matrix (column major vs row major) than the DirectX Math library
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W)); // Transpose for HLSL!

	// Create the View matrix
	// - In an actual game, recreate this matrix every time the camera 
	//    moves (potentially every frame)
	// - We're using the LOOK TO function, which takes the position of the
	//    camera and the direction vector along which to look (as well as "up")
	// - Another option is the LOOK AT function, to look towards a specific
	//    point in 3D space
	XMVECTOR pos = XMVectorSet(0, 0, -5, 0);
	XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(
		pos,     // The position of the "camera"
		dir,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V)); // Transpose for HLSL!

	// Create the Projection matrix
	// - This should match the window's aspect ratio, and also update anytime
	//    the window resizes (which is already happening in OnResize() below)
	/*XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!*/
}


// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{

	//adding three entities with the meshes
	entities.reserve(100);

	ID3D11ShaderResourceView* textureSRV;
	//trying to load a texture
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/CerebrusDiffuse.png",0,&textureSRV);

	//trying to load a normalMap
	ID3D11ShaderResourceView* normalTextureSRV;
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/CerebrusNormal.png", 0, &normalTextureSRV);

	ID3D11ShaderResourceView* roughnessTextureSRV;
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/CerebrusRoughness.png", 0, &roughnessTextureSRV);

	ID3D11ShaderResourceView* metalnessTextureSRV;
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/CerebrusMetallic.png", 0, &metalnessTextureSRV);

	ID3D11ShaderResourceView* goldTextureSRV;
	//trying to load a texture
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/GoldDiffuse.png", 0, &goldTextureSRV);

	//trying to load a normalMap
	ID3D11ShaderResourceView* goldNormalTextureSRV;
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/GoldNormal.png", 0, &goldNormalTextureSRV);

	ID3D11ShaderResourceView* goldRoughnessTextureSRV;
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/GoldRoughness.png", 0, &goldRoughnessTextureSRV);

	ID3D11ShaderResourceView* goldMetalnessTextureSRV;
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/GridMetallic.png", 0, &goldMetalnessTextureSRV);

	//creating a sampler state
	//sampler state description
	D3D11_SAMPLER_DESC samplerDesc;
	memset(&samplerDesc, 0, sizeof(samplerDesc));
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &samplerState); //creating the sampler state
	
	//creating a material for these entities
	std::shared_ptr<Material> material = std::make_shared<Material>(vertexShader, pbrPixelShader,samplerState,
		textureSRV, normalTextureSRV,roughnessTextureSRV,metalnessTextureSRV);

	std::shared_ptr<Material> goldMaterial = std::make_shared<Material>(vertexShader, pbrPixelShader, samplerState,
		goldTextureSRV, goldNormalTextureSRV, goldRoughnessTextureSRV, goldMetalnessTextureSRV);

	std::shared_ptr<Mesh> cerebrus = std::make_shared<Mesh>("../../Assets/Models/cerebrus.obj",device);
	std::shared_ptr<Mesh> object = std::make_shared<Mesh>("../../Assets/Models/cube.obj", device);
	//std::shared_ptr<Mesh> object = std::make_shared<Mesh>("../../Assets/Models/helix.obj", device);

	entities.emplace_back(std::make_shared<Entity>(cerebrus, material));
	entities.emplace_back(std::make_shared<Entity>(object, goldMaterial));
	//entities.emplace_back(std::make_shared<Entity>(object, material));


	auto position = entities[1]->GetPosition();
	entities[1]->SetScale(XMFLOAT3(10, 10, 10));
	position.x -= 0.f;
	position.z += 1.f;
	position.y -= 7.f;
	entities[1]->SetPosition(position);

	entities[0]->SetScale(XMFLOAT3(0.05f, 0.05f, 0.05f));
	auto q1 = entities[0]->GetRotation();

	auto q2 = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), 3.14159f);

	auto tempFinalRot =  XMQuaternionMultiply(XMLoadFloat4(&q1), q2);

	XMStoreFloat4(&q1, tempFinalRot);

	entities[0]->SetRotation(q1);
	
	//entities[0]->SetScale(XMFLOAT3(10.f, 10.f, 10.f));

	//entities[2]->SetPosition(XMFLOAT3(-2.f, 1.f, 0.f));

	ID3D11SamplerState* samplerStateCube;
	//sampler state description
	memset(&samplerDesc, 0, sizeof(samplerDesc));
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &samplerStateCube); //creating the sampler state

	skybox = std::make_shared<Skybox>();
	//creating skybox
	skybox->LoadSkybox(L"../../Assets/Textures/skybox3.dds", device, context,samplerStateCube);

}

void Game::CreateIrradianceMaps()
{
	XMFLOAT4X4 cubePosxView;
	XMFLOAT4X4 cubeNegxView;
	XMFLOAT4X4 cubePoszView;
	XMFLOAT4X4 cubeNegzView;
	XMFLOAT4X4 cubePosyView;
	XMFLOAT4X4 cubeNegyView;

	//postive x face of cube
	XMStoreFloat4x4(&cubePosxView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubePosxView);

	//negative x face of cube
	XMStoreFloat4x4(&cubeNegxView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubeNegxView);

	//postive y face of cube
	XMStoreFloat4x4(&cubePosyView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f))));
	cubemapViews.emplace_back(cubePosyView);

	//negative y face of cube
	XMStoreFloat4x4(&cubeNegyView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f))));
	cubemapViews.emplace_back(cubeNegyView);

	//postive z face of cube
	XMStoreFloat4x4(&cubePoszView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubePoszView);

	//negative z face of cube
	XMStoreFloat4x4(&cubeNegzView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubeNegzView);

	//setting the cubemap projection
	XMStoreFloat4x4(&cubemapProj, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), 1.0f, 0.1f, 10000.f)));

	//generating the irradience map
	D3D11_TEXTURE2D_DESC irradienceTexDesc;
	//ZeroMemory(&irradienceTexDesc, sizeof(D3D11_TEXTURE2D_DESC));
	irradienceTexDesc.ArraySize = 6;
	irradienceTexDesc.MipLevels = 1;
	irradienceTexDesc.Width = 64;
	irradienceTexDesc.Height = 64;
	irradienceTexDesc.CPUAccessFlags = 0;
	irradienceTexDesc.SampleDesc.Count = 1;
	irradienceTexDesc.SampleDesc.Quality = 0;
	irradienceTexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	irradienceTexDesc.Usage = D3D11_USAGE_DEFAULT;
	irradienceTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	irradienceTexDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	device->CreateTexture2D(&irradienceTexDesc, 0, &irradianceMapTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC irradienceSRVDesc;
	ZeroMemory(&irradienceSRV, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	irradienceSRVDesc.Format = irradienceTexDesc.Format;
	irradienceSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	irradienceSRVDesc.TextureCube.MipLevels = 1;
	irradienceSRVDesc.TextureCube.MostDetailedMip = 0;

	device->CreateShaderResourceView(irradianceMapTexture, &irradienceSRVDesc, &irradienceSRV);

	D3D11_RENDER_TARGET_VIEW_DESC irradianceRTVDesc;
	ZeroMemory(&irradianceRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	//irradianceRTVDesc
	irradianceRTVDesc.Format = irradienceTexDesc.Format;
	irradianceRTVDesc.Texture2DArray.ArraySize = 1;
	irradianceRTVDesc.Texture2DArray.MipSlice = 0;
	irradianceRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;

	//creating viewport
	ZeroMemory(&irradianceViewport, sizeof(D3D11_VIEWPORT));
	irradianceViewport.Width = (float)64;
	irradianceViewport.Height = (float)64;
	irradianceViewport.MaxDepth = 1.0f;
	irradianceViewport.MinDepth = 0.0f;
	irradianceViewport.TopLeftX = 0.0f;
	irradianceViewport.TopLeftY = 0.0f;

	const float color[4] = { 0.6f, 0.6f, 0.6f, 0.0f };

	UINT offset = 0;
	UINT stride = sizeof(Vertex);

	for (UINT i = 0; i < 6; i++)
	{
		irradianceRTVDesc.Texture2DArray.FirstArraySlice = i;
		device->CreateRenderTargetView(irradianceMapTexture, &irradianceRTVDesc, &irradienceRTV[i]);	

		context->OMSetRenderTargets(1, &irradienceRTV[i], 0);
		context->RSSetViewports(1, &irradianceViewport);
		context->ClearRenderTargetView(irradienceRTV[i], color);

		irradianceVS->SetMatrix4x4("view", cubemapViews[i]);
		irradianceVS->SetMatrix4x4("projection", cubemapProj);
		irradiancePS->SetShaderResourceView("skybox", skybox->GetSkyboxTexture());
		irradiancePS->SetSamplerState("basicSampler", samplerState);

		irradiancePS->CopyAllBufferData();
		irradianceVS->CopyAllBufferData();

		irradiancePS->SetShader();
		irradianceVS->SetShader();

		auto tempVertexBuffer = skybox->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &tempVertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(skybox->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		context->OMSetDepthStencilState(dssLessEqual, 0);

		context->DrawIndexed(skybox->GetIndexCount(), 0, 0);


	}

	
}

void Game::CreatePrefilteredMaps()
{

	XMFLOAT4X4 cubePosxView;
	XMFLOAT4X4 cubeNegxView;
	XMFLOAT4X4 cubePoszView;
	XMFLOAT4X4 cubeNegzView;
	XMFLOAT4X4 cubePosyView;
	XMFLOAT4X4 cubeNegyView;

	//postive x face of cube
	XMStoreFloat4x4(&cubePosxView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubePosxView);

	//negative x face of cube
	XMStoreFloat4x4(&cubeNegxView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubeNegxView);

	//postive y face of cube
	XMStoreFloat4x4(&cubePosyView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f))));
	cubemapViews.emplace_back(cubePosyView);

	//negative y face of cube
	XMStoreFloat4x4(&cubeNegyView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f))));
	cubemapViews.emplace_back(cubeNegyView);

	//postive z face of cube
	XMStoreFloat4x4(&cubePoszView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubePoszView);

	//negative z face of cube
	XMStoreFloat4x4(&cubeNegzView, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	cubemapViews.emplace_back(cubeNegzView);

	//setting the cubemap projection
	XMStoreFloat4x4(&cubemapProj, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), 1.0f, 0.1f, 10000.f)));

	//creating texture 2d for prefilted map
	D3D11_TEXTURE2D_DESC prefilteredTexDesc;
	UINT maxMipLevel = 5;
	prefilteredTexDesc.ArraySize = 6;
	prefilteredTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	prefilteredTexDesc.CPUAccessFlags = 0;
	prefilteredTexDesc.Usage = D3D11_USAGE_DEFAULT;
	prefilteredTexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	prefilteredTexDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
	prefilteredTexDesc.MipLevels = maxMipLevel;
	prefilteredTexDesc.Width = 128;
	prefilteredTexDesc.Height = 128;
	prefilteredTexDesc.SampleDesc.Count = 1;
	prefilteredTexDesc.SampleDesc.Quality = 0;

	device->CreateTexture2D(&prefilteredTexDesc, 0, &prefileteredMapTexture);

	//creating shader resource
	D3D11_SHADER_RESOURCE_VIEW_DESC prefilteredSRVDesc;
	ZeroMemory(&prefilteredSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	prefilteredSRVDesc.Format = prefilteredTexDesc.Format;
	prefilteredSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	prefilteredSRVDesc.TextureCube.MipLevels = maxMipLevel;
	prefilteredSRVDesc.TextureCube.MostDetailedMip = 0;

	device->CreateShaderResourceView(prefileteredMapTexture, &prefilteredSRVDesc, &prefilteredSRV);

	const float color[4] = { 0.6f, 0.6f, 0.6f, 0.0f };

	UINT offset = 0;
	UINT stride = sizeof(Vertex);

	for (size_t mip = 0; mip < maxMipLevel; mip++)
	{
		//each mip is sqrt times smaller than previous one
		double width = 128 * std::pow(0.5f, mip);
		double height = 128 * std::pow(0.5f, mip);

		D3D11_RENDER_TARGET_VIEW_DESC prefilteredRTVDesc;
		ZeroMemory(&prefilteredRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		prefilteredRTVDesc.Format = prefilteredRTVDesc.Format;
		prefilteredRTVDesc.Texture2DArray.ArraySize = 1;
		prefilteredRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		prefilteredRTVDesc.Texture2DArray.MipSlice = (UINT)mip;

		D3D11_VIEWPORT currentMipViewport;
		ZeroMemory(&currentMipViewport, sizeof(currentMipViewport));
		currentMipViewport.Width = (float)width;
		currentMipViewport.Height = (float)height;
		currentMipViewport.MinDepth = 0.0f;
		currentMipViewport.MaxDepth = 1.0f;
		currentMipViewport.TopLeftX = 0.0f;
		currentMipViewport.TopLeftY = 0.0f;

		float roughness = float(mip) / float(maxMipLevel - 1);

		for (size_t i = 0; i < 6; i++)
		{
			prefilteredRTVDesc.Texture2DArray.FirstArraySlice = (UINT)i;
			device->CreateRenderTargetView(prefileteredMapTexture, &prefilteredRTVDesc, &prefilteredRTV[i]);

			context->OMSetRenderTargets(1, &prefilteredRTV[i], 0);
			context->RSSetViewports(1, &currentMipViewport);
			context->ClearRenderTargetView(prefilteredRTV[i], color);

			irradianceVS->SetMatrix4x4("view", cubemapViews[i]);
			irradianceVS->SetMatrix4x4("projection", cubemapProj);
			prefilteredMapPS->SetShaderResourceView("skybox", skybox->GetSkyboxTexture());
			prefilteredMapPS->SetSamplerState("basicSampler", samplerState);
			prefilteredMapPS->SetFloat("roughness", roughness);

			prefilteredMapPS->CopyAllBufferData();
			irradianceVS->CopyAllBufferData();

			prefilteredMapPS->SetShader();
			irradianceVS->SetShader();

			auto tempVertexBuffer = skybox->GetVertexBuffer();
			context->IASetVertexBuffers(0, 1, &tempVertexBuffer, &stride, &offset);
			context->IASetIndexBuffer(skybox->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

			context->OMSetDepthStencilState(dssLessEqual, 0);

			context->DrawIndexed(skybox->GetIndexCount(), 0, 0);

		}

		for (size_t i = 0; i < 6; i++)
		{
			if (prefilteredRTV[i])
				prefilteredRTV[i]->Release();
		}

	}
	
}

void Game::CreateEnvironmentLUTs()
{

	D3D11_TEXTURE2D_DESC integrationBrdfDesc;
	//ZeroMemory(&irradienceTexDesc, sizeof(D3D11_TEXTURE2D_DESC));
	integrationBrdfDesc.ArraySize = 1;
	integrationBrdfDesc.MipLevels = 0;
	integrationBrdfDesc.Width = 512;
	integrationBrdfDesc.Height = 512;
	integrationBrdfDesc.CPUAccessFlags = 0;
	integrationBrdfDesc.SampleDesc.Count = 1;
	integrationBrdfDesc.SampleDesc.Quality = 0;
	integrationBrdfDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	integrationBrdfDesc.Usage = D3D11_USAGE_DEFAULT;
	integrationBrdfDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	integrationBrdfDesc.MiscFlags = 0;

	device->CreateTexture2D(&integrationBrdfDesc, 0, &environmentBrdfTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC environmentBrdfSRVDesc;
	ZeroMemory(&environmentBrdfSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	environmentBrdfSRVDesc.Format = integrationBrdfDesc.Format;
	environmentBrdfSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	environmentBrdfSRVDesc.Texture2D.MipLevels = 1;
	environmentBrdfSRVDesc.Texture2D.MostDetailedMip = 0;

	device->CreateShaderResourceView(environmentBrdfTexture, &environmentBrdfSRVDesc, &environmentBrdfSRV);

	D3D11_RENDER_TARGET_VIEW_DESC enironmentBrdfRTVDesc;
	ZeroMemory(&enironmentBrdfRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	enironmentBrdfRTVDesc.Format = integrationBrdfDesc.Format;
	enironmentBrdfRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(environmentBrdfTexture, &enironmentBrdfRTVDesc, &environmentBrdfRTV);

	//creating a quad to render the LUT to
	std::shared_ptr<Mesh> quad = std::make_shared<Mesh>("../../Assets/Models/quad.obj", device);

	const float color[4] = { 0.6f, 0.6f, 0.6f, 0.0f };

	UINT offset = 0;
	UINT stride = sizeof(Vertex);

	D3D11_VIEWPORT integrationBrdfViewport;
	ZeroMemory(&integrationBrdfViewport, sizeof(D3D11_VIEWPORT));
	integrationBrdfViewport.Height = 512;
	integrationBrdfViewport.Width = 512;
	integrationBrdfViewport.MaxDepth = 1.0f;
	integrationBrdfViewport.MinDepth = 0.0f;
	integrationBrdfViewport.TopLeftX = 0.0f;
	integrationBrdfViewport.TopLeftY = 0.0f;

	context->RSSetViewports(1, &integrationBrdfViewport);
	context->OMSetRenderTargets(1, &environmentBrdfRTV, 0);
	context->ClearRenderTargetView(environmentBrdfRTV, color);

	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	vertexShader->SetMatrix4x4("world", world);

	XMFLOAT4X4 view;
	XMStoreFloat4x4(&view, XMMatrixTranspose(XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	vertexShader->SetMatrix4x4("view", view);

	XMFLOAT4X4 proj;
	XMStoreFloat4x4(&proj, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), 1.0f, 0.1f, 10000.f)));
	vertexShader->SetMatrix4x4("projection", proj);

	//vertexShader->CopyAllBufferData();
	//vertexShader->SetShader();

	integrationBRDFPS->SetShader();
	fullScreenTriangleVS->SetShader();

	//auto tempVertBuffer = quad->GetVertexBuffer();
	//context->IASetVertexBuffers(0,1,&tempVertBuffer,&stride,&offset);
	//context->IASetIndexBuffer(quad->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	context->Draw(3, 0);
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	//updating the camera projection matrix
	camera->CreateProjectionMatrix((float)width / height);

	// Update our projection matrix since the window size changed
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,	// Field of View Angle
		(float)width / height,	// Aspect ratio
		0.1f,				  	// Near clip plane distance
		100.0f);			  	// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	//updating the camera
	camera->Update(deltaTime);
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	//stride of each vertex
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	//set depth stencil view to render everything to the shadow depth buffer
	//context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(shadowDepthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(0, nullptr, shadowDepthStencil);

	context->RSSetViewports(1, &shadowViewport);
	context->RSSetState(shadowRasterizerState);

	XMFLOAT3 directionLightPosition;
	XMFLOAT3 center(0.0f, 0.0f, 0.0f);
	XMFLOAT3 up(0.0f, 1.0f, 0.0f);

	//taking the center of the camera and backing up from the direction of the light
	//this is the position of the light
	XMStoreFloat3(&directionLightPosition, XMLoadFloat3(&center) - XMLoadFloat3(&directionalLight.direction) * 10000.f);
	//creating the camera look to matrix
	auto tempLightView = XMMatrixLookToLH(XMLoadFloat3(&directionLightPosition), 
		XMLoadFloat3(&directionalLight.direction), XMLoadFloat3(&up));

	//storing the light view matrix
	XMFLOAT4X4 lightView;
	XMStoreFloat4x4(&lightView, XMMatrixTranspose(tempLightView));

	//calculating projection matrix
	XMFLOAT4X4 lightProjection;
	XMMATRIX tempLightProjection = XMMatrixOrthographicLH((float)shadowViewport.Width/100.f,(float)shadowViewport.Height/100.f,
		0.0f,1000000.0f);
	XMStoreFloat4x4(&lightProjection, XMMatrixTranspose(tempLightProjection));

	shadowVertexShader->SetShader();
	context->PSSetShader(nullptr, nullptr, 0);

	for (size_t i = 0; i < entities.size(); i++)
	{
		auto tempVertexBuffer = entities[i]->GetMesh()->GetVertexBuffer();
		shadowVertexShader->SetMatrix4x4("view", lightView);
		shadowVertexShader->SetMatrix4x4("projection", lightProjection);
		shadowVertexShader->SetMatrix4x4("worldMatrix", entities[i]->GetModelMatrix());
		shadowVertexShader->CopyAllBufferData();
		context->IASetVertexBuffers(0, 1, &tempVertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(entities[i]->GetMesh()->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		//drawing the entity
		context->DrawIndexed(entities[i]->GetMesh()->GetIndexCount(), 0, 0);
	}

	
	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	context->RSSetState(nullptr);
	context->RSSetViewports(1, &viewport);

	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);

	//looping through the entities to draw them
	for (size_t i = 0; i < entities.size(); i++)
	{
		//preparing material for entity
		entities[i]->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightView", lightView);
		entities[i]->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightProj", lightProjection);
		entities[i]->PrepareMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix());

		//adding lights and sending camera position
		entities[i]->GetMaterial()->GetPixelShader()->SetData("light", &directionalLight, sizeof(DirectionalLight)); //adding directional lights to the scene
		//entities[i]->GetMaterial()->GetPixelShader()->SetData("light2", &directionalLight2, sizeof(DirectionalLight));
		entities[i]->GetMaterial()->GetPixelShader()->SetFloat3("cameraPosition",camera->GetPosition());

		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("cubeMap", skybox->GetSkyboxTexture());
		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("irradianceMap", irradienceSRV);
		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("shadowMap", shadowSRV);
		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("prefilteredMap", prefilteredSRV);
		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("environmentBRDF", environmentBrdfSRV);
		entities[i]->GetMaterial()->GetPixelShader()->SetSamplerState("shadowSampler", shadowSamplerState);

		entities[i]->GetMaterial()->SetPixelShaderData();

		//setting the vertex and index buffer
		auto tempVertexBuffer = entities[i]->GetMesh()->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &tempVertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(entities[i]->GetMesh()->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		//drawing the entity
		context->DrawIndexed(entities[i]->GetMesh()->GetIndexCount(), 0, 0);

		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("shadowMap", nullptr);
	}

	context->OMSetDepthStencilState(dssLessEqual, 0);

	//draw the skybox
	skybox->PrepareSkybox(camera->GetViewMatrix(), camera->GetProjectionMatrix(), camera->GetPosition());
	//skybox->GetPixelShader()->SetShaderResourceView("skyboxTexture", irradienceSRV);
	//skybox->GetPixelShader()->CopyAllBufferData();
	auto tempVertexBuffer = skybox->GetVertexBuffer();
	context->IASetVertexBuffers(0, 1, &tempVertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(skybox->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(skybox->GetIndexCount(), 0, 0);

	context->OMSetDepthStencilState(NULL, 0);

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);

	// Due to the usage of a more sophisticated swap chain effect,
	// the render target must be re-bound after every call to Present()
	context->OMSetRenderTargets(1,&backBufferRTV, depthStencilView);

	
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	//sending the mouse data to the camera

	//how much the mouse has moved since the previous call to this function
	int deltaX = x - prevMousePos.x;
	int deltaY = y - prevMousePos.y;

	//changing the yaw and pitch of the camera
	camera->ChangeYawAndPitch((float)deltaX, (float)deltaY);

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion