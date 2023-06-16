#ifndef __ENCODINGDETECT__H
#define	__ENCODINGDETECT__H
#include <Windows.h>


#define CODEPAGE_SHIFT_JIS	932
#define CODEPAGE_EUC_JP		20932
#define CODEPAGE_JIS		50222
#define CODEPAGE_UTF8		65001
#define CODEPAGE_ASCII		20127
#define CODEPAGE_ISO2022CN	50227
#define CODEPAGE_ISO2022KR	50225

#define CODEPAGE_UTF_32LE	12000
#define CODEPAGE_UTF_32BE	12001
#define CODEPAGE_UTF_16LE	1200
#define CODEPAGE_UTF_16BE	1201
#define CODEPAGE_LATIN1		1252
#define CODEPAGE_ASCI		437
/*
return:[int]codepage
*/
int DetectJaEncoding(const unsigned char* pBuffer,UINT_PTR bSize);
int DetectISO_2002Encoding(const unsigned char* pBuffer,UINT_PTR bSize);
#endif //__ENCODINGDETECT__H
