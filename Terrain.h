#pragma once
#include<vector>
#include"Vertex.h"
#include<d3d11.h>
#include<DirectXMath.h>
#include"Mesh.h"
#include<memory>
#include<fstream>

using namespace DirectX;
class Terrain
{
	//terrain mesh
	std::shared_ptr<Mesh> terrainMesh;
	unsigned int terrainWidth;
	unsigned int terrainHeight;
	XMFLOAT3* heightmap;
	Vertex* vertices;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	XMFLOAT4X4 world;

public:
	Terrain();

	void LoadHeightMap(ID3D11Device* device, std::string filename="default");

	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();
	XMFLOAT4X4 GetWorldMatrix();

	~Terrain();
};

