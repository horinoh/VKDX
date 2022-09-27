struct CALLABLEDATA
{
    float3 Data;
};

[shader("callable")]
void OnCallable_1(inout CALLABLEDATA In)
{
	//!< cü
	const float2 pos = float2(DispatchRaysIndex().xy / 8);
	In.Data = float3(fmod(pos.x, 2.0f).xxx);
}