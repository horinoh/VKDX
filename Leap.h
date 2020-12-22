#pragma once

#include <iostream>
#include <vector>
#include <thread>

#define USE_LEAP //!< LeapDX. LeapVK

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
			}
		}
#endif
	}
	virtual ~Leap() {
#ifdef USE_LEAP
		if (nullptr != LeapDevice) {
			LeapCloseDevice(LeapDevice);
		}
		if (nullptr != LeapConnection) {
			LeapCloseConnection(LeapConnection);
			PollThread.join();
			LeapDestroyConnection(LeapConnection);
		}
#endif
	}

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
					break;
				//!< コネクションが切れたらループを抜ける (Exit loop on connection lost)
				case eLeapEventType_ConnectionLost: return; 
				case eLeapEventType_Device:
				{
					if (eLeapRS_Success == LeapOpenDevice(Msg.device_event->device, &LeapDevice)) {
						//LEAP_DEVICE_INFO LDI;
						//if (eLeapRS_Success == LeapGetDeviceInfo(LeapDevice, &LDI)) {
						//	std::cout << LDI.size << std::endl;
						//}
					}
				}
					break;
				case eLeapEventType_DeviceFailure:
					break;
				case eLeapEventType_Policy:
					break;
				case eLeapEventType_Tracking:
				{
					const auto Tracking = Msg.tracking_event;
					for (uint32_t i = 0; i < Tracking->nHands; ++i) {
						const auto& Hand = Tracking->pHands[i];
						if (eLeapHandType_Right == Hand.type)  {}
						std::cout << Hand.palm.position.x << ", " << Hand.palm.position.y << ", " << Hand.palm.position.z << std::endl;
						for (auto j = 0; j < _countof(Hand.digits); ++j) {
							const auto& Digit = Hand.digits[j];
							for (auto k = 0; k < _countof(Digit.bones); ++k) {
								[[maybe_unused]] const auto& Bone = Digit.bones[k];
							}
						}
					}
				}
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
					break;
				case eLeapEventType_PointMappingChange:
					break;
				case eLeapEventType_LogEvents:
				{
					const auto Logs = Msg.log_events;
					for (uint32_t i = 0; i < Logs->nEvents; ++i) {
						const auto& Log = Logs->events[i];
						Log.severity;
						Log.message;
						Log.timestamp;
					}
				}
					break;
				case eLeapEventType_HeadPose:
					break;
				}
			}
		}
	}

protected:
	LEAP_CONNECTION LeapConnection = nullptr;
	LEAP_DEVICE LeapDevice = nullptr;

	std::thread PollThread;
#endif
};