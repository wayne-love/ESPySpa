#ifndef MQTTCLIENTWRAPPER_H
#define MQTTCLIENTWRAPPER_H

#include <PubSubClient.h>
#include <WiFiClient.h>


class MQTTClientWrapper : public PubSubClient
{
    public:
        MQTTClientWrapper() : PubSubClient(_wifiClient)
        {}

        MQTTClientWrapper(WiFiClient &wifi) : _wifiClient(wifi), PubSubClient(wifi)
        {}

        PubSubClient& setServer(String serverAddress, int port)
        {
            _serverAddress = serverAddress;
            return PubSubClient::setServer(_serverAddress.c_str(), port);
        }

        bool connect(const String id) {
            _id = id;
            return PubSubClient::connect(_id.c_str());
        }

        bool connect(const String id, const String user, const String pass) {
            _id = id;
            _user = user;
            _pass = pass;
            return PubSubClient::connect(_id.c_str(), _user.c_str(), _pass.c_str());
        }

        bool connect(const String id, const String willTopic, uint8_t willQos, boolean willRetain, const String willMessage) {
            _id = id;
            _willTopic = willTopic;
            _willMessage = willMessage;
            return PubSubClient::connect(_id.c_str(), _willTopic.c_str(), willQos, willRetain, _willMessage.c_str());
        }

        bool connect(const String id, const String user, const String pass, const String willTopic, uint8_t willQos, boolean willRetain, const String willMessage) {
            _id = id;
            _user = user;
            _pass = pass;
            _willTopic = willTopic;
            _willMessage = willMessage;
            return PubSubClient::connect(_id.c_str(), _user.c_str(), _pass.c_str(), _willTopic.c_str(), willQos, willRetain, _willMessage.c_str());
        }

        bool connect(const char *id, const char *user, const char *pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession) {
            _id = id;
            _user = user;
            _pass = pass;
            _willTopic = willTopic;
            _willMessage = willMessage;
            return PubSubClient::connect(_id.c_str(), _user.c_str(), _pass.c_str(), _willTopic.c_str(), willQos, willRetain, _willMessage.c_str(), cleanSession);
         }

    private:
        String _serverAddress;
        String _id;
        String _user;
        String _pass;
        String _willTopic;
        String _willMessage;
        WiFiClient _wifiClient;

};

#endif // MQTTCLIENTWRAPPER_H
