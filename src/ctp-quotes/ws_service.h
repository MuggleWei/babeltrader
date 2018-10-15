#ifndef WS_SERVICE_H_
#define WS_SERVICE_H_

#include "uWS/uWS.h"

class WsService
{
public:
	void onConnection(uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest &req);
	void onDisconnection(uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length);
	void onMessage(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode);
};

#endif