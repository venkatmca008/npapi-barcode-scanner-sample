//BarcodeData.h

#ifndef _BARCODEDATA_H_
#define _BARCODEDATA_H_

#ifndef MAX_SCAN_BUFFER
#define MAX_SCAN_BUFFER	2048
#endif

struct BARCODEDATA{
		char data[MAX_SCAN_BUFFER];
		char type[32];
		char time[32];
};

char* BarcodeTypeStr[];

#endif //_BARCODEDATA_H_