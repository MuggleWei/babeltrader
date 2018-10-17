#ifndef BABELTRADER_SERIALIZATION_H_
#define BABELTRADER_SERIALIZATION_H_

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "common/common_struct.h"

void SerializeQuoteBegin(rapidjson::Writer<rapidjson::StringBuffer> &writer, const Quote &quote);
void SerializeQuoteEnd(rapidjson::Writer<rapidjson::StringBuffer> &writer, const Quote &quote);

void SerializeMarketData(rapidjson::Writer<rapidjson::StringBuffer> &writer, const MarketData &md);
void SerializeKline(rapidjson::Writer<rapidjson::StringBuffer> &writer, const Kline &kline);

#endif