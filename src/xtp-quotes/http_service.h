#ifndef HTTP_SERVICE_H_
#define HTTP_SERVICE_H_

#include "uWS/uWS.h"

class HttpService
{
public:
	void onMessage(uWS::HttpResponse *res, uWS::HttpRequest &req, char *data, size_t length, size_t remainingBytes);
};

#endif