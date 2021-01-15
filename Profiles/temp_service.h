#ifndef _TEMP_SERVICE_H_
#define _TEMP_SERVICE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <bcomdef.h>

/*********************************************************************
 * CONSTANTS
 */
// Temperature Service UUID
#define TEMP_SERVICE_SERV_UUID 0xAAA0
#define TEMP_SERVICE_SERV_UUID_BASE128(uuid) 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0xB0, 0x00, 0x40, 0x51, 0x04, LO_UINT16(uuid), HI_UINT16(uuid), 0x00, \
    0xF0

// Temperature Characteristic defines
#define LS_TEMP_ID                 0
#define LS_TEMP_UUID               0xAAA1 // service + 1
#define LS_TEMP_UUID_BASE128(uuid) 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0xB0, 0x00, 0x40, 0x51, 0x04, LO_UINT16(uuid), HI_UINT16(uuid), 0x00, 0xF0
#define LS_TEMP_LEN                5
#define LS_TEMP_LEN_MIN            5

// Delay Characteristic defines
#define LS_DELAY_ID 1
#define LS_DELAY_UUID 0xAAA2 //service + 2
#define LS_DELAY_UUID_BASE128(uuid) 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
        0xB0, 0x00, 0x40, 0x51, 0x04, LO_UINT16(uuid), HI_UINT16(uuid), 0x00, 0xF0
#define LS_DELAY_LEN 4
#define LS_DELAY_LEN_MIN 4


/*********************************************************************
 * Profile Callbacks
 */
// Callback when a characteristic value has changed
typedef void (*TempServiceChange_t)(uint16_t connHandle, uint8_t paramID,
                                   uint16_t len, uint8_t *pValue);

typedef struct
{
    TempServiceChange_t pfnChangeCb;          // Called when characteristic value changes
    TempServiceChange_t pfnCfgChangeCb;       // Called when characteristic CCCD changes
} TempServiceCBs_t;


/*********************************************************************
 * API FUNCTIONS
 */

/*
 * LedService_AddService- Initializes the LedService service by registering
 *          GATT attributes with the GATT server.
 *
 *    rspTaskId - The ICall Task Id that should receive responses for Indications.
 */
extern bStatus_t TempService_AddService(uint8_t rspTaskId);

/*
 * LedService_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t TempService_RegisterAppCBs(TempServiceCBs_t *appCallbacks);

/*
 * LedService_SetParameter - Set a LedService parameter.
 *
 *    param - Profile parameter ID
 *    len   - length of data to write
 *    value - pointer to data to write.  This is dependent on
 *            the parameter ID and may be cast to the appropriate
 *            data type (example: data type of uint16_t will be cast to
 *            uint16_t pointer).
 */
extern bStatus_t TempService_SetParameter(uint8_t param,
                                         uint16_t len,
                                         void *value);


/*
 * LedService_GetParameter - Get a LedService parameter.
 *
 *    param - Profile parameter ID
 *    len   - pointer to a variable that contains the maximum length that can be written to *value.
              After the call, this value will contain the actual returned length.
 *    value - pointer to data to write.  This is dependent on
 *            the parameter ID and may be cast to the appropriate
 *            data type (example: data type of uint16_t will be cast to
 *            uint16_t pointer).
 */
extern bStatus_t TempService_GetParameter(uint8_t param,
                                         uint16_t *len,
                                         void *value);

#ifdef __cplusplus
}
#endif

#endif
