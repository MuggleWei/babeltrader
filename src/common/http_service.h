#ifndef BABELTRADER_HTTP_SERVICE_H_
#define BABELTRADER_HTTP_SERVICE_H_

#include "uWS/uWS.h"
#include "common/quote_service.h"
#include "common/trade_service.h"
#include "common/common_struct.h"

class HttpService
{
public:
	HttpService(QuoteService *quote_service, TradeService *trade_service);

	void onMessage(uWS::HttpResponse *res, uWS::HttpRequest &req, char *data, size_t length, size_t remainingBytes);

private:
	void GetSubtopics(uWS::HttpResponse *res);
	void SubTopic(uWS::HttpResponse *res, uWS::HttpRequest &req, char *data, size_t length, size_t remainingBytes);
	void UnsubTopic(uWS::HttpResponse *res, uWS::HttpRequest &req, char *data, size_t length, size_t remainingBytes);

	bool OnRestSubunsub(uWS::HttpResponse *res, uWS::HttpRequest &req, char *data, size_t length, size_t remainingBytes, Quote &msg);

	void RestReturn(uWS::HttpResponse *res, int err_id, const char *err_msg);
	bool ParseSubunsubMsg(const char *data, size_t length, Quote& msg, std::string &err_msg);

private:
	QuoteService *quote_;
	TradeService *trade_;
};

#endif