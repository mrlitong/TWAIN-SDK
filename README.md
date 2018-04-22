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
> 从硬件到软件，TWAIN包含四层：硬件、源、源管理器和软件。硬件厂家的TWAIN支持通常体现为支持TWAIN接口的驱动程序。TWAIN的硬件层接口被称为源，源管理器负责选择和管理来自不同硬件厂家的源。在微软的Windows上，源管理器是以DLL方式实现。TWAIN软件不直接调用硬件厂家的TWAIN接口，而是通过源管理器。用户在TWAIN软件中选择获取图像之后，TWAIN软件和硬件通过一系列交涉来决定如何传输数据。软件描述它需要的图像，而硬件描述它能够提供的图像。如果软硬件在图像格式上达成一致，那么控制被传递到源。源现在可以设置扫描选项，以及开始扫描。
> [https://zh.wikipedia.org/wiki/TWAIN](https://zh.wikipedia.org/wiki/TWAIN "https://zh.wikipedia.org/wiki/TWAIN")

## TWAIN结构

### 分层结构
![](https://github.com/mrlitong/TWAIN-SDK/blob/master/source/twain_layout.jpg)

### Application
开发者的应用程序

### Source Manager
在该图中可以看到Source Manager作为Application与Source沟通的桥梁起着非常重要的作用，通过统一入口函数DSM_Entry(),我们可以在Application和Source之间传递消息，从而达到通讯的目的。而Source Manager与Source的通讯通过DS_Entry(）来进行，开发者不必也无需关心此函数如何被调用。
Source Manager可以自动检测用户计算机上的已安装的图像设备，并可以根据需要去加载相应设备。

### Source
TWAIN核心模块，由设备厂家提供的一个dll文件，此dll与TWAIN接口保持统一，但**不同厂家间并非完全一致**。需要开发者在Application层进行逻辑判断等验证。此组件便是致力于解决此问题。

## TWAIN-SDK

TWAIN-SDK向应用程序提供了统一的开发者接口，此接口支持以下型号扫描仪：
- Canon CR-1XX系列 3XX系列，6XX系列，D12XX系列，FS27XX系列，FB63XX系列
- Panasonic KV-S50XX系列，KV-S20XX系列
- Kodak i30XX系列，i26XX系列
- Fujitsu
- UNISLAN
- Epson

开发者无需关心不同扫描仪之间的硬件差异或接口差异。


## **使用方式**

### 1.获取基于OS的TWAIN指针

```C
	DSMENTRYPROC lpDSM_Entry;   //* DSM_Entry 入口函数的指针
	HMODULE      hDSMDLL;       //* Twain_32.Dll句柄

	if ((hDSMDLL = LoadLibrary("TWAIN_32.DLL")) != NULL)
	{
		 if ( (lpDSM_Entry =(DSMENTRYPROC) GetProcAddress(hDSMDLL,MAKEINTRESOURCE(1)))!=NULL)
	}
	//成功获取DSN_Entry()指针，后续所有操作只需通过此指针进行。
```

### 2.打开Source Manager

```C
int CTwain::OpenSourceManager(void)
{
	TW_UINT16 rc;
	//...
 	//hPWnd是指定为Source的父窗口的句柄
    rc = (*lpDSM_Entry) (
						&AppID, 
						NULL, 
						DG_CONTROL,DAT_PARENT,MSG_OPENDSM,                              (TW_MEMREF) & (*hPWnd)
						) ; 

     switch (rc)    // 检查打开Source Manager是否成功
     {
     	case TWRC_SUCCESS:   //  成功
        //...
     	case TWRC_CANCEL:
    	//...
     }
	//...
}
```

### 3.打开Source

```C
	int CTwain::OpenSource(void)
	{
	    TW_UINT16 rc;
	    rc = (*lpDSM_Entry) (&AppID,NULL,
	                        DG_CONTROL,DAT_IDENTITY,MSG_OPENDS,
	                       (TW_MEMREF) &SourceID);
	  switch (rc)
	  {
	  	case TWRC_SUCCESS: //  成功
	    //...
	  }
	    //...
	}
```

### 4.处理Source事件

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

### 5.传输数据

```C
int CTwain::DoNativeTransfer(void)
{
    TW_UINT32  hBitMap = NULL;  // 指向图像数据地址
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

### 6.处理图像数据

```C
LRESULT CTwainAppView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message==PM_XFERDONE)   // 收到 PM_XFERDONE 消息
	{
	    HBITMAP hBmp=FixUp(HANDLE(wParam));  // 图像转换处理
	    Bitmap *pBm=0;           // GDI+的Bitmap对象
	    pBmp=pBm->FromHBITMAP(hBmp,hDibPal);
	    Invalidate();
   		//...
     }
    return CView::WindowProc(message, wParam, lParam);
}
```
