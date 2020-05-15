#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 InTexcoord[];

layout (location = 0) out vec2 OutTexcoord;

layout (triangles, invocations = 4) in;
layout (triangle_strip, max_vertices = 3) out;
void main()
{
	for(int i=0;i<gl_in.length();++i) {
		gl_Position = gl_in[i].gl_Position;
		OutTexcoord = InTexcoord[i];
		//!< 分割画面ではビューポート1以降を使用する
		gl_ViewportIndex = gl_InvocationID + 1;
		EmitVertex();
	}
	EndPrimitive();	
}
