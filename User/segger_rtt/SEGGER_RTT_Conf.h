/*********************************************************************
*               SEGGER MICROCONTROLLER GmbH & Co. KG                 *
*       Solutions for real time microcontroller applications         *
**********************************************************************
*                                                                    *
*       (c) 2014 - 2015  SEGGER Microcontroller GmbH & Co. KG        *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : SEGGER_RTT_Conf.h
Purpose : Implementation of SEGGER real-time transfer (RTT) which
          allows real-time communication on targets which support
          debugger memory accesses while the CPU is running.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef SEGGER_RTT_CONF_H
#define SEGGER_RTT_CONF_H
//#include "compiler_abstraction.h"
//#include "app_util_platform.h"
#ifdef __ICCARM__
  #include <intrinsics.h>
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/


#define  SGEE_MODE SEGGER_RTT_MODE_NO_BLOCK_SKIP//SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL//   SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL; //SEGGER_RTT_MODE_NO_BLOCK_SKIP

#define SEGGER_RTT_MAX_NUM_UP_BUFFERS             1     // Max. number of up-buffers (T->H) available on this target    (Default: 2)
#define SEGGER_RTT_MAX_NUM_DOWN_BUFFERS           1     // Max. number of down-buffers (H->T) available on this target  (Default: 2)
#define BUFFER_SIZE_UP                            2048  // Size of the buffer for terminal output of target, up to host (Default: 1k)
#define BUFFER_SIZE_DOWN                          10    // Size of the buffer for terminal input to target from host (Usually keyboard input) (Default: 16)
#define SEGGER_RTT_PRINTF_BUFFER_SIZE             (BUFFER_SIZE_UP)    // Size of buffer for RTT printf to bulk-send chars via RTT     (Default: 64)
#define SEGGER_RTT_MODE_DEFAULT                   0 // Mode for preinitialized terminal channel (buffer 0)

//
// Target is not allowed to perform other RTT operations while string still has not been stored completely.
// Otherwise we would probably end up with a mixed string in the buffer.
// If using  RTT from within interrupts, multiple tasks or multi processors, define the SEGGER_RTT_LOCK() and SEGGER_RTT_UNLOCK() function here.
//
/*********************************************************************
*
*       RTT lock configuration for SEGGER Embedded Studio,
*       Rowley CrossStudio and GCC
*/

#define SEGGER_RTT_LOCK(SavedState)       \
                   SavedState = 0;       
//                   CRITICAL_REGION_ENTER()

#define SEGGER_RTT_UNLOCK(SavedState)     \
                   (void)SavedState;      
//                   CRITICAL_REGION_EXIT()

/*************************** End of file ****************************/
#endif // SEGGER_RTT_CONF_H
