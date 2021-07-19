#pragma once

#include <iostream>
#include <vector>
#include <numbers>

//#define USE_HOLO //!< HoloDX, HoloVK

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
		int Size = 3360;
		int Column = 8, Row = 6;
	};

	Holo() {
#ifdef USE_HOLO
		if (hpc_CLIERR_NOERROR == hpc_InitializeApp("HoloPlayTest", hpc_LICENSE_NONCOMMERCIAL)) {
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

			std::cout << "---- hpc_LightfieldVertShaderGLSL ------------" << std::endl;
			std::cout << hpc_LightfieldVertShaderGLSL << std::endl;
			std::cout << "---- hpc_LightfieldFragShaderGLSL ------------" << std::endl;
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
					//<! currently one of "standard", "large", "pro", "8k"
					std::vector<char> Buf(hpc_GetDeviceType(i, nullptr, 0));
					hpc_GetDeviceType(i, data(Buf), size(Buf));
					std::cout << "hpc_GetDeviceType = " << data(Buf) << std::endl;
				}

				//!< ウインドウ情報
				std::cout << "hpc_GetDevicePropertyWinX, Y = " << hpc_GetDevicePropertyWinX(i) << ", " << hpc_GetDevicePropertyWinY(i) << std::endl;
				std::cout << "hpc_GetDevicePropertyScreenW, H = " << hpc_GetDevicePropertyScreenW(i) << ", " << hpc_GetDevicePropertyScreenH(i) << std::endl;
				//!< ディスプレイのアスペクト比 (W/H)
				std::cout << "hpc_GetDevicePropertyDisplayAspect = " << hpc_GetDevicePropertyDisplayAspect(i) << std::endl;
				//!< ディスプレイのサブピクセルサイズ
				std::cout << "hpc_GetDevicePropertySubp = " << hpc_GetDevicePropertySubp(i) << std::endl;

				//!< レンチキュラーサブピクセルの 赤, 青のインデックス
				std::cout << "hpc_GetDevicePropertyRi = " << hpc_GetDevicePropertyRi(i) << std::endl;
				std::cout << "hpc_GetDevicePropertyBi = " << hpc_GetDevicePropertyBi(i) << std::endl;
				if (hpc_GetDevicePropertyRi(i) == 0 && hpc_GetDevicePropertyBi(i) == 2) {
					std::cout << "\t" << "RGB" << std::endl;
				}
				else if (hpc_GetDevicePropertyRi(i) == 2 && hpc_GetDevicePropertyBi(i) == 0) {
					std::cout << "\t" << "BGR" << std::endl;
				}

				//!< レンチキュラーレンズのピッチ
				std::cout << "hpc_GetDevicePropertyPitch = " << hpc_GetDevicePropertyPitch(i) << std::endl;
				//!< レンチキュラーのチルト角度
				std::cout << "hpc_GetDevicePropertyTilt = " << hpc_GetDevicePropertyTilt(i) << std::endl;
				//!< レンチキュラーの中心オフセット
				std::cout << "hpc_GetDevicePropertyCenter = " << hpc_GetDevicePropertyCenter(i) << std::endl;
							
				//!< レンチキュラーシェーダを反転すべきかどうか
				std::cout << "hpc_GetDevicePropertyInvView = " << (hpc_GetDevicePropertyInvView(i) ? "true" : "false") << std::endl;
				
				//!< Display fringe correction uniform (currently only applicable to 15.6" Developer/Pro units)
				std::cout << "hpc_GetDevicePropertyFringe = " << hpc_GetDevicePropertyFringe(i) << std::endl;

				//!< 基本的に libHoloPlayCore.h の内容は理解する必要はないと書いてあるが、サンプルでは使っている…
#pragma region libHoloPlayCore
				//!< ビューの取りうる範囲 [-viewCone * 0.5f, viewCone * 0.5f]
				std::cout << "hpc_GetDevicePropertyFloat(viewCone) = " << hpc_GetDevicePropertyFloat(i, "/calibration/viewCone/value") << std::endl;
#pragma endregion
			}

			//!< キルト設定
			{
				//Looking Glass Portrait : 8 columns by 6 rows, 3360 by 3360
				//Looking Glass 15.6": 5 columns by 9 rows, 4096 by 4096
				//Looking Glass 8K : 5 columns by 9 rows, 8192 by 8192

				//!< [ DX, VK ] 一度に描画できるビュー
				//!< DX : D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE 16
				//!< VK : VkPhysicalDeviceProperties.limits.maxViewports = 16
				
				//!< 必要なビュー数
				//!< [ Portrait ] 
				//!< 3360 x 3360, column x row = 8 x 6 = 48views
				//! [ 15.6 ]
				//!< 4096 x 4096, column x row = 5 x 9 = 45views
				//! [ 8K ]
				//!< 8192 x 8192, column x row = 5 x 9 = 45views
			}
		}

		if(0 < hpc_GetNumDevices()) {
			//!< ここでは最初のデバイスを選択 (Select 1st device here)
			DeviceIndex = 0;
			{
				std::vector<char> Buf(hpc_GetDeviceType(GetDeviceIndex(), nullptr, 0));
				//!< currently one of "standard", "large", "pro", "8k"
				hpc_GetDeviceType(GetDeviceIndex(), data(Buf), size(Buf));
				std::string_view TypeStr(data(Buf));

				if ("standard" == TypeStr) {
					QuiltSetting = QUILT_SETTING({ .Size = 2048, .Column = 4, .Row = 8 });
				}
				else if ("8k" == TypeStr) {
					QuiltSetting = QUILT_SETTING({ .Size = 8192, .Column = 5, .Row = 9 });
				}
				else if ("portrait" == TypeStr) {
					QuiltSetting = QUILT_SETTING({ .Size = 3360, .Column = 8, .Row = 6 });
				}
				else {
					QuiltSetting = QUILT_SETTING({ .Size = 4096, .Column = 5, .Row = 9 });
				}
			}
		}
		else {
			std::cerr << "[ HoloPlay ] Device not found" << std::endl;
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

	void SetWindow([[maybe_unused]] HWND hWnd) {
#ifdef USE_HOLO
		const auto i = GetDeviceIndex();
		if (-1 != i) {
			::SetWindowPos(hWnd, nullptr, hpc_GetDevicePropertyWinX(i), hpc_GetDevicePropertyWinY(i), hpc_GetDevicePropertyScreenW(i), hpc_GetDevicePropertyScreenH(i), SWP_FRAMECHANGED);
			::ShowWindow(hWnd, SW_SHOW);
		}
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
	float GetAspectRatio([[maybe_unused]] const int i) const {
#ifdef USE_HOLO
		if (-1 != i) { return hpc_GetDevicePropertyDisplayAspect(i); }
#endif
		return 1.0f;
	}
	virtual int GetViewportMax() const = 0;

protected:
	int DeviceIndex = -1;
	QUILT_SETTING QuiltSetting;
};