#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InNormal[];

layout (set = 0, binding = 0) uniform Transform {
	mat4 Projection;
	mat4 View; 
	mat4 World;

	float Aspect;
	float ViewCone;
	int ViewTotal;
};
//! [ ë§ñ ê} ]
//!  / Fov(=rad(14.0)) * 0.5 | CameraSize
//! +------------------------|
//!   CameraDistance
const float CameraSize = 5.0f;
const float CameraDistance = -CameraSize / tan(radians(14.0f) * 0.5f);

layout (location = 0) out vec3 OutNormal;
layout (location = 1) out vec3 OutViewDirection;
layout (location = 2) out float OutViewIndex;

layout (triangles, invocations = 16) in;
layout (triangle_strip, max_vertices = 3) out;
void main()
{
    const float ViewIndex = float(gl_InvocationID); //!< TODO [0, 15]Ç»ÇÃÇ≈ÅAÇQé¸ñ⁄à»ç~ÇÃï`âÊÇ≈ÇÕâ∫ë ÇóöÇ©ÇπÇ»Ç¢Ç∆Ç¢ÇØÇ»Ç¢

	//!< [ è„ñ ê} ]
	//!             --|--
	//! OffsetRadian /|
	//!             / | CameraDistance
	//!            /  |
	//!		     +----| 
	//!           OffsetX
	const float OffsetRadian = (ViewIndex / (ViewTotal - 1) - 0.5f) * ViewCone;
	const float OffsetX = CameraDistance * tan(OffsetRadian);

	vec4 T = View * vec4(OffsetX, 0.0f, CameraDistance, 1.0f);
	mat4 V = View;
//	V = View * mat4(1, 0, 0, T.x,
//	0, 1, 0, T.y,
//	0, 0, 1, T.z,
//	0, 0, 0, 1);

	mat4 P = Projection;
	//P[0][2] += OffsetX / (CameraSize * Aspect);

	const vec3 CamPos = vec3(View[3][0], View[3][1], View[3][2]);
	const mat4 PVW = P * V * World;

	for(int i=0;i<gl_in.length();++i) {
		gl_Position = PVW * gl_in[i].gl_Position;
		OutNormal = mat3(World) * InNormal[i];
		OutViewDirection = CamPos - (World * gl_Position).xyz;
		OutViewIndex = ViewIndex / (ViewTotal-1);
		gl_ViewportIndex = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();	
}
