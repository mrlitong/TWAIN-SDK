#include "twain.h"

#include <iostream>
using namespace std;

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

	cerr << "Error: id [" << dw << "] msg [" << lpMsgBuf << "]" << endl;

	LocalFree(lpMsgBuf);
	
}

