struct CallableDataIn
{
  float3 CallableData;
};

[shader("callable")]
void OnCallable(inout CallableDataIn In)
{
	const float2 pos = float2(DispatchRaysIndex().xy / 8);
	In.CallableData = float3(fmod(pos.x + fmod(pos.y, 2.0f), 2.0f).xxx);
}
