#include "ctp_quote_handler.h"

#include "glog/logging.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "common/serialization.h"

CTPQuoteHandler::CTPQuoteHandler(CTPQuoteConf &conf)
	: api_(nullptr)
	, conf_(conf)
	, req_id_(1)
	, ws_service_(this, nullptr)
	, http_service_(this, nullptr)
{}

void CTPQuoteHandler::run()
{
	// load sub topics
	for (auto topic : conf_.default_sub_topics) {
		sub_topics_[topic] = false;
	}

	// init ctp api
	RunAPI();

	// run service
	RunService();
}

std::vector<Quote> CTPQuoteHandler::GetSubTopics(std::vector<bool> &vec_b)
{
	std::vector<Quote> topics;
	vec_b.clear();

	std::unique_lock<std::mutex> lock(topic_mtx_);
	for (auto it = sub_topics_.begin(); it != sub_topics_.end(); ++it) {
		Quote msg;
		msg.market = "ctp";
		msg.exchange = "";
		msg.type = "future";
		SplitInstrument(it->first.c_str(), msg.symbol, msg.contract);
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
void CTPQuoteHandler::SubTopic(const Quote &msg)
{
	auto instrument = msg.symbol + msg.contract;
	std::unique_lock<std::mutex> lock(topic_mtx_);
	if (sub_topics_.find(instrument) != sub_topics_.end()) {
		return;
	}

	sub_topics_[instrument] = false;

	char buf[2][64];
	char* topics[2];
	topics[0] = buf[0];
	strncpy(buf[0], instrument.c_str(), 64-1);

	api_->SubscribeMarketData(topics, 1);
}
void CTPQuoteHandler::UnsubTopic(const Quote &msg)
{
	auto instrument = msg.symbol + msg.contract;
	std::unique_lock<std::mutex> lock(topic_mtx_);
	if (sub_topics_.find(instrument) == sub_topics_.end()) {
		return;
	}

	char buf[2][64];
	char* topics[2];
	topics[0] = buf[0];
	strncpy(buf[0], instrument.c_str(), 64 - 1);

	api_->UnSubscribeMarketData(topics, 1);
}

void CTPQuoteHandler::OnFrontConnected()
{
	// output
	OutputFrontConnected();

	// user login
	CThostFtdcReqUserLoginField req_user_login = { 0 };
	strncpy(req_user_login.BrokerID, conf_.broker_id.c_str(), sizeof(req_user_login.BrokerID) - 1);
	strncpy(req_user_login.UserID, conf_.user_id.c_str(), sizeof(req_user_login.UserID) - 1);
	strncpy(req_user_login.Password, conf_.password.c_str(), sizeof(req_user_login.Password) - 1);
	api_->ReqUserLogin(&req_user_login, req_id_++);
}
void CTPQuoteHandler::OnFrontDisconnected(int nReason)
{
	// output
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
void CTPQuoteHandler::OnHeartBeatWarning(int nTimeLapse)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("heartbeat_warning");
	writer.Key("time_elapse");
	writer.Int(nTimeLapse);
	writer.EndObject();
	LOG(INFO) << s.GetString();
}

void CTPQuoteHandler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspUserLogin(pRspUserLogin, pRspInfo, nRequestID, bIsLast);

	// sub topics
	SubTopics();
}
void CTPQuoteHandler::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspUserLogout(pUserLogout, pRspInfo, nRequestID, bIsLast);
}

void CTPQuoteHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("on_error");
	writer.Key("req_id");
	writer.Int(nRequestID);
	writer.Key("error_id");
	writer.Int(pRspInfo->ErrorID);
	writer.Key("error_msg");
	writer.String(pRspInfo->ErrorMsg);

	writer.EndObject();
	LOG(INFO) << s.GetString();
}

void CTPQuoteHandler::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspSubMarketData(pSpecificInstrument, pRspInfo, nRequestID, bIsLast);

	// set topic flag
	if (pRspInfo->ErrorID == 0) {
		std::unique_lock<std::mutex> lock(topic_mtx_);
		sub_topics_[pSpecificInstrument->InstrumentID] = true;
		kline_builder_.add(pSpecificInstrument->InstrumentID);
	}
}
void CTPQuoteHandler::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspUnsubMarketData(pSpecificInstrument, pRspInfo, nRequestID, bIsLast);

	// delete topics
	if (pRspInfo->ErrorID == 0) {
		std::unique_lock<std::mutex> lock(topic_mtx_);
		sub_topics_.erase(pSpecificInstrument->InstrumentID);
		kline_builder_.del(pSpecificInstrument->InstrumentID);
	}
}

void CTPQuoteHandler::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}
void CTPQuoteHandler::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CTPQuoteHandler::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
#ifndef NDEBUG
	OutputMarketData(pDepthMarketData);
#endif

	// convert to common struct
	Quote quote;
	MarketData md;
	ConvertMarketData(pDepthMarketData, quote, md);

	BroadcastMarketData(quote, md);

	// try update kline
	int64_t sec = (int64_t)time(nullptr);
	Kline kline;
	if (kline_builder_.updateMarketData(sec, pDepthMarketData->InstrumentID, md, kline)) {
		quote.info1 = "kline";
		quote.info2 = "1m";
		BroadcastKline(quote, kline);
	}
}
void CTPQuoteHandler::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) {}


void CTPQuoteHandler::RunAPI()
{
	api_ = CThostFtdcMdApi::CreateFtdcMdApi();
	LOG(INFO) << "CTP quotes API version:" << api_->GetApiVersion();

	char addr[256] = { 0 };
	strncpy(addr, conf_.addr.c_str(), sizeof(addr) - 1);

	api_->RegisterSpi(this);
	api_->RegisterFront(addr);
	api_->Init();
}
void CTPQuoteHandler::RunService()
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

void CTPQuoteHandler::OutputFrontConnected()
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("connected");
	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPQuoteHandler::OutputFrontDisconnected()
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("disconnected");
	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPQuoteHandler::OutputRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("login");
	writer.Key("req_id");
	writer.Int(nRequestID);
	writer.Key("error_id");
	writer.Int(pRspInfo->ErrorID);
	writer.Key("error_msg");
	writer.String(pRspInfo->ErrorMsg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("TradingDay");
	writer.String(pRspUserLogin->TradingDay);
	writer.Key("LoginTime");
	writer.String(pRspUserLogin->LoginTime);
	writer.Key("BrokerID");
	writer.String(pRspUserLogin->BrokerID);
	writer.Key("UserID");
	writer.String(pRspUserLogin->UserID);
	writer.Key("SystemName");
	writer.String(pRspUserLogin->SystemName);
	writer.Key("FrontID");
	writer.Int(pRspUserLogin->FrontID);
	writer.Key("SessionID");
	writer.Int(pRspUserLogin->SessionID);
	writer.Key("MaxOrderRef");
	writer.String(pRspUserLogin->MaxOrderRef);
	writer.Key("SHFETime");
	writer.String(pRspUserLogin->SHFETime);
	writer.Key("DCETime");
	writer.String(pRspUserLogin->DCETime);
	writer.Key("CZCETime");
	writer.String(pRspUserLogin->CZCETime);
	writer.Key("FFEXTime");
	writer.String(pRspUserLogin->FFEXTime);
	writer.Key("INETime");
	writer.String(pRspUserLogin->INETime);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPQuoteHandler::OutputRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("logout");
	writer.Key("req_id");
	writer.Int(nRequestID);
	writer.Key("error_id");
	writer.Int(pRspInfo->ErrorID);
	writer.Key("error_msg");
	writer.String(pRspInfo->ErrorMsg);

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPQuoteHandler::OutputRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspsubmarketdata");
	writer.Key("req_id");
	writer.Int(nRequestID);
	writer.Key("error_id");
	writer.Int(pRspInfo->ErrorID);
	writer.Key("error_msg");
	writer.String(pRspInfo->ErrorMsg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("InstrumentID");
	writer.String(pSpecificInstrument->InstrumentID);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPQuoteHandler::OutputRspUnsubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspunsubmarketdata");
	writer.Key("req_id");
	writer.Int(nRequestID);
	writer.Key("error_id");
	writer.Int(pRspInfo->ErrorID);
	writer.Key("error_msg");
	writer.String(pRspInfo->ErrorMsg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("InstrumentID");
	writer.String(pSpecificInstrument->InstrumentID);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPQuoteHandler::OutputMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("marketdata");

	writer.Key("data");
	writer.StartObject();
	writer.Key("TradingDay");
	writer.String(pDepthMarketData->TradingDay);
	writer.Key("InstrumentID");
	writer.String(pDepthMarketData->InstrumentID);
	writer.Key("ExchangeID");
	writer.String(pDepthMarketData->ExchangeID);
	writer.Key("ExchangeInstID");
	writer.String(pDepthMarketData->ExchangeInstID);
	writer.Key("LastPrice");
	writer.Double(pDepthMarketData->LastPrice);
	writer.Key("PreSettlementPrice");
	writer.Double(pDepthMarketData->PreSettlementPrice);
	writer.Key("PreClosePrice");
	writer.Double(pDepthMarketData->PreClosePrice);
	writer.Key("PreOpenInterest");
	writer.Double(pDepthMarketData->PreOpenInterest);
	writer.Key("OpenPrice");
	writer.Double(pDepthMarketData->OpenPrice);
	writer.Key("HighestPrice");
	writer.Double(pDepthMarketData->HighestPrice);
	writer.Key("LowestPrice");
	writer.Double(pDepthMarketData->LowestPrice);
	writer.Key("Volume");
	writer.Double(pDepthMarketData->Volume);
	writer.Key("Turnover");
	writer.Double(pDepthMarketData->Turnover);
	writer.Key("OpenInterest");
	writer.Double(pDepthMarketData->OpenInterest);
	writer.Key("ClosePrice");
	writer.Double(pDepthMarketData->ClosePrice);
	writer.Key("SettlementPrice");
	writer.Double(pDepthMarketData->SettlementPrice);
	writer.Key("UpperLimitPrice");
	writer.Double(pDepthMarketData->UpperLimitPrice);
	writer.Key("LowerLimitPrice");
	writer.Double(pDepthMarketData->LowerLimitPrice);
	writer.Key("PreDelta");
	writer.Double(pDepthMarketData->PreDelta);
	writer.Key("CurrDelta");
	writer.Double(pDepthMarketData->CurrDelta);
	writer.Key("UpdateTime");
	writer.String(pDepthMarketData->UpdateTime);
	writer.Key("UpdateMillisec");
	writer.Int(pDepthMarketData->UpdateMillisec);
	writer.Key("BidPrice1");
	writer.Double(pDepthMarketData->BidPrice1);
	writer.Key("BidVolume1");
	writer.Int(pDepthMarketData->BidVolume1);
	writer.Key("AskPrice1");
	writer.Double(pDepthMarketData->AskPrice1);
	writer.Key("AskVolume1");
	writer.Int(pDepthMarketData->AskVolume1);
	writer.Key("BidPrice2");
	writer.Double(pDepthMarketData->BidPrice2);
	writer.Key("BidVolume2");
	writer.Int(pDepthMarketData->BidVolume2);
	writer.Key("AskPrice2");
	writer.Double(pDepthMarketData->AskPrice2);
	writer.Key("AskVolume2");
	writer.Int(pDepthMarketData->AskVolume2);
	writer.Key("BidPrice3");
	writer.Double(pDepthMarketData->BidPrice3);
	writer.Key("BidVolume3");
	writer.Int(pDepthMarketData->BidVolume3);
	writer.Key("AskPrice3");
	writer.Double(pDepthMarketData->AskPrice3);
	writer.Key("AskVolume3");
	writer.Int(pDepthMarketData->AskVolume3);
	writer.Key("BidPrice4");
	writer.Double(pDepthMarketData->BidPrice4);
	writer.Key("BidVolume4");
	writer.Int(pDepthMarketData->BidVolume4);
	writer.Key("AskPrice4");
	writer.Double(pDepthMarketData->AskPrice4);
	writer.Key("AskVolume4");
	writer.Int(pDepthMarketData->AskVolume4);
	writer.Key("BidPrice5");
	writer.Double(pDepthMarketData->BidPrice5);
	writer.Key("BidVolume5");
	writer.Int(pDepthMarketData->BidVolume5);
	writer.Key("AskPrice5");
	writer.Double(pDepthMarketData->AskPrice5);
	writer.Key("AskVolume5");
	writer.Int(pDepthMarketData->AskVolume5);
	writer.Key("AveragePrice");
	writer.Double(pDepthMarketData->AveragePrice);
	writer.Key("ActionDay");
	writer.String(pDepthMarketData->ActionDay);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}

void CTPQuoteHandler::ConvertMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, Quote &quote, MarketData &md)
{
	// split instrument into symbol and contract
	std::string symbol, contract;
	SplitInstrument(pDepthMarketData->InstrumentID, symbol, contract);

	// get update time
	int64_t ts = GetUpdateTimeMs(pDepthMarketData);

	quote.market = "ctp";
	quote.exchange = pDepthMarketData->ExchangeID;
	quote.type = "future";
	quote.symbol = std::move(symbol);
	quote.contract = std::move(contract);
	quote.contract_id = quote.contract;
	quote.info1 = "marketdata";

	md.ts = ts;
	md.last = pDepthMarketData->LastPrice;
	md.bids.push_back({ pDepthMarketData->BidPrice1, pDepthMarketData->BidVolume1 });
	md.bids.push_back({ pDepthMarketData->BidPrice2, pDepthMarketData->BidVolume2 });
	md.bids.push_back({ pDepthMarketData->BidPrice3, pDepthMarketData->BidVolume3 });
	md.bids.push_back({ pDepthMarketData->BidPrice4, pDepthMarketData->BidVolume4 });
	md.bids.push_back({ pDepthMarketData->BidPrice5, pDepthMarketData->BidVolume5 });
	md.asks.push_back({ pDepthMarketData->AskPrice1, pDepthMarketData->AskVolume1 });
	md.asks.push_back({ pDepthMarketData->AskPrice2, pDepthMarketData->AskVolume2 });
	md.asks.push_back({ pDepthMarketData->AskPrice3, pDepthMarketData->AskVolume3 });
	md.asks.push_back({ pDepthMarketData->AskPrice4, pDepthMarketData->AskVolume4 });
	md.asks.push_back({ pDepthMarketData->AskPrice5, pDepthMarketData->AskVolume5 });
	md.vol = pDepthMarketData->Volume;
	md.pre_settlement = pDepthMarketData->PreSettlementPrice;
	md.pre_close = pDepthMarketData->PreClosePrice;
	md.pre_open_interest = pDepthMarketData->PreOpenInterest;
	md.settlement = pDepthMarketData->SettlementPrice;
	md.close = pDepthMarketData->ClosePrice;
	md.open_interest = pDepthMarketData->OpenInterest;
	md.upper_limit = pDepthMarketData->UpperLimitPrice;
	md.lower_limit = pDepthMarketData->LowerLimitPrice;
	md.trading_day = pDepthMarketData->TradingDay;
	md.action_day = pDepthMarketData->ActionDay;
}

void CTPQuoteHandler::BroadcastMarketData(const Quote &quote, const MarketData &md)
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
void CTPQuoteHandler::BroadcastKline(const Quote &quote, const Kline &kline)
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

void CTPQuoteHandler::SplitInstrument(const char *instrument, std::string &symbol, std::string &contract)
{
	char buf[64] = { 0 };
	const char *p = instrument;
	while (*p) {
		if (*p >= '0' && *p <= '9') {
			break;
		}
		p++;
	}
	auto len = p - instrument;
	strncpy(buf, instrument, len);
	symbol = buf;
	contract = p;
}

int64_t CTPQuoteHandler::GetUpdateTimeMs(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	struct tm time_info = { 0 };
	int date = atoi(pDepthMarketData->ActionDay);
	time_info.tm_year = date / 10000 - 1900;
	time_info.tm_mon = (date % 10000) / 100 - 1;
	time_info.tm_mday = date % 100;

	char buf[16] = { 0 };
	strncpy(buf, pDepthMarketData->UpdateTime, sizeof(buf));
	buf[2] = '\0';
	buf[5] = '\0';
	time_info.tm_hour = atoi(&buf[0]);
	time_info.tm_min = atoi(&buf[3]);
	time_info.tm_sec = atoi(&buf[6]);

	time_t utc_sec = mktime(&time_info);
	return (int64_t)utc_sec * 1000 + (int64_t)pDepthMarketData->UpdateMillisec;
}

void CTPQuoteHandler::SubTopics()
{
	char buf[16][64];
	char* topics[16];
	for (int i = 0; i < 16; i++) {
		topics[i] = buf[i];
	}

	std::unique_lock<std::mutex> lock(topic_mtx_);
	int cnt = 0;
	for (auto it = sub_topics_.begin(); it != sub_topics_.end(); ++it) {
		if (it->second == false) {
			memset(buf[cnt], 0, sizeof(buf[cnt]));
			strncpy(buf[cnt++], it->first.c_str(), sizeof(buf[0]) - 1);
			if (cnt == 16) {
				api_->SubscribeMarketData(topics, cnt);
				cnt = 0;
			}
		}
	}

	if (cnt != 0) {
		api_->SubscribeMarketData(topics, cnt);
	}
}