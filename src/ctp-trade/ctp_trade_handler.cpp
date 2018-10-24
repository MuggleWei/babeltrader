#include "ctp_trade_handler.h"

#include <thread>

#include "glog/logging.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "common/serialization.h"

CTPTradeHandler::CTPTradeHandler(CTPTradeConf &conf)
	: api_(nullptr)
	, conf_(conf)
	, req_id_(1)
	, ws_service_(nullptr, this)
	, http_service_(nullptr, this)
{}

void CTPTradeHandler::run()
{
	// init ctp api
	RunAPI();

	// run service
	RunService();
}

void CTPTradeHandler::InsertOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	// TODO:
}
void CTPTradeHandler::CancelOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	// TODO:
}

void CTPTradeHandler::OnFrontConnected()
{
	// output
	OutputFrontConnected();

	// auth
	DoAuthenticate();
}
void CTPTradeHandler::OnFrontDisconnected(int nReason)
{
	// output
	OutputFrontDisconnected(nReason);
}
void CTPTradeHandler::OnHeartBeatWarning(int nTimeLapse)
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

void CTPTradeHandler::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputAuthenticate(pRspAuthenticateField, pRspInfo, nRequestID, bIsLast);

	// login
	DoLogin();
}
void CTPTradeHandler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspUserLogin(pRspUserLogin, pRspInfo, nRequestID, bIsLast);
}
void CTPTradeHandler::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspUserLogout(pUserLogout, pRspInfo, nRequestID, bIsLast);
}

void CTPTradeHandler::RunAPI()
{
	api_ = CThostFtdcTraderApi::CreateFtdcTraderApi("./");
	LOG(INFO) << "CTP trade API version:" << api_->GetApiVersion();

	api_->RegisterSpi(this);
	api_->SubscribePrivateTopic(THOST_TERT_QUICK);
	api_->SubscribePublicTopic(THOST_TERT_QUICK);

	char addr[256] = { 0 };
	strncpy(addr, conf_.addr.c_str(), sizeof(addr) - 1);
	api_->RegisterFront(addr);

	api_->Init();
}
void CTPTradeHandler::RunService()
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

		if (!uws_hub_.listen(conf_.trade_ip.c_str(), conf_.trade_port, nullptr, uS::ListenOptions::REUSE_PORT)) {
			LOG(INFO) << "Failed to listen";
			exit(-1);
		}
		uws_hub_.run();
	});

	loop_thread.join();
}

void CTPTradeHandler::DoAuthenticate()
{
	CThostFtdcReqAuthenticateField auth = { 0 };
	strncpy(auth.BrokerID, conf_.broker_id.c_str(), sizeof(auth.BrokerID) - 1);
	strncpy(auth.UserID, conf_.user_id.c_str(), sizeof(auth.UserID) - 1);
	strncpy(auth.UserProductInfo, conf_.product_info.c_str(), sizeof(auth.UserProductInfo) - 1);
	strncpy(auth.AuthCode, conf_.auth_code.c_str(), sizeof(auth.AuthCode) - 1);
	api_->ReqAuthenticate(&auth, req_id_++);
}
void CTPTradeHandler::DoLogin()
{
	CThostFtdcReqUserLoginField req_user_login = { 0 };
	strncpy(req_user_login.BrokerID, conf_.broker_id.c_str(), sizeof(req_user_login.BrokerID) - 1);
	strncpy(req_user_login.UserID, conf_.user_id.c_str(), sizeof(req_user_login.UserID) - 1);
	strncpy(req_user_login.Password, conf_.password.c_str(), sizeof(req_user_login.Password) - 1);
	// api_->ReqUserLogin(&req_user_login, req_id_++);
	api_->ReqUserLogin2(&req_user_login, req_id_++);
}

void CTPTradeHandler::OutputFrontConnected()
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("connected");
	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputFrontDisconnected(int reason)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("disconnected");
	writer.Key("data");
	writer.StartObject();
	writer.Key("reason");
	writer.Int(reason);
	writer.EndObject();
	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("auth");
	writer.Key("req_id");
	writer.Int(nRequestID);
	writer.Key("error_id");
	writer.Int(pRspInfo->ErrorID);
	writer.Key("error_msg");
	writer.String(pRspInfo->ErrorMsg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("BrokerID");
	writer.String(pRspAuthenticateField->BrokerID);
	writer.Key("UserID");
	writer.String(pRspAuthenticateField->UserID);
	writer.Key("UserProductInfo");
	writer.String(pRspAuthenticateField->UserProductInfo);
	writer.EndObject(); // data

	writer.EndObject(); // object
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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
void CTPTradeHandler::OutputRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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
