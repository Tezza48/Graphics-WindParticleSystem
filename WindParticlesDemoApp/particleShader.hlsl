Texture2D diffuseFrontTex;
SamplerState sampleType;

cbuffer frameBuffer : register(b0)
{
	float4x4 world;
	float4x4 worldInvTrans;
	float4x4 worldViewProj;
	float4 eyePos;
};

struct VS_IN
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct VS_OUT
{
	float4 position: SV_POSITION;
	float2 texCoord: TEXCOORD;
};

VS_OUT p0_VS(VS_IN i)
{
	VS_OUT o;
	o.position = mul(worldViewProj, float4(i.position, 1.0));
	o.texCoord = i.texCoord;
	return o;
}

float4 p0_PS(VS_OUT i) : SV_TARGET
{
	float4 diffuseColor = diffuseFrontTex.Sample(sampleType, i.texCoord).rgba;
	if (diffuseColor.a < 0.5) discard;

	return diffuseColor;
}