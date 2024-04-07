#include "MQTTClient.h"
#include "ADXL345Subscriber.h"

#define ADDRESS    "tcp://192.168.0.243:1883"
#define CLIENTID   "rpi2"
#define MAXTEMP    30.0

using namespace subscriber;

int main(int argc, char* argv[]) {
    
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    ADXL345Subscriber sub;
    sub.setCpuLimit(MAXTEMP);
    int rc = sub.run(client);
    return rc;
};
