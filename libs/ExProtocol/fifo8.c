#include "cmn.h"
#include "fifo8.h"
//--------------------------------------
// Function		: FF8_Reset()
// Description	: Reset the FF8 to the original state.
// Note: None.
inline void FF8_Reset(FF8 *pFF, FF_TYPE *pBuff, int nSize) {
	ASSERT_VOID ((pFF != NULL) && (pBuff != NULL));
    pFF->pArrBuff = pBuff;
	pFF->nMaxSize = nSize;
	pFF->nSize = 0;
	pFF->nHead = 0;
	pFF->nTail = 0;
	
	pFF->bEnProtect  = FALSE;
	pFF->nProtectPtr = 0;
}

//--------------------------------------
// Function		: FF8_Push()
// Description	: Push an element to the FF8.
// Note: This function does not perform thread safe. User must take care of it.
inline BOOL FF8_Push(FF8 *pFF, FF_TYPE b) {
	ASSERT_NONVOID(pFF != NULL, FALSE);

	if(pFF->nSize < pFF->nMaxSize) {
		pFF->pArrBuff[pFF->nTail] = b;
		pFF->nTail++;				
		if(pFF->nTail >= pFF->nMaxSize) {
			pFF->nTail = 0;
		}
		pFF->nSize++;
		return TRUE;
	}	

	return FALSE;
}

//--------------------------------------
// Function		: FF8_Pop()
// Description	: Pop a element from the FF8
// Note: This function does not perform thread safe. User must take care of it.
inline BOOL FF8_Pop(FF8 *pFF, FF_TYPE *b) {
	ASSERT_NONVOID(((pFF != NULL) && (b != NULL)), FALSE);

	if(pFF->nSize != 0) {
		*b = pFF->pArrBuff[pFF->nHead];
		pFF->nHead++;
		if(pFF->nHead >= pFF->nMaxSize) {
			pFF->nHead = 0;
		}
		pFF->nSize--;
		return TRUE;
	}		

	*b = 0xFF;
	return FALSE;
}

//--------------------------------------
// Function		: FF8_GetData()
// Description	: Get number of data in FF8 from Head but not Pop it out of FF
// Note:
inline BOOL FF8_GetData(FF8 *pFF, FF_TYPE *b, int nSize) {
	int i;
	ASSERT_NONVOID((pFF != NULL) && (b != NULL), FALSE);
	int iPtr = pFF->nHead;

	if(pFF->nSize >= nSize) {
		for (i = 0; i < nSize; i++) {
			b[i] = pFF->pArrBuff[iPtr];
			iPtr++;
			if(iPtr >= pFF->nMaxSize) {
				iPtr = 0;
			}
		}
		return TRUE;
	}

	return FALSE;
}

//--------------------------------------
// Function		: FF8_GetData()
// Description	: Get number of data in FF8 from Head but not Pop it out of FF
// Note:
inline BOOL FF8_PopArray(FF8 *pFF, FF_TYPE *b, int nSize) {
    int i;
    ASSERT_NONVOID((pFF != NULL) && (b != NULL), FALSE);

    if (pFF->nSize >= nSize) {
        for (i = 0; i < nSize; i++) {
            FF8_Pop(pFF, b + i);
        }
        return TRUE;
    }

    return FALSE;
}

//--------------------------------------
// Function		: FF8_EnableProtect()
// Description	: Enable FF8 protection mode.
// Note: None.
inline void FF8_EnableProtect(FF8 *pFF) {

	ASSERT_VOID (pFF != NULL);
	pFF->bEnProtect  = TRUE;
	pFF->nProtectPtr = pFF->nHead;
}

//--------------------------------------
// Function		: FF8_DisableProtect()
// Description	: Disable FF8 protection mode.
// Note: None.
inline void FF8_DisableProtect(FF8 *pFF) {
	ASSERT_VOID (pFF != NULL);

	pFF->bEnProtect  = FALSE;
}

//--------------------------------------
// Function		: FF8_RewindHead()
// Description	: Push back the FF8 some marked protected bytes.
// Note: This function does not perform thread safe. User must take care of it.
inline void FF8_RewindHead(FF8 *pFF) {
	ASSERT_VOID (pFF != NULL);

	if(pFF->bEnProtect) {
		while(pFF->nHead != pFF->nProtectPtr) {
			if(pFF->nHead == 0) {
				pFF->nHead = pFF->nMaxSize - 1;
			}
			else {
				pFF->nHead--;
			}
			pFF->nSize++;
			
			if(pFF->nHead == pFF->nTail)
				break;
		}
	}	
}

//--------------------------------------
// Function		: FF8_IsEnablePush()
// Description	: Check if possible to push more data into the FF8
// Note: None.
inline BOOL FF8_IsEnablePush(FF8 *pFF) {
	ASSERT_NONVOID((pFF != NULL), FALSE);

	if(  (pFF->nSize == pFF->nMaxSize) ||
	     (pFF->bEnProtect == TRUE && pFF->nSize  != 0 && pFF->nTail  == pFF->nProtectPtr) 
	  ) {
		return FALSE;	    	
	}
	
	return TRUE;
}

//--------------------------------------
// Function			: FIFO_IsEnablePop()
// Description	: Check if possible to push more data into the FIFO
// Note: None.
inline BOOL FF8_IsEnablePop(FF8 *pFF) {
	ASSERT_NONVOID((pFF != NULL), FALSE);

	return (pFF->nSize > 0);
}

//--------------------------------------
// Function		: FF8_IsEmpty()
// Description	: Get emptiness state of the FF8
inline BOOL FF8_IsEmpty(FF8 *pFF) {
	ASSERT_NONVOID((pFF != NULL), FALSE);

	return (pFF->nSize == 0);
}

//--------------------------------------
// Function		: FF8_IsEmpty()
// Description	: Get fullness state of the FF8
inline BOOL FF8_IsFull(FF8 *pFF) {
	ASSERT_NONVOID((pFF != NULL), FALSE);

	return (FF8_IsEnablePush(pFF) ? FALSE : TRUE);
}

//--------------------------------------
// Function		: FF8_GetCount()
// Description	: Get number of stored elements in the FF8
inline int FF8_GetCount(FF8 *pFF) {
	ASSERT_NONVOID((pFF != NULL), FALSE);

	return (pFF->nSize);
}

//--------------------------------------
// Function		: FF8_GetAvaiLen()
// Description	: Get number of available elements to be stored in the FF8
inline int FF8_GetAvaiLen(FF8 *pFF) {
	ASSERT_NONVOID((pFF != NULL), FALSE);

	return (pFF->nMaxSize - pFF->nSize);
}

//--------------------------------------
// Function     : FF8_GetMaxSize()
// Description  : Get size of the buffer to store data
inline int FF8_GetMaxSize(FF8 *pFF) {
	ASSERT_NONVOID(pFF != NULL, -1);

  return (pFF->nMaxSize);
}
