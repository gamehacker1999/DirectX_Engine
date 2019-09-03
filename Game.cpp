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
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in memory
	//    over to a DirectX-controlled data structure (the vertex buffer)
	Vertex verticesMesh1[] =
	{
		{ XMFLOAT3(+0.0f, +1.0f, +0.0f), XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.0f,0.0f)},
		{ XMFLOAT3(+1.5f, -1.0f, +0.0f), XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.0f,0.0f)},
		{ XMFLOAT3(-1.5f, -1.0f, +0.0f), XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.0f,0.0f) },
	};

	// Set up the indices, which tell us which vertices to use and in which order
	// - This is somewhat redundant for just 3 vertices (it's a simple example)
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indicesMesh1[] = { 0, 1, 2 };

	mesh1 = std::make_shared<Mesh>(verticesMesh1, 3, indicesMesh1, 3, device);

	//vertices and indices for mesh 2
	Vertex verticesMesh2[] = 
	{
		{XMFLOAT3(1.8f,1.0f,0.0f),XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.0f,0.0f)},
		{XMFLOAT3(2.8f,1.0f,0.0f),XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.0f,0.0f)},
		{XMFLOAT3(2.8f,-1.0f,0.0f),XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.0f,0.0f)},
		{XMFLOAT3(1.8f,-1.0f,0.0f),XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.0f,0.0f)}
	};

	unsigned int indicesMesh2[] = 
	{ 
		3,1,2,
		3,0,1 
	};

	//creating the mesh data for the second mesh
	mesh2 = std::make_shared<Mesh>(verticesMesh2, 4, indicesMesh2, 6, device);

	//setting the vertex and index buffer for the third mesh
	Vertex verticesMesh3[] =
	{
		{XMFLOAT3(-2.3f,1.0f,0.0f),XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.0f,0.0f)},
		{XMFLOAT3(-1.8f,0.0f,0.0f),XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.0f,0.0f)},
		{XMFLOAT3(-1.8f,-1.0f,0.0f),XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.0f,0.0f)},
		{XMFLOAT3(-2.8f,-1.0f,0.0f),XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.0f,0.0f)},
		{XMFLOAT3(-2.8f,0.0f,0.0f),XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.0f,0.0f)}
	};

	unsigned int indicesMesh3[] =
	{
		0,1,4,
		1,2,4,
		2,3,4
	};

	//setting the data for the third mesh
	mesh3 = std::make_shared<Mesh>(verticesMesh3, 5, indicesMesh3, 9,device);

	//adding three entities with the meshes
	entities.reserve(100);

	//creating a material for these entities
	std::shared_ptr<Material> material = std::make_shared<Material>(vertexShader, pixelShader);

	//entities.emplace_back(std::make_shared<Entity>(mesh1,material));
	//entities.emplace_back(std::make_shared<Entity>(mesh1,material)); //this entity is sharing a mesh with the previous one
	//entities.emplace_back(std::make_shared<Entity>(mesh3,material));

	std::shared_ptr<Mesh> cube = std::make_shared<Mesh>("../../Assets/Models/sphere.obj",device);

	entities.emplace_back(std::make_shared<Entity>(cube, material));
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
	camera->CreateProjectionMatrix(float(width / height));

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

	//moving the first entity
	auto pos = entities[0]->GetPosition();
	pos.x += 0.1f * deltaTime;
	entities[0]->SetPosition(pos);

	//making the second entity move in a sine wave
	pos = entities[1]->GetPosition();
	pos.x -= 0.1f * deltaTime;
	float yPos = sin(pos.x);
	pos.y = yPos;
	entities[1]->SetPosition(pos);

	//rotating the third entity
	auto q1 = XMLoadFloat4(&entities[2]->GetRotation());

	//XMFLOAT4 newRotation=rotation; 
	static float rotAngle = 0.001f;

	XMFLOAT3 axis(0.0f, 0.0f, 1.0f*deltaTime);
	auto q2 = XMQuaternionRotationAxis(XMLoadFloat3(&axis), rotAngle); //new orientation for the entity
	XMFLOAT4 finalRotation;

	XMStoreFloat4(&finalRotation,XMQuaternionMultiply(q1, q2));//getting the final rotation
	entities[2]->SetRotation(finalRotation); //setting the final rotation

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

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	// Send data to shader variables
	//  - Do this ONCE PER OBJECT you're drawing
	//  - This is actually a complex process of copying data to a local buffer
	//    and then copying that entire buffer to the GPU.  
	//  - The "SimpleShader" class handles all of that for you.
	/*vertexShader->SetMatrix4x4("world", worldMatrix);
	vertexShader->SetMatrix4x4("view", camera->GetViewMatrix());
	vertexShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());*/

	// Once you've set all of the data you care to change for
	// the next draw call, you need to actually send it to the GPU
	//  - If you skip this, the "SetMatrix" calls above won't make it to the GPU!
	//vertexShader->CopyAllBufferData();

	// Set the vertex and pixel shaders to use for the next Draw() command
	//  - These don't technically need to be set every frame...YET
	//  - Once you start applying different shaders to different objects,
	//    you'll need to swap the current shaders before each draw
	/*vertexShader->SetShader();
	pixelShader->SetShader();*/

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	//looping through the entities to draw them
	for (size_t i = 0; i < entities.size(); i++)
	{
		//setting the model matrix of the entity
		/*vertexShader->SetMatrix4x4("world", entities[i]->GetModelMatrix());
		vertexShader->CopyAllBufferData();*/
		//setting the vertex and index buffer
		entities[i]->PrepareMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix());
		auto tempVertexBuffer = entities[i]->GetMesh()->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &tempVertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(entities[i]->GetMesh()->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		//drawing the entity
		context->DrawIndexed(entities[i]->GetMesh()->GetIndexCount(), 0, 0);
	}

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);

	// Due to the usage of a more sophisticated swap chain effect,
	// the render target must be re-bound after every call to Present()
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
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