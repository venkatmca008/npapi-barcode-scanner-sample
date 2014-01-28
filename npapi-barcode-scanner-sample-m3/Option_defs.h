#ifndef _OPTION_DEFS_HEADER_
#define _OPTION_DEFS_HEADER_

typedef enum {
    DISABLE=0,
		ENABLE
}ABLE;

typedef enum {
    FULL_ASCII=0,
		STD_ASCII
}XMIT_ASCII;

/*
typedef enum {
    FIX_LENGTH=0,
		ANY_LENGTH
}LENGTH;


typedef enum {
    STRIP_CHAR=0,
		FILL_CHAR
}FILLER;
*/

typedef enum {
    NO_Supp=0,
		WITH_OR_WITHOUT
//		TWO_SUPP,
//		FIVE_SUPP
}SUPP;


typedef enum {
    XMIT_NUMBER=0,
		NO_XMIT_NUMBER
}XMIT_NUM_SYSTEM;


typedef enum {
    XMIT_CHECK_DIGIT=0,
		NO_XMIT_CHECK_DIGIT
}XMIT_CHECK_CHAR;

typedef enum {
    NO_XMIT=0,
		XMIT
}XMIT_STARTSTOP;

typedef enum {
    AS_UPCA=0,
		AS_EAN13,
		AS_UPCE,
		AS_EAN8,
		
		AS_CODE32,
		
		AS_UCCEAN128,
		AS_BOOKLAND
//		AS_COUPONCODE
}FORMAT;

typedef enum {
    ANGLE_WIDE=0,
		ANGLE_NARROW
}ENGINE_ANGLE;

typedef enum {
    FILTER_WIDE=0,
		FILTER_NARROW
}ENGINE_FILTER;


typedef enum {
	MODULO10 = 0,
		MODULO11,
		MODULO1010,
		MODULO1110
}MOD_METHOD;


typedef enum {
	CODE25KIND_INTER	= 0x01, // 1,
		CODE25KIND_MATRIX	= 0x02, // 2,
		CODE25KIND_DLOGIC	     = 0x04, // 4,
		CODE25KIND_INDUSTRY = 0x08,  // 8,
		CODE25KIND_IATA	     = 0x10,  //16,
		CODE25KIND_ITF14        = 0x20, // 32,
//		CODE25KIND_DPL	     = 0x40, // 64,		// Deutsche Post Leitcode
//		CODE25KIND_DPI	=   0x80, //128,		// Deutsche Post Identcode
//		CODE25KIND_GTIN14	=   0x00//256		// GS-14 (GTIN-14 Code)
}CODE25_KIND;

// Jiyong modified - spain
typedef enum {
    DIGIT0=0,
		DIGIT1,
		DIGIT2
}CHECK_DIGIT;

#endif //_OPTION_DEFS_HEADER_