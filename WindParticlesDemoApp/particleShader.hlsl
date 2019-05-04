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
};

struct VS_OUT
{
	float4 position: SV_POSITION;
};

VS_OUT p0_VS(VS_IN i)
{
	VS_OUT o;
	o.position = mul(worldViewProj, float4(i.position, 1.0));
	return o;
}

float4 p0_PS(VS_OUT i) : SV_TARGET
{
	return float4(1.0, 1.0, 1.0, 1.0);
}