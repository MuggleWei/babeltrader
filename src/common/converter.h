#ifndef BABELTRADER_SERIALIZATION_H_
#define BABELTRADER_SERIALIZATION_H_

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "common/common_struct.h"

namespace babeltrader
{


void SerializeQuoteBegin(rapidjson::Writer<rapidjson::StringBuffer> &writer, const Quote &quote);
void SerializeQuoteEnd(rapidjson::Writer<rapidjson::StringBuffer> &writer, const Quote &quote);

void SerializeMarketData(rapidjson::Writer<rapidjson::StringBuffer> &writer, const MarketData &md);
void SerializeOrderBook(rapidjson::Writer<rapidjson::StringBuffer> &writer, const OrderBook &order_book);
void SerializeKline(rapidjson::Writer<rapidjson::StringBuffer> &writer, const Kline &kline);
void SerializeLevel2(rapidjson::Writer<rapidjson::StringBuffer> &writer, const OrderBookLevel2 &level2);

void SerializeOrder(rapidjson::Writer<rapidjson::StringBuffer> &writer, const Order &order);
void SerializeOrderStatus(rapidjson::Writer<rapidjson::StringBuffer> &writer, const OrderStatusNotify &order_status);
void SerializeOrderDeal(rapidjson::Writer<rapidjson::StringBuffer> &writer, const OrderDealNotify &order_deal);
void SerializeOrderQuery(rapidjson::Writer<rapidjson::StringBuffer> &writer, const OrderQuery &order_query);
void SerializeTradeQuery(rapidjson::Writer<rapidjson::StringBuffer> &writer, const TradeQuery &trade_query);
void SerializePositionQuery(rapidjson::Writer<rapidjson::StringBuffer> &writer, const PositionQuery &position_query);
void SerializeTradeAccountQuery(rapidjson::Writer<rapidjson::StringBuffer> &writer, const TradeAccountQuery &tradeaccount_query);
void SerializeProductQuery(rapidjson::Writer<rapidjson::StringBuffer> &writer, const ProductQuery &product_query);

void SerializePositionSummaryType1(rapidjson::Writer<rapidjson::StringBuffer> &writer, const PositionSummaryType1 &position_summary);
void SerializePositionDetailType1(rapidjson::Writer<rapidjson::StringBuffer> &writer, const PositionDetailType1 &position_detail);
void SerializeTradeAccountType1(rapidjson::Writer<rapidjson::StringBuffer> &writer, const TradeAccountType1 &trade_account);
void SerializeProductType1(rapidjson::Writer<rapidjson::StringBuffer> &writer, const ProductType1 &product_type);

void SerializePositionSummaryType2(rapidjson::Writer<rapidjson::StringBuffer> &writer, const PositionSummaryType2 &position_summary);
void SerializeTradeAccountType2(rapidjson::Writer<rapidjson::StringBuffer> &writer, const TradeAccountType2 &trade_account);

Order ConvertOrderJson2Common(rapidjson::Value &msg);
OrderQuery ConvertOrderQueryJson2Common(rapidjson::Value &msg);
TradeQuery ConvertTradeQueryJson2Common(rapidjson::Value &msg);
PositionQuery ConvertPositionQueryJson2Common(rapidjson::Value &msg);
ProductQuery ConvertProductQueryJson2Common(rapidjson::Value &msg);
TradeAccountQuery ConvertTradeAccountJson2Common(rapidjson::Value &msg);
TradingDayQuery ConvertTradingDayJson2Common(rapidjson::Value &msg);


bool SplitOrderDir(const char *order_dir, int len, const char **action, const char **dir);

}

#endif