
cbuffer externalData: register(b0)
{
	matrix view; //light view matrix
	matrix projection; //light projection
	matrix worldMatrix; //world matrix of entity
}

struct VertexShaderInput
{
	float3 position		: POSITION;     // XYZ position
	//float4 color		: COLOR;        // RGBA color
	float3 normal		: NORMAL;		//Normal of the vertex
	float3 tangent		: TANGENT;      //tangent of the vertex
	float2 uv			: TEXCOORD;		//Texture coordinates
};


float4 main(VertexShaderInput input): SV_POSITION
{
	//calculating the MVP matrix
	matrix worldViewProj = mul(mul(worldMatrix,view),projection);

	//transforming the point
	float4 outpos = mul(float4(input.position,1.0f),worldViewProj);

	return outpos; //returning the position
	
}