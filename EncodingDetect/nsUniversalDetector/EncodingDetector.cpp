#include "EncodingDetector.h"
//#include <string.h>

EncodingDetector::EncodingDetector(PRUint32 aLanguageFilter)
 : nsUniversalDetector(aLanguageFilter)
{
	
}
EncodingDetector::~EncodingDetector() 
{
}
int EncodingDetector::Detecting(const char* aBuf, PRUint32 aLen)
{
	if(mDone) return EncodingDetector_OK;
	if(NS_ERROR_OUT_OF_MEMORY== this->HandleData(aBuf, aLen))
	{
		return NS_ERROR_OUT_OF_MEMORY;
	}
	return EncodingDetector_NOT_FOUND;

}
int EncodingDetector::Detected(char* encoding)
{
	this->DataEnd();
	if(!mGotData)
	{
		return EncodingDetector_NO_DATA;
	}
	if(mDetectedCharset)
	{
		//strco
		//strcpy(encoding ,mDetectedCharset);
		int i;
		char *pEnc = encoding;
		//char *pcharset = (char*)mDetectedCharset;
		for(i=0;i<127;i++){
			if(!mDetectedCharset[i])
				break;
			*pEnc++=mDetectedCharset[i];
		}
		*pEnc='\0';
		return mDone?EncodingDetector_OK:EncodingDetector_NO_SURE;
	}
	if(mInputState==ePureAscii)
	{
		//encoding=nsnull;
		return EncodingDetector_ASCII;
	}
	return EncodingDetector_NOT_FOUND;
}

int EncodingDetector::DetectString(const char* aBuf, PRUint32 aLen,char* encoding)
{
	Detecting(aBuf,aLen);
	return Detected(encoding);
}
void EncodingDetector::Report(const char* aCharset) {
	if(!mDone)
	{
		mDetectedCharset =aCharset;
	}
}
