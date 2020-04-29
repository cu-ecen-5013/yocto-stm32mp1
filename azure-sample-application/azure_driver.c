/**
 * azure_driver.c
 * 
 * This file contains the implementation for the azure driver. The functions
 * here allow you to send data to the azure cloud. To use this file, note that
 * you will need to set the connectionString variable below to your own device
 * connection string (which can be found on your IoT Hub online).
 * 
 * This file is based on the iothub_convenience_sample example from the azure
 * C SDK, available here:
 * https://github.com/Azure/azure-iot-sdk-c/tree/master/iothub_client/samples/iothub_convenience_sample
 * 
 *  - Bryan Cisneros
 */

// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include "azure_driver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <azureiot/iothub.h>
#include <azureiot/iothub_device_client.h>
#include <azureiot/iothub_client_options.h>
#include <azureiot/iothub_message.h>
#include <azure_c_shared_utility/threadapi.h>
#include <azure_c_shared_utility/crt_abstractions.h>
#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/shared_util_options.h>
#include <azure_c_shared_utility/tickcounter.h>

// The protocol you wish to use should be uncommented
// 
#define SAMPLE_MQTT
//#define SAMPLE_MQTT_OVER_WEBSOCKETS
//#define SAMPLE_AMQP
//#define SAMPLE_AMQP_OVER_WEBSOCKETS
//#define SAMPLE_HTTP

#ifdef SAMPLE_MQTT
    #include <azureiot/iothubtransportmqtt.h>
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
    #include <azureiot/iothubtransportmqtt_websockets.h>
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
    #include <azureiot/iothubtransportamqp.h>
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
    #include <azureiot/iothubtransportamqp_websockets.h>
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
    #include <azureiot/iothubtransporthttp.h>
#endif // SAMPLE_HTTP

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
#include "certs.h"
#endif // SET_TRUSTED_CERT_IN_SAMPLES

#define DO_WORK_10_MS (10)

/* Device connection string  */
static const char* connectionString = "ENTER YOUR CONNECTION STRING HERE";

static size_t g_message_count_send_confirmations = 0; // number of message confirmations received

IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol;
IOTHUB_DEVICE_CLIENT_HANDLE device_handle;
IOTHUB_MESSAGE_HANDLE message_handle;

static IOTHUBMESSAGE_DISPOSITION_RESULT receive_msg_callback(IOTHUB_MESSAGE_HANDLE message, void* user_context)
{
    (void)user_context;
    const char* messageId;
    const char* correlationId;

    // Message properties
    if ((messageId = IoTHubMessage_GetMessageId(message)) == NULL)
    {
        messageId = "<unavailable>";
    }

    if ((correlationId = IoTHubMessage_GetCorrelationId(message)) == NULL)
    {
        correlationId = "<unavailable>";
    }

    IOTHUBMESSAGE_CONTENT_TYPE content_type = IoTHubMessage_GetContentType(message);
    if (content_type == IOTHUBMESSAGE_BYTEARRAY)
    {
        const unsigned char* buff_msg;
        size_t buff_len;

        if (IoTHubMessage_GetByteArray(message, &buff_msg, &buff_len) != IOTHUB_MESSAGE_OK)
        {
            (void)printf("Failure retrieving byte array message\r\n");
        }
        else
        {
            (void)printf("Received Binary message\r\nMessage ID: %s\r\n Correlation ID: %s\r\n Data: <<<%.*s>>> & Size=%d\r\n", messageId, correlationId, (int)buff_len, buff_msg, (int)buff_len);
        }
    }
    else
    {
        const char* string_msg = IoTHubMessage_GetString(message);
        if (string_msg == NULL)
        {
            (void)printf("Failure retrieving byte array message\r\n");
        }
        else
        {
            (void)printf("Received String Message\r\nMessage ID: %s\r\n Correlation ID: %s\r\n Data: <<<%s>>>\r\n", messageId, correlationId, string_msg);
        }
    }
    return IOTHUBMESSAGE_ACCEPTED;
}


static int device_method_callback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback)
{
    const char* SetTelemetryIntervalMethod = "SetTelemetryInterval";
    const char* device_id = (const char*)userContextCallback;
    char* end = NULL;
    int newInterval;

    int status = 501;
    const char* RESPONSE_STRING = "{ \"Response\": \"Unknown method requested.\" }";

    static int g_interval = 0;

    printf("\r\nDevice Method called for device %s\r\n", device_id);
    printf("Device Method name:    %s\r\n", method_name);
    printf("Device Method payload: %.*s\r\n", (int)size, (const char*)payload);

    if (strcmp(method_name, SetTelemetryIntervalMethod) == 0)
    {
        if (payload)
        {
            newInterval = (int)strtol((char*)payload, &end, 10);

            // Interval must be greater than zero.
            if (newInterval > 0)
            {
                // expect sec and covert to ms
                g_interval = 1000 * (int)strtol((char*)payload, &end, 10);
                printf("interval updated to %d\r\n", g_interval);
                status = 200;
                RESPONSE_STRING = "{ \"Response\": \"Telemetry reporting interval updated.\" }";
            }
            else
            {
                status = 500;
                RESPONSE_STRING = "{ \"Response\": \"Invalid telemetry reporting interval.\" }";
            }
        }
    }

    printf("\r\nResponse status: %d\r\n", status);
    printf("Response payload: %s\r\n\r\n", RESPONSE_STRING);

    *resp_size = strlen(RESPONSE_STRING);
    if ((*response = malloc(*resp_size)) == NULL)
    {
        status = -1;
    }
    else
    {
        memcpy(*response, RESPONSE_STRING, *resp_size);
    }
    
    return status;
}


static void connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
{
    (void)reason;
    (void)user_context;
    // This sample DOES NOT take into consideration network outages.
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
    {
        (void)printf("The device client is connected to iothub\r\n");
    }
    else
    {
        (void)printf("The device client has been disconnected\r\n");
    }
}

static void send_confirm_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    (void)userContextCallback;
    // When a message is sent this callback will get invoked
    g_message_count_send_confirmations++;
    (void)printf("Confirmation callback received for message %lu with result %s\r\n", (unsigned long)g_message_count_send_confirmations, MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
}

bool azure_init(void)
{
    bool error = false;

    // Select the Protocol to use with the connection
#ifdef SAMPLE_MQTT
    protocol = MQTT_Protocol;
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
    protocol = MQTT_WebSocket_Protocol;
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
    protocol = AMQP_Protocol;
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
    protocol = AMQP_Protocol_over_WebSocketsTls;
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
    protocol = HTTP_Protocol;
#endif // SAMPLE_HTTP

    if (!error)
    {
        // Used to initialize IoTHub SDK subsystem
        if (IoTHub_Init() != 0)
        {
            printf("IoTHub_Init() failed!\r\n");
            error = true;
        }
    }

    if (!error)
    {
        printf("Creating IoTHub handle\r\n");
        // Create the iothub handle here
        device_handle = IoTHubDeviceClient_CreateFromConnectionString(connectionString, protocol);
        if (device_handle == NULL)
        {
            printf("Failure creating Iothub device.  Hint: Check you connection string.\r\n");
            error = true;
        }
    }
    
    if (!error)
    {
        // Setting message callback to get C2D messages
        if ( IOTHUB_CLIENT_OK != IoTHubDeviceClient_SetMessageCallback(device_handle, receive_msg_callback, NULL))
        {
            printf("Failed to set message callback!\r\n");
            error = true;
        }
    }

    if (!error)
    {
        // Setting method callback to handle a SetTelemetryInterval method to control
        //   how often telemetry messages are sent from the simulated device.
        if ( IOTHUB_CLIENT_OK != IoTHubDeviceClient_SetDeviceMethodCallback(device_handle, device_method_callback, NULL))
        {
            printf("Failed to set device method callback!\r\n");
            error = true;
        }
    }

    if (!error)
    {
        // Setting connection status callback to get indication of connection to iothub
        if ( IOTHUB_CLIENT_OK != IoTHubDeviceClient_SetConnectionStatusCallback(device_handle, connection_status_callback, NULL))
        {
            printf("Failed to set connection status callback!\r\n");
            error = true;
        }
    }

    if (!error)
    {
        // Setting Log Tracing. 
        // Log tracing is supported in MQTT and AMQP. Not HTTP.
#ifndef SAMPLE_HTTP
        bool traceOn = true;
        if ( IOTHUB_CLIENT_OK != IoTHubDeviceClient_SetOption(device_handle, OPTION_LOG_TRACE, &traceOn))
        {
            printf("Failed to set log tracing!\r\n");
            error = true;
        }
#endif
    }

    if (!error)
    {
        // Setting the frequency of DoWork calls by the underlying process thread.
        // The value ms_delay is a delay between DoWork calls, in milliseconds. 
        // ms_delay can only be between 1 and 100 milliseconds. 
        // Without the SetOption, the delay defaults to 1 ms. 
        tickcounter_ms_t ms_delay = DO_WORK_10_MS;
        if ( IOTHUB_CLIENT_OK != IoTHubDeviceClient_SetOption(device_handle, OPTION_DO_WORK_FREQUENCY_IN_MS, &ms_delay))
        {
            printf("Failed to set do work frequency!\r\n");
            error = true;
        }
    }

    return error;
}

void azure_deinit(void)
{
    // Clean up the iothub sdk handle
    IoTHubDeviceClient_Destroy(device_handle);

    // Free all the sdk subsystem
    IoTHub_Deinit();
}

bool azure_send_measurement(char* name, float data)
{
    bool error = false;
    char telemetry_msg_buffer[80];
    sprintf(telemetry_msg_buffer, "{\"%s\":%.3f}", name, data);

    if (!error)
    {
        message_handle = IoTHubMessage_CreateFromString(telemetry_msg_buffer);
        if (message_handle == NULL)
        {
            printf("Failed to create message from string!\r\n");
            error = true;
        }
    }

    // Set Message property
    if (!error)
    {
        if (IOTHUB_MESSAGE_OK != IoTHubMessage_SetMessageId(message_handle, "MSG_ID"))
        {
            printf("Failed to set message id!\r\n");
            error = true;
        }
    }

    if (!error)
    {
        if (IOTHUB_MESSAGE_OK != IoTHubMessage_SetCorrelationId(message_handle, "CORE_ID"))
        {
            printf("Failed to set correlation id!\r\n");
            error = true;
        }
    }

    if (!error)
    {
        if (IOTHUB_MESSAGE_OK != IoTHubMessage_SetContentTypeSystemProperty(message_handle, "application%2fjson"))
        {
            printf("Failed to set content type!\r\n");
            error = true;
        }
    }

    if (!error)
    {
        if (IOTHUB_MESSAGE_OK != IoTHubMessage_SetContentEncodingSystemProperty(message_handle, "utf-8"))
        {
            printf("Failed to set content encoding\r\n");
            error = true;
        }
    }

    if (!error)
    {
        printf("\r\nSending message to IoTHub\r\nMessage: %s\r\n", telemetry_msg_buffer);
        if (IOTHUB_CLIENT_OK != IoTHubDeviceClient_SendEventAsync(device_handle, message_handle, send_confirm_callback, NULL))
        {
            printf("Failed to send message!\r\n");
            error = true;
        }
    }

    if (message_handle != NULL)
    {
        // The message is copied to the sdk so the we can destroy it
        IoTHubMessage_Destroy(message_handle);
    }

    return error;
}
