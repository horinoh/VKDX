#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//!< USE_SPECIALIZATION_INFO : パイプライン作成時にシェーダ内の定数値を上書き指定できる(スカラ値のみ)
//!< (上書きしない場合はただのシェーダ内定数となるだけなので、シェーダを書き換える必要は無い)
layout (constant_id = 0) const float TessLevelOuter = 15.0f;
layout (constant_id = 1) const float TessLevelInner = 15.0f;

layout (vertices = 4) out;
void main()
{
	for(int i=0;i<gl_TessLevelOuter.length();++i) {
		gl_TessLevelOuter[i] = TessLevelOuter;
	}
	for(int i=0;i<gl_TessLevelInner.length();++i) {
		gl_TessLevelInner[i] = TessLevelInner;
	}
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}

