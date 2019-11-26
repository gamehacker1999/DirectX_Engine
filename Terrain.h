#pragma once

#include "Mesh.h"
#include"SimpleShader.h"
#include"Lights.h"

enum class TerrainBitDepth
{
	BitDepth_8,
	BitDepth_16
};

// Note: Mesh was changed to make all private data protected instead!
class Terrain
{
public:
	Terrain(
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
		SimplePixelShader* pixelShader);
	~Terrain();

	void SetPosition(XMFLOAT3 pos);

	XMFLOAT4X4 GetWorldMatrix();

	void Draw(XMFLOAT4X4 view, XMFLOAT4X4 projection, ID3D11DeviceContext* context, Light light);

private:

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	unsigned int numIndices;
	XMFLOAT4X4 worldMatrix;
	bool recalculateMatrix;
	XMFLOAT3 position;

	ID3D11ShaderResourceView* texture1	  ;
	ID3D11ShaderResourceView* texture2		  ;
	ID3D11ShaderResourceView* texture3		  ;
	ID3D11ShaderResourceView* blend	  ;
	ID3D11ShaderResourceView* normalTexture1  ;
	ID3D11ShaderResourceView* normalTexture2  ;
	ID3D11ShaderResourceView* normalTexture3  ;
	ID3D11SamplerState* sampleOptions;
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	void LoadHeightMap(std::string heightmap, unsigned int width, unsigned int height, float yScale, float xzScale,
		Vertex* verts, TerrainBitDepth bitDepth);

	void CreateBuffers(Vertex* vertices, int numVerts, unsigned int* indices, unsigned int numIndices, ID3D11Device* device);

	void CreateTangents(Vertex* vertices, int numVerts, unsigned int* indices, unsigned int numIndices);

	void Update();

};

