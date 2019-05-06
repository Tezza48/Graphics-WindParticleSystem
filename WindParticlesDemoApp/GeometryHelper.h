#pragma once
#include <d3d11.h>
#include <glm/glm.hpp>

struct VertexPosTex
{
	glm::vec3 position;
	glm::vec2 texCoord;
};

struct VertexPosTexNormal
{
	glm::vec3 position;
	glm::vec2 texCoord;
	glm::vec3 normal;
};

namespace GeometryHelper
{
	void CreateQuadBuffer(ID3D11Device* device, ID3D11Buffer*& quadVBuffer, unsigned int* numVerts, float scale = 1.0f);
	void CreateGrid(ID3D11Device* device, ID3D11Buffer*& vBuffer, unsigned int* numVerts);
};

