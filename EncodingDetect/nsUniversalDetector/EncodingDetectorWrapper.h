#ifndef __EncodingDetectorWrapper__H
#define __EncodingDetectorWrapper__H

//typedef void CEncodingDetector;


#define FILTER_CHINESE_SIMPLIFIED  0x01
#define FILTER_CHINESE_TRADITIONAL 0x02
#define FILTER_JAPANESE            0x04
#define FILTER_KOREAN              0x08
#define FILTER_NON_CJK             0x10
#define FILTER_ALL                 0x1F
#define FILTER_CHINESE (FILTER_CHINESE_SIMPLIFIED | \
                           FILTER_CHINESE_TRADITIONAL)
#define FILTER_CJK (FILTER_CHINESE_SIMPLIFIED | \
                       FILTER_CHINESE_TRADITIONAL | \
                       FILTER_JAPANESE | \
                       FILTER_KOREAN)


#define CHARSET_NOTFOUND -2

#ifdef __cplusplus
extern "C" {
#endif

//int getEncodingFromString(const char * encodingAlias);
//CEncodingDetector * EncodingDetector_new(unsigned int filter);
//int EncodingDetector_Detect(const char* aBuf, unsigned int aLen,unsigned int lang,int defaultASCII,wchar_t *wszError);
int EncodingDetector_Detect(const char* aBuf, unsigned int aLen,unsigned int lang,char* encoding,int nDefaultASCIICodePage);
//void  EncodingDetector_delete(CEncodingDetector *t);
#ifdef __cplusplus
}
#endif

#endif