#include "quote_service.h"

#include "glog/logging.h"

#include "converter.h"
#include "ws_service.h"
#include "utils_func.h"


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
#if ENABLE_PERFORMANCE_TEST
	// cache peak monitor
	static int64_t max_tunnel_cache = 0;

	// avg cache pkgs and avg elapsed time monitor
	static int64_t total_pkg = 0;
	static int64_t step = 1024;

	static int64_t rec_cnt = 0;

	static int64_t total_elapsed_time = 0;

	struct timespec ts;
#endif

	std::queue<QuoteBlock> queue;
	while (true) {
		tunnel_.Read(queue, true);

#if ENABLE_PERFORMANCE_TEST
		if (queue.size() > max_tunnel_cache)
		{
			max_tunnel_cache = queue.size();
			LOG(INFO) << "quote service async loop cache peak: " << max_tunnel_cache;
		}

		total_pkg += queue.size();
		rec_cnt++;
#endif

		if (queue.size() == 0) {
			continue;
		}

		rapidjson::StringBuffer s;
		rapidjson::Writer<rapidjson::StringBuffer> writer(s);

		writer.StartArray();

		while (queue.size()) {
			QuoteBlock &msg = queue.front();

#if ENABLE_PERFORMANCE_TEST
			QuoteBlockCommon *p = (QuoteBlockCommon*)&msg;
			timespec_get(&p->quote.ts[1], TIME_UTC);

			struct timespec *ts0 = &p->quote.ts[0];
			struct timespec *ts1 = &p->quote.ts[1];
			total_elapsed_time += (ts1->tv_sec - ts0->tv_sec) * 1000000000 + ts1->tv_nsec - ts0->tv_nsec;
#endif

			// Dispatch(msg);
			switch (msg.quote_type)
			{
				case QuoteBlockType_MarketData:
				{
					SerializeQuoteBegin(writer, ((const QuoteMarketData*)&msg)->quote);
					SerializeMarketData(writer, ((const QuoteMarketData*)&msg)->market_data);
					SerializeQuoteEnd(writer, ((const QuoteMarketData*)&msg)->quote);
				}break;
				case QuoteBlockType_Kline:
				{
					SerializeQuoteBegin(writer, ((const QuoteKline*)&msg)->quote);
					SerializeKline(writer, ((const QuoteKline*)&msg)->kline);
					SerializeQuoteEnd(writer, ((const QuoteKline*)&msg)->quote);
				}break;
				case QuoteBlockType_OrderBook:
				{
					SerializeQuoteBegin(writer, ((const QuoteOrderBook*)&msg)->quote);
					SerializeOrderBook(writer, ((const QuoteOrderBook*)&msg)->order_book);
					SerializeQuoteEnd(writer, ((const QuoteOrderBook*)&msg)->quote);
				}break;
				case QuoteBlockType_Level2:
				{
					SerializeQuoteBegin(writer, ((const QuoteOrderBookLevel2*)&msg)->quote);
					SerializeLevel2(writer, ((const QuoteOrderBookLevel2*)&msg)->level2);
					SerializeQuoteEnd(writer, ((const QuoteOrderBookLevel2*)&msg)->quote);
				}break;
			}

			queue.pop();
		}

		writer.EndArray();
		uws_hub_.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);

#if ENABLE_PERFORMANCE_TEST
		if (total_pkg >= step)
		{
			double avg_tunnel_cache = (double)total_pkg / rec_cnt;
			double avg_elapsed_time = (double)total_elapsed_time / total_pkg;
			LOG(INFO)
				<< "tunnel read: " << rec_cnt << " times"
				<< ", total pkg: " << total_pkg
				<< ", avg cache pkg: " << avg_tunnel_cache
				<< ", total elapsed: " << total_elapsed_time
				<< ", avg elapsed: " << avg_elapsed_time;
			total_pkg = 0;
			rec_cnt = 0;
			total_elapsed_time = 0;
		}
#endif

	}
}
// void QuoteService::Dispatch(QuoteBlock &msg)
// {
// #if ENABLE_PERFORMANCE_TEST
// 	static QuoteTransferMonitor monitor;
// 	monitor.start();
// #endif
// 
// 	switch (msg.quote_type)
// 	{
// 	case QuoteBlockType_MarketData:
// 	{
// 		SyncBroadcastMarketData((const QuoteMarketData*)&msg);
// 	}break;
// 	case QuoteBlockType_Kline:
// 	{
// 		SyncBroadcastKline((const QuoteKline*)&msg);
// 	}break;
// 	case QuoteBlockType_OrderBook:
// 	{
// 		SyncBroadcastOrderBook((const QuoteOrderBook*)&msg);
// 	}break;
// 	case QuoteBlockType_Level2:
// 	{
// 		SyncBroadcastLevel2((const QuoteOrderBookLevel2*)&msg);
// 	}break;
// 	}
// 
// #if ENABLE_PERFORMANCE_TEST
// 	monitor.end("quote service dispatch");
// #endif
// }

void QuoteService::SyncBroadcastMarketData(const QuoteMarketData *msg)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartArray();
	SerializeQuoteBegin(writer, msg->quote);
	SerializeMarketData(writer, msg->market_data);
	SerializeQuoteEnd(writer, msg->quote);
	writer.EndArray();

	uws_hub_.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void QuoteService::SyncBroadcastKline(const QuoteKline *msg)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartArray();
	SerializeQuoteBegin(writer, msg->quote);
	SerializeKline(writer, msg->kline);
	SerializeQuoteEnd(writer, msg->quote);
	writer.EndArray();

	uws_hub_.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void QuoteService::SyncBroadcastOrderBook(const QuoteOrderBook *msg)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartArray();
	SerializeQuoteBegin(writer, msg->quote);
	SerializeOrderBook(writer, msg->order_book);
	SerializeQuoteEnd(writer, msg->quote);
	writer.EndArray();

	uws_hub_.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void QuoteService::SyncBroadcastLevel2(const QuoteOrderBookLevel2 *msg)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartArray();
	SerializeQuoteBegin(writer, msg->quote);
	SerializeLevel2(writer, msg->level2);
	SerializeQuoteEnd(writer, msg->quote);
	writer.EndArray();

	uws_hub_.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}


}
