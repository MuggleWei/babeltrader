#include "quote_service.h"

#include "glog/logging.h"

#include "converter.h"
#include "ws_service.h"

namespace babeltrader
{

void QuoteService::BroadcastMarketData(uWS::Hub &hub, const QuoteMarketData &msg)
{
	// serialize
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	SerializeQuoteBegin(writer, msg.quote);
	SerializeMarketData(writer, msg.market_data);
	SerializeQuoteEnd(writer, msg.quote);

	hub.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void QuoteService::BroadcastKline(uWS::Hub &hub, const QuoteKline &msg)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	SerializeQuoteBegin(writer, msg.quote);
	SerializeKline(writer, msg.kline);
	SerializeQuoteEnd(writer, msg.quote);

	hub.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}

void QuoteService::BroadcastOrderBook(uWS::Hub &hub, const QuoteOrderBook &msg)
{
	// serialize
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	SerializeQuoteBegin(writer, msg.quote);
	SerializeOrderBook(writer, msg.order_book);
	SerializeQuoteEnd(writer, msg.quote);

	hub.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void QuoteService::BroadcastLevel2(uWS::Hub &hub, const QuoteOrderBookLevel2 &msg)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	SerializeQuoteBegin(writer, msg.quote);
	SerializeLevel2(writer, msg.level2);
	SerializeQuoteEnd(writer, msg.quote);

	hub.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}


}