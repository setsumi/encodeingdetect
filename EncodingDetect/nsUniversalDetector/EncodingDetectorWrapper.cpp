#include "EncodingDetectorWrapper.h"
#include "EncodingDetector.h"
#include <stdio.h>
#include <Windows.h>
#ifdef __cplusplus
extern "C" {
#endif

/*
#include "..\StrFunc.h"
#define xstrcmpiA
#define  xatoiW
#define  xitoaW
#define xuitoaW
#define dec2hexW
#define xstrcpynW
#define xstrlenW
#define xprintfW
#include "..\StrFunc.h"
*/

int EncodingDetector_Detect(const char* aBuf, unsigned int aLen,unsigned int lang,char* encoding,int nDefaultASCIICodePage) {
    //char encoding[128];
	int result;
	//encoding[0] = '\0';
	//char *pencoding = encoding;
	EncodingDetector detector(lang);
	result=detector.DetectString(aBuf,aLen,encoding);
	//
	if(EncodingDetector_ASCII == result || result ==  EncodingDetector_NO_DATA)
	{
		return nDefaultASCIICodePage;
	}
	if(result <=EncodingDetector_NO_SURE) //found
	{
		return 0;
	}
	 //xstrcpynW(wszError,L"Can not detect!",100);
	return -1;
}
/*
void  EncodingDetector_delete(CEncodingDetector *t) {
        EncodingDetector *detector = (EncodingDetector *)t;
        delete detector;
}
*/
#ifdef __cplusplus
}
#endif