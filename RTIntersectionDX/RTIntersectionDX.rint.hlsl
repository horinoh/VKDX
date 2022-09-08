struct AttrNT
{
    float3 Normal;
    float2 Texcoord;
};

bool Sphere(const float3 Center, const float Radius, out float t)
{
    //!< ���C�̃p�����[�^�\�� Ray = o + d * t ������ o = ObjectRayOrigin(), d = ObjectRayDirection()
    //!< (Ray - Center)^2 = Radius^2
    //!< (d * t + Tmp0)^2 = Radius^2 ������ Tmp0 = (o - Center)
    //!< d^2 * t^2 + 2 * Tmp0 * d * t + Tmp0^2 - Radius^2 = 0
    //!< A * t^2 + B * t + C = 0 ������ A = d^2, B = 2 * Tmp0 * d, C = Tmp0^2 - Radius^2
    //!< ���ʎ� D = B * B - 4.0f * A * C
    //!< ���ʎ� D4 = B2 * B2 - A * C ������ B2 = B / 2
    //!< ���̌��� t = (-B +- sqrt(D)) / 2 * A
    //!< ���̌��� t = (-B2 +- sqrt(D4)) / A

    const float3 Tmp0 = ObjectRayOrigin() - Center;
    const float3 d = ObjectRayDirection();
    
    const float A = dot(d, d);
    const float B2 = dot(Tmp0, d);
    const float C = dot(Tmp0, Tmp0) - Radius * Radius;
    const float D4 = B2 * B2 - A * C;
        
    if (D4 >= 0.0f) {
        const float Sq = sqrt(D4);
        const float Tmp1 = (-B2 - Sq) / A;
        const float Tmp2 = (-B2 + Sq) / A;
        const float t0 = min(Tmp1, Tmp2);
        t = (t0 >= RayTMin() && t0 <= RayTCurrent()) ? t0 : max(Tmp1, Tmp2);
        if (t >= RayTMin() && t <= RayTCurrent()) {
            return true;
        }
    }
    
    return false;
}

//!< �y�C���[�h�֏������݂͂ł��Ȃ��A�A�g���r���[�g�𐶐����đ��V�F�[�_�֋���
[shader("intersection")]
void OnIntersection()
{
    const float3 Center = float3(0.0f, 0.0f, 0.0f);
    const float Radius = 0.5f;
    float t;
    if (Sphere(Center, Radius, t)) {
        AttrNT Attr;

        const float3 Hit = ObjectRayOrigin() + ObjectRayDirection() * t;        
        Attr.Normal = normalize(Hit - Center);
            
        const float PI = 4.0f * atan(1.0f);
        const float PI2 = PI * 2;
        Attr.Texcoord = float2(frac(atan2(Attr.Normal.y, Attr.Normal.x) / PI2), acos(-Attr.Normal.z) / PI);
        
        const uint Kind = 0; //!< �����ł͎g�p���Ȃ��̂� 0
        ReportHit(t, Kind, Attr);
    }
}
