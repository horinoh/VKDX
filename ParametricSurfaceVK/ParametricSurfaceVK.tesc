#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//!< USE_SPECIALIZATION_INFO : �p�C�v���C���쐬���ɃV�F�[�_���̒萔�l���㏑���w��ł���(�X�J���l�̂�)
//!< (�㏑�����Ȃ��ꍇ�͂����̃V�F�[�_���萔�ƂȂ邾���Ȃ̂ŁA�V�F�[�_������������K�v�͖���)
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

