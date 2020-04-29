/**
 * azure_driver.h
 * 
 * This file contains the API for the azure driver. The functions here allow
 * you to send data to the azure cloud
 * 
 *  - Bryan Cisneros
 */

#include <stdbool.h>

bool azure_init(void);

void azure_deinit(void);

bool azure_send_measurement(char* name, float data);