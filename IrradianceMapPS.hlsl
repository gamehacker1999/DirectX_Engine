
//struct to represent the vertex shader input
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 worldPos		: TEXCOORD;
};

TextureCube skybox : register(t0);
SamplerState basicSampler: register(s0);

float4 main(VertexToPixel input):SV_TARGET
{
	//calculating the irradience map
	float3 irradiance = float3(0.0f,0.0f,0.0f);

	float3 normal = normalize(input.worldPos);

	float3 up=float3(0.0f, 1.0f, 0.0f);

	float3 right = normalize(cross(up,normal));
	up = normalize(cross(normal, right));

	float PI = 3.141592653f;

	float sampleDelta = 0.025f;
	float numOfSamples = 0.0f;
	//solving the monte carlo integral of the irradience equation
	//[loop]
	for (float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta)
	{
		//[loop]
		for (float theta = 0.0f; theta < PI * 0.5f; theta += sampleDelta)
		{
			//convert spherical coordinates to cartesian space
			float3 cartesian = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

			//connverting from tangent space to world space
			float3 sampleVector = (cartesian.x * right) + (cartesian.y * up) + (cartesian.z * normal);

			//adding all the texure values
			irradiance += skybox.Sample(basicSampler, sampleVector).rgb*sin(theta)*cos(theta);

			numOfSamples++;
		}
	}

	//dividing by the total number of samples
	irradiance = PI * irradiance * (1 / numOfSamples);

	//returning the sample value
	return float4(irradiance, 1.0f);
	
}