#include "ctp_quote_handler.h"

#include <iostream>
#include <thread>

#include "glog/logging.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

void runService(uWS::Hub &h, const std::string &ip, int port)
{
	WsService ws_service;
	HttpService http_service;

	// ws
	h.onConnection([&ws_service](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
		ws_service.onConnection(ws, req);
	});
	h.onMessage([&ws_service](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
		ws_service.onMessage(ws, message, length, opCode);
	});
	h.onDisconnection([&ws_service](uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {
	});

	// rest
	h.onHttpRequest([&http_service](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t length, size_t remainingBytes) {
		http_service.onMessage(res, req, data, length, remainingBytes);
	});

	if (!h.listen(ip.c_str(), port, nullptr, uS::ListenOptions::REUSE_PORT)) {
		LOG(INFO) << "Failed to listen" << std::endl;
		exit(-1);
	}
	h.run();
}

CTPQuoteHandler::CTPQuoteHandler(CTPQuoteConf &conf)
	: api_(nullptr)
	, conf_(conf)
	, req_id_(1)
{}

void CTPQuoteHandler::run()
{
	// init ctp api
	RunAPI();

	// run service
	RunService();

//	auto broadcast_thread = std::thread([&h] {
//		rapidjson::StringBuffer s;
//		rapidjson::Writer<rapidjson::StringBuffer> writer(s);
//
//		writer.StartObject();
//		writer.Key("msg");
//		writer.String("notify");
//
//		writer.Key("data");
//		writer.StartObject();
//		writer.Key("msg");
//		writer.String("it's uWebSockets");
//		writer.EndObject();
//
//		writer.EndObject();
//
//		std::string msg = s.GetString();
//		while (true) {
//			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//			std::cout << msg << std::endl;
//			h.getDefaultGroup<uWS::SERVER>().broadcast(msg.c_str(), msg.size(), uWS::OpCode::TEXT);
//		}
//	});
}

void CTPQuoteHandler::OnFrontConnected()
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("connected");
	writer.EndObject();
	LOG(INFO) << s.GetString();

	// user login
	CThostFtdcReqUserLoginField req_user_login = { 0 };
	strncpy(req_user_login.BrokerID, conf_.broker_id.c_str(), sizeof(req_user_login.BrokerID) - 1);
	strncpy(req_user_login.UserID, conf_.user_id.c_str(), sizeof(req_user_login.UserID) - 1);
	strncpy(req_user_login.Password, conf_.password.c_str(), sizeof(req_user_login.Password) - 1);
	api_->ReqUserLogin(&req_user_login, req_id_++);
}
void CTPQuoteHandler::OnFrontDisconnected(int nReason)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("disconnected");
	writer.EndObject();
	LOG(INFO) << s.GetString();

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

	// sub default topics
	char buf[16][64];
	char* topics[16];
	for (int i = 0; i < 16; i++) {
		topics[i] = buf[i];
	}

	int cnt = 0;
	for (auto topic : conf_.default_sub_topics) {
		memset(buf[cnt], 0, sizeof(buf[cnt]));
		strncpy(buf[cnt++], topic.c_str(), sizeof(buf[0]) - 1);
		if (cnt == 16) {
			api_->SubscribeMarketData(topics, cnt);
			cnt = 0;
		}
	}

	if (cnt != 0) {
		api_->SubscribeMarketData(topics, cnt);
	}
}
void CTPQuoteHandler::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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
void CTPQuoteHandler::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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

void CTPQuoteHandler::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}
void CTPQuoteHandler::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CTPQuoteHandler::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
#ifndef NDEBUG
	OutputMarketData(pDepthMarketData);
#endif

	std::string json_str = SerializeMarketData(pDepthMarketData);

	// TODO: broadcast to all clients
	LOG(INFO) << json_str;
}
void CTPQuoteHandler::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) {}


void CTPQuoteHandler::RunAPI()
{
	api_ = CThostFtdcMdApi::CreateFtdcMdApi();
	LOG(INFO) << "CTP quotes API version:" << api_->GetApiVersion() << std::endl;

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
			LOG(INFO) << "Failed to listen" << std::endl;
			exit(-1);
		}
		uws_hub_.run();
	});

	loop_thread.join();
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
std::string CTPQuoteHandler::SerializeMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	// split instrument into symbol and contract
	std::string symbol, contract;
	SplitInstrument(pDepthMarketData->InstrumentID, symbol, contract);

	// get update time
	int64_t ts =  GetUpdateTimeMs(pDepthMarketData);

	// serialize
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("quote");
	
	{
		// quote data
		writer.Key("data");
		writer.StartObject();
		writer.Key("market");
		writer.String("CTP");
		writer.Key("exchange_id");
		writer.String(pDepthMarketData->ExchangeID);
		writer.Key("type");
		writer.String("future");
		writer.Key("symbol");
		writer.String(symbol.c_str());
		writer.Key("contract");
		writer.String(contract.c_str());
		writer.Key("contract_id");
		writer.String(contract.c_str());
		writer.Key("info1");
		writer.String("marketdata");

		{
			// inner data
			writer.Key("data");
			writer.StartObject();
			writer.Key("last");
			writer.Double(pDepthMarketData->LastPrice);
			writer.Key("vol");
			writer.Int(pDepthMarketData->Volume);
			writer.Key("ts");
			writer.Int64(ts);

			writer.Key("bids");
			writer.StartArray();
			writer.StartArray();
			writer.Double(pDepthMarketData->BidPrice1);
			writer.Int(pDepthMarketData->BidVolume1);
			writer.EndArray();
			writer.StartArray();
			writer.Double(pDepthMarketData->BidPrice2);
			writer.Int(pDepthMarketData->BidVolume2);
			writer.EndArray();
			writer.StartArray();
			writer.Double(pDepthMarketData->BidPrice3);
			writer.Int(pDepthMarketData->BidVolume3);
			writer.EndArray();
			writer.StartArray();
			writer.Double(pDepthMarketData->BidPrice4);
			writer.Int(pDepthMarketData->BidVolume4);
			writer.EndArray();
			writer.StartArray();
			writer.Double(pDepthMarketData->BidPrice5);
			writer.Int(pDepthMarketData->BidVolume5);
			writer.EndArray();
			writer.EndArray();  // bids

			writer.Key("asks");
			writer.StartArray();
			writer.StartArray();
			writer.Double(pDepthMarketData->AskPrice1);
			writer.Int(pDepthMarketData->AskVolume1);
			writer.EndArray();
			writer.StartArray();
			writer.Double(pDepthMarketData->AskPrice2);
			writer.Int(pDepthMarketData->AskVolume2);
			writer.EndArray();
			writer.StartArray();
			writer.Double(pDepthMarketData->AskPrice3);
			writer.Int(pDepthMarketData->AskVolume3);
			writer.EndArray();
			writer.StartArray();
			writer.Double(pDepthMarketData->AskPrice4);
			writer.Int(pDepthMarketData->AskVolume4);
			writer.EndArray();
			writer.StartArray();
			writer.Double(pDepthMarketData->AskPrice5);
			writer.Int(pDepthMarketData->AskVolume5);
			writer.EndArray();
			writer.EndArray();  // bids

			writer.EndObject(); // inner data
		}

		writer.EndObject(); // quote data
	}
	

	writer.EndObject(); // object

	return std::string(s.GetString());
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