#pragma once

enum ScannerVendor
{
	OTHER,
	CANON,
	PANASONIC,
	FUJITSU,
	KODAK
};

bool CheckScannerVendor(ScannerVendor vendor);

