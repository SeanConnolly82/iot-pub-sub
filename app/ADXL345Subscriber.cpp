#include <iostream>
#include <cstring>
#include <string>
#include <cmath>
#include <json-c/json.h>
#include "MQTTClient.h"
#include "ADXL345Subscriber.h"
#include "wiringPi.h"

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

#define LED_CPU     0
#define LED_PITCH   2
#define LED_ROLL    3

volatile MQTTClient_deliveryToken deliveredtoken;

namespace subscriber {

/**
 * @brief Constructor for ADXL345Subscriber class.
 * Initializes MQTT client connection options.
 */
ADXL345Subscriber::ADXL345Subscriber() {
    this->opts = MQTTClient_connectOptions_initializer;
    this->opts.keepAliveInterval = 20;
    this->opts.cleansession = 0;
    this->opts.username = AUTHMETHOD;
    this->opts.password = AUTHTOKEN;
};

/**
 * @brief Set the maximum limits for CPU temperature celsius.
 * @param maxCPUTemp The maximum CPU temperature threshold.
 */
void ADXL345Subscriber::setCpuLimit(float maxCPUTemp) {
    this->cpuTempLimit = maxCPUTemp;
};

/**
 * @brief Set the maximum limits for pitch degrees.
 * @param maxPitch The maximum pitch threshold.
 */
void ADXL345Subscriber::setPitchLimit(float maxPitch) {
    this->pitchLimit = maxPitch;
};

/**
 * @brief Set the maximum limits for roll degrees.
 * @param maxPitch The maximum roll threshold.
 */
void ADXL345Subscriber::setRollLimit(float maxRoll) {
    this->rollLimit = maxRoll;
};

/**
 * @brief Process the incoming MQTT message payload.
 * @param payload The MQTT message payload.
 */
void ADXL345Subscriber::processMessage(std::string payload) {
    SensorData data = this->parseJSONMessage(payload);
    this->driveLeds(data);
};

/**
 * @brief If a non-default (!= 0.0 value) has been set, then drive a LED
 * if it exists the limit that has been set.
 * @param data A SensorData struct
 */
void ADXL345Subscriber::driveLeds(SensorData data) {
    if (this->cpuTempLimit != 0.0) {
        digitalWrite(LED_CPU, this->checkWithinLimit(data.cpuTemp, this->cpuTempLimit));
    }
    if (this->cpuTempLimit != 0.0) {
        digitalWrite(LED_PITCH, this->checkWithinLimit(abs(data.pitch), this->cpuTempLimit));
    }
    if (this->rollLimit != 0.0) {
        digitalWrite(LED_ROLL, this->checkWithinLimit(abs(data.roll), this->rollLimit));
    }
};

/**
 * @brief Initializes the GPIO pins for the LEDs.
 */
void ADXL345Subscriber::initialiseLeds() {
    wiringPiSetup();
    pinMode(LED_CPU, OUTPUT);
    pinMode(LED_PITCH, OUTPUT);
    pinMode(LED_ROLL, OUTPUT);
};

/**
 * @brief Checks if a value is within a specified limit.
 * @param value The value to check.
 * @param limit The limit to compare against.
 * @return 1 if the value is within the limit, 0 otherwise.
 */
int ADXL345Subscriber::checkWithinLimit(float value, float limit) {
    return (value >= limit) ? 1 : 0;
};

/**
 * @brief Callback function invoked when a message is delivered.
 * @param context The context associated with the callback.
 * @param dt The delivery token associated with the delivered message.
 */
void ADXL345Subscriber::delivered(void *context, MQTTClient_deliveryToken dt) {
    cout << "Message with token value " << dt << " delivery confirmed" << endl;
    deliveredtoken = dt;
};

/**
 * @brief Callback function invoked when a message is received.
 * @param context The context associated with the callback.
 * @param topicName The name of the topic associated with the message.
 * @param topicLen The length of the topic name.
 * @param message The MQTT message received.
 * @return The status code indicating the result of message processing.
 */
int ADXL345Subscriber::msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    int i;
    ADXL345Subscriber* sub = static_cast<ADXL345Subscriber*>(context);
    string payloadString(static_cast<char*>(message->payload), message->payloadlen);
    sub->processMessage(payloadString);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
};

/**
 * @brief Callback function invoked when the MQTT connection is lost.
 * @param context The context associated with the callback.
 * @param cause The cause of the connection loss.
 */
void ADXL345Subscriber::connlost(void *context, char *cause) {
    cout << endl << "Connection lost" << endl;
    cout << "     cause: " << cause << endl;
};

/**
 * @brief Set the MQTT client callbacks.
 * @param client The MQTT client object.
 */
void ADXL345Subscriber::setMQTTCallbacks(MQTTClient& client) {
    MQTTClient_setCallbacks(client, this, this->connlost, this->msgarrvd, this->delivered);
}

/**
 * @brief Run the MQTT subscriber.
 * @param client The MQTT client object.
 * @return The status code indicating the result of the subscriber run.
 */
int ADXL345Subscriber::run(MQTTClient& client) {

    int rc, ch;
    this->setMQTTCallbacks(client);
    this->initialiseLeds();

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

/**
 * @brief Parse the JSON-formatted message payload and extract sensor data.
 * @param jsonString The JSON-formatted message payload.
 * @return A SensorData structure containing parsed sensor data.
 */
ADXL345Subscriber::SensorData ADXL345Subscriber::parseJSONMessage(const std::string& jsonString) {

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
    return sensorData;
}
}
