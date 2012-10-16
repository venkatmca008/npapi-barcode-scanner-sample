//MyScannerPlugin.cpp

#include "MyScannerPlugin.h"
#include "npfunctions.h"

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

	//  Setting the default properties of the created object.
	obj->m_bBarcodereadEnabled=FALSE;
	obj->m_bScannAhead=FALSE;
	obj->m_hScannRead=NULL;		// event notifier to let scan thread know we did receive the scan msg
	obj->m_hHandleBarcodeRead = CreateEvent(NULL, FALSE, FALSE, NULL);
	obj->m_hStopScannerMonitor = CreateEvent(NULL, FALSE, FALSE, NULL);
	obj->m_hBarCodeReaderHandle = NULL;
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
		CloseHandle(CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)BarcodeReadThread, obj, 0, NULL));
		//SetScannerEnable(FALSE);
		bRetVal = true;
	}
  return bRetVal;
}

void MyScannerPluginObject::SetScannerEnable(BOOL bEnable){
	if(bEnable)
		DEBUGMSG(1, (L"SetScannerEnable: Scanner will be enabled\n"));
	else
		DEBUGMSG(1, (L"SetScannerEnable: Scanner will be disabled\n"));
	if (this->m_hBarCodeReaderHandle != NULL){
		ITCSCAN_SetScannerEnable((INT32) this->m_hBarCodeReaderHandle, (INT32) bEnable);
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
		if(SUCCEEDED(ITCSCAN_GetScannerEnable((INT32) this->m_hBarCodeReaderHandle, (INT32*)bEnabled))){
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
		ITCSCAN_CancelRead((INT32)this->m_hBarCodeReaderHandle, TRUE, &dwDiscardedMsgs, &dwDiscardedBytes);
		DEBUGMSG(1, (L"ITCSCAN_CancelRead: disMsgs=%i, disBytes=%i\n", dwDiscardedMsgs, dwDiscardedBytes));
	}
}

//  Called when the JS Object is destroyed
MyScannerPluginObject::~MyScannerPluginObject()
{
	this->ScannerCancelRead();
	SetEvent(m_hStopScannerMonitor);
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
		this->m_bBarcodereadEnabled=GetScannerEnable();
		DEBUGMSG(TRUE, (L"...returning '%i'\n", this->m_bBarcodereadEnabled));
		BOOLEAN_TO_NPVARIANT(this->m_bBarcodereadEnabled, *result);
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
			this->m_bBarcodereadEnabled=TRUE;
			this->SetScannerEnable(TRUE);
		}
		else
		{
			DEBUGMSG(TRUE, (L"... arg is false\n"));
			//  JAVASCRIPT: mySensor.enablebarcode=false;
			this->SetScannerEnable(FALSE);
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

	// mySensor.enablebarcode
	if (methodName == sBarcodeEnabled_id)
	{
		//  Expect one argument which is a boolean (start / stop)
		if (argCount == 1 && NPVARIANT_IS_BOOLEAN(args[0]))
		{
			if (NPVARIANT_TO_BOOLEAN(args[0]))
			{
				//  mySensor.enablebarcode(true);
				//  Create a thread to monitor barcode reading			
				m_bBarcodereadEnabled=TRUE;
				SetScannerEnable(TRUE);
			}
			else
			{
				//  mySensor.enablebarcode(false);
				m_bBarcodereadEnabled=FALSE;
				SetScannerEnable(FALSE);
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

//thread to monitor barcode reader
DWORD MyScannerPluginObject::BarcodeReadThread(LPVOID lpParameter){
	MyScannerPluginObject* pSensor = (MyScannerPluginObject*)lpParameter;
	bool exitThread = false;
	
	HANDLE hWaitHandles[2];
	hWaitHandles[0] = pSensor->m_hStopScannerMonitor;
	hWaitHandles[1] = pSensor->m_hScannRead;

	DEBUGMSG(TRUE, (L"Barcode Read Thread Starting\n"));

		// Initialize the buffer.
		pSensor->m_gOneScan.rgbDataBuffer = pSensor->m_bScanDataBuffer;
		pSensor->m_gOneScan.dwDataBufferSize = MAX_SCAN_BUFFER;
		//	either periodically try a scan or wait infinite. If INFINITE, use ITCSCAN_CancelRead
		//  to stop read
		pSensor->m_gOneScan.dwTimeout = INFINITE;// 500;	
		
		//HANDLE hBarCodeReaderHandle=NULL;
		if(pSensor->m_hBarCodeReaderHandle != NULL)
		{
			DWORD dwDiscardedMsgs, dwDiscardedBytes;
			ITCSCAN_CancelRead((INT32)pSensor->m_hBarCodeReaderHandle, TRUE, &dwDiscardedMsgs, &dwDiscardedBytes);
			ITCSCAN_Close((INT32) pSensor->m_hBarCodeReaderHandle);
		}
		pSensor->m_hBarCodeReaderHandle=NULL;

		// Open the scanner.
		if ( SUCCEEDED( ITCSCAN_Open( (INT32*)&pSensor->m_hBarCodeReaderHandle, TEXT("default") ) ) )
		{
			DEBUGMSG(TRUE, (L"Barcode Scanner opened...\n"));
			while ( TRUE) //pSensor->m_bBarcodereadEnabled )
			{
				DEBUGMSG(TRUE, (L"Barcode Scanner SyncRead...\n"));
				if ( SUCCEEDED( ITCSCAN_SyncRead( (INT32)pSensor->m_hBarCodeReaderHandle, &pSensor->m_gOneScan) ) )
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
					strncpy(szBarcodeReading, (char*)pSensor->m_gOneScan.rgbDataBuffer, pSensor->m_gOneScan.dwBytesReturned);
					//sprintf(szBarcodeReading, "%s", pSensor->m_gOneScan.rgbDataBuffer);
					
					TCHAR* szBarcodeReadingW;
					szBarcodeReadingW = (TCHAR*)malloc(pSensor->m_gOneScan.dwBytesReturned * sizeof(TCHAR));
					mbstowcs(szBarcodeReadingW, szBarcodeReading, pSensor->m_gOneScan.dwBytesReturned);
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
					sprintf(barcodedata.type, "%s", BarcodeTypeStr[pSensor->m_gOneScan.iSymbology]);	//STCIdentifiers.h

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
			ITCSCAN_Close( (INT32)pSensor->m_hBarCodeReaderHandle);
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
		
		SetEvent(pSensor->m_hScannRead);	//let the barcode read thread know
		return 0;
	}
	else if (message==WM_SCANNER_NAVIGATE){
		DEBUGMSG(TRUE, (L"NpapiProc Barcode called...\n"));
		MyScannerPluginObject* pSensor = (MyScannerPluginObject*)wparam;

		// invoke ScannerNavigate with data, type, time
		BARCODEDATA* barcodedata = (BARCODEDATA*)lparam;
		
		pSensor->MessageToUser(barcodedata);
		
		SetEvent(pSensor->m_hScannRead);	//let the barcode read thread know
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

	NPObject *mySensorObject =NPN_CreateObject(m_pNPInstance,GET_NPOBJECT_CLASS(MyScannerPluginObject));
	NPVariant v;
	OBJECT_TO_NPVARIANT(mySensorObject, v);
	NPIdentifier n = NPN_GetStringIdentifier("MyScanner");
	NPN_SetProperty(m_pNPInstance, sWindowObj, n, &v);

	NPN_ReleaseObject(mySensorObject);
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
