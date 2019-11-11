#include "Terrain.h"

Terrain::Terrain()
{
	vertexBuffer = nullptr;
	indexBuffer = nullptr;
	terrainMesh = nullptr;
	terrainWidth = 0;
	terrainHeight = 0;
	heightmap = nullptr;
	vertices = nullptr;
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	
}

void Terrain::LoadHeightMap(ID3D11Device* device, std::string filename)
{
	//vector to hold heights
	std::vector<unsigned char> rawHeights(512 * 512*3);

	//initilizing the vertex list
	//vertices = std::vector<Vertex>(512*512,XMFLOAT3(0,0,0));
	heightmap = new XMFLOAT3[512 * 512];
	vertices = new Vertex[512 * 512];

	//opening the raw file
	std::ifstream ifile;
	ifile.open(filename.c_str(), std::ios_base::binary);

	if (ifile)
	{
		//reading the raw file to vector
		ifile.read((char*)& rawHeights[0], 512 * 512);
		ifile.close();
	}

	//adding all the data to the position list
	int k = 0;
	for (size_t j = 0; j < 512; j++)
	{
		for (size_t i = 0; i < 512; i++)
		{
			size_t index = (j * 512) + i;
			XMFLOAT3 position;
			position.x = (float)i;
			position.z = (float)j;
			position.y = (float)rawHeights[k]/255.0f;

			heightmap[index] = position;

			k += 3;

		}
	}

	//creating the vertices
	for (size_t i = 0; i < 512; i++)
	{
		for (size_t j = 0; j < 512; j++)
		{
			vertices[i * 512 + j].Position = heightmap[i * 512 + j];
			vertices[i * 512 + j].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		}
	}

	//num triangles and vertices
	int numVertices = 512 * 512;
	int numTriangles = 511 * 511 * 2;


	std::vector<unsigned int> indices((size_t)numTriangles*3);

	k = 0;
	float textureU = 0;
	float textureV = 0;

	for (unsigned int i = 0; i < 511; i++)
	{
		for (unsigned int j = 0; j < 511; j++)
		{
			indices[(size_t)k+5] = i * 512 + j; //bottom left of quad
			vertices[i * 512 + j].uv = XMFLOAT2(textureU, textureV + 1.0f);

			indices[(size_t)k + 4] = i * 512 + j + 1; //bottom right of quad
			vertices[i * 512 + j + 1].uv = XMFLOAT2(textureU + 1.0f, textureV + 1.0f);

			indices[(size_t)k+3] = (i + 1) * 512 + j; //top left
			vertices[(i + 1) * 512 + j].uv = XMFLOAT2(textureU, textureV);

			indices[(size_t)k] = (i + 1) * 512 + j; //top left
			vertices[(i + 1) * 512 + j].uv = XMFLOAT2(textureU, textureV);

			indices[(size_t)k + 2] = i * 512 + j + 1; //bottom right of quad
			vertices[i * 512 + j + 1].uv = XMFLOAT2(textureU + 1.0f, textureV + 1.0f);

			indices[(size_t)k+1] = (i+1) * 512 + j + 1; //top right of quad
			vertices[(i+1) * 512 + j + 1].uv = XMFLOAT2(textureU + 1.0f, textureV + 0.0f);

			k += 6;

			textureU++;
		}
		textureU = 0;
		textureV++;
	}

	//getting the normals
	std::vector<XMFLOAT3> tempNormal;

	for (size_t i = 0; i < numTriangles; i++)
	{
		XMVECTOR vec1;
		XMVECTOR vec2;

		//get two edges to get normal of triangle
		vec1 = XMLoadFloat3(&vertices[indices[i*3]].Position) - XMLoadFloat3(&vertices[indices[(i * 3) + 2]].Position);
		vec2 = XMLoadFloat3(&vertices[indices[(i * 3) + 2]].Position) - XMLoadFloat3(&vertices[indices[(i * 3) + 1]].Position);

		XMFLOAT3 unnormalized;
		XMStoreFloat3(&unnormalized, XMVector3Cross(vec2, vec1));

		tempNormal.emplace_back(unnormalized);
	}

	//averaging the normals
	for (size_t i = 0; i < numVertices; i++)
	{
		XMFLOAT3 normalSum(0.0f,0.0f,0.0f);
		int facesUsing = 0;
		for (size_t j = 0; j < numTriangles; j++)
		{
			if (indices[j * 3] == i ||
				indices[(j * 3) + 1] == i ||
				indices[(j * 3) + 2] == i )
			{
				normalSum.x += tempNormal[j].x;
				normalSum.y += tempNormal[j].y;
				normalSum.z += tempNormal[j].z;
				facesUsing++;
			}
		}

		normalSum.x /= facesUsing;
		normalSum.y /= facesUsing;
		normalSum.z /= facesUsing;

		XMStoreFloat3(&normalSum,XMVector3Normalize(XMLoadFloat3(&normalSum)));

		vertices[i].normal = normalSum;	
	}

	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * numVertices;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.StructureByteStride = 0;
	vbd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA intermediateVb = {};
	intermediateVb.pSysMem = reinterpret_cast<void*>(&vertices[0]);

	device->CreateBuffer(&vbd, &intermediateVb, &vertexBuffer);

	//creating index buffer
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.ByteWidth = numTriangles * 3*sizeof(unsigned int);
	ibd.StructureByteStride = 0;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA intermediateIbd = {};
	intermediateIbd.pSysMem = reinterpret_cast<void*>(indices.data());

	device->CreateBuffer(&ibd, &intermediateIbd, &indexBuffer);
}

ID3D11Buffer* Terrain::GetVertexBuffer()
{
	return vertexBuffer;
}

ID3D11Buffer* Terrain::GetIndexBuffer()
{
	return indexBuffer;
}

XMFLOAT4X4 Terrain::GetWorldMatrix()
{
	return world;
}

Terrain::~Terrain()
{
	delete[] heightmap;
	delete[] vertices;
}
