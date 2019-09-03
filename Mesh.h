#pragma once

#include<d3d11.h>
#include<memory>
#include"Vertex.h"
#include<string>
#include<vector>
#include<fstream>

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
	Mesh(std::string fileName,ID3D11Device* device);
	~Mesh();

	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();
	unsigned int GetIndexCount();

	//method to load obj files
	void LoadOBJ(std::string fileName);
};

