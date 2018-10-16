#include "http_service.h"

#include <assert.h>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

HttpService::HttpService(QuoteService *quote_service, TradeService *trade_service)
	: quote_(quote_service)
	, trade_(trade_service)
{}

void HttpService::onMessage(uWS::HttpResponse *res, uWS::HttpRequest &req, char *data, size_t length, size_t remainingBytes)
{
	auto url = req.getUrl().toString();
	if (url == "/topics/get") {
		GetSubtopics(res);
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
		const SubUnsubMsg &msg = topics[idx];

		writer.StartObject();
		writer.Key("market");
		writer.String(msg.market.c_str());
		writer.Key("exchange");
		writer.String(msg.exchange.c_str());
		writer.Key("type");
		writer.String(msg.type.c_str());
		writer.Key("symbol");
		writer.String(msg.symbol.c_str());
		writer.Key("contract");
		writer.String(msg.contract.c_str());
		writer.Key("contract_id");
		writer.String(msg.contract_id.c_str());
		writer.Key("info1");
		writer.String(msg.info1.c_str());
		writer.Key("info2");
		writer.String(msg.info2.c_str());
		writer.Key("subed");
		writer.Int(vec_b[idx] ? 1 : 0);
		writer.EndObject();
	}
	writer.EndArray();

	writer.EndObject();

	res->end(s.GetString(), s.GetLength());
}