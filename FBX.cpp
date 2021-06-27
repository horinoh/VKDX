#ifdef _DEBUG
#pragma comment(lib, "vs2019\\x64\\debug\\libfbxsdk.lib") //!< Dynamic linking
//#pragma comment(lib, "vs2019\\x64\\debug\\libfbxsdk-md.lib") //!< Static linking
//#pragma comment(lib, "vs2019\\x64\\debug\\libfbxsdk-mt.lib") //!< Static linking, multithread
#else
#pragma comment(lib, "vs2019\\x64\\release\\libfbxsdk.lib")
//#pragma comment(lib, "vs2019\\x64\\release\\libfbxsdk-md.lib")
//#pragma comment(lib, "vs2019\\x64\\release\\libfbxsdk-mt.lib")
#endif