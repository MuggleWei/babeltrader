#include "quote_service.h"

#include "glog/logging.h"

#include "converter.h"
#include "ws_service.h"


namespace babeltrader
{

void QuoteService::RunAsyncLoop()
{
	std::thread th(&QuoteService::AsyncLoop, this);
	th.detach();
}

void QuoteService::BroadcastMarketData(QuoteMarketData &msg, bool async)
{
	if (async)
	{
		static_assert(sizeof(QuoteBlock) >= sizeof(QuoteMarketData), "QuoteBlock size is not enough");

		msg.quote_type = QuoteBlockType_MarketData;
		QuoteBlock *p_block = (QuoteBlock*)&msg;
		tunnel_.Write(*p_block);
	}
	else
	{
		SyncBroadcastMarketData(&msg);
	}
	
}
void QuoteService::BroadcastKline(QuoteKline &msg, bool async)
{
	if (async)
	{
		static_assert(sizeof(QuoteBlock) >= sizeof(QuoteKline), "QuoteBlock size is not enough");

		msg.quote_type = QuoteBlockType_Kline;
		QuoteBlock *p_block = (QuoteBlock*)&msg;
		tunnel_.Write(*p_block);
	}
	else
	{
		SyncBroadcastKline(&msg);
	}
}
void QuoteService::BroadcastOrderBook(QuoteOrderBook &msg, bool async)
{
	if (async)
	{
		static_assert(sizeof(QuoteBlock) >= sizeof(QuoteOrderBook), "QuoteBlock size is not enough");

		msg.quote_type = QuoteBlockType_OrderBook;
		QuoteBlock *p_block = (QuoteBlock*)&msg;
		tunnel_.Write(*p_block);
	}
	else
	{
		SyncBroadcastOrderBook(&msg);
	}
}
void QuoteService::BroadcastLevel2(QuoteOrderBookLevel2 &msg, bool async)
{
	if (async)
	{
		static_assert(sizeof(QuoteBlock) >= sizeof(QuoteOrderBookLevel2), "QuoteBlock size is not enough");

		msg.quote_type = QuoteBlockType_Level2;
		QuoteBlock *p_block = (QuoteBlock*)&msg;
		tunnel_.Write(*p_block);
	}
	else
	{
		SyncBroadcastLevel2(&msg);
	}
}

void QuoteService::AsyncLoop()
{
	std::queue<QuoteBlock> queue;
	while (true) {
		tunnel_.Read(queue, true);
		while (queue.size()) {
			QuoteBlock &msg = queue.front();
			Dispatch(msg);
			queue.pop();
		}
	}
}
void QuoteService::Dispatch(QuoteBlock &msg)
{
	switch (msg.quote_type)
	{
	case QuoteBlockType_MarketData:
	{
		SyncBroadcastMarketData((const QuoteMarketData*)msg.buf);
	}break;
	case QuoteBlockType_Kline:
	{
		SyncBroadcastKline((const QuoteKline*)msg.buf);
	}break;
	case QuoteBlockType_OrderBook:
	{
		SyncBroadcastOrderBook((const QuoteOrderBook*)msg.buf);
	}break;
	case QuoteBlockType_Level2:
	{
		SyncBroadcastLevel2((const QuoteOrderBookLevel2*)msg.buf);
	}break;
	}
}

void QuoteService::SyncBroadcastMarketData(const QuoteMarketData *msg)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	SerializeQuoteBegin(writer, msg->quote);
	SerializeMarketData(writer, msg->market_data);
	SerializeQuoteEnd(writer, msg->quote);

	uws_hub_.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void QuoteService::SyncBroadcastKline(const QuoteKline *msg)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	SerializeQuoteBegin(writer, msg->quote);
	SerializeKline(writer, msg->kline);
	SerializeQuoteEnd(writer, msg->quote);

	uws_hub_.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void QuoteService::SyncBroadcastOrderBook(const QuoteOrderBook *msg)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	SerializeQuoteBegin(writer, msg->quote);
	SerializeOrderBook(writer, msg->order_book);
	SerializeQuoteEnd(writer, msg->quote);

	uws_hub_.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void QuoteService::SyncBroadcastLevel2(const QuoteOrderBookLevel2 *msg)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	SerializeQuoteBegin(writer, msg->quote);
	SerializeLevel2(writer, msg->level2);
	SerializeQuoteEnd(writer, msg->quote);

	uws_hub_.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}


}