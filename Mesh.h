#pragma once

#include<d3d11.h>
#include<memory>
#include"Vertex.h"
#include<string>
#include<vector>
#include<fstream>
#include<DirectXMath.h>

using namespace DirectX;
//class to hold the index and vertex data for basic geometry
class Mesh
{
	//buffers to hold vertex and index buffers
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	unsigned int numIndices; //number of indices in the mesh

public:

	//constructor and destructor
	Mesh(Vertex* vertices, unsigned int numVertices, unsigned int* indices, int numIndices, ID3D11Device* device);
	Mesh(std::string fileName, ID3D11Device* device);
	void CalculateTangents(std::vector<Vertex>& vertices, std::vector<XMFLOAT3>& position, std::vector<XMFLOAT3>& normals,
		std::vector<XMFLOAT2>& uvs, std::vector<XMFLOAT3>& tangents, std::vector<XMFLOAT3>& bitangents, unsigned int vertCount);
	~Mesh();

	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();
	unsigned int GetIndexCount();

	//method to load obj files
	void LoadOBJ(std::string fileName);
};

