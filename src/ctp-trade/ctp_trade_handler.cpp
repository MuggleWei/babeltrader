#include "ctp_trade_handler.h"

#include <thread>

#include "glog/logging.h"

#include "common/converter.h"
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

void CTPTradeHandler::InsertOrder(uWS::WebSocket<uWS::SERVER> *ws, Order &order)
{
	// convert to ctp struct
	CThostFtdcInputOrderField req = { 0 };
	ConvertInsertOrderCommon2CTP(order, req);

	// log output
	OutputOrderInsert(&req);

	// record order
	RecordOrder(order, req.OrderRef, ctp_front_id_, ctp_session_id_);
	
	// send req
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}
	api_->ReqOrderInsert(&req, req_id_++);
}
void CTPTradeHandler::CancelOrder(uWS::WebSocket<uWS::SERVER> *ws, Order &order)
{
	CThostFtdcInputOrderActionField req = { 0 };
	ConvertCancelOrderCommon2CTP(order, req);

	// output
	OutputOrderAction(&req);

	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}
	api_->ReqOrderAction(&req, req_id_++);
}
void CTPTradeHandler::QueryOrder(uWS::WebSocket<uWS::SERVER> *ws, OrderQuery &order_query)
{
	CThostFtdcQryOrderField req = { 0 };
	ConvertQueryOrderCommon2CTP(order_query, req);	

	// output
	OutputOrderQuery(&req);

	// cache query order
	CacheQryOrder(req_id_, ws, order_query);

	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}
	api_->ReqQryOrder(&req, req_id_++);
}
void CTPTradeHandler::QueryTrade(uWS::WebSocket<uWS::SERVER> *ws, TradeQuery &trade_query)
{
	CThostFtdcQryTradeField req = { 0 };
	ConvertQueryTradeCommon2CTP(trade_query, req);

	// output
	OutputTradeQuery(&req);

	// cache query order
	CacheQryTrade(req_id_, ws, trade_query);

	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}
	api_->ReqQryTrade(&req, req_id_++);
}
void CTPTradeHandler::QueryPosition(uWS::WebSocket<uWS::SERVER> *ws, PositionQuery &position_query)
{
	CThostFtdcQryInvestorPositionField req = { 0 };
	ConvertQueryPositionCommon2CTP(position_query, req);

	// output
	OutputPositionQuery(&req);

	// cache query position
	CacheQryPosition(req_id_, ws, position_query);

	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}
	api_->ReqQryInvestorPosition(&req, req_id_++);
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

	ctp_tradeing_day_ = pRspUserLogin->TradingDay;
	ctp_login_time_ = pRspUserLogin->LoginTime;
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

	ws_service_.BroadcastConfirmOrder(uws_hub_, order, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
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
		ws_service_.BroadcastConfirmOrder(uws_hub_, order, 0, "");
	}
	ws_service_.BroadcastOrderStatus(uws_hub_, order, order_status, 0, "");
}
void CTPTradeHandler::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	// output
	OutputRtnTrade(pTrade);

	// notify
	Order order;
	OrderDealNotify order_deal;
	ConvertRtnTradeCTP2Common(pTrade, order, order_deal);

	ws_service_.BroadcastOrderDeal(uws_hub_, order, order_deal);
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

	auto it = rsp_qry_order_caches_.find(nRequestID);
	if (it == rsp_qry_order_caches_.end())
	{
		std::vector<CThostFtdcOrderField> vec;
		rsp_qry_order_caches_[nRequestID] = std::move(vec);
		it = rsp_qry_order_caches_.find(nRequestID);
	}
	if (pOrder)
	{
		CThostFtdcOrderField copy_order;
		memcpy(&copy_order, pOrder, sizeof(copy_order));
		it->second.push_back(copy_order);
	}

	if (bIsLast)
	{
		uWS::WebSocket<uWS::SERVER>* ws;
		OrderQuery order_qry;
		GetAndClearCacheQryOrder(nRequestID, ws, order_qry);
		std::vector<CThostFtdcOrderField> &orders = rsp_qry_order_caches_[nRequestID];

		if (ws)
		{
			std::vector<Order> common_orders;
			std::vector<OrderStatusNotify> common_order_status;
			for (CThostFtdcOrderField &ctp_order : orders)
			{
				Order order;
				OrderStatusNotify order_status;
				ConvertRtnOrderCTP2Common(&ctp_order, order, order_status);
				common_orders.push_back(std::move(order));
				common_order_status.push_back(std::move(order_status));
			}

			int error_id = 0;
			if (pRspInfo) {
				error_id = pRspInfo->ErrorID;
			}
			ws_service_.RspOrderQry(ws, order_qry, common_orders, common_order_status, error_id);
		}

		rsp_qry_order_caches_.erase(nRequestID);
	}
}
void CTPTradeHandler::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspTradeQuery(pTrade, pRspInfo, nRequestID, bIsLast);

	auto it = rsp_qry_trade_caches_.find(nRequestID);
	if (it == rsp_qry_trade_caches_.end())
	{
		std::vector<CThostFtdcTradeField> vec;
		rsp_qry_trade_caches_[nRequestID] = std::move(vec);
		it = rsp_qry_trade_caches_.find(nRequestID);
	}
	if (pTrade)
	{
		CThostFtdcTradeField copy_trade;
		memcpy(&copy_trade, pTrade, sizeof(copy_trade));
		it->second.push_back(copy_trade);
	}

	if (bIsLast)
	{
		uWS::WebSocket<uWS::SERVER>* ws;
		TradeQuery trade_qry;
		GetAndClearCacheQryTrade(nRequestID, ws, trade_qry);
		std::vector<CThostFtdcTradeField> &trades = rsp_qry_trade_caches_[nRequestID];

		if (ws)
		{
			std::vector<Order> common_orders;
			std::vector<OrderDealNotify> common_deals;
			for (CThostFtdcTradeField &ctp_trade : trades)
			{
				Order order;
				OrderDealNotify order_deal;
				ConvertRtnTradeCTP2Common(&ctp_trade, order, order_deal);
				common_orders.push_back(std::move(order));
				common_deals.push_back(std::move(order_deal));
			}

			int error_id = 0;
			if (pRspInfo) {
				error_id = pRspInfo->ErrorID;
			}
			ws_service_.RspTradeQry(ws, trade_qry, common_orders, common_deals, error_id);
		}

		rsp_qry_trade_caches_.erase(nRequestID);
	}
}
void CTPTradeHandler::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspPositionQuery(pInvestorPosition, pRspInfo, nRequestID, bIsLast);

	auto it = rsp_qry_position_caches_.find(nRequestID);
	if (it == rsp_qry_position_caches_.end())
	{
		std::vector<CThostFtdcInvestorPositionField> vec;
		rsp_qry_position_caches_[nRequestID] = std::move(vec);
		it = rsp_qry_position_caches_.find(nRequestID);
	}
	if (pInvestorPosition)
	{
		CThostFtdcInvestorPositionField copy_position;
		memcpy(&copy_position, pInvestorPosition, sizeof(copy_position));
		it->second.push_back(copy_position);
	}

	if (bIsLast)
	{
		uWS::WebSocket<uWS::SERVER>* ws;
		PositionQuery position_qry;
		GetAndCleanCacheQryPosition(nRequestID, ws, position_qry);
		std::vector<CThostFtdcInvestorPositionField> &ctp_positions = rsp_qry_position_caches_[nRequestID];

		if (ws)
		{
			std::vector<PositionSummaryType1> positions;
			for (CThostFtdcInvestorPositionField &ctp_position : ctp_positions)
			{
				PositionSummaryType1 position;
				ConvertPositionCTP2Common(&ctp_position, position);
				positions.push_back(std::move(position));
			}

			int error_id = 0;
			if (pRspInfo) {
				error_id = pRspInfo->ErrorID;
			}
			ws_service_.RspPositionQryType1(ws, position_qry, positions, error_id);
		}

		rsp_qry_position_caches_.erase(nRequestID);
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

void CTPTradeHandler::ConvertInsertOrderCommon2CTP(Order &order, CThostFtdcInputOrderField &req)
{
	// check invalid field
	if (order.exchange.empty()) {
		throw std::runtime_error("field \"exchange\" need string");
	}
	if (order.symbol.empty()) {
		throw std::runtime_error("field \"symbol\" need string");
	}
	if (order.contract.empty()) {
		throw std::runtime_error("field \"contract\" need string");
	}
	if (order.order_type.empty()) {
		throw std::runtime_error("field \"order_type\" need string");
	}
	if (order.amount <= 0) {
		throw std::runtime_error("field \"amount\" need numeric");
	}

	// convert
	req.RequestID = req_id_;
	snprintf(req.OrderRef, sizeof(req.OrderRef), "%d", order_ref_++);

	strncpy(req.BrokerID, conf_.broker_id.c_str(), sizeof(req.BrokerID) - 1);
	strncpy(req.InvestorID, conf_.user_id.c_str(), sizeof(req.InvestorID) - 1);
	strncpy(req.UserID, conf_.user_id.c_str(), sizeof(req.UserID) - 1);
	strncpy(req.ExchangeID, order.exchange.c_str(), order.exchange.length());
	snprintf(req.InstrumentID, sizeof(req.InstrumentID), "%s%s",
		order.symbol.c_str(), order.contract.c_str());

	req.OrderPriceType = ConvertOrderTypeCommon2CTP(order.order_type.c_str());
	if (req.OrderPriceType == (char)0)
	{
		throw std::runtime_error("unsupport order type");
	}

	if (!ConvertOrderDirCommon2CTP(order.dir.c_str(), req.CombOffsetFlag[0], req.Direction)) {
		throw std::runtime_error("unsupport order dir");
	}

	if (!order.order_flag1.empty())
	{
		req.CombHedgeFlag[0] = ConvertOrderFlag1Common2CTP(order.order_flag1.c_str());
	}
	else
	{
		req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	}

	req.LimitPrice = order.price;
	req.VolumeTotalOriginal = (TThostFtdcVolumeType)order.amount;

	req.TimeCondition = THOST_FTDC_TC_GFD;
	req.VolumeCondition = THOST_FTDC_VC_AV;
	req.MinVolume = 1;
	req.ContingentCondition = THOST_FTDC_CC_Immediately;
	req.StopPrice = 0;
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	req.IsAutoSuspend = 0;
}
void CTPTradeHandler::ConvertCancelOrderCommon2CTP(Order &order, CThostFtdcInputOrderActionField &req)
{
	// check invalid field
	if (order.outside_id.empty()) {
		throw std::runtime_error("field \"outside_id\" need string");
	}
	if (order.exchange.empty()) {
		throw std::runtime_error("field \"exchange\" need string");
	}
	if (order.symbol.empty()) {
		throw std::runtime_error("field \"symbol\" need string");
	}
	if (order.contract.empty()) {
		throw std::runtime_error("field \"contract\" need string");
	}

	// convert
	req.RequestID = req_id_;
	req.OrderActionRef = order_action_ref_++;

	strncpy(req.BrokerID, conf_.broker_id.c_str(), sizeof(req.BrokerID) - 1);
	strncpy(req.InvestorID, conf_.user_id.c_str(), sizeof(req.InvestorID) - 1);
	strncpy(req.UserID, conf_.user_id.c_str(), sizeof(req.UserID) - 1);

	GetCTPIdFromExtend(order.outside_id.c_str(), order.outside_id.length(), req.OrderSysID, sizeof(req.OrderSysID));

	strncpy(req.ExchangeID, order.exchange.c_str(), sizeof(req.ExchangeID) - 1);
	snprintf(req.InstrumentID, sizeof(req.InstrumentID) - 1, "%s%s", order.symbol.c_str(), order.contract.c_str());
	req.ActionFlag = THOST_FTDC_AF_Delete;
}
void CTPTradeHandler::ConvertQueryOrderCommon2CTP(OrderQuery &order_qry, CThostFtdcQryOrderField &req)
{
	strncpy(req.BrokerID, conf_.broker_id.c_str(), sizeof(req.BrokerID) - 1);
	strncpy(req.InvestorID, conf_.user_id.c_str(), sizeof(req.InvestorID) - 1);
	if (!order_qry.exchange.empty()) {
		strncpy(req.ExchangeID, order_qry.exchange.c_str(), sizeof(req.ExchangeID) - 1);
	}
	if (!order_qry.symbol.empty() && !order_qry.contract.empty()) {
		snprintf(req.InstrumentID, sizeof(req.InstrumentID) - 1, "%s%s", order_qry.symbol.c_str(), order_qry.contract.c_str());
	}
	if (!order_qry.outside_id.empty()) {
		GetCTPIdFromExtend(order_qry.outside_id.c_str(), order_qry.outside_id.length(), req.OrderSysID, sizeof(req.OrderSysID));
	}
}
void CTPTradeHandler::ConvertQueryTradeCommon2CTP(TradeQuery &trade_qry, CThostFtdcQryTradeField &req)
{
	strncpy(req.BrokerID, conf_.broker_id.c_str(), sizeof(req.BrokerID) - 1);
	strncpy(req.InvestorID, conf_.user_id.c_str(), sizeof(req.InvestorID) - 1);
	if (!trade_qry.exchange.empty()) {
		strncpy(req.ExchangeID, trade_qry.exchange.c_str(), sizeof(req.ExchangeID) - 1);
	}
	if (!trade_qry.symbol.empty() && !trade_qry.contract.empty()) {
		snprintf(req.InstrumentID, sizeof(req.InstrumentID) - 1, "%s%s", trade_qry.symbol.c_str(), trade_qry.contract.c_str());
	}
	if (!trade_qry.trade_id.empty()) {
		GetCTPIdFromExtend(trade_qry.trade_id.c_str(), trade_qry.trade_id.length(), req.TradeID, sizeof(req.TradeID));
	}
}
void CTPTradeHandler::ConvertQueryPositionCommon2CTP(PositionQuery &position_qry, CThostFtdcQryInvestorPositionField &req)
{
	strncpy(req.BrokerID, conf_.broker_id.c_str(), sizeof(req.BrokerID) - 1);
	strncpy(req.InvestorID, conf_.user_id.c_str(), sizeof(req.InvestorID) - 1);
	if (!position_qry.exchange.empty()) {
		strncpy(req.ExchangeID, position_qry.exchange.c_str(), sizeof(req.ExchangeID) - 1);
	}
	if (!position_qry.symbol.empty() && !position_qry.contract.empty()) {
		snprintf(req.InstrumentID, sizeof(req.InstrumentID) - 1, "%s%s", position_qry.symbol.c_str(), position_qry.contract.c_str());
	}
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
			order.dir = "closehistory_short";
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
			order.dir = "closehistory_long";
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
void CTPTradeHandler::ConvertRtnOrderCTP2Common(CThostFtdcOrderField *pOrder, Order &order, OrderStatusNotify &order_status_notify)
{
	// order
	{
		order.outside_id = ExtendCTPId(pOrder->InvestorID, pOrder->TradingDay, pOrder->OrderSysID);
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

		ConvertOrderDirCTP2Common(pOrder->Direction, pOrder->CombHedgeFlag[0], pOrder->CombOffsetFlag[0], order);

		order.price = pOrder->LimitPrice;
		order.amount = pOrder->VolumeTotalOriginal;
		order.total_price = 0;

		order.ts = CTPGetTimestamp(pOrder->InsertDate, pOrder->InsertTime, 0);
	}

	// status
	{
		order_status_notify.order_status = (OrderStatusEnum)ConvertOrderStatusCTP2Common(pOrder->OrderStatus);
		order_status_notify.order_submit_status = (OrderSubmitStatusEnum)ConvertOrderSubmitStatusCTP2Common(pOrder->OrderSubmitStatus);
		order_status_notify.amount = pOrder->VolumeTotalOriginal;
		order_status_notify.dealed_amount = pOrder->VolumeTraded;
	}
}
void CTPTradeHandler::ConvertRtnTradeCTP2Common(CThostFtdcTradeField *pTrade, Order &order, OrderDealNotify &order_deal)
{
	// order
	{
		order.outside_id = ExtendCTPId(pTrade->InvestorID, pTrade->TradingDay, pTrade->OrderSysID);
		order.market = "ctp";
		order.exchange = pTrade->ExchangeID;
		order.type = "future";
		CTPSplitInstrument(pTrade->InstrumentID, order.symbol, order.contract);
		order.contract_id = order.contract;
		ConvertOrderDirCTP2Common(pTrade->Direction, pTrade->HedgeFlag, pTrade->OffsetFlag, order);
	}

	// deal
	{
		order_deal.price = pTrade->Price;
		order_deal.amount = pTrade->Volume;
		order_deal.trading_day = pTrade->TradingDay;
		order_deal.trade_id = ExtendCTPId(pTrade->InvestorID, pTrade->TradingDay, pTrade->TradeID);
		order_deal.ts = CTPGetTimestamp(pTrade->TradeDate, pTrade->TradeTime, 0);
	}
}
void CTPTradeHandler::ConvertPositionCTP2Common(CThostFtdcInvestorPositionField *pPosition, PositionSummaryType1 &position_summary)
{
	position_summary.market = "ctp";
	position_summary.exchange = pPosition->ExchangeID;
	CTPSplitInstrument(pPosition->InstrumentID, position_summary.symbol, position_summary.contract);
	position_summary.dir = ConvertPositionDirCTP2Common(pPosition->PosiDirection);
	position_summary.order_flag1 = ConvertHedgeFlagCTP2Common(pPosition->HedgeFlag);
	position_summary.date_type = ConvertDateTypeCTP2Common(pPosition->PositionDate);
	position_summary.amount = pPosition->Position;
	position_summary.today_amount = pPosition->TodayPosition;
	position_summary.margin = pPosition->UseMargin;
	position_summary.long_frozen = pPosition->LongFrozen;
	position_summary.short_frozen = pPosition->ShortFrozen;
	position_summary.frozen_margin = pPosition->FrozenMargin;
	position_summary.trading_day = pPosition->TradingDay;
	position_summary.pre_settlement_price = pPosition->PreSettlementPrice;
	position_summary.settlement_price = pPosition->SettlementPrice;
	position_summary.open_cost = pPosition->OpenCost;
	position_summary.position_cost = pPosition->PositionCost;
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

void CTPTradeHandler::CacheQryOrder(int req_id, uWS::WebSocket<uWS::SERVER>* ws, OrderQuery &order_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	qry_ws_cache_[req_id] = ws;
	qry_order_cache_[req_id] = order_qry;
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

	auto it_order_qry = qry_order_cache_.find(req_id);
	if (it_order_qry != qry_order_cache_.end())
	{
		order_qry = it_order_qry->second;
		qry_order_cache_.erase(it_order_qry);
	}
}

void CTPTradeHandler::CacheQryTrade(int req_id, uWS::WebSocket<uWS::SERVER>* ws, TradeQuery &trade_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	qry_ws_cache_[req_id] = ws;
	qry_trade_cache_[req_id] = trade_qry;
}
void CTPTradeHandler::GetAndClearCacheQryTrade(int req_id, uWS::WebSocket<uWS::SERVER>*& ws, TradeQuery &trade_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	auto it_ws = qry_ws_cache_.find(req_id);
	if (it_ws != qry_ws_cache_.end())
	{
		ws = it_ws->second;
		qry_ws_cache_.erase(it_ws);
	}

	auto it_trade_qry = qry_trade_cache_.find(req_id);
	if (it_trade_qry != qry_trade_cache_.end())
	{
		trade_qry = it_trade_qry->second;
		qry_trade_cache_.erase(it_trade_qry);
	}
}

void CTPTradeHandler::CacheQryPosition(int req_id, uWS::WebSocket<uWS::SERVER>* ws, PositionQuery &position_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	qry_ws_cache_[req_id] = ws;
	qry_position_cache_[req_id] = position_qry;
}
void CTPTradeHandler::GetAndCleanCacheQryPosition(int req_id, uWS::WebSocket<uWS::SERVER>*& ws, PositionQuery &position_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	auto it_ws = qry_ws_cache_.find(req_id);
	if (it_ws != qry_ws_cache_.end())
	{
		ws = it_ws->second;
		qry_ws_cache_.erase(it_ws);
	}

	auto it_position_qry = qry_position_cache_.find(req_id);
	if (it_position_qry != qry_position_cache_.end())
	{
		position_qry = it_position_qry->second;
		qry_position_cache_.erase(it_position_qry);
	}
}

std::string CTPTradeHandler::ExtendCTPId(const char *investor_id, const char *trading_day, const char *ctp_id)
{
	if (ctp_id[0] == '\0')
	{
		return "";
	}
	char buf[256] = { 0 };
	snprintf(buf, sizeof(buf), "%s_%s_%s", investor_id, trading_day, ctp_id);
	return std::string(buf);
}
bool CTPTradeHandler::GetCTPIdFromExtend(const char *ext_ctp_id, int len, char *ctp_id, int ctp_id_size)
{
	const char *p = ext_ctp_id + len - 1;
	int ctp_id_len = 0;
	for (ctp_id_len = 0; ctp_id_len <len; ctp_id_len++)
	{
		if (*(p - ctp_id_len) == '_')
		{
			break;
		}
	}

	if (ctp_id_len == 0 || ctp_id_len >= ctp_id_size)
	{
		return false;
	}

	strncpy(ctp_id, p - ctp_id_len + 1, ctp_id_len);
	return true;
}

char CTPTradeHandler::ConvertOrderTypeCommon2CTP(const char *order_type)
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
char CTPTradeHandler::ConvertOrderFlag1Common2CTP(const char *order_flag1)
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
bool CTPTradeHandler::ConvertOrderDirCommon2CTP(const char *order_dir, char& action, char& dir)
{
	static const char od_open[] = "open";
	static const char od_close[] = "close";
	static const char od_closetoday[] = "closetoday";
	static const char od_closehistory[] = "closehistory";
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
	else if (strncmp(order_dir, od_closehistory, split_pos) == 0)
	{
		action = THOST_FTDC_OF_CloseYesterday;
	}
	else
	{
		return false;
	}

	if (strncmp(p + 1, od_long, sizeof(od_long) - 1) == 0)
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

int CTPTradeHandler::ConvertOrderStatusCTP2Common(TThostFtdcOrderStatusType OrderStatus)
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
int CTPTradeHandler::ConvertOrderSubmitStatusCTP2Common(TThostFtdcOrderSubmitStatusType OrderSubmitStatus)
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
void CTPTradeHandler::ConvertOrderDirCTP2Common(const char ctp_dir, const char ctp_hedge_flag, const char ctp_offset_flag, Order &order)
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
			order.dir = "closehistory_short";
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
			order.dir = "closehistory_long";
		}
		else
		{
			order.dir = "close_long";
		}
	}
}

std::string CTPTradeHandler::ConvertPositionDirCTP2Common(TThostFtdcPosiDirectionType ctp_position_dir)
{
	switch (ctp_position_dir)
	{
	case THOST_FTDC_PD_Long:
		return "long";
	case THOST_FTDC_PD_Short:
		return "short";
	case THOST_FTDC_PD_Net: 
	default:
		return "net";
	}
}
std::string CTPTradeHandler::ConvertHedgeFlagCTP2Common(TThostFtdcHedgeFlagType ctp_hedge_flag)
{
	switch (ctp_hedge_flag)
	{
	case THOST_FTDC_HF_Arbitrage:
		return "arbitrage";
	case THOST_FTDC_HF_Hedge:
		return "hedge";
	case THOST_FTDC_HF_MarketMaker:
		return "marketmaker";
	case THOST_FTDC_HF_Speculation:
	default:
		return "speculation";
	}
}

std::string CTPTradeHandler::ConvertDateTypeCTP2Common(TThostFtdcPositionDateType ctp_date_type)
{
	switch (ctp_date_type)
	{
	case THOST_FTDC_PSD_Today:
		return "today";
	case THOST_FTDC_PSD_History:
	default:
		return "history";
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
void CTPTradeHandler::SerializeCTPQueryTrade(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcQryTradeField *pQryTrade)
{
	writer.Key("BrokerID");
	writer.String(pQryTrade->BrokerID);

	writer.Key("InvestorID");
	writer.String(pQryTrade->InvestorID);

	writer.Key("InstrumentID");
	writer.String(pQryTrade->InstrumentID);

	writer.Key("ExchangeID");
	writer.String(pQryTrade->ExchangeID);

	writer.Key("TradeID");
	writer.String(pQryTrade->TradeID);

	writer.Key("TradeTimeStart");
	writer.String(pQryTrade->TradeTimeStart);

	writer.Key("TradeTimeEnd");
	writer.String(pQryTrade->TradeTimeEnd);

	writer.Key("InvestUnitID");
	writer.String(pQryTrade->InvestUnitID);
}
void CTPTradeHandler::SerializeCTPQueryPosition(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcQryInvestorPositionField *pQryPosition)
{
	writer.Key("BrokerID");
	writer.String(pQryPosition->BrokerID);

	writer.Key("InvestorID");
	writer.String(pQryPosition->InvestorID);

	writer.Key("ExchangeID");
	writer.String(pQryPosition->ExchangeID);

	writer.Key("InstrumentID");
	writer.String(pQryPosition->InstrumentID);

	writer.Key("InvestUnitID");
	writer.String(pQryPosition->InvestUnitID);
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
void CTPTradeHandler::SerializeCTPPosition(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcInvestorPositionField *pInvestorPosition)
{
	char buf[2] = { 0 };

	writer.Key("InstrumentID");
	writer.String(pInvestorPosition->InstrumentID);

	writer.Key("BrokerID");
	writer.String(pInvestorPosition->BrokerID);

	writer.Key("InvestorID");
	writer.String(pInvestorPosition->InvestorID);

	buf[0] = pInvestorPosition->PosiDirection;
	writer.Key("PosiDirection");
	writer.String(buf);

	buf[0] = pInvestorPosition->HedgeFlag;
	writer.Key("HedgeFlag");
	writer.String(buf);

	buf[0] = pInvestorPosition->PositionDate;
	writer.Key("PositionDate");
	writer.String(buf);

	writer.Key("YdPosition");
	writer.Int(pInvestorPosition->YdPosition);

	writer.Key("Position");
	writer.Int(pInvestorPosition->Position);

	writer.Key("LongFrozen");
	writer.Int(pInvestorPosition->LongFrozen);

	writer.Key("ShortFrozen");
	writer.Int(pInvestorPosition->ShortFrozen);

	writer.Key("LongFrozenAmount");
	writer.Double(pInvestorPosition->LongFrozenAmount);

	writer.Key("ShortFrozenAmount");
	writer.Double(pInvestorPosition->ShortFrozenAmount);

	writer.Key("OpenVolume");
	writer.Int(pInvestorPosition->OpenVolume);

	writer.Key("CloseVolume");
	writer.Int(pInvestorPosition->CloseVolume);

	writer.Key("OpenAmount");
	writer.Double(pInvestorPosition->OpenAmount);

	writer.Key("CloseAmount");
	writer.Double(pInvestorPosition->CloseAmount);

	writer.Key("PositionCost");
	writer.Double(pInvestorPosition->PositionCost);

	writer.Key("PreMargin");
	writer.Double(pInvestorPosition->PreMargin);

	writer.Key("UseMargin");
	writer.Double(pInvestorPosition->UseMargin);

	writer.Key("FrozenMargin");
	writer.Double(pInvestorPosition->FrozenMargin);

	writer.Key("FrozenCash");
	writer.Double(pInvestorPosition->FrozenCash);

	writer.Key("FrozenCommission");
	writer.Double(pInvestorPosition->FrozenCommission);

	writer.Key("CashIn");
	writer.Double(pInvestorPosition->CashIn);

	writer.Key("Commission");
	writer.Double(pInvestorPosition->Commission);

	writer.Key("CloseProfit");
	writer.Double(pInvestorPosition->CloseProfit);

	writer.Key("PositionProfit");
	writer.Double(pInvestorPosition->PositionProfit);

	writer.Key("PreSettlementPrice");
	writer.Double(pInvestorPosition->PreSettlementPrice);

	writer.Key("SettlementPrice");
	writer.Double(pInvestorPosition->SettlementPrice);

	writer.Key("TradingDay");
	writer.String(pInvestorPosition->TradingDay);

	writer.Key("SettlementID");
	writer.Int(pInvestorPosition->SettlementID);

	writer.Key("OpenCost");
	writer.Double(pInvestorPosition->OpenCost);

	writer.Key("ExchangeMargin");
	writer.Double(pInvestorPosition->ExchangeMargin);

	writer.Key("CombPosition");
	writer.Int(pInvestorPosition->CombPosition);

	writer.Key("CombLongFrozen");
	writer.Int(pInvestorPosition->CombLongFrozen);

	writer.Key("CombShortFrozen");
	writer.Int(pInvestorPosition->CombShortFrozen);

	writer.Key("CloseProfitByDate");
	writer.Double(pInvestorPosition->CloseProfitByDate);

	writer.Key("CloseProfitByTrade");
	writer.Double(pInvestorPosition->CloseProfitByTrade);

	writer.Key("TodayPosition");
	writer.Int(pInvestorPosition->TodayPosition);

	writer.Key("MarginRateByMoney");
	writer.Double(pInvestorPosition->MarginRateByMoney);

	writer.Key("MarginRateByVolume");
	writer.Double(pInvestorPosition->MarginRateByVolume);

	writer.Key("StrikeFrozen");
	writer.Int(pInvestorPosition->StrikeFrozen);

	writer.Key("StrikeFrozenAmount");
	writer.Double(pInvestorPosition->StrikeFrozenAmount);

	writer.Key("AbandonFrozen");
	writer.Int(pInvestorPosition->AbandonFrozen);

	writer.Key("ExchangeID");
	writer.String(pInvestorPosition->ExchangeID);

	writer.Key("YdStrikeFrozen");
	writer.Int(pInvestorPosition->YdStrikeFrozen);

	writer.Key("InvestUnitID");
	writer.String(pInvestorPosition->InvestUnitID);
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
void CTPTradeHandler::OutputTradeQuery(CThostFtdcQryTradeField *req)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_tradequery");

	writer.Key("data");
	writer.StartObject();
	SerializeCTPQueryTrade(writer, req);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputPositionQuery(CThostFtdcQryInvestorPositionField *req)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_positionquery");

	writer.Key("data");
	writer.StartObject();
	SerializeCTPQueryPosition(writer, req);
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
	if (pOrder)
	{
		SerializeCTPOrder(writer, pOrder);
	}
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputRspTradeQuery(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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
	if (pTrade)
	{
		SerializeCTPTrade(writer, pTrade);
	}
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputRspPositionQuery(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_rspqryposition");
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
	if (pInvestorPosition)
	{
		SerializeCTPPosition(writer, pInvestorPosition);
	}
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}