// Struct representing a single vertex worth of data
struct VertexShaderInput
{
	float3 position		: POSITION;     // XYZ position
	//float4 color		: COLOR;        // RGBA color
	float3 normal		: NORMAL;		//Normal of the vertex
	float3 tangent		: TANGENT;      //tangent of the vertex
	float2 uv			: TEXCOORD;		//Texture coordinates
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 worldPos		: TEXCOORD;
};

//cbuffer for matrix position
cbuffer externalData: register(b0)
{
	matrix view;
	matrix projection;
	float3 cameraPos;
}

//entry point of the skybox vertex shader
VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output; //this specifies the data that gets passed along the render pipeline

	//view projection matrix
	matrix viewProj = mul(view, projection);

	//calculating the vertex position
	float4 posWorld = float4(input.position, 1.0);
	//posWorld.xyz += cameraPos;

	//getting the perspective devide to be equal to one
	output.position = mul(posWorld, viewProj).xyww;

	//sending the world position to pixelshader
	output.worldPos = input.position;

	return output;
}