#include "stdafx.h"

#include "VKCompile.h"

#ifdef _DEBUG
#pragma comment(lib, "../glslang/SPIRV/Debug/SPIRVd.lib")
#pragma comment(lib, "../glslang/glslang/Debug/glslangd.lib")
#pragma comment(lib, "../glslang/OGLCompilersDLL/Debug/OGLCompilerd.lib")
#pragma comment(lib, "../glslang/glslang/OSDependent/Windows/Debug/OSDependentd.lib")
#else
#pragma comment(lib, "../glslang/SPIRV/Release/SPIRV.lib")
#pragma comment(lib, "../glslang/glslang/Release/glslang.lib")
#pragma comment(lib, "../glslang/OGLCompilersDLL/Release/OGLCompiler.lib")
#pragma comment(lib, "../glslang/glslang/OSDependent/Windows/Release/OSDependent.lib")
#endif //!< _DEBUG
/**
const auto Stage = EShLangVertex;
auto Shader = new glslang::TShader(Stage);
const char* ShaderStrings[1];
Shader->setStrings(ShaderStrings, sizeof(ShaderStrings));
TBuiltInResource Resources;
//InitailizeResource(Resources);
const auto Message = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
if (Shader->parse(&Resources, 100, false, Message)) {
auto Program = new glslang::TProgram();
Program->addShader(Shader);
if (Program->link(Message)) {
std::vector<unsigned int> Spirv;
glslang::GlslangToSpv(*Program->getIntermediate(Stage), Spirv);
VkShaderModuleCreateInfo ShaderModuleCreateInfo;
ShaderModuleCreateInfo.pCode = Spirv.data();
ShaderModuleCreateInfo.codeSize = Spirv.size() * sizeof(Spirv[0]);
}
delete Program;
}
delete Shader;
*/

