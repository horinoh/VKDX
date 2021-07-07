#include "FBX.h"

#ifdef _DEBUG
#ifdef FBXSDK_SHARED
#pragma comment(lib, "vs2019\\x64\\debug\\libfbxsdk.lib") //!< Dynamic linking
#else
#pragma comment(lib, "vs2019\\x64\\debug\\libfbxsdk-md.lib") //!< Static linking
//#pragma comment(lib, "vs2019\\x64\\debug\\libfbxsdk-mt.lib") //!< Static linking, multithread
#endif
#else
#ifdef FBXSDK_SHARED
#pragma comment(lib, "vs2019\\x64\\release\\libfbxsdk.lib")
#else
#pragma comment(lib, "vs2019\\x64\\release\\libfbxsdk-md.lib")
//#pragma comment(lib, "vs2019\\x64\\release\\libfbxsdk-mt.lib")
#endif
#endif