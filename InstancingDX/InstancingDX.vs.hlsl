struct IN
{
	//!< Per Vertex
	float3 Position : POSITION;
	float4 Color : COLOR;
	
	//!< Per Instance (SV_InstanceID)
	float2 Offset : OFFSET;
	uint InstanceID : SV_InstanceID;
};

struct OUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
}; 

OUT main(IN In)
{
	OUT Out;

	Out.Position = float4(In.Position, 1.0f) + float4(In.Offset, 0.0f, 0.0f);
	Out.Color = In.Color;

	//const float4 Colors[] = { float4(1, 0, 0, 1), float4(0, 1, 0, 1), float4(0, 0, 1, 1), float4(1, 1, 0, 1), float4(0, 1, 1, 1) };
	//Out.Color = Colors[In.InstanceID];

	return Out;
}
