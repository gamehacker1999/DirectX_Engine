#include "Terrain.h"

#include <DirectXMath.h>
#include <vector>
#include <fstream>
#include "Terrain.h"

using namespace DirectX;

Terrain::Terrain(
	ID3D11Device* device,
	std::string heightmap,
	unsigned int heightmapWidth,
	unsigned int heightmapHeight,
	TerrainBitDepth bitDepth,
	float yScale ,
	float xzScale,
	float uvScale,
	ID3D11ShaderResourceView * texture1,
	ID3D11ShaderResourceView * texture2,
	ID3D11ShaderResourceView * texture3,
	ID3D11ShaderResourceView* blend,
	ID3D11ShaderResourceView* normalTexture1,
	ID3D11ShaderResourceView* normalTexture2,
	ID3D11ShaderResourceView* normalTexture3,
	ID3D11SamplerState* sampleOptions,
	SimpleVertexShader* vertexShader,
	SimplePixelShader* pixelShader)
{
	unsigned int numVertices = heightmapWidth * heightmapHeight;
	unsigned int numIndices = (heightmapWidth - 1) * (heightmapHeight - 1) * 6;

	this->texture1 = texture1;
	this->texture2 = texture2;
	this->texture3 = texture3;
	this->normalTexture1 = normalTexture1;
	this->normalTexture2 = normalTexture2;
	this->normalTexture3 = normalTexture3;
	this->blend = blend;
	this->sampleOptions = sampleOptions;
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;

	Vertex* verts = new Vertex[numVertices];

	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());

	LoadHeightMap(heightmap, heightmapWidth, heightmapHeight, yScale, xzScale, verts, bitDepth);

	// Create indices and, while we're at it, calculate the normal
	// of each triangle (as we'll need those for vertex normals)
	unsigned int* indices = new unsigned int[numIndices];
	std::vector<XMFLOAT3> triangleNormals;

	int indexCounter = 0;
	for (int z = 0; z < heightmapHeight - 1; z++)
	{
		for (int x = 0; x < heightmapWidth - 1; x++)
		{
			// Calc the vertex index
			int vertIndex = z * heightmapWidth + x;

			// Calculate the indices for these two triangles
			int i0 = vertIndex;
			int i1 = vertIndex + heightmapWidth;
			int i2 = vertIndex + 1 + heightmapWidth;

			int i3 = vertIndex;
			int i4 = vertIndex + 1 + heightmapWidth;
			int i5 = vertIndex + 1;

			// Put these in the index array
			indices[indexCounter++] = i0;
			indices[indexCounter++] = i1;
			indices[indexCounter++] = i2;

			indices[indexCounter++] = i3;
			indices[indexCounter++] = i4;
			indices[indexCounter++] = i5;

			// Get the positions of the three verts of each triangle
			XMVECTOR pos0 = XMLoadFloat3(&verts[i0].Position);
			XMVECTOR pos1 = XMLoadFloat3(&verts[i1].Position);
			XMVECTOR pos2 = XMLoadFloat3(&verts[i2].Position);

			XMVECTOR pos3 = XMLoadFloat3(&verts[i3].Position);
			XMVECTOR pos4 = XMLoadFloat3(&verts[i4].Position);
			XMVECTOR pos5 = XMLoadFloat3(&verts[i5].Position);

			// Calculate the normal of each triangle
			XMFLOAT3 normal0;
			XMFLOAT3 normal1;

			// Cross the edges of the triangle
			XMStoreFloat3(&normal0,
				XMVector3Normalize(XMVector3Cross(pos1 - pos0, pos2 - pos0)));

			XMStoreFloat3(&normal1,
				XMVector3Normalize(XMVector3Cross(pos4 - pos3, pos5 - pos3)));

			// Push the normals into the list
			triangleNormals.push_back(normal0);
			triangleNormals.push_back(normal1);
		}
	}


	// Calculate normals!
	for (int z = 0; z < heightmapHeight; z++)
	{
		for (int x = 0; x < heightmapWidth; x++)
		{
			// Get the index of this vertex, and triangle-related indices
			int index = z * heightmapWidth + x;
			int triIndex = index * 2 - (2 * z);
			int triIndexPrevRow = triIndex - (heightmapWidth * 2 - 1);

			// Running total of normals
			int normalCount = 0;
			XMVECTOR normalTotal = XMVectorSet(0, 0, 0, 0);

			// Normals
			//XMVECTOR upLeft = XMLoadFloat3(&triangleNormals[triIndexPrevRow - 1]);
			//XMVECTOR up = XMLoadFloat3(&triangleNormals[triIndexPrevRow]);
			//XMVECTOR upRight = XMLoadFloat3(&triangleNormals[triIndexPrevRow + 1]);
			//XMVECTOR downLeft = XMLoadFloat3(&triangleNormals[triIndex - 1]);
			//XMVECTOR down = XMLoadFloat3(&triangleNormals[triIndex]);
			//XMVECTOR downRight = XMLoadFloat3(&triangleNormals[triIndex + 1]);

			// x-----x-----x
			// |\    |\    |  
			// | \ u | \   |  
			// |  \  |  \  |  ul = up left
			// |   \ |   \ |  u  = up
			// | ul \| ur \|  ur = up right
			// x-----O-----x
			// |\ dl |\ dr |  dl = down left
			// | \   | \   |  d  = down
			// |  \  |  \  |  dr = down right
			// |   \ | d \ |
			// |    \|    \|
			// x-----x-----x

			// If not top row and not first column
			if (z > 0 && x > 0)
			{
				// "Up left" and "up"
				normalTotal += XMLoadFloat3(&triangleNormals[triIndexPrevRow - 1]);
				normalTotal += XMLoadFloat3(&triangleNormals[triIndexPrevRow]);

				normalCount += 2;
			}

			// If not top row and not last column
			if (z > 0 && x < heightmapWidth - 1)
			{
				// "Up right"
				normalTotal += XMLoadFloat3(&triangleNormals[triIndexPrevRow + 1]);

				normalCount++;
			}

			// If not bottom row and not first column
			if (z < heightmapHeight - 1 && x > 0)
			{
				// "Down left"
				normalTotal += XMLoadFloat3(&triangleNormals[triIndex - 1]);

				normalCount++;
			}

			// If not bottom row and not last column
			if (z < heightmapHeight - 1 && x < heightmapWidth - 1)
			{
				// "Down right" and "down"
				normalTotal += XMLoadFloat3(&triangleNormals[triIndex]);
				normalTotal += XMLoadFloat3(&triangleNormals[triIndex + 1]);

				normalCount += 2;
			}

			// Average normal
			normalTotal /= normalCount;
			XMStoreFloat3(&verts[index].normal, normalTotal);
		}
	}

	// Create the buffers and clean up arrays
	this->CreateBuffers(verts, numVertices, indices, numIndices, device);
	delete[] verts;
	delete[] indices;
}

Terrain::~Terrain()
{
	vertexBuffer->Release();
	indexBuffer->Release();
}

void Terrain::SetPosition(XMFLOAT3 pos)
{
	this->position = pos;
	recalculateMatrix = true;
}

XMFLOAT4X4 Terrain::GetWorldMatrix()
{
	//check if matrix has to be recalculated
//if yes then calculate it 
	if (recalculateMatrix)
	{
		//getting the translation, scale, and rotation matrices
		XMMATRIX translate = XMMatrixTranslationFromVector(XMLoadFloat3(&position));

		//calculating the model matrix from these three matrices and storing it
		//we transpose it before storing the matrix
		XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(translate));

		recalculateMatrix = false;
	}

	//returning the model matrix
	return worldMatrix;
}


void Terrain::LoadHeightMap(std::string heightmap, unsigned int width, unsigned int height, float yScale, float xzScale,
	Vertex* verts, TerrainBitDepth bitDepth)
{
	unsigned int numVertices = width * height;
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;

	int numVertsFactor = bitDepth == TerrainBitDepth::BitDepth_8 ? 1 : 2;
	float yFactor = bitDepth == TerrainBitDepth::BitDepth_8 ? 255.0f : 65535.0f;

	// Vector to hold heights
	std::vector<unsigned short> heights(numVertices);

	// Open the file (remember to #include <fstream>)
	std::ifstream file;
	file.open(heightmap.c_str(), std::ios_base::binary);
	if (!file)
		return;

	// Read raw bytes to vector
	file.read((char*)& heights[0], numVertices * numVertsFactor); // Double the size, since each pixel is 16-bit
	file.close();

	// Create the initial mesh data
	for (int z = 0; z < height; z++)
	{
		for (int x = 0; x < width; x++)
		{
			// This vert index
			int index = z * width + x;

			// Set up this vertex
			verts[index] = {};

			// Position on a regular grid, heights from heightmap
			verts[index].Position.x = (x - halfWidth) * xzScale;
			verts[index].Position.y = (heights[index] / yFactor) * yScale; // 16-bit, so max value is 65535
			verts[index].Position.z = (z - halfHeight) * xzScale;

			// Assume we're starting flat
			verts[index].normal.x = 0.0f;
			verts[index].normal.y = 1.0f;
			verts[index].normal.z = 0.0f;

			// Simple UV (0-1)
			verts[index].uv.x = x / (float)width;
			verts[index].uv.y = z / (float)height;
		}
	}
}

void Terrain::CreateBuffers(Vertex* vertices, int numVerts, unsigned int* indices, unsigned int numIndices, ID3D11Device* device)
{
	CreateTangents(vertices, numVerts, indices, numIndices);

	//setting up the vertex buffer description
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = numVerts * sizeof(Vertex);       // number of vertices in the buffer
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells DirectX this is a vertex buffer
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	//holding the initial vertex data
	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = reinterpret_cast<void*>(vertices); //casting the vertex data to a void pointer

	//creating the vertex buffer data
	device->CreateBuffer(&vbd, &initialVertexData, &vertexBuffer);

	// Create the INDEX BUFFER description ------------------------------------
// - The description is created on the stack because we only need
//    it to create the buffer.  The description is then useless.
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = numIndices * sizeof(unsigned int);         // 3 = number of indices in the buffer
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER; // Tells DirectX this is an index buffer
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	this->numIndices = numIndices;

	// Create the proper struct to hold the initial index data
	// - This is how we put the initial data into the buffer
	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = reinterpret_cast<void*>(indices);

	// Actually create the buffer with the initial data
	// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
	device->CreateBuffer(&ibd, &initialIndexData, &indexBuffer);
}

void Terrain::CreateTangents(Vertex* vertices, int numVerts, unsigned int* indices, unsigned int numIndices)
{
	//compute the tangents and bitangents for each triangle
	for (size_t i = 0; i < numVerts; i += 3)
	{
		//getting the position, normal, and uv data for vertex
		XMFLOAT3 vert1 = vertices[i].Position;
		XMFLOAT3 vert2 = vertices[i + 1].Position;
		XMFLOAT3 vert3 = vertices[i + 2].Position;

		XMFLOAT2 uv1 = vertices[i].uv;
		XMFLOAT2 uv2 = vertices[i + 1].uv;
		XMFLOAT2 uv3 = vertices[i + 2].uv;

		//finding the two edges of the triangles
		auto tempEdge = XMLoadFloat3(&vert2) - XMLoadFloat3(&vert1);
		XMFLOAT3 edge1;
		XMStoreFloat3(&edge1, tempEdge);
		tempEdge = XMLoadFloat3(&vert3) - XMLoadFloat3(&vert1);
		XMFLOAT3 edge2;
		XMStoreFloat3(&edge2, tempEdge);

		//finding the difference in UVs
		XMFLOAT2 deltaUV1;
		XMStoreFloat2(&deltaUV1, XMLoadFloat2(&uv2) - XMLoadFloat2(&uv1));
		XMFLOAT2 deltaUV2;
		XMStoreFloat2(&deltaUV2, XMLoadFloat2(&uv3) - XMLoadFloat2(&uv1));

		//calculate the inverse of the delta uv matrix
		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		//calculating the tangent of the triangle
		XMFLOAT3 tangent;
		XMStoreFloat3(&tangent, (XMLoadFloat3(&edge1) * deltaUV2.y - XMLoadFloat3(&edge2) * deltaUV1.y) * r);

		vertices[i].tangent = tangent;
		vertices[i + 1].tangent = tangent;
		vertices[i + 2].tangent = tangent;

	}
}

void Terrain::Draw(XMFLOAT4X4 view, XMFLOAT4X4 projection, ID3D11DeviceContext* context,Light light)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	vertexShader->SetMatrix4x4("model", GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", view);
	vertexShader->SetMatrix4x4("projection", projection);
	vertexShader->SetFloat4("clipDistance", XMFLOAT4(0, 0, 0, 0));

	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();

	pixelShader->SetData("dirLight", &light, sizeof(light));

	pixelShader->SetFloat("uvScale0", 50.0f);
	pixelShader->SetFloat("uvScale1", 50.0f);
	pixelShader->SetFloat("uvScale2", 50.0f);

	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();
	pixelShader->SetShaderResourceView("blendMap", blend);
	pixelShader->SetShaderResourceView("texture0", texture1);
	pixelShader->SetShaderResourceView("texture1", texture2);
	pixelShader->SetShaderResourceView("texture2", texture3);
	pixelShader->SetShaderResourceView("normalMap0", normalTexture1);
	pixelShader->SetShaderResourceView("normalMap1", normalTexture2);
	pixelShader->SetShaderResourceView("normalMap2", normalTexture3);
	pixelShader->SetSamplerState("samplerOptions", sampleOptions);


	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(numIndices, 0, 0);
}


