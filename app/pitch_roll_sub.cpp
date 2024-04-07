#include "MQTTClient.h"
#include "ADXL345Subscriber.h"

#define ADDRESS    "tcp://192.168.0.243:1883"
#define CLIENTID   "rpi2"
#define MAXTTEMP   30.0
#define MAXPITCH   5.0
#define MAXROLL    5.0

using namespace subscriber;

int main(int argc, char* argv[]) {
    
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    ADXL345Subscriber sub;
    sub.setPitchLimit(MAXPITCH);
    sub.setRollLimit(MAXROLL);
    int rc = sub.run(client);
    return rc;
};
