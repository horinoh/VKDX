#include "stdafx.h"

#define CV_VERSION "310"
#ifdef _DEBUG
#define CV_CONFIGURATION "d"
#else
#define CV_CONFIGURATION ""
#endif
#define CV_SUFFIX CV_VERSION CV_CONFIGURATION ".lib"

#pragma comment(lib, "opencv_calib3d" CV_SUFFIX)
#pragma comment(lib, "opencv_core" CV_SUFFIX)
#pragma comment(lib, "opencv_features2d" CV_SUFFIX)
#pragma comment(lib, "opencv_flann" CV_SUFFIX)
#pragma comment(lib, "opencv_highgui" CV_SUFFIX)
#pragma comment(lib, "opencv_imgcodecs" CV_SUFFIX)
#pragma comment(lib, "opencv_imgproc" CV_SUFFIX)
#pragma comment(lib, "opencv_ml" CV_SUFFIX)
#pragma comment(lib, "opencv_objdetect" CV_SUFFIX)
#pragma comment(lib, "opencv_photo" CV_SUFFIX)
#pragma comment(lib, "opencv_shape" CV_SUFFIX)
#pragma comment(lib, "opencv_stitching" CV_SUFFIX)
#pragma comment(lib, "opencv_superres" CV_SUFFIX)
#pragma comment(lib, "opencv_ts" CV_SUFFIX)
#pragma comment(lib, "opencv_video" CV_SUFFIX)
#pragma comment(lib, "opencv_videoio" CV_SUFFIX)
#pragma comment(lib, "opencv_videostab" CV_SUFFIX)
