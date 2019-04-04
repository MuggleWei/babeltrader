#ifndef BABELTRADER_QUOTE_SERVICE_H_
#define BABELTRADER_QUOTE_SERVICE_H_

#include <vector>

#include "uWS/uWS.h"
#include "muggle/cpp/tunnel/tunnel.hpp"
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

	void RunAsyncLoop();

	void BroadcastMarketData(QuoteMarketData &msg, bool async = true);
	void BroadcastKline(QuoteKline &msg, bool async = true);
	void BroadcastOrderBook(QuoteOrderBook &msg, bool async = true);
	void BroadcastLevel2(QuoteOrderBookLevel2 &msg, bool async = true);

private:
	void AsyncLoop();
	// void Dispatch(QuoteBlock &msg);

	void SyncBroadcastMarketData(const QuoteMarketData *msg);
	void SyncBroadcastKline(const QuoteKline *msg);
	void SyncBroadcastOrderBook(const QuoteOrderBook *msg);
	void SyncBroadcastLevel2(const QuoteOrderBookLevel2 *msg);

public:
	uWS::Hub uws_hub_;
	WsService *p_ws_service_;
	muggle::Tunnel<QuoteBlock> tunnel_;
};


}

#endif