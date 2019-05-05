#define GLFW_EXPOSE_NATIVE_WIN32

#include <cstdio>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "vendor/stb/stb_image.h"
#include "Utilities.h"
#include "GeometryHelper.h"

#define PI 3.141592653589793f



int width = 1600;
int height = 900;

using namespace glm;

namespace TextureLibrary
{
	const char* LEAF_FRONT = "res/images/leafFront.png";
	const char* LEAF_BACK = "res/images/leafBack.png";
}

struct FrameBuffer
{
	mat4x4 world;
	mat4x4 worldInvTrans;
	mat4x4 worldViewProj;
	vec4 eyePos;
};

int main(int* argc, char** argv)
{
#pragma region GLFW_Initialization
	if (!glfwInit())
	{
		std::puts("Failed to Init GLFW");
		return 0;
	}

	GLFWwindow * window = glfwCreateWindow(width, height, "Particle Demo", nullptr, nullptr);

	if (!window)
	{
		glfwTerminate();
		return 0;
	}
#pragma endregion Init and window creation

#pragma region D3D11_Initialization
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* rtv;
	ID3D11DepthStencilView* dsv;

	unsigned int flags = 0;
#if _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2;
	sd.OutputWindow = glfwGetWin32Window(window);
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	sd.Flags = NULL;

	D3D_FEATURE_LEVEL featureLevel;

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
		flags, NULL, NULL, D3D11_SDK_VERSION, &sd, &swapChain,
		&device, &featureLevel, &context);

	ID3D11Texture2D* backBuffer;
	D3D_CALL(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
	D3D_CALL(device->CreateRenderTargetView(backBuffer, 0, &rtv));
	SafeRelease(backBuffer);

	ID3D11Texture2D* depthStencilBuffer;
	D3D_CALL(device->CreateTexture2D(&CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_D24_UNORM_S8_UINT, width, height, 1, 1, D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, 0, 1, 0, 0), 0, &depthStencilBuffer));
	if (!depthStencilBuffer)
	{
		std::puts("Depth Stencil Buffer was not created\n");
		return 0;
	}
	D3D_CALL(device->CreateDepthStencilView(depthStencilBuffer, 0, &dsv));
	SafeRelease(depthStencilBuffer);

	context->OMSetRenderTargets(1, &rtv, dsv);

	context->RSSetViewports(1, &CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)));

#pragma endregion Create Device and set up render pipeline

	vec4 clearColor { 0.0f, 0.0f, 0.0f, 1.0f };

	ID3D11Buffer* quadVBuffer;
	unsigned int numQuadVerts;
	GeometryHelper::CreateQuadBuffer(device, quadVBuffer, &numQuadVerts, 0.2f);

	ID3D11Buffer* gridVB;
	unsigned int numGridVerts;
	GeometryHelper::CreateGrid(device, gridVB, &numGridVerts);

#pragma region Shader_Creation
	ID3D11VertexShader* vShader;
	ID3D11PixelShader* pShader;

	ID3D11VertexShader* vGridShader;
	ID3D11PixelShader* pGridShader;

	unsigned int flags1 = 0;
#if DEBUG || _DEBUG
	flags1 |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* error;
	ID3DBlob* vsSource;
	ID3DBlob* psSource;

	D3D_CALL(D3DCompileFromFile(L"particleShader.hlsl", 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "p0_VS", "vs_5_0", flags1, 0, &vsSource, &error));
	if (error)
	{
		std::puts(static_cast<char*>(error->GetBufferPointer()));
		SafeRelease(error);
	}

	D3D_CALL(D3DCompileFromFile(L"particleShader.hlsl", 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "p0_PS", "ps_5_0", flags1, 0, &psSource, &error));
	if (error)
	{
		std::puts(static_cast<char*>(error->GetBufferPointer()));
		SafeRelease(error);
	}

	device->CreateVertexShader(vsSource->GetBufferPointer(), vsSource->GetBufferSize(), NULL, &vShader);
	device->CreatePixelShader(psSource->GetBufferPointer(), psSource->GetBufferSize(), NULL, &pShader);
	SafeRelease(psSource);

	// Grid Shader
	D3D_CALL(D3DCompileFromFile(L"gridShader.hlsl", 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "p0_VS", "vs_5_0", flags1, 0, &vsSource, &error));
	if (error)
	{
		std::puts(static_cast<char*>(error->GetBufferPointer()));
		SafeRelease(error);
	}

	D3D_CALL(D3DCompileFromFile(L"gridShader.hlsl", 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "p0_PS", "ps_5_0", flags1, 0, &psSource, &error));
	if (error)
	{
		std::puts(static_cast<char*>(error->GetBufferPointer()));
		SafeRelease(error);
	}

	device->CreateVertexShader(vsSource->GetBufferPointer(), vsSource->GetBufferSize(), NULL, &vGridShader);
	device->CreatePixelShader(psSource->GetBufferPointer(), psSource->GetBufferSize(), NULL, &pGridShader);
	SafeRelease(psSource);

	// Pipeline State Objects
	ID3D11InputLayout* iLayout;
	D3D11_INPUT_ELEMENT_DESC iaDescs[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D_CALL(device->CreateInputLayout(iaDescs, 2, vsSource->GetBufferPointer(), vsSource->GetBufferSize(), &iLayout));
	SafeRelease(vsSource);

	ID3D11RasterizerState* rs;
	D3D_CALL(device->CreateRasterizerState(&CD3D11_RASTERIZER_DESC(D3D11_DEFAULT), &rs));

	ID3D11DepthStencilState* ds;
	D3D_CALL(device->CreateDepthStencilState(&CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT), &ds));

	context->IASetInputLayout(iLayout);

	context->RSSetState(rs);
	context->OMSetDepthStencilState(ds, 0);

	auto view = lookAtLH(vec3(0.0f, 2.0f, -1.0f), vec3(), vec3(0.0f, 1.0f, 0.0f));
	auto proj = perspectiveLH(PI / 2.0f, static_cast<float>(width) / height, 0.01f, 1000.0f);

	FrameBuffer perFrameBuffer =
	{
		identity<mat4x4>(),
		identity<mat4x4>(),
		proj * view,
		vec4(0.0f, 0.0f, -1.0f, 1.0f)
	};

	D3D11_SUBRESOURCE_DATA cbufferData;
	cbufferData.pSysMem = &perFrameBuffer;

	ID3D11Buffer* cbFrameBuffer;
	device->CreateBuffer(&CD3D11_BUFFER_DESC(sizeof(FrameBuffer), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0, 0), &cbufferData, &cbFrameBuffer);

	context->VSSetConstantBuffers(0, 1, &cbFrameBuffer);
#pragma endregion

#pragma region Texture
	ID3D11SamplerState* samplerState;
	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	D3D_CALL(device->CreateSamplerState(&CD3D11_SAMPLER_DESC(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 
			D3D11_TEXTURE_ADDRESS_WRAP, 0.0f, 1, D3D11_COMPARISON_ALWAYS, borderColor, 0, D3D11_FLOAT32_MAX), &samplerState));

	int imgWidth, imgHeight, imgBPP;
	unsigned char * texData = stbi_load(TextureLibrary::LEAF_FRONT, &imgWidth, &imgHeight, &imgBPP, STBI_rgb_alpha);

	ID3D11Texture2D* petalTex;
	D3D_CALL(device->CreateTexture2D(&CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R8G8B8A8_UNORM, imgWidth, imgHeight, 1, 0, 
			D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, D3D11_USAGE_DEFAULT, 0, 1, 0, D3D11_RESOURCE_MISC_GENERATE_MIPS), NULL, &petalTex));
	if (!petalTex)
	{
		std::puts("Texture was not successfully created\n");
		return 0;
	}

	unsigned int rowPitch = (imgWidth * imgBPP) * sizeof(char);
	context->UpdateSubresource(petalTex, 0, NULL, texData, rowPitch, 0);

	ID3D11ShaderResourceView* textureView;
	D3D_CALL(device->CreateShaderResourceView(petalTex, &CD3D11_SHADER_RESOURCE_VIEW_DESC(
		D3D_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM), &textureView));
	if (!textureView)
	{
		std::puts("Texture SRV was not successfully created\n");
		return 0;
	}

	context->GenerateMips(textureView);

	context->PSSetShaderResources(0, 1, &textureView);
	context->PSSetSamplers(0, 1, &samplerState);

	stbi_image_free(texData);

#pragma endregion

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		context->ClearRenderTargetView(rtv, value_ptr(clearColor));


		UINT stride = sizeof(VertexPosTex);
		UINT offset = 0;

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		context->IASetVertexBuffers(0, 1, &quadVBuffer, &stride, &offset);
		context->VSSetShader(vShader, 0, 0);
		context->PSSetShader(pShader, 0, 0);
		context->Draw(numQuadVerts, 0);

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		context->IASetVertexBuffers(0, 1, &gridVB, &stride, &offset);
		context->VSSetShader(vGridShader, 0, 0);
		context->PSSetShader(pGridShader, 0, 0);
		context->Draw(numGridVerts, 0);

		// Swap Buffers
		D3D_CALL(swapChain->Present(1, 0));
		SafeRelease(rtv);
		ID3D11Texture2D* backBuffer;
		D3D_CALL(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
		D3D_CALL(device->CreateRenderTargetView(backBuffer, 0, &rtv));
		SafeRelease(backBuffer);

		context->OMSetRenderTargets(1, &rtv, dsv);
	}

	SafeRelease(petalTex);
	SafeRelease(textureView);
	SafeRelease(samplerState);
	SafeRelease(rs);
	SafeRelease(ds);
	SafeRelease(iLayout);
	SafeRelease(vShader);
	SafeRelease(pShader);
	SafeRelease(quadVBuffer);
	SafeRelease(rtv);
	SafeRelease(dsv);
	SafeRelease(swapChain);
	SafeRelease(context);
	SafeRelease(device);
}
