
#include "ADXL345Publisher.h"

#define ADDRESS    "tcp://192.168.0.243:1883"
#define CLIENTID   "rpi1"

using namespace ee513;
using namespace publisher;

int main(int argc, char* argv[]) {

    ADXL345 accelerometer(1, 0x53);
    accelerometer.setResolution(ADXL345::NORMAL);
    accelerometer.setRange(ADXL345::PLUSMINUS_4_G);
    
    ADXL345Publisher publisher;

    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    // Run the MQTT publishing loop
    publisher.run(client, &accelerometer);

    return 0;
}