#include "xtp_quote_handler.h"

#include "glog/logging.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "common/converter.h"
#include "common/utils_func.h"

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
		topic_exchange_[topic.symbol] = (ExchangeEnum)topic.exchange;
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
		Quote msg = { 0 };
		msg.market = Market_XTP;
		msg.exchange = topic_exchange_[it->first];
		msg.type = ProductType_Spot;
		strncpy(msg.symbol, it->first.c_str(), sizeof(msg.symbol) - 1);
		msg.info1 = QuoteInfo1_MarketData;
		topics.push_back(msg);
		vec_b.push_back(it->second);

		msg.info1 = QuoteInfo1_Kline;
		msg.info2 = QuoteInfo2_1Min;
		topics.push_back(msg);
		vec_b.push_back(it->second);

		if (conf_.sub_orderbook)
		{
			msg.info1 = QuoteInfo1_OrderBook;
			msg.info2 = QuoteInfo2_Unknown;
			topics.push_back(msg);
			vec_b.push_back(it->second);
		}

		if (conf_.sub_l2)
		{
			msg.info1 = QuoteInfo1_Level2;
			msg.info2 = QuoteInfo2_Unknown;
			topics.push_back(msg);
			vec_b.push_back(it->second);
		}
	}

	return std::move(topics);
}
void XTPQuoteHandler::SubTopic(const Quote &msg)
{
	if (conf_.sub_all)
	{
		return;
	}

	{
		std::unique_lock<std::mutex> lock(topic_mtx_);
		if (sub_topics_.find(msg.symbol) != sub_topics_.end() && sub_topics_[msg.symbol] == true) {
			return;
		}

		sub_topics_[msg.symbol] = false;
		topic_exchange_[msg.symbol] = (ExchangeEnum)msg.exchange;
	}

	char buf[2][64];
	char* topics[2];
	topics[0] = buf[0];
	strncpy(buf[0], msg.symbol, 64 - 1);

	XTP_EXCHANGE_TYPE xtp_exchange_type = ConvertExchangeTypeCommon2XTP((ExchangeEnum)msg.exchange);
	api_->SubscribeMarketData(topics, 1, xtp_exchange_type);
	if (conf_.sub_orderbook)
	{
		api_->SubscribeOrderBook(topics, 1, xtp_exchange_type);
	}
	if (conf_.sub_l2)
	{
		api_->SubscribeTickByTick(topics, 1, xtp_exchange_type);
	}
}
void XTPQuoteHandler::UnsubTopic(const Quote &msg)
{
	if (conf_.sub_all)
	{
		return;
	}

	ExchangeEnum exhange_type = Exchange_Unknown;
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
	strncpy(buf[0], msg.symbol, 64 - 1);

	XTP_EXCHANGE_TYPE xtp_exchange_type = ConvertExchangeTypeCommon2XTP(exhange_type);
	api_->UnSubscribeMarketData(topics, 1, xtp_exchange_type);
	if (conf_.sub_orderbook)
	{
		api_->UnSubscribeOrderBook(topics, 1, xtp_exchange_type);
	}
	if (conf_.sub_l2)
	{
		api_->UnSubscribeTickByTick(topics, 1, xtp_exchange_type);
	}
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
	Reconn();
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
void XTPQuoteHandler::OnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	OutputRspSubOrderBook(ticker, error_info, is_last);
}
void XTPQuoteHandler::OnUnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	OutputRspUnsubOrderBook(ticker, error_info, is_last);
}
void XTPQuoteHandler::OnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	OutputRspSubTickByTick(ticker, error_info, is_last);
}
void XTPQuoteHandler::OnUnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	OutputRspUnsubTickByTick(ticker, error_info, is_last);
}

void XTPQuoteHandler::OnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info)
{
	OutputRspSubAllMarketData(exchange_id, error_info);
}
void XTPQuoteHandler::OnUnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info)
{
	OutputRspUnsubAllMarketData(exchange_id, error_info);
}
void XTPQuoteHandler::OnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info)
{
	OutputRspSubAllOrderBook(exchange_id, error_info);
}
void XTPQuoteHandler::OnUnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info)
{
	OutputRspUnsubAllOrderBook(exchange_id, error_info);
}
void XTPQuoteHandler::OnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info)
{
	OutputRspSubAllTickByTick(exchange_id, error_info);
}
void XTPQuoteHandler::OnUnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info)
{
	OutputRspUnsubAllTickByTick(exchange_id, error_info);
}

void XTPQuoteHandler::OnDepthMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count)
{
	QuoteMarketData msg = { 0 };

#if ENABLE_PERFORMANCE_TEST
	// OutputMarketData(market_data, bid1_qty, bid1_count, max_bid1_count, ask1_qty, ask1_count, max_ask1_count);
	static QuoteTransferMonitor monitor;
	monitor.start();

	auto t = monitor.ts_.time_since_epoch();
	msg.quote.ts = std::chrono::duration_cast<std::chrono::milliseconds>(t).count();
#endif

	ConvertMarketData(market_data, msg.quote, msg.market_data);
	BroadcastMarketData(msg);

	// try update kline
	int64_t sec = (int64_t)time(nullptr);
	QuoteKline kline_msg = { 0 };
	if (kline_builder_.updateMarketData(sec, market_data->ticker, msg.market_data, kline_msg.kline)) {
		memcpy(&kline_msg.quote, &msg.quote, sizeof(kline_msg.quote));
		kline_msg.quote.info1 = QuoteInfo1_Kline;
		kline_msg.quote.info2 = QuoteInfo2_1Min;
		BroadcastKline(kline_msg);
	}

#if ENABLE_PERFORMANCE_TEST
	monitor.end("xtp OnDepthMarketData");
#endif
}
void XTPQuoteHandler::OnOrderBook(XTPOB *order_book)
{
	QuoteOrderBook msg = { 0 };

#if ENABLE_PERFORMANCE_TEST
	// OutputMarketData(market_data, bid1_qty, bid1_count, max_bid1_count, ask1_qty, ask1_count, max_ask1_count);
	static QuoteTransferMonitor monitor;
	monitor.start();

	auto t = monitor.ts_.time_since_epoch();
	msg.quote.ts = std::chrono::duration_cast<std::chrono::milliseconds>(t).count();
#endif

	ConvertOrderBook(order_book, msg.quote, msg.order_book);
	BroadcastOrderBook(msg);

#if ENABLE_PERFORMANCE_TEST
	monitor.end("xtp OnOrderBook");
#endif
}
void XTPQuoteHandler::OnTickByTick(XTPTBT *tbt_data)
{
	QuoteOrderBookLevel2 msg = { 0 };

#if ENABLE_PERFORMANCE_TEST
	// OutputMarketData(market_data, bid1_qty, bid1_count, max_bid1_count, ask1_qty, ask1_count, max_ask1_count);
	static QuoteTransferMonitor monitor;
	monitor.start();

	auto t = monitor.ts_.time_since_epoch();
	msg.quote.ts = std::chrono::duration_cast<std::chrono::milliseconds>(t).count();
#endif
	
	ConvertTickByTick(tbt_data, msg.quote, msg.level2);
	BroadcastLevel2(msg);

#if ENABLE_PERFORMANCE_TEST
	monitor.end("xtp OnTickByTick");
#endif
}


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
	RunAsyncLoop();

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

void XTPQuoteHandler::Reconn()
{
	int ret = 0;
	do {
		LOG(WARNING) << "xtp reconnect...";
		int ms = 1000;
#if WIN32
		Sleep(ms);
#else
		usleep((double)(ms) * 1000.0);
#endif

		ret = api_->Login(
			conf_.ip.c_str(), conf_.port,
			conf_.user_id.c_str(), conf_.password.c_str(),
			(XTP_PROTOCOL_TYPE)conf_.quote_protocol);
	} while (ret != 0);

	SubTopics();
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
	writer.Int(ticker->exchange_id);
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
	writer.Int(ticker->exchange_id);
	writer.Key("symbol");
	writer.String(ticker->ticker);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void XTPQuoteHandler::OutputRspSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspsuborderbook");
	writer.Key("error_id");
	writer.Int(error_info->error_id);
	writer.Key("error_msg");
	writer.String(error_info->error_msg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("exhange");
	writer.Int(ticker->exchange_id);
	writer.Key("symbol");
	writer.String(ticker->ticker);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void XTPQuoteHandler::OutputRspUnsubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspunsuborderbook");
	writer.Key("error_id");
	writer.Int(error_info->error_id);
	writer.Key("error_msg");
	writer.String(error_info->error_msg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("exhange");
	writer.Int(ticker->exchange_id);
	writer.Key("symbol");
	writer.String(ticker->ticker);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void XTPQuoteHandler::OutputRspSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspsubtickbytick");
	writer.Key("error_id");
	writer.Int(error_info->error_id);
	writer.Key("error_msg");
	writer.String(error_info->error_msg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("exhange");
	writer.Int(ticker->exchange_id);
	writer.Key("symbol");
	writer.String(ticker->ticker);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void XTPQuoteHandler::OutputRspUnsubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspunsubtickbytick");
	writer.Key("error_id");
	writer.Int(error_info->error_id);
	writer.Key("error_msg");
	writer.String(error_info->error_msg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("exhange");
	writer.Int(ticker->exchange_id);
	writer.Key("symbol");
	writer.String(ticker->ticker);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}

void XTPQuoteHandler::OutputRspSubAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspsuballmarketdata");
	writer.Key("error_id");
	writer.Int(error_info->error_id);
	writer.Key("error_msg");
	writer.String(error_info->error_msg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("exhange");
	writer.Int(exchange_id);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void XTPQuoteHandler::OutputRspUnsubAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspunsuballmarketdata");
	writer.Key("error_id");
	writer.Int(error_info->error_id);
	writer.Key("error_msg");
	writer.String(error_info->error_msg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("exhange");
	writer.Int(exchange_id);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void XTPQuoteHandler::OutputRspSubAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspsuballorderbook");
	writer.Key("error_id");
	writer.Int(error_info->error_id);
	writer.Key("error_msg");
	writer.String(error_info->error_msg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("exhange");
	writer.Int(exchange_id);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void XTPQuoteHandler::OutputRspUnsubAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspunsuballorderbook");
	writer.Key("error_id");
	writer.Int(error_info->error_id);
	writer.Key("error_msg");
	writer.String(error_info->error_msg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("exhange");
	writer.Int(exchange_id);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void XTPQuoteHandler::OutputRspSubAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspsuballtickbytick");
	writer.Key("error_id");
	writer.Int(error_info->error_id);
	writer.Key("error_msg");
	writer.String(error_info->error_msg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("exhange");
	writer.Int(exchange_id);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void XTPQuoteHandler::OutputRspUnsubAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspunsuballtickbytick");
	writer.Key("error_id");
	writer.Int(error_info->error_id);
	writer.Key("error_msg");
	writer.String(error_info->error_msg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("exhange");
	writer.Int(exchange_id);
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
		writer.Int(market_data->exchange_id);
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
void XTPQuoteHandler::OutputOrderBook(XTPOB *order_book)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("orderbook");
	writer.Key("data");
	writer.StartObject();

	if (order_book)
	{
		writer.Key("exchange_id");
		writer.Int((int)order_book->exchange_id);
		writer.Key("ticker");
		writer.String(order_book->ticker);
		writer.Key("last_price");
		writer.Double(order_book->last_price);
		writer.Key("qty");
		writer.Int64(order_book->qty);
		writer.Key("turnover");
		writer.Double(order_book->turnover);
		writer.Key("trades_count");
		writer.Int64(order_book->trades_count);

		int len = sizeof(order_book->bid) / sizeof(order_book->bid[0]);

		writer.Key("bid");
		writer.StartArray();
		for (int i = 0; i < len; i++)
		{
			writer.Double(order_book->bid[i]);
		}
		writer.EndArray();

		writer.Key("ask");
		writer.StartArray();
		for (int i = 0; i < len; i++)
		{
			writer.Double(order_book->ask[i]);
		}
		writer.EndArray();

		writer.Key("bid_qty");
		writer.StartArray();
		for (int i = 0; i < len; i++)
		{
			writer.Int64(order_book->bid_qty[i]);
		}
		writer.EndArray();

		writer.Key("ask_qty");
		writer.StartArray();
		for (int i = 0; i < len; i++)
		{
			writer.Int64(order_book->ask_qty[i]);
		}
		writer.EndArray();

		writer.Key("data_time");
		writer.Int64(order_book->data_time);
	}

	writer.EndObject(); // data

	writer.EndObject(); // object

	LOG(INFO) << s.GetString();

}
void XTPQuoteHandler::OutputTickByTick(XTPTBT *tbt_data)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("tickbytick");
	writer.Key("data");
	writer.StartObject();

	if (tbt_data)
	{
		writer.Key("exchange_id");
		writer.Int((int)tbt_data->exchange_id);
		writer.Key("ticker");
		writer.String(tbt_data->ticker);
		writer.Key("seq");
		writer.Int64(tbt_data->seq);
		writer.Key("data_time");
		writer.Int64(tbt_data->data_time);
		writer.Key("type");
		writer.Int((int)tbt_data->type);
		switch (tbt_data->type)
		{
		case XTP_TBT_ENTRUST:
		{
			char buf[2] = { 0 };
			writer.Key("channel_no");
			writer.Int(tbt_data->entrust.channel_no);
			writer.Key("seq");
			writer.Int64(tbt_data->entrust.seq);
			writer.Key("price");
			writer.Double(tbt_data->entrust.price);
			writer.Key("qty");
			writer.Int64(tbt_data->entrust.qty);

			buf[0] = tbt_data->entrust.side;
			writer.Key("side");
			writer.String(buf);
			buf[0] = tbt_data->entrust.ord_type;
			writer.Key("ord_type");
			writer.String(buf);
		}break;
		case XTP_TBT_TRADE:
		{
			char buf[2] = { 0 };
			writer.Key("channel_no");
			writer.Int(tbt_data->trade.channel_no);
			writer.Key("seq");
			writer.Int64(tbt_data->trade.seq);
			writer.Key("price");
			writer.Double(tbt_data->trade.price);
			writer.Key("qty");
			writer.Int64(tbt_data->trade.qty);
			writer.Key("money");
			writer.Double(tbt_data->trade.money);
			writer.Key("bid_no");
			writer.Int64(tbt_data->trade.bid_no);
			writer.Key("ask_no");
			writer.Int64(tbt_data->trade.ask_no);
			buf[0] = tbt_data->trade.trade_flag;
			writer.Key("trade_flag");
			writer.String(buf);
		}break;
		}
	}

	writer.EndObject(); // data

	writer.EndObject(); // object

	LOG(INFO) << s.GetString();
}

void XTPQuoteHandler::ConvertMarketData(XTPMD *market_data, Quote &quote, MarketData &md)
{
	quote.market = Market_XTP;
	quote.exchange = ConvertExchangeTypeXTP2Common(market_data->exchange_id);
	switch (market_data->data_type)
	{
	case XTP_MARKETDATA_OPTION: quote.type = ProductType_Option; break;
	default: quote.type = ProductType_Spot; break;
	}
	strncpy(quote.symbol, market_data->ticker, sizeof(quote.symbol) - 1);
	quote.info1 = QuoteInfo1_MarketData;

	md.ts = XTPGetTimestamp(market_data->data_time);
	md.last = market_data->last_price;
	md.bid_ask_len = 10;
	for (int i = 0; i < 10; i++) {
		md.bids[i].price = market_data->bid[i];
		md.bids[i].vol = market_data->bid_qty[i];
		md.asks[i].price = market_data->ask[i];
		md.asks[i].vol = market_data->ask_qty[i];
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
	md.trading_day[0] = '\0';
	md.action_day[0] = '\0';
}
void XTPQuoteHandler::ConvertOrderBook(XTPOB *xtp_order_book, Quote &quote, OrderBook &order_book)
{
	quote.market = Market_XTP;
	quote.exchange = ConvertExchangeTypeXTP2Common(xtp_order_book->exchange_id);
	strncpy(quote.symbol, xtp_order_book->ticker, sizeof(quote.symbol) - 1);
	quote.info1 = QuoteInfo1_OrderBook;

	order_book.ts = XTPGetTimestamp(xtp_order_book->data_time);
	order_book.last = xtp_order_book->last_price;
	order_book.vol = xtp_order_book->qty;
	order_book.bid_ask_len = sizeof(xtp_order_book->bid) / sizeof(xtp_order_book->bid[0]) > BIDASK_MAX_LEN ? BIDASK_MAX_LEN : sizeof(xtp_order_book->bid) / sizeof(xtp_order_book->bid[0]);
	for (int i = 0; i < order_book.bid_ask_len; i++) {
		order_book.bids[i].price = xtp_order_book->bid[i];
		order_book.bids[i].vol = xtp_order_book->bid_qty[i];
		order_book.asks[i].price = xtp_order_book->ask[i];
		order_book.asks[i].vol = xtp_order_book->ask_qty[i];
	}
}
void XTPQuoteHandler::ConvertTickByTick(XTPTBT *tbt_data, Quote &quote, OrderBookLevel2 &level2)
{
	quote.market = Market_XTP;
	quote.exchange = ConvertExchangeTypeXTP2Common(tbt_data->exchange_id);
	strncpy(quote.symbol, tbt_data->ticker, sizeof(quote.symbol) - 1);
	quote.info1 = QuoteInfo1_Level2;

	level2.ts = XTPGetTimestamp(tbt_data->data_time);
	if (tbt_data->type == XTP_TBT_ENTRUST)
	{
		level2.action = OrderBookL2Action_Entrust;
		level2.entrust.channel_no = tbt_data->entrust.channel_no;
		level2.entrust.seq = tbt_data->entrust.seq;
		level2.entrust.price = tbt_data->entrust.price;
		level2.entrust.vol = tbt_data->entrust.qty;
		level2.entrust.dir = ConvertOrderAction(tbt_data->entrust.side);
		level2.entrust.order_type = ConvertOrderType(tbt_data->entrust.ord_type);
	}
	else
	{
		level2.action = OrderBookL2Action_Trade;
		level2.trade.channel_no = tbt_data->trade.channel_no;
		level2.trade.seq = tbt_data->trade.seq;
		level2.trade.price = tbt_data->trade.price;
		level2.trade.vol = tbt_data->trade.qty;
		level2.trade.bid_no = tbt_data->trade.bid_no;
		level2.trade.ask_no = tbt_data->trade.ask_no;
		level2.trade.trade_flag = ConvertTradeFlag(tbt_data->trade.trade_flag);
	}
}

void XTPQuoteHandler::SubTopics()
{
	if (conf_.sub_all)
	{
		api_->SubscribeAllMarketData();
		if (conf_.sub_orderbook)
		{
			api_->SubscribeAllOrderBook();
		}
		if (conf_.sub_l2)
		{
			api_->SubscribeAllTickByTick();
		}
	}
	else
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
				XTP_EXCHANGE_TYPE xtp_exchange_type = ConvertExchangeTypeCommon2XTP(topic_exchange_[it->first]);
				api_->SubscribeMarketData(topics, 1, xtp_exchange_type);
				if (conf_.sub_orderbook)
				{
					api_->SubscribeOrderBook(topics, 1, xtp_exchange_type);
				}
				if (conf_.sub_l2)
				{
					api_->SubscribeTickByTick(topics, 1, xtp_exchange_type);
				}
			}
		}
	}
}

XTP_EXCHANGE_TYPE XTPQuoteHandler::ConvertExchangeTypeCommon2XTP(ExchangeEnum exchange)
{
	switch (exchange)
	{
	case Exchange_SSE: return XTP_EXCHANGE_SH;
	case Exchange_SZSE: return XTP_EXCHANGE_SZ;
	default: return XTP_EXCHANGE_UNKNOWN;
	}
}
ExchangeEnum XTPQuoteHandler::ConvertExchangeTypeXTP2Common(XTP_EXCHANGE_TYPE exchange_type)
{
	switch (exchange_type) {
	case XTP_EXCHANGE_SH: return Exchange_SSE;
	case XTP_EXCHANGE_SZ: return Exchange_SZSE;
	default: return Exchange_Unknown;
	}
}

OrderActionEnum XTPQuoteHandler::ConvertOrderAction(char side)
{
	switch (side)
	{
	case '1': return OrderAction_Buy;
	case '2': return OrderAction_Sell;
	case 'G': return OrderAction_Borrow;
	case 'F': return OrderAction_Lend;
	}
	return OrderAction_Unknown;
}
OrderTypeEnum XTPQuoteHandler::ConvertOrderType(char order_type)
{
	switch (order_type)
	{
	case '1': return OrderType_Market;
	case '2': return OrderType_Limit;
	case 'U': return OrderType_Best;
	}
	return OrderType_Unknown;
}
OrderBookL2TradeFlagEnum XTPQuoteHandler::ConvertTradeFlag(char trade_flag)
{
	switch (trade_flag)
	{
	case 'B': return OrderBookL2TradeFlag_Buy;
	case 'S': return OrderBookL2TradeFlag_Sell;
	case '4': return OrderBookL2TradeFlag_Cancel;
	case 'F': return OrderBookL2TradeFlag_Deal;
	}
	return OrderBookL2TradeFlag_Unknown;
}