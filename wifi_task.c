#include <stdio.h>
#include "wifi_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "cy_wcm.h"

#include "wifi_helper.h"
typedef enum {
    WIFI_CMD_ENABLE,

} wifi_cmd_t;

typedef struct {
    wifi_cmd_t cmd;
    void *arg0;
    void *arg1;
    void *arg2;
} wifi_cmdMsg_t;


static QueueHandle_t wifi_cmdQueue;
static bool wifi_initialized=false;
static cy_wcm_interface_t wifi_network_mode;



static void wifi_network_event_cb(cy_wcm_event_t event, cy_wcm_event_data_t *event_data)
{
    cy_wcm_ip_address_t ip_addr;

    switch(event)
    {
        case CY_WCM_EVENT_CONNECTING:            /**< STA connecting to an AP.         */
            printf("Connecting to AP ... \n");
        break;
        case CY_WCM_EVENT_CONNECTED:             /**< STA connected to the AP.         */
            printf("Connected to AP and network is up !! \n");
        break;
        case CY_WCM_EVENT_CONNECT_FAILED:        /**< STA connection to the AP failed. */
            printf("Connection to AP Failed ! \n");
        break;
        case CY_WCM_EVENT_RECONNECTED:          /**< STA reconnected to the AP.       */
            printf("Network is up again! \n");
        break;
        case CY_WCM_EVENT_DISCONNECTED:         /**< STA disconnected from the AP.    */
            printf("Network is down! \n");
        break;
        case CY_WCM_EVENT_IP_CHANGED:           /**< IP address change event. This event is notified after connection, re-connection, and IP address change due to DHCP renewal. */
                cy_wcm_get_ip_addr(wifi_network_mode, &ip_addr, 1);
                printf("Station IP Address Changed: %s\n",wifi_ntoa(&ip_addr));
        break;
        case CY_WCM_EVENT_STA_JOINED_SOFTAP:    /**< An STA device connected to SoftAP. */
            printf("STA Joined: %s\n",wifi_mac_to_string(event_data->sta_mac));
        break;
        case CY_WCM_EVENT_STA_LEFT_SOFTAP:      /**< An STA device disconnected from SoftAP. */
            printf("STA Left: %s\n",wifi_mac_to_string(event_data->sta_mac));
        break;
    }

}


static void wifi_enable(cy_wcm_interface_t interface)
{
	cy_rslt_t result;
	cy_wcm_config_t config = {.interface = interface}; 
	result = cy_wcm_init(&config); // Initialize the connection manager
    CY_ASSERT(result == CY_RSLT_SUCCESS);

    result = cy_wcm_register_event_callback(wifi_network_event_cb);
	CY_ASSERT(result == CY_RSLT_SUCCESS);

    wifi_network_mode = interface;
    wifi_initialized = true;
    
    printf("\nWi-Fi Connection Manager initialized\n");
    
}

void wifi_task(void *arg)
{
    wifi_cmdQueue = xQueueCreate(10,sizeof(wifi_cmdMsg_t));

    wifi_cmdMsg_t msg;

    while(1)
    {
        xQueueReceive(wifi_cmdQueue,&msg,portMAX_DELAY);
        switch(msg.cmd)
        {
            case WIFI_CMD_ENABLE:
                wifi_enable((cy_wcm_interface_t)msg.arg0);
            break;

        }
    }
}

bool wifi_cmd_enable(char *interface)
{
    wifi_cmdMsg_t msg;
    msg.cmd = WIFI_CMD_ENABLE;
    msg.arg0 = (void *)CY_WCM_INTERFACE_TYPE_STA;

    if(strcmp(interface,"STA") == 0)
        msg.arg0 = (void *)CY_WCM_INTERFACE_TYPE_STA;

    else if(strcmp(interface,"AP") == 0)
        msg.arg0 = (void *)CY_WCM_INTERFACE_TYPE_AP;

    else if(strcmp(interface,"APSTA") == 0)
        msg.arg0 = (void *)CY_WCM_INTERFACE_TYPE_AP_STA;
    
    else
    {
        printf("Legal options are STA, AP, APSTA\n");
        return false;
    }

    xQueueSend(wifi_cmdQueue,&msg,0);
    return true;
}
