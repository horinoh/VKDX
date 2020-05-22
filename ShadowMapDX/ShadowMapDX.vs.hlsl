struct OUT
{
	float3 Position : POSITION;
	uint InstanceID : TEXCOORD0;
};

OUT main(uint VertexId : SV_VertexID, uint InstanceID : SV_InstanceID)
{
	OUT Out;
	Out.InstanceID = InstanceID;
	return Out;
}
