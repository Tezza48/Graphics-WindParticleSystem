#pragma once
#include <d3d11.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <chrono>

class ParticleEmitter
{
	struct Particle
	{
		glm::vec3 position;
	};

	union
	{
		ID3D11Buffer* vertexBuffers[2];
		struct { ID3D11Buffer* perVertexBuffer; ID3D11Buffer* perInstanceBuffer; };
	};

	ID3D11SamplerState* samplerState;
	ID3D11ShaderResourceView* textureSRV;

	ID3D11VertexShader* vShader;
	ID3D11PixelShader* pShader;
	ID3D11InputLayout* iLayoutInstanced;

	bool generateMipmaps;

	unsigned int numQuadVerts;

	static const size_t NUM_LEAVES = 100;
	Particle particles[NUM_LEAVES]; // particles array

	std::chrono::time_point<std::chrono::steady_clock> lastTime;

public:
	ParticleEmitter(ID3D11Device * device, ID3D11DeviceContext* context);
	~ParticleEmitter();

	void Draw(ID3D11DeviceContext* context);

private:
	glm::mat4* CalculateInstanceMatrices() const;
};

