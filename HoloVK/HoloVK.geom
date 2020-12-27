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
// radian(14) = 0.244346097
// tan(0.244346097 * 0.5f) = tan(0.122) = 0.12260891 
// CameraDistance = -40.78
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
	const float OffsetRadian = (ViewIndex / (ViewTotal - 1) - 0.5f) * ViewCone; // ViewCone = 0.698131680, OffsetRadian [-0.349, 0.349]
	const float OffsetX = CameraDistance * tan(OffsetRadian); // tan(OffsetRadian) = [-0.36389566, 0.36389566], OffsetX = [14.8, -14,8]

	vec4 T = View * vec4(OffsetX, 0.0f, CameraDistance, 1.0f);
	mat4 V = View;
#if 1
	//V[3] = View[0] * T.x + View[1] * T.y + View[2] * T.z + View[3];
#else
//	V = View * mat4(1, 0, 0, 0,
//	0, 1, 0, 0,
//	0, 0, 1, 0,
//	T.x, T.y, T.z, 1);
#endif

	mat4 P = Projection;
	P[2][0] += OffsetX / (CameraSize * Aspect);

	const vec3 CamPos = vec3(View[3][0], View[3][1], View[3][2]);
	const mat4 PVW = P * V * World;

	for(int i=0;i<gl_in.length();++i) {
		gl_Position = PVW * gl_in[i].gl_Position;
		OutNormal = mat3(World) * InNormal[i];
		OutViewDirection = CamPos - (World * gl_Position).xyz;
		//OutViewIndex = (OffsetX + 14.8)/(14.8*2);
		OutViewIndex = ViewIndex / (ViewTotal - 1);
		gl_ViewportIndex = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();	
}
