#ifndef __EncodingDetector_h__
#define __EncodingDetector_h__
#include "nscore.h"
#include "nsDebug.h"
#include "nsUniversalDetector.h"
//Define const
//enum DetectResult{DETEC_OK,DETNOT_SURE,NO_DATA,ERROR,NOT_FOUND};
#define EncodingDetector_ASCII -1
#define EncodingDetector_OK 0
#define EncodingDetector_NO_SURE 1
#define EncodingDetector_NO_DATA 2
#define EncodingDetector_ERROR 3
#define EncodingDetector_NOT_FOUND 4

class EncodingDetector : public nsUniversalDetector {
public:
	EncodingDetector(PRUint32 aLanguageFilter);
	virtual ~EncodingDetector();
	int Detecting(const char* aBuf, PRUint32 aLen);
	int Detected( char* encoding);

	int DetectString(const char* aBuf, PRUint32 aLen, char* encoding);
	//const char* DetectFile(const char* fileName,int* result,
	//const char* GetCharset();
	//PRBool IsDetected();
	
protected:
	void Report(const char* aCharset);
};

#endif
