#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosition;
layout (location = 1) in vec4 InColor;

layout (location = 0) out vec4 OutColor;

void main()
{
	gl_Position = vec4(InPosition, 1.0f);
	OutColor = InColor;

#if 0
	//!< ピクセル指定
	const vec2 Inv = vec2(2.0f / 1280.0f, 2.0f / 720.0f);
	const mat4 Transform = mat4(Inv.x, 0.0f, 0.0f, -1.0f,
		0.0f, -Inv.y, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0, 0.0f, 0.0f, 1.0f);
	gl_Position = Transform * vec4(InPosition, 1.0f);
#endif
}
