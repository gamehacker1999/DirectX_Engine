
// Constant Buffer
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	matrix lightView;
	matrix lightProj;
};

// Struct representing a single vertex worth of data
struct VertexShaderInput
{

	float3 position		: POSITION;     // XYZ position
	//float4 color		: COLOR;        // RGBA color
	float3 normal		: NORMAL;		//Normal of the vertex
	float3 tangent		: TANGENT;      //tangent of the vertex
	float2 uv			: TEXCOORD;		//Texture coordinates
};

// Struct representing the data we're sending down the pipeline
struct VertexToPixel
{

	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float4 lightPos		: TEXCOORD1;
	float3 normal		: NORMAL;		//normal of the vertex
	float3 worldPosition: POSITION; //position of vertex in world space
	float3 tangent		: TANGENT;	//tangent of the vertex
	float2 uv			: TEXCOORD;
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
	VertexToPixel output;

	// First we multiply them together to get a single matrix which represents
	// all of those transformations (world to view to projection space)
	matrix worldViewProj = mul(mul(world, view), projection);

	// The result is essentially the position (XY) of the vertex on our 2D 
	// screen and the distance (Z) from the camera (the "depth" of the pixel)
	output.position = mul(float4(input.position, 1.0f), worldViewProj);

	//applying the normal by removing the translation from it
	output.normal = mul(input.normal, (float3x3)world);

	//sending the world position of the vertex to the fragment shader
	output.worldPosition = mul(float4(input.position, 1.0f), world).xyz;

	//sending the world coordinates of the tangent to the pixel shader
	output.tangent = mul(input.tangent, (float3x3)world);

	//sending the UV coordinates
	output.uv = input.uv;

	// Whatever we return will make its way through the pipeline to the pixel shader
	return output;
}