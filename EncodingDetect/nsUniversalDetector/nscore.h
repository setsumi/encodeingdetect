#ifndef nscore_h__
#define nscore_h__
#include <cstdint>
typedef int PRBool;
typedef unsigned _int32 PRUint32;
typedef _int32 PRInt32;
#define NS_OK (0);
//if x64 0 nsnull 0LL
#define nsnull (0)
#define PR_FALSE  (0)
#define PR_TRUE  (!PR_FALSE) 
typedef _int32 nsresult;
#define NS_ERROR_OUT_OF_MEMORY ((nsresult) 0x8007000eL)

typedef unsigned _int8 PRUint8;
typedef _int8 PRInt8;
typedef unsigned _int16 PRUint16;
typedef _int16 PRInt16;
#define NS_ARRAY_LENGTH(array_) (sizeof(array_)/sizeof(array_[0]))
#endif