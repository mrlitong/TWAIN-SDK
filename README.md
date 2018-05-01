TWAIN protocol introduction
====

> TWAIN(full name：Technology Without An Interesting Name)is an applications programming interface (API) and communications protocol that regulates communication between software and digital imaging devices, such as image scanners and digital cameras.
TWAIN is not a hardware-level protocol; it requires a driver called Data Source for each device.
> - Provide multiple-platform support
> - Maintain and distribute a no-charge developer's toolkit
> - Ensure ease of implementation
> - Encourage widespread adoption
> - Open Source Data Source Manager
> - Multiple images format support
>
TWAIN is not a hardware-level protocol.  It requires a driver (mysteriously called a Data Source or DS) for each imaging device.  When a device is advertised as TWAIN Compatible this simply means the device has a TWAIN DS (driver) available for it.

TWAIN is available at this time (March 2007) on all 32-bit Intel versions of Microsoft Windows and on Apple OS X. Note that Dosadi does not support Apple products or OS X development.
The TWAIN Manager or DSM is provided for free by the TWAIN Working Group - it acts as liason and coordinator between TWAIN applications and Data Sources.

The DSM has minimal user interface - just the Select Source dialog.  All user interaction outside of the Application is handled by the Data Source.

Each Data Source is basically a high-level device driver, provided by the device manufacturer. The TWAIN Working Group does not enforce compliance with the TWAIN standard. TWAIN compliance and compliance testing are voluntary, and left up to application and DS developers.

The TWAIN manager and the Data Source are DLLs, so they load into the application's memory space and run essentially as subroutines of the application.  The DSM uses interprocess communication to coordinate with other instances of itself when more than one application uses TWAIN.

Much simplified, the steps of an application using TWAIN are:

1. Open a device and have a conversation with the device (actually the DS) about the capabilities and settings of the device.  There is a huge list of capabilities, every device implements a different set.  But for example, most devices can list the resolutions they support, and most will allow an application to select a resolution for subsequent images.

2. Enable a device -which in TWAIN terms means 'give the device permission to deliver images.'  Most scanners when enabled display their user interface and allow the user to adjust scan settings, eventually hitting a scan button to actually start the scan.

The application can request that a DS not display a user interface, but some devices ignore the request and show their dialog anyway.  This is a common cause of surprise and frustration for TWAIN application developers.

3. Once a device is enabled, the application waits for a notification from the DS that an image is ready.  While it waits, the application must somehow ensure that any messages posted to it are routed through TWAIN.  Unless this is done correctly, the application will never receive the image-ready notification, and the device's user interface may not work correctly.

4. The application accepts the image from the DS.

TWAIN defines three modes of image transfer: 
    Native - on Windows this is a DIB (Device Independent Bitmap) in memory.  Note that Windows has two kinds of bitmaps!
    Memory - strips or blocks of pixels in a series of memory buffers.
    File - the DS writes the image out directly to a file.   [A DS is not required to support this mode.]

5. If appropriate, the application may transfer multiple images until it chooses to stop, or until the DS signals that no more images are available.

6. The application normally then disables the DS and closes it - in symmetry with the open/enable steps we started with.

EZTWAIN
EZTwain sits between the application and the TWAIN DSM, hiding the complexity of the TWAIN API from the rest of the application. With EZTwain, TWAIN is still needed - all the TWAIN parts play their usual roles.  To the application, EZTWAIN presents a simplified version of the TWAIN API, plus additional features that are useful to scanning applications, such as image file loading and saving.   EZTWAIN relieves the application of the chore of routing all windows messages through TWAIN as needed, and of waiting for and reacting to the image-ready notification.
> [https://zh.wikipedia.org/wiki/TWAIN](https://zh.wikipedia.org/wiki/TWAIN "https://zh.wikipedia.org/wiki/TWAIN")

## TWAIN Stucture

### Layout
![](https://github.com/mrlitong/TWAIN-SDK/blob/master/source/twain_layout.jpg)

### Application
TODO

### Source Manager

In this figure, we can see that Source Manager plays a very important role as a bridge between Application and Source. Through the unified entry function DSM_Entry(), we can transfer messages between Application and Source to achieve the purpose of communication. The communication between the Source Manager and the Source is done through DS_Entry(). The developer doesn't have to worry about how this function is called.
Source Manager can automatically detect the installed image device on the user's computer and can load the device as needed.

### Source
TWAIN core module, a dll file provided by the device manufacturer, this dll and TWAIN interface remain unified, but ** different manufacturers are not exactly the same **. Developers need to perform logical judgments and other verifications at the Application layer. This component is dedicated to solving this problem.

## TWAIN-SDK

TWAIN-SDK provides a unified developer interface for applications that supports the following scanner models:
- Canon CR-1XX series 3XX series，6XX series，D12XX series，FS27XX series，FB63XX series
- Panasonic KV-S50XX series，KV-S20XX series
- Kodak i30XX series，i26XX series
- Fujitsu
- UNISLAN
- Epson

Developers do not need to care about hardware differences or interface differences between different scanners.

## **Guide**

### 1.Get TWAIN pointer based on OS

```C
	DSMENTRYPROC lpDSM_Entry;   //* DSM_Entry pointer
	HMODULE      hDSMDLL;       //* Twain_32.Dll handle

	if ((hDSMDLL = LoadLibrary("TWAIN_32.DLL")) != NULL)
	{
		 if ( (lpDSM_Entry =(DSMENTRYPROC) GetProcAddress(hDSMDLL,MAKEINTRESOURCE(1)))!=NULL)
	}
	// All below operation based on this pointer.
```

### 2. Open Source Manager

```C
int CTwain::OpenSourceManager(void)
{
	TW_UINT16 rc;
	//...
 	//hPWnd is the handler of the parent's windows of Source.
    rc = (*lpDSM_Entry) (
						&AppID, 
						NULL, 
						DG_CONTROL,DAT_PARENT,MSG_OPENDSM,                              (TW_MEMREF) & (*hPWnd)
						) ; 

     switch (rc)    // Check if Source Manager is prepared.
     {
     	case TWRC_SUCCESS:   //  done!
        //...
     	case TWRC_CANCEL:
    	//...
     }
	//...
}
```

### 3.Open Source

```C
	int CTwain::OpenSource(void)
	{
	    TW_UINT16 rc;
	    rc = (*lpDSM_Entry) (&AppID,NULL,
	                        DG_CONTROL,DAT_IDENTITY,MSG_OPENDS,
	                       (TW_MEMREF) &SourceID);
	  switch (rc)
	  {
	  	case TWRC_SUCCESS: //  done!
	    //...
	  }
	    //...
	}
```

### 4. Handle event of Source

```C
int CTwain::DealSourceMsg(MSG *pMSG)
{
    TW_UINT16  rc    = TWRC_NOTDSEVENT;
    TW_EVENT  twEvent;
    twEvent.pEvent = (TW_MEMREF) pMSG;
    rc = (*lpDSM_Entry) (&AppID,&SourceID,
                       DG_CONTROL,DAT_EVENT,MSG_PROCESSEVENT,
                       (TW_MEMREF) &twEvent);
    switch (twEvent.TWMessage)
    {
	    case MSG_XFERREADY:	//iStatus=6
	        iStatus=6;
	        GetBmpInfo();
	        DoNativeTransfer();
	    case MSG_CLOSEDSREQ:	//close
	    case MSG_CLOSEDSOK: 
	    case MSG_NULL:
    }  
 . . .
}
```

### 5.Data transport

```C
int CTwain::DoNativeTransfer(void)
{
    TW_UINT32  hBitMap = NULL;  // Pointing to address of image data.
    TW_UINT16  rc;
     HANDLE     hbm_acq = NULL;
     rc = (*lpDSM_Entry)(&AppID,&SourceID,
                        DG_IMAGE,DAT_IMAGENATIVEXFER,MSG_GET,
                        (TW_MEMREF)&hBitMap);
    switch (rc)
    {
	   case TWRC_XFERDONE:
	        hbm_acq = (HBITMAP)hBitMap;
	        SendMessage(*hPWnd, PM_XFERDONE, (WPARAM)hbm_acq, 0);
	        iStatus = 7;
			break;
	   case TWRC_CANCEL:
	   case TWRC_FAILURE:
    }
    //...
}
```

### 6.Handle image data

```C
LRESULT CTwainAppView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message==PM_XFERDONE)   // Recieve PM_XFERDONE message
	{
	    HBITMAP hBmp=FixUp(HANDLE(wParam));  // image transform handling
	    Bitmap *pBm=0;           // GDI+ object
	    pBmp=pBm->FromHBITMAP(hBmp,hDibPal);
	    Invalidate();
   		//...
     }
    return CView::WindowProc(message, wParam, lParam);
}
```
