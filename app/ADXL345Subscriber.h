#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <iostream>
#include <cstring>
#include <string>
#include <cmath>
#include <json-c/json.h>
#include "MQTTClient.h"
#include "ADXL345Subscriber.h"
#include "wiringPi.h"

namespace subscriber {

/**
 * @class ADXL345Subscriber
 * @brief Class for subscribing to MQTT messages and processing ADXL345 sensor data.
 */
class ADXL345Subscriber {
private:
    static void delivered(void *context, MQTTClient_deliveryToken dt);
    static int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
    static void connlost(void *context, char *cause);
    MQTTClient_connectOptions opts;
    float maxCPUTemp;
    float maxPitch;
    float maxRoll;
public:
    ADXL345Subscriber();
    struct SensorData {
        double cpuTemp;
        double pitch;
        double roll;
    };
    virtual void setMaxLimits(float maxCPUTemp, float maxPitch, float maxRoll);
    virtual void processMessage(std::string payload);
    virtual void initialiseLEDS();
    virtual int checkWithinLimit(float value, float limit);
    virtual void setMQTTCallbacks(MQTTClient& client);
    SensorData parseJSONMessage(const std::string& jsonString);
    virtual int run(MQTTClient& client);
};
}
#endif /* SUBSCRIBER_H */