struct IN
{
	float3 Dummy : POSITION;
}; 
struct OUT
{
	float3 Dummy : POSITION;
};

OUT main(IN In)
{
	OUT Out;
	Out.Dummy = In.Dummy;
	return Out;
	//return (OUT)0;
}
