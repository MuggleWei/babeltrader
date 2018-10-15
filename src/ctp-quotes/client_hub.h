#ifndef CLIENT_HUB_H_
#define CLIENT_HUB_H_

#include "uWS/uWS.h"

class ClientHub
{
public:
	virtual onWsConnect(uWS::WebSocket<uWS::SERVER> *ws);
	virtual onWsDisconnect(uWS::WebSocket<uWS::SERVER> *ws);

private:
	
};

#endif