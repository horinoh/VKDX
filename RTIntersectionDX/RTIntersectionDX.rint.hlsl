//!< �y�C���[�h�֏������݂͂ł��Ȃ��A�A�g���r���[�g�𐶐����đ��V�F�[�_�֋���
[shader("intersection")]
void OnIntersection()
{
    const float3 Center = float3(0.0f, 0.0f, 0.0f);
    const float Radius = 0.25f;
    
    //!< ���C�̃p�����[�^�\�� Ray = o + d * t ������ o = ObjectRayOrigin(), d = ObjectRayDirection()
    //!< (Ray - Center)^2 = Radius^2
    //!< (d * t + Tmp)^2 = Radius^2 ������ Tmp = (o - Center)
    //!< d^2 * t^2 + 2 * Tmp * d * t + Tmp^2 - Radius^2 = 0
    //!< A * t^2 + B * t + C = 0 ������ A = d^2, B = 2 * Tmp * d, C = Tmp^2 - Radius^2
    //!< ���ʎ� D = B * B - 4.0f * A * C
    //!< ���ʎ� D4 = B2 * B2 - A * C ������ B2 = B / 2
    //!< ���̌��� t = (-B +- sqrt(D)) / 2 * A
    //!< ���̌��� t = (-B2 +- sqrt(D4)) / A

    const float3 Tmp = ObjectRayOrigin() - Center;
    const float3 d = ObjectRayDirection();
    
    const float A = dot(d, d);
    const float B = 2.0f * dot(Tmp, d);
    const float B2 = dot(Tmp, d);
    const float C = dot(Tmp, Tmp) - Radius * Radius;
    const float D = B * B - 4.0f * A * C;
    const float D4 = B2 * B2 - A * C;
        
    if (D4 >= 0.0f) {    
        //const float t0 = (-B - sqrt(D)) / (A * 2.0f);
        //const float t1 = (-B + sqrt(D)) / (A * 2.0f);
        const float t0 = (-B2 - sqrt(D4)) / A;
        const float t1 = (-B2 + sqrt(D4)) / A;
        
        //!< �q�b�g�����ꍇ�̂�
        BuiltInTriangleIntersectionAttributes Attr;
        const uint Kind = 0; //!< �����ł͎g�p���Ȃ��̂� 0
        ReportHit(t0, Kind, Attr);
    }
}
