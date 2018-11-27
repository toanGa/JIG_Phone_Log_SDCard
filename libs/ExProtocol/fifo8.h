#ifndef _FF8_H_
#define _FF8_H_

#include "cmn.h"

#define FF_TYPE   uint8_t

typedef struct _FF8 {
  FF_TYPE   *pArrBuff;
  int   nMaxSize;
  int   nSize;
  int   nHead;
  int   nTail;

  // Protection Mode (usefull for packet handling)
  BOOL  bEnProtect;
  int   nProtectPtr;
  
} FF8;

inline void   FF8_Reset         (FF8 *pFF, FF_TYPE *pBuff, int nSize);
inline BOOL   FF8_Push          (FF8 *pFF, FF_TYPE b);
inline BOOL   FF8_Pop           (FF8 *pFF, FF_TYPE *b);
inline void   FF8_EnableProtect (FF8 *pFF);
inline void   FF8_DisableProtect(FF8 *pFF);
inline void   FF8_RewindHead    (FF8 *pFF);
inline BOOL   FF8_IsEnablePush  (FF8 *pFF);
inline BOOL   FF8_IsEnablePop   (FF8 *pFF);
inline BOOL   FF8_IsEmpty       (FF8 *pFF);
inline BOOL   FF8_IsFull        (FF8 *pFF);
inline int    FF8_GetCount      (FF8 *pFF);
inline int    FF8_GetAvaiLen    (FF8 *pFF);
inline int    FF8_GetMaxSize    (FF8 *pFF);
inline BOOL   FF8_PopArray      (FF8 *pFF, FF_TYPE *b, int nSize);
inline BOOL   FF8_GetData		(FF8 *pFF, FF_TYPE *b, int nSize);

#endif  //_FF8_H_
