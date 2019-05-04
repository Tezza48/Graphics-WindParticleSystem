#define GLFW_EXPOSE_NATIVE_WIN32
#include <cstdio>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <comdef.h>
#include <d3dcompiler.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define PI 3.141592653589793f


#define SafeRelease(x) if (x) { x->Release(); x = nullptr;}
#define D3D_CALL(x) if(FAILED(x)){\
	_com_error err(x);\
	printf("Error: in file %s at line (%d), hr: %s", __FILE__, __LINE__, err.ErrorMessage());\
	\
	}

int width = 1600;
int height = 900;

using namespace glm;

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

	D3D11_TEXTURE2D_DESC dsd;
	dsd.Width = width;
	dsd.Height = height;
	dsd.MipLevels = 1;
	dsd.ArraySize = 1;
	dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsd.SampleDesc.Count = 1;
	dsd.SampleDesc.Quality = 0;
	dsd.Usage = D3D11_USAGE_DEFAULT;
	dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsd.CPUAccessFlags = 0;
	dsd.MiscFlags = 0;

	ID3D11Texture2D* depthStencilBuffer;
	D3D_CALL(device->CreateTexture2D(&dsd, 0, &depthStencilBuffer));
	D3D_CALL(device->CreateDepthStencilView(depthStencilBuffer, 0, &dsv));
	SafeRelease(depthStencilBuffer);

	context->OMSetRenderTargets(1, &rtv, dsv);

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(width);
	vp.Height = static_cast<float>(height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	context->RSSetViewports(1, &vp);

#pragma endregion Create Device and set up render pipeline

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	ID3D11Buffer* quadVBuffer;
	
	D3D11_BUFFER_DESC vbd;
	vbd.ByteWidth = sizeof(float) * 12;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.BindFlags= D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = NULL;
	vbd.MiscFlags = NULL;
	vbd.StructureByteStride = 0;

	float initialData[] =
	{
		-0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.5f, 0.5f, 0.0f
	};

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = initialData;

	D3D_CALL(device->CreateBuffer(&vbd, &data, &quadVBuffer));

#pragma region Shader_Creation
	ID3D11VertexShader* vShader;
	ID3D11PixelShader* pShader;
	ID3DBlob* error;
	ID3DBlob* vsSource;
	ID3DBlob* psSource;

	unsigned int flags1 = 0;
#if DEBUG || _DEBUG
	flags1 |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

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

	ID3D11InputLayout* iLayout;
	D3D11_INPUT_ELEMENT_DESC iaDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D_CALL(device->CreateInputLayout(iaDesc, 1, vsSource->GetBufferPointer(), vsSource->GetBufferSize(), &iLayout));
	SafeRelease(vsSource);

	ID3D11RasterizerState* rs;
	D3D_CALL(device->CreateRasterizerState(&CD3D11_RASTERIZER_DESC(D3D11_DEFAULT), &rs));

	ID3D11DepthStencilState* ds;
	D3D_CALL(device->CreateDepthStencilState(&CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT), &ds));

	context->VSSetShader(vShader, 0, 0);
	context->IASetInputLayout(iLayout);

	context->PSSetShader(pShader, 0, 0);

	context->RSSetState(rs);
	context->OMSetDepthStencilState(ds, 0);
#pragma endregion 

	auto view = lookAtLH(vec3(0.0f, 0.0f, -1.0f), vec3(), vec3(0.0f, 1.0f, 0.0f));
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

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		context->ClearRenderTargetView(rtv, clearColor);

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		UINT stride = sizeof(float) * 3;
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &quadVBuffer, &stride, &offset);

		context->Draw(4, 0);

		// Swap Buffers
		D3D_CALL(swapChain->Present(1, 0));
		SafeRelease(rtv);
		ID3D11Texture2D* backBuffer;
		D3D_CALL(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
		D3D_CALL(device->CreateRenderTargetView(backBuffer, 0, &rtv));
		SafeRelease(backBuffer);

		context->OMSetRenderTargets(1, &rtv, dsv);
	}

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