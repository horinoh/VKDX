#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;
layout (location = 1) in vec3 InPosition;

layout (location = 0) out vec4 OutColor;

layout (set=0, binding=0) uniform sampler2D Sampler2D;

vec2 ToHue(const vec3 Color)
{
	return vec2(dot(vec3(-0.1687f, -0.3312f, 0.5f), Color), dot(vec3(0.5f, -0.4183f, -0.0816f), Color));
}

const float PI = 4.0f * atan(1.0f);
float Gauss(const float x, const float sigma2)
{
	const float coeff = 1.0f / sqrt(2.0f * PI * sigma2);
	const float expon = -(x * x) / (2.0f * sigma2);
    return coeff * exp(expon);
}
#if 0
vec3 GaussianFilter(const sampler2D textureMap, const ivec2 coord, const ivec2 hv)
{
	float weights[8];
	const float sigma2 = 8.0f;
	float sum = 0.0f;
	for(int i=0;i<weights.length();++i) {
		weights[i] = Gauss(i, sigma2);
		sum += sign(i) * 2.0f * weights[i];
	}
	sum = 1.0f / sum;
	for(int i=0;i<weights.length();++i) {
		weights[i] *= sum;
	}
	vec3 color = texelFetch(textureMap, coord, 0).rgb * weights[0];
	for(int i=1;i<weights.length();++i) {
		const ivec2 offset = ivec2(i) * hv; //!< 怒られる
		color += texelFetchOffset(textureMap, coord, 0,  offset).rgb * weights[i];
		color += texelFetchOffset(textureMap, coord, 0, -offset).rgb * weights[i];
	}	
	return color;
}
vec3 GaussianFilterH(const sampler2D textureMap, const ivec2 xy) { return GaussianFilter(textureMap, xy, ivec2(1, 0)); }
vec3 GaussianFilterV(const sampler2D textureMap, const ivec2 xy) { return GaussianFilter(textureMap, xy, ivec2(0, 1)); }
#else
vec3 GaussianFilterH(const sampler2D textureMap, const ivec2 xy) 
{
	//!< ウエイトを毎回求めているので効率は悪い
	float weights[8];
	const float sigma2 = 8.0f; //!< 分散
	float sum = 0.0f;
	for(int i=0;i<weights.length();++i) {
		weights[i] = Gauss(i, sigma2);
		sum += sign(i) * 2.0f * weights[i];
	}
	sum = 1.0f / sum;
	for(int i=0;i<weights.length();++i) {
		weights[i] *= sum;
	}
	vec3 color = texelFetch(textureMap, xy, 0).rgb * weights[0];
	color += texelFetchOffset(textureMap, xy, 0,  ivec2(1, 0)).rgb * weights[1];
	color += texelFetchOffset(textureMap, xy, 0, -ivec2(1, 0)).rgb * weights[1];
	color += texelFetchOffset(textureMap, xy, 0,  ivec2(2, 0)).rgb * weights[2];
	color += texelFetchOffset(textureMap, xy, 0, -ivec2(2, 0)).rgb * weights[2];
	color += texelFetchOffset(textureMap, xy, 0,  ivec2(3, 0)).rgb * weights[3];
	color += texelFetchOffset(textureMap, xy, 0, -ivec2(3, 0)).rgb * weights[3];
	color += texelFetchOffset(textureMap, xy, 0,  ivec2(4, 0)).rgb * weights[4];
	color += texelFetchOffset(textureMap, xy, 0, -ivec2(4, 0)).rgb * weights[4];
	color += texelFetchOffset(textureMap, xy, 0,  ivec2(5, 0)).rgb * weights[5];
	color += texelFetchOffset(textureMap, xy, 0, -ivec2(5, 0)).rgb * weights[5];
	color += texelFetchOffset(textureMap, xy, 0,  ivec2(6, 0)).rgb * weights[6];
	color += texelFetchOffset(textureMap, xy, 0, -ivec2(6, 0)).rgb * weights[6];
	color += texelFetchOffset(textureMap, xy, 0,  ivec2(7, 0)).rgb * weights[7];
	color += texelFetchOffset(textureMap, xy, 0, -ivec2(7, 0)).rgb * weights[7];
	return color;
}
vec3 GaussianFilterV(const sampler2D textureMap, const ivec2 xy)
{
	float weights[8];
	const float sigma2 = 8.0f;
	float sum = 0.0f;
	for(int i=0;i<weights.length();++i) {
		weights[i] = Gauss(i, sigma2);
		sum += sign(i) * 2.0f * weights[i];
	}
	sum = 1.0f / sum;
	for(int i=0;i<weights.length();++i) {
		weights[i] *= sum;
	}
	vec3 color = texelFetch(textureMap, xy, 0).rgb * weights[0];
	color += texelFetchOffset(textureMap, xy, 0,  ivec2(0, 1)).rgb * weights[1];
	color += texelFetchOffset(textureMap, xy, 0, -ivec2(0, 1)).rgb * weights[1];
	color += texelFetchOffset(textureMap, xy, 0,  ivec2(0, 2)).rgb * weights[2];
	color += texelFetchOffset(textureMap, xy, 0, -ivec2(0, 2)).rgb * weights[2];
	color += texelFetchOffset(textureMap, xy, 0,  ivec2(0, 3)).rgb * weights[3];
	color += texelFetchOffset(textureMap, xy, 0, -ivec2(0, 3)).rgb * weights[3];
	color += texelFetchOffset(textureMap, xy, 0,  ivec2(0, 4)).rgb * weights[4];
	color += texelFetchOffset(textureMap, xy, 0, -ivec2(0, 4)).rgb * weights[4];
	color += texelFetchOffset(textureMap, xy, 0,  ivec2(0, 5)).rgb * weights[5];
	color += texelFetchOffset(textureMap, xy, 0, -ivec2(0, 5)).rgb * weights[5];
	color += texelFetchOffset(textureMap, xy, 0,  ivec2(0, 6)).rgb * weights[6];
	color += texelFetchOffset(textureMap, xy, 0, -ivec2(0, 6)).rgb * weights[6];
	color += texelFetchOffset(textureMap, xy, 0,  ivec2(0, 7)).rgb * weights[7];
	color += texelFetchOffset(textureMap, xy, 0, -ivec2(0, 7)).rgb * weights[7];
	return color;
}
#endif

float Hash(float n) { return fract(sin(n) * 43758.5453f); }
float SimplexNoise(vec3 v)
{
	const vec3 p = floor(v);
	vec3 f = fract(v);
	
	f = f * f * (3.0f - 2.0f * f);
	
	const float n = p.x + p.y * 57.0f + 113.0f * p.z;

	return mix(mix(mix(Hash(n + 0.0f), Hash(n + 1.0f), f.x),
		mix(Hash(n + 57.0f), Hash(n + 58.0f), f.x), f.y),
		mix(mix(Hash(n + 113.0f), Hash(n + 114.0f), f.x),
			mix(Hash(n + 170.0f), Hash(n + 171.0f), f.x), f.y), f.z);
}

void main()
{
#if 0
	OutColor = texture(Sampler2D, InTexcoord);
#elif 0
	//!< セピア (Sepia)
	const mat3 YCbCr2RGB = { {1.0f, 0.0f, 1.402f}, {1.0f, -0.34414f, -0.71414f}, {1.0f, 1.772f, 0.0f} };
	const vec3 YCbCr = vec3(dot(vec3(0.299f, 0.587f, 0.114f), texture(Sampler2D, InTexcoord).rgb), -0.2f, 0.1f);
	OutColor = vec4(YCbCr * YCbCr2RGB, 1.0f);
#elif 0
	//!< モノトーン (Monotone)
	const float Mono = dot(vec3(0.299f, 0.587f, 0.114f), texture(Sampler2D, InTexcoord).rgb);
	OutColor = vec4(Mono, Mono, Mono, 1.0f);
#elif 0
	//!< ラスター (Raster)
	const float PI = 4.0f * atan(1.0f);
	const vec2 UV = InTexcoord + vec2(0.05f * cos(InTexcoord.y * PI * 10.0f), 0.0f);
	OutColor = texture(Sampler2D, UV);
#elif 0
	//!< 輪郭検出 (Edge detection)
	const vec2 Center = ToHue(texture(Sampler2D, InTexcoord).rgb);
	//const vec2 Inv = InTexcoord / max(gl_FragCoord.xy, vec2(1.0f, 1.0f)); //!< スクリーンサイズ(gl_FragCoord.xy / InTexcoord)の逆数
	const vec2 Inv = 1.0f / textureSize(Sampler2D, 0);
	const vec2 Ndx = ToHue(texture(Sampler2D, InTexcoord + Inv.x * vec2(-1.0f,  0.0f)).rgb) - Center;
	const vec2 Pdx = ToHue(texture(Sampler2D, InTexcoord + Inv.x * vec2( 1.0f,  0.0f)).rgb) - Center;
	const vec2 Ndy = ToHue(texture(Sampler2D, InTexcoord + Inv.y * vec2( 0.0f, -1.0f)).rgb) - Center;
	const vec2 Pdy = ToHue(texture(Sampler2D, InTexcoord + Inv.y * vec2( 0.0f,  1.0f)).rgb) - Center;
	float C = dot(Ndx, Ndx) + dot(Pdx, Pdx) + dot(Ndy, Ndy) + dot(Pdy, Pdy);
	OutColor = 1.0f - vec4(C, C, C, 0.0f);
#elif 0
	//!< 間輪郭検出 (Edge detection) ... dFdx, dFdy
	const vec2 Center = ToHue(texture(Sampler2D, InTexcoord).rgb);
	const float C = length(dFdx(Center) + dFdy(Center));
	OutColor = 1.0f - vec4(C, C, C, 0.0f);
#elif 0
	//!< ブラー
	const vec2 TexSize = textureSize(Sampler2D, 0);
	const vec2 Offset = 1.5f / TexSize;
	vec3 Color = texture(Sampler2D, InTexcoord).rgb;
	Color += texture(Sampler2D, InTexcoord + vec2( Offset.x, 0.0f)).rgb;
	Color += texture(Sampler2D, InTexcoord + vec2(-Offset.x, 0.0f)).rgb;
	Color += texture(Sampler2D, InTexcoord + vec2(0.0f,  Offset.y)).rgb;
	Color += texture(Sampler2D, InTexcoord + vec2(0.0f, -Offset.y)).rgb;
	Color += texture(Sampler2D, InTexcoord + vec2( Offset.x,  Offset.y)).rgb;
	Color += texture(Sampler2D, InTexcoord + vec2( Offset.x, -Offset.y)).rgb;
	Color += texture(Sampler2D, InTexcoord + vec2(-Offset.x,  Offset.y)).rgb;
	Color += texture(Sampler2D, InTexcoord + vec2(-Offset.x, -Offset.y)).rgb;
	Color /= 9.0f;
	OutColor = vec4(Color, 1.0f);
#elif 0
	//!< ガウスフィルタ (GaussianFilter) ... 本来は2パス必要 (ここでは1パス分だけ)
	OutColor = vec4(GaussianFilterH(Sampler2D, ivec2(gl_FragCoord.xy)), 1.0f);
	//OutColor = vec4(GaussianFilterV(Sampler2D, ivec2(gl_FragCoord.xy)), 1.0f);
#elif 0
	//!< ディザ (Dither)	... シェーダでは関数の再帰呼び出しができないので、計算で求めるのではなく、予めBayerを生成しておく
	const float N = 4.0f;
	const mat4 Bayer = mat4( 0.0f,  8.0f,  2.0f, 10.0f,
							12.0f,  4.0f, 14.0f,  6.0f,
							 3.0f, 11.0f,  1.0f,  9.0f,
							15.0f,  7.0f, 13.0f,  5.0f);
	const vec2 TexSize = textureSize(Sampler2D, 0);
	const ivec2 BayerUV = ivec2(mod(floor(InTexcoord.x * TexSize.x), N), mod(floor(InTexcoord.y * TexSize.y), N));
	const float Threshold = Bayer[BayerUV.x][BayerUV.y] / (N * N);

	const float Mono = dot(vec3(0.299f, 0.587f, 0.114f), texture(Sampler2D, InTexcoord).rgb);
	
	OutColor = vec4((Threshold > Mono).xxx, 1.0f);
#elif 0
	//!< シンプレックスノイズ (SimplexNoise) #VK_TODO
	const vec2 UV = InTexcoord + 0.01f * (SimplexNoise(InPosition) * 2.0f - 1.0f);
	OutColor = texture(Sampler2D, UV);
	//OutColor = vec4(SimplexNoise(InPosition.xyz).rrr, 1.0f);
#else
	//!< モザイク (Mosaic)
	const vec2 Resolution = vec2(800.0f, 600.0f);
	const float Block = 10.0f;
	const vec2 UV = floor(InTexcoord * Resolution / Block) * Block / Resolution;
	OutColor = texture(Sampler2D, UV);
#endif	
}