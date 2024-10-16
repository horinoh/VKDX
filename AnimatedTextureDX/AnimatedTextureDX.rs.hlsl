#if USE_SAMPLER
#define RS "RootFlags(DENY_VERTEX_SHADER_ROOT_ACCESS|DENY_HULL_SHADER_ROOT_ACCESS|DENY_DOMAIN_SHADER_ROOT_ACCESS|DENY_GEOMETRY_SHADER_ROOT_ACCESS),"\
			"DescriptorTable(SRV(t0, space=0), visibility=SHADER_VISIBILITY_PIXEL),"\
			"DescriptorTable(Sampler(s0, space=0), visibility=SHADER_VISIBILITY_PIXEL)"
#else
#define RS "RootFlags(DENY_VERTEX_SHADER_ROOT_ACCESS|DENY_HULL_SHADER_ROOT_ACCESS|DENY_DOMAIN_SHADER_ROOT_ACCESS|DENY_GEOMETRY_SHADER_ROOT_ACCESS),"\
			"DescriptorTable(SRV(t0, space=0), visibility=SHADER_VISIBILITY_PIXEL),"\
			"StaticSampler(s0, space=0, filter=FILTER_MIN_MAG_MIP_LINEAR, maxLOD=1.f, maxAnisotropy=0, comparisonFunc=COMPARISON_NEVER, visibility=SHADER_VISIBILITY_PIXEL)"
#endif
