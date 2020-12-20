#pragma once

#include <iostream>
#include <vector>
#include <numbers>

#ifdef USE_HOLO
#include <HoloPlayCore.h>
#include <HoloPlayShaders.h>
#endif

class Holo
{
public:
	struct QUILT_SETTING {
		int GetSize() const { return Size; }
		int GetWidth() const { return GetSize(); }
		int GetHeight() const { return GetSize(); }
		int GetViewColumn() const { return Column; }
		int GetViewRow() const { return Row; }
		int GetViewWidth() const { return GetWidth() / GetViewColumn(); }
		int GetViewHeight() const { return GetHeight() / GetViewRow(); }
		int GetViewTotal() const { return GetViewColumn() * GetViewRow(); }
		int Size = 4096;
		//int Column = 5, Row = 9;
		int Column = 4, Row = 4;
	};

	Holo() {
#ifdef USE_HOLO
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

			//std::cout << "----------------------------------------------" << std::endl;
			//std::cout << hpc_LightfieldVertShaderGLSL << std::endl;
			//std::cout << "----------------------------------------------" << std::endl;
			//std::cout << hpc_LightfieldFragShaderGLSL << std::endl;
			//std::cout << "----------------------------------------------" << std::endl;

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

		//!< ここでは最初のデバイスを選択 (Select 1st device here)
		if(0 < hpc_GetNumDevices()) {
			DeviceIndex = 0;

			{
				std::vector<char> Buf(hpc_GetDeviceType(GetDeviceIndex(), nullptr, 0));
				hpc_GetDeviceType(GetDeviceIndex(), data(Buf), size(Buf));
				std::string_view TypeStr(data(Buf));

				if ("standard" == TypeStr) {
					QuiltSetting = QUILT_SETTING({ .Size = 2048, .Column = 4, .Row = 8 });
				}
				else if ("8k" == TypeStr) {
					QuiltSetting = QUILT_SETTING({ .Size = 8192, .Column = 5, .Row = 9 });
				}
				else {
					QuiltSetting = QUILT_SETTING({ .Size = 4096, .Column = 5, .Row = 9 });
				}
			}
		}

		std::cout << "DeviceIndex = " << GetDeviceIndex() << std::endl;
		std::cout << "QuiltSettings = " << GetQuiltSetting().GetWidth() << " x " << GetQuiltSetting().GetHeight() << " (" << GetQuiltSetting().GetViewColumn() << " x " << GetQuiltSetting().GetViewRow() << " = " << GetQuiltSetting().GetViewTotal() << ")" << std::endl;
#endif
	}
	virtual ~Holo() {
#ifdef USE_HOLO
		hpc_CloseApp(); 
#endif
	}

	int GetDeviceIndex() const { return DeviceIndex; }
	const QUILT_SETTING& GetQuiltSetting() const { return QuiltSetting; }

	float GetViewCone([[maybe_unused]] const int i) const {
#ifdef USE_HOLO
		if (-1 != i) { return hpc_GetDevicePropertyFloat(i, "/calibration/viewCone/value"); }
#endif
		return 40.0f;
	}
	float GetRatio([[maybe_unused]] const int i) const {
#ifdef USE_HOLO
		if (-1 != i) { return static_cast<float>(hpc_GetDevicePropertyScreenW(i)) / hpc_GetDevicePropertyScreenH(i); }
#endif
		return 16.0f / 9.0f;
	}
	virtual int GetViewportMax() const = 0;

protected:
	int DeviceIndex = -1;
	QUILT_SETTING QuiltSetting;
};