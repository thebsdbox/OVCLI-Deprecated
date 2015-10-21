//
//  libinfluxdb.c
//  OVCLI
//
//  Created by Daniel Finneran on 13/10/2015.
//  Copyright © 2015 Daniel Finneran. All rights reserved.
//

#include "libinfluxdb.h"
#include "OVHttps.h"
#include "OVUtils.h"

#include "jansson.h"

ovServerHardware *parse(json_t *json)
{
    // This will work through the json that is returned and produce a struct with the correct info
    if (!json) {
        return NULL;
    }
    ovServerHardware *serverHardware;
    serverHardware = malloc(sizeof(ovServerHardware));
    
    // Parse the JSON to find the correct embedded object
    json_t *resource = json_object_get(json, "resource");
    char *serverURI = (char *)json_string_value(json_object_get(resource, "uri"));
    strcpy(serverHardware->serverURI, serverURI);
    json_t *memberArray = json_object_get(resource, "resourceDataList");
    // Get the first object in the array and use this object to parse for data
    json_t *metricArray = json_object_get(json_array_get(memberArray, 0), "metricSampleList");

    if (json_array_size(metricArray) != 0) {
        size_t index;
        json_t *value;
        json_array_foreach(metricArray, index, value) {
            char *name = (char *)json_string_value(json_object_get(value, "name"));
            if (stringMatch(name, "AveragePower")) {
                serverHardware->avgPower  = json_integer_value(json_array_get(json_object_get(value, "valueArray"),0));
            }
            if (stringMatch(name, "PeakPower")) {
                serverHardware->peakPower  = json_integer_value(json_array_get(json_object_get(value, "valueArray"),0));
            }
            if (stringMatch(name, "PowerCap")) {
                serverHardware->powerCap  = json_integer_value(json_array_get(json_object_get(value, "valueArray"),0));
            }
            if (stringMatch(name, "AmbientTemperature")) {
                serverHardware->temp  = json_integer_value(json_array_get(json_object_get(value, "valueArray"),0));
            }
            if (stringMatch(name, "CpuUtilization")) {
                serverHardware->cpuUse  = json_integer_value(json_array_get(json_object_get(value, "valueArray"),0));
            }
            if (stringMatch(name, "CpuAverageFreq")) {
                serverHardware->cpu  = json_integer_value(json_array_get(json_object_get(value, "valueArray"),0));
            }
        }
    }
    return serverHardware;
}

int postToServer(char *address, char *database, char *data)
{
    // This function will make use of influxDBs API ennpoints.
    
    json_error_t error; // Used as a passback for error data during json processing
    json_t *root = json_loads(data, 0, &error);
    ovServerHardware *serverHardware = parse(root);
    if (serverHardware == NULL) {
        return  -1;
    }
    char influxData[2048]; // A buffer for the data to be passed to the influx HTTP API
    char influxURL[1024]; // A buffer to store the URL for the influx Database
    sprintf(influxURL, "%s:8086/write?db=%s", address, database);
    sprintf(influxData, "server_stats,server=%s avgPower=%ld,peakPower=%ld,powerCap=%ld,ambTemp=%ld,cpuUtil=%ld,cpuAvg=%ld",
            serverHardware->serverURI,
            serverHardware->avgPower,
            serverHardware->peakPower,
            serverHardware->powerCap,
            serverHardware->temp,
            serverHardware->cpuUse,
            serverHardware->cpu);
    
    postRequestWithUrlAndData(influxURL, influxData);

    return 0;
}

