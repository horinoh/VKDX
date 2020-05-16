#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;

layout (set=0, binding=0) uniform sampler2D Sampler2D;

vec2 ToHue(const vec3 Color)
{
	return vec2(dot(vec3(-0.1687f, -0.3312f, 0.5f), Color), dot(vec3(0.5f, -0.4183f, -0.0816f), Color));
}

void main()
{
#if 0
	OutColor = texture(Sampler2D, InTexcoord);
#elif 0
	//!< セピア
	mat3 YCbCr2RGB = { {1.0f, 0.0f, 1.402f}, {1.0f, -0.34414f, -0.71414f}, {1.0f, 1.772f, 0.0f} };
	vec3 YCbCr = vec3(dot(vec3(0.299f, 0.587f, 0.114f), texture(Sampler2D, InTexcoord).rgb), -0.2f, 0.1f);
	OutColor = vec4(YCbCr * YCbCr2RGB, 1.0f);
#elif 0
	//!< モノトーン
	float Mono = dot(vec3(0.299f, 0.587f, 0.114f), texture(Sampler2D, InTexcoord).rgb);
	OutColor = vec4(Mono, Mono, Mono, 1.0f);
#elif 0
	//!< ラスター
	const float PI = 4.0f * atan(1.0f);
	const vec2 UV = InTexcoord + vec2(0.05f * cos(InTexcoord.y * PI * 10.0f), 0.0f);
	OutColor = texture(Sampler2D, UV);
#elif 0
	//!< 輪郭検出
	const vec2 Center = ToHue(texture(Sampler2D, InTexcoord).rgb);
	const vec2 Ndx = ToHue(texture(Sampler2D, InTexcoord + 0.001f * vec2(-1.0f,  0.0f)).rgb) - Center;
	const vec2 Pdx = ToHue(texture(Sampler2D, InTexcoord + 0.001f * vec2( 1.0f,  0.0f)).rgb) - Center;
	const vec2 Ndy = ToHue(texture(Sampler2D, InTexcoord + 0.001f * vec2( 0.0f, -1.0f)).rgb) - Center;
	const vec2 Pdy = ToHue(texture(Sampler2D, InTexcoord + 0.001f * vec2( 0.0f,  1.0f)).rgb) - Center;
	float C = dot(Ndx, Ndx) + dot(Pdx, Pdx) + dot(Ndy, Ndy) + dot(Pdy, Pdy);	
	OutColor = 1.0f - vec4(C, C, C, 1.0f);
#else
	//!< モザイク
	const vec2 Resolution = vec2(800.0f, 600.0f);
	const float Block = 10.0f;
	const vec2 UV = floor(InTexcoord * Resolution / Block) * Block / Resolution;
	OutColor = texture(Sampler2D, UV);
#endif

	
}