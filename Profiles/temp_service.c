/*
 * temp_service.c
 *
 *  Created on: Dec 18, 2020
 *      Author: lpasi
 */



/*********************************************************************
 * INCLUDES
 */
#include <string.h>

//#include <xdc/runtime/Log.h> // Comment this in to use xdc.runtime.Log
#include <ti/common/cc26xx/uartlog/UartLog.h>  // Comment out if using xdc Log

#include <icall.h>

/* This Header file contains all BLE API and icall structure definition */
#include "icall_ble_api.h"

#include "temp_service.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Temperature Service UUID
CONST uint8_t TempServiceUUID[ATT_UUID_SIZE] =
{
     TEMP_SERVICE_SERV_UUID_BASE128(TEMP_SERVICE_SERV_UUID)
};

// Temperature UUID
CONST uint8_t ls_TempUUID[ATT_UUID_SIZE] =
{
     LS_TEMP_UUID_BASE128(LS_TEMP_UUID)
};

// Delay UUID
CONST uint8_t ls_DelayUUID[ATT_UUID_SIZE] =
{
     LS_DELAY_UUID_BASE128(LS_DELAY_UUID)
};

/*********************************************************************
 * LOCAL VARIABLES
 */
static TempServiceCBs_t *pAppCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// Service declaration
static CONST gattAttrType_t TempServiceDecl = { ATT_UUID_SIZE, TempServiceUUID };

// ################# TEMP ##############################
// Characteristic "TEMP" Properties (for declaration)
// read + no response
static uint8_t ls_TempProps = GATT_PROP_READ;

// Characteristic "TEMP" Value variable
static uint8_t ls_TempVal[LS_TEMP_LEN] = {0};

// Length of data in characteristic "TEMP" Value variable, initialized to minimal size.
static uint16_t ls_TempValLen = LS_TEMP_LEN_MIN;

// ################# DELAY #############################
// Characteristic "DEALY" Properties (for declaration)
// read + no response
static uint8_t ls_DelayProps = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;

// Characteristic "DELAY" Value variable
static uint8_t ls_DelayVal[LS_DELAY_LEN] = {0};

// Length of data in characteristic "DELAY" Value variable, initialized to minimal size.
static uint16_t ls_DelayValLen = LS_DELAY_LEN_MIN;


/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t TEMP_ServiceAttrTbl[] =
{
    // TEMP_Service Service Declaration
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID },
        GATT_PERMIT_READ,
        0,
        (uint8_t *)&TempServiceDecl
    },
    // TEMP Characteristic Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &ls_TempProps
    },
    // TEMP Characteristic Value
    {
        { ATT_UUID_SIZE, ls_TempUUID },
        GATT_PERMIT_READ,
        0,
        ls_TempVal
    },
    // DELAY Characteristic Decl
    {
        { ATT_BT_UUID_SIZE, characterUUID },
            GATT_PERMIT_READ,
            0,
            &ls_DelayProps
    },
    // DELAY Characteristic Value
    {
            { ATT_UUID_SIZE, ls_DelayUUID },
            GATT_PERMIT_READ | GATT_PERMIT_WRITE,
            0,
            ls_DelayVal
        },
};


/*********************************************************************
 * LOCAL FUNCTIONS FWD DECL
 */
static bStatus_t TEMP_Service_ReadAttrCB(uint16_t connHandle,
                                        gattAttribute_t *pAttr,
                                        uint8_t *pValue,
                                        uint16_t *pLen,
                                        uint16_t offset,
                                        uint16_t maxLen,
                                        uint8_t method);

static bStatus_t TEMP_Service_WriteAttrCB(uint16_t connHandle,
                                         gattAttribute_t *pAttr,
                                         uint8_t *pValue,
                                         uint16_t len,
                                         uint16_t offset,
                                         uint8_t method);

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t Temp_ServiceCBs =
{
    TEMP_Service_ReadAttrCB, // Read callback function pointer
    TEMP_Service_WriteAttrCB, // Write callback function pointer
    NULL                     // Authorization callback function pointer
};


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*
 * TempService_AddService - Initializes the TempService service by registering
 *          GATT attributes with the GATT server.
 *
 *    rspTaskId - The ICall Task Id that should receive responses for Indications.
 */
extern bStatus_t TempService_AddService(uint8_t rspTaskId)
{
    uint8_t status;

    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService(TEMP_ServiceAttrTbl,
                                         GATT_NUM_ATTRS(TEMP_ServiceAttrTbl),
                                         GATT_MAX_ENCRYPT_KEY_SIZE,
                                         &Temp_ServiceCBs);
    Log_info2("Registered service, %d attributes, status 0x%02x",
              GATT_NUM_ATTRS(
                  TEMP_ServiceAttrTbl), status);
    return(status);
}


/*
 * TempService_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
bStatus_t TempService_RegisterAppCBs(TempServiceCBs_t *appCallbacks)
{
    if(appCallbacks)
    {
        pAppCBs = appCallbacks;
        Log_info1("Registered callbacks to application. Struct %p",
                  (uintptr_t)appCallbacks);
        return(SUCCESS);
    }
    else
    {
        Log_warning0("Null pointer given for app callbacks.");
        return(FAILURE);
    }
}

/*
 * TempService_SetParameter - Set a TempService parameter.
 *
 *    param - Profile parameter ID
 *    len   - length of data to write
 *    value - pointer to data to write.  This is dependent on
 *            the parameter ID and may be cast to the appropriate
 *            data type (example: data type of uint16_t will be cast to
 *            uint16_t pointer).
 */
bStatus_t TempService_SetParameter(uint8_t param, uint16_t len, void *value)
{
    bStatus_t ret = SUCCESS;
    uint8_t  *pAttrVal;
    uint16_t *pValLen;
    uint16_t valMinLen;
    uint16_t valMaxLen;

    switch(param)
    {
    case LS_TEMP_ID:
        pAttrVal = ls_TempVal;
        pValLen = &ls_TempValLen;
        valMinLen = LS_TEMP_LEN_MIN;
        valMaxLen = LS_TEMP_LEN;
        Log_info2("SetParameter : %s len: %d val: %hu", (uintptr_t)"TEMP", len);
        break;
    case LS_DELAY_ID:
       pAttrVal = ls_DelayVal;
       pValLen = &ls_DelayValLen;
       valMinLen = LS_DELAY_LEN_MIN;
       valMaxLen = LS_DELAY_LEN;
       Log_info2("SetParameter : %s len: %d val: %hu", (uintptr_t)"DELAY", len);
       break;
    default:
        Log_error1("SetParameter: Parameter #%d not valid.", param);
        return(INVALIDPARAMETER);
    }

    // Check bounds, update value and send notification or indication if possible.
    if(len <= valMaxLen && len >= valMinLen)
    {
        memcpy(pAttrVal, value, len);
        *pValLen = len; // Update length for read and get.
    }
    else
    {
        Log_error3("Length outside bounds: Len: %d MinLen: %d MaxLen: %d.", len,
                   valMinLen,
                   valMaxLen);
        ret = bleInvalidRange;
    }

    return(ret);
}

/*
 * TempService_GetParameter - Get a TempService parameter.
 *
 *    param - Profile parameter ID
 *    len   - pointer to a variable that contains the maximum length that can be written to *value.
              After the call, this value will contain the actual returned length.
 *    value - pointer to data to write.  This is dependent on
 *            the parameter ID and may be cast to the appropriate
 *            data type (example: data type of uint16_t will be cast to
 *            uint16_t pointer).
 */
bStatus_t TempService_GetParameter(uint8_t param, uint16_t *len, void *value)
{
    bStatus_t ret = SUCCESS;
    switch(param)
    {
    case LS_TEMP_ID:
        *len = MIN(*len, ls_TempValLen);
        memcpy(value, ls_TempVal, *len);
        Log_info2("GetParameter : %s returning %d bytes", (uintptr_t)"TEMP",
                  *len);
        break;
    case LS_DELAY_ID:
        *len = MIN(*len, ls_DelayValLen);
        memcpy(value, ls_DelayVal, *len);
        Log_info2("GetParameter : %s returning %d bytes", (uintptr_t)"DELAY",
                  *len);
        break;

    default:
        Log_error1("GetParameter: Parameter #%d not valid.", param);
        ret = INVALIDPARAMETER;
        break;
    }
    return(ret);
}

/*********************************************************************
 * @internal
 * @fn          TEMP_Service_findCharParamId
 *
 * @brief       Find the logical param id of an attribute in the service's attr table.
 *
 *              Works only for Characteristic Value attributes and
 *              Client Characteristic Configuration Descriptor attributes.
 *
 * @param       pAttr - pointer to attribute
 *
 * @return      uint8_t paramID (ref temp_service.h) or 0xFF if not found.
 */
static uint8_t TEMP_Service_findCharParamId(gattAttribute_t *pAttr)
{
    // Is this a Client Characteristic Configuration Descriptor?
    if(ATT_BT_UUID_SIZE == pAttr->type.len && GATT_CLIENT_CHAR_CFG_UUID ==
       *(uint16_t *)pAttr->type.uuid)
    {
        return(TEMP_Service_findCharParamId(pAttr - 1)); // Assume the value attribute precedes CCCD and recurse
    }
    // Is this attribute in "TEMP"?
    else if(ATT_UUID_SIZE == pAttr->type.len &&
            !memcmp(pAttr->type.uuid, ls_TempUUID, pAttr->type.len))
    {
        return(LS_TEMP_ID);
    }
    // Is this attr in "DELAY"?
    else if(ATT_UUID_SIZE == pAttr->type.len &&
                !memcmp(pAttr->type.uuid, ls_DelayUUID, pAttr->type.len))
        {
            return(LS_DELAY_ID);
        }
    else
    {
        return(0xFF); // Not found. Return invalid.
    }
}

/*********************************************************************
 * @fn          TEMP_Service_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t TEMP_Service_ReadAttrCB(uint16_t connHandle,
                                        gattAttribute_t *pAttr,
                                        uint8_t *pValue, uint16_t *pLen,
                                        uint16_t offset,
                                        uint16_t maxLen,
                                        uint8_t method)
{
    bStatus_t status = SUCCESS;
    uint16_t valueLen;
    uint8_t paramID = 0xFF;

    // Find settings for the characteristic to be read.
    paramID = TEMP_Service_findCharParamId(pAttr);
    switch(paramID)
    {
    case LS_TEMP_ID:
        valueLen = ls_TempValLen;

        Log_info4("ReadAttrCB : %s connHandle: %d offset: %d method: 0x%02x",
                  (uintptr_t)"TEMP",
                  connHandle,
                  offset,
                  method);
        /* Other considerations for TEMP can be inserted here */
        break;

    case LS_DELAY_ID:
        valueLen = ls_DelayValLen;
        Log_info4("ReadAttrCB : %s connHandle: %d offset: %d method: 0x%02x",
                         (uintptr_t)"DELAY",
                         connHandle,
                         offset,
                         method);
        break;
    default:
        Log_error0("Attribute was not found.");
        return(ATT_ERR_ATTR_NOT_FOUND);
    }
    // Check bounds and return the value
    if(offset > valueLen)   // Prevent malicious ATT ReadBlob offsets.
    {
        Log_error0("An invalid offset was requested.");
        status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
        *pLen = MIN(maxLen, valueLen - offset); // Transmit as much as possible
        memcpy(pValue, pAttr->pValue + offset, *pLen);
    }

    return(status);
}

/*********************************************************************
 * @fn      TEMP_Service_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t TEMP_Service_WriteAttrCB(uint16_t connHandle,
                                         gattAttribute_t *pAttr,
                                         uint8_t *pValue,
                                         uint16_t len,
                                         uint16_t offset,
                                         uint8_t method)
{
    bStatus_t status = SUCCESS;
    uint8_t paramID = 0xFF;
    uint8_t changeParamID = 0xFF;
    uint16_t writeLenMin;
    uint16_t writeLenMax;
    uint16_t *pValueLenVar;

    // Find settings for the characteristic to be written.
    paramID = TEMP_Service_findCharParamId(pAttr);
    switch(paramID)
    {
    case LS_TEMP_ID:
        writeLenMin = 4;
        writeLenMax = LS_TEMP_LEN;
        pValueLenVar = &ls_TempValLen;

        Log_info5(
            "WriteAttrCB : %s connHandle(%d) len(%d) offset(%d) method(0x%02x)",
            (uintptr_t)"TEMP",
            connHandle,
            len,
            offset,
            method);
        /* Other considerations for TEMP can be inserted here */
        break;
    case LS_DELAY_ID:
        writeLenMin = 4;
                writeLenMax = LS_DELAY_LEN;
                pValueLenVar = &ls_DelayValLen;

                Log_info5(
                    "WriteAttrCB : %s connHandle(%d) len(%d) offset(%d) method(0x%02x)",
                    (uintptr_t)"DELAY",
                    connHandle,
                    len,
                    offset,
                    method);

                /* Other considerations for DELAY can be inserted here */
                break;
    default:
        Log_error0("Attribute was not found.");
        return(ATT_ERR_ATTR_NOT_FOUND);
    }
    // Check whether the length is within bounds.
    if(offset >= writeLenMax)
    {
        Log_error0("An invalid offset was requested.");
        status = ATT_ERR_INVALID_OFFSET;
    }
    else if(offset + len > writeLenMax)
    {
        Log_error0("Invalid value length was received.");
        status = ATT_ERR_INVALID_VALUE_SIZE;
    }
    else if(offset + len < writeLenMin &&
            (method == ATT_EXECUTE_WRITE_REQ || method == ATT_WRITE_REQ))
    {
        // Refuse writes that are lower than minimum.
        // Note: Cannot determine if a Reliable Write (to several chars) is finished, so those will
        //       only be refused if this attribute is the last in the queue (method is execute).
        //       Otherwise, reliable writes are accepted and parsed piecemeal.
        Log_error0("Invalid value length was received.");
        status = ATT_ERR_INVALID_VALUE_SIZE;
    }
    else
    {
        // Copy pValue into the variable we point to from the attribute table.
        memcpy(pAttr->pValue + offset, pValue, len);

        // Only notify application and update length if enough data is written.
        //
        // Note: If reliable writes are used (meaning several attributes are written to using ATT PrepareWrite),
        //       the application will get a callback for every write with an offset + len larger than _LEN_MIN.
        // Note: For Long Writes (ATT Prepare + Execute towards only one attribute) only one callback will be issued,
        //       because the write fragments are concatenated before being sent here.
        if(offset + len >= writeLenMin)
        {
            changeParamID = paramID;
            *pValueLenVar = offset + len; // Update data length.
        }
    }

    // Let the application know something changed (if it did) by using the
    // callback it registered earlier (if it did).
    if(changeParamID != 0xFF)
    {
        if(pAppCBs && pAppCBs->pfnChangeCb)
        {
            pAppCBs->pfnChangeCb(connHandle, paramID, len + offset, pValue);
        }
    }
    return(status);
}
