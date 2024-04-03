#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <chrono>
#include <thread>
#include <ctime>
#include "MQTTClient.h"
#include "ADXL345.h"
#include "ADXL345Publisher.h"

using namespace std;
using namespace device;
using namespace ee513;

#define CPU_TEMP "/sys/class/thermal/thermal_zone0/temp"
#define AUTHMETHOD "sean"
#define AUTHTOKEN  "password123"
#define TOPIC      "ee513/assignment2"
#define QOS        2
#define TIMEOUT    1000L
#define PUBLISH_INTERVAL 1 // in seconds
#define LWT_MESSAGE "The ADXL345 has disconnected unexpectedly"
#define LWT_RETAINED 0

namespace publisher {

/**
 * @brief Constructor for ADXL345Publisher class.
 * Initializes MQTT subscriber client connection options.
 */
ADXL345Publisher::ADXL345Publisher() {

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

/**
 * @brief Set the exit flag to indicate whether to stop publishing data.
 * @param value The value to set the exit flag.
 */
void ADXL345Publisher::setExitFlag(bool value) {
    cout << "The publisher will disconnect after sending the current message." << endl;
    this->exitFlag = value;
}

/**
 * @brief Calls setExitFlag based on user input. 
 */
void ADXL345Publisher::exitHandler() {
    std::cout << "Press Q<Enter> to quit" << std::endl;
    int ch;
    do{
        ch = getchar();
    } while(ch != 'Q' && ch != 'q');
    this->setExitFlag(true);
}

/**
 * @brief Start the exit handler thread.
 */
void ADXL345Publisher::startExitHandler() {
        std::thread exitHandlerThread(&ADXL345Publisher::exitHandler, this);
        exitHandlerThread.detach();
}

/**
 * @brief Get the CPU temperature from the system.
 * @return The CPU temperature in Celsius.
 */
float ADXL345Publisher::getCPUTemperature() {
    int cpuTemp;
    fstream fs;
    fs.open(CPU_TEMP, fstream::in);
    fs >> cpuTemp;
    fs.close();
    return (((float)cpuTemp)/1000);
}

/**
 * @brief Get the data from the ADXL345 accelerometer.
 * @param ADXL345Device Pointer to the ADXL345 device object.
 * @return A JSON-formatted string containing accelerometer data.
 */
string ADXL345Publisher::getDeviceData(ADXL345* ADXL345Device) {
    stringstream dataStream;
    ADXL345Device->readSensorState();
    dataStream << "{\"Pitch\" : " << ADXL345Device->getPitch() << ", \"Roll\": " << ADXL345Device->getRoll() << "}";
    return dataStream.str();
}

/**
 * @brief Get the current timestamp in string format.
 * @return The current timestamp as a string.
 */
string ADXL345Publisher::getCurrentTimestamp() {
    auto now = chrono::system_clock::now();
    time_t current_time = chrono::system_clock::to_time_t(now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&current_time));
    return string(buffer);
}

/**
 * @brief Main function to run the publisher.
 * Publishes data from the ADXL345 accelerometer via MQTT.
 * @param client The MQTT client object.
 * @param ADXL345Device Pointer to the ADXL345 device object.
 */
void ADXL345Publisher::run(MQTTClient& client, ADXL345* ADXL345Device) {
    int rc;
    this->startExitHandler();
    // Connect to the MQTT broker
    if ((rc = MQTTClient_connect(client, &this->opts)) != MQTTCLIENT_SUCCESS) {
        cout << "Failed to connect, return code " << rc << endl;
        return;
    }
    // Main loop for publishing MQTT messages
    while (!this->exitFlag) {
        string deviceData = this->getDeviceData(ADXL345Device);
        string timestamp = this->getCurrentTimestamp();
        char str_payload[1000];
        sprintf(str_payload,
            "{\"Publish timestamp\": \"%s\", \"Accelerometer\": %s,\"CPU Temp\": %f}", 
              timestamp.c_str(), 
              deviceData.c_str(), 
              this->getCPUTemperature());
        cout << str_payload << endl;
        pubmsg.payload = str_payload;
        pubmsg.payloadlen = strlen(str_payload);
        pubmsg.qos = QOS;
        pubmsg.retained = 0;

        MQTTClient_publishMessage(client, TOPIC, &this->pubmsg, &this->token);

        rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
        std::this_thread::sleep_for(std::chrono::seconds(PUBLISH_INTERVAL));
    }
    // Disconnect MQTT client after the loop
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
};
}

