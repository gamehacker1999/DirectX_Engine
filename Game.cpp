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
	directionalLight.ambientColor = XMFLOAT4(0.1f, 0.1f ,0.1f,1.f);
	directionalLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLight.direction = XMFLOAT3(0.0f, 0.0f, 1.0f);

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
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/GoldDiffuse.png",0,&textureSRV);

	//trying to load a normalMap
	ID3D11ShaderResourceView* normalTextureSRV;
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/GoldNormal.png", 0, &normalTextureSRV);

	ID3D11ShaderResourceView* roughnessTextureSRV;
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/LayeredRoughness.png", 0, &roughnessTextureSRV);

	ID3D11ShaderResourceView* metalnessTextureSRV;
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/GridMetallic.png", 0, &metalnessTextureSRV);

	//creating a sampler state
	ID3D11SamplerState* samplerState;
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
	std::shared_ptr<Material> material = std::make_shared<Material>(vertexShader, pixelShader,samplerState,
		textureSRV, normalTextureSRV,roughnessTextureSRV,metalnessTextureSRV);

	std::shared_ptr<Mesh> sphere = std::make_shared<Mesh>("../../Assets/Models/cube.obj",device);
	std::shared_ptr<Mesh> helix = std::make_shared<Mesh>("../../Assets/Models/sphere.obj", device);


	entities.emplace_back(std::make_shared<Entity>(sphere, material));
	entities.emplace_back(std::make_shared<Entity>(helix, material));

	auto position = entities[1]->GetPosition();
	position.x -= 0;
	position.z += 1;
	entities[0]->SetPosition(position);
	entities[0]->SetScale(XMFLOAT3(2, 2, 2));
	//entities[0]->SetScale(XMFLOAT3(2.f, 2.f, 2.f));

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
	skybox->LoadSkybox(L"../../Assets/Textures/cubemap.dds", device, context,samplerStateCube);

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
	/**/context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(shadowDepthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(0, nullptr, shadowDepthStencil);

	context->RSSetViewports(1, &shadowViewport);
	//context->RSSetState(shadowRasterizerState);

	XMFLOAT3 directionLightPosition;
	XMFLOAT3 center(0.0f, 0.0f, 0.0f);
	XMFLOAT3 up(0.0f, 1.0f, 0.0f);

	//taking the center of the camera and backing up from the direction of the light
	//this is the position of the light
	XMStoreFloat3(&directionLightPosition, XMLoadFloat3(&center) - XMLoadFloat3(&directionalLight.direction) * 30.f);
	//creating the camera look to matrix
	auto tempLightView = XMMatrixLookToLH(XMLoadFloat3(&directionLightPosition), 
		XMLoadFloat3(&directionalLight.direction), XMLoadFloat3(&up));

	//storing the light view matrix
	XMFLOAT4X4 lightView;
	XMStoreFloat4x4(&lightView, XMMatrixTranspose(tempLightView));

	//calculating projection matrix
	XMFLOAT4X4 lightProjection;
	XMMATRIX tempLightProjection = XMMatrixOrthographicOffCenterLH(-5.f, 5.f, -5.f, 5.f, 0.1f, 1000.f);
	XMStoreFloat4x4(&lightProjection, XMMatrixTranspose(tempLightProjection));

	context->PSSetShader(0, 0, 0); //no pixel shader for shadows
	shadowVertexShader->SetShader();


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
		//entities[i]->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightView", lightView);
		//entities[i]->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightProj", lightProjection);
		entities[i]->PrepareMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix());

		//adding lights and sending camera position
		entities[i]->GetMaterial()->GetPixelShader()->SetData("light", &directionalLight, sizeof(DirectionalLight)); //adding directional lights to the scene
		//entities[i]->GetMaterial()->GetPixelShader()->SetData("light2", &directionalLight2, sizeof(DirectionalLight));
		entities[i]->GetMaterial()->GetPixelShader()->SetFloat3("cameraPosition",camera->GetPosition());

		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("cubeMap", skybox->GetSkyboxTexture());
		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("shadowMap", shadowSRV);
		entities[i]->GetMaterial()->GetPixelShader()->SetSamplerState("shadowSampler", shadowSamplerState);
		entities[i]->GetMaterial()->SetPixelShaderData();

		//setting the vertex and index buffer
		auto tempVertexBuffer = entities[i]->GetMesh()->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &tempVertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(entities[i]->GetMesh()->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		//drawing the entity
		context->DrawIndexed(entities[i]->GetMesh()->GetIndexCount(), 0, 0);
	}

	context->OMSetDepthStencilState(dssLessEqual, 0);

	//draw the skybox
	skybox->PrepareSkybox(camera->GetViewMatrix(), camera->GetProjectionMatrix(), camera->GetPosition());
	auto tempVertexBuffer = skybox->GetVertexBuffer();
	auto skyboxStride = skybox->GetStride();
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