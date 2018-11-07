#include "xtp_quote_handler.h"

#include "glog/logging.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "common/converter.h"

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
		msg.market = g_markets[Market_XTP];
		msg.exchange = topic_exchange_[it->first];
		msg.type = g_product_types[ProductType_Spot];
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
	{
		std::unique_lock<std::mutex> lock(topic_mtx_);
		if (sub_topics_.find(msg.symbol) != sub_topics_.end() && sub_topics_[msg.symbol] == true) {
			return;
		}

		sub_topics_[msg.symbol] = false;
		topic_exchange_[msg.symbol] = GetExchangeType(msg.exchange);
	}

	char buf[2][64];
	char* topics[2];
	topics[0] = buf[0];
	strncpy(buf[0], msg.symbol.c_str(), 64 - 1);

	api_->SubscribeMarketData(topics, 1, GetExchangeType(msg.exchange));
}
void XTPQuoteHandler::UnsubTopic(const Quote &msg)
{
	XTP_EXCHANGE_TYPE exhange_type = XTP_EXCHANGE_UNKNOWN;
	{
		std::unique_lock<std::mutex> lock(topic_mtx_);
		if (sub_topics_.find(msg.symbol) == sub_topics_.end()) {
			return;
		}
		exhange_type = topic_exchange_[msg.symbol];
	}

	char buf[2][64];
	char* topics[2];
	topics[0] = buf[0];
	strncpy(buf[0], msg.symbol.c_str(), 64 - 1);

	api_->UnSubscribeMarketData(topics, 1, exhange_type);
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
	// output
	OutputRspSubMarketData(ticker, error_info, is_last);

	// set topic flag
	if (error_info->error_id == 0) {
		std::unique_lock<std::mutex> lock(topic_mtx_);
		sub_topics_[ticker->ticker] = true;
		kline_builder_.add(ticker->ticker);
	}
}
void XTPQuoteHandler::OnUnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	// output
	OutputRspUnsubMarketData(ticker, error_info, is_last);

	// delete topics
	if (error_info->error_id == 0) {
		std::unique_lock<std::mutex> lock(topic_mtx_);
		sub_topics_.erase(ticker->ticker);
		topic_exchange_.erase(ticker->ticker);
		kline_builder_.del(ticker->ticker);
	}
}
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

void XTPQuoteHandler::OnDepthMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count)
{
#ifndef NDEBUG
	OutputMarketData(market_data, bid1_qty, bid1_count, max_bid1_count, ask1_qty, ask1_count, max_ask1_count);
#endif

	// convert to common struct
	Quote quote;
	MarketData md;

	ConvertMarketData(market_data, quote, md);

	BroadcastMarketData(quote, md);

	// try update kline
	int64_t sec = (int64_t)time(nullptr);
	Kline kline;
	if (kline_builder_.updateMarketData(sec, market_data->ticker, md, kline)) {
		quote.info1 = "kline";
		quote.info2 = "1m";
		BroadcastKline(quote, kline);
	}

}
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
		auto ri = api_->GetApiLastError();
		LOG(INFO) << "Failed to login "
			<< "(" << ri->error_id << ") "
			<< ri->error_msg;
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
			ws_service_.onDisconnection(ws, code, message, length);
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
void XTPQuoteHandler::OutputRspUnsubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspunsubmarketdata");
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
void XTPQuoteHandler::OutputMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("marketdata");
	writer.Key("data");
	writer.StartObject();

	{
		writer.Key("exchange_id");
		writer.String(ConvertExchangeType2Str(market_data->exchange_id));
		writer.Key("ticker");
		writer.String(market_data->ticker);
		writer.Key("last_price");
		writer.Double(market_data->last_price);
		writer.Key("pre_close_price");
		writer.Double(market_data->pre_close_price);
		writer.Key("open_price");
		writer.Double(market_data->open_price);
		writer.Key("high_price");
		writer.Double(market_data->high_price);
		writer.Key("low_price");
		writer.Double(market_data->low_price);
		writer.Key("close_price");
		writer.Double(market_data->close_price);

		writer.Key("pre_total_long_positon");
		writer.Int64(market_data->pre_total_long_positon);
		writer.Key("total_long_positon");
		writer.Int64(market_data->total_long_positon);
		writer.Key("pre_settl_price");
		writer.Double(market_data->pre_settl_price);
		writer.Key("settl_price");
		writer.Double(market_data->settl_price);

		writer.Key("upper_limit_price");
		writer.Double(market_data->upper_limit_price);
		writer.Key("lower_limit_price");
		writer.Double(market_data->lower_limit_price);
		writer.Key("pre_delta");
		writer.Double(market_data->pre_delta);
		writer.Key("curr_delta");
		writer.Double(market_data->curr_delta);

		writer.Key("data_time");
		writer.Int64(market_data->data_time);

		writer.Key("qty");
		writer.Int64(market_data->qty);

		writer.Key("turnover");
		writer.Double(market_data->turnover);
		writer.Key("avg_price");
		writer.Double(market_data->avg_price);

		writer.Key("bid");
		writer.StartArray();
		for (int i = 0; i < 10; i++) {
			writer.Double(market_data->bid[i]);
		}
		writer.EndArray();

		writer.Key("ask");
		writer.StartArray();
		for (int i = 0; i < 10; i++) {
			writer.Double(market_data->ask[i]);
		}
		writer.EndArray();

		writer.Key("bid_qty");
		writer.StartArray();
		for (int i = 0; i < 10; i++) {
			writer.Int64(market_data->bid_qty[i]);
		}
		writer.EndArray();

		writer.Key("ask_qty");
		writer.StartArray();
		for (int i = 0; i < 10; i++) {
			writer.Int64(market_data->ask_qty[i]);
		}
		writer.EndArray();

		writer.Key("trades_count");
		writer.Int64(market_data->trades_count);
	}
	
	writer.EndObject(); // data

	writer.EndObject(); // object

	LOG(INFO) << s.GetString();
}

void XTPQuoteHandler::ConvertMarketData(XTPMD *market_data, Quote &quote, MarketData &md)
{
	quote.market = g_markets[Market_XTP];
	quote.exchange = ConvertExchangeType2Str(market_data->exchange_id);
	switch (market_data->data_type)
	{
	case XTP_MARKETDATA_OPTION: quote.type = g_product_types[ProductType_Option]; break;
	default: quote.type = g_product_types[ProductType_Spot]; break;
	}
	quote.symbol = market_data->ticker;
	quote.contract = "";
	quote.contract_id = "";
	quote.info1 = "marketdata";
	quote.info2 = "";

	md.ts = GetUpdateTimeMs(market_data);
	md.last = market_data->last_price;
	for (int i = 0; i < 10; i++) {
		md.bids.push_back({ market_data->bid[i], market_data->bid_qty[i] });
		md.asks.push_back({ market_data->ask[i], market_data->ask_qty[i] });
	}
	md.vol = market_data->qty;
	md.turnover = market_data->turnover;
	md.avg_price = market_data->avg_price;
	md.pre_settlement = market_data->pre_settl_price;
	md.pre_close = market_data->pre_close_price;
	md.pre_open_interest = market_data->pre_total_long_positon;
	md.settlement = market_data->settl_price;
	md.close = market_data->close_price;
	md.open_interest = market_data->total_long_positon;
	md.upper_limit = market_data->upper_limit_price;
	md.lower_limit = market_data->lower_limit_price;
	md.open = market_data->open_price;
	md.high = market_data->high_price;
	md.low = market_data->low_price;
	md.trading_day = "";
	md.action_day = "";
}

void XTPQuoteHandler::BroadcastMarketData(const Quote &quote, const MarketData &md)
{
	// serialize
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	SerializeQuoteBegin(writer, quote);
	SerializeMarketData(writer, md);
	SerializeQuoteEnd(writer, quote);

#ifndef NDEBUG
	LOG(INFO) << s.GetString();
#endif

	uws_hub_.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void XTPQuoteHandler::BroadcastKline(const Quote &quote, const Kline &kline)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	SerializeQuoteBegin(writer, quote);
	SerializeKline(writer, kline);
	SerializeQuoteEnd(writer, quote);

#ifndef NDEBUG
	LOG(INFO) << s.GetString();
#endif

	uws_hub_.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}

int64_t XTPQuoteHandler::GetUpdateTimeMs(XTPMD *market_data)
{
	struct tm time_info = { 0 };

	int64_t year = market_data->data_time / 10000000000000;
	int64_t month = (market_data->data_time / 100000000000) % 100;
	int64_t day = (market_data->data_time / 1000000000) % 100;
	int64_t hour = (market_data->data_time / 10000000) % 100;
	int64_t minute = (market_data->data_time / 100000) % 100;
	int64_t sec = (market_data->data_time / 1000) % 100;
	int64_t mill = market_data->data_time % 1000;

	time_info.tm_year = year - 1900;
	time_info.tm_mon = month - 1;
	time_info.tm_mday = day;
	time_info.tm_hour = hour;
	time_info.tm_min = minute;
	time_info.tm_sec = sec;

	time_t utc_sec = mktime(&time_info);
	return (int64_t)utc_sec * 1000 + mill;
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