#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (set=0, binding=0) uniform sampler2DArray LeapMap;
layout (set=0, binding=1) uniform sampler2DArray DistortionMap;
layout (set=0, binding=2) uniform Tracking 
{
    vec4 Hands[2][5][4]; 
};

layout (location = 0) out vec4 OutColor;

//!< 円の範囲内なら1.0f、範囲外なら0.0fを返す (Is inside circle return 1.0f, otherwise 0.0f)
float IsInCircle(const vec2 pos, const float r)
{
    return max(sign(r * r - dot(pos, pos)), 0.0f); 
}

void main()
{
#if 0
    //!< 修正済 Leap イメージの描画 (Draw rectified leap image)
    const float ArrayIndex = 1.0f - step(InTexcoord.x, 0.5f);
    const vec2 UV = vec2(fract(InTexcoord.x * 2.0f), InTexcoord.y);
    
    const vec2 DistortionIndex = texture(DistortionMap, vec3(UV, ArrayIndex)).xy;
    if(any(lessThan(DistortionIndex, vec2(0.0f)))){ discard; }
    if(any(greaterThan(DistortionIndex, vec2(1.0f)))){ discard; }
    OutColor = vec4(texture(LeapMap, vec3(DistortionIndex, ArrayIndex)).rrr, 1.0f);
#elif 0
    //!< ディストーションマップの描画 (Draw distortion map)
    const float ArrayIndex = 1.0f - step(InTexcoord.x, 0.5f);
    const vec2 UV = vec2(fract(InTexcoord.x * 2.0f), InTexcoord.y);
    
    //OutColor = texture(LeapMap, vec3(UV, ArrayIndex));
	OutColor = texture(DistortionMap, vec3(UV, ArrayIndex));
#else
    //!< ハンドトラッキングの描画 (Draw hand tracking)
    const vec4 BGColor = { 0.0f, 0.0f, 0.0f, 1.0f }; //!< 色を加算するので黒であること (Must be black)
    const vec4 Colors[] = { { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }; //!< 右、左手色 (Right, left hand color)
    OutColor = BGColor;
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                OutColor.rgb += mix(BGColor, Colors[i], IsInCircle(Hands[i][j][k].xz - (InTexcoord - 0.5f) * 2.0f, Hands[i][j][k].y * 0.1f)).rgb;
            }
        }
    }
#endif
}