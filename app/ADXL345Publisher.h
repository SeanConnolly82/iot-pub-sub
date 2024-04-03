#ifndef ADXL345PUBLISHER_H
#define ADXL345PUBLISHER_H

#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <chrono>
#include <thread>
#include <ctime>
#include "MQTTClient.h"
#include "ADXL345.h"

#define CPU_TEMP "/sys/class/thermal/thermal_zone0/temp"

using namespace std;
using namespace ee513;

namespace publisher {

/**
 * @class ADXL345Publisher
 * @brief Class for publishing data from an ADXL345 accelerometer via MQTT.
 */
class ADXL345Publisher {
private:
    MQTTClient_message pubmsg;
    MQTTClient_connectOptions opts;
    MQTTClient_willOptions willOptions;
    MQTTClient_deliveryToken token;
    bool exitFlag;
public:
    ADXL345Publisher();
    virtual float getCPUTemperature();
    virtual string getDeviceData(ADXL345* ADXL345Device);
    virtual string getCurrentTimestamp();
    virtual void run(MQTTClient& client, ADXL345* ADXL345Device);
    virtual void setExitFlag(bool value);
    virtual void exitHandler();
    virtual void startExitHandler();
}; /* namespace publisher */
}
#endif // ADXL345PUBLISHER_H