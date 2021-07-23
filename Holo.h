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
		int Size;
		int Column, Row;
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
					std::cout << "hpc_GetDeviceHDMIName = " << data(Buf) << std::endl; //!< 参考値) LKG-PO3996
				}
				{
					std::vector<char> Buf(hpc_GetDeviceSerial(i, nullptr, 0));
					hpc_GetDeviceSerial(i, data(Buf), size(Buf));
					std::cout << "hpc_GetDeviceSerial = " << data(Buf) << std::endl; //!< 参考値) LKG-PORT-03996
				}
				{
					std::vector<char> Buf(hpc_GetDeviceType(i, nullptr, 0));
					hpc_GetDeviceType(i, data(Buf), size(Buf));
					std::cout << "hpc_GetDeviceType = " << data(Buf) << std::endl; //!< 参考値) portrait
				}

				//!< ウインドウ情報
				std::cout << "hpc_GetDevicePropertyWinX, Y = " << hpc_GetDevicePropertyWinX(i) << ", " << hpc_GetDevicePropertyWinY(i) << std::endl; //!< 参考値) 1920, 0
				std::cout << "hpc_GetDevicePropertyScreenW, H = " << hpc_GetDevicePropertyScreenW(i) << ", " << hpc_GetDevicePropertyScreenH(i) << std::endl; //!< 参考値) 1536, 2048
				//!< ディスプレイのアスペクト比 (W/H)
				std::cout << "hpc_GetDevicePropertyDisplayAspect = " << hpc_GetDevicePropertyDisplayAspect(i) << std::endl; //!< 参考値) 0.75f
				//!< ディスプレイのサブピクセルサイズ
				std::cout << "hpc_GetDevicePropertySubp = " << hpc_GetDevicePropertySubp(i) << std::endl; //!< 参考値) 0.000217014f

				//!< レンチキュラーサブピクセルの 赤, 青のインデックス
				std::cout << "hpc_GetDevicePropertyRi = " << hpc_GetDevicePropertyRi(i) << std::endl; //!< 参考値) 0
				std::cout << "hpc_GetDevicePropertyBi = " << hpc_GetDevicePropertyBi(i) << std::endl; //!< 参考値) 2
				if (hpc_GetDevicePropertyRi(i) == 0 && hpc_GetDevicePropertyBi(i) == 2) {
					std::cout << "\t" << "RGB" << std::endl;
				}
				else if (hpc_GetDevicePropertyRi(i) == 2 && hpc_GetDevicePropertyBi(i) == 0) {
					std::cout << "\t" << "BGR" << std::endl;
				}

				//!< レンチキュラーレンズのピッチ
				std::cout << "hpc_GetDevicePropertyPitch = " << hpc_GetDevicePropertyPitch(i) << std::endl; //!< 参考値) 246.866f
				//!< レンチキュラーのチルト角度
				std::cout << "hpc_GetDevicePropertyTilt = " << hpc_GetDevicePropertyTilt(i) << std::endl; //!< 参考値) -0.185377f
				//!< レンチキュラーの中心オフセット
				std::cout << "hpc_GetDevicePropertyCenter = " << hpc_GetDevicePropertyCenter(i) << std::endl; //!< 参考値) 0.565845f

				//!< レンチキュラーシェーダを反転すべきかどうか
				std::cout << "hpc_GetDevicePropertyInvView = " << (hpc_GetDevicePropertyInvView(i) ? "true" : "false") << std::endl; //!< 参考値) 1

				//!< Display fringe correction uniform (currently only applicable to 15.6" Developer/Pro units)
				std::cout << "hpc_GetDevicePropertyFringe = " << hpc_GetDevicePropertyFringe(i) << std::endl; //!< 参考値) 0

				//!< 基本的に libHoloPlayCore.h の内容は理解する必要はないと書いてあるが、サンプルでは使っている…
#pragma region libHoloPlayCore
				//!< ビューの取りうる範囲 [-viewCone * 0.5f, viewCone * 0.5f]
				std::cout << "hpc_GetDevicePropertyFloat(viewCone) = " << hpc_GetDevicePropertyFloat(i, "/calibration/viewCone/value") << std::endl; //!< 参考値) 40
#pragma endregion
			}
		}

		if (0 < hpc_GetNumDevices()) {
			//!< ここでは最初のデバイスを選択 (Select 1st device here)
			DeviceIndex = 0;
			std::cout << "DeviceIndex = " << GetDeviceIndex() << std::endl;

			//!< キルト設定
			//Looking Glass Portrait : 8 columns by 6 rows, 3360 by 3360
			//Looking Glass 15.6": 5 columns by 9 rows, 4096 by 4096
			//Looking Glass 8K : 5 columns by 9 rows, 8192 by 8192
			{
				//!< [ DX, VK ] 一度に描画できるビュー数
				//!< DX : D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE 16
				//!< VK : VkPhysicalDeviceProperties.limits.maxViewports = 16

				//!< 必要なビュー数 (3回くらいは描画することになる)
				//!< [ Portrait ] 
				//!< 3360 x 3360, column x row = 8 x 6 = 48views
				//! [ 15.6 ]
				//!< 4096 x 4096, column x row = 5 x 9 = 45views
				//! [ 8K ]
				//!< 8192 x 8192, column x row = 5 x 9 = 45views

				std::vector<char> Buf(hpc_GetDeviceType(GetDeviceIndex(), nullptr, 0));
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
			std::cout << "QuiltSettings = " << GetQuiltSetting().GetWidth() << " x " << GetQuiltSetting().GetHeight() << " (" << GetQuiltSetting().GetViewColumn() << " x " << GetQuiltSetting().GetViewRow() << " = " << GetQuiltSetting().GetViewTotal() << ")" << std::endl;
		
#pragma region ROOT_CONSTANT, PUSH_CONSTANT
			{
				HoloDraw.Pitch = hpc_GetDevicePropertyPitch(DeviceIndex);
				HoloDraw.Tilt = hpc_GetDevicePropertyTilt(DeviceIndex);
				HoloDraw.Center = hpc_GetDevicePropertyCenter(DeviceIndex);
				HoloDraw.InvView = hpc_GetDevicePropertyInvView(DeviceIndex);
				HoloDraw.Subp = hpc_GetDevicePropertySubp(DeviceIndex);
				HoloDraw.DisplayAspect = hpc_GetDevicePropertyDisplayAspect(DeviceIndex);
				HoloDraw.Ri = hpc_GetDevicePropertyRi(DeviceIndex);
				HoloDraw.Bi = hpc_GetDevicePropertyBi(DeviceIndex);

				HoloDraw.Tile[0] = static_cast<float>(QuiltSetting.GetViewColumn());
				HoloDraw.Tile[1] = static_cast<float>(QuiltSetting.GetViewRow());
				HoloDraw.Tile[2] = static_cast<float>(QuiltSetting.GetViewTotal());
				std::cout << "Tile[] = " << HoloDraw.Tile[0] << ", " << HoloDraw.Tile[1] << ", " << HoloDraw.Tile[2] << std::endl;

				std::cout << "ViewWidth = " << static_cast<float>(QuiltSetting.GetSize()) / QuiltSetting.GetViewColumn() << std::endl;
				std::cout << "ViewHeight = " << static_cast<float>(QuiltSetting.GetSize()) / QuiltSetting.GetViewRow() << std::endl;
				std::cout << "ViewPortion[] = " << static_cast<float>(QuiltSetting.GetSize()) / QuiltSetting.GetViewColumn() * QuiltSetting.GetViewColumn() / QuiltSetting.GetSize() << ", " << static_cast<float>(QuiltSetting.GetSize()) / QuiltSetting.GetViewRow() * QuiltSetting.GetViewRow() / QuiltSetting.GetSize() << std::endl;
				//!< 1.0f にしかならない為、不要な気がする…
				//const auto qs_ViewWidth = static_cast<int>(static_cast<float>(QuiltSetting.GetSize()) / QuiltSetting.GetViewColumn());
				//const auto qs_ViewHeight = static_cast<int>(static_cast<float>(QuiltSetting.GetSize()) / QuiltSetting.GetViewRow());
				//HoloDraw.ViewPortion[0] = static_cast<float>(qs_ViewWidth) * QuiltSetting.GetViewColumn() / QuiltSetting.GetSize(); // == 1.0f
				//HoloDraw.ViewPortion[1] = static_cast<float>(qs_ViewHeight) * QuiltSetting.GetViewRow() / QuiltSetting.GetSize();   // == 1.0f
				//!< DisplayAspect と同じなので不要な気がする
				//HoloDraw.QuiltAspect = hpc_GetDevicePropertyDisplayAspect(i);
				//!< 0 固定の為、不要な気がする
				//HoloDraw.Overscan = 0;
				//HoloDraw.QuiltInvert = 0;
			}
#pragma endregion
		}
		else {
			std::cerr << "[ HoloPlay ] Device not found" << std::endl;
			DefaultSettings();
		}
#else
		DefaultSettings();
#endif
	}
	virtual ~Holo() {
#ifdef USE_HOLO
		hpc_CloseApp(); 
#endif
	}

	void DefaultSettings() {
		QuiltSetting = QUILT_SETTING({ .Size = 3360, .Column = 8, .Row = 6 });
#pragma region ROOT_CONSTANT, PUSH_CONSTANT
		QuiltDraw.ViewIndexOffset = 0;
		QuiltDraw.ViewTotal = GetQuiltSetting().GetViewTotal();
		QuiltDraw.Aspect = 0.75f;
		QuiltDraw.ViewCone = 40.0f * std::numbers::pi_v<float> / 180.0f;

		HoloDraw.Pitch = 246.866f;
		HoloDraw.Tilt = -0.185377f;
		HoloDraw.Center = 0.565845f;
		HoloDraw.InvView = 1;
		HoloDraw.Subp = 0.000217014f;
		HoloDraw.DisplayAspect = 0.75f;
		HoloDraw.Ri = 0;
		HoloDraw.Bi = 2;
		HoloDraw.Tile[0] = static_cast<float>(QuiltSetting.GetViewColumn());
		HoloDraw.Tile[1] = static_cast<float>(QuiltSetting.GetViewRow());
		HoloDraw.Tile[2] = static_cast<float>(QuiltSetting.GetViewTotal());
#pragma endregion
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
	virtual int GetViewportMax() const = 0;

protected:
	int DeviceIndex = -1;
	QUILT_SETTING QuiltSetting;

#pragma region ROOT_CONSTANT, PUSH_CONSTANT
	struct QUILT_DRAW {
		int ViewIndexOffset;
		int ViewTotal;
		float Aspect;
		float ViewCone;
	};
	QUILT_DRAW QuiltDraw;

	struct HOLO_DRAW {
		float Pitch;
		float Tilt;
		float Center;
		int InvView;
		float Subp;
		float DisplayAspect;
		int Ri;
		int Bi;

		float Tile[3];
		//float ViewPortion[2];
		//float QuiltAspect;
		//int Overscan;
		//int QuiltInvert;
	};
	HOLO_DRAW HoloDraw;
#pragma endregion
};