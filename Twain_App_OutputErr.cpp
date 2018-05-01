#include "twain.h"

#include <iostream>

/**
* Display the last windows error messages.
*/

void printWindowsErrorMessage()
{

	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

    std::cerr << "Error: id [" << dw << "] msg [" << lpMsgBuf << "]" << std::endl;

	LocalFree(lpMsgBuf);

}

