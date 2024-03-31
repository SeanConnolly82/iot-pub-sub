#ifndef ADXL345PUBLISHER_H
#define ADXL345PUBLISHER_H

#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream> 
#include <chrono>
#include <thread>
#include <csignal>
#include "MQTTClient.h"
#include "ADXL345.h"

#define CPU_TEMP "/sys/class/thermal/thermal_zone0/temp"

using namespace std;
using namespace ee513;

namespace publisher {

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
    virtual void run(ADXL345* ADXL345Device);
    virtual void setExitFlag(bool value);
    static void signalHandler(int signal, ADXL345Publisher* publisher);
}; /* namespace publisher */
}
#endif // ADXL345PUBLISHER_H