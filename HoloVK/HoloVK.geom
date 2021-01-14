#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InNormal[];

layout (set = 0, binding = 0) uniform TRANSFORM {
	mat4 Projection;
	mat4 View; 
	mat4 World;
};

layout (push_constant) uniform QUILT_DRAW
{
	int ViewIndexOffset;
	int ViewTotal;
	float Aspect;
	float ViewCone;
} QuiltDraw;


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
#if 1
layout (location = 2) out float OutViewIndex;
#endif

layout (triangles, invocations = 16) in;
layout (triangle_strip, max_vertices = 3) out;
void main()
{
    const float ViewIndex = float(gl_InvocationID + QuiltDraw.ViewIndexOffset); //!< ÇQé¸ñ⁄à»ç~ÇÃï`âÊÇ≈ÇÕÅ@ViewIndexOffset ï™ÇÃâ∫ë ÇóöÇ©ÇπÇƒÇ¢ÇÈ

	//!< [ è„ñ ê} ]
	//!             --|--
	//! OffsetRadian /|
	//!             / | CameraDistance
	//!            /  |
	//!		     +----| 
	//!           OffsetX
	const float OffsetRadian = (ViewIndex / (QuiltDraw.ViewTotal - 1) - 0.5f) * QuiltDraw.ViewCone;
	const float OffsetX = CameraDistance * tan(OffsetRadian);

	vec4 Trans = View * vec4(OffsetX, 0.0f, CameraDistance, 1.0f);
	mat4 V = View;
	V[3] = View[0] * Trans.x + View[1] * Trans.y + View[2] * Trans.z + View[3];

	mat4 P = Projection;
	P[2][0] += OffsetX / (CameraSize * QuiltDraw.Aspect);

	const vec3 CamPos = vec3(View[3][0], View[3][1], View[3][2]);
	const mat4 PVW = P * V * World;

	for(int i=0;i<gl_in.length();++i) {
		gl_Position = PVW * gl_in[i].gl_Position;
		OutNormal = mat3(World) * InNormal[i];
		OutViewDirection = CamPos - (World * gl_Position).xyz;
#if 1
		OutViewIndex = ViewIndex / (QuiltDraw.ViewTotal - 1);
#endif
		gl_ViewportIndex = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();	
}
