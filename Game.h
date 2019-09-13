#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include"Mesh.h"
#include"Entity.h"
#include<vector>
#include"Camera.h"
#include"Lights.h"
#include"Skybox.h"
class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateMatrices();
	void CreateBasicGeometry();

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	//vertexshader for shadows
	SimpleVertexShader* shadowVertexShader;

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	//3 meshes to draw on the scene
	std::shared_ptr<Mesh> mesh1;
	std::shared_ptr<Mesh> mesh2;
	std::shared_ptr<Mesh> mesh3;

	//creating a list of vectors
	std::vector<std::shared_ptr<Entity>> entities;

	//camera
	std::shared_ptr<Camera> camera;

	//creating a directional light
	DirectionalLight directionalLight;
	DirectionalLight directionalLight2;

	//creating a skybox and it's shaders
	SimpleVertexShader* skyboxVS;
	SimplePixelShader* skyboxPS;
	std::shared_ptr<Skybox> skybox;

	//depth stencil for skybox
	ID3D11DepthStencilState* dssLessEqual;


};

