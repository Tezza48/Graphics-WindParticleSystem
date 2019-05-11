#include "GeometryHelper.h"
#include "Utilities.h"

using namespace glm;

void GeometryHelper::CreateQuadBuffer(ID3D11Device* device, ID3D11Buffer*& quadVBuffer, unsigned int* numVerts, float scale)
{
	*numVerts = 4;

	VertexPosTex initialData[] =
	{
		{ vec3(-0.5f, -0.5f, 0.0f) * scale, vec2(0.0f, 0.0f)},
		{ vec3(-0.5f,  0.5f, 0.0f) * scale, vec2(0.0f, 1.0f)},
		{ vec3( 0.5f, -0.5f, 0.0f) * scale, vec2(1.0f, 0.0f)},
		{ vec3( 0.5f,  0.5f, 0.0f) * scale, vec2(1.0f, 1.0f)}
	};

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = initialData;

	D3D_CALL(device->CreateBuffer(&CD3D11_BUFFER_DESC(sizeof(initialData), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE), &data, &quadVBuffer));
}

void GeometryHelper::CreateGrid(ID3D11Device* device, ID3D11Buffer*& vBuffer, unsigned int * numVerts)
{
	*numVerts = 44;
	const int gridSize = 10;
	VertexPosTex gridVerts[] = {
		// Z
		{ { -5.0f, 0.0f, -5.0f }, { 0.0f, 1.0f } },
		{ { -5.0f, 0.0f,  5.0f }, { 0.0f, 1.0f } },
		{ { -4.0f, 0.0f, -5.0f }, { 0.0f, 1.0f } },
		{ { -4.0f, 0.0f,  5.0f }, { 0.0f, 1.0f } },
		{ { -3.0f, 0.0f, -5.0f }, { 0.0f, 1.0f } },
		{ { -3.0f, 0.0f,  5.0f }, { 0.0f, 1.0f } },
		{ { -2.0f, 0.0f, -5.0f }, { 0.0f, 1.0f } },
		{ { -2.0f, 0.0f,  5.0f }, { 0.0f, 1.0f } },
		{ { -1.0f, 0.0f, -5.0f }, { 0.0f, 1.0f } },
		{ { -1.0f, 0.0f,  5.0f }, { 0.0f, 1.0f } },
		{ {  0.0f, 0.0f, -5.0f }, { 0.0f, 1.0f } },
		{ {  0.0f, 0.0f,  5.0f }, { 0.0f, 1.0f } },
		{ {  1.0f, 0.0f, -5.0f }, { 0.0f, 1.0f } },
		{ {  1.0f, 0.0f,  5.0f }, { 0.0f, 1.0f } },
		{ {  2.0f, 0.0f, -5.0f }, { 0.0f, 1.0f } },
		{ {  2.0f, 0.0f,  5.0f }, { 0.0f, 1.0f } },
		{ {  3.0f, 0.0f, -5.0f }, { 0.0f, 1.0f } },
		{ {  3.0f, 0.0f,  5.0f }, { 0.0f, 1.0f } },
		{ {  4.0f, 0.0f, -5.0f }, { 0.0f, 1.0f } },
		{ {  4.0f, 0.0f,  5.0f }, { 0.0f, 1.0f } },
		{ {  5.0f, 0.0f, -5.0f }, { 0.0f, 1.0f } },
		{ {  5.0f, 0.0f,  5.0f }, { 0.0f, 1.0f } },

		// X
		{ { -5.0f, 0.0f, -5.0f }, { 1.0f, 0.0f } },
		{ { 5.0f, 0.0f,  -5.0f }, { 1.0f, 0.0f } },
		{ { -5.0f, 0.0f, -4.0f }, { 1.0f, 0.0f } },
		{ { 5.0f, 0.0f,  -4.0f }, { 1.0f, 0.0f } },
		{ { -5.0f, 0.0f, -3.0f }, { 1.0f, 0.0f } },
		{ { 5.0f, 0.0f,  -3.0f }, { 1.0f, 0.0f } },
		{ { -5.0f, 0.0f, -2.0f }, { 1.0f, 0.0f } },
		{ { 5.0f, 0.0f,  -2.0f }, { 1.0f, 0.0f } },
		{ { -5.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } },
		{ { 5.0f, 0.0f,  -1.0f }, { 1.0f, 0.0f } },
		{ { -5.0f, 0.0f,  0.0f }, { 1.0f, 0.0f } },
		{ { 5.0f, 0.0f,   0.0f }, { 1.0f, 0.0f } },
		{ { -5.0f, 0.0f,  1.0f }, { 1.0f, 0.0f } },
		{ { 5.0f, 0.0f,   1.0f }, { 1.0f, 0.0f } },
		{ { -5.0f, 0.0f,  2.0f }, { 1.0f, 0.0f } },
		{ { 5.0f, 0.0f,   2.0f }, { 1.0f, 0.0f } },
		{ { -5.0f, 0.0f,  3.0f }, { 1.0f, 0.0f } },
		{ { 5.0f, 0.0f,   3.0f }, { 1.0f, 0.0f } },
		{ { -5.0f, 0.0f,  4.0f }, { 1.0f, 0.0f } },
		{ { 5.0f, 0.0f,   4.0f }, { 1.0f, 0.0f } },
		{ { -5.0f, 0.0f,  5.0f }, { 1.0f, 0.0f } },
		{ { 5.0f, 0.0f,   5.0f }, { 1.0f, 0.0f } },
	};

	D3D11_SUBRESOURCE_DATA vData;
	vData.pSysMem = gridVerts;

	D3D_CALL(device->CreateBuffer(&CD3D11_BUFFER_DESC(sizeof(gridVerts), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE), &vData, &vBuffer));
}
