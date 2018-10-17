#include "xtp_quote_handler.h"

#include "glog/logging.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

const char *exchange_SSE = "SSE";
const char *exchange_SZSE = "SZSE";
const char *exchange_UNKNOWN = "UNKNOWN";

XTPQuoteHandler::XTPQuoteHandler(XTPQuoteConf &conf)
	: api_(nullptr)
	, conf_(conf)
	, req_id_(1)
	, ws_service_(this, nullptr)
	, http_service_(this, nullptr)
{}

void XTPQuoteHandler::run()
{
	// load sub topics
	for (auto topic : conf_.default_sub_topics) {
		sub_topics_[topic.symbol] = false;
		topic_exchange_[topic.symbol] = GetExchangeType(topic.exchange);
	}

	// init xtp api
	RunAPI();

	// run service
	RunService();
}

std::vector<Quote> XTPQuoteHandler::GetSubTopics(std::vector<bool> &vec_b)
{
	std::vector<Quote> topics;
	vec_b.clear();

	std::unique_lock<std::mutex> lock(topic_mtx_);
	for (auto it = sub_topics_.begin(); it != sub_topics_.end(); ++it) {
		Quote msg;
		msg.market = "xtp";
		msg.exchange = topic_exchange_[it->first];
		msg.type = "spot";
		msg.symbol = it->first;
		msg.contract = "";
		msg.contract_id = msg.contract;
		msg.info1 = "marketdata";
		topics.push_back(msg);
		vec_b.push_back(it->second);

		msg.info1 = "kline";
		msg.info2 = "1m";
		topics.push_back(msg);
		vec_b.push_back(it->second);
	}

	return std::move(topics);
}
void XTPQuoteHandler::SubTopic(const Quote &msg)
{
	// TODO:
}
void XTPQuoteHandler::UnsubTopic(const Quote &msg)
{
	// TODO:
}


void XTPQuoteHandler::OnDisconnected(int reason)
{
	OutputFrontDisconnected();

	// set sub topics flag
	{
		std::unique_lock<std::mutex> lock(topic_mtx_);
		for (auto it = sub_topics_.begin(); it != sub_topics_.end(); ++it) {
			it->second = false;
		}
	}

	// reconnect
	auto old_api = api_;
	RunAPI();
	old_api->Release();
}
void XTPQuoteHandler::OnError(XTPRI *error_info)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("on_error");
	writer.Key("error_id");
	writer.Int(error_info->error_id);
	writer.Key("error_msg");
	writer.String(error_info->error_msg);
	writer.EndObject();
	LOG(INFO) << s.GetString();
}

void XTPQuoteHandler::OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	OutputRspSubMarketData(ticker, error_info, is_last);

	// TODO: 
}
void XTPQuoteHandler::OnUnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last) {}
void XTPQuoteHandler::OnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last) {}
void XTPQuoteHandler::OnUnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last) {}
void XTPQuoteHandler::OnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last) {}
void XTPQuoteHandler::OnUnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last) {}

void XTPQuoteHandler::OnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}
void XTPQuoteHandler::OnUnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}
void XTPQuoteHandler::OnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}
void XTPQuoteHandler::OnUnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}
void XTPQuoteHandler::OnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}
void XTPQuoteHandler::OnUnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}

void XTPQuoteHandler::OnDepthMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count) {}
void XTPQuoteHandler::OnOrderBook(XTPOB *order_book) {}
void XTPQuoteHandler::OnTickByTick(XTPTBT *tbt_data) {}


void XTPQuoteHandler::RunAPI()
{
	api_ = XTP::API::QuoteApi::CreateQuoteApi(conf_.client_id, "./log");
	api_->RegisterSpi(this);
	api_->SetHeartBeatInterval(15);
	
	int ret = -1;
	ret = api_->Login(
		conf_.ip.c_str(), conf_.port, 
		conf_.user_id.c_str(), conf_.password.c_str(), 
		(XTP_PROTOCOL_TYPE)conf_.quote_protocol);
	if (ret != 0) {
		LOG(INFO) << "Failed to login";
		exit(-1);
	}

	SubTopics();
}
void XTPQuoteHandler::RunService()
{
	auto loop_thread = std::thread([&] {
		uws_hub_.onConnection([&](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
			ws_service_.onConnection(ws, req);
		});
		uws_hub_.onMessage([&](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
			ws_service_.onMessage(ws, message, length, opCode);
		});
		uws_hub_.onDisconnection([&](uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {
		});

		// rest
		uws_hub_.onHttpRequest([&](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t length, size_t remainingBytes) {
			http_service_.onMessage(res, req, data, length, remainingBytes);
		});

		if (!uws_hub_.listen(conf_.quote_ip.c_str(), conf_.quote_port, nullptr, uS::ListenOptions::REUSE_PORT)) {
			LOG(INFO) << "Failed to listen";
			exit(-1);
		}
		uws_hub_.run();
	});

	loop_thread.join();
}

void XTPQuoteHandler::OutputFrontDisconnected()
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("disconnected");
	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void XTPQuoteHandler::OutputRspSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspsubmarketdata");
	writer.Key("error_id");
	writer.Int(error_info->error_id);
	writer.Key("error_msg");
	writer.String(error_info->error_msg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("exhange");
	writer.String(ConvertExchangeType2Str(ticker->exchange_id));
	writer.Key("symbol");
	writer.String(ticker->ticker);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}

void XTPQuoteHandler::SubTopics()
{
	char buf[2][64];
	char* topics[2];
	for (int i = 0; i < 2; i++) {
		topics[i] = buf[i];
	}

	std::unique_lock<std::mutex> lock(topic_mtx_);
	for (auto it = sub_topics_.begin(); it != sub_topics_.end(); ++it) {
		if (it->second == false) {
			strncpy(buf[0], it->first.c_str(), sizeof(buf[0]) - 1);
			api_->SubscribeMarketData(topics, 1, topic_exchange_[it->first]);
		}
	}
}

XTP_EXCHANGE_TYPE XTPQuoteHandler::GetExchangeType(const std::string &exchange)
{
	if (exchange == "SSE")
	{
		return XTP_EXCHANGE_SH;
	}
	else if (exchange == "SZSE")
	{
		return XTP_EXCHANGE_SZ;
	}
	
	return XTP_EXCHANGE_UNKNOWN;
}

const char* XTPQuoteHandler::ConvertExchangeType2Str(XTP_EXCHANGE_TYPE exchange_type)
{
	switch (exchange_type) {
	case XTP_EXCHANGE_SH: return exchange_SSE;
	case XTP_EXCHANGE_SZ: return exchange_SZSE;
	default: return exchange_UNKNOWN;
	}
}