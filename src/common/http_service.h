#ifndef BABELTRADER_HTTP_SERVICE_H_
#define BABELTRADER_HTTP_SERVICE_H_

#include "uWS/uWS.h"
#include "common/quote_service.h"
#include "common/trade_service.h"

class HttpService
{
public:
	HttpService(QuoteService *quote_service, TradeService *trade_service);

	void onMessage(uWS::HttpResponse *res, uWS::HttpRequest &req, char *data, size_t length, size_t remainingBytes);

private:
	void GetSubtopics(uWS::HttpResponse *res);

private:
	QuoteService *quote_;
	TradeService *trade_;
};

#endif