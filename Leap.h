#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <thread>

#define USE_LEAP //!< LeapDX. LeapVK

#ifdef USE_LEAP
#pragma warning(push)
#pragma warning(disable : 4201)
#include <LeapC.h>
#pragma warning(pop)
#endif

/*        +Y
*          |  -Z
*          |  /
*          | /
* +X ------+------ -X
*         /| 
*        / |
*       /  | 
*     +Z  -Y
*/
class Leap
{
public:
	Leap() {
#ifdef USE_LEAP
		if (eLeapRS_Success == LeapCreateConnection(nullptr, &LeapConnection)) {
			if (eLeapRS_Success == LeapOpenConnection(LeapConnection)) {
				PollThread = std::thread::thread(&Leap::Poll, this);
				//!< IMAGE_EVENT を発行する設定
				LeapSetPolicyFlags(LeapConnection, eLeapPolicyFlag_Images, 0);

				LeapCreateClockRebaser(&ClockRebaser);
			
				//!< アプリタイムを取得
				LARGE_INTEGER AppTime;
				if (QueryPerformanceCounter(&AppTime)) {
					LARGE_INTEGER Freq;
					if (QueryPerformanceFrequency(&Freq)) {
						//!< アプリタイムとLeapタイムを同期しておく
						const auto AppMicroSec = AppTime.QuadPart * 1000 * 1000 / Freq.QuadPart;
						LeapUpdateRebase(ClockRebaser, AppMicroSec, LeapGetNow());
					}
				}
			}
		}
#endif
	}
	virtual ~Leap() {
#ifdef USE_LEAP		
		if (nullptr != LeapConnection) {
			LeapCloseConnection(LeapConnection);
			PollThread.join();
			LeapDestroyConnection(LeapConnection);
		}
#endif
	}

protected:
#ifdef USE_LEAP
	void Poll() {
		while (true) {
			LEAP_CONNECTION_MESSAGE Msg;
			if (eLeapRS_Success == LeapPollConnection(LeapConnection, 1000, &Msg)) {
				switch (Msg.type)
				{
				default: break;
				case eLeapEventType_None: break;
				case eLeapEventType_Connection:
					std::cout << "[ LEAP ] Connection" << std::endl;
					break;
				case eLeapEventType_ConnectionLost: 
					//!< コネクションが切れたらループを抜ける (Exit loop on connection lost)
					return;
				case eLeapEventType_Device:
					break;
				case eLeapEventType_DeviceFailure:
					break;
				case eLeapEventType_Policy:
				{
					std::cout << "[ LEAP ] PolicyEvent = ";
					const auto PolicyEvent = Msg.policy_event;
					if (eLeapPolicyFlag_BackgroundFrames & PolicyEvent->current_policy) { std::cout << "BackgroundFrames | "; }
					if (eLeapPolicyFlag_Images & PolicyEvent->current_policy) { std::cout << "Images | "; }
					if (eLeapPolicyFlag_OptimizeHMD & PolicyEvent->current_policy) { std::cout << "OptimizeHMD | "; }
					if (eLeapPolicyFlag_AllowPauseResume & PolicyEvent->current_policy) { std::cout << "AllowPauseResume | "; }
					if (eLeapPolicyFlag_MapPoints & PolicyEvent->current_policy) { std::cout << "MapPoints | "; }
					std::cout << std::endl;
				}
					break;
				case eLeapEventType_Tracking:
					//!< ここではやらず、アプリのタイミングで行う
					//OnTrackingEvent(Msg.tracking_event);
					break;
				case eLeapEventType_ImageRequestError:
					break;
				case eLeapEventType_ImageComplete:
					break;
				case eLeapEventType_LogEvent:
					break;
				case eLeapEventType_DeviceLost:
					break;
				case eLeapEventType_ConfigResponse:
					break;
				case eLeapEventType_ConfigChange:
					break;
				case eLeapEventType_DeviceStatusChange:
					break;
				case eLeapEventType_DroppedFrame:
					break;
				case eLeapEventType_Image:
					OnImageEvent(Msg.image_event);
					break;
				case eLeapEventType_PointMappingChange:
					break;
				case eLeapEventType_LogEvents:
				{
					const auto LogEvents = Msg.log_events;
					for (uint32_t i = 0; i < LogEvents->nEvents; ++i) {
						const auto& Log = LogEvents->events[i];
						switch (Log.severity)
						{
						default: break;
						case eLeapLogSeverity_Unknown: break;
						case eLeapLogSeverity_Critical: std::cout << "[ LEAP ] Critical : " << Log.message << std::endl; break;
						case eLeapLogSeverity_Warning: std::cout << "[ LEAP ] Warning : " << Log.message << std::endl; break;
						//case eLeapLogSeverity_Information: std::cout << "[ LEAP ] Information : " << Log.message << std::endl; break;
						}
						//Log.timestamp;
					}
				}
					break;
				case eLeapEventType_HeadPose:
					break;
				}
			}
			else {
				//!< 正常に処理できなかったらループを抜ける (Exit loop on fail)
				return;
			}
		}
	}
	void InterpolatedTrackingEvent() {
		//!< アプリタイムを取得
		LARGE_INTEGER AppTime;
		if (QueryPerformanceCounter(&AppTime)) {
			LARGE_INTEGER Freq;
			if (QueryPerformanceFrequency(&Freq)) {
				//!< マイクロ秒で扱う
				const auto AppMicroSec = AppTime.QuadPart * 1000 * 1000 / Freq.QuadPart;
				int64_t LeapTime;
				//!< アプリタイムをLeapタイムに変換
				LeapRebaseClock(ClockRebaser, AppMicroSec, &LeapTime);
				//!< アプリタイムとLeapタイムを同期しておく
				LeapUpdateRebase(ClockRebaser, AppMicroSec, LeapGetNow());

				//!< トラッキングイベント取得に必要なサイズ
				uint64_t Size;
				if (eLeapRS_Success == LeapGetFrameSize(LeapConnection, LeapTime, &Size)) {
					std::vector<std::byte> Buffer(Size);
					auto TrackingEvent = reinterpret_cast<LEAP_TRACKING_EVENT*>(data(Buffer));
					//!< Leapタイムにおけるトラッキングイベントを取得する
					if (eLeapRS_Success == LeapInterpolateFrame(LeapConnection, LeapTime, TrackingEvent, Size)) {
						OnTrackingEvent(TrackingEvent);
					}
				}
			}
		}
	}
	virtual void OnHand(const LEAP_HAND& Hand) {
		//std::cout << "grab_angle = " << Hand.grab_angle << std::endl;
		//std::cout << "grab_strength = " << Hand.grab_strength << std::endl;

		//std::cout << "pinch_distance = " << Hand.pinch_distance << std::endl;
		//std::cout << "pinch_strength = " << Hand.pinch_strength << std::endl;

		if (eLeapHandType_Right == Hand.type) {
		}
		else { //!< eLeapHandType_Left
		}

		//!< 手のひら (Plam)
		const auto& Palm = Hand.palm;
		Palm.direction; //LEAP_VECTOR
		Palm.normal; //LEAP_VECTOR
		Palm.orientation; //LEAP_QUATERNION
		Palm.stabilized_position; //LEAP_VECTOR
		Palm.velocity; //LEAP_VECTOR
		Palm.width; //float

		//!< 指[5] (thumb, index, middle, ring, pinky)
		for (auto i = 0; i < _countof(Hand.digits); ++i) {
			const auto& Digit = Hand.digits[i];
			//!< 関節[4] (metacarpal, proximal, intermediate, distal)
			for (auto j = 0; j < _countof(Digit.bones); ++j) {
				const auto & Bone = Digit.bones[j];
				Bone.prev_joint; //LEAP_VECTOR
				Bone.next_joint; //LEAP_VECTOR
				Bone.width; //float
				Bone.rotation; //LEAP_QUATERNION
			}
		}
	}
	virtual void OnTrackingEvent(const LEAP_TRACKING_EVENT* TE) {
		//!< 手 (eLeapHandType_Left, eLeapHandType_Right)
		for (uint32_t i = 0; i < TE->nHands; ++i) {			
			OnHand(TE->pHands[i]);
		}
	}
	virtual void OnImageEvent(const LEAP_IMAGE_EVENT* IE) {
		//!< (回転等により)ディストーションマップが変化したときにこの値が変わる
		const auto MatrixVersion = IE->image[0].matrix_version;
		if (CurrentMatrixVersion != MatrixVersion) {
			CurrentMatrixVersion = MatrixVersion;
			OnMatrixVersionChanged(IE);
		}
		for (uint32_t i = 0; i < _countof(IE->image); ++i) {
			if (!empty(ImageData[i])) {
				std::copy(reinterpret_cast<std::byte*>(IE->image[i].data) + IE->image[i].offset, reinterpret_cast<std::byte*>(IE->image[i].data) + IE->image[i].offset + size(ImageData[i]), data(ImageData[i]));
			}

			//std::cout << "[" << i << "]" << std::endl;

			//const auto& Image = IE->image[i];
			//std::cout << "\toffset = " << Image.offset << std::endl; // [0]0, [1]153600(=640x240x1)

			//const auto& Prop = Image.properties;
			//std::cout << "\tProp.bpp = " << Prop.bpp << std::endl; // 1 (Byte Per Pixel)
			//std::cout << "\tProp.width x height = " << Prop.width << " x " << Prop.height << std::endl; //640x240
			//std::cout << "\tProp.format = " << std::hex << Prop.format << std::dec << std::endl; //eLeapImageFormat_IR
			//std::cout << "\tProp.type = " << Prop.type << std::endl; //eLeapImageType_Default
			////!< 将来用 (Reserved for future use)
			////std::cout << "\tProp.offset = " << Prop.x_offset << ", " << Prop.y_offset << std::endl; //-320, -238
			////std::cout << "\tProp.scale = " << Prop.x_scale << ", " << Prop.y_scale << std::endl; //1, 2
		}
	}
	virtual void OnMatrixVersionChanged(const LEAP_IMAGE_EVENT* IE) {
		for (auto i = 0; i < _countof(IE->image); ++i) {
			ImageProperties[i] = IE->image[i].properties;

			if (empty(ImageData[i])) {
				ImageData[i].resize(ImageProperties[i].width * ImageProperties[i].height * ImageProperties[i].bpp);
			}
			std::copy(reinterpret_cast<std::byte*>(IE->image[i].data) + IE->image[i].offset, reinterpret_cast<std::byte*>(IE->image[i].data) + IE->image[i].offset + size(ImageData[i]), data(ImageData[i]));

			DistortionMatrices[i] = *IE->image[i].distortion_matrix;
		}
	}
	virtual void UpdateLeapImage() {}
	virtual void UpdateDistortionImage() {}

	LEAP_CONNECTION LeapConnection = nullptr;
	LEAP_CLOCK_REBASER ClockRebaser  = nullptr;
	std::thread PollThread;
	uint64_t CurrentMatrixVersion = (std::numeric_limits<uint64_t>::max)();

	std::array<LEAP_IMAGE_PROPERTIES, 2> ImageProperties;
	std::array<std::vector<std::byte>, 2> ImageData;

	std::array<LEAP_DISTORTION_MATRIX, 2> DistortionMatrices;
#endif
};