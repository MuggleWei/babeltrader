#include "ctp_trade_handler.h"

#include <thread>

#include "glog/logging.h"

#include "common/serialization.h"
#include "common/utils_func.h"

CTPTradeHandler::CTPTradeHandler(CTPTradeConf &conf)
	: api_(nullptr)
	, api_ready_(false)
	, conf_(conf)
	, req_id_(1)
	, order_ref_(1)
	, order_action_ref_(1)
	, ws_service_(nullptr, this)
	, http_service_(nullptr, this)
	, ctp_front_id_(0)
	, ctp_session_id_(0)
{}

void CTPTradeHandler::run()
{
	// init ctp api
	RunAPI();

	// run service
	RunService();
}

void CTPTradeHandler::InsertOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Value &msg)
{
	CThostFtdcInputOrderField req = { 0 };
	ConvertInsertOrderJson2CTP(msg, req);

	// output
	OutputOrderInsert(&req);

	// record
	Order order;
	ConvertInsertOrderJson2Common(msg, order);
	RecordOrder(order, req.OrderRef, ctp_front_id_, ctp_session_id_);
	
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}
	api_->ReqOrderInsert(&req, req_id_++);
}
void CTPTradeHandler::CancelOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Value &msg)
{
	CThostFtdcInputOrderActionField req = { 0 };
	ConvertCancelOrderJson2CTP(msg, req);

	// output
	OutputOrderAction(&req);

	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}
	api_->ReqOrderAction(&req, req_id_++);
}
void CTPTradeHandler::QueryOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Value &msg)
{
	CThostFtdcQryOrderField req = { 0 };
	ConvertQueryOrderJson2CTP(msg, req);

	// output
	OutputOrderQuery(&req);

	// convert query order json to common
	OrderQuery order_query;
	ConvertQryOrderJson2Common(msg, order_query);
	CacheQryOrder(req_id_, ws, std::move(order_query));

	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}
	api_->ReqQryOrder(&req, req_id_++);
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

	// clear connection information
	ClearConnectionInfo();
}
void CTPTradeHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRsperror(pRspInfo, nRequestID, bIsLast);
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

	// record connection information
	FillConnectionInfo(pRspUserLogin->TradingDay, pRspUserLogin->LoginTime, pRspUserLogin->FrontID, pRspUserLogin->SessionID);

	// confirm settlement
	DoSettlementConfirm();

	api_ready_ = true;
}
void CTPTradeHandler::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspUserLogout(pUserLogout, pRspInfo, nRequestID, bIsLast);

	// clear connection information
	ClearConnectionInfo();
}
void CTPTradeHandler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	
}

void CTPTradeHandler::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspOrderInsert(pInputOrder, pRspInfo, nRequestID, bIsLast);

	// notify
	Order order;
	GetAndCleanRecordOrder(order, pInputOrder->UserID, pInputOrder->OrderRef, ctp_front_id_, ctp_session_id_);
	ConvertInsertOrderCTP2Common(*pInputOrder, order);
	BroadcastOrderConfirm(order, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}
void CTPTradeHandler::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
	// output
	OutputErrRtnOrderInsert(pInputOrder, pRspInfo);
	// use OnRspOrderInsert is enough
}
void CTPTradeHandler::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	// output
	OutputRtnOrder(pOrder);

	// notify
	if (pOrder->OrderSysID[0] == '\0' && pOrder->OrderStatus == THOST_FTDC_OST_Unknown) {
		return;
	}

	Order order;
	OrderStatusNotify order_status;
	bool ret = GetAndCleanRecordOrder(order, pOrder->UserID, pOrder->OrderRef, pOrder->FrontID, pOrder->SessionID);
	ConvertRtnOrderCTP2Common(pOrder, order, order_status);
	if (ret)
	{
		BroadcastOrderConfirm(order, 0, "");
	}
	BroadcastOrderStatus(order, order_status, 0, "");
}
void CTPTradeHandler::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	// output
	OutputRtnTrade(pTrade);

	// notify
	Order order;
	OrderDealNotify order_deal;
	ConvertRtnTradeCTP2Common(pTrade, order, order_deal);
	BroadcastOrderDeal(order, order_deal);
}

void CTPTradeHandler::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspOrderAction(pInputOrderAction, pRspInfo, nRequestID, bIsLast);
}

void CTPTradeHandler::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspOrderQuery(pOrder, pRspInfo, nRequestID, bIsLast);

	auto it = qry_order_caches_.find(nRequestID);
	if (it == qry_order_caches_.end())
	{
		std::vector<CThostFtdcOrderField> vec;
		qry_order_caches_[nRequestID] = vec;
		it = qry_order_caches_.find(nRequestID);
	}
	CThostFtdcOrderField copy_order;
	memcpy(&copy_order, pOrder, sizeof(copy_order));
	it->second.push_back(copy_order);

	if (bIsLast)
	{
		uWS::WebSocket<uWS::SERVER>* ws;
		OrderQuery order_qry;
		GetAndClearCacheQryOrder(nRequestID, ws, order_qry);
		std::vector<CThostFtdcOrderField> &orders = qry_order_caches_[nRequestID];

		if (ws)
		{
			SendRspOrderQry2Client(ws, order_qry, orders, pRspInfo);
		}

		qry_order_caches_.erase(nRequestID);
	}
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
void CTPTradeHandler::DoSettlementConfirm()
{
	CThostFtdcSettlementInfoConfirmField confirm = { 0 };
	strncpy(confirm.BrokerID, conf_.broker_id.c_str(), sizeof(confirm.BrokerID) - 1);
	strncpy(confirm.InvestorID, conf_.user_id.c_str(), sizeof(confirm.InvestorID) - 1);
	api_->ReqSettlementInfoConfirm(&confirm, req_id_++);
}

void CTPTradeHandler::BroadcastOrderConfirm(Order &order, int error_id, const char *error_msg)
{
	// serialize
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("confirmorder");
	writer.Key("error_id");
	writer.Int(error_id);
	// don't response error message, it will lead json loads error in python
	// writer.Key("error_msg");
	// writer.String(error_msg);

	writer.Key("data");
	writer.StartObject();
	SerializeOrder(writer, order);
	writer.EndObject();

	writer.EndObject();

	LOG(INFO) << s.GetString();

	uws_hub_.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void CTPTradeHandler::BroadcastOrderStatus(Order &order, OrderStatusNotify &order_status_notify, int error_id, const char *error_msg)
{
	// serialize
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("orderstatus");
	writer.Key("error_id");
	writer.Int(error_id);
	// don't response error message, it will lead json loads error in python
	// writer.Key("error_msg");
	// writer.String(error_msg);

	writer.Key("data");
	writer.StartObject();

	SerializeOrderStatus(writer, order_status_notify);

	writer.Key("order");
	writer.StartObject();
	SerializeOrder(writer, order);
	writer.EndObject();  // order end

	writer.EndObject();  // data end
	writer.EndObject();  // object end

	LOG(INFO) << s.GetString();

	uws_hub_.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void CTPTradeHandler::BroadcastOrderDeal(Order &order, OrderDealNotify &order_deal)
{
	// serialize
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("orderdeal");
	writer.Key("error_id");
	writer.Int(0);
	// don't response error message, it will lead json loads error in python
	// writer.Key("error_msg");
	// writer.String(error_msg);

	writer.Key("data");
	writer.StartObject();

	SerializeOrderDeal(writer, order_deal);

	writer.Key("order");
	writer.StartObject();
	SerializeOrder(writer, order);
	writer.EndObject();  // order end

	writer.EndObject();  // data end
	writer.EndObject();  // object end

	LOG(INFO) << s.GetString();

	uws_hub_.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void CTPTradeHandler::SendRspOrderQry2Client(uWS::WebSocket<uWS::SERVER>* ws, OrderQuery &order_qry, std::vector<CThostFtdcOrderField> &orders, CThostFtdcRspInfoField *pRspInfo)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rsp_qryorder");
	if (pRspInfo)
	{
		writer.Key("error_id");
		writer.Int(pRspInfo->ErrorID);
	}
	else
	{
		writer.Key("error_id");
		writer.Int(0);
	}

	writer.Key("data");
	writer.StartObject();

	SerializeOrderQuery(writer, order_qry);

	writer.Key("data");
	writer.StartArray();

	for (CThostFtdcOrderField &ctp_order : orders)
	{
		Order order;
		OrderStatusNotify order_status;
		ConvertRtnOrderCTP2Common(&ctp_order, order, order_status);

		writer.StartObject();

		writer.Key("status");
		writer.StartObject();
		SerializeOrderStatus(writer, order_status);
		writer.EndObject();

		writer.Key("order");
		writer.StartObject();
		SerializeOrder(writer, order);
		writer.EndObject();

		writer.EndObject();  // order end
	}

	writer.EndArray();  // orders
	
	writer.EndObject();  // data end

	writer.EndObject();  // object end

	LOG(INFO) << s.GetString();

	ws_service_.SendMsgToClient(ws, s.GetString());
}

void CTPTradeHandler::ConvertInsertOrderJson2CTP(rapidjson::Value &msg, CThostFtdcInputOrderField &req)
{
	// check invalid field
	if (!(msg.HasMember("exchange") && msg["exchange"].IsString())) {
		throw std::runtime_error("field \"exchange\" need string");
	}
	if (!(msg.HasMember("symbol") && msg["symbol"].IsString())) {
		throw std::runtime_error("field \"symbol\" need string");
	}
	if (!(msg.HasMember("contract") && msg["contract"].IsString())) {
		throw std::runtime_error("field \"contract\" need string");
	}
	if (!(msg.HasMember("order_type") && msg["order_type"].IsString())) {
		throw std::runtime_error("field \"order_type\" need string");
	}
	if (!(msg.HasMember("amount") && (msg["amount"].IsInt() || msg["amount"].IsDouble()))) {
		throw std::runtime_error("field \"amount\" need numeric");
	}

	// start convert
	strncpy(req.BrokerID, conf_.broker_id.c_str(), sizeof(req.BrokerID) - 1);
	strncpy(req.InvestorID, conf_.user_id.c_str(), sizeof(req.InvestorID) - 1);
	strncpy(req.ExchangeID, msg["exchange"].GetString(), msg["exchange"].GetStringLength());
	snprintf(req.InstrumentID, sizeof(req.InstrumentID), "%s%s",
		msg["symbol"].GetString(), msg["contract"].GetString());
	strncpy(req.UserID, conf_.user_id.c_str(), sizeof(req.UserID) - 1);

	req.RequestID = req_id_;
	snprintf(req.OrderRef, sizeof(req.OrderRef), "%d", order_ref_);
	++order_ref_;

	req.OrderPriceType = getOrderType(msg["order_type"].GetString());
	if (req.OrderPriceType == (char)0)
	{
		throw std::runtime_error("unsupport order type");
	}

	if (!getOrderDir(msg["dir"].GetString(), req.CombOffsetFlag[0], req.Direction)) {
		throw std::runtime_error("unsupport order dir");
	}

	if (msg.HasMember("order_flag1") && msg["order_flag1"].IsString())
	{
		req.CombHedgeFlag[0] = getOrderFlag1(msg["order_flag1"].GetString());
	}
	else
	{
		req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	}

	if (req.OrderPriceType == THOST_FTDC_OPT_LimitPrice) {
		if (!(msg.HasMember("price") && (msg["price"].IsDouble() || msg["price"].IsInt()))) {
			throw std::runtime_error("field \"price\" need double");
		}

		if (msg["price"].IsDouble())
		{
			req.LimitPrice = msg["price"].GetDouble();
		}
		else
		{
			req.LimitPrice = (double)msg["price"].GetInt();
		}

	}

	if (msg["amount"].IsInt())
	{
		req.VolumeTotalOriginal = msg["amount"].GetInt();
	}
	else if (msg["amount"].IsDouble())
	{
		req.VolumeTotalOriginal = (int)msg["amount"].GetDouble();
	}

	req.TimeCondition = THOST_FTDC_TC_GFD;
	req.VolumeCondition = THOST_FTDC_VC_AV;
	req.MinVolume = 1;
	req.ContingentCondition = THOST_FTDC_CC_Immediately;
	req.StopPrice = 0;
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	req.IsAutoSuspend = 0;
}
void CTPTradeHandler::ConvertCancelOrderJson2CTP(rapidjson::Value &msg, CThostFtdcInputOrderActionField &req)
{
	// check invalid field
	if (!(msg.HasMember("outside_id") && msg["outside_id"].IsString())) {
		throw std::runtime_error("field \"outside_id\" need string");
	}
	if (msg["outside_id"].GetStringLength() == 0)
	{
		throw std::runtime_error("field \"outside_id\" need string");
	}
	if (!(msg.HasMember("exchange") && msg["exchange"].IsString())) {
		throw std::runtime_error("field \"exchange\" need string");
	}
	if (!(msg.HasMember("symbol") && msg["symbol"].IsString())) {
		throw std::runtime_error("field \"symbol\" need string");
	}
	if (!(msg.HasMember("contract") && msg["contract"].IsString())) {
		throw std::runtime_error("field \"contract\" need string");
	}

	// start convert
	strncpy(req.BrokerID, conf_.broker_id.c_str(), sizeof(req.BrokerID) - 1);
	strncpy(req.InvestorID, conf_.user_id.c_str(), sizeof(req.InvestorID) - 1);
	strncpy(req.UserID, conf_.user_id.c_str(), sizeof(req.UserID) - 1);

	req.RequestID = req_id_;
	req.OrderActionRef = order_action_ref_;
	++order_action_ref_;

	GetCTPOrderSysIDFromOutsideId(req.OrderSysID, msg["outside_id"].GetString(), msg["outside_id"].GetStringLength());

	strncpy(req.ExchangeID, msg["exchange"].GetString(), sizeof(req.ExchangeID) - 1);
	snprintf(req.InstrumentID, sizeof(req.InstrumentID) - 1, "%s%s", msg["symbol"].GetString(), msg["contract"].GetString());
	req.ActionFlag = THOST_FTDC_AF_Delete;
}
void CTPTradeHandler::ConvertQueryOrderJson2CTP(rapidjson::Value &msg, CThostFtdcQryOrderField &req)
{
	strncpy(req.BrokerID, conf_.broker_id.c_str(), sizeof(req.BrokerID) - 1);
	strncpy(req.InvestorID, conf_.user_id.c_str(), sizeof(req.InvestorID) - 1);
	if (msg.HasMember("exchange") && msg["exchange"].IsString() && msg["exchange"].GetStringLength() > 0) {
		strncpy(req.ExchangeID, msg["exchange"].GetString(), sizeof(req.ExchangeID) - 1);
	}
	if (msg.HasMember("symbol") && msg["symbol"].IsString() && msg["symbol"].GetStringLength() > 0 &&
		msg.HasMember("contract") && msg["contract"].IsString() && msg["contract"].GetStringLength() > 0) {
		snprintf(req.InstrumentID, sizeof(req.InstrumentID) - 1, "%s%s", msg["symbol"].GetString(), msg["contract"].GetString());
	}
	if (msg.HasMember("outside_id") && msg["outside_id"].IsString() && msg["outside_id"].GetStringLength() > 0) {
		GetCTPOrderSysIDFromOutsideId(req.OrderSysID, msg["outside_id"].GetString(), msg["outside_id"].GetStringLength());
	}
}

void CTPTradeHandler::SerializeCTPInputOrder(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcInputOrderField *pInputOrder)
{
	char buf[2] = { 0 };

	writer.Key("BrokerID");
	writer.String(pInputOrder->BrokerID);

	writer.Key("InvestorID");
	writer.String(pInputOrder->InvestorID);

	writer.Key("InstrumentID");
	writer.String(pInputOrder->InstrumentID);

	writer.Key("OrderRef");
	writer.String(pInputOrder->OrderRef);

	writer.Key("UserID");
	writer.String(pInputOrder->UserID);

	buf[0] = pInputOrder->OrderPriceType;
	writer.Key("OrderPriceType");
	writer.String(buf);

	buf[0] = pInputOrder->Direction;
	writer.Key("Direction");
	writer.String(buf);

	writer.Key("CombOffsetFlag");
	writer.String(pInputOrder->CombOffsetFlag);

	writer.Key("CombHedgeFlag");
	writer.String(pInputOrder->CombHedgeFlag);

	writer.Key("LimitPrice");
	writer.Double(pInputOrder->LimitPrice);

	writer.Key("VolumeTotalOriginal");
	writer.Int(pInputOrder->VolumeTotalOriginal);

	buf[0] = pInputOrder->TimeCondition;
	writer.Key("TimeCondition");
	writer.String(buf);

	writer.Key("GTDDate");
	writer.String(pInputOrder->GTDDate);

	buf[0] = pInputOrder->VolumeCondition;
	writer.Key("VolumeCondition");
	writer.String(buf);

	writer.Key("MinVolume");
	writer.Int(pInputOrder->MinVolume);

	buf[0] = pInputOrder->ContingentCondition;
	writer.Key("ContingentCondition");
	writer.String(buf);

	writer.Key("StopPrice");
	writer.Double(pInputOrder->StopPrice);

	buf[0] = pInputOrder->ForceCloseReason;
	writer.Key("ForceCloseReason");
	writer.String(buf);

	writer.Key("IsAutoSuspend");
	writer.Int(pInputOrder->IsAutoSuspend);

	writer.Key("BusinessUnit");
	writer.String(pInputOrder->BusinessUnit);

	writer.Key("RequestID");
	writer.Int(pInputOrder->RequestID);

	writer.Key("UserForceClose");
	writer.Int(pInputOrder->UserForceClose);

	writer.Key("IsSwapOrder");
	writer.Int(pInputOrder->IsSwapOrder);

	writer.Key("ExchangeID");
	writer.String(pInputOrder->ExchangeID);

	writer.Key("InvestUnitID");
	writer.String(pInputOrder->InvestUnitID);

	writer.Key("AccountID");
	writer.String(pInputOrder->AccountID);

	writer.Key("CurrencyID");
	writer.String(pInputOrder->CurrencyID);

	writer.Key("ClientID");
	writer.String(pInputOrder->ClientID);

	writer.Key("IPAddress");
	writer.String(pInputOrder->IPAddress);

	writer.Key("MacAddress");
	writer.String(pInputOrder->MacAddress);
}
void CTPTradeHandler::SerializeCTPActionOrder(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcInputOrderActionField *pActionOrder)
{
	char buf[2] = { 0 };

	writer.Key("BrokerID");
	writer.String(pActionOrder->BrokerID);

	writer.Key("InvestorID");
	writer.String(pActionOrder->InvestorID);

	writer.Key("OrderActionRef");
	writer.Int(pActionOrder->OrderActionRef);

	writer.Key("OrderRef");
	writer.String(pActionOrder->OrderRef);

	writer.Key("RequestID");
	writer.Int(pActionOrder->RequestID);

	writer.Key("FrontID");
	writer.Int(pActionOrder->FrontID);

	writer.Key("SessionID");
	writer.Int(pActionOrder->SessionID);

	writer.Key("ExchangeID");
	writer.String(pActionOrder->ExchangeID);

	writer.Key("OrderSysID");
	writer.String(pActionOrder->OrderSysID);

	buf[0] = pActionOrder->ActionFlag;
	writer.Key("ActionFlag");
	writer.String(buf);

	writer.Key("LimitPrice");
	writer.Double(pActionOrder->LimitPrice);

	writer.Key("VolumeChange");
	writer.Int(pActionOrder->VolumeChange);

	writer.Key("UserID");
	writer.String(pActionOrder->UserID);

	writer.Key("InstrumentID");
	writer.String(pActionOrder->InstrumentID);

	writer.Key("InvestUnitID");
	writer.String(pActionOrder->InvestUnitID);

	writer.Key("IPAddress");
	writer.String(pActionOrder->IPAddress);

	writer.Key("MacAddress");
	writer.String(pActionOrder->MacAddress);
}
void CTPTradeHandler::SerializeCTPQueryOrder(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcQryOrderField *pQryOrder)
{
	writer.Key("BrokerID");
	writer.String(pQryOrder->BrokerID);

	writer.Key("InvestorID");
	writer.String(pQryOrder->InvestorID);

	writer.Key("InstrumentID");
	writer.String(pQryOrder->InstrumentID);

	writer.Key("ExchangeID");
	writer.String(pQryOrder->ExchangeID);

	writer.Key("OrderSysID");
	writer.String(pQryOrder->OrderSysID);

	writer.Key("InsertTimeStart");
	writer.String(pQryOrder->InsertTimeStart);

	writer.Key("InsertTimeEnd");
	writer.String(pQryOrder->InsertTimeEnd);

	writer.Key("InvestUnitID");
	writer.String(pQryOrder->InvestUnitID);
}
void CTPTradeHandler::SerializeCTPOrder(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcOrderField *pOrder)
{
	char buf[2] = { 0 };

	writer.Key("BrokerID");
	writer.String(pOrder->BrokerID);

	writer.Key("InvestorID");
	writer.String(pOrder->InvestorID);

	writer.Key("InstrumentID");
	writer.String(pOrder->InstrumentID);

	writer.Key("OrderRef");
	writer.String(pOrder->OrderRef);

	writer.Key("UserID");
	writer.String(pOrder->UserID);

	buf[0] = pOrder->OrderPriceType;
	writer.Key("OrderPriceType");
	writer.String(buf);

	buf[0] = pOrder->Direction;
	writer.Key("Direction");
	writer.String(buf);

	writer.Key("CombOffsetFlag");
	writer.String(pOrder->CombOffsetFlag);

	writer.Key("CombHedgeFlag");
	writer.String(pOrder->CombHedgeFlag);

	writer.Key("LimitPrice");
	writer.Double(pOrder->LimitPrice);

	writer.Key("VolumeTotalOriginal");
	writer.Int(pOrder->VolumeTotalOriginal);

	buf[0] = pOrder->TimeCondition;
	writer.Key("TimeCondition");
	writer.String(buf);

	writer.Key("GTDDate");
	writer.String(pOrder->GTDDate);

	buf[0] = pOrder->VolumeCondition;
	writer.Key("VolumeCondition");
	writer.String(buf);

	writer.Key("MinVolume");
	writer.Int(pOrder->MinVolume);

	buf[0] = pOrder->ContingentCondition;
	writer.Key("ContingentCondition");
	writer.String(buf);

	writer.Key("StopPrice");
	writer.Double(pOrder->StopPrice);

	buf[0] = pOrder->ForceCloseReason;
	writer.Key("ForceCloseReason");
	writer.String(buf);

	writer.Key("IsAutoSuspend");
	writer.Int(pOrder->IsAutoSuspend);

	writer.Key("BusinessUnit");
	writer.String(pOrder->BusinessUnit);

	writer.Key("RequestID");
	writer.Int(pOrder->RequestID);

	writer.Key("OrderLocalID");
	writer.String(pOrder->OrderLocalID);

	writer.Key("ExchangeID");
	writer.String(pOrder->ExchangeID);

	writer.Key("ParticipantID");
	writer.String(pOrder->ParticipantID);

	writer.Key("ClientID");
	writer.String(pOrder->ClientID);

	writer.Key("ExchangeInstID");
	writer.String(pOrder->ExchangeInstID);

	writer.Key("TraderID");
	writer.String(pOrder->TraderID);

	buf[0] = pOrder->OrderSubmitStatus;
	writer.Key("OrderSubmitStatus");
	writer.String(buf);

	writer.Key("NotifySequence");
	writer.Int(pOrder->NotifySequence);

	writer.Key("TradingDay");
	writer.String(pOrder->TradingDay);

	writer.Key("SettlementID");
	writer.Int(pOrder->SettlementID);

	writer.Key("OrderSysID");
	writer.String(pOrder->OrderSysID);

	buf[0] = pOrder->OrderSource;
	writer.Key("OrderSource");
	writer.String(buf);

	buf[0] = pOrder->OrderStatus;
	writer.Key("OrderStatus");
	writer.String(buf);

	buf[0] = pOrder->OrderType;
	writer.Key("OrderType");
	writer.String(buf);

	writer.Key("VolumeTraded");
	writer.Int(pOrder->VolumeTraded);

	writer.Key("VolumeTotal");
	writer.Int(pOrder->VolumeTotal);

	writer.Key("InsertDate");
	writer.String(pOrder->InsertDate);

	writer.Key("InsertTime");
	writer.String(pOrder->InsertTime);

	writer.Key("ActiveTime");
	writer.String(pOrder->ActiveTime);

	writer.Key("SuspendTime");
	writer.String(pOrder->SuspendTime);

	writer.Key("UpdateTime");
	writer.String(pOrder->UpdateTime);

	writer.Key("CancelTime");
	writer.String(pOrder->CancelTime);

	writer.Key("ActiveTraderID");
	writer.String(pOrder->ActiveTraderID);

	writer.Key("ClearingPartID");
	writer.String(pOrder->ClearingPartID);

	writer.Key("SequenceNo");
	writer.Int(pOrder->SequenceNo);

	writer.Key("FrontID");
	writer.Int(pOrder->FrontID);

	writer.Key("SessionID");
	writer.Int(pOrder->SessionID);

	writer.Key("UserProductInfo");
	writer.String(pOrder->UserProductInfo);

	writer.Key("StatusMsg");
	writer.String(pOrder->StatusMsg);

	writer.Key("UserForceClose");
	writer.Int(pOrder->UserForceClose);

	writer.Key("ActiveUserID");
	writer.String(pOrder->ActiveUserID);

	writer.Key("BrokerOrderSeq");
	writer.Int(pOrder->BrokerOrderSeq);

	writer.Key("RelativeOrderSysID");
	writer.String(pOrder->RelativeOrderSysID);

	writer.Key("ZCETotalTradedVolume");
	writer.Int(pOrder->ZCETotalTradedVolume);

	writer.Key("IsSwapOrder");
	writer.Int(pOrder->IsSwapOrder);

	writer.Key("BranchID");
	writer.String(pOrder->BranchID);

	writer.Key("InvestUnitID");
	writer.String(pOrder->InvestUnitID);

	writer.Key("AccountID");
	writer.String(pOrder->AccountID);

	writer.Key("CurrencyID");
	writer.String(pOrder->CurrencyID);

	writer.Key("IPAddress");
	writer.String(pOrder->IPAddress);

	writer.Key("MacAddress");
	writer.String(pOrder->MacAddress);
}
void CTPTradeHandler::SerializeCTPTrade(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcTradeField *pTrade)
{
	char buf[2] = { 0 };

	writer.Key("BrokerID");
	writer.String(pTrade->BrokerID);

	writer.Key("InvestorID");
	writer.String(pTrade->InvestorID);

	writer.Key("InstrumentID");
	writer.String(pTrade->InstrumentID);

	writer.Key("OrderRef");
	writer.String(pTrade->OrderRef);

	writer.Key("UserID");
	writer.String(pTrade->UserID);

	writer.Key("ExchangeID");
	writer.String(pTrade->ExchangeID);

	writer.Key("TradeID");
	writer.String(pTrade->TradeID);

	buf[0] = pTrade->Direction;
	writer.Key("Direction");
	writer.String(buf);

	writer.Key("OrderSysID");
	writer.String(pTrade->OrderSysID);

	writer.Key("ParticipantID");
	writer.String(pTrade->ParticipantID);

	writer.Key("ClientID");
	writer.String(pTrade->ClientID);

	buf[0] = pTrade->TradingRole;
	writer.Key("TradingRole");
	writer.String(buf);

	writer.Key("ExchangeInstID");
	writer.String(pTrade->ExchangeInstID);

	buf[0] = pTrade->OffsetFlag;
	writer.Key("OffsetFlag");
	writer.String(buf);

	buf[0] = pTrade->HedgeFlag;
	writer.Key("HedgeFlag");
	writer.String(buf);

	writer.Key("Price");
	writer.Double(pTrade->Price);

	writer.Key("Volume");
	writer.Int(pTrade->Volume);

	writer.Key("TradeDate");
	writer.String(pTrade->TradeDate);

	writer.Key("TradeTime");
	writer.String(pTrade->TradeTime);

	buf[0] = pTrade->TradeType;
	writer.Key("TradeType");
	writer.String(buf);

	buf[0] = pTrade->PriceSource;
	writer.Key("PriceSource");
	writer.String(buf);

	writer.Key("TraderID");
	writer.String(pTrade->TraderID);

	writer.Key("OrderLocalID");
	writer.String(pTrade->OrderLocalID);

	writer.Key("ClearingPartID");
	writer.String(pTrade->ClearingPartID);

	writer.Key("BusinessUnit");
	writer.String(pTrade->BusinessUnit);

	writer.Key("SequenceNo");
	writer.Int(pTrade->SequenceNo);

	writer.Key("TradingDay");
	writer.String(pTrade->TradingDay);

	writer.Key("SettlementID");
	writer.Int(pTrade->SettlementID);

	writer.Key("BrokerOrderSeq");
	writer.Int(pTrade->BrokerOrderSeq);

	buf[0] = pTrade->TradeSource;
	writer.Key("TradeSource");
	writer.String(buf);

	writer.Key("InvestUnitID");
	writer.String(pTrade->InvestUnitID);
}

void CTPTradeHandler::ConvertInsertOrderCTP2Common(CThostFtdcInputOrderField &req, Order &order)
{
	order.market = "ctp";
	order.exchange = req.ExchangeID;
	order.type = "future";
	CTPSplitInstrument(req.InstrumentID, order.symbol, order.contract);
	order.contract_id = order.contract;
	if (req.OrderPriceType == THOST_FTDC_OPT_AnyPrice)
	{
		order.order_type = "market";
	}
	else
	{
		order.order_type = "limit";
	}

	switch (req.CombHedgeFlag[0])
	{
	case THOST_FTDC_HF_Speculation:
	{
		order.order_flag1 = "speculation";
	}break;
	case THOST_FTDC_HF_Arbitrage:
	{
		order.order_flag1 = "arbitrage";
	}break;
	case THOST_FTDC_HF_Hedge:
	{
		order.order_flag1 = "hedge";
	}break;
	case THOST_FTDC_HF_MarketMaker:
	{
		order.order_flag1 = "marketmaker";
	}break;
	}

	if (req.Direction == THOST_FTDC_D_Buy)
	{
		if (req.CombOffsetFlag[0] == THOST_FTDC_OF_Open)
		{
			order.dir = "open_long";
		}
		else if (req.CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday)
		{
			order.dir = "closetoday_short";
		}
		else if (req.CombOffsetFlag[0] == THOST_FTDC_OF_CloseYesterday)
		{
			order.dir = "closeyesterday_short";
		}
		else
		{
			order.dir = "close_short";
		}
	}
	else
	{
		if (req.CombOffsetFlag[0] == THOST_FTDC_OF_Open)
		{
			order.dir = "open_short";
		}
		else if (req.CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday)
		{
			order.dir = "closetoday_long";
		}
		else if (req.CombOffsetFlag[0] == THOST_FTDC_OF_CloseYesterday)
		{
			order.dir = "closeyesterday_long";
		}
		else
		{
			order.dir = "close_long";
		}
	}

	order.price = req.LimitPrice;
	order.amount = req.VolumeTotalOriginal;
	order.total_price = 0;
	order.ts = time(nullptr) * 1000;
}
void CTPTradeHandler::ConvertInsertOrderJson2Common(rapidjson::Value &msg, Order &order)
{
	if (msg.HasMember("user_id") && msg["user_id"].IsString()) {
		order.user_id = msg["user_id"].GetString();
	}

	if (msg.HasMember("order_id") && msg["order_id"].IsString()) {
		order.order_id = msg["order_id"].GetString();
	}

	if (msg.HasMember("client_order_id") && msg["client_order_id"].IsString()) {
		order.client_order_id = msg["client_order_id"].GetString();
	}

	if (msg.HasMember("market") && msg["market"].IsString()) {
		order.market = msg["market"].GetString();
	}

	if (msg.HasMember("exchange") && msg["exchange"].IsString()) {
		order.exchange = msg["exchange"].GetString();
	}

	if (msg.HasMember("type") && msg["type"].IsString()) {
		order.type = msg["type"].GetString();
	}

	if (msg.HasMember("symbol") && msg["symbol"].IsString()) {
		order.symbol = msg["symbol"].GetString();
	}

	if (msg.HasMember("contract") && msg["contract"].IsString()) {
		order.contract = msg["contract"].GetString();
	}

	if (msg.HasMember("contract_id") && msg["contract_id"].IsString()) {
		order.contract_id = msg["contract_id"].GetString();
	}

	if (msg.HasMember("order_type") && msg["order_type"].IsString()) {
		order.order_type = msg["order_type"].GetString();
	}

	if (msg.HasMember("order_flag1") && msg["order_flag1"].IsString()) {
		order.order_flag1 = msg["order_flag1"].GetString();
	}

	if (msg.HasMember("dir") && msg["dir"].IsString()) {
		order.dir = msg["dir"].GetString();
	}

	if (msg.HasMember("price")) {
		if (msg["price"].IsInt())
		{
			order.price = msg["price"].GetInt();
		}
		else if (msg["price"].IsDouble())
		{
			order.price = msg["price"].GetDouble();
		}
	}

	if (msg.HasMember("amount")) {
		if (msg["amount"].IsInt())
		{
			order.amount = msg["amount"].GetInt();
		}
		else if (msg["amount"].IsDouble())
		{
			order.amount = msg["amount"].GetDouble();
		}
	}

	if (msg.HasMember("total_price")) {
		if (msg["total_price"].IsInt())
		{
			order.total_price = msg["total_price"].GetInt();
		}
		else if (msg["total_price"].IsDouble())
		{
			order.total_price = msg["total_price"].GetDouble();
		}
	}

	if (msg.HasMember("ts") && msg["ts"].IsInt64()) {
		order.ts = msg["ts"].GetInt64();
	}
}
void CTPTradeHandler::ConvertRtnOrderCTP2Common(CThostFtdcOrderField *pOrder, Order &order, OrderStatusNotify &order_status_notify)
{
	// order
	{
		order.outside_id = GenOutsideOrderIdFromOrder(pOrder);
		order.market = "ctp";
		order.exchange = pOrder->ExchangeID;
		order.type = "future";
		CTPSplitInstrument(pOrder->InstrumentID, order.symbol, order.contract);
		order.contract_id = order.contract;
		if (pOrder->OrderPriceType == THOST_FTDC_OPT_AnyPrice)
		{
			order.order_type = "market";
		}
		else
		{
			order.order_type = "limit";
		}

		GetOrderDirection(pOrder->Direction, pOrder->CombHedgeFlag[0], pOrder->CombOffsetFlag[0], order);

		order.price = pOrder->LimitPrice;
		order.amount = pOrder->VolumeTotalOriginal;
		order.total_price = 0;

		order.ts = CTPGetTimestamp(pOrder->InsertDate, pOrder->InsertTime, 0);
	}

	// status
	{
		order_status_notify.order_status = (OrderStatusEnum)GetOrderStatus(pOrder->OrderStatus);
		order_status_notify.order_submit_status = (OrderSubmitStatusEnum)GetOrderSubmitStatus(pOrder->OrderSubmitStatus);
		order_status_notify.amount = pOrder->VolumeTotalOriginal;
		order_status_notify.dealed_amount = pOrder->VolumeTraded;
	}
}
void CTPTradeHandler::ConvertRtnTradeCTP2Common(CThostFtdcTradeField *pTrade, Order &order, OrderDealNotify &order_deal)
{
	// order
	{
		order.outside_id = GenOutsideOrderIdFromDeal(pTrade);
		order.market = "ctp";
		order.exchange = pTrade->ExchangeID;
		order.type = "future";
		CTPSplitInstrument(pTrade->InstrumentID, order.symbol, order.contract);
		order.contract_id = order.contract;
		GetOrderDirection(pTrade->Direction, pTrade->HedgeFlag, pTrade->OffsetFlag, order);
	}

	// deal
	{
		order_deal.price = pTrade->Price;
		order_deal.amount = pTrade->Volume;
		order_deal.trading_day = pTrade->TradingDay;
		order_deal.trade_id = GenOutsideTradeIdFromDeal(pTrade);
		order_deal.ts = CTPGetTimestamp(pTrade->TradeDate, pTrade->TradeTime, 0);
	}
}
void CTPTradeHandler::ConvertQryOrderJson2Common(rapidjson::Value &msg, OrderQuery &order_qry)
{
	if (msg.HasMember("qry_id") && msg["qry_id"].IsString()) {
		order_qry.qry_id = msg["qry_id"].GetString();
	}

	if (msg.HasMember("user_id") && msg["user_id"].IsString()) {
		order_qry.user_id = msg["user_id"].GetString();
	}

	if (msg.HasMember("outside_id") && msg["outside_id"].IsString()) {
		order_qry.outside_id = msg["outside_id"].GetString();
	}

	if (msg.HasMember("market") && msg["market"].IsString()) {
		order_qry.market = msg["market"].GetString();
	}

	if (msg.HasMember("exchange") && msg["exchange"].IsString()) {
		order_qry.exchange = msg["exchange"].GetString();
	}

	if (msg.HasMember("type") && msg["type"].IsString()) {
		order_qry.type = msg["type"].GetString();
	}

	if (msg.HasMember("symbol") && msg["symbol"].IsString()) {
		order_qry.symbol = msg["symbol"].GetString();
	}

	if (msg.HasMember("contract") && msg["contract"].IsString()) {
		order_qry.contract = msg["contract"].GetString();
	}

	if (msg.HasMember("contract_id") && msg["contract_id"].IsString()) {
		order_qry.contract_id = msg["contract_id"].GetString();
	}
}

void CTPTradeHandler::OutputOrderInsert(CThostFtdcInputOrderField *req)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_orderinsert");

	writer.Key("data");
	writer.StartObject();
	SerializeCTPInputOrder(writer, req);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputOrderAction(CThostFtdcInputOrderActionField *req)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_orderaction");

	writer.Key("data");
	writer.StartObject();
	SerializeCTPActionOrder(writer, req);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputOrderQuery(CThostFtdcQryOrderField *req)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_orderquery");

	writer.Key("data");
	writer.StartObject();
	SerializeCTPQueryOrder(writer, req);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
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
void CTPTradeHandler::OutputRsperror(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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
void CTPTradeHandler::OutputRspSettlementConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspsettlementconfirm");
	writer.Key("req_id");
	writer.Int(nRequestID);
	writer.Key("error_id");
	writer.Int(pRspInfo->ErrorID);
	writer.Key("error_msg");
	writer.String(pRspInfo->ErrorMsg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("BrokerID");
	writer.String(pSettlementInfoConfirm->BrokerID);
	writer.Key("InvestorID");
	writer.String(pSettlementInfoConfirm->InvestorID);
	writer.Key("ConfirmDate");
	writer.String(pSettlementInfoConfirm->ConfirmDate);
	writer.Key("ConfirmTime");
	writer.String(pSettlementInfoConfirm->ConfirmTime);
	writer.Key("SettlementID");
	writer.Int(pSettlementInfoConfirm->SettlementID);
	writer.Key("AccountID");
	writer.String(pSettlementInfoConfirm->AccountID);
	writer.Key("CurrencyID");
	writer.String(pSettlementInfoConfirm->CurrencyID);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_rsporderinsert");
	writer.Key("req_id");
	writer.Int(nRequestID);
	writer.Key("error_id");
	writer.Int(pRspInfo->ErrorID);
	writer.Key("error_msg");
	writer.String(pRspInfo->ErrorMsg);

	writer.Key("data");
	writer.StartObject();
	SerializeCTPInputOrder(writer, pInputOrder);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_errrtnorderinsert");
	writer.Key("error_id");
	writer.Int(pRspInfo->ErrorID);
	writer.Key("error_msg");
	writer.String(pRspInfo->ErrorMsg);

	writer.Key("data");
	writer.StartObject();
	SerializeCTPInputOrder(writer, pInputOrder);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputRtnOrder(CThostFtdcOrderField *pOrder)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_rtnorder");

	writer.Key("data");
	writer.StartObject();
	SerializeCTPOrder(writer, pOrder);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputRtnTrade(CThostFtdcTradeField *pTrade)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_rtntrade");

	writer.Key("data");
	writer.StartObject();
	SerializeCTPTrade(writer, pTrade);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_rsporderaction");
	writer.Key("req_id");
	writer.Int(nRequestID);
	writer.Key("error_id");
	writer.Int(pRspInfo->ErrorID);
	writer.Key("error_msg");
	writer.String(pRspInfo->ErrorMsg);

	writer.Key("data");
	writer.StartObject();
	SerializeCTPActionOrder(writer, pInputOrderAction);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputRspOrderQuery(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_rspqryorder");
	writer.Key("req_id");
	writer.Int(nRequestID);
	writer.Key("is_last");
	writer.Bool(bIsLast);
	if (pRspInfo)
	{
		writer.Key("error_id");
		writer.Int(pRspInfo->ErrorID);
		writer.Key("error_msg");
		writer.String(pRspInfo->ErrorMsg);
	}
	else
	{
		writer.Key("error_id");
		writer.Int(0);
	}

	writer.Key("data");
	writer.StartObject();
	SerializeCTPOrder(writer, pOrder);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}

char CTPTradeHandler::getOrderType(const char *order_type)
{
	static const char ot_limit[] = "limit";
	static const char ot_market[] = "market";

	if (strncmp(order_type, ot_limit, sizeof(ot_limit) - 1) == 0)
	{
		return THOST_FTDC_OPT_LimitPrice;
	}
	else if (strncmp(order_type, ot_market, sizeof(ot_limit) - 1) == 0)
	{
		return THOST_FTDC_OPT_AnyPrice;
	}

	return (char)0;
}
char CTPTradeHandler::getOrderFlag1(const char *order_flag1)
{
	static const char of_spec[] = "speculation";
	static const char of_hedge[] = "hedge";
	static const char of_arbitrage[] = "arbitrage";

	if (strncmp(order_flag1, of_spec, sizeof(of_spec) - 1) == 0)
	{
		return THOST_FTDC_HF_Speculation;
	}
	else if (strncmp(order_flag1, of_hedge, sizeof(of_hedge) - 1) == 0)
	{
		return THOST_FTDC_HF_Hedge;
	}
	else if (strncmp(order_flag1, of_arbitrage, sizeof(of_arbitrage) - 1) == 0)
	{
		return THOST_FTDC_HF_Arbitrage;
	}

	return (char)0;
}
bool CTPTradeHandler::getOrderDir(const char *order_dir, char& action, char& dir)
{
	static const char od_open[] = "open";
	static const char od_close[] = "close";
	static const char od_closetoday[] = "closetoday";
	static const char od_closeyesterday[] = "closeyesterday";
	static const char od_long[] = "long";
	static const char od_short[] = "short";

	const char *p = order_dir;
	while (p) {
		if (*p != '_') 
		{
			++p;
		}
		else
		{
			break;
		}
	}

	int len = strlen(order_dir);
	int split_pos = p - order_dir;
	if (split_pos == 0 || split_pos == len) {
		return false;
	}

	if (strncmp(order_dir, od_open, split_pos) == 0)
	{
		action = THOST_FTDC_OF_Open;
	}
	else if (strncmp(order_dir, od_close, split_pos) == 0)
	{
		action = THOST_FTDC_OF_Close;
	}
	else if (strncmp(order_dir, od_closetoday, split_pos) == 0)
	{
		action = THOST_FTDC_OF_CloseToday;
	}
	else if (strncmp(order_dir, od_closeyesterday, split_pos) == 0)
	{
		action = THOST_FTDC_OF_CloseYesterday;
	}
	else
	{
		return false;
	}

	if (strncmp(p+1, od_long, sizeof(od_long) - 1) == 0)
	{
		if (action == THOST_FTDC_OF_Open)
		{
			dir = THOST_FTDC_D_Buy;
		}
		else
		{
			dir = THOST_FTDC_D_Sell;
		}
	}
	else if (strncmp(p + 1, od_short, sizeof(od_short) - 1) == 0)
	{
		if (action == THOST_FTDC_OF_Open)
		{
			dir = THOST_FTDC_D_Sell;
		}
		else
		{
			dir = THOST_FTDC_D_Buy;
		}
	}
	else
	{
		return false;
	}

	return true;
}

void CTPTradeHandler::FillConnectionInfo(const char *tradeing_day, const char *login_time, int front_id, int session_id)
{
	ctp_tradeing_day_ = tradeing_day;
	ctp_login_time_ = login_time;
	ctp_front_id_ = front_id;
	ctp_session_id_ = session_id;
}
void CTPTradeHandler::ClearConnectionInfo()
{
	api_ready_ = false;

	ctp_tradeing_day_ = "";
	ctp_login_time_ = "";
	ctp_front_id_ = 0;
	ctp_session_id_ = 0;
}

void CTPTradeHandler::RecordOrder(Order &order, const std::string &order_ref, int front_id, int session_id)
{
	char buf[256] = { 0 };
	snprintf(buf, sizeof(buf), "%s_%s_%d#%d", conf_.user_id.c_str(), order_ref.c_str(), front_id, session_id);
	{
		std::unique_lock<std::mutex> lock(wati_deal_order_mtx_);
		wait_deal_orders_[std::string(buf)] = order;
	}
}
bool CTPTradeHandler::GetRecordOrder(Order &order, const std::string &user_id, const std::string &order_ref, int front_id, int session_id)
{
	char buf[256] = { 0 };
	snprintf(buf, sizeof(buf), "%s_%s_%d#%d", user_id.c_str(), order_ref.c_str(), front_id, session_id);
	{
		std::unique_lock<std::mutex> lock(wati_deal_order_mtx_);
		auto it = wait_deal_orders_.find(std::string(buf));
		if (it != wait_deal_orders_.end())
		{
			order = it->second;
			return true;
		}
		else
		{
			return false;
		}
	}
}
bool CTPTradeHandler::GetAndCleanRecordOrder(Order &order, const std::string &user_id, const std::string &order_ref, int front_id, int session_id)
{
	char buf[256] = { 0 };
	snprintf(buf, sizeof(buf), "%s_%s_%d#%d", user_id.c_str(), order_ref.c_str(), front_id, session_id);
	{
		std::unique_lock<std::mutex> lock(wati_deal_order_mtx_);
		auto it = wait_deal_orders_.find(std::string(buf));
		if (it != wait_deal_orders_.end())
		{
			order = it->second;
			wait_deal_orders_.erase(buf);
			return true;
		}
		else
		{
			return false;
		}
	}
}

int CTPTradeHandler::GetOrderStatus(TThostFtdcOrderStatusType OrderStatus)
{
	int ret = OrderStatus_Unknown;
	switch (OrderStatus)
	{
	case THOST_FTDC_OST_PartTradedQueueing: 
	case THOST_FTDC_OST_PartTradedNotQueueing:
	{
		ret = OrderStatus_PartDealed;
	}break;
	case THOST_FTDC_OST_AllTraded:
	{
		ret = OrderStatus_AllDealed;
	}break;
	case THOST_FTDC_OST_Canceled:
	{
		ret = OrderStatus_Canceled;
	}break;
	}

	return ret;
}
int CTPTradeHandler::GetOrderSubmitStatus(TThostFtdcOrderSubmitStatusType OrderSubmitStatus)
{
	int ret = OrderSubmitStatus_Unknown;

	switch (OrderSubmitStatus)
	{
	case THOST_FTDC_OSS_InsertSubmitted:
	case THOST_FTDC_OSS_CancelSubmitted:
	case THOST_FTDC_OSS_ModifySubmitted:
	{
		ret = OrderSubmitStatus_Submitted;
	}break;
	case THOST_FTDC_OSS_Accepted:
	{
		ret = OrderSubmitStatus_Accepted;
	}break;
	case THOST_FTDC_OSS_InsertRejected:
	case THOST_FTDC_OSS_CancelRejected:
	case THOST_FTDC_OSS_ModifyRejected:
	{
		ret = OrderSubmitStatus_Rejected;
	}break;
	}

	return ret;
}
void CTPTradeHandler::GetOrderDirection(const char ctp_dir, const char ctp_hedge_flag, const char ctp_offset_flag, Order &order)
{
	switch (ctp_hedge_flag)
	{
	case THOST_FTDC_HF_Speculation:
	{
		order.order_flag1 = "speculation";
	}break;
	case THOST_FTDC_HF_Arbitrage:
	{
		order.order_flag1 = "arbitrage";
	}break;
	case THOST_FTDC_HF_Hedge:
	{
		order.order_flag1 = "hedge";
	}break;
	case THOST_FTDC_HF_MarketMaker:
	{
		order.order_flag1 = "marketmaker";
	}break;
	}

	if (ctp_dir == THOST_FTDC_D_Buy)
	{
		if (ctp_offset_flag == THOST_FTDC_OF_Open)
		{
			order.dir = "open_long";
		}
		else if (ctp_offset_flag == THOST_FTDC_OF_CloseToday)
		{
			order.dir = "closetoday_short";
		}
		else if (ctp_offset_flag == THOST_FTDC_OF_CloseYesterday)
		{
			order.dir = "closeyesterday_short";
		}
		else
		{
			order.dir = "close_short";
		}
	}
	else
	{
		if (ctp_offset_flag == THOST_FTDC_OF_Open)
		{
			order.dir = "open_short";
		}
		else if (ctp_offset_flag == THOST_FTDC_OF_CloseToday)
		{
			order.dir = "closetoday_long";
		}
		else if (ctp_offset_flag == THOST_FTDC_OF_CloseYesterday)
		{
			order.dir = "closeyesterday_long";
		}
		else
		{
			order.dir = "close_long";
		}
	}
}
std::string CTPTradeHandler::GenOutsideOrderIdFromOrder(CThostFtdcOrderField *pOrder)
{
	if (pOrder->OrderSysID[0] == '\0')
	{
		return "";
	}

	char buf[256] = { 0 };
	const char *p = pOrder->OrderSysID;
	/*
	while (p) {
		if (*p == ' ') {
			++p;
			continue;
		}
		break;
	}
	*/
	snprintf(buf, sizeof(buf), "%s_%s_%s", pOrder->InvestorID, pOrder->TradingDay, p);
	return std::string(buf);
}
std::string CTPTradeHandler::GenOutsideOrderIdFromDeal(CThostFtdcTradeField *pTrade)
{
	if (pTrade->OrderSysID[0] == '\0')
	{
		return "";
	}

	char buf[256] = { 0 };
	const char *p = pTrade->OrderSysID;
	/*
	while (p) {
		if (*p == ' ') {
			++p;
			continue;
		}
		break;
	}
	*/
	snprintf(buf, sizeof(buf), "%s_%s_%s", pTrade->InvestorID, pTrade->TradingDay, p);
	return std::string(buf);
}
std::string CTPTradeHandler::GenOutsideTradeIdFromDeal(CThostFtdcTradeField *pTrade)
{
	char buf[256] = { 0 };
	const char *p = pTrade->TradeID;
	/*
	while (p)
	{
		if (*p == ' ') {
			p++;
			continue;
		}
		break;
	}
	*/
	snprintf(buf, sizeof(buf), "%s_%s_%s", pTrade->InvestorID, pTrade->TradingDay, p);
	return std::string(buf);
}
bool CTPTradeHandler::GetCTPOrderSysIDFromOutsideId(TThostFtdcOrderSysIDType &ctp_order_sys_id, const char *outside_id, int len)
{
	const char *p = outside_id + len - 1;
	int order_sys_id_len = 0;
	for (order_sys_id_len = 0; order_sys_id_len <len; order_sys_id_len++)
	{
		if (*(p - order_sys_id_len) == '_')
		{
			break;
		}
	}

	if (order_sys_id_len >= sizeof(ctp_order_sys_id) || order_sys_id_len == 0)
	{
		return false;
	}

	strncpy(ctp_order_sys_id, p - order_sys_id_len + 1, order_sys_id_len);
	return true;
}

std::string CTPTradeHandler::GetQryId(rapidjson::Value &msg)
{
	if (!(msg.HasMember("qry_id") && msg["qry_id"].IsString())) {
		return "";
	}
	return msg["qry_id"].GetString();
}

void CTPTradeHandler::CacheQryOrder(int req_id, uWS::WebSocket<uWS::SERVER>* ws, OrderQuery &&order_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	qry_ws_cache_[req_id] = ws;
	qry_info_cache_[req_id] = std::move(order_qry);
}
void CTPTradeHandler::GetAndClearCacheQryOrder(int req_id, uWS::WebSocket<uWS::SERVER>*& ws, OrderQuery &order_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	auto it_ws = qry_ws_cache_.find(req_id);
	if (it_ws != qry_ws_cache_.end())
	{
		ws = it_ws->second;
		qry_ws_cache_.erase(it_ws);
	}

	auto it_order_qry = qry_info_cache_.find(req_id);
	if (it_order_qry != qry_info_cache_.end())
	{
		order_qry = it_order_qry->second;
		qry_info_cache_.erase(it_order_qry);
	}
}