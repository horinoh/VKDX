#pragma once

#include <iostream>
#include <vector>
#include <thread>

//#define USE_LEAP //!< LeapDX. LeapVK

#ifdef USE_LEAP
#pragma warning(push)
#pragma warning(disable : 4201)
#include <LeapC.h>
#pragma warning(pop)
#endif

class Leap
{
public:
	Leap() {
#ifdef USE_LEAP
		if (eLeapRS_Success == LeapCreateConnection(nullptr, &LeapConnection)) {
			if (eLeapRS_Success == LeapOpenConnection(LeapConnection)) {
				PollThread = std::thread::thread(&Leap::Poll, this);
				//!< IMAGE_EVENT �𔭍s����ݒ�
				LeapSetPolicyFlags(LeapConnection, eLeapPolicyFlag_Images, 0);

				LeapCreateClockRebaser(&ClockRebaser);
			
				//!< �A�v���^�C�����擾
				LARGE_INTEGER AppTime;
				if (QueryPerformanceCounter(&AppTime)) {
					LARGE_INTEGER Freq;
					if (QueryPerformanceFrequency(&Freq)) {
						//!< �A�v���^�C����Leap�^�C���𓯊����Ă���
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
		//if (nullptr != LeapDevice) {
		//	LeapCloseDevice(LeapDevice);
		//}
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
					//!< �R�l�N�V�������؂ꂽ�烋�[�v�𔲂��� (Exit loop on connection lost)
					return;
				case eLeapEventType_Device:
					//if (eLeapRS_Success == LeapOpenDevice(Msg.device_event->device, &LeapDevice)) {
					//	std::cout << "[ LEAP ] Device" << std::endl;
					//	//LEAP_DEVICE_INFO LDI;
					//	//if (eLeapRS_Success == LeapGetDeviceInfo(LeapDevice, &LDI)) {
					//	//	std::cout << LDI.size << std::endl;
					//	//}
					//}
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
					//!< �����ł͂�炸�A�A�v���̃^�C�~���O�ōs��
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
				return;
			}
		}
	}
	void InterpolatedTrackingEvent() {
		//!< �A�v���^�C�����擾
		LARGE_INTEGER AppTime;
		if (QueryPerformanceCounter(&AppTime)) {
			LARGE_INTEGER Freq;
			if (QueryPerformanceFrequency(&Freq)) {
				//!< �}�C�N���b�ň���
				const auto AppMicroSec = AppTime.QuadPart * 1000 * 1000 / Freq.QuadPart;
				int64_t LeapTime;
				//!< �A�v���^�C����Leap�^�C���ɕϊ�
				LeapRebaseClock(ClockRebaser, AppMicroSec, &LeapTime);
				//!< �A�v���^�C����Leap�^�C���𓯊����Ă���
				LeapUpdateRebase(ClockRebaser, AppMicroSec, LeapGetNow());

				//!< �g���b�L���O�C�x���g�擾�ɕK�v�ȃT�C�Y
				uint64_t Size;
				if (eLeapRS_Success == LeapGetFrameSize(LeapConnection, LeapTime, &Size)) {
					std::vector<std::byte> Buffer(Size);
					auto TrackingEvent = reinterpret_cast<LEAP_TRACKING_EVENT*>(data(Buffer));
					//!< Leap�^�C���ɂ�����g���b�L���O�C�x���g���擾����
					if (eLeapRS_Success == LeapInterpolateFrame(LeapConnection, LeapTime, TrackingEvent, Size)) {
						OnTrackingEvent(TrackingEvent);
					}
				}
			}
		}
	}
	virtual void OnTrackingEvent(const LEAP_TRACKING_EVENT* TE) {
		//!< �� (eLeapHandType_Left, eLeapHandType_Right)
		for (uint32_t i = 0; i < TE->nHands; ++i) {
			const auto& Hand = TE->pHands[i];
			std::cout << "grab_angle = " << Hand.grab_angle << std::endl;
			std::cout << "grab_strength = " << Hand.grab_strength << std::endl;
			std::cout << "pinch_distance = " << Hand.pinch_distance << std::endl;
			std::cout << "pinch_strength = " << Hand.pinch_strength << std::endl;

			switch (Hand.type) {
			case eLeapHandType_Left:
			{
				//!< ��̂Ђ�
				const auto& Palm = Hand.palm;
				Palm.direction;
				Palm.normal;
				Palm.orientation;
				Palm.position;
				Palm.stabilized_position;
				Palm.velocity;
				Palm.width;

				//!< �w (thumb, index, middle, ring, pinky)
				for (auto j = 0; j < _countof(Hand.digits); ++j) {
					const auto& Digit = Hand.digits[j];
					//!< �֐� (metacarpal, proximal, intermediate, distal)
					for (auto k = 0; k < _countof(Digit.bones); ++k) {
						const auto& Bone = Digit.bones[k];
						Bone.prev_joint;
						Bone.next_joint;
						Bone.width;
						Bone.rotation;
					}
				}
			}
			break;
			case eLeapHandType_Right: break;
			}
		}
	}
	virtual void OnImageEvent(const LEAP_IMAGE_EVENT * IE) {
		for (uint32_t i = 0; i < _countof(IE->image); ++i) {
			//std::cout << "[" << i << "]" << std::endl;

			//const auto& Image = IE->image[i];
			//std::cout << "\toffset = " << Image.offset << std::endl; // [0]0, [1]153600

			//const auto& Prop = Image.properties;
			//std::cout << "\tProp.bpp = " << Prop.bpp << std::endl; //1(Byte Per Pixel)
			//std::cout << "\tProp.width x height = " << Prop.width << " x " << Prop.height << std::endl; //640x240
			//std::cout << "\tProp.format = " << std::hex << Prop.format << std::dec << std::endl; //eLeapImageFormat_IR
			//std::cout << "\tProp.type = " << Prop.type << std::endl; //eLeapImageType_Default
			////!< �����p (Reserved for future use)
			////std::cout << "\tProp.offset = " << Prop.x_offset << ", " << Prop.y_offset << std::endl; //-320, -238
			////std::cout << "\tProp.scale = " << Prop.x_scale << ", " << Prop.y_scale << std::endl; //1, 2

			//!< �f�B�X�g�[�V�����}�b�v
			//!< (��]���ɂ��)�f�B�X�g�[�V�����}�b�v���ω������Ƃ��ɂ��̒l���ς��A�{���͌��o���ăf�B�X�g�[�V�����}�b�v���X�V����A�����ł͊ȒP�̂��ߌŒ舵���Ƃ���
			//Image.matrix_version;
			//std::cout << sizeof(*Image.distortion_matrix) << std::endl; //!< LEAP_DISTORTION_MATRIX_N * LEAP_DISTORTION_MATRIX_N * 2 * sizeof(float)
		}
	}

	LEAP_CONNECTION LeapConnection = nullptr;
	//LEAP_DEVICE LeapDevice = nullptr;
	LEAP_CLOCK_REBASER ClockRebaser  = nullptr;
	std::thread PollThread;
#endif
};