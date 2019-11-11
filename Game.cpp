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

	celShadingSRV = nullptr;

	terrain = nullptr;

	blendState = nullptr;

	backCullRS = nullptr;

	skyRS = nullptr;

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

	if (environmentBrdfSRV)
		environmentBrdfSRV->Release();

	if (celShadingSRV)
		celShadingSRV->Release();

	if (particleBlendState)
		particleBlendState->Release();

	if (blendState)
		blendState->Release();

	if (backCullRS)
		backCullRS->Release();

	if (skyRS)
		skyRS->Release();

	if (particlePS)
		delete particlePS;

	if (particleVS)
		delete particleVS;

	if (particleTexture)
		particleTexture->Release();

	if (shadowDepthStencil) shadowDepthStencil->Release();
	if (shadowMapTexture) shadowMapTexture->Release();
	if (shadowRasterizerState) { shadowRasterizerState->Release(); }
	if (shadowSamplerState) { shadowSamplerState->Release(); }
	if (shadowSRV) shadowSRV->Release();


	textureSRV->Release();
	//trying to load a texture

	//trying to load a normalMap
	normalTextureSRV->Release();

	roughnessTextureSRV->Release();

	metalnessTextureSRV->Release();

	goldTextureSRV->Release();
	//trying to load a texture

	//trying to load a normalMap
	goldNormalTextureSRV->Release();

	goldRoughnessTextureSRV->Release();

	goldMetalnessTextureSRV->Release();

	waterDiffuse->Release();

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
	CreateBasicGeometry();

	//initalizing camera
	camera = std::make_shared<Camera>(XMFLOAT3(0.0f, 3.5f, -18.0f), XMFLOAT3(0.0f, 0.0f, 1.0f));

	camera->CreateProjectionMatrix((float)width / height); //creating the camera projection matrix

	//specifying the directional light
	directionalLight.ambientColor = XMFLOAT4(0.3f, 0.3f ,0.3f,1.f);
	directionalLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);

	//second light
	directionalLight2.ambientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	directionalLight2.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLight2.direction = XMFLOAT3(-1.0f, -1.0f, 1.0f);

	XMFLOAT3 worldOrigin = XMFLOAT3(0.f, 0.f, 0.f);
	lights[0] = {};
	lights[0].type = LIGHT_TYPE_DIR;
	lights[0].direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	lights[0].diffuse = XMFLOAT3(1.0f, 1.0f, 1.0f);
	XMFLOAT3 lightDir1Pos;
	XMStoreFloat3(&lightDir1Pos, XMLoadFloat3(&worldOrigin) - XMLoadFloat3(&lights[0].direction) * 100);
	lights[0].position = lightDir1Pos;
	lights[0].range = 10.f;

	//setting depth stencil for skybox;
		//depth stencil state for skybox
	D3D11_DEPTH_STENCIL_DESC dssDesc;
	memset(&dssDesc, 0, sizeof(dssDesc));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	device->CreateDepthStencilState(&dssDesc, &dssLessEqual);

	//creating a blend state
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha=D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	device->CreateBlendState(&blendDesc, &blendState);

	D3D11_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].BlendEnable = true;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // Still respect pixel shader output alpha
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blend, &particleBlendState);

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Turns off depth writing
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&dsDesc, &particleDepth);

	//description of the shadow mapping depth buffer
	D3D11_TEXTURE2D_DESC shadowMapDesc;
	ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
	shadowMapDesc.Width = 1024;
	shadowMapDesc.Height = 1024;
	shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowMapDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowMapDesc.ArraySize = 1;
	shadowMapDesc.MipLevels = 1;
	shadowMapDesc.CPUAccessFlags = 0;
	shadowMapDesc.MiscFlags = 0;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.SampleDesc.Quality = 0;
	shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;

	//creating a texture
	device->CreateTexture2D(&shadowMapDesc, nullptr, &shadowMapTexture);

	//description for depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	device->CreateDepthStencilView(shadowMapTexture, &depthStencilViewDesc, &shadowDepthStencil);

	//creating a shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc;
	ZeroMemory(&shadowSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shadowSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSRVDesc.Texture2D.MipLevels = 1;
	shadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(shadowMapTexture, &shadowSRVDesc, &shadowSRV);

	//setting up the shadow viewport
	ZeroMemory(&shadowViewport, sizeof(D3D11_VIEWPORT));
	shadowViewport.Width = 1024.0f;
	shadowViewport.Height = 1024.0f;
	shadowViewport.MinDepth = 0.0f;
	shadowViewport.MaxDepth = 1.0f;
	shadowViewport.TopLeftX = 0.0f;
	shadowViewport.TopLeftY = 0.0f;

	//rasterizer for pixel shader
	D3D11_RASTERIZER_DESC shadowRasterizerDesc;
	ZeroMemory(&shadowRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	shadowRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	shadowRasterizerDesc.CullMode = D3D11_CULL_BACK;
	shadowRasterizerDesc.DepthClipEnable = true;
	shadowRasterizerDesc.FrontCounterClockwise = false;
	shadowRasterizerDesc.DepthBias = 1000;
	shadowRasterizerDesc.DepthBiasClamp = 0.0f;
	shadowRasterizerDesc.SlopeScaledDepthBias = 1.0f;

	//creating this rasterizer
	device->CreateRasterizerState(&shadowRasterizerDesc, &shadowRasterizerState);

	//sampler for the shadow texture
	D3D11_SAMPLER_DESC shadowSamplerDesc;
	memset(&shadowSamplerDesc, 0, sizeof(shadowSamplerDesc));
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	shadowSamplerDesc.BorderColor[1] = 1.0f;
	shadowSamplerDesc.BorderColor[2] = 1.0f;
	shadowSamplerDesc.BorderColor[3] = 1.0f;
	shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;

	device->CreateSamplerState(&shadowSamplerDesc, &shadowSamplerState);


	//terrain = std::make_shared<Terrain>();
	//terrain->LoadHeightMap(device,"../../Assets/Textures/terrain.raw" );

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CreateIrradianceMaps();

	CreatePrefilteredMaps();

	CreateEnvironmentLUTs();

	InitializeEntities();

	bulletCounter = 0;

	shipGas = std::make_shared<Emitter>(300, 50, 0.7f, 0.8f, 0.03f, XMFLOAT4(1, 1.0f, 1.0f, 1.0f),
		XMFLOAT4(1, 0.1f, 0.1f, 0.6f), XMFLOAT3(0, 0, -1.f), XMFLOAT3(0.2f, 0.2f, 0.2f),
		ship->GetPosition(), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT4(-2, 2, -2, 2),
		XMFLOAT3(0.f, -1.f, 0.f), device, particleVS, particlePS, particleTexture);

	shipGas2 = std::make_shared<Emitter>(300, 50, 0.7f, 0.8f, 0.03f, XMFLOAT4(1, 1.0f, 1.0f, 1.0f),
		XMFLOAT4(1, 0.6f, 0.1f, 0.f), XMFLOAT3(0, 0, -1.f), XMFLOAT3(0.2f, 0.2f, 0.2f),
		ship->GetPosition(), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT4(-2, 2, -2, 2),
		XMFLOAT3(0.f, -1.f, 0.f), device, particleVS, particlePS, particleTexture);

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

	waterPS = new SimplePixelShader(device, context);
	waterPS->LoadShaderFile(L"WaterPS.cso");

	particlePS = new SimplePixelShader(device, context);
	particlePS->LoadShaderFile(L"ParticlesPS.cso");

	particleVS = new SimpleVertexShader(device, context);
	particleVS->LoadShaderFile(L"ParticlesVS.cso");
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{

	//adding three entities with the meshes
	entities.reserve(100);

	//trying to load a texture
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/shipDiffuse.jpg",0,&textureSRV);

	//trying to load a normalMap
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/shipNormal.jpg", 0, &normalTextureSRV);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/shipRoughness.jpg", 0, &roughnessTextureSRV);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/shipMetallic.jpg", 0, &metalnessTextureSRV);

	//trying to load a texture
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/BronzeDiffuse.png", 0, &goldTextureSRV);

	//trying to load a normalMap
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/BronzeNormal.png", 0, &goldNormalTextureSRV);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/BronzeRoughness.png", 0, &goldRoughnessTextureSRV);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/BronzeMetallic.png", 0, &goldMetalnessTextureSRV);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/waterDiffuse.jpg", 0, &waterDiffuse);

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/particle.jpg", 0, &particleTexture);

	//loading cel shading
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/ColorBand.jpg",0,&celShadingSRV);

	//creating a sampler state
	//sampler state description
	D3D11_SAMPLER_DESC samplerDesc;
	memset(&samplerDesc, 0, sizeof(samplerDesc));
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &samplerState); //creating the sampler state
	
	//creating a material for these entities

	//also used for obstacles
	material = std::make_shared<Material>(vertexShader, pbrPixelShader,samplerState,
		textureSRV, normalTextureSRV,roughnessTextureSRV,metalnessTextureSRV);

	std::shared_ptr<Material> goldMaterial = std::make_shared<Material>(vertexShader, pbrPixelShader, samplerState,
		goldTextureSRV, goldNormalTextureSRV, goldRoughnessTextureSRV, goldMetalnessTextureSRV);

	waterMat = std::make_shared<Material>(vertexShader,waterPS,samplerState,waterDiffuse);

	shipMesh = std::make_shared<Mesh>("../../Assets/Models/ship.obj",device);
	std::shared_ptr<Mesh> object = std::make_shared<Mesh>("../../Assets/Models/cube.obj", device);
	obstacleMesh = std::make_shared<Mesh>("../../Assets/Models/sphere.obj", device);
	bulletMesh = std::make_shared<Mesh>("../../Assets/Models/sphere.obj", device);

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
	skybox->LoadSkybox(L"../../Assets/Textures/SunnyCubeMap.dds", device, context,samplerStateCube);

	D3D11_RASTERIZER_DESC skyRSDesc = {};
	skyRSDesc.FillMode = D3D11_FILL_SOLID;
	skyRSDesc.CullMode = D3D11_CULL_FRONT;
	device->CreateRasterizerState(&skyRSDesc, &skyRS);
}

void Game::InitializeEntities()
{
	ship = std::make_shared<Ship>(shipMesh, material);
	ship->UseRigidBody();
	ship->SetTag("Player");
	entities.emplace_back(ship);

	auto shipOrientation = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), 3.14159f);
	XMFLOAT4 retShipRotation;
	XMStoreFloat4(&retShipRotation, shipOrientation);
	ship->SetRotation(retShipRotation);
	ship->SetOriginalRotation(retShipRotation);

	for (size_t i = 0; i < MAX_BULLETS; i++)
	{
		std::shared_ptr<Bullet> newBullet = std::make_shared<Bullet>(bulletMesh, material);
		newBullet->UseRigidBody();
		bullets.emplace_back(newBullet);
	}

	std::shared_ptr<Mesh> waterMesh = std::make_shared<Mesh>("../../Assets/Models/quad.obj", device);
	water = std::make_shared<Entity>(waterMesh, waterMat);
	XMFLOAT3 waterScale = XMFLOAT3(3.f, 3.f, 3.f);
	water->SetScale(waterScale);

	XMFLOAT3 waterPos = ship->GetPosition();
	waterPos.y -= 3;
	water->SetPosition(waterPos);

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

	environmentBrdfRTV->Release();
	environmentBrdfTexture->Release();
}

void Game::RestartGame()
{

	//std::this_thread::sleep_for(std::chrono::seconds(2));

	camera->SetPositionTargetAndUp(XMFLOAT3(0.0f, 3.5f, -18.0f), XMFLOAT3(0.0f, 0.0f, 1.0f));

	for (size_t i = 0; i < entities.size(); i++)
	{
		entities[i] = nullptr;
	}

	bulletCounter = 0;
	
	InitializeEntities();


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

	// add obstacles to screen
	frameCounter += deltaTime;

	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i]->GetAliveState())
		{
			entities[i]->Update(deltaTime);
		}
	}

	shipGas->UpdateParticles(deltaTime,totalTime);
	auto shipPos = ship->GetPosition();
	auto shipForward = ship->GetForward();
	XMFLOAT3 em1Pos;
	XMStoreFloat3(&em1Pos, XMLoadFloat3(&shipPos) + XMLoadFloat3(&shipForward) * 3);
	em1Pos.x -= 1.2f;
	shipGas->SetAcceleration(shipForward);
	shipGas->SetPosition(em1Pos);

	shipGas2->UpdateParticles(deltaTime,totalTime);
	shipPos = ship->GetPosition();
	XMFLOAT3 em2Pos;
	XMStoreFloat3(&em2Pos, XMLoadFloat3(&shipPos) + XMLoadFloat3(&shipForward) * 3);
	em2Pos.x += 1.2f;
	shipGas2->SetPosition(em2Pos);


	//checking for collision
	for (int i = 0; i < entities.size(); i++)
	{
		for (int j = 0; j < entities.size(); j++)
		{
			entities[i]->IsColliding(entities[j]);
		}
	}

	entities.erase(std::remove(entities.begin(), entities.end(), nullptr), entities.end());

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

	/*XMFLOAT3 directionLightPosition;
	XMFLOAT3 center(0.0f, 0.0f, 0.0f);*/
	XMFLOAT3 up(0.0f, 1.0f, 0.0f);

	//taking the center of the camera and backing up from the direction of the light
	//this is the position of the light
	//XMStoreFloat3(&directionLightPosition, XMLoadFloat3(&center) - XMLoadFloat3(&directionalLight.direction) * 10000.f);
	//creating the camera look to matrix
	auto tempLightView = XMMatrixLookAtLH(XMLoadFloat3(&lights[0].position), 
		XMLoadFloat3(&ship->GetPosition()), XMLoadFloat3(&up));

	//storing the light view matrix
	XMFLOAT4X4 lightView;
	XMStoreFloat4x4(&lightView, XMMatrixTranspose(tempLightView));

	//calculating projection matrix
	XMFLOAT4X4 lightProjection;
	XMMATRIX tempLightProjection = XMMatrixOrthographicLH(100.f,100.f,
		0.1f,1000.0f);
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
		entities[i]->GetMaterial()->GetPixelShader()->SetData("lights", &lights[0], sizeof(Light)*MAX_LIGHTS);
		entities[i]->GetMaterial()->GetPixelShader()->SetInt("lightCount",2);

		//entities[i]->GetMaterial()->GetPixelShader()->SetData("light2", &directionalLight2, sizeof(DirectionalLight));
		entities[i]->GetMaterial()->GetPixelShader()->SetFloat3("cameraPosition",camera->GetPosition());

		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("cubeMap", skybox->GetSkyboxTexture());
		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("celShading", celShadingSRV);
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


	//drawing the terrain
	/*vertexShader->SetMatrix4x4("world", terrain->GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetViewMatrix());
	vertexShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	pixelShader->SetData("light", &directionalLight, sizeof(DirectionalLight)); //adding directional lights to the scene

	vertexShader->CopyAllBufferData();
	pixelShader->CopyAllBufferData();
	vertexShader->SetShader();
	pixelShader->SetShader();*/

	/*auto tempTerrainVB = terrain->GetVertexBuffer();
	context->IASetVertexBuffers(0, 1, &tempTerrainVB, &stride, &offset);
	context->IASetIndexBuffer(terrain->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	context->DrawIndexed(511 * 511 * 2*3, 0, 0);*/

	context->OMSetDepthStencilState(dssLessEqual, 0);

	//draw the skybox
	context->RSSetState(skyRS);
	skybox->PrepareSkybox(camera->GetViewMatrix(), camera->GetProjectionMatrix(), camera->GetPosition());
	auto tempVertexBuffer = skybox->GetVertexBuffer();
	context->IASetVertexBuffers(0, 1, &tempVertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(skybox->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(skybox->GetIndexCount(), 0, 0);

	context->OMSetDepthStencilState(NULL, 0);

	//rendering particle
	float blend[4] = { 1,1,1,1 };
	context->OMSetBlendState(particleBlendState, blend, 0xffffffff);
	context->OMSetDepthStencilState(particleDepth, 0);

	particlePS->SetSamplerState("sampleOptions", samplerState);
	shipGas->Draw(context, camera,totalTime);
	shipGas2->Draw(context, camera,totalTime);

	context->OMSetDepthStencilState(0, 0);
	context->OMSetBlendState(0, blend, 0xffffffff);

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

	if (buttonState & 0x0001)
	{
		int deltaX = x - prevMousePos.x;
		int deltaY = y - prevMousePos.y;

		//changing the yaw and pitch of the camera
		camera->ChangeYawAndPitch((float)deltaX, (float)deltaY);
	}
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