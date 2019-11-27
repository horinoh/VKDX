#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InTangent;
layout (location = 1) in vec3 InPosition;
layout (location = 2) in vec3 InNormal;

//!< モーフターゲット1
layout (location = 3) in vec3 InTangent1;
layout (location = 4) in vec3 InPosition1;
layout (location = 5) in vec3 InNormal1;
//!< モーフターゲット2
layout (location = 6) in vec3 InTangent2;
layout (location = 7) in vec3 InPosition2;
layout (location = 8) in vec3 InNormal2;

layout (location = 0) out vec3 OutTangent;
layout (location = 1) out vec3 OutNormal;

const float Scale = 30.0f;

//!< モーフウエイト
const float W1 = 0.5f, W2 = 1.0f - W1;

void main()
{
	OutTangent = InTangent + W1 * InTangent1 + W2 * InTangent2;
	gl_Position = vec4((InPosition + W1 * InPosition1 + W2 * InPosition2) * Scale, 1.0f);
	OutNormal = InNormal + W1 * InNormal1 + W2 * InNormal2;
}
