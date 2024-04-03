#include <iostream>
#include <cstring>
#include <string>
#include <cmath>
#include <json-c/json.h>
#include "MQTTClient.h"
#include "ADXL345Subscriber.h"

using namespace std;
using namespace subscriber;

#define ADDRESS     "tcp://192.168.0.243:1883"
#define CLIENTID    "rpi2"
#define AUTHMETHOD  "sean"
#define AUTHTOKEN   "password123"
#define TOPIC       "ee513/assignment2"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

volatile MQTTClient_deliveryToken deliveredtoken;

namespace subscriber {
ADXL345Subscriber::ADXL345Subscriber() {
    this->opts = MQTTClient_connectOptions_initializer;
    this->opts.keepAliveInterval = 20;
    this->opts.cleansession = 0;
    this->opts.username = AUTHMETHOD;
    this->opts.password = AUTHTOKEN;
};

void ADXL345Subscriber::setMaxLimits(float maxCPUTemp, float maxPitch, float maxRoll) {
    this->maxCPUTemp = maxCPUTemp;
    this->maxPitch = maxPitch;
    this->maxRoll = maxRoll;
};

void ADXL345Subscriber::processMessage(std::string payload) {
    this->parseJSONMessage(payload);
};

void ADXL345Subscriber::delivered(void *context, MQTTClient_deliveryToken dt) {
    cout << "Message with token value " << dt << " delivery confirmed" << endl;
    deliveredtoken = dt;
};

int ADXL345Subscriber::msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    int i;
    ADXL345Subscriber* sub = static_cast<ADXL345Subscriber*>(context);
    string payloadString(static_cast<char*>(message->payload), message->payloadlen);
    sub->processMessage(payloadString);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
};

void ADXL345Subscriber::connlost(void *context, char *cause) {
    cout << endl << "Connection lost" << endl;
    cout << "     cause: " << cause << endl;
};

void ADXL345Subscriber::setMQTTCallbacks(MQTTClient& client) {
    MQTTClient_setCallbacks(client, this, this->connlost, this->msgarrvd, this->delivered);
}

int ADXL345Subscriber::run(MQTTClient& client) {

    int rc, ch;
    this->setMQTTCallbacks(client);

    if ((rc = MQTTClient_connect(client, &this->opts)) != MQTTCLIENT_SUCCESS) {
        cout << "Failed to connect, return code " << rc << std::endl;
        exit(-1);
    }
    cout << "Subscribing to topic " << TOPIC << " for client " << CLIENTID << " using QoS " << QOS << endl << endl;
    cout << "Press Q<Enter> to quit" << endl << endl;
    MQTTClient_subscribe(client, TOPIC, QOS);
    do {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
};

double ADXL345Subscriber::parseJSONMessage(const std::string& jsonString) {

    cout << jsonString << endl;
    const char* jsonStringCStr = jsonString.c_str();
    json_object* root = json_tokener_parse(jsonStringCStr);

    SensorData sensorData;

    if (root != nullptr) {
        // Retrieve CPU temperature from the parsed JSON object
        json_object* cpuTempObj;
        if (json_object_object_get_ex(root, "CPU Temp", &cpuTempObj)) {
            sensorData.cpuTemp = json_object_get_double(cpuTempObj);
        } else {
            cerr << "CPU Temp not found in JSON" << endl;
        }
        // Retrieve accelerometer data from the parsed JSON object
        json_object* accelerometerObj;
        if (json_object_object_get_ex(root, "Accelerometer", &accelerometerObj)) {
            json_object* pitchObj = json_object_object_get(accelerometerObj, "Pitch");
            json_object* rollObj = json_object_object_get(accelerometerObj, "Roll");
            
            if (pitchObj != nullptr && rollObj != nullptr) {
                sensorData.pitch = json_object_get_double(pitchObj);
                sensorData.roll = json_object_get_double(rollObj);
            } else {
                cerr << "Pitch or Roll not found in Accelerometer data" << endl;
            }
        } else {
            cerr << "Accelerometer data not found in JSON" << endl;
        }
        json_object_put(root);
    } else {
        cerr << "Failed to parse JSON" << endl;
    }
};
}
