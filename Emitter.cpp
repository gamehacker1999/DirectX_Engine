#include "Emitter.h"

Emitter::Emitter(int maxParticles, int particlesPerSecond, 
	float lifetime, float startSize, float endSize, 
	XMFLOAT4 startColor, 
	XMFLOAT4 endColor, 
	XMFLOAT3 startVelocity, 
	XMFLOAT3 velocityRandomRange, 
	XMFLOAT3 emitterPosition, 
	XMFLOAT3 positionRandomRange, 
	XMFLOAT4 rotationRandomRanges, 
	XMFLOAT3 emitterAcceleration, 
	ID3D11Device* device, 
	SimpleVertexShader* vs, 
	SimplePixelShader* ps, 
	ID3D11ShaderResourceView* texture)
{
	this->maxParticles = maxParticles; //max particles spewed
	this->particlesPerSecond = particlesPerSecond; //particles spewed per second
	this->secondsPerParticle = 1.0f / particlesPerSecond; //amount after which a particle is spawned
	this->lifetime = lifetime; //lifetime of each particle
	this->startSize = startSize; //start size
	this->endSize = endSize; //end size
	this->startColor = startColor; //start color to interpolate from
	this->endColor = endColor; //end color to interpolate to
	this->startVelocity = startVelocity; //start velocity
	this->velocityRandomRange = velocityRandomRange; //range of velocity
	this->emitterPosition = emitterPosition; //position of emitter
	this->positionRandomRange = positionRandomRange; //range of pos
	this->rotationRandomRanges = rotationRandomRanges; //random ranges of rotation
	this->emitterAcceleration = emitterAcceleration; //acceleration of emmiter
	this->ps = ps;
	this->vs = vs;
	this->texture = texture;

	timeSinceEmit = 0;//how long since the last particle was emmited
	livingParticleCount = 0; //count of how many particles
	//circular buffer indices
	firstAliveIndex = 0;
	firstDeadIndex = 0;
	isDead = false;
	isTemp = false;
	emitterAge = true;
	explosive = false;

	particles = new Particle[maxParticles];//list of particles
	ZeroMemory(particles, sizeof(Particle) * maxParticles);

	unsigned int* indices = new unsigned int[6 * maxParticles];
	int indexCount = 0;
	for (int i = 0; i < maxParticles*4; i+=4)
	{
		indices[indexCount++] = i;
		indices[indexCount++] = i+1;
		indices[indexCount++] = i+2;
		indices[indexCount++] = i;
		indices[indexCount++] = i+2;
		indices[indexCount++] = i+3;

	}

	//creating subresource
	D3D11_SUBRESOURCE_DATA ibdSub = {};
	ibdSub.pSysMem =  (indices);

	//creating indexbuffer
	D3D11_BUFFER_DESC ibd = {};
	ibd.CPUAccessFlags = 0;
	ibd.ByteWidth = 6 * sizeof(unsigned int) * maxParticles;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.Usage = D3D11_USAGE_DEFAULT;

	device->CreateBuffer(&ibd, &ibdSub, &indexBuffer);

	//create a structured buffer to hold the particles
	D3D11_BUFFER_DESC particleBufferDesc = {};
	particleBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; //bind it to srv
	particleBufferDesc.ByteWidth = maxParticles*sizeof(Particle);
	particleBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	particleBufferDesc.StructureByteStride = sizeof(Particle); //sizeof structure
	particleBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	particleBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED; //this is a structured buffer
	//creat the buffer
	device->CreateBuffer(&particleBufferDesc, 0, &particleBuffer);

	D3D11_SHADER_RESOURCE_VIEW_DESC particleSRV = {}; //srv for particles
	particleSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	particleSRV.Format = DXGI_FORMAT_UNKNOWN;
	particleSRV.Buffer.FirstElement = 0;
	particleSRV.Buffer.NumElements = maxParticles;	

	device->CreateShaderResourceView(particleBuffer, &particleSRV, &particleData);

	delete[] indices;
}

Emitter::~Emitter()
{
	indexBuffer->Release();
	particleBuffer->Release();
	particleData->Release();
	delete[] particles;
}

XMFLOAT3 Emitter::GetPosition()
{
	return emitterPosition;
}

void Emitter::SetPosition(XMFLOAT3 pos)
{
	emitterPosition = pos;
}

void Emitter::SetAcceleration(XMFLOAT3 acel)
{
	emitterAcceleration = acel;
}

void Emitter::UpdateParticles(float deltaTime, float currentTime)
{

	if (isTemp)
	{
		emitterAge += deltaTime;
		if (emitterAge >= emitterLifetime)
		{
			isDead = true;
		}
	}

	if (livingParticleCount > 0)
	{
		//looping through the circular buffer
		if (firstAliveIndex < firstDeadIndex)
		{
			for (int i = firstAliveIndex; i < firstDeadIndex; i++)
			{
				UpdateSingleParticle(deltaTime, i,currentTime);
			}
		}

		//if firse alive is ahead
		else
		{
			//go from the first alive to end of list
			for (int i = firstAliveIndex; i < maxParticles; i++)
			{
				UpdateSingleParticle(deltaTime, i,currentTime);
			}

			//go from zero to dead
			for (int i = 0; i < firstDeadIndex; i++)
			{
				UpdateSingleParticle(deltaTime, i,currentTime);
			}
		}
	}

	timeSinceEmit += deltaTime;

	while(timeSinceEmit >= secondsPerParticle)
	{
		SpawnParticle(currentTime);
		timeSinceEmit -= secondsPerParticle;
	}

}

void Emitter::Draw(ID3D11DeviceContext* context, XMFLOAT4X4 view, XMFLOAT4X4 projection, float currentTime)
{
	//mapping the data so that gpu cannot write to it
	D3D11_MAPPED_SUBRESOURCE mapped = {};

	//mapping the vertex buffer so that cpu can write to it
	context->Map(particleBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	//copying the data from cpu to gpu
	memcpy(mapped.pData, particles, sizeof(Particle) * maxParticles);

	//unmapping the resource
	context->Unmap(particleBuffer, 0);

	//setting the up the buffer
	UINT stride = 0;
	UINT offset = 0;
	ID3D11Buffer* nullBuffer=nullptr;
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);

	//setting the view and projection matrix
	vs->SetMatrix4x4("view", view);
	vs->SetMatrix4x4("projection", projection);
	vs->SetFloat3("acceleration", emitterAcceleration);
	vs->SetFloat4("startColor", startColor);
	vs->SetFloat4("endColor", endColor);
	vs->SetFloat("startSize", startSize);
	vs->SetFloat("endSize", endSize);
	vs->SetFloat("lifetime",lifetime);
	vs->SetFloat("currentTime",currentTime);
	vs->SetShader();

	context->VSSetShaderResources(0, 1, &particleData);

	ps->SetShaderResourceView("particle", texture);
	ps->SetShader();
	ps->CopyAllBufferData();

	if (firstAliveIndex < firstDeadIndex)
	{
		vs->SetInt("startIndex", firstAliveIndex);
		vs->CopyAllBufferData();
		context->DrawIndexed(livingParticleCount * 6, 0, 0);
	}

	else
	{
		//draw from 0 to dead
		vs->SetInt("startIndex", 0);
		vs->CopyAllBufferData();
		context->DrawIndexed(firstDeadIndex*6, 0, 0);

		//draw from alive to max
		vs->SetInt("startIndex", firstAliveIndex);
		vs->CopyAllBufferData();
		context->DrawIndexed((maxParticles - firstAliveIndex) * 6, 0, 0);
	}

}

void Emitter::SetTemporary(float emitterLife)
{
	isTemp = true;
	this->emitterLifetime = emitterLife;
}

bool Emitter::IsDead()
{
	return isDead;
}

void Emitter::Explosive()
{
	explosive = true;
}

void Emitter::UpdateSingleParticle(float dt, int index,float currentTime)
{
	float age = currentTime - particles[index].spawnTime;

	//if age exceeds its lifespan
	if (age >= lifetime)
	{
		//increase the first alive index
		firstAliveIndex++;
		//wrap it
		firstAliveIndex %= maxParticles;
		//kill this particle
		livingParticleCount--;
		return;
	}
}

void Emitter::SpawnParticle(float currentTime)
{
	if (livingParticleCount == maxParticles)
		return;

	//spawinig a new particle
	particles[firstDeadIndex].spawnTime = currentTime;

	//random position and veloctiy of the particle
	std::random_device rd;
	std::mt19937 randomGenerator(rd());
	std::uniform_real_distribution<float> dist(-1, 1);

	particles[firstDeadIndex].startPosition = emitterPosition; //particles start at emitter
	//randomizing their x,y,z
	particles[firstDeadIndex].startPosition.x += dist(randomGenerator) * positionRandomRange.x;
	particles[firstDeadIndex].startPosition.y += dist(randomGenerator) * positionRandomRange.y;
	particles[firstDeadIndex].startPosition.z += dist(randomGenerator) * positionRandomRange.z;

	particles[firstDeadIndex].startVelocity = startVelocity;
	particles[firstDeadIndex].startVelocity.x += dist(randomGenerator) * velocityRandomRange.x;
	particles[firstDeadIndex].startVelocity.y += dist(randomGenerator) * velocityRandomRange.y;
	particles[firstDeadIndex].startVelocity.z += dist(randomGenerator) * velocityRandomRange.z;

	//random start rotation
	float rotStartMin = rotationRandomRanges.x;
	float rotStartMax = rotationRandomRanges.y;

	//choosing a random start rotation
	particles[firstDeadIndex].rotationStart = dist(randomGenerator) * (rotStartMax - rotStartMin) + rotStartMin;

	//random start rotation
	float rotEndMin = rotationRandomRanges.z;
	float rotEndMax = rotationRandomRanges.w;

	//choosing a random start rotation
	particles[firstDeadIndex].rotationEnd = dist(randomGenerator) * (rotEndMax - rotEndMin) + rotEndMin;

	//increment the first dead index
	firstDeadIndex++;
	firstDeadIndex %= maxParticles;

	//increment living particles
	livingParticleCount++;

}

