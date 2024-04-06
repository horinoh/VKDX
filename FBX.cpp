#include "FBX.h"

#ifdef FBXSDK_SHARED
#pragma comment(lib, "libfbxsdk.lib") //!< Dynamic linking
#else
#pragma comment(lib, "libfbxsdk-md.lib") //!< Static linking
//#pragma comment(lib, "libfbxsdk-mt.lib") //!< Static linking, multithread
#endif