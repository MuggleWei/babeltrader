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

	void BroadcastMarketData(uWS::Hub &hub, const QuoteMarketData &msg);
	void BroadcastKline(uWS::Hub &hub, const QuoteKline &msg);
	void BroadcastOrderBook(uWS::Hub &hub, const QuoteOrderBook &msg);
	void BroadcastLevel2(uWS::Hub &hub, const QuoteOrderBookLevel2 &msg);

public:
	WsService *ws_service_;
};


}

#endif