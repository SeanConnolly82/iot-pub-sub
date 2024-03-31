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

namespace publisher {

// Constructor definition with member initializer list
ADXL345Publisher::ADXL345Publisher() {

    cout << "Constructor" << endl;

    this->exitFlag = false;

    this->pubmsg = MQTTClient_message_initializer;
    this->opts = MQTTClient_connectOptions_initializer;
    this->willOptions = MQTTClient_willOptions_initializer;

    this->opts.keepAliveInterval = 20;
    this->opts.cleansession = 1;
    this->opts.username = AUTHMETHOD;
    this->opts.password = AUTHTOKEN;

    this->willOptions.topicName = TOPIC;
    this->willOptions.message = LWT_MESSAGE;
    this->willOptions.qos = QOS;
    this->willOptions.retained = LWT_RETAINED;

    this->opts.will = &willOptions;
}

void ADXL345Publisher::setExitFlag(bool value) {
    exitFlag = value;
}

void ADXL345Publisher::signalHandler(int signal, ADXL345Publisher* publisher) {
    cout << "Received signal: " << signal << ", exiting gracefully..." << endl;
    publisher->setExitFlag(true);
}

float ADXL345Publisher::getCPUTemperature() {
    int cpuTemp;
    fstream fs;
    fs.open(CPU_TEMP, fstream::in);
    fs >> cpuTemp;
    fs.close();
    return (((float)cpuTemp)/1000);
}

string ADXL345Publisher::getDeviceData(ADXL345* ADXL345Device) {
    stringstream dataStream;
    ADXL345Device->readSensorState();
    dataStream << "{Pitch: " << ADXL345Device->getPitch() << ", Roll: " << ADXL345Device->getRoll() << "}";
    return dataStream.str();
}

void ADXL345Publisher::run(ADXL345* ADXL345Device) {
    // Connect to the MQTT broker
    int rc;
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    if ((rc = MQTTClient_connect(client, &this->opts)) != MQTTCLIENT_SUCCESS) {
        cout << "Failed to connect, return code " << rc << endl;
        return;
    }
    // Main loop for publishing MQTT messages
    while (!this->exitFlag) {
        string deviceData = getDeviceData(ADXL345Device);
        char str_payload[1000];
        sprintf(str_payload,
            "{\"Accelerometer\": %s , \"CPU Temp\": %f}", deviceData.c_str(), getCPUTemperature());
        pubmsg.payload = str_payload;
        pubmsg.payloadlen = strlen(str_payload);
        pubmsg.qos = QOS;
        pubmsg.retained = 0;

        MQTTClient_publishMessage(client, TOPIC, &this->pubmsg, &this->token);
        cout << str_payload << endl;

        rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
        std::this_thread::sleep_for(std::chrono::seconds(PUBLISH_INTERVAL));
    }

    // Disconnect MQTT client after the loop
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
};
}

