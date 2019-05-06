Texture2D diffuseFrontTex;
Texture2D diffuseBackTex;
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
	float3 position : POSITION0;
	float2 texCoord : TEXCOORD;
	float3 normal: NORMAL;
};

struct VS_Instance_IN
{
	float4x4 world : POSITION1;
};

struct VS_OUT
{
	float4 positionH : SV_POSITION;
	float4 positionW: POSITION;
	float2 texCoord : TEXCOORD;
	float3 normalW : NORMAL;
};

VS_OUT p0_VS(VS_IN i, VS_Instance_IN iInst, uint instanceID : SV_InstanceID)
{
	VS_OUT o;
	i.position = mul(iInst.world, float4(i.position, 1.0)).xyz;

	o.positionH = mul(worldViewProj, float4(i.position, 1.0));
	o.positionW = mul(world, float4(i.position, 1.0));
	o.texCoord = i.texCoord;
	o.normalW = mul((float3x3)worldInvTrans, i.normal);
	return o;
}

float4 p0_PS(VS_OUT i) : SV_TARGET
{
	float3 lightVec = normalize(float3(0.0, -1.0, 0.0));

	float ndotl = abs(dot(i.normalW, -lightVec));

	float4 diffuseColor;

	diffuseColor = diffuseBackTex.Sample(sampleType, i.texCoord).rgba;
	if (diffuseColor.a < 0.5) discard;

	//if (facingAmount > 0.0) diffuseColor = float4(1.0, 0.0, 0.0, 1.0);
	//else diffuseColor = float4(0.0, 1.0, 0.0, 1.0);
	
	//diffuseColor.rgb *= ndotl * 2.0f;

	return diffuseColor;
}