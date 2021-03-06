/******************************************************************************
  Filename:       Ep5.c
  Revised:        $Date: 2012-03-07 01:04:58 -0800 (Wed, 07 Mar 2012) $
  Revision:       $Revision: 29656 $

  Description:    Generic Application (no Profile).


  Copyright 2004-2012 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED �AS IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
******************************************************************************/

/*********************************************************************
  This application isn't intended to do anything useful, it is
  intended to be a simple example of an application's structure.

  This application sends "Hello World" to another "Generic"
  application every 5 seconds.  The application will also
  receives "Hello World" packets.

  The "Hello World" messages are sent/received as MSG type message.

  This applications doesn't have a profile, so it handles everything
  directly - itself.

  Key control:
    SW1:
    SW2:  initiates end device binding
    SW3:
    SW4:  initiates a match description request
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "Ep5.h"
#include "DebugTrace.h"

#if !defined( WIN32 )
  #include "OnBoard.h"
#endif

#include "DHT11.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"

/* RTOS */
#if defined( IAR_ARMCM3_LM )
#include "RTOS_App.h"
#endif  

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
// This list should be filled with Application specific Cluster IDs.
const cId_t Ep5_ClusterList[Ep5_MAX_CLUSTERS] =
{
  Ep5_CLUSTERID
};

const SimpleDescriptionFormat_t Ep5_SimpleDesc =
{
  Ep5_ENDPOINT,              //  int Endpoint;
  Ep5_PROFID,                //  uint16 AppProfId[2];
  Ep5_DEVICEID,              //  uint16 AppDeviceId[2];
  Ep5_DEVICE_VERSION,        //  int   AppDevVer:4;
  Ep5_FLAGS,                 //  int   AppFlags:4;
  Ep5_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)Ep5_ClusterList,  //  byte *pAppInClusterList;
  Ep5_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)Ep5_ClusterList   //  byte *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in Ep5_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t Ep5_epDesc;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
byte Ep5_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // Ep5_Init() is called.
devStates_t Ep5_NwkState;


byte Ep5_TransID;  // This is the unique message ID (counter)

afAddrType_t Ep5_DstAddr;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void Ep5_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg );
static void Ep5_HandleKeys( byte shift, byte keys );
static void Ep5_MessageMSGCB( afIncomingMSGPacket_t *pckt );

//Add by Frank
//external function(no global)
void Ep5_SendTheMessage( uint8 isTurnOnLed );

#if defined( IAR_ARMCM3_LM )
static void Ep5_ProcessRtosMessage( void );
#endif

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      Ep5_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void Ep5_Init( uint8 task_id )
{
  Ep5_TaskID = task_id;
  Ep5_NwkState = DEV_INIT;
  Ep5_TransID = 0;
 
  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

  Ep5_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  Ep5_DstAddr.endPoint = 10;

  // Fill out the endpoint description.
  Ep5_epDesc.endPoint = Ep5_ENDPOINT;
  Ep5_epDesc.task_id = &Ep5_TaskID;
  Ep5_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&Ep5_SimpleDesc;
  Ep5_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &Ep5_epDesc );

  // Update the display
#if defined ( LCD_SUPPORTED )
  HalLcdWriteString( "Ep5", HAL_LCD_LINE_1 );
#endif

  ZDO_RegisterForZDOMsg( Ep5_TaskID, End_Device_Bind_rsp );
  ZDO_RegisterForZDOMsg( Ep5_TaskID, Match_Desc_rsp );

#if defined( IAR_ARMCM3_LM )
  // Register this task with RTOS task initiator
  RTOS_RegisterApp( task_id, Ep5_RTOS_MSG_EVT );
#endif
}

/*********************************************************************
 * @fn      Ep5_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
uint16 Ep5_ProcessEvent( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  afDataConfirm_t *afDataConfirm;

  // Data Confirmation message fields
  byte sentEP;
  ZStatus_t sentStatus;
  byte sentTransID;       // This should match the value sent
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( Ep5_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZDO_CB_MSG:
          Ep5_ProcessZDOMsgs( (zdoIncomingMsg_t *)MSGpkt );
          break;

        case KEY_CHANGE:
          Ep5_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case AF_DATA_CONFIRM_CMD:
          // This message is received as a confirmation of a data packet sent.
          // The status is of ZStatus_t type [defined in ZComDef.h]
          // The message fields are defined in AF.h
          afDataConfirm = (afDataConfirm_t *)MSGpkt;
          sentEP = afDataConfirm->endpoint;
          sentStatus = afDataConfirm->hdr.status;
          sentTransID = afDataConfirm->transID;
          (void)sentEP;
          (void)sentTransID;

          // Action taken when confirmation is received.
          if ( sentStatus != ZSuccess )
          {
            // The data wasn't delivered -- Do something
          }
          break;

        case AF_INCOMING_MSG_CMD:
          Ep5_MessageMSGCB( MSGpkt );
          break;

        case ZDO_STATE_CHANGE:
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( Ep5_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  
#if defined( IAR_ARMCM3_LM )
  // Receive a message from the RTOS queue
  if ( events & Ep5_RTOS_MSG_EVT )
  {
    // Process message from RTOS queue
    Ep5_ProcessRtosMessage();

    // return unprocessed events
    return (events ^ Ep5_RTOS_MSG_EVT);
  }
#endif

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * Event Generation Functions
 */

/*********************************************************************
 * @fn      Ep5_ProcessZDOMsgs()
 *
 * @brief   Process response messages
 *
 * @param   none
 *
 * @return  none
 */
static void Ep5_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg )
{
  switch ( inMsg->clusterID )
  {
    case End_Device_Bind_rsp:
      if ( ZDO_ParseBindRsp( inMsg ) == ZSuccess )
      {
        // Light LED
        HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
      }
#if defined( BLINK_LEDS )
      else
      {
        // Flash LED to show failure
        HalLedSet ( HAL_LED_4, HAL_LED_MODE_FLASH );
      }
#endif
      break;

    case Match_Desc_rsp:
      {
        ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp( inMsg );
        if ( pRsp )
        {
          if ( pRsp->status == ZSuccess && pRsp->cnt )
          {
            Ep5_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
            Ep5_DstAddr.addr.shortAddr = pRsp->nwkAddr;
            // Take the first endpoint, Can be changed to search through endpoints
            Ep5_DstAddr.endPoint = pRsp->epList[0];

            // Light LED
            HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
          }
          osal_mem_free( pRsp );
        }
      }
      break;
  }
}

/*********************************************************************
 * @fn      Ep5_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_3
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void Ep5_HandleKeys( uint8 shift, uint8 keys )
{
  zAddrType_t dstAddr;

  // Shift is used to make each button/switch dual purpose.
  if ( shift )
  {
    if ( keys & HAL_KEY_SW_1 )
    {
    }
    if ( keys & HAL_KEY_SW_2 )
    {
    }
    if ( keys & HAL_KEY_SW_3 )
    {
    }
    if ( keys & HAL_KEY_SW_4 )
    {
    }
  }
  else
  {
    if ( keys & HAL_KEY_SW_1 )
    {
      // Since SW1 isn't used for anything else in this application...
#if defined( SWITCH1_BIND )
      // we can use SW1 to simulate SW2 for devices that only have one switch,
      keys |= HAL_KEY_SW_2;
#elif defined( SWITCH1_MATCH )
      // or use SW1 to simulate SW4 for devices that only have one switch
      keys |= HAL_KEY_SW_4;
#endif
    }

    if ( keys & HAL_KEY_SW_2 )
    {
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );

      // Initiate an End Device Bind Request for the mandatory endpoint
      dstAddr.addrMode = Addr16Bit;
      dstAddr.addr.shortAddr = 0x0000; // Coordinator
      ZDP_EndDeviceBindReq( &dstAddr, NLME_GetShortAddr(),
                            Ep5_epDesc.endPoint,
                            Ep5_PROFID,
                            Ep5_MAX_CLUSTERS, (cId_t *)Ep5_ClusterList,
                            Ep5_MAX_CLUSTERS, (cId_t *)Ep5_ClusterList,
                            FALSE );
    }

    if ( keys & HAL_KEY_SW_3 )
    {
    }

    if ( keys & HAL_KEY_SW_4 )
    {
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
      // Initiate a Match Description Request (Service Discovery)
      dstAddr.addrMode = AddrBroadcast;
      dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
      ZDP_MatchDescReq( &dstAddr, NWK_BROADCAST_SHORTADDR,
                        Ep5_PROFID,
                        Ep5_MAX_CLUSTERS, (cId_t *)Ep5_ClusterList,
                        Ep5_MAX_CLUSTERS, (cId_t *)Ep5_ClusterList,
                        FALSE );
    }
  }
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      Ep5_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
static void Ep5_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  nodeUARTData node1_Ep5;
  
  switch(pkt->clusterId)
  {
  case SHORT_ADDR_INFO_CLUSTERID:
    //get the shortAddr of scr
    Ep5_DstAddr.addr.shortAddr = pkt->srcAddr.addr.shortAddr;
    break;
    
  case TEMP_HUMI_INFO_CLUSTERID:
    node1_Ep5.nodeUARTData_t.nodeName = 5 + '0';
    node1_Ep5.nodeUARTData_t.temperature[0] = pkt->cmd.Data[0];
    node1_Ep5.nodeUARTData_t.temperature[1] = pkt->cmd.Data[1];
    node1_Ep5.nodeUARTData_t.humidity[0] = pkt->cmd.Data[2];
    node1_Ep5.nodeUARTData_t.humidity[1] = pkt->cmd.Data[3];
    
    HalUARTWrite(0,node1_Ep5.buf,5);
    HalLedSet ( HAL_LED_ALL, HAL_LED_MODE_TOGGLE );
    break;
  }
}

//Modify by Frank
//send a message(Led) to node5
void Ep5_SendTheMessage( uint8 isTurnOnLed )
{
  bool StatusLed;
  if(isTurnOnLed == '1')
  {
    StatusLed = true; //turn on led
  }else
  {
    StatusLed = false; //turn off led
  }
  
  if ( AF_DataRequest( &Ep5_DstAddr, &Ep5_epDesc,
                       (uint16)StatusLed,
                       0,
                       NULL,
                       &Ep5_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
    // Successfully requested to be sent.
  }
  else
  {
    // Error occurred in request to send.
  }
}

#if defined( IAR_ARMCM3_LM )
/*********************************************************************
 * @fn      Ep5_ProcessRtosMessage
 *
 * @brief   Receive message from RTOS queue, send response back.
 *
 * @param   none
 *
 * @return  none
 */
static void Ep5_ProcessRtosMessage( void )
{
  osalQueue_t inMsg;

  if ( osal_queue_receive( OsalQueue, &inMsg, 0 ) == pdPASS )
  {
    uint8 cmndId = inMsg.cmnd;
    uint32 counter = osal_build_uint32( inMsg.cbuf, 4 );

    switch ( cmndId )
    {
      case CMD_INCR:
        counter += 1;  /* Increment the incoming counter */
                       /* Intentionally fall through next case */

      case CMD_ECHO:
      {
        userQueue_t outMsg;

        outMsg.resp = RSP_CODE | cmndId;  /* Response ID */
        osal_buffer_uint32( outMsg.rbuf, counter );    /* Increment counter */
        osal_queue_send( UserQueue1, &outMsg, 0 );  /* Send back to UserTask */
        break;
      }
      
      default:
        break;  /* Ignore unknown command */    
    }
  }
}
#endif

/*********************************************************************
 */
