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
	float3 cameraPosition;
	Light lights[MAX_LIGHTS];
	int lightCount;
};

//textures and basic samplers
Texture2D diffuseTexture: register(t0);
Texture2D normalMap: register(t1);
Texture2D roughnessMap: register(t2);
Texture2D metalnessMap: register(t3);
Texture2D shadowMap	: register(t4);
TextureCube cubeMap: register(t5);
TextureCube irradianceMap: register(t6);
Texture2D environmentBRDF: register(t7);
TextureCube prefilteredMap: register(t8);
SamplerState basicSampler: register(s0);
SamplerComparisonState shadowSampler: register(s1);


float3 CalculateDiffuse(float3 n, float3 l, float4 diffuseColor, DirectionalLight light)
{
	float3 L = l;
	L = normalize(L); //normalizing the negated direction
	float3 N = n;
	N = normalize(N); //normalizing the normal

	float NdotL = dot(N, L);
	NdotL = saturate(NdotL); //this is the light amount, we need to clamp it to 0 and 1.0

	//adding diffuse, ambient color
	float3 finalLight = light.diffuse.xyz * NdotL;
	//finalLight += light.ambientColor;
	return finalLight;
}

float3 CalculateRadiance(Light light, float3 N, float3 V, float3 L, float3 R, float3 H)
{
	float3 Li = float3(0.0f, 0.0f, 0.0f);

	//do this calculation for each light
	float3 F = float3(0.0f, 0.0f, 0.0f);
	float D = 0.0f;
	float G = 0.0f;

	//calculating the diffuse and specular color
	float3 lambert;// = CalculateDiffuse(N, L, light.diffuse.xyzz, light);
	//CookTorrence(N, H, roughness, V, f0, L, F, D, G);

	float3 ks = F; //fresnel is specular term
	float3 kd = float3(1.0f, 1.0f, 1.0f) - ks;
	//kd *= (float3(1.0f, 1.0f, 1.0f) - metalColor);

	float3 numSpec = D * F * G;
	float denomSpec = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f);
	float3 specular = numSpec / max(denomSpec, 0.0001f); //just in case denominator is zero

	//Li += ((kd * surfaceColor.xyz / PI) + specular) * lambert;

	return Li;
}

float3 DirectionalLightCalculations(Light light, float3 N, float3 worldPos)
{
	float3 c;

	float3 L = -light.direction;
	L = normalize(L); //normalizing the negated direction
	N = normalize(N); //normalizing the normal

	float3 V = normalize(cameraPosition - worldPos); //view vector
	float3 H = normalize(L + V);
	float3 R = reflect(-V, N); //reflect R over N

	c = CalculateRadiance(light, N, V, L, R, H);

	return c;
}

float3 SpotLightCalculations(Light light, float3 N, float3 worldPos)
{
	float3 c;
	return c;
}

float3 PointLightCalculations(Light light, float3 N, float3 worldPos)
{
	float3 c;
	return c;
}

//function to calculate diffuse color based on energy conservation	
float3 DiffuseEnergyConserve(
	float3 diffuse, float3 specular, float3 metalness)
{
	return diffuse *((1 - saturate(specular)) * (1 - metalness));
}


float3 CalculateEnvironmentReflection(float3 normal, float3 position)
{
	float3 I = position - cameraPosition;
	I = normalize(I); //incident ray
	float3 reflected = reflect(I, normal);

	float3 reflectedColor = cubeMap.Sample(basicSampler, reflected).xyz;

	return reflectedColor;
}


//calculate shadows
float ShadowCalculation(DirectionalLight light, float2 shadowUV, float lightDepth)
{
	//getting the width and height of shadow map
	float w;
	float h;
	float shadowDepth = 0.0f;

	shadowMap.GetDimensions(w, h);

	for (float j = -1.0f; j <= 1.0f; j++)
	{
		for (float i = -1.0f; i <= 1.0f; i++)
		{
			float2 offset = float2(i / (float)w, j / (float)h);
			shadowDepth += shadowMap.SampleCmpLevelZero(shadowSampler, offset + shadowUV, lightDepth);
		}
	}

	return (shadowDepth / 9.0f);
}

float4 main(VertexToPixel input) : SV_TARGET
{
	//clip(input.clip);
	//sampling the diffuse texture
	float4 surfaceColor = diffuseTexture.Sample(basicSampler,input.uv);

	surfaceColor = pow(abs(surfaceColor), 2.2);
	
	//getting the normal texture
	float3 normalColor = normalMap.Sample(basicSampler, input.uv).xyz;
	float3 unpackedNormal = normalColor * 2.0f - 1.0f;

	//orthonormalizing T, B and N using the gram-schimdt process
	float3 N = normalize(input.normal);
	float3 T = input.tangent - dot(input.tangent, N) * N;
	T = normalize(T);
	float3 B = normalize(cross(T,N));

	float3x3 TBN = float3x3(T, B, N); //getting the tbn matrix

	//transforming normal from map to world space
	float3 finalNormal = mul(unpackedNormal, TBN);

	//getting the metalness of the pixel
	float3 metalColor = metalnessMap.Sample(basicSampler, input.uv).xyz;

	float3 f0 = float3(0.04f, 0.04f, 0.04f);
	f0 = lerp(f0, surfaceColor.xyz, metalColor);

	//getting the roughness of pixel
	float roughness = roughnessMap.Sample(basicSampler, input.uv).x;

	//step 1 --- Solving the radiance integral for direct lighting, the integral is just the number of light sources
	// the solid angle on the hemisphere in infinitely small, so the wi is just a direction vector
	//for now radiance is just the color of the direction light, the diffuse part is lambertian*c/pi
	//specular is calculated by cook torrence, which ontains in ks term in is due to fresnel

	float3 L = -light.direction;
	L = normalize(L); //normalizing the negated direction
	N = finalNormal;
	N = normalize(N); //normalizing the normal
	float3 V = normalize(cameraPosition - input.worldPosition); //view vector
	float3 H = normalize(L+V);
	float3 R = reflect(-V, N); //reflect R over N

	float3 Lo = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < lightCount; i++)
	{
		switch (lights[i].type)
		{
		case LIGHT_TYPE_DIR: Lo += DirectLightPBR(lights[i], N, input.worldPosition, cameraPosition, 
			roughness,	metalColor.r, surfaceColor.xyz,  f0); break;
		case LIGHT_TYPE_SPOT: Lo += SpotLightPBR(lights[i], N, input.worldPosition, cameraPosition,
			roughness, metalColor.r, surfaceColor.xyz, f0); break;
		case LIGHT_TYPE_POINT: Lo += PointLightPBR(lights[i], N, input.worldPosition, cameraPosition,
			roughness, metalColor.r, surfaceColor.xyz, f0); break;
		}
	}


	//calulating skybox reflection
	float3 skyboxReflection = CalculateEnvironmentReflection(finalNormal, input.worldPosition);
	
	//doing calculations from shadow map to see if it is a shadow or not
	float lightDepth = input.lightPos.z / input.lightPos.w;

	//adjust the -1 to 1 range to 0 to 1
	float2 shadowUV = input.lightPos.xy / input.lightPos.w * 0.5f + 0.5f;

	//flip the y coordinates
	shadowUV.y = 1.0f - shadowUV.y;

	//read shadow map for closest surface
	float shadowDepth = ShadowCalculation(light, shadowUV, lightDepth);

	float3 ksIndirect = FresnelRoughness(dot(N,V), f0,roughness);

	float3 kdIndirect = float3(1.0f, 1.0f, 1.0f) - ksIndirect;

	kdIndirect *= (1 - metalColor);

	kdIndirect *= surfaceColor.rgb / PI;

	float3 irradiance = irradianceMap.Sample(basicSampler, N).rgb;

	float3 diffuseIndirect = surfaceColor.rgb * irradiance;

	float3 prefilteredColor = prefilteredMap.SampleLevel(basicSampler, R, roughness * 4.0).rgb;

	float2 envBRDF = environmentBRDF.Sample(basicSampler, float2(saturate(dot(N, V)), roughness)).rg;

	float3 specularIndirect = prefilteredColor * (ksIndirect * envBRDF.x + envBRDF.y);

	float3 ambientIndirect = (kdIndirect * diffuseIndirect + specularIndirect * surfaceColor.rgb);

	float3 color = (Lo*shadowDepth + ambientIndirect);

	float4 rimColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
	
	color = pow(abs(color), 1.f / 2.2f);

	//return lerp(rimColor, float4(color, 1.0f), float4(ksIndirect,1.0f));

	return float4(color, 1.0f);
}