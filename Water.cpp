#include "Water.h"

Water::Water(std::shared_ptr<Mesh> waterMesh, ID3D11ShaderResourceView* waterTex,
	ID3D11ShaderResourceView* waterNormal1, ID3D11ShaderResourceView* waterNormal2,
	SimplePixelShader* waterPS, SimpleVertexShader* waterVS,
	SimpleComputeShader* h0CS, SimpleComputeShader* htCS, SimpleComputeShader* twiddleFactorsCS,
	SimpleComputeShader* butterflyCS, SimpleComputeShader* inversionCS, SimpleComputeShader* sobelFilter,
	SimpleComputeShader* jacobianCS, ID3D11SamplerState* samplerState,
	ID3D11Device* device, ID3D11ShaderResourceView* noiseR1, ID3D11ShaderResourceView* noiseI1,
	ID3D11ShaderResourceView* noiseR2, ID3D11ShaderResourceView* noiseI2)
{
	this->waterMesh = waterMesh;
	this->waterTex = waterTex;
	this->waterNormal1 = waterNormal1;
	this->waterNormal2 = waterNormal2;
	this->waterPS = waterPS;
	this->waterVS = waterVS;
	this->samplerState = samplerState;
	this->noiseR1 = noiseR1;
	this->noiseI1 = noiseI1;
	this->noiseR2 = noiseR2;
	this->noiseI2 = noiseI2;
	this->h0CS = h0CS;
	this->htCS = htCS;
	this->twiddleFactorsCS = twiddleFactorsCS;
	this->butterflyCS = butterflyCS;
	this->inversionCS = inversionCS;
	this->sobelFilter = sobelFilter;
	this->jacobianCS = jacobianCS;

	texSize = 256;
	int bits = (int)(log(256) / log(2));

	//initializing the UAV

	//h0 texture
	ID3D11Texture2D* h0Tex2D;

	D3D11_TEXTURE2D_DESC h0TexDesc = {};
	h0TexDesc.Width = texSize;
	h0TexDesc.Height = texSize;
	h0TexDesc.ArraySize = 1;
	h0TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	h0TexDesc.CPUAccessFlags = 0;
	h0TexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	h0TexDesc.MipLevels = 1;
	h0TexDesc.MiscFlags = 0;
	h0TexDesc.SampleDesc.Count = 1;
	h0TexDesc.SampleDesc.Quality = 0;
	h0TexDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&h0TexDesc, 0, &h0Tex2D);

	D3D11_SHADER_RESOURCE_VIEW_DESC h0SrvDesc = {};
	h0SrvDesc.Format = h0TexDesc.Format;
	h0SrvDesc.Texture2D.MipLevels = 1;
	h0SrvDesc.Texture2D.MostDetailedMip = 0;
	h0SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(h0Tex2D, &h0SrvDesc, &h0SRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC h0UavDesc = {};
	h0UavDesc.Format = h0TexDesc.Format;
	h0UavDesc.Texture2D.MipSlice = 0;
	h0UavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(h0Tex2D, &h0UavDesc, &h0URV);

	h0Tex2D->Release();


	//h0 minus texture
	//initializing the UAV
	ID3D11Texture2D* h0MinusTex2D;

	D3D11_TEXTURE2D_DESC h0MinusTexDesc = {};
	h0MinusTexDesc.Width = texSize;
	h0MinusTexDesc.Height = texSize;
	h0MinusTexDesc.ArraySize = 1;
	h0MinusTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	h0MinusTexDesc.CPUAccessFlags = 0;
	h0MinusTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	h0MinusTexDesc.MipLevels = 1;
	h0MinusTexDesc.MiscFlags = 0;
	h0MinusTexDesc.SampleDesc.Count = 1;
	h0MinusTexDesc.SampleDesc.Quality = 0;
	h0MinusTexDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&h0MinusTexDesc, 0, &h0MinusTex2D);

	D3D11_SHADER_RESOURCE_VIEW_DESC h0MinusSrvDesc = {};
	h0MinusSrvDesc.Format = h0TexDesc.Format;
	h0MinusSrvDesc.Texture2D.MipLevels = 1;
	h0MinusSrvDesc.Texture2D.MostDetailedMip = 0;
	h0MinusSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(h0MinusTex2D, &h0MinusSrvDesc, &h0MinusSRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC h0MinusUavDesc = {};
	h0MinusUavDesc.Format = h0TexDesc.Format;
	h0MinusUavDesc.Texture2D.MipSlice = 0;
	h0MinusUavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(h0MinusTex2D, &h0MinusUavDesc, &h0MinusURV);

	h0MinusTex2D->Release();



	//hkx minus texture
	//initializing the UAV
	ID3D11Texture2D* hkxTex2D;

	D3D11_TEXTURE2D_DESC hkxTex2DDesc = {};
	hkxTex2DDesc.Width = texSize;
	hkxTex2DDesc.Height = texSize;
	hkxTex2DDesc.ArraySize = 1;
	hkxTex2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	hkxTex2DDesc.CPUAccessFlags = 0;
	hkxTex2DDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	hkxTex2DDesc.MipLevels = 1;
	hkxTex2DDesc.MiscFlags = 0;
	hkxTex2DDesc.SampleDesc.Count = 1;
	hkxTex2DDesc.SampleDesc.Quality = 0;
	hkxTex2DDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&hkxTex2DDesc, 0, &hkxTex2D);

	D3D11_SHADER_RESOURCE_VIEW_DESC hkxTexSRVDesc = {};
	hkxTexSRVDesc.Format = hkxTex2DDesc.Format;
	hkxTexSRVDesc.Texture2D.MipLevels = 1;
	hkxTexSRVDesc.Texture2D.MostDetailedMip = 0;
	hkxTexSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(hkxTex2D, &hkxTexSRVDesc, &htxSRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC hkxTexUAVDesc = {};
	hkxTexUAVDesc.Format = hkxTex2DDesc.Format;
	hkxTexUAVDesc.Texture2D.MipSlice = 0;
	hkxTexUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(hkxTex2D, &hkxTexUAVDesc, &htxUAV);

	hkxTex2D->Release();

	//hky minus texture
	//initializing the UAV
	ID3D11Texture2D* hkyTex2D;

	D3D11_TEXTURE2D_DESC hkyTex2DDesc = {};
	hkyTex2DDesc.Width = texSize;
	hkyTex2DDesc.Height = texSize;
	hkyTex2DDesc.ArraySize = 1;
	hkyTex2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	hkyTex2DDesc.CPUAccessFlags = 0;
	hkyTex2DDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	hkyTex2DDesc.MipLevels = 1;
	hkyTex2DDesc.MiscFlags = 0;
	hkyTex2DDesc.SampleDesc.Count = 1;
	hkyTex2DDesc.SampleDesc.Quality = 0;
	hkyTex2DDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&hkyTex2DDesc, 0, &hkyTex2D);

	D3D11_SHADER_RESOURCE_VIEW_DESC hkyTexSRVDesc = {};
	hkyTexSRVDesc.Format = hkyTex2DDesc.Format;
	hkyTexSRVDesc.Texture2D.MipLevels = 1;
	hkyTexSRVDesc.Texture2D.MostDetailedMip = 0;
	hkyTexSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(hkyTex2D, &hkyTexSRVDesc, &htySRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC hkyTexUAVDesc = {};
	hkyTexUAVDesc.Format = hkyTex2DDesc.Format;
	hkyTexUAVDesc.Texture2D.MipSlice = 0;
	hkyTexUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(hkyTex2D, &hkyTexUAVDesc, &htyUAV);

	hkyTex2D->Release();

	//hkx minus texture
	//initializing the UAV
	ID3D11Texture2D* hkzTex2D;

	D3D11_TEXTURE2D_DESC hkzTex2DDesc = {};
	hkzTex2DDesc.Width = texSize;
	hkzTex2DDesc.Height = texSize;
	hkzTex2DDesc.ArraySize = 1;
	hkzTex2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	hkzTex2DDesc.CPUAccessFlags = 0;
	hkzTex2DDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	hkzTex2DDesc.MipLevels = 1;
	hkzTex2DDesc.MiscFlags = 0;
	hkzTex2DDesc.SampleDesc.Count = 1;
	hkzTex2DDesc.SampleDesc.Quality = 0;
	hkzTex2DDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&hkzTex2DDesc, 0, &hkzTex2D);

	D3D11_SHADER_RESOURCE_VIEW_DESC hkzTexSRVDesc = {};
	hkzTexSRVDesc.Format = hkzTex2DDesc.Format;
	hkzTexSRVDesc.Texture2D.MipLevels = 1;
	hkzTexSRVDesc.Texture2D.MostDetailedMip = 0;
	hkzTexSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(hkzTex2D, &hkzTexSRVDesc, &htzSRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC hkzTexUAVDesc = {};
	hkzTexUAVDesc.Format = hkzTex2DDesc.Format;
	hkzTexUAVDesc.Texture2D.MipSlice = 0;
	hkzTexUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(hkzTex2D, &hkzTexUAVDesc, &htzUAV);

	hkzTex2D->Release();


	//twiddle indices texture
	ID3D11Texture2D* twiddleIndicesTex;

	D3D11_TEXTURE2D_DESC twiddleIndicesTexDesc = {};
	twiddleIndicesTexDesc.Width = bits;
	twiddleIndicesTexDesc.Height = texSize;
	twiddleIndicesTexDesc.ArraySize = 1;
	twiddleIndicesTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	twiddleIndicesTexDesc.CPUAccessFlags = 0;
	twiddleIndicesTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	twiddleIndicesTexDesc.MipLevels = 1;
	twiddleIndicesTexDesc.MiscFlags = 0;
	twiddleIndicesTexDesc.SampleDesc.Count = 1;
	twiddleIndicesTexDesc.SampleDesc.Quality = 0;
	twiddleIndicesTexDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&twiddleIndicesTexDesc, 0, &twiddleIndicesTex);

	D3D11_SHADER_RESOURCE_VIEW_DESC twiddleIndicesSRVDesc = {};
	twiddleIndicesSRVDesc.Format = twiddleIndicesTexDesc.Format;
	twiddleIndicesSRVDesc.Texture2D.MipLevels = 1;
	twiddleIndicesSRVDesc.Texture2D.MostDetailedMip = 0;
	twiddleIndicesSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(twiddleIndicesTex, &twiddleIndicesSRVDesc, &twiddleSRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC twiddleIndicesUAVDesc = {};
	twiddleIndicesUAVDesc.Format = twiddleIndicesTexDesc.Format;
	twiddleIndicesUAVDesc.Texture2D.MipSlice = 0;
	twiddleIndicesUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(twiddleIndicesTex, &twiddleIndicesUAVDesc, &twiddleUAV);

	twiddleIndicesTex->Release();

	//pingpong0
	ID3D11Texture2D* pingpong0Tex;

	D3D11_TEXTURE2D_DESC pingpong0TexDesc = {};
	pingpong0TexDesc.Width = texSize;
	pingpong0TexDesc.Height = texSize;
	pingpong0TexDesc.ArraySize = 1;
	pingpong0TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	pingpong0TexDesc.CPUAccessFlags = 0;
	pingpong0TexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	pingpong0TexDesc.MipLevels = 1;
	pingpong0TexDesc.MiscFlags = 0;
	pingpong0TexDesc.SampleDesc.Count = 1;
	pingpong0TexDesc.SampleDesc.Quality = 0;
	pingpong0TexDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&pingpong0TexDesc, 0, &pingpong0Tex);

	D3D11_SHADER_RESOURCE_VIEW_DESC pingpong0SRVDesc = {};
	pingpong0SRVDesc.Format = pingpong0TexDesc.Format;
	pingpong0SRVDesc.Texture2D.MipLevels = 1;
	pingpong0SRVDesc.Texture2D.MostDetailedMip = 0;
	pingpong0SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(pingpong0Tex, &pingpong0SRVDesc, &pingpong0SRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC pingpong0UAVDesc = {};
	pingpong0UAVDesc.Format = pingpong0TexDesc.Format;
	pingpong0UAVDesc.Texture2D.MipSlice = 0;
	pingpong0UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(pingpong0Tex, &pingpong0UAVDesc, &pingpong0UAV);

	pingpong0Tex->Release();

	//pingpong1
	ID3D11Texture2D* pingpong1Tex;

	D3D11_TEXTURE2D_DESC pingpong1TexDesc = {};
	pingpong1TexDesc.Width = texSize;
	pingpong1TexDesc.Height = texSize;
	pingpong1TexDesc.ArraySize = 1;
	pingpong1TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	pingpong1TexDesc.CPUAccessFlags = 0;
	pingpong1TexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	pingpong1TexDesc.MipLevels = 1;
	pingpong1TexDesc.MiscFlags = 0;
	pingpong1TexDesc.SampleDesc.Count = 1;
	pingpong1TexDesc.SampleDesc.Quality = 0;
	pingpong1TexDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&pingpong1TexDesc, 0, &pingpong1Tex);

	D3D11_SHADER_RESOURCE_VIEW_DESC pingpong1SRVDesc = {};
	pingpong1SRVDesc.Format = pingpong1TexDesc.Format;
	pingpong1SRVDesc.Texture2D.MipLevels = 1;
	pingpong1SRVDesc.Texture2D.MostDetailedMip = 0;
	pingpong1SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(pingpong1Tex, &pingpong1SRVDesc, &dySRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC pingpong1UAVDesc = {};
	pingpong1UAVDesc.Format = pingpong1TexDesc.Format;
	pingpong1UAVDesc.Texture2D.MipSlice = 0;
	pingpong1UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(pingpong1Tex, &pingpong1UAVDesc, &dyUAV);

	pingpong1Tex->Release();

	//dx
	ID3D11Texture2D* dxTex;

	D3D11_TEXTURE2D_DESC dxTexDesc = {};
	dxTexDesc.Width = texSize;
	dxTexDesc.Height = texSize;
	dxTexDesc.ArraySize = 1;
	dxTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	dxTexDesc.CPUAccessFlags = 0;
	dxTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	dxTexDesc.MipLevels = 1;
	dxTexDesc.MiscFlags = 0;
	dxTexDesc.SampleDesc.Count = 1;
	dxTexDesc.SampleDesc.Quality = 0;
	dxTexDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&dxTexDesc, 0, &dxTex);

	D3D11_SHADER_RESOURCE_VIEW_DESC dxSRVDesc = {};
	dxSRVDesc.Format = dxTexDesc.Format;
	dxSRVDesc.Texture2D.MipLevels = 1;
	dxSRVDesc.Texture2D.MostDetailedMip = 0;
	dxSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(dxTex, &dxSRVDesc, &dxSRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC dxUAVDesc = {};
	dxUAVDesc.Format = dxTexDesc.Format;
	dxUAVDesc.Texture2D.MipSlice = 0;
	dxUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(dxTex, &dxUAVDesc, &dxUAV);

	dxTex->Release();

	//dz
	ID3D11Texture2D* dzTex;

	D3D11_TEXTURE2D_DESC dzTexDesc = {};
	dzTexDesc.Width = texSize;
	dzTexDesc.Height = texSize;
	dzTexDesc.ArraySize = 1;
	dzTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	dzTexDesc.CPUAccessFlags = 0;
	dzTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	dzTexDesc.MipLevels = 1;
	dzTexDesc.MiscFlags = 0;
	dzTexDesc.SampleDesc.Count = 1;
	dzTexDesc.SampleDesc.Quality = 0;
	dzTexDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&dzTexDesc, 0, &dzTex);

	D3D11_SHADER_RESOURCE_VIEW_DESC dzSRVDesc = {};
	dzSRVDesc.Format = dzTexDesc.Format;
	dzSRVDesc.Texture2D.MipLevels = 1;
	dzSRVDesc.Texture2D.MostDetailedMip = 0;
	dzSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(dzTex, &dzSRVDesc, &dzSRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC dzUAVDesc = {};
	dzUAVDesc.Format = dzTexDesc.Format;
	dzUAVDesc.Texture2D.MipSlice = 0;
	dzUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(dzTex, &dzUAVDesc, &dzUAV);

	dzTex->Release();
	

	//normal map
	ID3D11Texture2D* normalMapTex;

	D3D11_TEXTURE2D_DESC normalMapTexDesc = {};
	normalMapTexDesc.Width = texSize;
	normalMapTexDesc.Height = texSize;
	normalMapTexDesc.ArraySize = 1;
	normalMapTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	normalMapTexDesc.CPUAccessFlags = 0;
	normalMapTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	normalMapTexDesc.MipLevels = 1;
	normalMapTexDesc.MiscFlags = 0;
	normalMapTexDesc.SampleDesc.Count = 1;
	normalMapTexDesc.SampleDesc.Quality = 0;
	normalMapTexDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&normalMapTexDesc, 0, &normalMapTex);

	D3D11_SHADER_RESOURCE_VIEW_DESC normalMapSRVDesc = {};
	normalMapSRVDesc.Format = normalMapTexDesc.Format;
	normalMapSRVDesc.Texture2D.MipLevels = 1;
	normalMapSRVDesc.Texture2D.MostDetailedMip = 0;
	normalMapSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(normalMapTex, &normalMapSRVDesc, &normalMapSRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC normalMapUAVDesc = {};
	normalMapUAVDesc.Format = normalMapTexDesc.Format;
	normalMapUAVDesc.Texture2D.MipSlice = 0;
	normalMapUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(normalMapTex, &normalMapUAVDesc, &normalMapUAV);

	normalMapTex->Release();

	//folding map
	ID3D11Texture2D* foldingMapTex;

	D3D11_TEXTURE2D_DESC foldingMapTexDesc = {};
	foldingMapTexDesc.Width = texSize;
	foldingMapTexDesc.Height = texSize;
	foldingMapTexDesc.ArraySize = 1;
	foldingMapTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	foldingMapTexDesc.CPUAccessFlags = 0;
	foldingMapTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	foldingMapTexDesc.MipLevels = 1;
	foldingMapTexDesc.MiscFlags = 0;
	foldingMapTexDesc.SampleDesc.Count = 1;
	foldingMapTexDesc.SampleDesc.Quality = 0;
	foldingMapTexDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&foldingMapTexDesc, 0, &foldingMapTex);

	D3D11_SHADER_RESOURCE_VIEW_DESC foldingMapSRVDesc = {};
	foldingMapSRVDesc.Format = foldingMapTexDesc.Format;
	foldingMapSRVDesc.Texture2D.MipLevels = 1;
	foldingMapSRVDesc.Texture2D.MostDetailedMip = 0;
	foldingMapSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(foldingMapTex, &foldingMapSRVDesc, &foldingMapSRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC foldingMapUAVDesc = {};
	foldingMapUAVDesc.Format = normalMapTexDesc.Format;
	foldingMapUAVDesc.Texture2D.MipSlice = 0;
	foldingMapUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(foldingMapTex, &foldingMapUAVDesc, &foldingMapUAV);

	foldingMapTex->Release();
}

Water::~Water()
{
	h0MinusURV->Release();
	h0URV->Release();
	h0SRV->Release();
	h0MinusSRV->Release();

	htxSRV->Release();
	htxUAV->Release();

	htySRV->Release();
	htyUAV->Release();

	htzSRV->Release();
	htzUAV->Release();

	twiddleSRV->Release();
	twiddleUAV->Release();

	pingpong0SRV->Release();
	pingpong0UAV->Release();

	dySRV->Release();
	dyUAV->Release();

	dxSRV->Release();
	dxUAV->Release();

	dzSRV->Release();
	dzUAV->Release();

	normalMapSRV->Release();
	normalMapUAV->Release();
}

void Water::Update(float deltaTime,XMFLOAT3 shipPos)
{
	//setting the world matrix for water
	XMFLOAT3 curPos = XMFLOAT3(50,0,0);
	curPos.y = -2;
	//curPos.z += 60;
	//curPos.x = 0;
	XMFLOAT3 scale = XMFLOAT3(2.f, 2.f, 2.f);
	XMMATRIX matTrans = XMMatrixTranslationFromVector(XMLoadFloat3(&curPos));
	XMMATRIX matScale = XMMatrixScalingFromVector(XMLoadFloat3(&scale));
	XMMATRIX rot = XMMatrixRotationQuaternion(XMQuaternionIdentity());

	XMStoreFloat4x4(&worldMat, XMMatrixTranspose(matScale* rot* matTrans));
}

void Water::Draw(Light lights, ID3D11ShaderResourceView* cubeMap, std::shared_ptr<Camera> camera,
	ID3D11DeviceContext* context, float deltaTime, float totalTime, ID3D11SamplerState* waterSampler)
{

	RenderFFT(totalTime*1.4);

	//static float totalTime = 0;
	//totalTime += deltaTime;
	
	waterVS->SetShaderResourceView("heightMap", dySRV);
	waterVS->SetShaderResourceView("heightMapX", dxSRV);
	waterVS->SetShaderResourceView("heightMapZ", dzSRV);
	waterVS->SetSamplerState("sampleOptions", samplerState);
	waterVS->SetMatrix4x4("world", worldMat);
	waterVS->SetMatrix4x4("view", camera->GetViewMatrix());
	waterVS->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	waterVS->SetFloat4("waveA", XMFLOAT4(1.0f, 1.0f, 0.3f, 2.0f));
	waterVS->SetFloat4("waveB", XMFLOAT4(0, 1, 0.3f, 2.0f));
	waterVS->SetFloat4("waveC", XMFLOAT4(-1, 1, 0.3f, 2.0f));
	waterVS->SetFloat4("waveD", XMFLOAT4(1, 0, 0.3f, 2.0f));
	waterVS->SetFloat("speed", 0.60f);
	waterVS->SetFloat2("windDir", XMFLOAT2(1, 1));
	waterVS->SetFloat("windSpeed", 40);
	waterVS->SetFloat("dt", totalTime);
	waterVS->CopyAllBufferData();
	waterVS->SetShader();

	static float scrollX = 0;
	static float scrollY = 0;

	scrollX += 0.07f*deltaTime;
	scrollY += 0.07f*deltaTime;

	waterPS->SetFloat("scrollX", scrollX);
	waterPS->SetFloat("scrollY", scrollY);
	waterPS->SetData("dirLight", &lights, sizeof(Light));
	waterPS->SetFloat3("cameraPosition", camera->GetPosition());
	waterPS->SetMatrix4x4("view", camera->GetViewMatrix());

	waterPS->SetShaderResourceView("waterTexture", waterTex);
	waterPS->SetShaderResourceView("normalTexture1", waterNormal1);
	waterPS->SetShaderResourceView("normalTexture2", waterNormal2);
	waterPS->SetShaderResourceView("normalTexture3", normalMapSRV);
	waterPS->SetShaderResourceView("foldingMap", foldingMapSRV);
	waterPS->SetSamplerState("sampleOptions", samplerState);
	waterPS->SetSamplerState("waterSampleOptions", waterSampler);


	waterPS->CopyAllBufferData();
	waterPS->SetShader();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	auto tempVertBuffer = waterMesh->GetVertexBuffer();
	context->IASetVertexBuffers(0, 1, &tempVertBuffer, &stride, &offset);
	context->IASetIndexBuffer(waterMesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(waterMesh->GetIndexCount(), 0, 0);
}

void Water::CreateH0Texture()
{
	h0CS->SetInt("fftRes", 256);
	h0CS->SetInt("L", 1000);
	h0CS->SetFloat("amp", 4);
	h0CS->SetFloat2("windDir", XMFLOAT2(1, 1));
	h0CS->SetFloat("windSpeed", 40.0f);

	h0CS->SetShaderResourceView("noiseR1", noiseR1);
	h0CS->SetShaderResourceView("noiseI1", noiseI1);
	h0CS->SetShaderResourceView("noiseR2", noiseR2);
	h0CS->SetShaderResourceView("noiseI2", noiseI2);
	h0CS->SetSamplerState("sampleOptions", samplerState);

	h0CS->SetUnorderedAccessView("tildeH0", h0URV);
	h0CS->SetUnorderedAccessView("tildeMinusH0", h0MinusURV);

	h0CS->CopyAllBufferData();
	h0CS->SetShader();
	h0CS->DispatchByThreads(256, 256, 1);

	h0CS->SetUnorderedAccessView("tildeH0", 0);
	h0CS->SetUnorderedAccessView("tildeMinusH0", 0);
}

void Water::CreateHtTexture(float totalTime)
{
	htCS->SetInt("fftRes", 256);
	htCS->SetInt("L", 1000);
	htCS->SetFloat("time", totalTime);

	htCS->SetShader();
	
	htCS->SetUnorderedAccessView("tildeH0", h0URV);
	htCS->SetUnorderedAccessView("tildeMinusH0", h0MinusURV);
	htCS->SetUnorderedAccessView("tildeHktDx", htxUAV);
	htCS->SetUnorderedAccessView("tildeHktDy", htyUAV);
	htCS->SetUnorderedAccessView("tildeHktDz", htzUAV);

	htCS->CopyAllBufferData();
	htCS->DispatchByThreads(256, 256, 1);

	htCS->SetUnorderedAccessView("tildeH0", 0);
	htCS->SetUnorderedAccessView("tildeMinusH0", 0);
	htCS->SetUnorderedAccessView("tildeHktDx", 0);
	htCS->SetUnorderedAccessView("tildeHktDy", 0);
	htCS->SetUnorderedAccessView("tildeHktDz", 0);
}

int Water::CreateBitReversedIndices(int num, int d)
{
	unsigned int count = sizeof(num) * 8 - 1;
	unsigned int reverseNum = num;

	num >>= 1;
	while (num)
	{
		reverseNum <<= 1;
		reverseNum |= num & 1;
		num >>= 1;
		count--;
	}
	//this is our bit reversed number
	reverseNum <<= count;

	//rotating this number left
	return (reverseNum << d) | (reverseNum >> (CHAR_BIT * sizeof(reverseNum) - d));
}

void Water::CreateTwiddleIndices()
{
	BitIndices bitreversedIndices[256];

	int bits = (int)(log(256) / log(2));

	for (int i = 0; i < 256; i++)
	{
		bitreversedIndices[i].index = CreateBitReversedIndices(i, bits);
	}

	twiddleFactorsCS->SetInt("fftRes", 256);
	twiddleFactorsCS->SetData("indices", &bitreversedIndices[0], sizeof(BitIndices) * 256);
	twiddleFactorsCS->SetUnorderedAccessView("twiddleIndices", twiddleUAV);

	twiddleFactorsCS->CopyAllBufferData();
	twiddleFactorsCS->SetShader();
	twiddleFactorsCS->DispatchByThreads(bits, 256, 1);

	twiddleFactorsCS->SetUnorderedAccessView("twiddleIndices", 0);
}

void Water::RenderFFT(float totalTime)
{
	CreateHtTexture(totalTime+500);

	butterflyCS->SetUnorderedAccessView("twiddleIndices", twiddleUAV);
	butterflyCS->SetUnorderedAccessView("pingpong0", htyUAV);
	butterflyCS->SetUnorderedAccessView("pingpong1", pingpong0UAV);

	int log2N = (int)(log(256) / log(2));

	int pingpong = 0;

	butterflyCS->SetShader();
	
	//horizontal fft
	for (int i = 0; i < log2N; i++)
	{
	
		butterflyCS->SetInt("pingpong", pingpong);
		butterflyCS->SetInt("direction", 0);
		butterflyCS->SetInt("stage", i);
		butterflyCS->CopyAllBufferData();
		butterflyCS->DispatchByThreads(256, 256, 1);
		
	
		pingpong++;
		pingpong %= 2;
	
	}
	
	//vertical fft
	for (int i = 0; i < log2N; i++)
	{
		butterflyCS->SetInt("pingpong", pingpong);
		butterflyCS->SetInt("direction", 1);
		butterflyCS->SetInt("stage", i);
		butterflyCS->CopyAllBufferData();
		butterflyCS->DispatchByThreads(256, 256, 1);
	
		pingpong++;
		pingpong %= 2;
	
	}
	
	butterflyCS->SetUnorderedAccessView("twiddleIndices", 0);
	butterflyCS->SetUnorderedAccessView("pingpong0", 0);
	butterflyCS->SetUnorderedAccessView("pingpong1", 0);
	
	inversionCS->SetShader();
	inversionCS->SetInt("N", 256);
	inversionCS->SetInt("pingpong", pingpong);
	inversionCS->SetUnorderedAccessView("displacement", dyUAV);
	inversionCS->SetUnorderedAccessView("pingpong0", htyUAV);
	inversionCS->SetUnorderedAccessView("pingpong1", pingpong0UAV);
	inversionCS->CopyAllBufferData();
	inversionCS->DispatchByThreads(256, 256, 1);
	inversionCS->SetUnorderedAccessView("displacement", 0);
	inversionCS->SetUnorderedAccessView("pingpong0", 0);
	inversionCS->SetUnorderedAccessView("pingpong1", 0);

	if (true)
	{
		//dx fft
		pingpong = 0;

		butterflyCS->SetUnorderedAccessView("twiddleIndices", twiddleUAV);
		butterflyCS->SetUnorderedAccessView("pingpong0", htxUAV);
		butterflyCS->SetUnorderedAccessView("pingpong1", pingpong0UAV);

		int pingpong = 0;

		butterflyCS->SetShader();

		//horizontal fft
		for (int i = 0; i < log2N; i++)
		{

			butterflyCS->SetInt("pingpong", pingpong);
			butterflyCS->SetInt("direction", 0);
			butterflyCS->SetInt("stage", i);
			butterflyCS->CopyAllBufferData();
			butterflyCS->DispatchByThreads(256, 256, 1);


			pingpong++;
			pingpong %= 2;

		}

		//vertical fft
		for (int i = 0; i < log2N; i++)
		{
			butterflyCS->SetInt("pingpong", pingpong);
			butterflyCS->SetInt("direction", 1);
			butterflyCS->SetInt("stage", i);
			butterflyCS->CopyAllBufferData();
			butterflyCS->DispatchByThreads(256, 256, 1);

			pingpong++;
			pingpong %= 2;

		}

		butterflyCS->SetUnorderedAccessView("twiddleIndices", 0);
		butterflyCS->SetUnorderedAccessView("pingpong0", 0);
		butterflyCS->SetUnorderedAccessView("pingpong1", 0);

		inversionCS->SetShader();
		inversionCS->SetInt("N", 256);
		inversionCS->SetInt("pingpong", pingpong);
		inversionCS->SetUnorderedAccessView("displacement", dxUAV);
		inversionCS->SetUnorderedAccessView("pingpong0", htxUAV);
		inversionCS->SetUnorderedAccessView("pingpong1", pingpong0UAV);
		inversionCS->CopyAllBufferData();
		inversionCS->DispatchByThreads(256, 256, 1);
		inversionCS->SetUnorderedAccessView("displacement", 0);
		inversionCS->SetUnorderedAccessView("pingpong0", 0);
		inversionCS->SetUnorderedAccessView("pingpong1", 0);

	}

	if (true)
	{
		//dz fft
		pingpong = 0;

		butterflyCS->SetUnorderedAccessView("twiddleIndices", twiddleUAV);
		butterflyCS->SetUnorderedAccessView("pingpong0", htzUAV);
		butterflyCS->SetUnorderedAccessView("pingpong1", pingpong0UAV);

		int pingpong = 0;

		butterflyCS->SetShader();

		//horizontal fft
		for (int i = 0; i < log2N; i++)
		{

			butterflyCS->SetInt("pingpong", pingpong);
			butterflyCS->SetInt("direction", 0);
			butterflyCS->SetInt("stage", i);
			butterflyCS->CopyAllBufferData();
			butterflyCS->DispatchByThreads(256, 256, 1);


			pingpong++;
			pingpong %= 2;

		}

		//vertical fft
		for (int i = 0; i < log2N; i++)
		{
			butterflyCS->SetInt("pingpong", pingpong);
			butterflyCS->SetInt("direction", 1);
			butterflyCS->SetInt("stage", i);
			butterflyCS->CopyAllBufferData();
			butterflyCS->DispatchByThreads(256, 256, 1);

			pingpong++;
			pingpong %= 2;

		}

		butterflyCS->SetUnorderedAccessView("twiddleIndices", 0);
		butterflyCS->SetUnorderedAccessView("pingpong0", 0);
		butterflyCS->SetUnorderedAccessView("pingpong1", 0);

		inversionCS->SetShader();
		inversionCS->SetInt("N", 256);
		inversionCS->SetInt("pingpong", pingpong);
		inversionCS->SetUnorderedAccessView("displacement", dzUAV);
		inversionCS->SetUnorderedAccessView("pingpong0", htzUAV);
		inversionCS->SetUnorderedAccessView("pingpong1", pingpong0UAV);
		inversionCS->CopyAllBufferData();
		inversionCS->DispatchByThreads(256, 256, 1);
		inversionCS->SetUnorderedAccessView("displacement", 0);
		inversionCS->SetUnorderedAccessView("pingpong0", 0);
		inversionCS->SetUnorderedAccessView("pingpong1", 0);
	}

	jacobianCS->SetShader();
	jacobianCS->SetUnorderedAccessView("heightMapDX",dxUAV);
	jacobianCS->SetUnorderedAccessView("heightMapDY",dyUAV);
	jacobianCS->SetUnorderedAccessView("foldingMap",foldingMapUAV);
	jacobianCS->CopyAllBufferData();
	jacobianCS->DispatchByThreads(256, 256, 1);
	jacobianCS->SetUnorderedAccessView("heightMapDX",0);
	jacobianCS->SetUnorderedAccessView("heightMapDY",0);
	jacobianCS->SetUnorderedAccessView("foldingMap",0);


	sobelFilter->SetShader();
	sobelFilter->SetInt("N", 256);
	sobelFilter->SetFloat("normalStrength", 8);
	sobelFilter->SetSamplerState("sampleOptions", samplerState);
	sobelFilter->SetShaderResourceView("heightMap", dySRV);
	sobelFilter->SetUnorderedAccessView("normalMap", normalMapUAV);
	sobelFilter->CopyAllBufferData();
	sobelFilter->DispatchByThreads(256, 256, 1);
	sobelFilter->SetUnorderedAccessView("heightMap", 0);
	sobelFilter->SetUnorderedAccessView("normalMap", 0);

}
