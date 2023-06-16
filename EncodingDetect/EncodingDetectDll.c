#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <richedit.h>
#include "AkelEdit.h"
#include "AkelDLL.h"
#include "resource.h"
#include "EncodingDetect.h"

//#include "DefStrFunc.h"
#include "nsUniversalDetector\EncodingDetectorWrapper.h"
#include "x64Func.h"
#define ReadFile64
#include "x64Func.h"

#include "StrFunc.h"
#define xstrcmpiA
#define xstrcmpiW
#define  xatoiW
#define  xitoaW
#define xuitoaW
#define dec2hexW
#define xstrcpynW
#define xstrlenW
#define xprintfW
#include "StrFunc.h"

void __declspec(dllexport) DllAkelPadID(PLUGINVERSION *pv)
{
	pv->dwAkelDllVersion=AKELDLL;
	pv->dwExeMinVersion3x=MAKE_IDENTIFIER(-1, -1, -1, -1);
	pv->dwExeMinVersion4x=MAKE_IDENTIFIER(4, 7, 0, 0);
	pv->pPluginName="EncodingDetect";
}

//*****Define****/
#define MSG_FILE_WILL_BE_REOPENED 68
#define BUFFER_SIZE 1024
#define APP_MAIN_TITLEW             L"AkelPad"
#define DEFAULT_DETECTSIZE 2048
#define MOT_DWORD       0x01  
#define MSG_ERROR_NOT_ENOUGH_MEMORY 4
#define MSG_ERROR_BINARY			5
#define MSG_CANNOT_OPEN_FILE		59

#define PLUGINNAME L"EncodingDetect"
#define ENC_TYPE_ASCII 0
#define ENC_TYPE_ISO_2002 1
#define ENC_TYPE_NON_ASCII 2

#define STR_UNICODE_UTF16LEW L"1200  (UTF-16 LE)"
#define STR_UNICODE_UTF16BEW L"1201  (UTF-16 BE)"
//#define STR_UNICODE_UTF8W, nLen);
//    else if (nCodePage == CP_UNICODE_UTF7)
//      xstrcpynW(wszCodePage, STR_UNICODE_UTF7W, nLen);
#define STR_UNICODE_UTF32LEW L"12000  (UTF-32 LE)"
#define STR_UNICODE_UTF32BEW L"12001  (UTF-32 BE)"

/***struct****/
struct
{
	int nNsLangID;
	wchar_t * name;
}ENC_DetectTypes[]=
{
	{0,L"Japanese/ISO-2002(not using universal charset detector)"},
	{FILTER_JAPANESE,L"Japanese"},
	{FILTER_CHINESE_SIMPLIFIED,L"Chinese simplified"},
	{FILTER_CHINESE_TRADITIONAL,L"Chinese traditional"},
	{FILTER_CHINESE,L"Chinese"},
	{FILTER_KOREAN,L"Korean"},
	{FILTER_CJK,L"CJK"},
	{FILTER_NON_CJK,L"Non CJK"},
	{FILTER_ALL,L"All"}
};

/***global variables***/
/*---handle--*/
HINSTANCE hLangLib = NULL;
HINSTANCE hInstanceEXE = NULL;
HWND hMainWnd = NULL;
HANDLE hHeap = NULL;

/*--Options--*/
int nDetectType = 0; //not using UniversalDetector
int nDetectSize = 0;
int nDefaultCodepage = 0; //akelpad's default codepage
int nDefaultASCIICodepage = 0;
int nCodePageForNFOFile = 0; //UAS EOM ASCI
BOOL bSaveCodepages = FALSE;

WNDPROCDATA *NewMainProcData=NULL;
BOOL bInAutoMode = FALSE;

/****flags***/
int bIsRunning = 0;
BOOL bIsIntitCommon = FALSE;

//****akepad function*****/
int __IsFileW(const wchar_t *wpFile)
{
	DWORD dwAttr;

	dwAttr=GetFileAttributesW(wpFile);
	if (dwAttr == INVALID_FILE_ATTRIBUTES)
		return ERROR_INVALID_HANDLE;
	if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
		return ERROR_DIRECTORY;
	return ERROR_SUCCESS;
}

__inline int LoadAkelPadMsg(unsigned int uid, wchar_t *wbuf,int maxLen){
	int result;
	if(hLangLib && (result = LoadStringW(hLangLib,uid,wbuf,maxLen)))
	{
		return result;
	}
	if(result=LoadStringW(hInstanceEXE,uid,wbuf,maxLen))
	{
		return result;
	}else{

		*wbuf = L'\0';
		return 0;
	}
}

LPVOID API_HeapAlloc(DWORD dwFlags, SIZE_T dwBytes)
{
	return HeapAlloc(hHeap, dwFlags, dwBytes);
}

LPVOID API_HeapAllocMSG(DWORD dwFlags, SIZE_T dwBytes)
{
	LPVOID result;
	HANDLE hHeap = GetProcessHeap();
	if(!(result=HeapAlloc(hHeap, dwFlags, dwBytes))){
		wchar_t wbuf[1024];
		LoadAkelPadMsg(MSG_ERROR_NOT_ENOUGH_MEMORY, wbuf, 1024);
		MessageBoxW(hMainWnd,wbuf, L"Encoding Detect", MB_OK|MB_ICONERROR);

	}
	return result;
}

BOOL API_HeapFree(DWORD dwFlags, LPVOID lpMem)
{
	return HeapFree(hHeap, dwFlags, lpMem);
}

/**init functions**/
LRESULT CALLBACK NewMainProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitMain()
{
	bInAutoMode = TRUE;
	NewMainProcData = NULL;
	SendMessage(hMainWnd, AKD_SETMAINPROC, (WPARAM)NewMainProc, (LPARAM)&NewMainProcData);
}
void UnInitMain()
{
	bInAutoMode = FALSE;
	if(NewMainProcData)
	{
		SendMessage(hMainWnd, AKD_SETMAINPROC, (WPARAM)NULL, (LPARAM)&NewMainProcData);
		NewMainProcData=NULL;
	}
}
//Entry point
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{

	}
	else if (fdwReason == DLL_THREAD_ATTACH)
	{
	}
	else if (fdwReason == DLL_THREAD_DETACH)
	{
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
		//MessageBoxW(NULL,L"dll unload",L"Test",MB_OK);
		if (bInAutoMode)
		{
			UnInitMain();
		}
	}
	return TRUE;
}

void ReadOptions()
{
	PLUGINOPTION po;
	HANDLE hOptions;
	//read options of akelpad
	if (hOptions=(HANDLE)SendMessageW(hMainWnd, AKD_BEGINOPTIONSW, POB_READ,0))
	{
		po.pOptionName=L"CodepageRecognitionBuffer";
		po.dwType=MOT_DWORD;
		po.lpData=(LPBYTE)&nDetectSize;
		po.dwData=sizeof(DWORD);
		if(!SendMessageW(hMainWnd, AKD_OPTION, (WPARAM)hOptions, (LPARAM)&po))
		{
			nDetectSize = BUFFER_SIZE; //set default
		}

		po.pOptionName=L"DefaultCodepage";
		po.lpData=(LPBYTE)&nDefaultCodepage;
		if(!SendMessageW(hMainWnd, AKD_OPTION, (WPARAM)hOptions, (LPARAM)&po))
		{
			nDefaultCodepage = CODEPAGE_LATIN1; //set default
		}
		po.pOptionName=L"SaveCodepages";
		po.lpData=(LPBYTE)&bSaveCodepages;
		if(!SendMessageW(hMainWnd, AKD_OPTION, (WPARAM)hOptions, (LPARAM)&po))
		{
			bSaveCodepages = FALSE; //set default
		}
		SendMessageW(hMainWnd, AKD_ENDOPTIONS, (WPARAM)hOptions, 0);
	}
	else
	{
		nDetectSize = BUFFER_SIZE; //set default
		nDefaultCodepage = CODEPAGE_LATIN1; //set default
		bSaveCodepages = FALSE; //set default
	}
	//read plugin's options
	if (hOptions=(HANDLE)SendMessageW(hMainWnd, AKD_BEGINOPTIONSW, POB_READ,(LPARAM)PLUGINNAME))
	{
		po.pOptionName=L"DetectType";
		//po.dwType=MOT_DWORD;
		po.lpData=(LPBYTE)&nDetectType;
		//po.dwData=sizeof(DWORD);
		if(!SendMessageW(hMainWnd, AKD_OPTION, (WPARAM)hOptions, (LPARAM)&po) || nDetectType <0 || nDetectType > 8)
		{
			nDetectType = 0; //set default
		}

		po.pOptionName=L"DefaultASCIICodepage";
		po.lpData=(LPBYTE)&nDefaultASCIICodepage;
		if(!SendMessageW(hMainWnd, AKD_OPTION, (WPARAM)hOptions, (LPARAM)&po))
		{
			nDefaultASCIICodepage = CODEPAGE_LATIN1; //set default
		}
		po.pOptionName=L"CodePageForNFOFile";
		po.lpData=(LPBYTE)&nCodePageForNFOFile;
		if(!SendMessageW(hMainWnd, AKD_OPTION, (WPARAM)hOptions, (LPARAM)&po))
		{
			nCodePageForNFOFile = CODEPAGE_ASCI; //set default
		}

		SendMessageW(hMainWnd, AKD_ENDOPTIONS, (WPARAM)hOptions,(LPARAM)PLUGINNAME);
	}
	else
	{

		nDetectType = 0; //set default
		nDefaultASCIICodepage = CODEPAGE_LATIN1; //set default
		nCodePageForNFOFile = CODEPAGE_ASCI; 

	}
}

void SaveOptions()
{
	PLUGINOPTION po;
	HANDLE hOptions;
	po.dwData=sizeof(DWORD);
	po.dwType=MOT_DWORD;
	//save plugin's options
	if (hOptions=(HANDLE)SendMessageW(hMainWnd, AKD_BEGINOPTIONSW, POB_SAVE,(LPARAM)PLUGINNAME))
	{
		po.pOptionName=L"DetectType";
		
		po.lpData=(LPBYTE)&nDetectType;
		
		SendMessageW(hMainWnd, AKD_OPTION, (WPARAM)hOptions, (LPARAM)&po);

		po.pOptionName=L"DefaultASCIICodepage";
		po.lpData=(LPBYTE)&nDefaultASCIICodepage;
		SendMessageW(hMainWnd, AKD_OPTION, (WPARAM)hOptions, (LPARAM)&po);

		po.pOptionName=L"CodePageForNFOFile";
		po.lpData=(LPBYTE)&nCodePageForNFOFile;
		SendMessageW(hMainWnd, AKD_OPTION, (WPARAM)hOptions, (LPARAM)&po);

		SendMessageW(hMainWnd, AKD_ENDOPTIONS, (WPARAM)hOptions,(LPARAM)PLUGINNAME);
	}
}

void InitCommon(PLUGINDATA *pd)
{
	hMainWnd = pd->hMainWnd;
	hInstanceEXE = pd->hInstanceEXE;
	hHeap = GetProcessHeap();
	if(!pd->wszLangModule[0] || !(hLangLib=GetModuleHandleW(pd->wszLangModule)))
	{
		hLangLib = NULL;
	}
	ReadOptions();
	bIsIntitCommon = TRUE;
	
}

/***setting***/

BOOL GetCodePageName(int nCodePage, wchar_t *wszCodePage, int nLen)
{
  CPINFOEXW CPInfoExW;

  if (nCodePage)
  {
	  
	  if (nCodePage == CODEPAGE_UTF_16LE)
      xstrcpynW(wszCodePage, STR_UNICODE_UTF16LEW, nLen);
    else if (nCodePage == CODEPAGE_UTF_16BE)
      xstrcpynW(wszCodePage, STR_UNICODE_UTF16BEW, nLen);
  //  else if (nCodePage == CP_UNICODE_UTF8)
  //    xstrcpynW(wszCodePage, STR_UNICODE_UTF8W, nLen);
//    else if (nCodePage == CP_UNICODE_UTF7)
//      xstrcpynW(wszCodePage, STR_UNICODE_UTF7W, nLen);
    else if (nCodePage == CODEPAGE_UTF_32LE)
      xstrcpynW(wszCodePage, STR_UNICODE_UTF32LEW, nLen);
    else if (nCodePage == CODEPAGE_UTF_32BE)
      xstrcpynW(wszCodePage, STR_UNICODE_UTF32BEW, nLen);
    else
    {
	
      if (GetCPInfoExW (nCodePage, 0, &CPInfoExW) && nCodePage == xatoiW(CPInfoExW.CodePageName, NULL))
        xstrcpynW(wszCodePage, CPInfoExW.CodePageName, nLen);
      else
        xprintfW(wszCodePage, L"%d ", nCodePage);
    }
	return TRUE;
  }
  else wszCodePage[0]='\0';
  return FALSE;
}

LRESULT CALLBACK SettingDlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	//static HICON hPluginIcon;
	static HWND cbDetectType;
	static HWND cbCPASCI;
	static HWND cbNFO;
	int i;
	int *lpCodepageList=NULL;
	//int nCount=0;
	static int nASCICP_Index;
	static int nLATIN1CP_Index;
	switch(Msg)
	{
	case WM_INITDIALOG:
		//hPluginIcon=LoadIconA(pluginData->hInstanceDLL, MAKEINTRESOURCEA(IDI_ICON1));
		//SendMessage(hWndDlg, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hPluginIcon);

		cbDetectType =GetDlgItem(hWndDlg,IDC_COMBO_DETECT_TYPE);
		for(i=0;i<9;i++)
		{
			SendMessageW(cbDetectType, CB_ADDSTRING, 0, (LPARAM)ENC_DetectTypes[i].name);
		}
		SendMessageW(cbDetectType, CB_SETCURSEL , nDetectType, 0);

		cbCPASCI =GetDlgItem(hWndDlg,IDC_COMBO_CP_ASCI);
		cbNFO = GetDlgItem(hWndDlg,IDC_COMBO_CP_NFO);
		//nCount = 0;
		if(lpCodepageList=(int *)SendMessage(hMainWnd, AKD_GETCODEPAGELIST, (WPARAM)NULL, 0))
		{
			wchar_t wbuf[BUFFER_SIZE];
			//int nfoiIndex=0;
			//int asciiIndex =0;
			
			int nfoCurrent = 0;
			int asciiCurrent =0;
			nASCICP_Index = 0;
			nLATIN1CP_Index = 0;
			SendMessageW(cbNFO, CB_ADDSTRING, 0, (LPARAM)L"None");
			for (i=0; lpCodepageList[i]; i++)
			{
				if(lpCodepageList[i] == CODEPAGE_ASCI) nASCICP_Index = i+1;
				else if(lpCodepageList[i] == CODEPAGE_LATIN1) nLATIN1CP_Index = i;
				if(lpCodepageList[i] == nDefaultASCIICodepage) asciiCurrent = i;
				if(lpCodepageList[i] == nCodePageForNFOFile) nfoCurrent = i+1;

				GetCodePageName(lpCodepageList[i], wbuf, BUFFER_SIZE);
				SendMessageW(cbCPASCI, CB_ADDSTRING, 0, (LPARAM)wbuf);
				SendMessageW(cbNFO, CB_ADDSTRING, 0, (LPARAM)wbuf);
			}
			
			//nCount = i;
			//if(!nfoCurrent) nfoCurrent = 0;
			if(!asciiCurrent) asciiCurrent = nLATIN1CP_Index;
			SendMessageW(cbCPASCI, CB_SETCURSEL , asciiCurrent, 0);	
			SendMessageW(cbNFO, CB_SETCURSEL , nfoCurrent, 0);
			
		}
		return TRUE;
	case WM_CLOSE:
		EndDialog(hWndDlg, 0);
		return TRUE;
	//case WM_DESTROY:
		//DestroyIcon(hPluginIcon);
	//	return FALSE;

	case WM_COMMAND:
		switch(wParam)
		{
			wchar_t wbuf[BUFFER_SIZE];
		case IDOK:
			//JSmin_Options[1].value = SendMessageW(CKeepComment, BM_GETCHECK,0,0)==BST_CHECKED?1:0;
			//SaveOption(JSmin_Options,2);
			nDetectType=(int)SendMessageW(cbDetectType, CB_GETCURSEL , 0, 0);
			i = (int)SendMessageW(cbCPASCI, CB_GETCURSEL , 0, 0);
			if(SendMessageW(cbCPASCI,CB_GETLBTEXT,i,(LPARAM)wbuf))
			{
				//MessageBox(hMainWnd,wbuf+2,L"ASCII",MB_OK);
				nDefaultASCIICodepage =(int)xatoiW(wbuf,NULL);

			}
			i = (int)SendMessageW(cbNFO, CB_GETCURSEL , 0, 0);
			if(SendMessageW(cbNFO,CB_GETLBTEXT,i,(LPARAM)wbuf))
			{
				nCodePageForNFOFile =(int)xatoiW(wbuf,NULL);
			}
			SaveOptions();
			EndDialog(hWndDlg, 0);
			return TRUE;
		case IDCANCEL:
			EndDialog(hWndDlg, 0);
			return FALSE;
		case IDC_SETDEAULT:
			SendMessageW(cbDetectType, CB_SETCURSEL , 0, 0);
			SendMessageW(cbNFO, CB_SETCURSEL , nASCICP_Index, 0);
			SendMessageW(cbCPASCI, CB_SETCURSEL , nLATIN1CP_Index, 0);
			return TRUE;
		}
	}
	return FALSE;
}
void __declspec(dllexport) Settings(PLUGINDATA *pd)
{
	//char buf[1024];
	pd->dwSupport|=PDS_NOAUTOLOAD|PDS_NOANSI;
	if(pd->dwSupport & PDS_GETSUPPORT) 
		return;
	else
	{
		if(!bIsIntitCommon) InitCommon(pd);
		DialogBoxW(pd->hInstanceDLL, MAKEINTRESOURCEW(IDD_SETTINGDLG), pd->hMainWnd, (DLGPROC)SettingDlgProc);
		if (pd->bInMemory) pd->nUnload=UD_NONUNLOAD_NONACTIVE;
	}
}

int getCodepageFormCharset(const char *charset)
{
	//int nbItem,i;
	int i;
	const char* charsets[] ={"UTF-8","Shift_JIS","Big5","gb18030","x-euc-tw","HZ-GB-2312","ISO-2022-CN","EUC-JP","ISO-2022-JP","EUC-KR","ISO-2022-KR","KOI8-R","x-mac-cyrillic","IBM855","IBM866","ISO-8859-5","windows-1251","ISO-8859-2","windows-1250","windows-1252","ISO-8859-7","windows-1253","ISO-8859-8","windows-1255","TIS-620","UTF-16BE","UTF-16"};
	int codepages[] = {65001,932 ,950,54936,51950,52936,50227,20932,50222,51949,50225,20866,10007,865,866 ,28595,1251,28592,1250,1252,28597,1253,28592,1255,874,1201,1200};
	int len = 27;
	
	for (i = 0 ; i < len ; i++)
	{
		
		if(0==(int)xstrcmpiA(charsets[i],charset))
		{
			return codepages[i];
		}
	}
	//xprintfW(wszError,L"charset \"%S\" is not found!",charset);
	return 0;
}

int AutodetectCodePage(const wchar_t *wpFile, UINT_PTR dwBytesToCheck, DWORD dwFlags, int *nCodePage, BOOL *bBOM, int nDetectLang,BOOL bAuto)
{

	HANDLE hFile;
	UINT_PTR dwBytesRead=0;
	unsigned char *pBuffer=NULL;
	int nRegCodePage=0;
	UINT_PTR a;
	UINT_PTR b;
	int nANSIrate=5;
	int nUTF16LErate=0;
	int nUTF16BErate=0;
	wchar_t wbuf[BUFFER_SIZE];

	UINT_PTR nBeginJaCheckIndex = 0;
	int enc_type  = 0; //ascii
	int nFNameLen = (int)xstrlenW(wpFile);

	//Remembered code page from registry
	
	if (bAuto && bSaveCodepages && (dwFlags & ADT_REG_CODEPAGE))
	{

		RECENTFILESTACK *rfs;
		RECENTFILE *rf;
		int nMaxRecentFiles;
		dwFlags&=~ADT_REG_CODEPAGE;
		
		if (nMaxRecentFiles=(int)SendMessageW(hMainWnd, AKD_RECENTFILES, RF_GET, (LPARAM)&rfs))
		{
			for (rf=rfs->first; rf; rf=rf->next)
			{
				if (rf->nFileLen == nFNameLen && !xstrcmpiW(rf->wszFile, wpFile))
				{
					nRegCodePage = rf->nCodePage;
					break;
				}
			}
		}

		if (nRegCodePage)
		{
			if (nRegCodePage == CODEPAGE_UTF_32LE || nRegCodePage == CODEPAGE_UTF_32BE)
				dwFlags&=~ADT_BINARY_ERROR;
			*nCodePage=nRegCodePage;
			dwFlags&=~ADT_DETECT_CODEPAGE;
		}

	}

	//Default
	// if (dwFlags & ADT_DETECT_CODEPAGE) *nCodePage=nDefaultCodepage;
	// if (dwFlags & ADT_DETECT_BOM) *bBOM=FALSE;

	//Read file
	if (dwFlags & ADT_BINARY_ERROR || dwFlags & ADT_DETECT_CODEPAGE || dwFlags & ADT_DETECT_BOM)
	{
		hFile=CreateFileW(wpFile, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			if(LoadAkelPadMsg(MSG_CANNOT_OPEN_FILE,wbuf,BUFFER_SIZE))
			{
				MessageBoxW(hMainWnd,wbuf,PLUGINNAME,MB_OK|MB_ICONERROR);
			}
			return EDT_OPEN;
		}
		if (!(pBuffer=(unsigned char *)API_HeapAllocMSG(0, dwBytesToCheck + 1)))
		{
			CloseHandle(hFile);
			return EDT_ALLOC;
		}

		if (!ReadFile64(hFile, pBuffer, dwBytesToCheck, &dwBytesRead, NULL))
		{
			SendMessage(hMainWnd, WM_COMMAND, IDM_INTERNAL_ERRORIO_MSG, 0);
			API_HeapFree(0, (LPVOID)pBuffer);
			CloseHandle(hFile);
			return EDT_READ;
		}
		CloseHandle(hFile);
	}

	//Detect Unicode BOM
	if (dwFlags & ADT_DETECT_CODEPAGE || dwFlags & ADT_DETECT_BOM)
	{
		if (dwBytesRead >= 4)
		{
			if (pBuffer[0] == 0xFF && pBuffer[1] == 0xFE && pBuffer[2] == 0x00 && pBuffer[3] == 0x00)
			{
				if (!nRegCodePage || nRegCodePage == CODEPAGE_UTF_32LE)
				{
					if (dwFlags & ADT_DETECT_CODEPAGE) *nCodePage=CODEPAGE_UTF_32LE;
					if (dwFlags & ADT_DETECT_BOM) *bBOM=TRUE;
					goto Free;
				}
			}
			else if (pBuffer[0] == 0x00 && pBuffer[1] == 0x00 && pBuffer[2] == 0xFE && pBuffer[3] == 0xFF)
			{
				if (!nRegCodePage || nRegCodePage == CODEPAGE_UTF_32BE)
				{
					if (dwFlags & ADT_DETECT_CODEPAGE) *nCodePage=CODEPAGE_UTF_32BE;
					if (dwFlags & ADT_DETECT_BOM) *bBOM=TRUE;
					goto Free;
				}
			}
		}
		if (dwBytesRead >= 2)
		{
			if (pBuffer[0] == 0xFF && pBuffer[1] == 0xFE)
			{
				if (!nRegCodePage || nRegCodePage == CODEPAGE_UTF_16LE)
				{
					if (dwFlags & ADT_DETECT_CODEPAGE) *nCodePage=CODEPAGE_UTF_16LE;
					if (dwFlags & ADT_DETECT_BOM) *bBOM=TRUE;
					goto Free;
				}
			}
			else if (pBuffer[0] == 0xFE && pBuffer[1] == 0xFF)
			{
				if (!nRegCodePage || nRegCodePage == CODEPAGE_UTF_16BE)
				{
					if (dwFlags & ADT_DETECT_CODEPAGE) *nCodePage=CODEPAGE_UTF_16BE;
					if (dwFlags & ADT_DETECT_BOM) *bBOM=TRUE;
					goto Free;
				}
			}
		}
		if (dwBytesRead >= 3)
		{
			if (pBuffer[0] == 0xEF && pBuffer[1] == 0xBB && pBuffer[2] == 0xBF)
			{
				if (!nRegCodePage || nRegCodePage == CODEPAGE_UTF8)
				{
					if (dwFlags & ADT_DETECT_CODEPAGE) *nCodePage=CODEPAGE_UTF8;
					if (dwFlags & ADT_DETECT_BOM) *bBOM=TRUE;
					goto Free;
				}
			}
		}
	}

	if (dwFlags & ADT_BINARY_ERROR || dwFlags & ADT_DETECT_CODEPAGE)
	{
		if (dwBytesRead >= 2)
		{
			for (a=0, b=dwBytesRead - 1; a < b; a+=2)
			{
				//Detect binary file
				if (dwFlags & ADT_BINARY_ERROR)
				{
					if (pBuffer[a] == 0x00 && pBuffer[a + 1] == 0x00)
					{
						API_HeapFree(0, (LPVOID)pBuffer);
						return EDT_BINARY;
					}
				}

				//Detect UTF-16LE, UTF-16BE without BOM
				if (dwFlags & ADT_DETECT_CODEPAGE)
				{
					if (pBuffer[a + 1] == 0x00 && pBuffer[a] <= 0x7E)
					{
						++nUTF16LErate;
						nUTF16BErate=-0xFFFF;
					}
					else if (pBuffer[a] == 0x00 && pBuffer[a + 1] <= 0x7E)
					{
						++nUTF16BErate;
						nUTF16LErate=-0xFFFF;
					}
					else if(enc_type!=ENC_TYPE_NON_ASCII)
					{
						if(pBuffer[a] > 0x7f || pBuffer[a+1] > 0x7f) //non ascii
						{
							enc_type = ENC_TYPE_NON_ASCII;
							nBeginJaCheckIndex = a;
						}
						else if(enc_type == ENC_TYPE_ASCII && pBuffer[a] == 0x1B || pBuffer[a+1] == 0x1B)
						{
							enc_type = ENC_TYPE_ISO_2002;
							nBeginJaCheckIndex = a;
						}			  
					}
				}
			}

			if (dwFlags & ADT_DETECT_CODEPAGE)
			{
				if (nUTF16LErate >= nANSIrate && nUTF16LErate >= nUTF16BErate)
				{
					*nCodePage=CODEPAGE_UTF_16LE;
					dwFlags&=~ADT_DETECT_CODEPAGE;

					if (dwFlags & ADT_DETECT_BOM)
					{
						*bBOM=FALSE;
						dwFlags&=~ADT_DETECT_BOM;
					}
				}
				else if (nUTF16BErate >= nANSIrate && nUTF16BErate >= nUTF16LErate)
				{
					*nCodePage=CODEPAGE_UTF_16BE;
					dwFlags&=~ADT_DETECT_CODEPAGE;

					if (dwFlags & ADT_DETECT_BOM)
					{
						*bBOM=FALSE;
						dwFlags&=~ADT_DETECT_BOM;
					}
				}
			}
		}
	}
	
	if(nCodePageForNFOFile && (dwFlags & ADT_DETECT_CODEPAGE))//.NFO file
	{
		if(nFNameLen >=4 && (0==(int)xstrcmpiW(L".NFO",wpFile + nFNameLen-4)))
		{

			*nCodePage=nCodePageForNFOFile;
			dwFlags&=~ADT_DETECT_CODEPAGE;
		}
	}

	if(nDetectLang < 0 ) nDetectLang = nDetectType;
	//Detect non-Unicode
	if(dwFlags & ADT_DETECT_CODEPAGE)
	{

		if(nDetectLang) //using chardet
		{
			char encoding[128];
			int cpage = EncodingDetector_Detect((char*)pBuffer,(unsigned int)dwBytesRead,ENC_DetectTypes[nDetectLang].nNsLangID,encoding,CODEPAGE_LATIN1);
			if(cpage==0) //
			{
				cpage = getCodepageFormCharset(encoding);
				if(!cpage)
					MessageBoxA(hMainWnd,encoding,"CodePage Not Found!",MB_OK|MB_ICONINFORMATION);
			}
			if(cpage>0)
			{
				*nCodePage = cpage;
			}

		}
		else if(dwBytesRead<2)
		{
			*nCodePage = (dwBytesRead==0||(dwBytesRead==1 && pBuffer[0]<0x7f))?nDefaultASCIICodepage:0;
		}
		else
		{
			if(enc_type==ENC_TYPE_ISO_2002)
			{
				*nCodePage = DetectISO_2002Encoding(pBuffer+nBeginJaCheckIndex,dwBytesRead-nBeginJaCheckIndex);
			}
			else if(enc_type == ENC_TYPE_NON_ASCII)
			{
				*nCodePage = DetectJaEncoding(pBuffer+nBeginJaCheckIndex,dwBytesRead-nBeginJaCheckIndex);
			}
			else
			{
				*nCodePage= nDefaultASCIICodepage;
			}
		}
	}
	if(dwFlags & ADT_DETECT_BOM)
	{
		*bBOM=FALSE;
	}
	//Free buffer
Free:
		
	if (pBuffer) API_HeapFree(0, (LPVOID)pBuffer);
	return EDT_SUCCESS;
}


LRESULT CALLBACK NewMainProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
//  LRESULT lResult;
  DWORD dwFlags;// = *(nod->dwFlags);
  int nCodePage = 0;
  int result;
  BOOL bBOM = FALSE;
  if (uMsg == AKDN_OPENDOCUMENT_START)
  {
    NOPENDOCUMENT *nod=(NOPENDOCUMENT *)lParam;
	dwFlags = *(nod->dwFlags);
	if(!(dwFlags & OD_REOPEN))// && (dwFlags & ADT_BINARY_ERROR || dwFlags & ADT_DETECT_CODEPAGE || dwFlags & ADT_DETECT_BOM))
	{
		//bIsRunning++;
		dwFlags |= ADT_DETECT_CODEPAGE|ADT_DETECT_BOM|ADT_BINARY_ERROR;
		//ReadOptions(FILTER_ALL);
		//result = AutodetectCodePage(nod->wszFile,nDetectSize,dwFlags,&nCodePage,&bBOM,TRUE);
		result = AutodetectCodePage(nod->wszFile,nDetectSize,dwFlags,&nCodePage,&bBOM,-1,TRUE);
		if(nCodePage>0)
		{
			//xprintfW(wbuf,L"%d:%d-%d",nCodePage,bBOM,dwFlags);
			//MessageBoxW(hMainWnd,wbuf,L"detected",MB_OK);
			*(nod->nCodePage) = nCodePage;
			*(nod->bBOM) = bBOM;
			dwFlags&=~ADT_DETECT_CODEPAGE;
			dwFlags&=~ADT_DETECT_BOM;
			dwFlags&=~ADT_BINARY_ERROR;
			dwFlags&=~ADT_REG_CODEPAGE;
			*(nod->dwFlags) = dwFlags;
		}
		/*
		if(EDT_BINARY == result)
		{
			wchar_t wbuf[BUFFER_SIZE],wszMsg[BUFFER_SIZE];
			LoadAkelPadMsg(MSG_ERROR_BINARY,wbuf,BUFFER_SIZE);
			xprintfW(wszMsg, wbuf, nod->wszFile);
			if (MessageBoxW(hMainWnd, wszMsg, APP_MAIN_TITLEW, MB_OKCANCEL|MB_ICONEXCLAMATION|MB_DEFBUTTON2) == IDCANCEL)
			{
				nod->bProcess = FALSE;
			}
		}
		*/
		//bIsRunning--;
	}
	//MessageBoxW(NULL,L"file open",L"Test",MB_OK);
  }

  //Call next procedure
  return NewMainProcData->NextProc(hWnd, uMsg, wParam, lParam);
}

//auto load
void __declspec(dllexport) Main(PLUGINDATA *pd)
{
	pd->dwSupport|=PDS_NOANSI;
	if(pd->dwSupport & PDS_GETSUPPORT) 
		return;
	//if(!plugindata) plugindata = pd;
	if(!bIsIntitCommon)InitCommon(pd);
	if(bInAutoMode)
	{
		UnInitMain();
		//if(pd->bInMemory) pd->nUnload = UD_NONUNLOAD_NONACTIVE;

	}
	else
	{
		InitMain();
		pd->nUnload = UD_NONUNLOAD_ACTIVE;
	}
}


void DetectCodePage(PLUGINDATA *pd, int nDetectTypeID)
{
	FRAMEDATA *lpFrame=(FRAMEDATA *)SendMessage(pd->hMainWnd, AKD_FRAMEFIND, FWF_CURRENT, 0);
	pd->dwSupport|=PDS_NOAUTOLOAD|PDS_NOANSI;
	if(pd->dwSupport & PDS_GETSUPPORT) 
		goto end;
	if(!lpFrame || !lpFrame->wszFile[0] || __IsFileW(lpFrame->wszFile)!=ERROR_SUCCESS)
		goto end;
	else
	{
		int nCodePage;
		BOOL bBOM;
		int result;
		DWORD dwFlags = 0;
		if(!bIsIntitCommon) InitCommon(pd);
		dwFlags |=ADT_DETECT_CODEPAGE|ADT_DETECT_BOM|ADT_BINARY_ERROR;
		result = AutodetectCodePage(lpFrame->wszFile,nDetectSize,dwFlags,&nCodePage,&bBOM,nDetectTypeID,FALSE);
		if(nCodePage)
		{
			OPENDOCUMENT od;
			od.pFile=lpFrame->wszFile;
			od.pWorkDir=NULL;
			od.dwFlags=OD_REOPEN;
			od.nCodePage = nCodePage;
			od.bBOM = bBOM;
			if(lpFrame->ei.bModified)
			{
				wchar_t wbuf[BUFFER_SIZE];
				LoadAkelPadMsg(MSG_FILE_WILL_BE_REOPENED, wbuf, BUFFER_SIZE);
				if(IDOK != MessageBoxW(hMainWnd, wbuf, L"Encoding Detect", MB_OKCANCEL|MB_ICONEXCLAMATION)) 
					goto end;
			}
			SendMessage(pd->hMainWnd, AKD_OPENDOCUMENT, (WPARAM)NULL, (LPARAM)&od);
		}
		else
		{
			MessageBoxW(pd->hMainWnd, L"Can not detect!", L"Encoding Detect", MB_OK|MB_ICONINFORMATION);
		}
	}
end:
	if(pd->bInMemory) pd->nUnload = UD_NONUNLOAD_NONACTIVE;
}

void __declspec(dllexport) NonUniEncoding(PLUGINDATA *pd)
{
	DetectCodePage(pd,0);
}
void __declspec(dllexport) UniJaEncodingDetect(PLUGINDATA *pd)
{
	DetectCodePage(pd,1);
}
void __declspec(dllexport) UniChineseSimEncodingDetect(PLUGINDATA *pd)
{
	DetectCodePage(pd,2);
}
void __declspec(dllexport) UniChineseTRAEncodingDetect(PLUGINDATA *pd)
{
	DetectCodePage(pd,3);
}
void __declspec(dllexport) UniChineseEncodingDetect(PLUGINDATA *pd)
{
	DetectCodePage(pd,4);
}
void __declspec(dllexport) UniKoreaEncodingDetect(PLUGINDATA *pd)
{
	DetectCodePage(pd,5);
}
void __declspec(dllexport) UniCJKEncodingDetect(PLUGINDATA *pd)
{
	DetectCodePage(pd,6);
}
void __declspec(dllexport) UniNonCJKEncodingDetect(PLUGINDATA *pd)
{
	DetectCodePage(pd,7);
}
void __declspec(dllexport) UniEncodingDetect(PLUGINDATA *pd)
{
	DetectCodePage(pd,8);
}