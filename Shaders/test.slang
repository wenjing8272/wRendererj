struct VSInput
{
	[[vk::location(0)]]float4 inPosition;
	[[vk::location(1)]]float4 inColor;
}

struct VSOutput
{
	float4 svPosition:SV_Position;
	float4 color;
}

[shader("vertex")]
VSOutput mainVert(VSInput input)
{
	VSOutput output;
	output.svPosition = input.inPosition;
	output.color = input.inColor;
	return output;
}

[shader("fragment")]
float4 mainFrag(VSOutput inFrag):SV_Target
{
	return inFrag.color;
}