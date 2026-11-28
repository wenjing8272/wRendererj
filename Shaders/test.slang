struct VSInput
{
	[[vk::location(0)]]float3 inPosition;
	[[vk::location(1)]]float3 inNormal;
	[[vk::location(2)]]float2 inUV;
}

struct VSOutput
{
	float3 position;
	float3 normal;
	float2 uv;
}

[shader("vertex")]
VSOutput mainVert(VSInput input)
{
	VSOutput output;
	output.position = input.inPosition,1.0f;
	output.normal = input.inNormal,1.0f;
	output.uv = input.inUV;
	return output;
}

[shader("fragment")]
float4 mainFrag(VSOutput inFrag):SV_Target
{
	float3 n = inFrag.normal;
	return float4(0.5 * n + 0.5,1.0);
}
