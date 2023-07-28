#include <mmdeviceapi.h>
#include <endpointvolume.h>

class PeakListener {
private:
	IAudioMeterInformation* pMeterInfo = NULL;
	float peak = 0;
public:
	PeakListener() {
		IMMDeviceEnumerator* pEnumerator = NULL;
		IMMDevice* pDevice = NULL;

		CoInitialize(NULL);
		CoCreateInstance(__uuidof(MMDeviceEnumerator),
			NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IMMDeviceEnumerator),
			(void**)&pEnumerator);
		pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
		pDevice->Activate(__uuidof(IAudioMeterInformation),
			CLSCTX_ALL, NULL, (void**)&pMeterInfo);
	}
	~PeakListener() {
		CoUninitialize();
	}
	float getPeak() {
		pMeterInfo->GetPeakValue(&peak);
		return peak;
	}
};