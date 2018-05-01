// USBTest.cpp : Defines the entry point for the console application.
//


#include <windows.h>
#include <stdio.h>
#include <Shlwapi.h>
#include <conio.h>
extern "C" {
	// Declare the C libraries used
#include <setupapi.h>  // Must link in setupapi.lib
	//#include <hidsdi.h>   // Must link in hid.lib

}

#include <string>
#include <algorithm>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "glog/logging.h"

#include "USBTest.h"


using namespace std;


static /*const*/ GUID GUID_DEVINTERFACE_USB_DEVICE =
{ 0xA5DCBF10L, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };

TCHAR * GetErrString(TCHAR *str, DWORD errcode)
{
	LPVOID lpbuf;
	if (FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpbuf,
		0,
		NULL
		))
	{
		lstrcpy(str, (LPCSTR)lpbuf);
		LocalFree(lpbuf);
	}

	return str;
}


bool CheckScannerVendor(ScannerVendor vendor) {
	char szTraceBuf[256];
	// Get device interface info set handle for all devices attached to system
	HDEVINFO hDevInfo = SetupDiGetClassDevs(
		&GUID_DEVINTERFACE_USB_DEVICE, /* CONST GUID * ClassGuid - USB class GUID */
		NULL, /* PCTSTR Enumerator */
		NULL, /* HWND hwndParent */
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE /* DWORD Flags */
		);

	if (hDevInfo == INVALID_HANDLE_VALUE) {
		sprintf_s(szTraceBuf, "SetupDiClassDevs() failed. GetLastError() " \
			"returns: 0x%x\n", GetLastError());
		OutputDebugStringA(szTraceBuf);
		return false;
	}
	sprintf_s(szTraceBuf, "Device info set handle for all devices attached to " \
		"system: 0x%x\n", hDevInfo);
	OutputDebugStringA(szTraceBuf);
	// Retrieve a context structure for a device interface of a device
	// information set.
	DWORD dwIndex = 0;
	SP_DEVICE_INTERFACE_DATA devInterfaceData;
	ZeroMemory(&devInterfaceData, sizeof(SP_DEVICE_INTERFACE_DATA));
	devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	BOOL bRet = FALSE;
	ULONG  neededLength, requiredLength;
	PSP_DEVICE_INTERFACE_DETAIL_DATA         ClassDeviceData;
	//HIDD_ATTRIBUTES   attributes;

	bool found = false;

	while (TRUE) {
		bRet = SetupDiEnumDeviceInterfaces(
			hDevInfo, /* HDEVINFO DeviceInfoSet */
			NULL, /* PSP_DEVINFO_DATA DeviceInfoData */
			&GUID_DEVINTERFACE_USB_DEVICE, /* CONST GUID * InterfaceClassGuid */
			dwIndex,
			&devInterfaceData /* PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData */
			);
		if (!bRet) {
			TCHAR buffer[1024];
			TCHAR szTraceBuf[1024];
			GetErrString(buffer, GetLastError());

			sprintf_s(szTraceBuf, "SetupDiEnumDeviceInterfaces failed msg:%s", buffer);
			OutputDebugStringA(szTraceBuf);

			if (GetLastError() == ERROR_NO_MORE_ITEMS) {
				break;
			}
		} else {
			SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInterfaceData,
				NULL, 0, &requiredLength, NULL);
			neededLength = requiredLength;
			ClassDeviceData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(neededLength);
			ClassDeviceData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInterfaceData,
				ClassDeviceData, neededLength, &requiredLength, NULL)) {
				free(ClassDeviceData);
				SetupDiDestroyDeviceInfoList(hDevInfo);
				return   false;
			}


			// vid_1083 canon
			// vid_04da panasonic
			// vid_040a kodak
			// vid_04c5 FUJITSU  USB\VID_04C5&PID_1176 ¸»Ê¿Í¨ fi-6670
			ScannerVendor v = OTHER;

			string devpath = ClassDeviceData->DevicePath;

			transform(devpath.begin(), devpath.end(), devpath.begin(), ::tolower);

			if (devpath.find("vid_1083", 0) != string::npos) {  // Canon
				v = CANON;
			} else if (devpath.find("vid_04da", 0) != string::npos) { // Panasonic
				v = PANASONIC;
			} else if (devpath.find("vid_040a", 0) != string::npos ||
				devpath.find("vid_046d", 0) != string::npos ||
				devpath.find("vid_29cc", 0) != string::npos) {
				v = KODAK;
			} else if (devpath.find("vid_04c5", 0) != string::npos) {
				v = FUJITSU;
			}else {
				LOG(ERROR) << "[CheckScannerVendor] fail to recognize scanner: ";//<< devpath;
			}

			//HANDLE handle = CreateFile(ClassDeviceData->DevicePath,
			//	GENERIC_READ | GENERIC_WRITE,
			//	FILE_SHARE_READ | FILE_SHARE_WRITE,
			//	NULL, OPEN_EXISTING, 0, NULL);

			//BOOLEAN retOK = HidD_GetAttributes(handle, &attributes);

			//CloseHandle(handle);
			free(ClassDeviceData);


			if (vendor == v) {
				found = true;
				break;
			}

		}

		dwIndex++;
	}

	sprintf_s(szTraceBuf, "Number of device interface sets representing all " \
		"devices attached to system: %d\n", dwIndex);
	OutputDebugStringA(szTraceBuf);
	SetupDiDestroyDeviceInfoList(hDevInfo);

	return found;
}
