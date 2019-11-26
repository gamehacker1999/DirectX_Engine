#include "Lighting.hlsli"

struct VertexToPixel
{

	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float4 lightPos		: TEXCOORD1;
	float3 normal		: NORMAL;		//normal of the vertex
	float3 worldPosition: POSITION; //position of vertex in world space
	float3 tangent		: TANGENT;	//tangent of the vertex
	float2 uv			: TEXCOORD;
	float clip : SV_ClipDistance0;
};


//constant buffer to hold light data
cbuffer LightData: register(b0)
{
	float3 cameraPosition;
	matrix view;
	Light dirLight;

	float uvScale0;
	float uvScale1;
	float uvScale2;

	float3 surfaceColor;

};

//function that accepts light and normal and then calculates the final color
float4 CalculateLight(Light light, float3 normal, VertexToPixel input)
{
	//standard N dot L calculation for the light
	float3 L = -light.direction;
	L = normalize(L); //normalizing the negated direction
	float3 N = normal;
	N = normalize(N); //normalizing the normal
	float3 R = reflect(-L, N); //reflect R over N
	float3 V = normalize(cameraPosition - input.worldPosition); //view vector
	float4 NdotV = saturate(dot(N, V));
	float4 rimColor = float4(0.0f, 0.0f, 1.0f, 1.0f);

	//calculate the cosine of the angle to calculate specularity
	//I am calculating the light based on the phong reflection model
	float cosine = dot(R, V);
	cosine = saturate(cosine);
	float shininess = 64.f;
	float specularAmount = pow(cosine, shininess); //increase the cosine curve fall off

	float NdotL = dot(N, L);
	NdotL = saturate(NdotL); //this is the light amount, we need to clamp it to 0 and 1.0
	//return diffuse;

	//adding diffuse, ambient, and specular color
	float4 finalLight = float4(light.diffuse, 1.0f) * NdotL;
	finalLight += specularAmount;
	finalLight += float4(0.3, 0.3, 0.3, 1.0f);

	return finalLight;
}

// Textures and samplers
Texture2D blendMap			: register(t0);

Texture2D texture0			: register(t1);
Texture2D texture1			: register(t2);
Texture2D texture2			: register(t3);

Texture2D normalMap0		: register(t4);
Texture2D normalMap1		: register(t5);
Texture2D normalMap2		: register(t6);

SamplerState samplerOptions : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	// Re-normalize interpolated normals!
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	
	// Sample the texture, using the provided options, as
	// the specified UV coordinates
	float4 blend = blendMap.Sample(samplerOptions, input.uv);
	
	float4 color0 = texture0.Sample(samplerOptions, input.uv * uvScale0);
	float4 color1 = texture1.Sample(samplerOptions, input.uv * uvScale1);
	float4 color2 = texture2.Sample(samplerOptions, input.uv * uvScale2);
	
	float3 normalFromMap0 = normalMap0.Sample(samplerOptions, input.uv * uvScale0).rgb * 2 - 1;
	float3 normalFromMap1 = normalMap1.Sample(samplerOptions, input.uv * uvScale1).rgb * 2 - 1;
	float3 normalFromMap2 = normalMap2.Sample(samplerOptions, input.uv * uvScale2).rgb * 2 - 1;
	
	// Blend the textures together
	float4 textureColor =
		color0 * blend.r +
		color1 * blend.g +
		color2 * blend.b;
	
	float3 normalFromMap =
		normalize(
			normalFromMap0 * blend.r +
			normalFromMap1 * blend.g +
			normalFromMap2 * blend.b);
	
	// === Normal mapping here!  We need a new normal for the rest of the lighting steps ===
	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N)); // Orthogonalize!
	float3 B = cross(T, N); // The bi-tangent
	
	float3x3 TBN = float3x3(T, B, N);
	
	// Rotate the normal from the normal map by our TBN rotation matrix
	input.normal = normalize(mul(normalFromMap, TBN));

	float4 lightingColor = CalculateLight(dirLight, input.normal, input);

	return lightingColor * textureColor;
}