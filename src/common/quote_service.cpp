#include "quote_service.h"

#include "glog/logging.h"

#include "converter.h"
#include "ws_service.h"

namespace babeltrader
{

void QuoteService::BroadcastMarketData(uWS::Hub &hub, const Quote &quote, const MarketData &md)
{
	// serialize
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	SerializeQuoteBegin(writer, quote);
	SerializeMarketData(writer, md);
	SerializeQuoteEnd(writer, quote);

	hub.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void QuoteService::BroadcastKline(uWS::Hub &hub, const Quote &quote, const Kline &kline)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	SerializeQuoteBegin(writer, quote);
	SerializeKline(writer, kline);
	SerializeQuoteEnd(writer, quote);

	hub.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}

void QuoteService::BroadcastOrderBook(uWS::Hub &hub, const Quote &quote, const OrderBook &order_book)
{
	// serialize
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	SerializeQuoteBegin(writer, quote);
	SerializeOrderBook(writer, order_book);
	SerializeQuoteEnd(writer, quote);

	hub.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void QuoteService::BroadcastLevel2(uWS::Hub &hub, const Quote &quote, const OrderBookLevel2 &level2)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	SerializeQuoteBegin(writer, quote);
	SerializeLevel2(writer, level2);
	SerializeQuoteEnd(writer, quote);

	hub.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}


}