// MyScannerPlugin.h

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#define MAX_SCAN_BUFFER	2048
#include "BarcodeData.h"	//a struct to hold barcode data

#include "./common/npapi.h"
#include "./common/npruntime.h"
#include "./common/nullplugin.h"
#include "./common/ScriptablePluginObjectBase.h"

//LK #include "itcscan.h"
//LK#pragma comment (lib, "itcscan.lib")

#include "KScanBar.h"
#pragma comment (lib, "KScanBar.lib")

#define WM_HAVE_SCAN	WM_USER + 2
#define WM_SCANNER_NAVIGATE	WM_USER + 3

int KSCANAPI KScanReadCallBack(LPVOID pRead);

class CMyScannerPlugin
{
	private:
	  NPP m_pNPInstance;

	  NPObject* m_document;

	  NPWindow * m_Window;
	  NPStream * m_pNPStream;
	  NPBool m_bInitialized;
	  NPObject *m_pScriptableObject;
//	  PluginInstance nullPluginInstance;

	public:
	  CMyScannerPlugin(NPP pNPInstance);
	  ~CMyScannerPlugin();

	  NPBool init(NPWindow* pNPWindow);
	  void shut();
	  NPBool isInitialized();
	  
	  NPError SetWindow(NPWindow* pNPWindow);
	  NPError SetDocument(NPWindow* pNPWindow);

	  NPObject *GetScriptableObject();
};

class MyScannerPluginObject : public ScriptablePluginObjectBase
{
	public:
		MyScannerPluginObject(NPP npp) : ScriptablePluginObjectBase(npp)  { }
		virtual ~MyScannerPluginObject();

		virtual bool Construct(const NPVariant *args, uint32_t argCount, NPVariant *result);
		char* npStrDup(const char* sIn);	//helper function

		virtual bool HasMethod(NPIdentifier name);
		virtual bool HasProperty(NPIdentifier name);
		virtual bool GetProperty(NPIdentifier name, NPVariant *result);
		virtual bool SetProperty(NPIdentifier name, const NPVariant *value);
		virtual bool Invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
		virtual bool InvokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result);

		//a way to invoke javascript from the plugin code
		void MessageToUser(char* szMessage);
		void MessageToUser(BARCODEDATA* stBarcodeData);

/*LK
		//enable disable scanner
		void SetScannerEnable(BOOL bEnable);
		BOOL GetScannerEnable();
		void ScannerCancelRead();

		/// the barcode reader thread
		static DWORD BarcodeReadThread(LPVOID lpParameter);	
LK*/
		//the plugin internal WndProc
		static LRESULT CALLBACK NpapiProc (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

		// Scanner Functions
		void SetScannerSettings();
		char* GetBarCodeTypeString(int nType);
		void Scanner_Enable();
		void Scanner_Disable();

		// Scanner Object
		CKScan m_KScan;

		KSCANREAD	 kRead;
		KSCANREADEX	 kReadEx;
		KSCANREADEX2 kReadEx2;

		// Scanner Members
		BOOL m_bBarcodeEnabled;
		BOOL m_bBarcodeReading;
		

/*LK
		//handle to be able to stop background thread
		HANDLE m_hStopScannerMonitor;

		//handle to the barcode scanner
		HANDLE m_hBarCodeReaderHandle;
		//to store the last scanned data for later access
		BYTE m_bScanDataBuffer[MAX_SCAN_BUFFER];	//helper var

		//property to control Scanner Enabled
		BOOL m_bBarcodereadEnabled;
		BOOL m_bScannAhead;
		HANDLE m_hScannRead;

//LK		READ_DATA_STRUCT m_gOneScan;
		char m_gOneScan[10];
		HANDLE m_hHandleBarcodeRead;
LK*/
		char m_szBarcodedata[MAX_SCAN_BUFFER];

		HWND hWindow;
	
};


#endif // __PLUGIN_H__
