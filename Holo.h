#pragma once

#include <iostream>
#include <vector>

#include <HoloPlayCore.h>
#include <HoloPlayShaders.h>

class Holo
{
public:
	Holo() {
		if (hpc_CLIERR_NOERROR == hpc_InitializeApp("TriangleDX.cpp", hpc_LICENSE_NONCOMMERCIAL)) {
			{
				std::vector<char> Buf(hpc_GetStateAsJSON(nullptr, 0));
				hpc_GetStateAsJSON(data(Buf), size(Buf));
				std::cout << "hpc_GetStateAsJSON = " << data(Buf) << std::endl;
			}
			{
				std::vector<char> Buf(hpc_GetHoloPlayCoreVersion(nullptr, 0));
				hpc_GetHoloPlayCoreVersion(data(Buf), size(Buf));
				std::cout << "hpc_GetHoloPlayCoreVersion = " << data(Buf) << std::endl;
			}
			{
				std::vector<char> Buf(hpc_GetHoloPlayServiceVersion(nullptr, 0));
				hpc_GetHoloPlayServiceVersion(data(Buf), size(Buf));
				std::cout << "hpc_GetHoloPlayServiceVersion = " << data(Buf) << std::endl;
			}

			std::cout << "----------------------------------------------" << std::endl;
			std::cout << hpc_LightfieldVertShaderGLSL << std::endl;
			std::cout << "----------------------------------------------" << std::endl;
			std::cout << hpc_LightfieldFragShaderGLSL << std::endl;
			std::cout << "----------------------------------------------" << std::endl;

			std::cout << "hpc_GetNumDevices = " << hpc_GetNumDevices() << std::endl;
			for (auto i = 0; i < hpc_GetNumDevices(); ++i) {
				{
					std::vector<char> Buf(hpc_GetDeviceHDMIName(i, nullptr, 0));
					hpc_GetDeviceHDMIName(i, data(Buf), size(Buf));
					std::cout << "hpc_GetDeviceHDMIName = " << data(Buf) << std::endl;
				}
				{
					std::vector<char> Buf(hpc_GetDeviceSerial(i, nullptr, 0));
					hpc_GetDeviceSerial(i, data(Buf), size(Buf));
					std::cout << "hpc_GetDeviceSerial = " << data(Buf) << std::endl;
				}
				{
					//!< "standard", "large", "pro", "8k"
					std::vector<char> Buf(hpc_GetDeviceType(i, nullptr, 0));
					hpc_GetDeviceType(i, data(Buf), size(Buf));
					std::cout << "hpc_GetDeviceType = " << data(Buf) << std::endl;
				}

				//!< ウインドウ作成時
				std::cout << "hpc_GetDevicePropertyWinX, Y = " << hpc_GetDevicePropertyWinX(i) << ", " << hpc_GetDevicePropertyWinY(i) << std::endl;
				std::cout << "hpc_GetDevicePropertyScreenW, H = " << hpc_GetDevicePropertyScreenW(i) << ", " << hpc_GetDevicePropertyScreenH(i) << std::endl;

				std::cout << "hpc_GetDevicePropertyDisplayAspect = " << hpc_GetDevicePropertyDisplayAspect(i) << std::endl;
				std::cout << "hpc_GetDevicePropertyPitch = " << hpc_GetDevicePropertyPitch(i) << std::endl;
				std::cout << "hpc_GetDevicePropertyTilt = " << hpc_GetDevicePropertyTilt(i) << std::endl;
				std::cout << "hpc_GetDevicePropertyCenter = " << hpc_GetDevicePropertyCenter(i) << std::endl;
				std::cout << "hpc_GetDevicePropertySubp = " << hpc_GetDevicePropertySubp(i) << std::endl;
				std::cout << "hpc_GetDevicePropertyRi, Bi = " << hpc_GetDevicePropertyRi(i) << ", " << hpc_GetDevicePropertyBi(i) << std::endl;
				std::cout << "hpc_GetDevicePropertyInvView = " << hpc_GetDevicePropertyInvView(i) << std::endl;
				
				//!< ビューの取りうる範囲 [-viewCone * 0.5f, viewCone * 0.5f]
				std::cout << "hpc_GetDevicePropertyFloat(viewCone) = " << hpc_GetDevicePropertyFloat(i, "/calibration/viewCone/value") << std::endl;

				//!< サンプルでは使われていない
				std::cout << "hpc_GetDevicePropertyFringe = " << hpc_GetDevicePropertyFringe(i) << std::endl;
			}

			//!< キルト設定
			{
				//!< DX : D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE 16
				//!< VK : VkPhysicalDeviceProperties.limits.maxViewports = 16
				
				//!< Standard
				//!< 2048 x 2048, column x row = 4 x 8 = 32views,

				//! Hires
				//!< 4096 x 4096, column x row = 5 x 9 = 45views,

				//! 8K
				//!< 8192 x 8192, column x row = 5 x 9 = 45views,
			}
		}
	}
	virtual ~Holo() { hpc_CloseApp(); }
};