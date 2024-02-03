#include "cJSON.h"
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

#define CONFIG_LASER_MODULE_NUMBER 1

char json_format[4096];
char message_string[4096];

void read_set_json() {
    FILE* fp;

    // Read in json format
    fp = fopen("laser_set.json", "r");
    int len = fread(json_format, 1, 4096, fp);
    json_format[len] = '\0';
    fclose(fp);
}

int main(int argc, char *argv[]) {
    cJSON *settings_json = NULL, *laser1_json, *laser2_json;

    printf("Reading JSON file...");
    printf("\n");
    read_set_json();
    printf("Reading ended");
    printf("\n");

    printf("MQTT_EVENT_DATA");
    printf("\n");

    if (settings_json != NULL)
        cJSON_Delete(settings_json);
    settings_json = cJSON_Parse(json_format);
    // uint8_t report_interval = (uint8_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(settings_json, "report_interval"));
#if CONFIG_LASER_MODULE_NUMBER == 1
    laser1_json = cJSON_GetObjectItemCaseSensitive(settings_json, "laser_976_1");
    laser2_json = cJSON_GetObjectItemCaseSensitive(settings_json, "laser_976_2");
#else
    laser1_json = cJSON_GetObjectItemCaseSensitive(settings_json, "laser_1480_1");
    laser2_json = cJSON_GetObjectItemCaseSensitive(settings_json, "laser_1480_2");
#endif
    // Parse enable's
    cJSON *laser1_enable_json = cJSON_GetObjectItemCaseSensitive(laser1_json, "enabled");
    bool laser1_enabled = (bool)cJSON_IsTrue(laser1_enable_json);
    cJSON *laser2_enable_json = cJSON_GetObjectItemCaseSensitive(laser2_json, "enabled");
    bool laser2_enabled = (bool)cJSON_IsTrue(laser2_enable_json);
    printf("Enable/Disable| Laser1: %d, Laser2: %d", laser1_enabled, laser2_enabled);
    printf("\n");

    // Parse desired temperature
    uint8_t laser1_desired_temp = (uint8_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(laser1_json, "desired_temperature"));
    uint8_t laser2_desired_temp = (uint8_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(laser2_json, "desired_temperature"));
    printf("Desired temperature| Laser1: %d, Laser2: %d", laser1_desired_temp, laser2_desired_temp);
    printf("\n");

    // Parse desired monitor diode current
    uint32_t laser1_desired_current = (uint32_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(laser1_json, "desired_laser_diode_current"));
    uint32_t laser2_desired_current = (uint32_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(laser2_json, "desired_laser_diode_current"));
    printf("Desired monitor diode| Laser1: %d, Laser2: %d", laser1_desired_current, laser2_desired_current);
    printf("\n");
}