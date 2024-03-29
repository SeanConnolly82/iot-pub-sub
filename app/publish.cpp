#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream> 
#include <chrono>
#include <thread>
#include "MQTTClient.h"
#include "ADXL345.h"

#define CPU_TEMP "/sys/class/thermal/thermal_zone0/temp"

using namespace std;
using namespace EE513;

// Please replace the following address with the address of your server
#define ADDRESS    "tcp://192.168.0.243:1883"
#define CLIENTID   "rpi1"
#define AUTHMETHOD "sean"
#define AUTHTOKEN  "password123"
#define TOPIC      "ee513/CPUTemp"
#define QOS        1
#define TIMEOUT    10000L
#define PUBLISH_INTERVAL 10 // in seconds

string getDeviceData(ADXL345* device) {
    stringstream dataStream;
    device->readSensorState();
    dataStream << "Pitch: " << device->getPitch() << ", Roll: " << device->getRoll();
    return dataStream.str();
}

int main(int argc, char* argv[]) {
    char str_payload[100]; // Set your max message size here

    ADXL345 device(1, 0x53);
    device.setResolution(ADXL345::NORMAL);
    device.setRange(ADXL345::PLUSMINUS_4_G);

    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    while (true) {
        int rc;
        if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
            cout << "Failed to connect, return code " << rc << endl;
            return -1;
        }
        string deviceData = getDeviceData(&device);
        sprintf(str_payload, "{\"d\":{\"CPUTemp\": %s }}", deviceData);
        pubmsg.payload = str_payload;
        pubmsg.payloadlen = strlen(str_payload);
        pubmsg.qos = QOS;
        pubmsg.retained = 0;

        MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);

        cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
            " seconds for publication of " << str_payload <<
            " \non topic " << TOPIC << " for ClientID: " << CLIENTID << endl;

        rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
        cout << "Message with token " << (int)token << " delivered." << endl;
        // Disconnect MQTT client after publishing
        MQTTClient_disconnect(client, 10000);
        // Sleep for 10 seconds
        std::this_thread::sleep_for(std::chrono::seconds(PUBLISH_INTERVAL));
    }

    // Disconnect MQTT client after the loop
    MQTTClient_destroy(&client);

    return 0;
}
