#include "http_service.h"

#include <assert.h>

#include "glog/logging.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include "err.h"

namespace babeltrader
{


HttpService::HttpService(QuoteService *quote_service, TradeService *trade_service)
	: quote_(quote_service)
	, trade_(trade_service)
{}

void HttpService::onMessage(uWS::HttpResponse *res, uWS::HttpRequest &req, char *data, size_t length, size_t remainingBytes)
{
	auto url = req.getUrl().toString();
	if (url == "/topic/get") 
	{
		GetSubtopics(res);
	}
	else if (url == "/topic/sub" && req.getMethod() == uWS::HttpMethod::METHOD_POST) 
	{
		SubTopic(res, req, data, length, remainingBytes);
	}
	else if (url == "/topic/unsub" && req.getMethod() == uWS::HttpMethod::METHOD_POST)
	{
		UnsubTopic(res, req, data, length, remainingBytes);
	}
	else
	{
		res->getHttpSocket()->terminate();
	}
}

void HttpService::GetSubtopics(uWS::HttpResponse *res)
{
	std::vector<bool> vec_b;
	auto topics = quote_->GetSubTopics(vec_b);

	assert(topics.size() == vec_b.size());

	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("topics");
	
	writer.Key("data");
	writer.StartArray();
	for (auto idx = 0; idx < topics.size(); ++idx) {
		const Quote &msg = topics[idx];

		writer.StartObject();
		writer.Key("market");
		writer.String(g_markets[msg.market]);
		writer.Key("exchange");
		writer.String(g_exchanges[msg.exchange]);
		writer.Key("type");
		writer.String(g_product_types[msg.type]);
		writer.Key("symbol");
		writer.String(msg.symbol);
		writer.Key("contract");
		writer.String(msg.contract);
		writer.Key("contract_id");
		writer.String(msg.contract_id);
		writer.Key("info1");
		writer.String(g_quote_info1[msg.info1]);
		writer.Key("info2");
		writer.String(g_quote_info1[msg.info2]);
		writer.Key("subed");
		writer.Int(vec_b[idx] ? 1 : 0);
		writer.EndObject();
	}
	writer.EndArray();

	writer.EndObject();

	res->end(s.GetString(), s.GetLength());
}
void HttpService::SubTopic(uWS::HttpResponse *res, uWS::HttpRequest &req, char *data, size_t length, size_t remainingBytes)
{
	Quote msg;
	auto ret = OnRestSubunsub(res, req, data, length, remainingBytes, msg);
	if (ret) {
		quote_->SubTopic(msg);
	}
}
void HttpService::UnsubTopic(uWS::HttpResponse *res, uWS::HttpRequest &req, char *data, size_t length, size_t remainingBytes)
{
	Quote msg;
	auto ret = OnRestSubunsub(res, req, data, length, remainingBytes, msg);
	if (ret) {
		quote_->UnsubTopic(msg);
	}
}

bool HttpService::OnRestSubunsub(uWS::HttpResponse *res, uWS::HttpRequest &req, char *data, size_t length, size_t remainingBytes, Quote &msg)
{
	if (remainingBytes) {
		return false;
	}

	if (length > 4096) {
		const char *err_msg = "request length too long!";
		RestReturn(res, BABELTRADER_ERR_HTTPREQ_TOO_LONG, BABELTRADER_ERR_MSG[BABELTRADER_ERR_HTTPREQ_TOO_LONG - BABELTRADER_ERR_BEGIN]);
		return false;
	}

	std::string err_msg;
	auto ret = ParseSubunsubMsg(data, length, msg, err_msg);
	if (!ret) {
		RestReturn(res, BABELTRADER_ERR_HTTPREQ_FAILED_PARSE, err_msg.c_str());
		return false;
	}

	RestReturn(res, BABELTRADER_OK, "");
	return true;
}

void HttpService::RestReturn(uWS::HttpResponse *res, int err_id, const char *err_msg)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("error_id");
	writer.Int(err_id);
	writer.Key("error_msg");
	writer.String(err_msg);
	writer.EndObject();

	res->end(s.GetString(), s.GetLength());
}
bool HttpService::ParseSubunsubMsg(const char *data, size_t length, Quote& msg, std::string &err_msg)
{
	char buf[1024];

	rapidjson::Document d;
	if (d.Parse(data, length).HasParseError()) {
		snprintf(buf, sizeof(buf) - 1, "%s: Error(offset %u): %s",
			BABELTRADER_ERR_MSG[BABELTRADER_ERR_HTTPREQ_FAILED_PARSE - BABELTRADER_ERR_BEGIN],
			(unsigned)d.GetErrorOffset(),
			rapidjson::GetParseError_En(d.GetParseError()));
		err_msg = buf;
		return false;
	}

	if (!d.HasMember("market") || !d["market"].IsString())
	{
		msg.market = Market_Unknown;
	}
	else
	{
		msg.market = getMarketEnum(d["market"].GetString());
	}

	if (!d.HasMember("exchange") || !d["exchange"].IsString()) 
	{
		msg.exchange = Exchange_Unknown;
	}
	else
	{
		msg.exchange = getExchangeEnum(d["exchange"].GetString());
	}

	if (!d.HasMember("type") || !d["type"].IsString())
	{
		msg.type = ProductType_Unknown;
	}
	else
	{
		msg.type = getProductTypeEnum(d["type"].GetString());
	}

	if (!d.HasMember("symbol") || !d["symbol"].IsString()) {
		snprintf(buf, sizeof(buf) - 1, "%s: can't find field 'symbol'",
			BABELTRADER_ERR_MSG[BABELTRADER_ERR_HTTPREQ_FAILED_PARSE - BABELTRADER_ERR_BEGIN]);
		err_msg = buf;
		return false;
	}
	else
	{
		strncpy(msg.symbol, d["symbol"].GetString(), sizeof(msg.symbol) - 1);
	}

	if (!d.HasMember("contract") || !d["contract"].IsString())
	{
		msg.contract[0] = '\0';
	}
	else
	{
		strncpy(msg.contract, d["contract"].GetString(), sizeof(msg.contract) - 1);
	}

	if (!d.HasMember("contract_id") || !d["contract_id"].IsString())
	{
		msg.contract_id[0] = '\0';
	}
	else
	{
		strncpy(msg.contract_id, d["contract_id"].GetString(), sizeof(msg.contract_id) - 1);
	}

	if (!d.HasMember("info1") || !d["info1"].IsString()) 
	{
		msg.info1 = QuoteInfo1_Unknown;
	}
	else
	{
		msg.info1 = getQuoteInfo1Enum(d["info1"].GetString());
	}

	if (!d.HasMember("info2") || !d["info2"].IsString())
	{
		msg.info2 = QuoteInfo2_Unknown;
	}
	else
	{
		msg.info2 = getQuoteInfo2Enum(d["info2"].GetString());
	}

	return true;
}


}