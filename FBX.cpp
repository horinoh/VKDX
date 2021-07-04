#ifdef _DEBUG
#pragma comment(lib, "vs2019\\x64\\debug\\libfbxsdk.lib") //!< Dynamic linking

//#pragma comment(lib, "vs2019\\x64\\debug\\libfbxsdk-md.lib") //!< Static linking
//#pragma comment(lib, "vs2019\\x64\\debug\\libxml2-md.lib")
//#pragma comment(lib, "vs2019\\x64\\debug\\zlib-md.lib")

//#pragma comment(lib, "vs2019\\x64\\debug\\libfbxsdk-mt.lib") //!< Static linking, multithread
//#pragma comment(lib, "vs2019\\x64\\debug\\libxml2-mt.lib")
//#pragma comment(lib, "vs2019\\x64\\debug\\zlib-mt.lib")

#else
#pragma comment(lib, "vs2019\\x64\\release\\libfbxsdk.lib")

//#pragma comment(lib, "vs2019\\x64\\release\\libfbxsdk-md.lib")
// //#pragma comment(lib, "vs2019\\x64\\release\\libxml2-md.lib")
//#pragma comment(lib, "vs2019\\x64\\release\\zlib-md.lib")

//#pragma comment(lib, "vs2019\\x64\\release\\libfbxsdk-mt.lib")
//#pragma comment(lib, "vs2019\\x64\\release\\libxml2-mt.lib")
//#pragma comment(lib, "vs2019\\x64\\release\\zlib-mt.lib")
#endif