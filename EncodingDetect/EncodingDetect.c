#include "EncodingDetect.h"

#define bEscape	0x1B
int DetectISO_2002Encoding(const unsigned char* pBuffer,UINT_PTR bSize)
{
	unsigned char c1,c2,c3,c4;
	UINT_PTR uiBufferIndex;
	if(bSize < 3) return 0; //not iso-2002
	for (uiBufferIndex=0;uiBufferIndex < bSize - 2; uiBufferIndex++)
    {

        c1 = pBuffer[uiBufferIndex];
        c2 = pBuffer[uiBufferIndex + 1];
        c3 = pBuffer[uiBufferIndex + 2];
        if (c1 > 0x7F)
        {
            return 0;//not iso-2022
        }

        if (c1 == bEscape)
        {
			if(c2=='$')
			{
				if(c3=='@')					//JIS_0208 1978 
				{
					return CODEPAGE_JIS;
				}
				else if(c3=='B')			//JIS_0208 1983
				{
					return CODEPAGE_JIS;
				}
				else if(c3=='A')			// GB 2312-1980 (ISO-2022-JP-2)
				{
					return CODEPAGE_JIS;
				}
				else if(uiBufferIndex < (bSize - 3))
				{
					c4 = pBuffer[uiBufferIndex + 3];
					if(c3=='*' && c4=='H') //CNS 11643-1992
					{
						return CODEPAGE_ISO2022CN;
					}
					else if(c3=='(')
					{
						if(c4=='C')			//KS X 1001-1992 ISO-2022-JP-2.
						{
							return CODEPAGE_JIS;
						}
						else if(c4=='D')	//JIS X 0212-1990 ISO-2022-JP-1
						{
							return CODEPAGE_JIS;
						} 
						else if(c4=='O')	// JIS X 0213-2000 ISO-2022-JP-3
						{
							return CODEPAGE_JIS;
						}
						else if(c4=='P')	//JIS X 0213-2000 ISO-2022-JP-3
						{
							return CODEPAGE_JIS;
						}
						else if(c4=='Q')	//JIS X 0213-2000 ISO-2022-JP-2004
						{
							return CODEPAGE_JIS;
						}
					}
					else if(c3==')')
					{
						if(c4=='A')			//GB 2312-1980
						{
							return CODEPAGE_ISO2022CN;
						}
						else if(c4=='C')	// KS X 1001-1992
						{
							return CODEPAGE_ISO2022KR;
						}
						else if(c4=='E')	//ISO-IR-165 ISO-2022-CN-EXT
						{
							return CODEPAGE_ISO2022CN;
						}
						else if(c4=='G')	//CNS 11643-1992
						{
							return CODEPAGE_ISO2022CN;
						}
					}
					else if(c3=='+')
					{
						if(c4=='I')			//CNS 11643-1992 ISO-2022-CN-EXT
						{
							return CODEPAGE_ISO2022CN;
						}
						else if(c4=='J')	//CNS 11643-1992 ISO-2022-CN-EXT
						{
							return CODEPAGE_ISO2022CN;
						}
						else if(c4=='K')	//CNS 11643-1992 ISO-2022-CN-EXT
						{
							return CODEPAGE_ISO2022CN;
						}
						else if(c4=='L')	//CNS 11643-1992 ISO-2022-CN-EXT
						{
							return CODEPAGE_ISO2022CN;
						}
						else if(c4=='M')	//CNS 11643-1992 ISO-2022-CN-EXT
						{
							return CODEPAGE_ISO2022CN;
						}
					}
				}
			}
			else if(c2=='(')
			{
				if(c3=='B')	//ASCII 
				{
					return CODEPAGE_JIS;	
				}
				else if(c3=='I')	//JIS X 0201-1976 Kana set 
				{
					return CODEPAGE_JIS;
				}
				else if(c3=='J')	//JIS X 0201-1976
				{
					return CODEPAGE_JIS;
				}
			}
			else if(c2=='.')
			{
				if(c3=='A')	//ISO/IEC 8859-1
				{
					return CODEPAGE_JIS;
				}
				else if(c3=='F')//ISO/IEC 8859-7
				{
					return CODEPAGE_JIS;
				}
			}
      
        }
    }
	return 0;
}


/*
return:
-1:binary
codepage
*/
int DetectJaEncoding(const unsigned char* pBuffer,UINT_PTR bSize)
{

	UINT_PTR uiBufferIndex=0,utf8,euc,sjis,hanaku,euc_zen;
	//int isContinue = 1;
	unsigned char c1,c2,c3;//,c4;
//	int nCodePage;
	
	//firstNonASCIIIndex = uiBufferIndex;
	//count sjis chars
	sjis = 0;
	hanaku = 0;
	hanaku = 0;
	c2 = 0;
	for (uiBufferIndex = 0; uiBufferIndex < bSize; uiBufferIndex++)
    {
        c1 = pBuffer[uiBufferIndex];
        
        if (c1 < 0x80)
			continue; //asccii
		else if (c1 >= 0xA1 && c1 <= 0xDF) //hankaku kata
			hanaku++;
		else if(uiBufferIndex < (bSize-1))
		{
			c2 = pBuffer[uiBufferIndex + 1];
			if (((0x81 <= c1 && c1 <= 0x9F) || (0xE0 <= c1 && c1 <= 0xFC)) &&
            ((0x40 <= c2 && c2 <= 0x7E) || (0x80 <= c2 && c2 <= 0xFC)))
			{
				//SJIS_C
				sjis += 2;
				uiBufferIndex++;
			}
		}
        
    }
	//end count sjis
	//count euc
	euc = 0;
	euc_zen=0;
	for (uiBufferIndex = 0; uiBufferIndex < bSize - 1; uiBufferIndex++)
    {
		c1 = pBuffer[uiBufferIndex];
        c2 = pBuffer[uiBufferIndex + 1];
        if (((0xA1 <= c1 && c1 <= 0xFE) && (0xA1 <= c2 && c2 <= 0xFE)) ||
            (c1 == 0x8E && (0xA1 <= c2 && c2 <= 0xDF)))
        {
            //EUC_C
            //EUC_KANA
            euc += 2;
            uiBufferIndex++;
			if ((c1 == 0xa4 && c2 >= 0xa1 && c2 <= 0xf3)
                        || (c1 == 0xa5 && c2 >= 0xa1 && c2 <= 0xf6))
						euc_zen +=2;
        }
        else if (uiBufferIndex < bSize - 2)
        {
            c3 = pBuffer[uiBufferIndex + 2];
            if (c1 == 0x8F && (0xA1 <= c2 && c2 <= 0xFE) &&
                (0xA1 <= c3 && c3 <= 0xFE))
            {
                //EUC_0212
                euc += 3;
                uiBufferIndex += 2;
            }
        }
    }
	//End euc
	//count utf-8
	utf8 = 0;
	for (uiBufferIndex = 0; uiBufferIndex < bSize - 1; uiBufferIndex++)
    {
        c1 = pBuffer[uiBufferIndex];
        c2 = pBuffer[uiBufferIndex + 1];
        if ((0xC0 <= c1 && c1 <= 0xDF) && (0x80 <= c2 && c2 <= 0xBF))
        {
            //UTF8
            utf8 += 2;
            uiBufferIndex++;
        }
        else if (uiBufferIndex < bSize - 2)
        {
            c3 = pBuffer[uiBufferIndex + 2];
            if ((0xE0 <= c1 && c1 <= 0xEF) && (0x80 <= c2 && c2 <= 0xBF) &&
                (0x80 <= c3 && c3 <= 0xBF))
            {
                //UTF8
                utf8 += 3;
                uiBufferIndex += 2;
            }
        }
    }
	//end utf-8
	 if((utf8 + sjis+euc+hanaku)==0) return 0;
	 if (utf8 > euc && utf8 > sjis)
     {
        //UTF8
		return CODEPAGE_UTF8;
     }
	 if (sjis == 0 && hanaku >= euc)//hankaku shiftjis
		 return CODEPAGE_SHIFT_JIS;
	  if (euc>0)
      {
		  if(hanaku > 0)
			sjis += hanaku-2;
		   if ((euc_zen * 100 / euc) >= 25 || euc > sjis)
			   return CODEPAGE_EUC_JP;
      }
      /*if (euc > sjis)
      {
		  return CODEPAGE_EUC_JP;
      }*/

	 return CODEPAGE_SHIFT_JIS;
}