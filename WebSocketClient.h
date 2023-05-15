#ifndef WebSocketClient_H
#define WebSocketClient_H

#include <WiFiClient.h>
#include <WiFiClientSecure.h>


class WebSocketClient {
public:

	//WebSocketClient(bool secure = false);
  WebSocketClient(WiFiClient &client);
	~WebSocketClient();

	bool connect(String host, String path, int port);

	bool isConnected();

	void disconnect();

	void send(const String& str);

	bool getMessage(String& message);

	void setAuthorizationHeader(String header);

	void setSecureFingerprint(const char * fpStr);

private:
	int timedRead();

    void write(uint8_t data);

    void write(const char *str);

	String generateKey();

	WiFiClient *client;

	bool secure;

	String authorizationHeader = "";

    bool websocketEstablished = false;

};

#endif //WebSocketClient_H
