#include <iostream>
#include <cstring>
#include <string>
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

void Subscriber::delivered(void *context, MQTTClient_deliveryToken dt) {
    cout << "Message with token value " << dt << " delivery confirmed" << endl;
    deliveredtoken = dt;
};

int Subscriber::msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    //int i;
    //string jsonString;
    //char* payloadptr;
    //cout << "Message arrived" << endl;
    //cout << "     topic: " << topicName << endl;
    //cout << "   message: ";
    string payloadString(static_cast<char*>(message->payload), message->payloadlen);
    //cout << "Message payload: " << payloadString << endl;
    // payloadptr = (char*) message->payload;
    // for(i=0; i<message->payloadlen; i++) {
    //     cout << *payloadptr++;
    // }
    // cout << endl;
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
};

void Subscriber::connlost(void *context, char *cause) {
    cout << endl << "Connection lost" << endl;
    cout << "     cause: " << cause << endl;
};

double Subscriber::parseCpuTemp(const std::string& jsonString) {
    return 0.0;
    // JSONNode node = libjson::parse(jsonString);
    // // Check if the top-level node is an object
    // if (node.type() != JSON_NODE) {
    //     std::cerr << "Invalid JSON string: not an object" << std::endl;
    //     return 0.0;
    // }
    // // Find the "CPU Temp" key
    // JSONNode::const_iterator iter = node.find("CPU Temp");
    // if (iter != node.end()) {
    //     // Check if the value is a number
    //     if (iter->type() == JSON_NUMBER) {
    //         return iter->as_float();
    //     } else {
    //         std::cerr << "Invalid JSON string: CPU Temp value is not a number" << std::endl;
    //         return 0.0;
    //     }
    // } else {
    //     std::cerr << "Invalid JSON string: missing CPU Temp key" << std::endl;
    //     return 0.0;
    // }
};

int Subscriber::run(MQTTClient& client) {

    int rc, ch;
    MQTTClient_setCallbacks(client, NULL, this->connlost, this->msgarrvd, this->delivered);

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
}

int main(int argc, char* argv[]) {
    
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    Subscriber subscriber;
    int rc = subscriber.run(client);

    return rc;
};
