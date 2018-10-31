#ifndef BABELTRADER_SERIALIZATION_H_
#define BABELTRADER_SERIALIZATION_H_

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "common/common_struct.h"

namespace babeltrader
{


void SerializeQuoteBegin(rapidjson::Writer<rapidjson::StringBuffer> &writer, const Quote &quote);
void SerializeQuoteEnd(rapidjson::Writer<rapidjson::StringBuffer> &writer, const Quote &quote);

void SerializeMarketData(rapidjson::Writer<rapidjson::StringBuffer> &writer, const MarketData &md);
void SerializeKline(rapidjson::Writer<rapidjson::StringBuffer> &writer, const Kline &kline);

void SerializeOrder(rapidjson::Writer<rapidjson::StringBuffer> &writer, const Order &order);
void SerializeOrderStatus(rapidjson::Writer<rapidjson::StringBuffer> &writer, const OrderStatusNotify &order_status);
void SerializeOrderDeal(rapidjson::Writer<rapidjson::StringBuffer> &writer, const OrderDealNotify &order_deal);
void SerializeOrderQuery(rapidjson::Writer<rapidjson::StringBuffer> &writer, const OrderQuery &order_query);


}

#endif