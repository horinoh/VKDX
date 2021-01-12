struct IN
{
	float3 Position : POSITION;
    float3 Normal : NORMAL;
};

cbuffer Transform : register(b0, space0)
{
    float4x4 Projection;
    float4x4 View;
    float4x4 World; 
    float Aspect;
    float ViewCone;
    int ViewTotal;
    int Dummy;
#if 1
    float4x4 Projections[16];
    float4x4 Views[16];
#endif
};
//! [ ë§ñ ê} ]
//!  / Fov(=rad(14.0)) * 0.5 | CameraSize
//! +------------------------|
//!   CameraDistance
static const float CameraSize = 5.0f;
static const float CameraDistance = -CameraSize / tan(radians(14.0f) * 0.5f);

struct OUT
{
	float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float3 ViewDirection : TEXCOORD0;
    uint Viewport : SV_ViewportArrayIndex;
    float ViewIndex : TEXCOORD1;
};

[instance(16)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;

    const float ViewIndex = float(instanceID); //!< TODO [0, 15]Ç»ÇÃÇ≈ÅAÇQé¸ñ⁄à»ç~ÇÃï`âÊÇ≈ÇÕâ∫ë ÇóöÇ©ÇπÇ»Ç¢Ç∆Ç¢ÇØÇ»Ç¢
    
	//!< [ è„ñ ê} ]
	//!             --|--
	//! OffsetRadian /|
	//!             / | CameraDistance
	//!            /  |
	//!		     +----| 
	//!           OffsetX
    const float OffsetRadian = (ViewIndex / (ViewTotal - 1) - 0.5f) * ViewCone; // ViewCone = 0.698131680, OffsetRadian [-0.349, 0.349]
    const float OffsetX = CameraDistance * tan(OffsetRadian); // tan(OffsetRadian) = [-0.36389566, 0.36389566], OffsetX = [14.8, -14,8]
    
    float4 Trans = mul(View, float4(OffsetX, 0.0f, CameraDistance, 1.0f));
	float4 Tmp = View[0] * Trans.x + View[1] * Trans.y + View[2] * Trans.z + View[3];
    float4x4 V = View;
    V[0][3] = Tmp.x; V[1][3] = Tmp.y; V[2][3] = Tmp.z;
#if 1
    V = Views[instanceID];
#endif
    
    float4x4 P = Projection;
    P[0][2] += OffsetX / (CameraSize * Aspect);
#if 1
    P = Projections[instanceID];
#endif
    
    const float3 CamPos = float3(View[0][3], View[1][3], View[2][3]);
    const float4x4 PVW = mul(mul(P, V), World);
   
	[unroll]
	for (int i = 0; i<3; ++i) {
        Out.Position = mul(PVW, float4(In[i].Position, 1.0f));
        Out.Normal = mul((float3x3) World, In[i].Normal);
        Out.ViewDirection = CamPos - mul(World, Out.Position).xyz;
        Out.Viewport = instanceID;
		//Out.ViewIndex = (OffsetX + 14.8)/(14.8*2);
        Out.ViewIndex = ViewIndex / (ViewTotal - 1);
		stream.Append(Out);
	}
	stream.RestartStrip();
}
