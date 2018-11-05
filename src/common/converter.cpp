#include "converter.h"

namespace babeltrader
{


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

void SerializeOrder(rapidjson::Writer<rapidjson::StringBuffer> &writer, const Order &order)
{
	writer.Key("user_id");
	writer.String(order.user_id.c_str());
	writer.Key("order_id");
	writer.String(order.order_id.c_str());
	writer.Key("outside_id");
	writer.String(order.outside_id.c_str());
	writer.Key("client_order_id");
	writer.String(order.client_order_id.c_str());
	writer.Key("market");
	writer.String(order.market.c_str());
	writer.Key("exchange");
	writer.String(order.exchange.c_str());
	writer.Key("type");
	writer.String(order.type.c_str());
	writer.Key("symbol");
	writer.String(order.symbol.c_str());
	writer.Key("contract");
	writer.String(order.contract.c_str());
	writer.Key("contract_id");
	writer.String(order.contract_id.c_str());
	writer.Key("order_type");
	writer.String(order.order_type.c_str());
	writer.Key("order_flag1");
	writer.String(order.order_flag1.c_str());
	writer.Key("dir");
	writer.String(order.dir.c_str());
	writer.Key("price");
	writer.Double(order.price);
	writer.Key("amount");
	writer.Int(order.amount);
	writer.Key("total_price");
	writer.Double(order.total_price);
	writer.Key("ts");
	writer.Double(order.ts);
}
void SerializeOrderStatus(rapidjson::Writer<rapidjson::StringBuffer> &writer, const OrderStatusNotify &order_status)
{
	writer.Key("status");
	writer.Int(order_status.order_status);
	writer.Key("submit_status");
	writer.Int(order_status.order_submit_status);
	writer.Key("amount");
	writer.Int(order_status.amount);
	writer.Key("dealed_amount");
	writer.Int(order_status.dealed_amount);
}
void SerializeOrderDeal(rapidjson::Writer<rapidjson::StringBuffer> &writer, const OrderDealNotify &order_deal)
{
	writer.Key("price");
	writer.Double(order_deal.price);
	writer.Key("amount");
	writer.Int(order_deal.amount);
	writer.Key("trading_day");
	writer.String(order_deal.trading_day.c_str());
	writer.Key("trade_id");
	writer.String(order_deal.trade_id.c_str());
	writer.Key("ts");
	writer.Int64(order_deal.ts);
}
void SerializeOrderQuery(rapidjson::Writer<rapidjson::StringBuffer> &writer, const OrderQuery &order_query)
{
	writer.Key("qry_id");
	writer.String(order_query.qry_id.c_str());
	writer.Key("user_id");
	writer.String(order_query.user_id.c_str());
	writer.Key("outside_id");
	writer.String(order_query.outside_id.c_str());
	writer.Key("market");
	writer.String(order_query.market.c_str());
	writer.Key("exchange");
	writer.String(order_query.exchange.c_str());
	writer.Key("type");
	writer.String(order_query.type.c_str());
	writer.Key("symbol");
	writer.String(order_query.symbol.c_str());
	writer.Key("contract");
	writer.String(order_query.contract.c_str());
	writer.Key("contract_id");
	writer.String(order_query.contract_id.c_str());
}
void SerializeTradeQuery(rapidjson::Writer<rapidjson::StringBuffer> &writer, const TradeQuery &trade_query)
{
	writer.Key("qry_id");
	writer.String(trade_query.qry_id.c_str());
	writer.Key("user_id");
	writer.String(trade_query.user_id.c_str());
	writer.Key("trade_id");
	writer.String(trade_query.trade_id.c_str());
	writer.Key("market");
	writer.String(trade_query.market.c_str());
	writer.Key("exchange");
	writer.String(trade_query.exchange.c_str());
	writer.Key("type");
	writer.String(trade_query.type.c_str());
	writer.Key("symbol");
	writer.String(trade_query.symbol.c_str());
	writer.Key("contract");
	writer.String(trade_query.contract.c_str());
	writer.Key("contract_id");
	writer.String(trade_query.contract_id.c_str());
}
void SerializePositionQuery(rapidjson::Writer<rapidjson::StringBuffer> &writer, const PositionQuery &position_query)
{
	writer.Key("qry_id");
	writer.String(position_query.qry_id.c_str());
	writer.Key("user_id");
	writer.String(position_query.user_id.c_str());
	writer.Key("market");
	writer.String(position_query.market.c_str());
	writer.Key("exchange");
	writer.String(position_query.exchange.c_str());
	writer.Key("type");
	writer.String(position_query.type.c_str());
	writer.Key("symbol");
	writer.String(position_query.symbol.c_str());
	writer.Key("contract");
	writer.String(position_query.contract.c_str());
	writer.Key("contract_id");
	writer.String(position_query.contract_id.c_str());
}

void SerializePositionSummaryType1(rapidjson::Writer<rapidjson::StringBuffer> &writer, const PositionSummaryType1 &position_summary)
{
	writer.Key("market");
	writer.String(position_summary.market.c_str());
	writer.Key("exchange");
	writer.String(position_summary.exchange.c_str());
	writer.Key("type");
	writer.String(position_summary.type.c_str());
	writer.Key("symbol");
	writer.String(position_summary.symbol.c_str());
	writer.Key("contract");
	writer.String(position_summary.contract.c_str());
	writer.Key("contract_id");
	writer.String(position_summary.contract_id.c_str());
	writer.Key("dir");
	writer.String(position_summary.dir.c_str());
	writer.Key("order_flag1");
	writer.String(position_summary.order_flag1.c_str());
	writer.Key("date_type");
	writer.String(position_summary.date_type.c_str());
	writer.Key("amount");
	writer.Double(position_summary.amount);
	writer.Key("closed_amount");
	writer.Double(position_summary.closed_amount);
	writer.Key("today_amount");
	writer.Double(position_summary.today_amount);
	writer.Key("margin");
	writer.Double(position_summary.margin);
	writer.Key("margin_rate_by_money");
	writer.Double(position_summary.margin_rate_by_money);
	writer.Key("margin_rate_by_vol");
	writer.Double(position_summary.margin_rate_by_vol);
	writer.Key("long_frozen");
	writer.Double(position_summary.long_frozen);
	writer.Key("short_frozen");
	writer.Double(position_summary.short_frozen);
	writer.Key("frozen_margin");
	writer.Double(position_summary.frozen_margin);
	writer.Key("trading_day");
	writer.String(position_summary.trading_day.c_str());
	writer.Key("pre_settlement_price");
	writer.Double(position_summary.pre_settlement_price);
	writer.Key("settlement_price");
	writer.Double(position_summary.settlement_price);
	writer.Key("open_cost");
	writer.Double(position_summary.open_cost);
	writer.Key("position_cost");
	writer.Double(position_summary.position_cost);
	writer.Key("position_profit");
	writer.Double(position_summary.position_profit);
	writer.Key("close_profit_by_date");
	writer.Double(position_summary.close_profit_by_date);
	writer.Key("close_profit_by_trade");
	writer.Double(position_summary.close_profit_by_trade);
}
void SerializePositionDetailType1(rapidjson::Writer<rapidjson::StringBuffer> &writer, const PositionDetailType1 &position_detail)
{
	writer.Key("market");
	writer.String(position_detail.market.c_str());
	writer.Key("exchange");
	writer.String(position_detail.exchange.c_str());
	writer.Key("type");
	writer.String(position_detail.type.c_str());
	writer.Key("symbol");
	writer.String(position_detail.symbol.c_str());
	writer.Key("contract");
	writer.String(position_detail.contract.c_str());
	writer.Key("contract_id");
	writer.String(position_detail.contract_id.c_str());
	writer.Key("dir");
	writer.String(position_detail.dir.c_str());
	writer.Key("order_flag1");
	writer.String(position_detail.order_flag1.c_str());
	writer.Key("open_date");
	writer.String(position_detail.open_date.c_str());
	writer.Key("trading_day");
	writer.String(position_detail.trading_day.c_str());
	writer.Key("trade_id");
	writer.String(position_detail.trade_id.c_str());
	writer.Key("amount");
	writer.Double(position_detail.amount);
	writer.Key("closed_amount");
	writer.Double(position_detail.closed_amount);
	writer.Key("closed_money");
	writer.Double(position_detail.closed_money);
	writer.Key("pre_settlement_price");
	writer.Double(position_detail.pre_settlement_price);
	writer.Key("settlement_price");
	writer.Double(position_detail.settlement_price);
	writer.Key("open_price");
	writer.Double(position_detail.open_price);
	writer.Key("margin");
	writer.Double(position_detail.margin);
	writer.Key("margin_rate_by_money");
	writer.Double(position_detail.margin_rate_by_money);
	writer.Key("margin_rate_by_vol");
	writer.Double(position_detail.margin_rate_by_vol);
	writer.Key("close_profit_by_date");
	writer.Double(position_detail.close_profit_by_date);
	writer.Key("close_profit_by_trade");
	writer.Double(position_detail.close_profit_by_trade);
	writer.Key("position_profit_by_date");
	writer.Double(position_detail.position_profit_by_date);
	writer.Key("position_profit_by_trade");
	writer.Double(position_detail.position_profit_by_trade);
}

Order ConvertOrderJson2Common(rapidjson::Value &msg)
{
	Order order;

	if (msg.HasMember("user_id") && msg["user_id"].IsString()) {
		order.user_id = msg["user_id"].GetString();
	}

	if (msg.HasMember("order_id") && msg["order_id"].IsString()) {
		order.order_id = msg["order_id"].GetString();
	}

	if (msg.HasMember("outside_id") && msg["outside_id"].IsString()) {
		order.outside_id = msg["outside_id"].GetString();
	}

	if (msg.HasMember("client_order_id") && msg["client_order_id"].IsString()) {
		order.client_order_id = msg["client_order_id"].GetString();
	}

	if (msg.HasMember("market") && msg["market"].IsString()) {
		order.market = msg["market"].GetString();
	}

	if (msg.HasMember("exchange") && msg["exchange"].IsString()) {
		order.exchange = msg["exchange"].GetString();
	}

	if (msg.HasMember("type") && msg["type"].IsString()) {
		order.type = msg["type"].GetString();
	}

	if (msg.HasMember("symbol") && msg["symbol"].IsString()) {
		order.symbol = msg["symbol"].GetString();
	}

	if (msg.HasMember("contract") && msg["contract"].IsString()) {
		order.contract = msg["contract"].GetString();
	}

	if (msg.HasMember("contract_id") && msg["contract_id"].IsString()) {
		order.contract_id = msg["contract_id"].GetString();
	}

	if (msg.HasMember("order_type") && msg["order_type"].IsString()) {
		order.order_type = msg["order_type"].GetString();
	}

	if (msg.HasMember("order_flag1") && msg["order_flag1"].IsString()) {
		order.order_flag1 = msg["order_flag1"].GetString();
	}

	if (msg.HasMember("dir") && msg["dir"].IsString()) {
		order.dir = msg["dir"].GetString();
	}

	if (msg.HasMember("price")) {
		if (msg["price"].IsInt())
		{
			order.price = msg["price"].GetInt();
		}
		else if (msg["price"].IsDouble())
		{
			order.price = msg["price"].GetDouble();
		}
	}

	if (msg.HasMember("amount")) {
		if (msg["amount"].IsInt())
		{
			order.amount = msg["amount"].GetInt();
		}
		else if (msg["amount"].IsDouble())
		{
			order.amount = msg["amount"].GetDouble();
		}
	}

	if (msg.HasMember("total_price")) {
		if (msg["total_price"].IsInt())
		{
			order.total_price = msg["total_price"].GetInt();
		}
		else if (msg["total_price"].IsDouble())
		{
			order.total_price = msg["total_price"].GetDouble();
		}
	}

	if (msg.HasMember("ts") && msg["ts"].IsInt64()) {
		order.ts = msg["ts"].GetInt64();
	}

	return std::move(order);
}
OrderQuery ConvertOrderQueryJson2Common(rapidjson::Value &msg)
{
	OrderQuery order_qry;

	if (msg.HasMember("qry_id") && msg["qry_id"].IsString()) {
		order_qry.qry_id = msg["qry_id"].GetString();
	}

	if (msg.HasMember("user_id") && msg["user_id"].IsString()) {
		order_qry.user_id = msg["user_id"].GetString();
	}

	if (msg.HasMember("outside_id") && msg["outside_id"].IsString()) {
		order_qry.outside_id = msg["outside_id"].GetString();
	}

	if (msg.HasMember("market") && msg["market"].IsString()) {
		order_qry.market = msg["market"].GetString();
	}

	if (msg.HasMember("exchange") && msg["exchange"].IsString()) {
		order_qry.exchange = msg["exchange"].GetString();
	}

	if (msg.HasMember("type") && msg["type"].IsString()) {
		order_qry.type = msg["type"].GetString();
	}

	if (msg.HasMember("symbol") && msg["symbol"].IsString()) {
		order_qry.symbol = msg["symbol"].GetString();
	}

	if (msg.HasMember("contract") && msg["contract"].IsString()) {
		order_qry.contract = msg["contract"].GetString();
	}

	if (msg.HasMember("contract_id") && msg["contract_id"].IsString()) {
		order_qry.contract_id = msg["contract_id"].GetString();
	}

	return order_qry;
}
TradeQuery ConvertTradeQueryJson2Common(rapidjson::Value &msg)
{
	TradeQuery trade_qry;

	if (msg.HasMember("qry_id") && msg["qry_id"].IsString()) {
		trade_qry.qry_id = msg["qry_id"].GetString();
	}

	if (msg.HasMember("user_id") && msg["user_id"].IsString()) {
		trade_qry.user_id = msg["user_id"].GetString();
	}

	if (msg.HasMember("trade_id") && msg["trade_id"].IsString()) {
		trade_qry.trade_id = msg["trade_id"].GetString();
	}

	if (msg.HasMember("market") && msg["market"].IsString()) {
		trade_qry.market = msg["market"].GetString();
	}

	if (msg.HasMember("exchange") && msg["exchange"].IsString()) {
		trade_qry.exchange = msg["exchange"].GetString();
	}

	if (msg.HasMember("type") && msg["type"].IsString()) {
		trade_qry.type = msg["type"].GetString();
	}

	if (msg.HasMember("symbol") && msg["symbol"].IsString()) {
		trade_qry.symbol = msg["symbol"].GetString();
	}

	if (msg.HasMember("contract") && msg["contract"].IsString()) {
		trade_qry.contract = msg["contract"].GetString();
	}

	if (msg.HasMember("contract_id") && msg["contract_id"].IsString()) {
		trade_qry.contract_id = msg["contract_id"].GetString();
	}

	return trade_qry;
}
PositionQuery ConvertPositionQueryJson2Common(rapidjson::Value &msg)
{
	PositionQuery position_qry;

	if (msg.HasMember("qry_id") && msg["qry_id"].IsString()) {
		position_qry.qry_id = msg["qry_id"].GetString();
	}

	if (msg.HasMember("user_id") && msg["user_id"].IsString()) {
		position_qry.user_id = msg["user_id"].GetString();
	}

	if (msg.HasMember("market") && msg["market"].IsString()) {
		position_qry.market = msg["market"].GetString();
	}

	if (msg.HasMember("exchange") && msg["exchange"].IsString()) {
		position_qry.exchange = msg["exchange"].GetString();
	}

	if (msg.HasMember("type") && msg["type"].IsString()) {
		position_qry.type = msg["type"].GetString();
	}

	if (msg.HasMember("symbol") && msg["symbol"].IsString()) {
		position_qry.symbol = msg["symbol"].GetString();
	}

	if (msg.HasMember("contract") && msg["contract"].IsString()) {
		position_qry.contract = msg["contract"].GetString();
	}

	if (msg.HasMember("contract_id") && msg["contract_id"].IsString()) {
		position_qry.contract_id = msg["contract_id"].GetString();
	}

	return position_qry;
}


}