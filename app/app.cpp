
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <chrono>
#include <thread>
#include <csignal>
#include "MQTTClient.h"
#include "ADXL345.h"
#include "ADXL345Publisher.h"

using namespace std;
using namespace device;
using namespace ee513;
using namespace publisher;

#define CPU_TEMP "/sys/class/thermal/thermal_zone0/temp"
#define ADDRESS    "tcp://192.168.0.243:1883"
#define CLIENTID   "rpi1"
#define AUTHMETHOD "sean"
#define AUTHTOKEN  "password123"
#define TOPIC      "ee513/test"
#define QOS        1
#define TIMEOUT    1000L
#define PUBLISH_INTERVAL 1 // in seconds
#define LWT_MESSAGE "The ADXL345 has disconnected unexpectedly"
#define LWT_RETAINED 0

int main(int argc, char* argv[]) {
    // Initialize MQTTPublisher object
    cout <<  "start" << endl;
    ADXL345 accelerometer(1, 0x53);
    accelerometer.setResolution(ADXL345::NORMAL);
    accelerometer.setRange(ADXL345::PLUSMINUS_4_G);
    cout <<  "device created" << endl;
   // MQTTClient client;

    //MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    cout <<  "client created" << endl;
    ADXL345Publisher publisher;
    cout <<  "publisher created" << endl;

    // // signal(SIGINT, [&](int signal) { ADXL345Publisher::signalHandler(signal, &publisher); });
    
    // // Run the MQTT publishing loop
    publisher.run(&accelerometer);

    return 0;
}