#ifndef prmem_h___
#define prmem_h___
//#include "nscore.h"
//#include <stdlib.h>
#include <Windows.h>
#ifdef __cplusplus
#define PR_BEGIN_EXTERN_C       extern "C" {
#define PR_END_EXTERN_C         }
#else
#define PR_BEGIN_EXTERN_C
#define PR_END_EXTERN_C
#endif
//#define PR_IMPORT(__type) extern __type
//#define NSPR_API(__type) PR_IMPORT(__type)
PR_BEGIN_EXTERN_C
#define PR_Malloc(size) HeapAlloc(GetProcessHeap(),0,size)
//#define free(ptr)  PR_Free(ptr)
//NSPR_API(void *) PR_Malloc(PRUint32 size);
//NSPR_API(void) PR_Free(void *ptr);
/***********************************************************************
** FUNCTION:	PR_DELETE()
** DESCRIPTION:
**   PR_DELETE() unallocates an object previosly allocated via PR_NEW()
**   or PR_NEWZAP() to the heap.
** INPUTS:	pointer to previously allocated object
** OUTPUTS:	the referenced object is returned to the heap
** RETURN:	void
***********************************************************************/
#define PR_DELETE(_ptr) { HeapFree(GetProcessHeap(),0,(LPVOID)_ptr); (_ptr) = NULL; }

/***********************************************************************
** FUNCTION:	PR_FREEIF()
** DESCRIPTION:
**   PR_FREEIF() conditionally unallocates an object previously allocated
**   vial PR_NEW() or PR_NEWZAP(). If the pointer to the object is
**   equal to zero (0), the object is not released.
** INPUTS:	pointer to previously allocated object
** OUTPUTS:	the referenced object is conditionally returned to the heap
** RETURN:	void
***********************************************************************/
#define PR_FREEIF(_ptr)	if (_ptr) PR_DELETE(_ptr)

PR_END_EXTERN_C
#endif