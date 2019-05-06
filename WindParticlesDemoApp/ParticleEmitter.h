#pragma once
#include <d3d11.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <chrono>
#include <array>

class ParticleEmitter
{
	struct Particle
	{
		glm::vec3 position;
		float rotation;
		float age;
	};

	struct PerInstanceData
	{
		glm::mat4 world;
	};

	union
	{
		ID3D11Buffer* vertexBuffers[2];
		struct { ID3D11Buffer* perVertexBuffer; ID3D11Buffer* perInstanceBuffer; };
	};

	ID3D11SamplerState* samplerState;
	ID3D11ShaderResourceView* textureFrontSRV;
	ID3D11ShaderResourceView* textureBackSRV;

	ID3D11VertexShader* vShader;
	ID3D11PixelShader* pShader;
	ID3D11InputLayout* iLayoutInstanced;

	unsigned int numQuadVerts;

	const float particleLifetime = 8.0f;
	std::array<Particle, 100> particles; // particles array

	std::chrono::time_point<std::chrono::steady_clock> lastTime;

public:
	ParticleEmitter(ID3D11Device * device, ID3D11DeviceContext* context);
	~ParticleEmitter();

	void Draw(ID3D11DeviceContext* context);

private:
	PerInstanceData* CalculateInstanceMatrices() const;
};

