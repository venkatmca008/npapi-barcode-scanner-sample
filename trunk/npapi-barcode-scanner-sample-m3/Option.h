#ifndef _OPTION_HEADER_
#define _OPTION_HEADER_

#include "Option_defs.h"

//////////////////////////////////////////////////////////////////
#pragma pack(push, 1)

typedef struct {
	ABLE             Enable;
	SUPP             Supp;
	XMIT_NUM_SYSTEM  XmitNumber;
	XMIT_CHECK_CHAR  XmitCheckDigit;
	FORMAT           Format; 
//	DWORD            dwSecurityLevel;
}DEC_UPCEAN;

typedef struct {
	ABLE     	            Enable;
	XMIT_ASCII 	     SetAscii;
	ABLE                       CDV;      
	XMIT_CHECK_CHAR XmitCheckDigit;
	ABLE     	            AsCode32;  
	ABLE     	            AsPZN;
	BYTE                      MinLength;
	BYTE                      MaxLength;
}DEC_CODE39;

typedef struct {
	ABLE             Enable;
	ABLE             AsUCCEAN128;
	BYTE             *FNC1_ASCII;   //2009.09.30
	BYTE                      MinLength;
	BYTE                      MaxLength;
}DEC_CODE128;

typedef struct {
	ABLE             Enable;
	BYTE                      MinLength;
	BYTE                      MaxLength;
}DEC_CODE93;

typedef struct {
	ABLE             Enable;
}DEC_CODE35;

typedef struct {
	ABLE					Enable;
	CHECK_DIGIT             CheckDigit;      
	XMIT_CHECK_CHAR			XmitCheckDigit;
	BYTE                    MinLength;
	BYTE                    MaxLength;
}DEC_CODE11;

typedef struct {
	ABLE     Enable;
//	LENGTH   LengthType;
	BYTE     MinLength;
	BYTE     MaxLength;
	ABLE     CDV;      
	XMIT_CHECK_CHAR XmitCheckDigit; 
	INT		KindofDecode;			// CODE25_KIND
}DEC_CODE25;

typedef struct {
	ABLE             Enable;
	XMIT_STARTSTOP   XmitStartStop;
	
	BYTE     MinLength;
	BYTE     MaxLength;
}DEC_CODABAR;

typedef struct {
	ABLE			Enable;
	ABLE                       CDV;      
	XMIT_CHECK_CHAR XmitCheckDigit;
	BYTE                       MinLength;
	BYTE                       MaxLength;
//	MOD_METHOD	     Mod;
}DEC_MSI;

typedef struct {
	ABLE			Enable;
	ABLE                       CDV;      
	XMIT_CHECK_CHAR XmitCheckDigit;
	BYTE                       MinLength;
	BYTE                       MaxLength;
}DEC_PLESSEY;

typedef struct {
	ABLE			Enable;
}DEC_GS1;

typedef struct {
	ABLE			Enable;
}DEC_GS1_LIMITED;

typedef struct {
	ABLE			Enable;
}DEC_GS1_EXPANDED;

typedef struct {
	ABLE			Enable;
	ABLE			OldStyle;
}DEC_TELEPEN;
/*
typedef struct {
	ABLE		 Enable;
	unsigned char   Prefix;
	unsigned char   Suffix1;
	unsigned char   Suffix2;
}DATA_FORMAT;
*/

#pragma pack(pop)
/////////////////////////////////////////////////////////
#endif 

