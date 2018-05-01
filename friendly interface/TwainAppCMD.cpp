

#ifdef _WINDOWS
#include "stdafx.h"
#endif
//#include "mainService.h"

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <assert.h>
#include <stdio.h>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "glog/logging.h"

#include "TwainAppCMD.h"
#include "CTiffWriter.h"
#include "TwainString.h"

#include "../common/ConstantsString.h"
#include "../common/CommonLevelDB.h"
#include "../config/Configuration.h"
#include "../config/OtherScannerConfig.h"
#include "../common/CommonUtils.h"
#include "../scan/ScanManager.h"

#include "USBTest.h"

using namespace std;



//////////////////////////////////////////////////////////////////////////////
TwainAppCMD::TwainAppCMD(HWND parent /*=NULL*/) :TwainApp(parent) {
	m_DsIndex = -1;
}

//////////////////////////////////////////////////////////////////////////////
TwainAppCMD::~TwainAppCMD() {
#ifdef TWNDS_OS_LINUX
	// now destroy the event semaphore
	sem_destroy(&m_TwainEvent);   // Event semaphore handle
#endif
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::fillIdentity(TW_IDENTITY& _identity) {
	TwainApp::fillIdentity(_identity);

	SSTRCPY(_identity.Manufacturer, sizeof(_identity.Manufacturer), "云校");
	SSTRCPY(_identity.ProductFamily, sizeof(_identity.ProductFamily), "云校扫描");
	SSTRCPY(_identity.ProductName, sizeof(_identity.ProductName), "云校扫描客户端 ");

	return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::printIdentStruct(const TW_IDENTITY& _ident) {
	LOG(INFO) << "Id: " << _ident.Id;
	LOG(INFO) << "Version: " << _ident.Version.MajorNum << "." << _ident.Version.MinorNum;
	LOG(INFO) << "SupportedGroups: " << _ident.SupportedGroups;
	LOG(INFO) << "Manufacturer: " << _ident.Manufacturer;
	LOG(INFO) << "ProductFamily: " << _ident.ProductFamily;
	LOG(INFO) << "ProductName: " << _ident.ProductName;

	return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::printIdentityStruct(const TW_UINT32 _identityID) {
	for (unsigned int x = 0; x < m_DataSources.size(); ++x) {
		if (_identityID == m_DataSources[x].Id) {
			printIdentStruct(m_DataSources[x]);
			break;
		}
	}

	return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::printAvailableDataSources() {
	if (m_DSMState < 3) {
		PrintCMDMessage("The DSM has not been opened yet, please open it first\n");
		return;
	}

	// print the Id and name of each available source
	for (unsigned int x = 0; x < m_DataSources.size(); ++x) {
		PrintCMDMessage("%d: %.33s by %.33s\n", m_DataSources[x].Id, m_DataSources[x].ProductName, m_DataSources[x].Manufacturer);
	}

	return;
}

//////////////////////////////////////////////////////////////////////////////
bool findTypeInProducts(const std::string& name, const std::vector<string>& productsName) {
	for (size_t i = 0; i < productsName.size(); ++i) {
		std::string type = CommonUtils::GetProductType(productsName[i]);
		if (name.find(type) != std::string::npos) {
			return true;
		}
	}
	return false;
}
int TwainAppCMD::tryLoadDS(TW_INT32 _dsID) {
	if (_dsID == -1) { // try use previous one
		std::string scanner = CommonLevelDB::getInstance()->getValue("PrevScanner");
		Configuration* config = Configuration::getInstance();
		if (scanner.empty())
			return 1;
		for (size_t i = 0; i < m_DataSources.size(); i++) {
			if (m_DataSources[i].ProductName != scanner)
				continue;
			if (this->setDefaultDataSource(i) && !TwainApp::loadDS()) {
				LOG(INFO) << "[tryLoadDS] try previous done " << scanner;
				CommonLevelDB::getInstance()->putValue("PrevScanner", m_DataSources[i].ProductName);
				// update ... as well
				std::string product = m_pDataSource->ProductName;
				transform(product.begin(), product.end(), product.begin(), ::tolower);

				//#####################################################################################################################
				vector<string> productsName = config->getInstance()->getUserCustomizedDriverConfig().productNames;
				for (size_t i = 0; i < productsName.size(); ++i) {
					transform(productsName[i].begin(), productsName[i].end(), productsName[i].begin(), ::tolower);
				}
				std::string brand, type;
				size_t j = 0;
				for (size_t j = 0; j < productsName.size(); ++j) {
					brand = CommonUtils::GetProductBrand(productsName[j]);
					type = CommonUtils::GetProductType(productsName[j]);
					if (product.find(brand) != std::string::npos) {
						if (findTypeInProducts(product, productsName) == false) {
							current_scanner_type = brand.c_str();
							brand[0] = toupper(brand[0]);
							config->updateProduct(brand);
							return 0;
						}
						else if (product.find(type) != std::string::npos) {
							current_scanner_type = brand.c_str();
							brand[0] = toupper(brand[0]);
							config->updateProduct(brand + ' ' + type);
							return 0;
						}
					}
				}
				if (j == productsName.size()) {
					product = "other";
					LOG(INFO) << "[tryLoadDS] can not recoginaze this scanner." << _dsID;
				}
				//#####################################################################################################################
				return 0;
			}
			else {
				LOG(ERROR) << "[tryLoadDS] fail to use previous: " << scanner;
			}
		}
		return 1;
	}
	else {
		if (this->setDefaultDataSource(_dsID) && !TwainApp::loadDS()) {
			LOG(INFO) << "[tryLoadDS] try done " << _dsID;
			CommonLevelDB::getInstance()->putValue("PrevScanner", m_DataSources[_dsID].ProductName);
			return 0;
		}
		LOG(ERROR) << "[tryLoadDS] FAIL TO TRY " << _dsID;
		return 1;
	}
}

int TwainAppCMD::loadDS(bool isRedPaper) {
	// try to load DS
	int errorCode = 1;
	Configuration* config = Configuration::getInstance();
	if ((errorCode = tryLoadDS(-1)) != 0) { // fail to load previous available ds
		for (unsigned int x = 0; x < m_DataSources.size(); ++x) { // start from 1, skip '0' which stands for default ds
			std::string product = m_DataSources[x].ProductName;
			transform(product.begin(), product.end(), product.begin(), ::tolower);
			current_scanner_type = "other";
			LOG(INFO) << "[loadDS] will try load product " << product;

			//##################################################################################################
			map<string, ScannerVendor> scannerVenderMap;
			scannerVenderMap.insert(pair<string, ScannerVendor>("canon", CANON));
			scannerVenderMap.insert(pair<string, ScannerVendor>("panasonic", PANASONIC));
			scannerVenderMap.insert(pair<string, ScannerVendor>("kodak", KODAK));
			scannerVenderMap.insert(pair<string, ScannerVendor>("fujitsu", FUJITSU));
			vector<string> productsName = config->getInstance()->getUserCustomizedDriverConfig().productNames;
			for (int i = 0; i < productsName.size(); ++i) {
				transform(productsName[i].begin(), productsName[i].end(), productsName[i].begin(), ::tolower);
			}
			std::string brand, type;
			for (size_t i = 0; i < productsName.size(); ++i) {
				brand = CommonUtils::GetProductBrand(productsName[i]);
				type = CommonUtils::GetProductType(productsName[i]);
				if (product.find(brand) != std::string::npos && product.find(type) != std::string::npos) {
					if (CheckScannerVendor(scannerVenderMap[brand])==true) {
						if (!this->tryLoadDS(x)) {
							current_scanner_type = brand.c_str();
							brand[0] = toupper(brand[0]);
							config->updateProduct(brand + ' ' + type);
							errorCode = 0;
						}
					}
				}
				else if (product.find(brand) != std::string::npos && findTypeInProducts(product, productsName) == false) {
					if (CheckScannerVendor(scannerVenderMap[brand])==true) {
						if (!this->tryLoadDS(x)) {
							current_scanner_type = brand.c_str();
							brand[0] = toupper(brand[0]);
							config->updateProduct(brand);
							errorCode = 0;
						}
					}
				}
			}//for(size_t i = 0; i < productsName.size(); ++i)
			//##################################################################################################
		}// for(unsigned int x = 0; x < m_DataSources.size(); ++x)
	}   //if((errorCode = tryLoadDS(-1)) != 0)


	if (errorCode > 0)
		return errorCode;
	if (m_DSMState == 4) {
		initCaps();

		LOG(WARNING) << SCANNER_NAME_TAG << m_pDataSource->ProductName;
		CommonUtils::setScannnerName(m_pDataSource->ProductName);
		string productName = m_pDataSource->ProductName;
		transform(productName.begin(), productName.end(), productName.begin(), ::tolower);

		///////////////////////////////////////////////////////////////////////////////////////
		vector<string> productsName = config->getInstance()->getUserCustomizedDriverConfig().productNames;
		for (int i = 0; i < productsName.size(); ++i) {
			transform(productsName[i].begin(), productsName[i].end(), productsName[i].begin(), ::tolower);
		}
		std::string brand, type;
		for (size_t i = 0; i < productsName.size(); ++i) {
			brand = CommonUtils::GetProductBrand(productsName[i]);
			type = CommonUtils::GetProductType(productsName[i]);
			if (productName.find(brand) != std::string::npos && productName.find(type) != std::string::npos) {
				brand[0] = toupper(brand[0]);
				m_pDriverConfig = Configuration::getInstance()->getDriverConfig(brand + ' ' + type);
				this->InitScanner(brand);
				return 0;
			}
			else if (productName.find(brand) != std::string::npos && findTypeInProducts(productName, productsName) == false) {
				brand[0] = toupper(brand[0]);
				m_pDriverConfig = Configuration::getInstance()->getDriverConfig(brand);
				this->InitScanner(brand);
				return 0;
			}
		}
		///////////////////////////////////////////////////////////////////////////////////////
		LOG(ERROR) << "NO SCANNER FOUNDED";
		return 1;
	}
}
//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::unloadDS() {
	uninitCaps();

	TwainApp::unloadDS();
}
//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::initCaps() {
	if (m_DSMState < 3) {
		PrintCMDMessage("The DSM needs to be opened first.\n");
		return;
	}
	else if (m_DSMState < 4) {
		PrintCMDMessage("A Data Source needs to be opened first.\n");
		return;
	}

	// get the default pixel type
	memset(&m_CAP_XFERCOUNT, 0, sizeof(TW_CAPABILITY));
	m_CAP_XFERCOUNT.Cap = CAP_XFERCOUNT;
	get_CAP(m_CAP_XFERCOUNT);

	memset(&m_ICAP_PIXELTYPE, 0, sizeof(TW_CAPABILITY));
	m_ICAP_PIXELTYPE.Cap = ICAP_PIXELTYPE;
	get_CAP(m_ICAP_PIXELTYPE);

	memset(&m_ICAP_XFERMECH, 0, sizeof(TW_CAPABILITY));
	m_ICAP_XFERMECH.Cap = ICAP_XFERMECH;
	get_CAP(m_ICAP_XFERMECH);

	memset(&m_ICAP_IMAGEFILEFORMAT, 0, sizeof(TW_CAPABILITY));
	m_ICAP_IMAGEFILEFORMAT.Cap = ICAP_IMAGEFILEFORMAT;
	get_CAP(m_ICAP_IMAGEFILEFORMAT);

	memset(&m_ICAP_COMPRESSION, 0, sizeof(TW_CAPABILITY));
	m_ICAP_COMPRESSION.Cap = ICAP_COMPRESSION;
	get_CAP(m_ICAP_COMPRESSION);

	memset(&m_ICAP_UNITS, 0, sizeof(TW_CAPABILITY));
	m_ICAP_UNITS.Cap = ICAP_UNITS;
	get_CAP(m_ICAP_UNITS);

	memset(&m_ICAP_BITDEPTH, 0, sizeof(TW_CAPABILITY));
	m_ICAP_BITDEPTH.Cap = ICAP_BITDEPTH;
	get_CAP(m_ICAP_BITDEPTH);

	memset(&m_ICAP_XRESOLUTION, 0, sizeof(TW_CAPABILITY));
	m_ICAP_XRESOLUTION.Cap = ICAP_XRESOLUTION;
	get_CAP(m_ICAP_XRESOLUTION);

	memset(&m_ICAP_YRESOLUTION, 0, sizeof(TW_CAPABILITY));
	m_ICAP_YRESOLUTION.Cap = ICAP_YRESOLUTION;
	get_CAP(m_ICAP_YRESOLUTION);

	memset(&m_ICAP_FRAMES, 0, sizeof(TW_CAPABILITY));
	m_ICAP_FRAMES.Cap = ICAP_FRAMES;
	get_CAP(m_ICAP_FRAMES);

	return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::uninitCaps() {
	if (m_DSMState < 3) {
		PrintCMDMessage("The DSM needs to be opened first.\n");
		return;
	}
	else if (m_DSMState < 4) {
		PrintCMDMessage("A Data Source needs to be opened first.\n");
		return;
	}

	if (m_CAP_XFERCOUNT.hContainer) {
		_DSM_Free(m_CAP_XFERCOUNT.hContainer);
		m_CAP_XFERCOUNT.hContainer = 0;
	}
	if (m_ICAP_PIXELTYPE.hContainer) {
		_DSM_Free(m_ICAP_PIXELTYPE.hContainer);
		m_ICAP_PIXELTYPE.hContainer = 0;
	}
	if (m_ICAP_XFERMECH.hContainer) {
		_DSM_Free(m_ICAP_XFERMECH.hContainer);
		m_ICAP_XFERMECH.hContainer = 0;
	}
	if (m_ICAP_IMAGEFILEFORMAT.hContainer) {
		_DSM_Free(m_ICAP_IMAGEFILEFORMAT.hContainer);
		m_ICAP_IMAGEFILEFORMAT.hContainer = 0;
	}
	if (m_ICAP_COMPRESSION.hContainer) {
		_DSM_Free(m_ICAP_COMPRESSION.hContainer);
		m_ICAP_COMPRESSION.hContainer = 0;
	}
	if (m_ICAP_UNITS.hContainer) {
		_DSM_Free(m_ICAP_UNITS.hContainer);
		m_ICAP_UNITS.hContainer = 0;
	}
	if (m_ICAP_BITDEPTH.hContainer) {
		_DSM_Free(m_ICAP_BITDEPTH.hContainer);
		m_ICAP_BITDEPTH.hContainer = 0;
	}
	if (m_ICAP_XRESOLUTION.hContainer) {
		_DSM_Free(m_ICAP_XRESOLUTION.hContainer);
		m_ICAP_XRESOLUTION.hContainer = 0;
	}
	if (m_ICAP_YRESOLUTION.hContainer) {
		_DSM_Free(m_ICAP_YRESOLUTION.hContainer);
		m_ICAP_YRESOLUTION.hContainer = 0;
	}
	if (m_ICAP_FRAMES.hContainer) {
		_DSM_Free(m_ICAP_FRAMES.hContainer);
		m_ICAP_FRAMES.hContainer = 0;
	}

	return;
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////


void TwainAppCMD::startScan() {
	if (m_DSMState != 6) {
		printError(m_pDataSource, "A scan cannot be initiated unless we are in state 6");
		return;
	}

	TW_UINT16 mech;
	if (!getICAP_XFERMECH(mech)) {
		printError(m_pDataSource, "Error: could not get the transfer mechanism");
		return;
	}

	m_imageFiles.clear();

	switch (mech)
		//switch (mech + 1)
	{
	case TWSX_NATIVE:
		initiateTransfer_Native();
		break;

	case TWSX_FILE:
	{
		TW_UINT16 fileformat = TWFF_TIFF;
		if (!getICAP_IMAGEFILEFORMAT(fileformat)) {
			// Default back to TIFF
			fileformat = TWFF_TIFF;
		}
		initiateTransfer_File(fileformat);
	}
		break;

	case TWSX_MEMORY:
		initiateTransfer_Memory();
		break;
	}

	return;
}



//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_CAP_DUPLEX(const TW_INT16 _duplex) {
	//set_CapabilityOneValue(CAP_XFERCOUNT, _duplex, TWTY_INT16);

	//// now that we have set it, re-get it to ensure it was set
	//if (TWCC_SUCCESS == get_CAP(m_CAP_XFERCOUNT))
	//{
	//	TW_INT16 count;
	//	if (getCAP_XFERCOUNT(count) &&
	//		count == _duplex)
	//	{
	//		PrintCMDMessage("Capability successfully set!\n");
	//	}
	//}

	return;
}


//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_CAP_XFERCOUNT(const TW_INT16 _count) {
	set_CapabilityOneValue(CAP_XFERCOUNT, _count, TWTY_INT16);

	// now that we have set it, re-get it to ensure it was set
	if (TWCC_SUCCESS == get_CAP(m_CAP_XFERCOUNT)) {
		TW_INT16 count;
		if (getCAP_XFERCOUNT(count) &&
			count == _count) {
			PrintCMDMessage("Capability successfully set!\n");
		}
	}

	return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_UNITS(const TW_UINT16 _val) {
	set_CapabilityOneValue(ICAP_UNITS, _val, TWTY_UINT16);

	// now that we have set it, re-get it to ensure it was set
	if (TWCC_SUCCESS == get_CAP(m_ICAP_UNITS)) {
		if (TWON_ENUMERATION == m_ICAP_UNITS.ConType &&
			0 != m_ICAP_UNITS.hContainer) {
			pTW_ENUMERATION pCapPT = (pTW_ENUMERATION)_DSM_LockMemory(m_ICAP_UNITS.hContainer);

			if (_val == pCapPT->ItemList[pCapPT->CurrentIndex]) {
				PrintCMDMessage("Capability successfully set!\n");

				// successfully setting this cap means that we have to re-obtain the X/Y resolutions as well
				get_CAP(m_ICAP_XRESOLUTION);
				get_CAP(m_ICAP_YRESOLUTION);
			}
			_DSM_UnlockMemory(m_ICAP_UNITS.hContainer);
		}
	}

	return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_PIXELTYPE(const TW_UINT16 _pt) {
	set_CapabilityOneValue(ICAP_PIXELTYPE, _pt, TWTY_UINT16);

	// now that we have set it, re-get it to ensure it was set
	if (TWCC_SUCCESS == get_CAP(m_ICAP_PIXELTYPE)) {
		if (TWON_ENUMERATION == m_ICAP_PIXELTYPE.ConType &&
			0 != m_ICAP_PIXELTYPE.hContainer) {
			pTW_ENUMERATION pCapPT = (pTW_ENUMERATION)_DSM_LockMemory(m_ICAP_PIXELTYPE.hContainer);

			if (_pt == ((TW_UINT16*)(&pCapPT->ItemList))[pCapPT->CurrentIndex]) {
				PrintCMDMessage("Capability successfully set!\n");
			}
			_DSM_UnlockMemory(m_ICAP_PIXELTYPE.hContainer);
		}
	}

	get_CAP(m_ICAP_BITDEPTH);

	return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_RESOLUTION(const TW_UINT16 _ICAP, const pTW_FIX32 _pVal) {
	if ((ICAP_XRESOLUTION != _ICAP) &&
		(ICAP_YRESOLUTION != _ICAP)) {
		printError(m_pDataSource, "Invalid resolution passed in! Resolution set failed.");
		return;
	}

	set_CapabilityOneValue(_ICAP, _pVal);

	// Get the new RESOLUTION caps values to see if the set was successfull.
	get_CAP(m_ICAP_XRESOLUTION);
	get_CAP(m_ICAP_YRESOLUTION);

	pTW_CAPABILITY pCapRes = 0;

	if (ICAP_XRESOLUTION == _ICAP) {
		pCapRes = &m_ICAP_XRESOLUTION;
	}
	else {
		pCapRes = &m_ICAP_YRESOLUTION;
	}

	// check ICAP_XRESOLUTION
	if (TWON_ENUMERATION == pCapRes->ConType &&
		0 != pCapRes->hContainer) {
		pTW_ENUMERATION_FIX32 pdat = (pTW_ENUMERATION_FIX32)pCapRes->hContainer;

		if (TWTY_FIX32 == pdat->ItemType &&
			_pVal->Whole == pdat->ItemList[pdat->CurrentIndex].Whole &&
			_pVal->Frac == pdat->ItemList[pdat->CurrentIndex].Frac) {
			PrintCMDMessage("Resolution successfully set!\n");
		}
	}
	return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_FRAMES(const pTW_FRAME _pFrame) {

	set_CapabilityOneValue(ICAP_FRAMES, _pFrame);

	// now that we have set it, re-get it to ensure it was set
	if (TWCC_SUCCESS == get_CAP(m_ICAP_FRAMES)) {
		if (TWON_ENUMERATION == m_ICAP_FRAMES.ConType &&
			0 != m_ICAP_FRAMES.hContainer) {
			pTW_ENUMERATION_FRAME pCapPT = (pTW_ENUMERATION_FRAME)_DSM_LockMemory(m_ICAP_FRAMES.hContainer);

			pTW_FRAME ptframe = &pCapPT->ItemList[pCapPT->CurrentIndex];

			if ((_pFrame->Bottom == ptframe->Bottom) &&
				(_pFrame->Top == ptframe->Top) &&
				(_pFrame->Left == ptframe->Left) &&
				(_pFrame->Right == ptframe->Right)) {
				PrintCMDMessage("Frames successfully set!\n");
			}
			_DSM_UnlockMemory(m_ICAP_FRAMES.hContainer);
		}
	}

	return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_XFERMECH(const TW_UINT16 _mech) {
	set_CapabilityOneValue(ICAP_XFERMECH, _mech, TWTY_UINT16);

	// now that we have set it, re-get it to ensure it was set
	if (TWCC_SUCCESS == get_CAP(m_ICAP_XFERMECH)) {
		TW_UINT16 mech;
		if (getICAP_XFERMECH(mech) &&
			mech == _mech) {
			PrintCMDMessage("XferMech successfully set!\n");
		}
	}

	// Update compression and FileFormat after xfer is set
	get_CAP(m_ICAP_COMPRESSION);
	get_CAP(m_ICAP_IMAGEFILEFORMAT);

	return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_IMAGEFILEFORMAT(const TW_UINT16 _fileformat) {
	set_CapabilityOneValue(ICAP_IMAGEFILEFORMAT, _fileformat, TWTY_UINT16);

	// now that we have set it, re-get it to ensure it was set
	if (TWCC_SUCCESS == get_CAP(m_ICAP_IMAGEFILEFORMAT)) {
		TW_UINT16 fileformat;
		if (getICAP_IMAGEFILEFORMAT(fileformat) &&
			fileformat == _fileformat) {
			PrintCMDMessage("ImageFileFormat successfully set!\n");
		}
	}

	// Update compression after xfer is set
	get_CAP(m_ICAP_COMPRESSION);

	return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_COMPRESSION(const TW_UINT16 _comp) {
	set_CapabilityOneValue(ICAP_COMPRESSION, _comp, TWTY_UINT16);

	// now that we have set it, re-get it to ensure it was set
	if (TWCC_SUCCESS == get_CAP(m_ICAP_COMPRESSION)) {
		TW_UINT16 comp;
		if (getICAP_COMPRESSION(comp) &&
			comp == _comp) {
			PrintCMDMessage("Compression successfully set!\n");
		}
	}

	return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_BITDEPTH(const TW_UINT16 _nVal) {
	set_CapabilityOneValue(ICAP_BITDEPTH, _nVal, TWTY_UINT16);

	// now that we have set it, re-get it to ensure it was set
	if (TWCC_SUCCESS == get_CAP(m_ICAP_BITDEPTH)) {
		TW_UINT16 val;
		if (getICAP_BITDEPTH(val) &&
			val == _nVal) {
			PrintCMDMessage("BitDepth successfully set!\n");
		}
	}

	return;
}

//////////////////////////////////////////////////////////////////////////////
bool TwainAppCMD::getICAP_UNITS(TW_UINT16& _val) {
	TW_UINT32 val;
	bool rtn = getCurrent(&m_ICAP_UNITS, val);
	_val = (TW_UINT16)val;
	return rtn;
}

bool TwainAppCMD::getCAP_XFERCOUNT(TW_INT16& _val) {
	TW_UINT32 val;
	bool rtn = getCurrent(&m_CAP_XFERCOUNT, val);
	_val = (TW_UINT16)val;
	return rtn;
}

bool TwainAppCMD::getICAP_XFERMECH(TW_UINT16& _val) {
	TW_UINT32 val;
	bool rtn = getCurrent(&m_ICAP_XFERMECH, val);
	_val = (TW_UINT16)val;
	return rtn;
}

bool TwainAppCMD::getICAP_PIXELTYPE(TW_UINT16& _val) {
	TW_UINT32 val;
	bool rtn = getCurrent(&m_ICAP_PIXELTYPE, val);
	_val = (TW_UINT16)val;
	return rtn;
}

bool TwainAppCMD::getICAP_BITDEPTH(TW_UINT16& _val) {
	TW_UINT32 val;
	bool rtn = getCurrent(&m_ICAP_BITDEPTH, val);
	_val = (TW_UINT16)val;
	return rtn;
}

bool TwainAppCMD::getICAP_IMAGEFILEFORMAT(TW_UINT16& _val) {
	TW_UINT32 val;
	bool rtn = getCurrent(&m_ICAP_IMAGEFILEFORMAT, val);
	_val = (TW_UINT16)val;
	return rtn;
}

bool TwainAppCMD::getICAP_COMPRESSION(TW_UINT16& _val) {
	TW_UINT32 val;
	bool rtn = getCurrent(&m_ICAP_COMPRESSION, val);
	_val = (TW_UINT16)val;
	return rtn;
}

bool TwainAppCMD::getICAP_XRESOLUTION(TW_FIX32& _xres) {
	return getCurrent(&m_ICAP_XRESOLUTION, _xres);
}

bool TwainAppCMD::getICAP_YRESOLUTION(TW_FIX32& _yres) {
	return getCurrent(&m_ICAP_YRESOLUTION, _yres);
}


////////////////////////////////////////////////////////////


void TwainAppCMD::InitPanasonic() {

	//this->ResetAll();

	set_CapabilityOneValue(PCAP_AUTOSKIPBLANKPAGES, m_pDriverConfig->AutoDiscardBlankPage, TWTY_INT32);

	set_CapabilityOneValue(CAP_INDICATORS, m_pDriverConfig->Indicator, TWTY_BOOL);

	set_CapabilityOneValue(PCAP_HIDEERRDIALOG, m_pDriverConfig->HideErrDialog, TWTY_BOOL);

	set_CapabilityOneValue(PCAP_SHOWERRMSG, m_pDriverConfig->ShowErrMsg, TWTY_BOOL);

	//set_CapabilityOneValue(CAP_FEEDERENABLED, TRUE, TWTY_BOOL);
	//set_CapabilityOneValue(PCAP_FEEDINGSPEED, m_pDriverConfig->FeedingSpeed, TWTY_UINT16);

	set_CapabilityOneValue(CAP_DUPLEXENABLED, m_pDriverConfig->DuplexEnabled, TWTY_BOOL);

	if (m_pDriverConfig->PaperLen == 0) {
		set_CapabilityOneValue(ICAP_AUTOMATICLENGTHDETECTION, FALSE, TWTY_BOOL);
		set_CapabilityOneValue(ICAP_UNDEFINEDIMAGESIZE, TRUE, TWTY_BOOL);
		set_CapabilityOneValue(ICAP_AUTOMATICBORDERDETECTION, m_pDriverConfig->AutoBorderDetection, TWTY_BOOL);
	}
	else {
		//		TWSS_NONE TWSS_JISB5 TWSS_USLETTER TWSS_USLEGAL
		//		TWSS_USLEDGER TWSS_A3 TWSS_A4 TWSS_A5 TWSS_A6 TWSS_JISB4 TWSS_JISB6 TWSS_BUSINESSCARD
		set_CapabilityOneValue(ICAP_SUPPORTEDSIZES, TWSS_NONE, TWTY_UINT16);
		set_CapabilityOneValue(ICAP_AUTOMATICLENGTHDETECTION, TRUE, TWTY_BOOL);
	}

	set_CapabilityOneValue(ICAP_ORIENTATION, m_pDriverConfig->Orientation, TWTY_UINT16);

	set_CapabilityOneValue(ICAP_AUTOMATICDESKEW, m_pDriverConfig->AutoSkew, TWTY_BOOL);

	set_CapabilityOneValue(ICAP_PIXELTYPE, m_pDriverConfig->PixelType, TWTY_UINT16); // 0

	set_CapabilityOneValue(0x8200, m_pDriverConfig->RedColorEnhance, TWTY_UINT16);

	set_CapabilityOneValue(PCAP_DOUBLEFEEDDETECTION, m_pDriverConfig->DoubleFeedDetection, TWTY_UINT16);
	set_CapabilityOneValue(PCAP_DOUBLEFEEDDETECTION_SENSITIVITY, m_pDriverConfig->DoubleFeedDetectionSensitivity, TWTY_UINT16); // -灵敏度: 1. 正常
	set_CapabilityOneValue(PCAP_DOUBLEFEEDDETECTION_ACTION, m_pDriverConfig->DoubleFeedDetectionResponse, TWTY_UINT16); // -动作: 停止(2)

	TW_FIX32 contrast;
	contrast.Whole = m_pDriverConfig->Contrast;
	contrast.Frac = 0;
	set_CapabilityOneValue(ICAP_CONTRAST, &contrast);

	TW_FIX32 resolution;
	resolution.Whole = m_pDriverConfig->ResolutionWhole; // 200
	resolution.Frac = m_pDriverConfig->ResolutionFraction; // 0
	set_ICAP_RESOLUTION(ICAP_XRESOLUTION, &resolution);
	set_ICAP_RESOLUTION(ICAP_YRESOLUTION, &resolution);

	TW_FIX32 brightness;
	brightness.Whole = m_pDriverConfig->Brightness;
	brightness.Frac = 0;
	set_CapabilityOneValue(ICAP_BRIGHTNESS, &brightness); //



	//set_CapabilityOneValue(PCAP_AUTOROTATE, FALSE, TWTY_BOOL);

	//set_CapabilityOneValue(PCAP_AUTOROTATE_LANGUAGE,10, TWTY_UINT16);		//TWAR_LG_TCHINESE
}

void TwainAppCMD::InitCanon() {

	ScanType scanType = ScanManager::getInstance()->getScanType();
	if (/*scanType == kEditableTemplate ||*/ scanType == kOriginalPaper) {
		this->ResetAll();
		set_CapabilityOneValue(CCAP_AUTOROTATEDIRECTION, m_pDriverConfig->AutoRotate, TWTY_BOOL);

		set_CapabilityOneValue(CCAP_ORIENTATION, m_pDriverConfig->Orientation, TWTY_INT32);
	}
	else {
		set_CapabilityOneValue(CCAP_AUTOROTATEDIRECTION, FALSE, TWTY_BOOL);
		//TWOR_ROT0, TWOR_ROT90, TWOR_ROT180, TWOR_ROT270
		set_CapabilityOneValue(CCAP_ORIENTATION, m_pDriverConfig->Orientation, TWTY_INT32);
	}

	set_CapabilityOneValue(ICAP_AUTOMATICBORDERDETECTION, m_pDriverConfig->AutoBorderDetection, TWTY_BOOL);

	set_CapabilityOneValue(ICAP_AUTOMATICDESKEW, m_pDriverConfig->AutoSkew, TWTY_BOOL);

	set_CapabilityOneValue(CCAP_CEI_BLANKSKIP, m_pDriverConfig->AutoDiscardBlankPage, TWTY_INT32);

	set_CapabilityOneValue(CAP_DUPLEXENABLED, m_pDriverConfig->DuplexEnabled, TWTY_BOOL);

	set_CapabilityOneValue(CCAP_DOUBLEFEEDDETECTION, m_pDriverConfig->DoubleFeedDetection, TWTY_BOOL);

	set_CapabilityOneValue(CAP_INDICATORS, m_pDriverConfig->Indicator, TWTY_BOOL);

	// 0:BW, 1:GRAY, 2:RGB
	set_CapabilityOneValue(ICAP_PIXELTYPE, m_pDriverConfig->PixelType, TWTY_UINT16);

	set_CapabilityOneValue(CCAP_QUIETERRORS, m_pDriverConfig->QuietErrors, TWTY_BOOL);

	set_CapabilityOneValue(CCAP_QUIET_NOPAGEDIALOG, m_pDriverConfig->QuietNoPageDialog, TWTY_BOOL);

	TW_FIX32 resolution;
	resolution.Whole = m_pDriverConfig->ResolutionWhole; // 200
	resolution.Frac = m_pDriverConfig->ResolutionFraction; // 0
	set_ICAP_RESOLUTION(ICAP_XRESOLUTION, &resolution);
	set_ICAP_RESOLUTION(ICAP_YRESOLUTION, &resolution);

	TW_FIX32 contrast;
	contrast.Whole = m_pDriverConfig->Contrast;
	contrast.Frac = 0;
	set_CapabilityOneValue(ICAP_CONTRAST, &contrast);

	TW_FIX32 brightness;
	brightness.Whole = m_pDriverConfig->Brightness;
	brightness.Frac = 0;
	set_CapabilityOneValue(ICAP_BRIGHTNESS, &brightness); //

	//set_CapabilityOneValue(CAP_FEEDERENABLED, TRUE, TWTY_BOOL);  //CAP_FEEDERENDABLE must be TRUE to enable CAP_AUTOFEED
	set_CapabilityOneValue(CAP_AUTOFEED, TRUE, TWTY_BOOL);

	//	set_CapabilityOneValue(0x80B8, 1, TWTY_UINT16);
	//	set_CapabilityOneValue(0x80A2, 0, TWTY_BOOL);

	set_CapabilityOneValue(CAP_CUSTOMBASE + 50, m_pDriverConfig->RedColorEnhance, TWTY_INT32);

}

void TwainAppCMD::InitKodak() {

	set_CapabilityOneValue(ICAP_AUTOMATICBORDERDETECTION, m_pDriverConfig->AutoBorderDetection, TWTY_BOOL);

	set_CapabilityOneValue(ICAP_AUTOMATICDESKEW, m_pDriverConfig->AutoSkew, TWTY_BOOL);

	set_CapabilityOneValue(CAP_LANGUAGE, TWLG_CHINESE, TWTY_UINT16);

	set_CapabilityOneValue(CAP_INDICATORS, m_pDriverConfig->Indicator, TWTY_BOOL);

	set_CapabilityOneValue(CAP_DUPLEXENABLED, m_pDriverConfig->DuplexEnabled, TWTY_BOOL);  // 1

	if (m_pDriverConfig->DoubleFeedDetection == 0){
		set_CapabilityOneValue(KCAP_DOUBLEFEEDDETECTION, m_pDriverConfig->DoubleFeedDetection, TWTY_UINT16);
	}

	ScanType scanType = ScanManager::getInstance()->getScanType();
	if (scanType == kEditableTemplate || scanType == kOriginalPaper) {
		set_CapabilityOneValue(ICAP_AUTOMATICROTATE, m_pDriverConfig->AutoRotate, TWTY_BOOL);
		set_CapabilityOneValue(KCAP_ORIENTATION, m_pDriverConfig->Orientation, TWTY_INT32);
	}
	else{
		set_CapabilityOneValue(ICAP_AUTOMATICROTATE, FALSE, TWTY_BOOL);		//1
		//TWOR_ROT0, TWOR_ROT90, TWOR_ROT180, TWOR_ROT270
		set_CapabilityOneValue(KCAP_ORIENTATION, m_pDriverConfig->Orientation, TWTY_INT32);
	}

	// 0:BW, 1:GRAY, 2:RGB
	set_CapabilityOneValue(ICAP_PIXELTYPE, m_pDriverConfig->PixelType, TWTY_UINT16);

	// set resoluiton: 200 dpi.
	TW_FIX32 resolution;
	resolution.Whole = m_pDriverConfig->ResolutionWhole; // 200
	resolution.Frac = m_pDriverConfig->ResolutionFraction; // 0
	set_ICAP_RESOLUTION(ICAP_XRESOLUTION, &resolution);
	set_ICAP_RESOLUTION(ICAP_YRESOLUTION, &resolution);

	TW_FIX32 contrast;
	contrast.Whole = m_pDriverConfig->Contrast;
	contrast.Frac = 0;
	set_CapabilityOneValue(ICAP_CONTRAST, &contrast);

	//ICAP_BITDEPTHREDUCTION
	set_CapabilityOneValue(ICAP_BITDEPTHREDUCTION, TWBR_THRESHOLD, TWTY_UINT16);

	TW_FIX32 threshold;
	threshold.Whole = abs(255 - (m_pDriverConfig->Brightness + 1000) / (2000.0 / 255));
	threshold.Frac = 0;
	set_CapabilityOneValue(ICAP_THRESHOLD, &threshold);
}

void TwainAppCMD::InitFujitsu(){
	TW_UINT16 flag;
	set_CapabilityOneValue(ICAP_UNITS, 0, TWTY_UINT16);
	// 0:BW, 1:GRAY, 2:RGB, TBC..TBD
	set_CapabilityOneValue(ICAP_PIXELTYPE, m_pDriverConfig->PixelType, TWTY_UINT16);

	set_CapabilityOneValue(CAP_DUPLEXENABLED, m_pDriverConfig->DuplexEnabled, TWTY_BOOL);

	TW_FIX32 resolution;
	resolution.Whole = m_pDriverConfig->ResolutionWhole;
	resolution.Frac = m_pDriverConfig->ResolutionFraction;	// 0
	set_ICAP_RESOLUTION(ICAP_XRESOLUTION, &resolution);
	set_ICAP_RESOLUTION(ICAP_YRESOLUTION, &resolution);
	TW_FIX32 wf = FloatToFIX32(360);
	flag = set_CapabilityOneValue(ICAP_ROTATION, &wf);

	set_CapabilityOneValue(ICAP_AUTOSIZE, 1, TWTY_BOOL);
	//		TWSS_NONE TWSS_JISB5 TWSS_USLETTER TWSS_USLEGAL
	//		TWSS_USLEDGER TWSS_A3 TWSS_A4 TWSS_A5 TWSS_A6 TWSS_JISB4 TWSS_JISB6 TWSS_BUSINESSCARD
	set_CapabilityOneValue(ICAP_SUPPORTEDSIZES, TWSS_A3, TWTY_UINT16);

	set_CapabilityOneValue(ICAP_AUTOMATICBORDERDETECTION, m_pDriverConfig->AutoBorderDetection, TWTY_BOOL);

	set_CapabilityOneValue(PCAP_AUTOSKIPBLANKPAGES, m_pDriverConfig->AutoDiscardBlankPage, TWTY_INT32);

	set_CapabilityOneValue(CAP_INDICATORS, m_pDriverConfig->Indicator, TWTY_BOOL);

	set_CapabilityOneValue(PCAP_HIDEERRDIALOG, m_pDriverConfig->HideErrDialog, TWTY_BOOL);	//最好为false

	set_CapabilityOneValue(PCAP_SHOWERRMSG, m_pDriverConfig->ShowErrMsg, TWTY_BOOL); // 最好为true

	set_CapabilityOneValue(ICAP_AUTOMATICLENGTHDETECTION, FALSE, TWTY_BOOL);

	set_CapabilityOneValue(0x8200, m_pDriverConfig->RedColorEnhance, TWTY_UINT16);

	set_CapabilityOneValue(PCAP_DOUBLEFEEDDETECTION, m_pDriverConfig->DoubleFeedDetection, TWTY_UINT16);
	set_CapabilityOneValue(PCAP_DOUBLEFEEDDETECTION_SENSITIVITY, m_pDriverConfig->DoubleFeedDetectionSensitivity, TWTY_UINT16); // -灵敏度: 1. 正常
	set_CapabilityOneValue(PCAP_DOUBLEFEEDDETECTION_ACTION, m_pDriverConfig->DoubleFeedDetectionResponse, TWTY_UINT16); // -动作: 停止(2)


	//阈值是ICAP_THRESHOLD
	//富士通的阈值一定要放在对比度的前面 否则不生效
	TW_FIX32 brightness;
	brightness.Whole = abs(255 - (m_pDriverConfig->Brightness + 1000) / (2000.0 / 255));
	brightness.Frac = 0;
	set_CapabilityOneValue(ICAP_THRESHOLD, &brightness); // 亮度

	//对比度
	TW_FIX32 contrast;
	contrast.Whole = abs(m_pDriverConfig->Contrast) / (2000.0 / 255);
	contrast.Frac = 0;
	set_CapabilityOneValue(ICAP_CONTRAST, &contrast);
	set_CapabilityOneValue(ICAP_BITDEPTHREDUCTION, 0, TWTY_UINT16);
}

void TwainAppCMD::InitScanner(const std::string& scannerName) {
	std::string show_scan_ui = Configuration::getInstance()->getValue("show_scan_ui");
	if (show_scan_ui == "1")
	{
		return;
	}
	if (scannerName == "Kodak")
		InitKodak();
	else if (scannerName == "Canon")
		InitCanon();
	else if (scannerName == "Panasonic")
		InitPanasonic();
	else if (scannerName == "Fujitsu")
		InitFujitsu();
}


//>>This function resets all current values back to original power-on defaults.
//>>All current values areset to their default value except where mandatory values are required.
//>>All constraints are removed for all of the negotiable capabilities supported by the driver.
TW_INT16 TwainAppCMD::ResetAll() {
	TW_INT16        twrc = TWRC_FAILURE;
	TW_CAPABILITY   cap;

	cap.Cap = CAP_SUPPORTEDCAPS;
	cap.ConType = TWON_DONTCARE16;
	cap.hContainer = NULL;

	twrc = DSM_Entry(DG_CONTROL, DAT_CAPABILITY, MSG_RESETALL, (TW_MEMREF)&(cap));
	if (twrc == TWRC_SUCCESS)
		LOG(INFO)<<"ResetAll capabilities success!";
	else if (twrc == TWRC_FAILURE)
		printError(m_pDataSource, "Could not reset all capabilities.");

	return twrc;
}
