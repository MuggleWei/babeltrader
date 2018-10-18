#include "serialization.h"

void SerializeQuoteBegin(rapidjson::Writer<rapidjson::StringBuffer> &writer, const Quote &quote)
{
	// quote object
	writer.StartObject();
	writer.Key("msg");
	writer.String("quote");

	// quote data
	writer.Key("data");
	writer.StartObject();
	writer.Key("market");
	writer.String(quote.market.c_str());
	writer.Key("exchange_id");
	writer.String(quote.exchange.c_str());
	writer.Key("type");
	writer.String(quote.type.c_str());
	writer.Key("symbol");
	writer.String(quote.symbol.c_str());
	writer.Key("contract");
	writer.String(quote.contract.c_str());
	writer.Key("contract_id");
	writer.String(quote.contract_id.c_str());
	writer.Key("info1");
	writer.String(quote.info1.c_str());
	writer.Key("info2");
	writer.String(quote.info2.c_str());
	
	// inner data
	writer.String("data");
	writer.StartObject();
}
void SerializeQuoteEnd(rapidjson::Writer<rapidjson::StringBuffer> &writer, const Quote &quote)
{
	writer.EndObject();	// inner data
	writer.EndObject(); // quote data
	writer.EndObject(); // quote object
}

void SerializeMarketData(rapidjson::Writer<rapidjson::StringBuffer> &writer, const MarketData &md)
{
	writer.Key("ts");
	writer.Int64(md.ts);
	writer.Key("last");
	writer.Double(md.last);
	writer.Key("bids");
	writer.StartArray();
	for (auto price_vol : md.bids) {
		writer.StartArray();
		writer.Double(price_vol.price);
		writer.Int(price_vol.vol);
		writer.EndArray();
	}
	writer.EndArray();
	writer.Key("asks");
	writer.StartArray();
	for (auto price_vol : md.asks) {
		writer.StartArray();
		writer.Double(price_vol.price);
		writer.Int(price_vol.vol);
		writer.EndArray();
	}
	writer.EndArray();
	writer.Key("vol");
	writer.Double(md.vol);
	writer.Key("turnover");
	writer.Double(md.turnover);
	writer.Key("avg_price");
	writer.Double(md.avg_price);
	writer.Key("pre_settlement");
	writer.Double(md.pre_settlement);
	writer.Key("pre_close");
	writer.Double(md.pre_close);
	writer.Key("pre_open_interest");
	writer.Double(md.pre_open_interest);
	writer.Key("settlement");
	writer.Double(md.settlement);
	writer.Key("close");
	writer.Double(md.close);
	writer.Key("open_interest");
	writer.Double(md.open_interest);
	writer.Key("upper_limit");
	writer.Double(md.upper_limit);
	writer.Key("lower_limit");
	writer.Double(md.lower_limit);
	writer.Key("open");
	writer.Double(md.open);
	writer.Key("high");
	writer.Double(md.high);
	writer.Key("low");
	writer.Double(md.low);
	writer.Key("trading_day");
	writer.String(md.trading_day.c_str());
	writer.Key("action_day");
	writer.String(md.action_day.c_str());
}

void SerializeKline(rapidjson::Writer<rapidjson::StringBuffer> &writer, const Kline &kline)
{
	writer.Key("ts");
	writer.Int64(kline.ts);
	writer.Key("open");
	writer.Double(kline.open);
	writer.Key("high");
	writer.Double(kline.high);
	writer.Key("low");
	writer.Double(kline.low);
	writer.Key("close");
	writer.Double(kline.close);
	writer.Key("vol");
	writer.Double(kline.vol);
}