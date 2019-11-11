#include "Lighting.hlsli"

struct VertexToPixel
{

	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float4 lightPos		: TEXCOORD1;
	float3 normal		: NORMAL;		//normal of the vertex
	float3 worldPosition: POSITION; //position of vertex in world space
	float3 tangent		: TANGENT;	//tangent of the vertex
	float2 uv			: TEXCOORD;
};

//struct to desctibe the directional light
struct DirectionalLight
{
	float4 ambientColor;
	float4 diffuse;
	float4 specularity;
	float3 direction;
};

//constant buffer to hold light data
cbuffer LightData: register(b0)
{
	DirectionalLight light;
	DirectionalLight light2;
	float scrollX;
	float scrollY;
	float3 cameraPosition;
	Light lights[MAX_LIGHTS];
	int lightCount;
};

//function that accepts light and normal and then calculates the final color
float4 CalculateLight(DirectionalLight light, float3 normal, VertexToPixel input)
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
	float4 finalLight = light.diffuse * NdotL;
	finalLight += specularAmount;

	return finalLight;
}

Texture2D waterTexture: register(t0);
Texture2D normalTexture1: register(t1);
Texture2D normalTexture2: register(t2);
SamplerState sampleOptions: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	float2 scrollUV1 = float2(input.uv.x + scrollX,input.uv.y);
	float3 normal1 = normalTexture1.Sample(sampleOptions, scrollUV1).rgb;
	//unpacking the normal
	normal1 = (normal1 * 2.0f) - 1.0f;

	float2 scrollUV2 = float2(input.uv.x, input.uv.y+scrollY);
	float3 normal2 = normalTexture2.Sample(sampleOptions, scrollUV2).rgb;

	normal2 = (normal2 * 2.0f) - 1.0f;

	float4 surfaceColor = waterTexture.Sample(sampleOptions, input.uv);

	surfaceColor = pow(surfaceColor, 2.2);

	float3 N = normalize(input.normal);
	float3 T = input.tangent - (dot(input.tangent, N) * N);
	float3 B = normalize(cross(T, N));
	float3x3 TBN = float3x3(T, B, N);

	normal1 = mul(normal1, TBN);
	normal2 = mul(normal2, TBN);

	//averaging the two normals
	float3 finalNormal = normalize(normal1 + normal2);

	float3 f0 = float3(0.02f, 0.02f, 0.02f);
	float3 metalColor = float3(0.25f, 0.25f, 0.25f);
	float roughness = 0.5f;

	//calculate the lighting
	float3 L = -lights[0].direction;
	L = normalize(L); //normalizing the negated direction
	N = finalNormal;
	N = normalize(N); //normalizing the normal
	float3 V = normalize(cameraPosition - input.worldPosition); //view vector
	float3 H = normalize(L + V);
	float3 R = reflect(-V, N); //reflect R over N

	float3 Lo = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < lightCount; i++)
	{
		switch (lights[i].type)
		{
		case LIGHT_TYPE_DIR: Lo += DirectLightPBR(lights[i], N, input.worldPosition, cameraPosition,
			roughness, metalColor.r, surfaceColor.xyz, f0); break;
		case LIGHT_TYPE_SPOT: Lo += SpotLightPBR(lights[i], N, input.worldPosition, cameraPosition,
			roughness, metalColor.r, surfaceColor.xyz, f0); break;
		case LIGHT_TYPE_POINT: Lo += PointLightPBR(lights[i], N, input.worldPosition, cameraPosition,
			roughness, metalColor.r, surfaceColor.xyz, f0); break;
		}
	}

	/*float3 ksIndirect = FresnelRoughness(dot(N, V), f0, roughness);

	float3 kdIndirect = float3(1.0f, 1.0f, 1.0f) - ksIndirect;

	kdIndirect *= (1 - metalColor);

	kdIndirect *= surfaceColor.rgb / PI;

	float3 irradiance = irradianceMap.Sample(basicSampler, N).rgb;

	float3 diffuseIndirect = surfaceColor.rgb * irradiance;

	float3 prefilteredColor = prefilteredMap.SampleLevel(basicSampler, R, roughness * 4.0).rgb;

	float2 envBRDF = environmentBRDF.Sample(basicSampler, float2(saturate(dot(N, V)), roughness)).rg;

	float3 specularIndirect = prefilteredColor * (ksIndirect * envBRDF.x + envBRDF.y);

	float3 ambientIndirect = (kdIndirect * diffuseIndirect + specularIndirect * surfaceColor.rgb);*/

	float3 color = (Lo);

	color = pow(abs(color), 1.f / 2.2f);

	return float4(color, 1.0f);
}