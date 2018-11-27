#ifndef _CMN_H_
#define _CMN_H_
#include <stdint.h>
#include <stdlib.h>
#include "assert.h"
#include "uart_console.h"
/*define*/
#ifndef BOOL
#define BOOL  uint8_t
#endif
#ifndef TRUE
#define TRUE  (1)
#define FALSE (0)
#endif

#define ASSERT(x) assert(x)
#define ASSERT_NONVOID(x, y) {}
#define ASSERT_VOID

#ifndef LREP
#define LREP      SOS_DEBUG
#endif

#define inline

#ifndef NULL
#define NULL (0)
#endif

/*enum*/
typedef enum{
    RET_OK,
    RET_FAIL,
    RET_ERROR
}status_t;

#endif /*_CMN_H_*/
/*--------------EOF-------------*/
