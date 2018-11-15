#ifndef BABELTRADER_QUOTE_SERVICE_H_
#define BABELTRADER_QUOTE_SERVICE_H_

#include <vector>

#include "uWS/uWS.h"
#include "common/common_struct.h"

namespace babeltrader
{



class WsService;

class QuoteService
{
public:
	virtual std::vector<Quote> GetSubTopics(std::vector<bool> &vec_b) = 0;
	virtual void SubTopic(const Quote &msg) = 0;
	virtual void UnsubTopic(const Quote &msg) = 0;

	void BroadcastMarketData(uWS::Hub &hub, const Quote &quote, const MarketData &md);
	void BroadcastKline(uWS::Hub &hub, const Quote &quote, const Kline &kline);
	void BroadcastOrderBook(uWS::Hub &hub, const Quote &quote, const OrderBook &order_book);
	void BroadcastLevel2(uWS::Hub &hub, const Quote &quote, const OrderBookLevel2 &level2);

public:
	WsService *ws_service_;
};


}

#endif