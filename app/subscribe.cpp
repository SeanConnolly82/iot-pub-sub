#include <iostream>
#include <cstring>
#include "MQTTClient.h"

#define ADDRESS     "tcp://192.168.0.243:1883"
#define CLIENTID    "rpi2"
#define AUTHMETHOD  "sean"
#define AUTHTOKEN   "password123"
#define TOPIC       "ee513/assignment2"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
    std::cout << "Message with token value " << dt << " delivery confirmed" << std::endl;
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    int i;
    char* payloadptr;
    std::cout << "Message arrived" << std::endl;
    std::cout << "     topic: " << topicName << std::endl;
    std::cout << "   message: ";
    payloadptr = (char*) message->payload;
    for(i=0; i<message->payloadlen; i++) {
        std::cout << *payloadptr++;
    }
    std::cout << std::endl;
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause) {
    std::cout << std::endl << "Connection lost" << std::endl;
    std::cout << "     cause: " << cause << std::endl;
}

int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        std::cout << "Failed to connect, return code " << rc << std::endl;
        exit(-1);
    }
    std::cout << "Subscribing to topic " << TOPIC << " for client " << CLIENTID << " using QoS " << QOS << std::endl << std::endl;
    std::cout << "Press Q<Enter> to quit" << std::endl << std::endl;
    MQTTClient_subscribe(client, TOPIC, QOS);

    do {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
