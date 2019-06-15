//!< ƒRƒ“ƒpƒCƒ‹—á C:\Program Files (x86)\Windows Kits\10\bin\10.0.17763.0\x64\fxc.exe /T rootsig_1_1 Sample.rs.hlsl /E RS /Fo Sample.rs.cso
#define RS "RootFlags(0),"\
	"RootConstants(num32BitConstants=1, b0, space=0, visibility=SHADER_VISIBILITY_ALL),"\
	"CBV(b1, space=0, visibility=SHADER_VISIBILITY_ALL, flags=DATA_STATIC_WHILE_SET_AT_EXECUTE),"\
	"SRV(t0, space=0, visibility=SHADER_VISIBILITY_ALL, flags=DATA_STATIC_WHILE_SET_AT_EXECUTE),"\
	"UAV(u0, space=0, visibility=SHADER_VISIBILITY_ALL, flags=DATA_VOLATILE ),"\
	"DescriptorTable("\
		"CBV(b2, space=0, numDescriptors=1,offset=DESCRIPTOR_RANGE_OFFSET_APPEND, flags=DATA_STATIC_WHILE_SET_AT_EXECUTE),"\
		"SRV(t1, space=0, numDescriptors=1,offset=DESCRIPTOR_RANGE_OFFSET_APPEND, flags=DATA_STATIC_WHILE_SET_AT_EXECUTE),"\
		"UAV(u1, space=0, numDescriptors=1,offset=DESCRIPTOR_RANGE_OFFSET_APPEND, flags=0)"\
	"),"\
	"DescriptorTable("\
		"Sampler(s0, space=0, numDescriptors=1,offset=DESCRIPTOR_RANGE_OFFSET_APPEND, flags=0)"\
	"),"\
	"StaticSampler(s2, space=0, filter=FILTER_ANISOTROPIC, addressU=TEXTURE_ADDRESS_WRAP, addressV=TEXTURE_ADDRESS_WRAP, addressW=TEXTURE_ADDRESS_WRAP, mipLODBias=0.f, minLOD=0.f, maxLOD=3.402823466e+38f, maxAnisotropy=16, comparisonFunc=COMPARISON_LESS_EQUAL, borderColor=STATIC_BORDER_COLOR_OPAQUE_WHITE, visibility=SHADER_VISIBILITY_ALL)"