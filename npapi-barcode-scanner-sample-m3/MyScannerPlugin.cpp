//MyScannerPlugin.cpp

#include "MyScannerPlugin.h"
#include "./common/npfunctions.h"

#include <stdio.h>
#include <windows.h>

#include <time.h>
#include <stdlib.h>
//#pragma comment (lib, "crt.lib");
//#include <altcecrt.h>

//  These are the method / property names our plugin object will export

//barcode data property
static NPIdentifier sBarcodeData_id;
//enable disable barcode reading
static NPIdentifier sBarcodeEnabled_id;
static NPIdentifier sCurrentValue_id;

BOOL APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved ) {

	switch(ul_reason_for_call){
		case DLL_PROCESS_ATTACH:
			DEBUGMSG(1, (L"DLL_PROCESS_ATTACH\n"));
			break;
		case DLL_PROCESS_DETACH:
			DEBUGMSG(1, (L"DLL_PROCESS_DETACH\n"));
			break;
		case DLL_THREAD_ATTACH:
			DEBUGMSG(1, (L"DLL_THREAD_ATTACH\n"));
			break;
		case DLL_THREAD_DETACH:
			DEBUGMSG(1, (L"DLL_THREAD_DETACH\n"));
			break;
		default:
			break;
	}
	return TRUE;
}

char*  getLocalTimeStrA(char* str){
	SYSTEMTIME sysTime;
	memset(&sysTime,0,sizeof(sysTime));
	GetLocalTime(&sysTime);
	
    // Print local time as a string.	112
	sprintf( str, "%4i%02i%02i %02i:%02i:%02i", 
		sysTime.wYear, sysTime.wMonth, sysTime.wDay,
		sysTime.wHour, sysTime.wMinute, sysTime.wSecond
		); 
	return str;
}

TCHAR* getWString(NPIdentifier szIn){
	static TCHAR szOutW[MAX_PATH];
	memset (&szOutW, 0, MAX_PATH*sizeof(TCHAR));
	if(NPN_IdentifierIsString(szIn)){
		NPUTF8* strUTF8 = NPN_UTF8FromIdentifier(szIn);
		if(strUTF8!=NULL){
			int wCount = wcslen((TCHAR*)strUTF8);
			int cCount = strlen((char*)strUTF8);
			int iCvt = mbstowcs(szOutW, (char*)strUTF8, cCount);
			if(iCvt==0)
				wsprintf(szOutW, L"");
		}
		else{
			wsprintf(szOutW, L"");
		}
		NPN_MemFree(strUTF8);
	}
	else{
		int i=0;
		i=NPN_IntFromIdentifier(szIn);
		wsprintf(szOutW, L"%i", i);
	}
	return szOutW;
}

//used to initialize the object
static NPObject * AllocateMyScannerPluginObject(NPP npp, NPClass *aClass)
{
	DEBUGMSG(TRUE, (L"AllocateMyScannerPluginObject...\n"));
	//  Called in response to NPN_CreateObject
	MyScannerPluginObject* obj = new MyScannerPluginObject(npp);

/*LK
	//  Setting the default properties of the created object.
	obj->m_bBarcodereadEnabled=FALSE;
	obj->m_bScannAhead=FALSE;
	obj->m_hScannRead=NULL;		// event notifier to let scan thread know we did receive the scan msg
	obj->m_hHandleBarcodeRead = CreateEvent(NULL, FALSE, FALSE, NULL);
	obj->m_hStopScannerMonitor = CreateEvent(NULL, FALSE, FALSE, NULL);
	obj->m_hBarCodeReaderHandle = NULL;
*/
	obj->m_bBarcodeEnabled=FALSE;
	obj->m_bBarcodeReading=FALSE;
	memset(obj->m_szBarcodedata,0,MAX_SCAN_BUFFER);
	//  Create a Hidden Window so our sensor thread can re-synchronize back
	//  to the main thread
	obj->hWindow = CreateWindow(L"SensorWindow", NULL, 0, 0, 0, 0, 0, NULL, (HMENU) 0, NULL, NULL);
	if (obj->hWindow == NULL)
	{
		WNDCLASS wndclass;
		memset (&wndclass, 0, sizeof wndclass);
		wndclass.lpfnWndProc = obj->NpapiProc;
		wndclass.hInstance = NULL;
		wndclass.lpszClassName = L"SensorWindow";
		RegisterClass (&wndclass);
		obj->hWindow = CreateWindow(L"SensorWindow", NULL, 0, 0, 0, 0, 0, NULL, (HMENU) 0, NULL, NULL);
	}
	SetWindowLong(obj->hWindow, GWL_WNDPROC, (DWORD)obj->NpapiProc);
	return obj;
}

//  NPAPI Macro
DECLARE_NPOBJECT_CLASS_WITH_BASE(MyScannerPluginObject,
                                 AllocateMyScannerPluginObject);

bool MyScannerPluginObject::Construct(const NPVariant *args, uint32_t argCount,
                                     NPVariant *result)
{
	//  Where the JS Object is created, called when we say:
	//  var myObj = new MyScanner();
	bool bRetVal = false;

	//  Create Object, expect no arguments
	DEBUGMSG(TRUE, (L"Construct...\n"));
	if (argCount == 0)
	{
		NPObject* genericObj = NPN_CreateObject(mNpp, GET_NPOBJECT_CLASS(MyScannerPluginObject));
		if (!genericObj)
			return false;
		MyScannerPluginObject* obj = (MyScannerPluginObject*)genericObj;
		OBJECT_TO_NPVARIANT(genericObj, *result);
		
		//  We have a function in our plugin to output text to a text box
#ifdef DEBUG
		MessageToUser("Creating Object");
#endif
		//start the scanner thread but disable scans
//LK		CloseHandle(CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)BarcodeReadThread, obj, 0, NULL));
		//SetScannerEnable(FALSE);
		bRetVal = true;
	}
  return bRetVal;
}

/*LK

void MyScannerPluginObject::SetScannerEnable(BOOL bEnable){
	if(bEnable)
		DEBUGMSG(1, (L"SetScannerEnable: Scanner will be enabled\n"));
	else
		DEBUGMSG(1, (L"SetScannerEnable: Scanner will be disabled\n"));
	if (this->m_hBarCodeReaderHandle != NULL){
//LK		ITCSCAN_SetScannerEnable((INT32) this->m_hBarCodeReaderHandle, (INT32) bEnable);
		if(bEnable)
			this->m_bBarcodereadEnabled=TRUE;
		else{
			this->m_bBarcodereadEnabled=FALSE;
			this->ScannerCancelRead();
		}
	}
}

BOOL MyScannerPluginObject::GetScannerEnable(){
	if (this->m_hBarCodeReaderHandle != NULL){
		INT32 bEnabled=0;
//LK		if(SUCCEEDED(ITCSCAN_GetScannerEnable((INT32) this->m_hBarCodeReaderHandle, (INT32*)bEnabled))){
		if(true){
			DEBUGMSG(1, (L"ITCSCAN_GetScannerEnable: Scanner is %i\n", bEnabled));
			return bEnabled;
		}
		else{
			DEBUGMSG(1, (L"ITCSCAN_GetScannerEnable failed\n"));
			return FALSE;
		}
	}
	else{
		DEBUGMSG(1, (L"GetScannerEnable: m_hBarCodeReaderHandle is NULL\n"));
		return FALSE;
	}
}

void MyScannerPluginObject::ScannerCancelRead(){
	if(this->m_hBarCodeReaderHandle!=NULL){
		DWORD dwDiscardedMsgs, dwDiscardedBytes;
//LK		ITCSCAN_CancelRead((INT32)this->m_hBarCodeReaderHandle, TRUE, &dwDiscardedMsgs, &dwDiscardedBytes);
		DEBUGMSG(1, (L"ITCSCAN_CancelRead: disMsgs=%i, disBytes=%i\n", dwDiscardedMsgs, dwDiscardedBytes));
	}
}

LK*/

//  Called when the JS Object is destroyed
MyScannerPluginObject::~MyScannerPluginObject()
{
//LK	this->ScannerCancelRead();
//LK	SetEvent(m_hStopScannerMonitor);
//	ITCSCAN_Close((INT32) &this->m_hBarCodeReaderHandle);
	DEBUGMSG(TRUE, (L"Destruct...\n"));
}


bool MyScannerPluginObject::HasMethod(NPIdentifier name)
{
	//  Called by the plugin framework to query whether an object
	//  has a specified method, we only have one method, 'monitor()'
	DEBUGMSG(TRUE, (L"HasMethod...'%s'\n", getWString(name)));
	return (name == sBarcodeEnabled_id);
}

bool MyScannerPluginObject::HasProperty(NPIdentifier name)
{
	//  Called by the plugin framework to query whether a JS object
	//  has a specified property, we have three properties.
	DEBUGMSG(TRUE, (L"HasProperty...'%s'\n", getWString(name)));
	return (name == sBarcodeData_id ||
			name == sBarcodeEnabled_id
			);
}

bool MyScannerPluginObject::GetProperty(NPIdentifier name, NPVariant *result)
{
	//  Retrieve the value of a property.  *result is an out parameter
	//  into which we should store the value
	bool bReturnVal = false;
	VOID_TO_NPVARIANT(*result);
	DEBUGMSG(TRUE, (L"GetProperty...'%s'\n", getWString((char*) name)));

	if (name == sBarcodeEnabled_id){
//LK		this->m_bBarcodereadEnabled=GetScannerEnable();
		DEBUGMSG(TRUE, (L"...returning '%i'\n", this->m_bBarcodeEnabled));
		BOOLEAN_TO_NPVARIANT(this->m_bBarcodeEnabled, *result);
		bReturnVal = true;
	}
	else if (name== sBarcodeData_id){
		TCHAR* szBarcodeW;
		szBarcodeW = (TCHAR*)malloc(strlen(this->m_szBarcodedata)*sizeof(TCHAR));
		mbstowcs(szBarcodeW, this->m_szBarcodedata, strlen(this->m_szBarcodedata));
		DEBUGMSG(TRUE, (L"...returning '%s'\n", szBarcodeW));
		STRINGZ_TO_NPVARIANT((char*)this->m_szBarcodedata, *result);
		free(szBarcodeW);
		bReturnVal = true;
	}
	if (!bReturnVal)
		VOID_TO_NPVARIANT(*result);
	return bReturnVal;
}

bool MyScannerPluginObject::SetProperty(NPIdentifier name, const NPVariant *value)
{
	//  Sets the specified property to the specified value.
	DEBUGMSG(TRUE, (L"SetProperty...'%s'\n", getWString((char*)name)));
	bool bRetVal = false;
	if (name == sBarcodeEnabled_id)
	{
		BOOL bEnabled=FALSE;
		bEnabled = NPVARIANT_TO_BOOLEAN(*value);
		if(bEnabled){
			DEBUGMSG(TRUE, (L"... arg is true\n"));
			this->Scanner_Enable();
		}
		else
		{
			DEBUGMSG(TRUE, (L"... arg is false\n"));
			//  JAVASCRIPT: MyScanner.enablebarcode=false;
			this->Scanner_Disable();
		}

		bRetVal = true;
	}
	return bRetVal;
}

char* MyScannerPluginObject::npStrDup(const char* sIn)
{
	char* sOut = (char*)NPN_MemAlloc(strlen(sIn) + 1);
	if (sOut)
		strcpy(sOut, sIn);

	return sOut;
}

//called by javascript object methods
bool MyScannerPluginObject::Invoke(NPIdentifier name, const NPVariant *args,
                               uint32_t argCount, NPVariant *result)
{
	DEBUGMSG(TRUE, (L"Invoke...'%s'\n", getWString((char*) name)));
	//  Called when a method is called on an object
	bool bReturnVal = false;
	VOID_TO_NPVARIANT(*result);
	//  Convert to lower case to make our methods case insensitive
	char* szNameCmp = _strlwr(NPN_UTF8FromIdentifier(name));
	NPIdentifier methodName =  NPN_GetStringIdentifier(szNameCmp);
	NPN_MemFree(szNameCmp);

	// MyScanner.enablebarcode
	if (methodName == sBarcodeEnabled_id)
	{
		//  Expect one argument which is a boolean (start / stop)
		if (argCount == 1 && NPVARIANT_IS_BOOLEAN(args[0]))
		{
			if (NPVARIANT_TO_BOOLEAN(args[0]))
			{
				//  MyScanner.enablebarcode(true);
				//  Create a thread to monitor barcode reading			
//LK				m_bBarcodereadEnabled=TRUE;
				Scanner_Enable();
			}
			else
			{
				//  MyScanner.enablebarcode(false);
//LK				m_bBarcodereadEnabled=FALSE;
				Scanner_Disable();
			}
			//  Monitor has no return value
			VOID_TO_NPVARIANT(*result);
			bReturnVal = true;
		}
	}

	if (!bReturnVal)
		VOID_TO_NPVARIANT(*result);

	return bReturnVal;
}

/*LK
//thread to monitor barcode reader
DWORD MyScannerPluginObject::BarcodeReadThread(LPVOID lpParameter){
	MyScannerPluginObject* pSensor = (MyScannerPluginObject*)lpParameter;
	bool exitThread = false;
	
	HANDLE hWaitHandles[2];
	hWaitHandles[0] = pSensor->m_hStopScannerMonitor;
	hWaitHandles[1] = pSensor->m_hScannRead;

	DEBUGMSG(TRUE, (L"Barcode Read Thread Starting\n"));

		// Initialize the buffer.
//LK		pSensor->m_gOneScan.rgbDataBuffer = pSensor->m_bScanDataBuffer;
//LK		pSensor->m_gOneScan.dwDataBufferSize = MAX_SCAN_BUFFER;
		//	either periodically try a scan or wait infinite. If INFINITE, use ITCSCAN_CancelRead
		//  to stop read
//LK		pSensor->m_gOneScan.dwTimeout = INFINITE;// 500;	
		
		//HANDLE hBarCodeReaderHandle=NULL;
		if(pSensor->m_hBarCodeReaderHandle != NULL)
		{
			DWORD dwDiscardedMsgs, dwDiscardedBytes;
//LK			ITCSCAN_CancelRead((INT32)pSensor->m_hBarCodeReaderHandle, TRUE, &dwDiscardedMsgs, &dwDiscardedBytes);
//LK			ITCSCAN_Close((INT32) pSensor->m_hBarCodeReaderHandle);
		}
		pSensor->m_hBarCodeReaderHandle=NULL;

		// Open the scanner.
//LK		if ( SUCCEEDED( ITCSCAN_Open( (INT32*)&pSensor->m_hBarCodeReaderHandle, TEXT("default") ) ) )
			if(true)
		{
			DEBUGMSG(TRUE, (L"Barcode Scanner opened...\n"));
			while ( TRUE) //pSensor->m_bBarcodereadEnabled )
			{
				DEBUGMSG(TRUE, (L"Barcode Scanner SyncRead...\n"));
//LK				if ( SUCCEEDED( ITCSCAN_SyncRead( (INT32)pSensor->m_hBarCodeReaderHandle, &pSensor->m_gOneScan) ) )
				if(true)
				{
					DEBUGMSG(TRUE, (L"... got Barcode...\n"));

					// If blocking scan ahead, disable the scanner until this one is processed.
					if (!pSensor->m_bScannAhead){
						pSensor->SetScannerEnable(FALSE);
						//ITCSCAN_SetScannerEnable( (INT32)pSensor->m_hBarCodeReaderHandle, FALSE);
					}
					// Notify main thread that scan data is ready.
					//PostMessage( pSensor->hWindow, WM_HAVE_SCAN, 0, 0);

					//  Contention here if timeout is too small but this is just a demo
					//  Resynchronise with the main thread.
					char szBarcodeReading[MAX_SCAN_BUFFER];
					memset(szBarcodeReading,0,MAX_SCAN_BUFFER);
//LK					strncpy(szBarcodeReading, (char*)pSensor->m_gOneScan.rgbDataBuffer, pSensor->m_gOneScan.dwBytesReturned);
					//sprintf(szBarcodeReading, "%s", pSensor->m_gOneScan.rgbDataBuffer);
					
					TCHAR* szBarcodeReadingW;
//LK					szBarcodeReadingW = (TCHAR*)malloc(pSensor->m_gOneScan.dwBytesReturned * sizeof(TCHAR));
//LK					mbstowcs(szBarcodeReadingW, szBarcodeReading, pSensor->m_gOneScan.dwBytesReturned);
					DEBUGMSG(TRUE, (L"... got Barcode...'%s'\nPosting message...\n", szBarcodeReadingW));
					free(szBarcodeReadingW);

					//simple message
					PostMessage(pSensor->hWindow, WM_HAVE_SCAN, (WPARAM)pSensor, (LPARAM)szBarcodeReading);
					
					//extended message
					BARCODEDATA barcodedata;
					sprintf(barcodedata.data, "%s", szBarcodeReading);	
					char* str;
					str=(char*)malloc(32);
					sprintf(barcodedata.time, "%s", getLocalTimeStrA(str));
					//sprintf(barcodedata.type, "%04i", pSensor->m_gOneScan.iSymbology);	//STCIdentifiers.h
//LK					sprintf(barcodedata.type, "%s", BarcodeTypeStr[pSensor->m_gOneScan.iSymbology]);	//STCIdentifiers.h

					PostMessage(pSensor->hWindow, WM_SCANNER_NAVIGATE, (WPARAM)pSensor, (LPARAM)&barcodedata);
					free(str);

					//store scan data for later access
					memset(pSensor->m_szBarcodedata,0, MAX_SCAN_BUFFER);
					strncpy(pSensor->m_szBarcodedata, szBarcodeReading, strlen(szBarcodeReading));

					// Wait until scan data is processed. Some delay is made in this example
					// to demonstrate scan ahead.
					DEBUGMSG(TRUE, (L"Waiting for next scan release...\n"));
					WaitForSingleObject( pSensor->m_hScannRead, INFINITE );
					DWORD dwWaitReturn = WaitForMultipleObjects( 2, hWaitHandles, FALSE, INFINITE);
					switch (dwWaitReturn){
						case WAIT_OBJECT_0 + 1:		// m_hStopScannerMonitor signaled
							DEBUGMSG(TRUE, (L"m_hStopScannerMonitor signaled...\n"));
							goto _exit_scan_thread;
							break;
						case WAIT_OBJECT_0 :	// m_hScannRead signaled
							DEBUGMSG(TRUE, (L"m_hScannRead signaled...\n"));
							break;
						case WAIT_TIMEOUT:
							// uuups
							break;
						case WAIT_ABANDONED:
							break;
						case WAIT_FAILED:
							break;
						default:
							break;
					}

					// Re-enable scanner in case scan ahead was disabled.

					DEBUGMSG(TRUE, (L"ITCSCAN_SetScannerEnable after scan\n"));
					if (!pSensor->m_bScannAhead)
						pSensor->SetScannerEnable(TRUE);
				}
			}
_exit_scan_thread:
			DEBUGMSG(TRUE, (L"Closing scanner\n"));
//LK			ITCSCAN_Close( (INT32)pSensor->m_hBarCodeReaderHandle);
			pSensor->m_hBarCodeReaderHandle=NULL;
		}
		else
		{
			// Put up message to user and exit the program.
			MessageBox(NULL, _T("ITCSCAN_Open Failed"),NULL, MB_OK);
			return 0;
		}

	DEBUGMSG(TRUE, (L"Barcode Read Thread Exiting\n"));
	return 0;
}
LK*/


void MyScannerPluginObject::SetScannerSettings()
{
	DEBUGMSG(1, (L"SetScannerSettings()..."));
	////////////////////////////////////////////////////////
	// KSCANREAD kRead 초기화
    memset(&kRead, 0, sizeof(kRead));
	kRead.nSize = sizeof(kRead);

	// KSCANREADEX2 kReadEx2 초기화
	memset(&kReadEx2, 0, sizeof(kReadEx2));
	strcat(kReadEx2.Signature, "KSCANEX2");

	kRead.nTimeInSeconds	= 10;	// time out
	kRead.nMinLength		= 2;	// minimum data length
	kRead.nSecurity			= 1;    // security
	kReadEx2.XmitAIMID		= DISABLE;

	// KScanReadCallBack
	kRead.pUserData		= this;	
 	kRead.fnCallBack	= KScanReadCallBack;
	
	kRead.dwFlags |= KSCAN_FLAG_WIDESCANANGLE;


	//UPCA
	kReadEx2.UpcA.Enable		= ENABLE;
	kReadEx2.UpcA.Format		= AS_UPCA;
	kReadEx2.UpcA.XmitNumber	= XMIT_NUMBER;
	kReadEx2.UpcA.XmitCheckDigit= XMIT_CHECK_DIGIT;
	kReadEx2.UpcA.Supp			= NO_Supp;  

	//UPCE
	kReadEx2.UpcE.Enable		= ENABLE;
	kReadEx2.UpcE.Format		= AS_UPCE;
	kReadEx2.UpcE.XmitNumber	= XMIT_NUMBER;
	kReadEx2.UpcE.XmitCheckDigit= XMIT_CHECK_DIGIT;
    kReadEx2.UpcE.Supp			= NO_Supp;	// Not Supported

	//EAN13
	kReadEx2.Ean13.Enable		= ENABLE;
	kReadEx2.Ean13.Format		= AS_BOOKLAND;		// Including AS_EAN13;
	kReadEx2.Ean13.XmitNumber	= NO_XMIT_NUMBER;
	kReadEx2.Ean13.XmitCheckDigit= XMIT_CHECK_DIGIT;
    kReadEx2.Ean13.Supp			= NO_Supp;			

	//EAN8
	kReadEx2.Ean8.Enable		= ENABLE;
	kReadEx2.Ean8.Format		= AS_EAN8;
	kReadEx2.Ean8.XmitNumber	= NO_XMIT_NUMBER;
	kReadEx2.Ean8.XmitCheckDigit= XMIT_CHECK_DIGIT;   
	kReadEx2.Ean8.Supp			= NO_Supp;	// Not Supported

    //Code39
	kReadEx2.Code39.Enable		= ENABLE;
	kReadEx2.Code39.MinLength	= 4;
    kReadEx2.Code39.MaxLength	= 30;
    kReadEx2.Code39.SetAscii	= STD_ASCII;
    kReadEx2.Code39.CDV			= DISABLE;      
	kReadEx2.Code39.XmitCheckDigit= NO_XMIT_CHECK_DIGIT;
	kReadEx2.Code39.AsCode32	= ENABLE;
    kReadEx2.Code39.AsPZN		= ENABLE;

	//Code128
	kReadEx2.Code128.Enable		= ENABLE;
    kReadEx2.Code128.AsUCCEAN128= ENABLE;
//LK	kReadEx2.Code128.FNC1_ASCII = FNC1_Ascii; //NULL: No FNC1 conversion
	kReadEx2.Code128.FNC1_ASCII = NULL; //NULL: No FNC1 conversion
    kReadEx2.Code128.MinLength	= 4;
    kReadEx2.Code128.MaxLength	= 30;

	//Code93
	kReadEx2.Code93.Enable		= ENABLE;
    kReadEx2.Code93.MinLength	= 4;
    kReadEx2.Code93.MaxLength	= 30;

    //Code35
	kReadEx2.Code35.Enable		= ENABLE;

	//Code11
    kReadEx2.Code11.Enable		= ENABLE;
	kReadEx2.Code11.MinLength	= 4;
    kReadEx2.Code11.MaxLength	= 30;
	kReadEx2.Code11.CheckDigit	= DIGIT1;                          
    kReadEx2.Code11.XmitCheckDigit= NO_XMIT_CHECK_DIGIT; 
 
	//Interleaved 2of5
	kReadEx2.Code25.Enable		= ENABLE;
    kReadEx2.Code25.MinLength	= 4;
    kReadEx2.Code25.MaxLength	= 30;
    kReadEx2.Code25.CDV			= DISABLE;       
    kReadEx2.Code25.XmitCheckDigit= NO_XMIT_CHECK_DIGIT; 
	kReadEx2.Code25.KindofDecode = (CODE25KIND_INTER | CODE25KIND_ITF14|CODE25KIND_MATRIX | CODE25KIND_INDUSTRY | CODE25KIND_DLOGIC | CODE25KIND_IATA); 
//	kReadEx2.Code25.KindofDecode |= (CODE25KIND_MATRIX | CODE25KIND_INDUSTRY | CODE25KIND_DLOGIC | CODE25KIND_IATA); 

    //Codabar
    kReadEx2.Codabar.Enable			= ENABLE;
    kReadEx2.Codabar.XmitStartStop	= NO_XMIT;
    kReadEx2.Codabar.MinLength		= 4;
    kReadEx2.Codabar.MaxLength		= 30;
   
	//MSI
	kReadEx2.Msi.Enable			= ENABLE;
    kReadEx2.Msi.CDV			= ENABLE;                         
    kReadEx2.Msi.XmitCheckDigit	= NO_XMIT_CHECK_DIGIT; 
    kReadEx2.Msi.MinLength		= 4;
    kReadEx2.Msi.MaxLength		= 30;

	//Plessey
    kReadEx2.Plessey.Enable		= ENABLE;
    kReadEx2.Plessey.CDV		= ENABLE;                          
    kReadEx2.Plessey.XmitCheckDigit= NO_XMIT_CHECK_DIGIT;  
    kReadEx2.Plessey.MinLength	= 4;
    kReadEx2.Plessey.MaxLength	= 30;

	//GS1
    kReadEx2.Gs1.Enable			= ENABLE;

	//GS1 Limited
	kReadEx2.Gs1Limited.Enable	= ENABLE;

	//GS1 Expanded
	kReadEx2.Gs1Expanded.Enable	= ENABLE;

	// Telepen
	kReadEx2.Telepen.Enable		= ENABLE;
	kReadEx2.Telepen.OldStyle	= DISABLE;

	//////////////////////////////////////////////////////////////////////////

	// Koamtak과의 호환성으로 인해 추가
	kRead.dwReadType |= KSCAN_READ_TYPE_EAN_13;
	kRead.dwReadType |= KSCAN_READ_TYPE_EAN_8;
	kRead.dwReadType |= KSCAN_READ_TYPE_UPCA;
	kRead.dwReadType |= KSCAN_READ_TYPE_UPCE;
	kRead.dwReadType |= KSCAN_READ_TYPE_CODE_39;
	kRead.dwReadType |= KSCAN_READ_TYPE_ITF_14;
	kRead.dwReadType |= KSCAN_READ_TYPE_CODE_128;
	kRead.dwReadType |= KSCAN_READ_TYPE_CODE_I25;
	kRead.dwReadType |= KSCAN_READ_TYPE_CODA_BAR;
	kRead.dwReadType |= KSCAN_READ_TYPE_UCCEAN_128;
	kRead.dwReadType |= KSCAN_READ_TYPE_CODE_93;
	kRead.dwReadType |= KSCAN_READ_TYPE_CODE_35;
	kRead.dwReadType |= KSCAN_READ_TYPE_BOOKLAND;
	kRead.dwReadType |= KSCAN_READ_TYPE_EAN_13_ADDON;
	kRead.dwReadType |= KSCAN_READ_TYPE_EAN_8_ADDON;
	kRead.dwReadType |= KSCAN_READ_TYPE_UPCA_ADDON;
	kRead.dwReadType |= KSCAN_READ_TYPE_UPCE_ADDON;
	kRead.dwReadType |= KSCAN_READ_TYPE_MSI;
	kRead.dwReadType |= KSCAN_READ_TYPE_PZN;
	kRead.dwReadType |= KSCAN_READ_TYPE_CODE25_MATRIX;
	kRead.dwReadType |= KSCAN_READ_TYPE_CODE25_DLOGIC;
	kRead.dwReadType |= KSCAN_READ_TYPE_CODE25_INDUSTRY;
	kRead.dwReadType |= KSCAN_READ_TYPE_CODE25_IATA;
	kRead.dwReadType |= KSCAN_READ_TYPE_PLESSEY;
	kRead.dwReadType |= KSCAN_READ_TYPE_CODE11;
	kRead.dwReadType |= KSCAN_READ_TYPE_CODE32;
	kRead.dwReadType |= KSCAN_READ_TYPE_COUPONCODE;
	kRead.dwReadType |= KSCAN_READ_TYPE_GTIN14;
	kRead.dwReadType |= KSCAN_READ_TYPE_GS1;
	kRead.dwReadType |= KSCAN_READ_TYPE_GS1_LIMITED;
	kRead.dwReadType |= KSCAN_READ_TYPE_GS1_EXPANDED;

	kRead.pReadEx=&kReadEx2;
	DEBUGMSG(1, (L"SetScannerSettings() DONE"));
}


char* MyScannerPluginObject::GetBarCodeTypeString(int nType)
{
	switch(nType)
	{
	case KSCAN_RET_TYPE_EAN_13:		//0
		return "EAN-13";
		break;
	case KSCAN_RET_TYPE_EAN_8:		//1
		return "EAN-8";
		break;
	case KSCAN_RET_TYPE_UPCA:		//2
		return "UPC-A";
		break;
	case KSCAN_RET_TYPE_UPCE:		//3
		return "UPC-E";
		break;
	case KSCAN_RET_TYPE_CODE_39:	//4
		return "CODE 39";
		break;
	case KSCAN_RET_TYPE_ITF_14:		//5
		return "ITF 14";
		break;
	case KSCAN_RET_TYPE_CODE_128:	//6
		return "CODE 128";
		break;
	case KSCAN_RET_TYPE_CODE_I25:	//7
		return "Interleaved 2of5";
		break;
	case KSCAN_RET_TYPE_CODA_BAR:	//8
		return "CODABAR";
		break;
	case KSCAN_RET_TYPE_UCCEAN_128:	//9
		return "UCC/EAN-128";
		break;
	case KSCAN_RET_TYPE_CODE_93:	//10
		return "CODE 93";
		break;
	case KSCAN_RET_TYPE_CODE_35:	//11
		return "CODE 35";
		break;
	case KSCAN_RET_TYPE_PDF417:		//12	- Not Supported
		return "PDF417";
		break;
	case KSCAN_RET_TYPE_MACRO_PDF417://13	- Not Supported
		return "Micro PDF417";
		break;
	case KSCAN_RET_TYPE_BOOKLAND:	//14
		return "BOOKLAND_EAN";
		break;
	case KSCAN_RET_TYPE_MSI:		//15
		return "MSI";
		break;
	case KSCAN_RET_TYPE_PZN:		//16
		return "PZN";
		break;
	case KSCAN_RET_TYPE_PLESSEY:		//17
		return "PLESSEY";
		break;
	case KSCAN_RET_TYPE_MACRO_PDF417_INC:	//18 - Not Supported
		return "PDF417_INC";
		break;
	case KSCAN_RET_TYPE_MACRO_PDF417_ERROR://19 - Not Supported
		return "PDF417_ERROR";
		break;
	case KSCAN_RET_TYPE_CODE25_MATRIX:	//20
		return "CODE25_MATRIX";
		break;
	case KSCAN_RET_TYPE_CODE25_DLOGIC:	//21
		return "CODE25_DLOGIC";
		break;
	case KSCAN_RET_TYPE_CODE25_INDUSTRY://22
		return "CODE25_INDUSTRY";
		break;
	case KSCAN_RET_TYPE_CODE25_IATA:	//23
		return "CODE25_IATA";
		break;
	case KSCAN_RET_TYPE_CODE25_GTIN14:	//24
		return "CODE25_GTIN14";
		break;
	case KSCAN_RET_TYPE_CODE25_DPL:		//25
		return "CODE25_DPL";
		break;
	case KSCAN_RET_TYPE_CODE25_DPI:		//26
		return "CODE25_DPI";
		break;
	case KSCAN_RET_TYPE_CODE11:			//27
		return "CODE 11";
		break;
	case KSCAN_RET_TYPE_CODE32:			//28
		return "CODE 32";
		break;
	case KSCAN_RET_TYPE_COUPONCODE:		//29
		return "COUPONCODE";
		break;
	case KSCAN_RET_TYPE_CODABLOCK_A:	//30
		return "CODABLOCK_A";
		break;
	case KSCAN_RET_TYPE_CODABLOCK_F:	//31
		return "CODABLOCK_F";
		break;
	case KSCAN_RET_TYPE_GS1:			//32
		return "GS1";
		break;
	case KSCAN_RET_TYPE_GS1_LIMITED:	//33
		return "GS1_LIMITED";
		break;
	case KSCAN_RET_TYPE_GS1_EXPANDED:	//34
		return "GS1_EXPANDED";
		break;
	case KSCAN_RET_TYPE_STANDARD2OF5:	//35
		return "STANDARD 2OF5";
		break;
	case KSCAN_RET_TYPE_TELEPEN:		//36
		return "TELEPEN";
		break;
	case KSCAN_RET_TYPE_UNKNOWN:
	default:
		return "UNKNOWN TYPE";
		break;	
		
	}
}

void MyScannerPluginObject::Scanner_Enable()
{
	DEBUGMSG(1, (L"Scanner_Enable...\r\n"));
	if(m_KScan.IsScanOpen())
	{
		m_bBarcodeEnabled = TRUE;
#ifdef DEBUG
		MessageToUser("Scanner Already Open");
#endif
	}
	else
	{
		DEBUGMSG(1, (L"m_KScan.Open...\r\n"));
		m_bBarcodeEnabled = m_KScan.Open(6,FALSE,CBR_115200,FALSE,NULL);
	}
	if(!m_bBarcodeEnabled)
	{
		DEBUGMSG(1, (L"m_KScan.Open FAILED\r\n"));
#ifdef DEBUG
		MessageToUser("Scanner Open Failed");
#endif
	}
	else
	{
		DEBUGMSG(1, (L"m_KScan.Open OK\r\n"));
		if(m_bBarcodeReading)
		{
			DEBUGMSG(1, (L"Scanner_Enable(): m_KScan.ReadCancel()...\r\n"));
			m_KScan.ReadCancel();
		}
		SetScannerSettings();
		m_bBarcodeReading = TRUE;
		if(m_KScan.Read(&kRead))
		{
			DEBUGMSG(1, (L"m_KScan.Read(&kRead) FAILED"));
#ifdef DEBUG
		MessageToUser("Scanner Read Failed");
#endif
		}
		else
		{
			DEBUGMSG(1, (L"m_KScan.Read(&kRead) OK"));
			m_bBarcodeReading = FALSE;
		}
	}
}

void MyScannerPluginObject::Scanner_Disable()
{
	DEBUGMSG(1, (L"Scanner_Disable()..."));
	if(m_bBarcodeReading)
	{
		m_KScan.ReadCancel();
	}
	m_bBarcodeReading = FALSE;
	for(int i=0;i<3;i++)
	{		
		if(m_KScan.Close())
			break;
		Sleep(100);
	}
	m_bBarcodeEnabled = FALSE;
	DEBUGMSG(1, (L"Scanner_Disable() done"));
}


//  Message handler for our hidden window to resynchronize with the main thread (NPAPI is not 
//  threadsafe)
LRESULT CALLBACK MyScannerPluginObject::NpapiProc (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (message==WM_HAVE_SCAN){
		DEBUGMSG(TRUE, (L"NpapiProc Barcode called...\n"));
		MyScannerPluginObject* pSensor = (MyScannerPluginObject*)wparam;

		char* szMessage = (char*)lparam;
		//  Inform the user that the sensor has a new value
		pSensor->MessageToUser(szMessage);
		
//LK		SetEvent(pSensor->m_hScannRead);	//let the barcode read thread know
		return 0;
	}
	else if (message==WM_SCANNER_NAVIGATE){
		DEBUGMSG(TRUE, (L"NpapiProc Barcode called...\n"));
		MyScannerPluginObject* pSensor = (MyScannerPluginObject*)wparam;

		// invoke ScannerNavigate with data, type, time
		BARCODEDATA* barcodedata = (BARCODEDATA*)lparam;
		
		pSensor->MessageToUser(barcodedata);
		
//LK		SetEvent(pSensor->m_hScannRead);	//let the barcode read thread know
		return 0;
	}
	return DefWindowProc (hwnd, message, wparam, lparam);
}


bool MyScannerPluginObject::InvokeDefault(const NPVariant *args, uint32_t argCount,
													NPVariant *result)
{
	DEBUGMSG(TRUE, (L"InvokeDefault called...\n"));
	STRINGZ_TO_NPVARIANT(npStrDup("default method return val"), *result);
	return true;
}

//  Constructor for the Plugin, called when the embedded mime type is found on a web page (see npp_new).
//  <embed id="embed1" type="application/x-itc-myscanner" hidden=true> </embed> 
CMyScannerPlugin::CMyScannerPlugin(NPP pNPInstance) :
  m_pNPInstance(pNPInstance),
  m_pNPStream(NULL),
  m_bInitialized(FALSE),
  m_pScriptableObject(NULL)
{
	DEBUGMSG(TRUE, (L"CMyScannerPlugin called (npp_new)...\n"));
  	// Must initialise this before getting NPNVPluginElementNPObject, as it'll
	// call back into our GetValue method and require a valid plugin.
	pNPInstance->pdata = this;

    // Say that we're a windowless plugin.
    NPN_SetValue(m_pNPInstance, NPPVpluginWindowBool, false);

	//  Instantiate the values of the methods / properties we possess
	// Instance the values for a barcode read
	sBarcodeData_id = NPN_GetStringIdentifier("barcodedata");
	sBarcodeEnabled_id = NPN_GetStringIdentifier("enablebarcode");

	//  Export onto the webpage the JS object 'MyScanner'.  This enables us
	//  to say var myObj = new MyScanner();
	NPObject *sWindowObj;
	NPN_GetValue(m_pNPInstance, NPNVWindowNPObject, &sWindowObj);

	//#############################################################
	// http://stackoverflow.com/questions/4601682/npapi-use-javascript-objects-functions-from-within-plugin?rq=1
	///* Get window object */
		//NPObject* window = NULL; 
		//NPN_GetValue(aInstance, NPNVWindowNPObject, &window);
	
	/* Get document object */ 
	NPVariant controllerVar; 
	NPIdentifier id = NPN_GetStringIdentifier("MyScanner"); 
	NPN_GetProperty(m_pNPInstance, sWindowObj, id, &controllerVar);	//HGO 
	NPObject* document = NPVARIANT_TO_OBJECT(controllerVar);	
	//#############################################################

	NPObject *MyScannerObject =NPN_CreateObject(m_pNPInstance,GET_NPOBJECT_CLASS(MyScannerPluginObject));
	NPVariant v;
	OBJECT_TO_NPVARIANT(MyScannerObject, v);
	NPIdentifier n = NPN_GetStringIdentifier("MyScanner");
	NPN_SetProperty(m_pNPInstance, sWindowObj, n, &v);

	NPN_ReleaseObject(MyScannerObject);
	NPN_ReleaseObject(sWindowObj);
	NPN_ReleaseObject(document);
}

CMyScannerPlugin::~CMyScannerPlugin()
{

}

void MyScannerPluginObject::MessageToUser(BARCODEDATA* stBarcodeData){
	NPVariant functionval;
	NPVariant rval;
	NPObject *sWindowObj;
	NPN_GetValue(mNpp, NPNVWindowNPObject, &sWindowObj);

	//  Populate 'functionval' with the name of our function
	NPN_GetProperty(mNpp, sWindowObj, NPN_GetStringIdentifier("ScannerNavigate"), &functionval);
	NPVariant arg[3];

	if (NPVARIANT_TO_OBJECT(functionval) == 0)
		return;

	//  Create the argument to call 'ScannerNavigate' with
	//data
	char szData[MAX_SCAN_BUFFER + 32];
	sprintf(szData, "%s", stBarcodeData->data);
	//type
	char szType[MAX_PATH];
	sprintf(szType, "%s", stBarcodeData->type);
	//time
	char szTime[MAX_PATH];
	sprintf(szTime, "%s", stBarcodeData->time);

	//  Add the string argument to our javascript function to an argument, 'arg'
	STRINGZ_TO_NPVARIANT(szData, arg[0]);
	STRINGZ_TO_NPVARIANT(szType, arg[1]);
	STRINGZ_TO_NPVARIANT(szTime, arg[2]);

	//  Invoke the Javascript function on the page
	NPN_InvokeDefault(mNpp, NPVARIANT_TO_OBJECT(functionval), arg, 3,
						&rval);
	//  Clean up allocated memory
	NPN_ReleaseVariantValue(&functionval);
	NPN_ReleaseVariantValue(&rval);
	NPN_ReleaseObject(sWindowObj);

}

//  This code calls the Javascript function 'addSensorOutput' to interact
//  with the DOM.
void MyScannerPluginObject::MessageToUser(char* szMessage)
{
	TCHAR szMessageW[MAX_PATH];
	memset(szMessageW,0,MAX_PATH * sizeof(TCHAR));
	mbstowcs(szMessageW, szMessage, strlen(szMessage));
	DEBUGMSG(TRUE, (L"MessageToUser called '%s'...\n", szMessageW));

	NPVariant functionval;
	NPVariant rval;
	NPObject *sWindowObj;
	NPN_GetValue(mNpp, NPNVWindowNPObject, &sWindowObj);
	//  Populate 'functionval' with the name of our function
	NPN_GetProperty(mNpp, sWindowObj, NPN_GetStringIdentifier("addScannerOutput"), &functionval);
	NPVariant arg;
	if (NPVARIANT_TO_OBJECT(functionval) == 0)
		return;
	//  Create the argument to call 'addScannerOutput' with
	char szSourceMessage[MAX_SCAN_BUFFER + 32];

	sprintf(szSourceMessage, "%s", szMessage);

	//  Add the string argument to our javascript function to an argument, 'arg'
	STRINGZ_TO_NPVARIANT(szSourceMessage, arg);
	//  Invoke the Javascript function on the page
	NPN_InvokeDefault(mNpp, NPVARIANT_TO_OBJECT(functionval), &arg, 1,
						&rval);
	//  Clean up allocated memory
	NPN_ReleaseVariantValue(&functionval);
	NPN_ReleaseVariantValue(&rval);
	NPN_ReleaseObject(sWindowObj);
}

NPBool CMyScannerPlugin::init(NPWindow* pNPWindow)
{
  DEBUGMSG(TRUE, (L"init called '0x%x'...\n", pNPWindow));
  if(pNPWindow == NULL)
    return FALSE;

  if (SetWindow(pNPWindow))
      m_bInitialized = TRUE;

  if (SetDocument(pNPWindow))
	  m_bInitialized = TRUE;
  
  return m_bInitialized;
}

void CMyScannerPlugin::shut()
{

	DEBUGMSG(TRUE, (L"shut called ...\n"));
	m_bInitialized = FALSE;
}

NPBool CMyScannerPlugin::isInitialized()
{
	DEBUGMSG(TRUE, (L"isInitialized called ...\n"));
  return m_bInitialized;
}

NPError CMyScannerPlugin::SetDocument(NPWindow* pNPWindow){
	DEBUGMSG(TRUE, (L"SetDocument called '0x%x'...\n", pNPWindow));
	if(pNPWindow == NULL)
		return FALSE;

	NPObject *sWindowObj;
	NPN_GetValue(m_pNPInstance, NPNVWindowNPObject, &sWindowObj);
	/* Get document object */ 
	NPVariant MySensorVar; 
	NPIdentifier id = NPN_GetStringIdentifier("MyScanner"); 
	NPN_GetProperty(m_pNPInstance, sWindowObj, id, &MySensorVar);	//HGO 
	//NPObject* 
	m_document = NPVARIANT_TO_OBJECT(MySensorVar);
  return TRUE;
}

NPError CMyScannerPlugin::SetWindow(NPWindow* pNPWindow)
{
  DEBUGMSG(TRUE, (L"SetWindow called '0x%x'...\n", pNPWindow));
  if(pNPWindow == NULL)
    return FALSE;

  m_Window = pNPWindow;
  NPWindow* window = pNPWindow;
  return TRUE;
}


NPObject* CMyScannerPlugin::GetScriptableObject()
{
  DEBUGMSG(TRUE, (L"GetScriptableObject called...\n"));
	return NULL;
}


int KSCANAPI KScanReadCallBack(LPVOID pRead)
{
	int			Status;
	Status = ((PKSCANREAD)pRead)->out_Status;
	MyScannerPluginObject* lpCls = (MyScannerPluginObject*)((PKSCANREAD)pRead)->pUserData;


	switch(Status) {
	case KSCAN_RET_TIMEOUT:			// Timed out.
		Status = 0;
#ifdef DEBUG
		lpCls->MessageToUser("Error : Timed out");
#endif
		break;
	case KSCAN_RET_USER_CANCEL:		// User called stop
		Status = 0;
#ifdef DEBUG
		lpCls->MessageToUser("Error : User called stop");
#endif
		break;
	case KSCAN_RET_NORMAL:		// Barcode was read
	case KSCAN_RET_TYPE_UNKNOWN: 
		Status = 0;
		if(((PKSCANREAD)pRead)->out_Type == -1)
			break;
		if(((PKSCANREAD)pRead)->out_Status == KSCAN_RET_NORMAL)
		{
			if(!lpCls->m_bBarcodeReading)return 0;

			BARCODEDATA barcodedata;
			char str[32];
			sprintf(barcodedata.data, "%s", ((PKSCANREAD)pRead)->out_Barcode);	
			sprintf(barcodedata.time, "%s", getLocalTimeStrA((char*)str));
			sprintf(barcodedata.type, "%s", lpCls->GetBarCodeTypeString(((PKSCANREAD)pRead)->out_Type));

			PostMessage(lpCls->hWindow, WM_SCANNER_NAVIGATE, (WPARAM)lpCls, (LPARAM)&barcodedata);
	
			//LK lpCls->LoadResourceSound();

		}		
		lpCls->m_bBarcodeReading = FALSE;
		break;
		
	case KSCAN_RET_BAR_NOTFOUND:	// Not yet found - Continue.
	case KSCAN_RET_NORMAL_SWEEP:	// barcode was read, and security criteria was not met yet
#ifdef DEBUG
		lpCls->MessageToUser("Error : Not yet found - Continue.");
#endif
		Status = 1;
		break;
		
	default:	// Other error.
		Status = 0;					// just continue reading until barcode was read with security criteria met
		break;
	}	
	return Status;
}

