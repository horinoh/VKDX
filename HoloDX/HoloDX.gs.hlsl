struct IN
{
	float3 Position : POSITION;
    float3 Normal : NORMAL;
};

struct TRANSFORM
{
    float4x4 Projection;
    float4x4 View;
    float4x4 World;
};
ConstantBuffer<TRANSFORM> Transform : register(b0, space0);

struct QUILT_DRAW
{
    int ViewIndexOffset;
    int ViewTotal;
    float Aspect;
    float ViewCone;
};
ConstantBuffer<QUILT_DRAW> QuiltDraw : register(b1, space0);

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
#if 1
    float ViewIndex : TEXCOORD1;
#endif
};

[instance(16)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
    
    const float ViewIndex = float(instanceID + QuiltDraw.ViewIndexOffset); //!< ÇQé¸ñ⁄à»ç~ÇÃï`âÊÇ≈ÇÕÅ@ViewIndexOffset ï™ÇÃâ∫ë ÇóöÇ©ÇπÇƒÇ¢ÇÈ
    
	//!< [ è„ñ ê} ]
	//!             --|--
	//! OffsetRadian /|
	//!             / | CameraDistance
	//!            /  |
	//!		     +----| 
	//!           OffsetX
    const float OffsetRadian = (ViewIndex / (QuiltDraw.ViewTotal - 1) - 0.5f) * QuiltDraw.ViewCone; 
    const float OffsetX = CameraDistance * tan(OffsetRadian);
    
    float4 Trans = mul(Transform.View, float4(OffsetX, 0.0f, CameraDistance, 1.0f));
    float4 Tmp = Transform.View[0] * Trans.x + Transform.View[1] * Trans.y + Transform.View[2] * Trans.z + Transform.View[3];
    float4x4 V = Transform.View;
    V[0][3] = Tmp.x; V[1][3] = Tmp.y; V[2][3] = Tmp.z;
    
    float4x4 P = Transform.Projection;
    P[0][2] += OffsetX / (CameraSize * QuiltDraw.Aspect);
    
    const float3 CamPos = float3(Transform.View[0][3], Transform.View[1][3], Transform.View[2][3]);
    const float4x4 PVW = mul(mul(P, V), Transform.World);
   
	[unroll]
	for (int i = 0; i<3; ++i) {
        Out.Position = mul(PVW, float4(In[i].Position, 1.0f));
        Out.Normal = mul((float3x3) Transform.World, In[i].Normal);
        Out.ViewDirection = CamPos - mul(Transform.World, Out.Position).xyz;
        Out.Viewport = instanceID;
#if 1
        Out.ViewIndex = ViewIndex / (QuiltDraw.ViewTotal - 1);
#endif
		stream.Append(Out);
    }
	stream.RestartStrip();
}
