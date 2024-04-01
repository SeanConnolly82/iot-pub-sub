#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <iostream>
#include <cstring>
#include <string>
#include <cmath>
#include <json-c/json.h>
#include "MQTTClient.h"

namespace subscriber {
class Subscriber {
private:
    static void delivered(void *context, MQTTClient_deliveryToken dt);
    static int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
    static void connlost(void *context, char *cause);
    MQTTClient_connectOptions opts;
    std::string payload;
public:
    Subscriber();
    virtual void processMessage();
    virtual void setMQTTCallbacks(MQTTClient& client);
    virtual double parseCpuTemp(const std::string& jsonString);
    virtual int run(MQTTClient& client);
};
}
#endif /* SUBSCRIBER_H */