#define GLM_ENABLE_EXPERIMENTAL

#include "ParticleEmitter.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Utilities.h"
#include <cassert>
#include "vendor/stb/stb_image.h"
#include "TextureLibrary.h"
#include <d3dcompiler.h>
#include "GeometryHelper.h"
#include <random>

using namespace std::chrono;
using std::array;
using glm::vec3;

ParticleEmitter::ParticleEmitter(ID3D11Device* device, ID3D11DeviceContext* context)
{
	perVertexBuffer = nullptr;
	perInstanceBuffer = nullptr;
	samplerState = nullptr;
	textureFrontSRV = nullptr;
	textureBackSRV = nullptr;
	vShader = nullptr;
	pShader = nullptr;
	iLayoutInstanced = nullptr;
	particles = array<Particle, 100>();

	// Load Shader and create Pipeline State Objects
	unsigned int flags = 0;
#if DEBUG || _DEBUG
	flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* error;
	ID3DBlob* vsSource;
	ID3DBlob* psSource;

	D3D_CALL(D3DCompileFromFile(L"particleShader.hlsl", 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "p0_VS", "vs_5_0", flags, 0, &vsSource, &error));
	if (error)
	{
		std::puts(static_cast<char*>(error->GetBufferPointer()));
		SafeRelease(error);
	}

	D3D_CALL(D3DCompileFromFile(L"particleShader.hlsl", 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "p0_PS", "ps_5_0", flags, 0, &psSource, &error));
	if (error)
	{
		std::puts(static_cast<char*>(error->GetBufferPointer()));
		SafeRelease(error);
	}

	device->CreateVertexShader(vsSource->GetBufferPointer(), vsSource->GetBufferSize(), NULL, &vShader);
	device->CreatePixelShader(psSource->GetBufferPointer(), psSource->GetBufferSize(), NULL, &pShader);
	assert(vShader && pShader);
	SafeRelease(psSource);

	D3D11_INPUT_ELEMENT_DESC iaDescs[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		// Instance Buffer
		// makes float4x4 at POSITION1
		{"POSITION", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"POSITION", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"POSITION", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"POSITION", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	D3D_CALL(device->CreateInputLayout(iaDescs, 7, vsSource->GetBufferPointer(), vsSource->GetBufferSize(), &iLayoutInstanced));
	assert(iLayoutInstanced);
	SafeRelease(vsSource);

	// Create Texture and Sampler State
	D3D_CALL(device->CreateSamplerState(&CD3D11_SAMPLER_DESC(D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 
		0.0f, 1, D3D11_COMPARISON_ALWAYS, glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), 
		0, D3D11_FLOAT32_MAX), &samplerState));
	assert(samplerState);

	ID3D11Texture2D* texture;

	// Front Texture
	int w, h, bpp;
	unsigned char * textureData = stbi_load(TextureLibrary::LEAF_FRONT, &w, &h, &bpp, STBI_rgb_alpha);

	D3D_CALL(device->CreateTexture2D(&CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R8G8B8A8_UNORM, 
		w, h, 1, 0, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 
		D3D11_USAGE_DEFAULT, 0, 1, 0, D3D11_RESOURCE_MISC_GENERATE_MIPS), NULL, &texture));
	assert(texture);

	context->UpdateSubresource(texture, 0, NULL, textureData, (w * bpp) * sizeof(char), 0);

	D3D_CALL(device->CreateShaderResourceView(texture, &CD3D11_SHADER_RESOURCE_VIEW_DESC(
		D3D_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM), &textureFrontSRV));
	assert(textureFrontSRV);
	
	SafeRelease(texture);
	stbi_image_free(textureData);

	// Back Texture
	textureData = stbi_load(TextureLibrary::LEAF_BACK, &w, &h, &bpp, STBI_rgb_alpha);

	D3D_CALL(device->CreateTexture2D(&CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R8G8B8A8_UNORM,
		w, h, 1, 0, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
		D3D11_USAGE_DEFAULT, 0, 1, 0, D3D11_RESOURCE_MISC_GENERATE_MIPS), NULL, &texture));
	assert(texture);

	context->UpdateSubresource(texture, 0, NULL, textureData, (w * bpp) * sizeof(char), 0);

	D3D_CALL(device->CreateShaderResourceView(texture, &CD3D11_SHADER_RESOURCE_VIEW_DESC(
		D3D_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM), &textureBackSRV));
	assert(textureBackSRV);

	SafeRelease(texture);
	stbi_image_free(textureData);

	context->GenerateMips(textureFrontSRV);
	context->GenerateMips(textureBackSRV);

	GeometryHelper::CreateQuadBuffer(device, perVertexBuffer, &numQuadVerts, 1.0f);

	for (size_t i = 0; i < particles.size(); i++)
	{
		float fi = static_cast<float>(i);
		float age = fmod(fi / 12.5f, particleLifetime);
		particles[i].position = glm::vec3(age, 0.0f, ((float)std::rand() / RAND_MAX) * 4.0f - 2.0f);
		particles[i].rotation = ((float)std::rand() / RAND_MAX) *2 * 3.14159f;
		//particles[i].rotation = 3.1416f / -2;
		particles[i].age = age;
	}

	auto worlds = CalculateInstanceMatrices();

	D3D11_SUBRESOURCE_DATA instanceData;
	instanceData.pSysMem = worlds;
	
	device->CreateBuffer(&CD3D11_BUFFER_DESC(sizeof(PerInstanceData) * (unsigned int)particles.size(), D3D11_BIND_VERTEX_BUFFER,
		D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE), &instanceData, &perInstanceBuffer);

	delete[] worlds;
	worlds = nullptr;

	lastTime = steady_clock::now();
}


ParticleEmitter::~ParticleEmitter()
{
	SafeRelease(perVertexBuffer);
	SafeRelease(perInstanceBuffer);
	SafeRelease(samplerState);
	SafeRelease(textureFrontSRV);
	SafeRelease(textureBackSRV);
	SafeRelease(vShader);
	SafeRelease(pShader);
	SafeRelease(iLayoutInstanced);
}

void ParticleEmitter::Draw(ID3D11DeviceContext* context)
{
	// Update Particles
	time_point<steady_clock> now = steady_clock::now();
	duration<float> duration = now - lastTime;
	lastTime = now;

	float deltaTime = duration.count();
	for (auto & particle : particles)
	{
		particle.position.x += 4.0f * deltaTime;
		particle.position.y = sinf(particle.age * 2.0f + sinf(particle.position.x / 4.0f)) * 0.1f;
		particle.age += deltaTime;
		if (particle.age > particleLifetime)
		{
			particle.position.x = ((float)std::rand() / RAND_MAX) * 4.0f - 2.0f;
			particle.age = 0;
		}
	}

	D3D11_MAPPED_SUBRESOURCE instanceData;
	D3D_CALL(context->Map(perInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &instanceData));

	PerInstanceData* worlds = CalculateInstanceMatrices();
	memcpy(instanceData.pData, worlds, sizeof(PerInstanceData) * 100);

	context->Unmap(perInstanceBuffer, 0);

	delete[] worlds;
	worlds = nullptr;

	context->PSSetShaderResources(0, 1, &textureFrontSRV);
	context->PSSetShaderResources(1, 1, &textureBackSRV);
	context->PSSetSamplers(0, 1, &samplerState);

	UINT strides[] = { sizeof(VertexPosTex), sizeof(PerInstanceData) };
	UINT offsets[] = { 0, 0 };

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetInputLayout(iLayoutInstanced);
	context->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);
	context->VSSetShader(vShader, 0, 0);
	context->PSSetShader(pShader, 0, 0);
	context->DrawInstanced(numQuadVerts, static_cast<unsigned int>(particles.size()), 0, 0);
}

ParticleEmitter::PerInstanceData* ParticleEmitter::CalculateInstanceMatrices() const
{
	PerInstanceData* result = new PerInstanceData[100];

	for (size_t i = 0; i < particles.size(); i++)
	{
		auto translation = glm::translate(glm::mat4(1.0f), particles[i].position);
		auto rotation = glm::angleAxis(particles[i].rotation, vec3(1.0f, 0.0f, 0.0f));
		result[i].world = glm::toMat4(rotation) * translation;
		result[i].normalW = vec3(0.0f, 0.0f, -1.0f) * (glm::mat3)glm::inverseTranspose(result[i].world);
	}

	return result;
}
