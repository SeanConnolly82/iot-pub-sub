#include <iostream>
#include <cstring>
#include <string>
#include <cmath>
#include <json-c/json.h>
#include "MQTTClient.h"
#include "Subscriber.h"

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
Subscriber::Subscriber() {
    this->opts = MQTTClient_connectOptions_initializer;
    this->opts.keepAliveInterval = 20;
    this->opts.cleansession = 0;
    this->opts.username = AUTHMETHOD;
    this->opts.password = AUTHTOKEN;
};

void Subscriber::processMessage() {
    // parse the JSON generically
    // set some thresholds and trigger interrupts
    this->parseCpuTemp(this->payload);
};

void Subscriber::delivered(void *context, MQTTClient_deliveryToken dt) {
    cout << "Message with token value " << dt << " delivery confirmed" << endl;
    deliveredtoken = dt;
};

int Subscriber::msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    int i;
    Subscriber* sub = static_cast<Subscriber*>(context);
    string payloadString(static_cast<char*>(message->payload), message->payloadlen);
    // use set CPU temp
    sub->payload = payloadString;
    sub->processMessage();
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
};

void Subscriber::connlost(void *context, char *cause) {
    cout << endl << "Connection lost" << endl;
    cout << "     cause: " << cause << endl;
};

void Subscriber::setMQTTCallbacks(MQTTClient& client) {
    MQTTClient_setCallbacks(client, this, this->connlost, this->msgarrvd, this->delivered);
}

int Subscriber::run(MQTTClient& client) {

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

double Subscriber::parseCpuTemp(const std::string& jsonString) {

    // make this generic
    // Parse the JSON string
    cout << jsonString << endl;
    const char* jsonStringCStr = jsonString.c_str();
    json_object* root = json_tokener_parse(jsonStringCStr);

    double cpu_temp = 0.0; // Default value in case parsing fails

    if (root != nullptr) {
        // Retrieve CPU temperature from the parsed JSON object
        json_object* cpu_temp_obj;
        if (json_object_object_get_ex(root, "CPU Temp", &cpu_temp_obj)) {
            cpu_temp = json_object_get_double(cpu_temp_obj);
            std::cout << "CPU Temp: " << cpu_temp << std::endl;
        } else {
            std::cerr << "CPU Temp not found in JSON" << std::endl;
        }
        // Free the parsed JSON object
        json_object_put(root);
    } else {
        std::cerr << "Failed to parse JSON" << std::endl;
    }
    return cpu_temp; // Return the parsed CPU temperature
};

}

int main(int argc, char* argv[]) {
    
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    Subscriber sub;
    int rc = sub.run(client);
    return rc;
    // In the application have MAX_TEMP, pitch and roll, to set an alarm threshold
};
