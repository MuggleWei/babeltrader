#include "ctp_trade_handler.h"

#include <thread>

#include "glog/logging.h"

#include "common/converter.h"
#include "common/utils_func.h"


CTPTradeHandler::CTPTradeHandler(CTPTradeConf &conf)
	: api_(nullptr)
	, api_ready_(false)
	, conf_(conf)
	, ws_service_(nullptr, this)
	, http_service_(nullptr, this)
	, req_id_(1)
	, order_ref_(1)
	, order_action_ref_(1)
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
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}

	// convert to ctp struct
	CThostFtdcInputOrderField req = { 0 };
	ConvertInsertOrderCommon2CTP(order, req);

	// log output
	OutputOrderInsert(&req);

	// record order
	RecordOrder(order, req.OrderRef, ctp_front_id_, ctp_session_id_);
	
	// send req
	int ret = api_->ReqOrderInsert(&req, req_id_++);
	if (ret != 0)
	{
		GetAndCleanRecordOrder(nullptr, req.InvestorID, req.OrderRef, ctp_front_id_, ctp_session_id_);

		char buf[512];
		snprintf(buf, sizeof(buf) - 1, "failed in ReqOrderInsert, return %d", ret);
		throw std::runtime_error(buf);
	}
}
void CTPTradeHandler::CancelOrder(uWS::WebSocket<uWS::SERVER> *ws, Order &order)
{
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}

	CThostFtdcInputOrderActionField req = { 0 };
	ConvertCancelOrderCommon2CTP(order, req);

	// output
	OutputOrderAction(&req);

	int ret = api_->ReqOrderAction(&req, req_id_++);
	if (ret != 0)
	{
		char buf[512];
		snprintf(buf, sizeof(buf) - 1, "failed in ReqOrderAction, return %d", ret);
		throw std::runtime_error(buf);
	}
}
void CTPTradeHandler::QueryOrder(uWS::WebSocket<uWS::SERVER> *ws, OrderQuery &order_query)
{
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}

	CThostFtdcQryOrderField req = { 0 };
	ConvertQueryOrderCommon2CTP(order_query, req);	

	// output
	OutputOrderQuery(&req);

	// cache query order
	qry_cache_.CacheQryOrder(req_id_, ws, order_query);

	int ret = api_->ReqQryOrder(&req, req_id_++);
	if (ret != 0)
	{
		qry_cache_.GetAndClearCacheQryOrder(req_id_ - 1, nullptr, nullptr);

		char buf[512];
		snprintf(buf, sizeof(buf) - 1, "failed in ReqQryOrder, return %d", ret);
		throw std::runtime_error(buf);
	}
}
void CTPTradeHandler::QueryTrade(uWS::WebSocket<uWS::SERVER> *ws, TradeQuery &trade_query)
{
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}

	CThostFtdcQryTradeField req = { 0 };
	ConvertQueryTradeCommon2CTP(trade_query, req);

	// output
	OutputTradeQuery(&req);

	// cache query order
	qry_cache_.CacheQryTrade(req_id_, ws, trade_query);

	int ret = api_->ReqQryTrade(&req, req_id_++);
	if (ret != 0)
	{
		qry_cache_.GetAndClearCacheQryTrade(req_id_ - 1, nullptr, nullptr);

		char buf[512];
		snprintf(buf, sizeof(buf) - 1, "failed in ReqQryTrade, return %d", ret);
		throw std::runtime_error(buf);
	}
}
void CTPTradeHandler::QueryPosition(uWS::WebSocket<uWS::SERVER> *ws, PositionQuery &position_query)
{
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}

	CThostFtdcQryInvestorPositionField req = { 0 };
	ConvertQueryPositionCommon2CTP(position_query, req);

	// output
	OutputPositionQuery(&req);

	// cache query position
	qry_cache_.CacheQryPosition(req_id_, ws, position_query);

	int ret = api_->ReqQryInvestorPosition(&req, req_id_++);
	if (ret != 0)
	{
		qry_cache_.GetAndCleanCacheQryPosition(req_id_ - 1, nullptr, nullptr);

		char buf[512];
		snprintf(buf, sizeof(buf) - 1, "failed in ReqQryInvestorPosition, return %d", ret);
		throw std::runtime_error(buf);
	}
}
void CTPTradeHandler::QueryPositionDetail(uWS::WebSocket<uWS::SERVER> *ws, PositionQuery &position_query)
{
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}

	CThostFtdcQryInvestorPositionDetailField req = { 0 };
	ConvertQueryPositionDetailCommon2CTP(position_query, req);

	// output
	OutputPositionDetailQuery(&req);

	// cache query position detail
	qry_cache_.CacheQryPositionDetail(req_id_, ws, position_query);

	int ret = api_->ReqQryInvestorPositionDetail(&req, req_id_++);
	if (ret != 0)
	{
		qry_cache_.GetAndCleanCacheQryPositionDetail(req_id_ - 1, nullptr, nullptr);

		char buf[512];
		snprintf(buf, sizeof(buf) - 1, "failed in ReqQryInvestorPositionDetail, return %d", ret);
		throw std::runtime_error(buf);
	}
}
void CTPTradeHandler::QueryTradeAccount(uWS::WebSocket<uWS::SERVER> *ws, TradeAccountQuery &tradeaccount_query)
{
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}

	CThostFtdcQryTradingAccountField req = { 0 };
	ConvertQueryTradeAccountCommon2CTP(tradeaccount_query, req);

	// output
	OutputTradeAccountQuery(&req);

	// cache query trade account
	qry_cache_.CacheQryTradeAccount(req_id_, ws, tradeaccount_query);

	int ret = api_->ReqQryTradingAccount(&req, req_id_++);
	if (ret != 0)
	{
		qry_cache_.GetAndCleanCacheQryTradeAccount(req_id_ - 1, nullptr, nullptr);

		char buf[512];
		snprintf(buf, sizeof(buf) - 1, "failed in ReqQryTradingAccount, return %d", ret);
		throw std::runtime_error(buf);
	}
}
void CTPTradeHandler::QueryProduct(uWS::WebSocket<uWS::SERVER> *ws, ProductQuery &product_query)
{
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}

	if (product_query.symbol.size() != 0 && product_query.contract.size() == 0)
	{
		CThostFtdcQryProductField req = { 0 };
		strncpy(req.ExchangeID, product_query.exchange.c_str(), sizeof(req.ExchangeID) - 1);
		strncpy(req.ProductID, product_query.symbol.c_str(), sizeof(req.ProductID) - 1);

		OutputProductQuery(&req);

		qry_cache_.CacheQryProduct(req_id_, ws, product_query);

		int ret = api_->ReqQryProduct(&req, req_id_++);
		if (ret != 0)
		{
			qry_cache_.GetAndCleanCacheQryProduct(req_id_ - 1, nullptr, nullptr);

			char buf[512];
			snprintf(buf, sizeof(buf) - 1, "failed in ReqQryProduct, return %d", ret);
			throw std::runtime_error(buf);
		}
	}
	else
	{
		CThostFtdcQryInstrumentField req = { 0 };
		if (product_query.exchange.size() > 0) {
			strncpy(req.ExchangeID, product_query.exchange.c_str(), sizeof(req.ExchangeID) - 1);
		}
		if (product_query.symbol.size() > 0 && product_query.contract.size() > 0) {
			snprintf(req.InstrumentID, sizeof(req.InstrumentID) - 1, "%s%s", product_query.symbol.c_str(), product_query.contract.c_str());
		}

		OutputInstrumentQuery(&req);

		qry_cache_.CacheQryProduct(req_id_, ws, product_query);

		int ret = api_->ReqQryInstrument(&req, req_id_++);
		if (ret != 0)
		{
			qry_cache_.GetAndCleanCacheQryProduct(req_id_ - 1, nullptr, nullptr);

			char buf[512];
			snprintf(buf, sizeof(buf) - 1, "failed in ReqQryInstrument, return %d", ret);
			throw std::runtime_error(buf);
		}
	}
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
	GetAndCleanRecordOrder(&order, pInputOrder->UserID, pInputOrder->OrderRef, ctp_front_id_, ctp_session_id_);
	ConvertInsertOrderCTP2Common(*pInputOrder, order);

	BroadcastConfirmOrder(order, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
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
	bool ret = GetAndCleanRecordOrder(&order, pOrder->UserID, pOrder->OrderRef, pOrder->FrontID, pOrder->SessionID);
	ConvertRtnOrderCTP2Common(pOrder, order, order_status);
	if (ret)
	{
		BroadcastConfirmOrder(order, 0, "");
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
		qry_cache_.GetAndClearCacheQryOrder(nRequestID, &ws, &order_qry);
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
			RspOrderQry(ws, order_qry, common_orders, common_order_status, error_id);
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
		qry_cache_.GetAndClearCacheQryTrade(nRequestID, &ws, &trade_qry);
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
			RspTradeQry(ws, trade_qry, common_orders, common_deals, error_id);
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
		qry_cache_.GetAndCleanCacheQryPosition(nRequestID, &ws, &position_qry);
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
			RspPositionQryType1(ws, position_qry, positions, error_id);
		}

		rsp_qry_position_caches_.erase(nRequestID);
	}
}
void CTPTradeHandler::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspPositionDetailQuery(pInvestorPositionDetail, pRspInfo, nRequestID, bIsLast);

	auto it = rsp_qry_position_detail_caches_.find(nRequestID);
	if (it == rsp_qry_position_detail_caches_.end())
	{
		std::vector<CThostFtdcInvestorPositionDetailField> vec;
		rsp_qry_position_detail_caches_[nRequestID] = std::move(vec);
		it = rsp_qry_position_detail_caches_.find(nRequestID);
	}
	if (pInvestorPositionDetail)
	{
		CThostFtdcInvestorPositionDetailField copy_position_detail;
		memcpy(&copy_position_detail, pInvestorPositionDetail, sizeof(copy_position_detail));
		it->second.push_back(copy_position_detail);
	}

	if (bIsLast)
	{
		uWS::WebSocket<uWS::SERVER>* ws;
		PositionQuery position_detail_qry;
		qry_cache_.GetAndCleanCacheQryPositionDetail(nRequestID, &ws, &position_detail_qry);
		std::vector<CThostFtdcInvestorPositionDetailField> &ctp_position_details = rsp_qry_position_detail_caches_[nRequestID];

		if (ws)
		{
			std::vector<PositionDetailType1> position_details;
			for (CThostFtdcInvestorPositionDetailField &ctp_position_detail : ctp_position_details)
			{
				PositionDetailType1 position_detail;
				ConvertPositionDetailCTP2Common(&ctp_position_detail, position_detail);
				position_details.push_back(std::move(position_detail));
			}

			int error_id = 0;
			if (pRspInfo) {
				error_id = pRspInfo->ErrorID;
			}
			RspPositionDetailQryType1(ws, position_detail_qry, position_details, error_id);
		}

		rsp_qry_position_detail_caches_.erase(nRequestID);
	}
}
void CTPTradeHandler::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspTradingAccountQuery(pTradingAccount, pRspInfo, nRequestID, bIsLast);

	auto it = rsp_qry_trade_account_caches_.find(nRequestID);
	if (it == rsp_qry_trade_account_caches_.end())
	{
		std::vector<CThostFtdcTradingAccountField> vec;
		rsp_qry_trade_account_caches_[nRequestID] = std::move(vec);
		it = rsp_qry_trade_account_caches_.find(nRequestID);
	}
	if (pTradingAccount)
	{
		CThostFtdcTradingAccountField copy_trade_account;
		memcpy(&copy_trade_account, pTradingAccount, sizeof(copy_trade_account));
		it->second.push_back(copy_trade_account);
	}

	if (bIsLast)
	{
		uWS::WebSocket<uWS::SERVER>* ws;
		TradeAccountQuery trade_account_qry;
		qry_cache_.GetAndCleanCacheQryTradeAccount(nRequestID, &ws, &trade_account_qry);
		std::vector<CThostFtdcTradingAccountField> &ctp_trade_accounts = rsp_qry_trade_account_caches_[nRequestID];

		if (ws)
		{
			std::vector<TradeAccountType1> trade_accounts;
			for (CThostFtdcTradingAccountField &ctp_trade_account : ctp_trade_accounts)
			{
				TradeAccountType1 trade_account;
				ConvertTradeAccountCTP2Common(&ctp_trade_account, trade_account);
				trade_accounts.push_back(std::move(trade_account));
			}

			int error_id = 0;
			if (pRspInfo) {
				error_id = pRspInfo->ErrorID;
			}
			RspTradeAccountQryType1(ws, trade_account_qry, trade_accounts, error_id);
		}

		rsp_qry_trade_account_caches_.erase(nRequestID);
	}
}
void CTPTradeHandler::OnRspQryProduct(CThostFtdcProductField *pProduct, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspProductQuery(pProduct, pRspInfo, nRequestID, bIsLast);

	auto it = rsp_qry_product_caches_.find(nRequestID);
	if (it == rsp_qry_product_caches_.end())
	{
		std::vector<CThostFtdcProductField> vec;
		rsp_qry_product_caches_[nRequestID] = std::move(vec);
		it = rsp_qry_product_caches_.find(nRequestID);
	}
	if (pProduct)
	{
		CThostFtdcProductField copy_product;
		memcpy(&copy_product, pProduct, sizeof(copy_product));
		it->second.push_back(copy_product);
	}

	if (bIsLast)
	{
		uWS::WebSocket<uWS::SERVER>* ws;
		ProductQuery product_qry;
		qry_cache_.GetAndCleanCacheQryProduct(nRequestID, &ws, &product_qry);
		std::vector<CThostFtdcProductField> &ctp_products = rsp_qry_product_caches_[nRequestID];

		if (ws)
		{
			std::vector<ProductType1> products;
			for (CThostFtdcProductField &ctp_product : ctp_products)
			{
				ProductType1 product;
				ConvertProductCTP2Common(&ctp_product, product);
				products.push_back(std::move(product));
			}

			int error_id = 0;
			if (pRspInfo) {
				error_id = pRspInfo->ErrorID;
			}
			RspProductQryType1(ws, product_qry, products, error_id);
		}

		rsp_qry_product_caches_.erase(nRequestID);
	}
}
void CTPTradeHandler::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// output
	OutputRspInstrumentQuery(pInstrument, pRspInfo, nRequestID, bIsLast);

	auto it = rsp_qry_instrument_caches_.find(nRequestID);
	if (it == rsp_qry_instrument_caches_.end())
	{
		std::vector<CThostFtdcInstrumentField> vec;
		rsp_qry_instrument_caches_[nRequestID] = std::move(vec);
		it = rsp_qry_instrument_caches_.find(nRequestID);
	}
	if (pInstrument)
	{
		CThostFtdcInstrumentField copy_product;
		memcpy(&copy_product, pInstrument, sizeof(copy_product));
		it->second.push_back(copy_product);
	}

	if (bIsLast)
	{
		uWS::WebSocket<uWS::SERVER>* ws;
		ProductQuery product_qry;
		qry_cache_.GetAndCleanCacheQryProduct(nRequestID, &ws, &product_qry);
		std::vector<CThostFtdcInstrumentField> &ctp_products = rsp_qry_instrument_caches_[nRequestID];

		if (ws)
		{
			std::vector<ProductType1> products;
			for (CThostFtdcInstrumentField &ctp_product : ctp_products)
			{
				ProductType1 product;
				ConvertInstrumentCTP2Common(&ctp_product, product);
				products.push_back(std::move(product));
			}

			int error_id = 0;
			if (pRspInfo) {
				error_id = pRspInfo->ErrorID;
			}
			RspProductQryType1(ws, product_qry, products, error_id);
		}

		rsp_qry_instrument_caches_.erase(nRequestID);
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
//	if (order.exchange.empty()) {
//		throw std::runtime_error("field \"exchange\" need string");
//	}
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
	snprintf(req.OrderRef, sizeof(req.OrderRef)-1, "%d", order_ref_++);

	strncpy(req.BrokerID, conf_.broker_id.c_str(), sizeof(req.BrokerID) - 1);
	strncpy(req.InvestorID, conf_.user_id.c_str(), sizeof(req.InvestorID) - 1);
	strncpy(req.UserID, conf_.user_id.c_str(), sizeof(req.UserID) - 1);
	strncpy(req.ExchangeID, order.exchange.c_str(), order.exchange.length());
	snprintf(req.InstrumentID, sizeof(req.InstrumentID)-1, "%s%s",
		order.symbol.c_str(), order.contract.c_str());

	req.OrderPriceType = ConvertOrderTypeCommon2CTP(order.order_type.c_str());
	if (req.OrderPriceType == (char)0)
	{
		throw std::runtime_error("unsupport order type");
	}

	if (!ConvertOrderDirCommon2CTP(order.dir.c_str(), order.dir.size(), req.CombOffsetFlag[0], req.Direction)) {
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
	snprintf(req.OrderRef, sizeof(req.OrderRef)-1, "%d", order_ref_++);
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
void CTPTradeHandler::ConvertQueryPositionDetailCommon2CTP(PositionQuery &position_qry, CThostFtdcQryInvestorPositionDetailField &req)
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
void CTPTradeHandler::ConvertQueryTradeAccountCommon2CTP(TradeAccountQuery &tradeaccount_query, CThostFtdcQryTradingAccountField &req)
{
	strncpy(req.BrokerID, conf_.broker_id.c_str(), sizeof(req.BrokerID) - 1);
	strncpy(req.InvestorID, conf_.user_id.c_str(), sizeof(req.InvestorID) - 1);
	if (tradeaccount_query.currency_id.size() > 0)
	{
		strncpy(req.CurrencyID, tradeaccount_query.currency_id.c_str(), sizeof(req.CurrencyID) - 1);
	}
	else
	{
		strncpy(req.CurrencyID, "CNY", sizeof(req.CurrencyID) - 1);
	}
}

void CTPTradeHandler::ConvertInsertOrderCTP2Common(CThostFtdcInputOrderField &req, Order &order)
{
	order.market = g_markets[Market_CTP];
	order.outside_user_id = req.InvestorID;
	order.exchange = req.ExchangeID;
	order.type = g_product_types[ProductType_Future];
	CTPSplitInstrument(req.InstrumentID, order.symbol, order.contract);
	order.contract_id = order.contract;
	if (req.OrderPriceType == THOST_FTDC_OPT_AnyPrice)
	{
		order.order_type = g_order_type[OrderType_Market];
	}
	else
	{
		order.order_type = g_order_type[OrderType_Limit];
	}

	ConvertOrderDirCTP2Common(req.Direction, req.CombHedgeFlag[0], req.CombOffsetFlag[0], order);

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
		order.market = g_markets[Market_CTP];
		order.outside_user_id = pOrder->InvestorID;
		order.exchange = pOrder->ExchangeID;
		order.type = g_product_types[ProductType_Future];
		CTPSplitInstrument(pOrder->InstrumentID, order.symbol, order.contract);
		order.contract_id = order.contract;
		if (pOrder->OrderPriceType == THOST_FTDC_OPT_AnyPrice)
		{
			order.order_type = g_order_type[OrderType_Market];
		}
		else
		{
			order.order_type = g_order_type[OrderType_Limit];
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
		order.market = g_markets[Market_CTP];
		order.outside_user_id = pTrade->InvestorID;
		order.exchange = pTrade->ExchangeID;
		order.type = g_product_types[ProductType_Future];
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
	position_summary.market = g_markets[Market_CTP];
	position_summary.outside_user_id = pPosition->InvestorID;
	position_summary.exchange = pPosition->ExchangeID;
	CTPSplitInstrument(pPosition->InstrumentID, position_summary.symbol, position_summary.contract);
	position_summary.contract_id = position_summary.contract;
	position_summary.dir = ConvertPositionDirCTP2Common(pPosition->PosiDirection);
	position_summary.order_flag1 = ConvertHedgeFlagCTP2Common(pPosition->HedgeFlag);
	position_summary.date_type = ConvertDateTypeCTP2Common(pPosition->PositionDate);
	position_summary.amount = pPosition->Position;
	position_summary.closed_amount = pPosition->CloseVolume;
	position_summary.today_amount = pPosition->TodayPosition;
	position_summary.margin = pPosition->UseMargin;
	position_summary.margin_rate_by_money = pPosition->MarginRateByMoney;
	position_summary.margin_rate_by_vol = pPosition->MarginRateByVolume;
	position_summary.long_frozen = pPosition->LongFrozen;
	position_summary.short_frozen = pPosition->ShortFrozen;
	position_summary.frozen_margin = pPosition->FrozenMargin;
	position_summary.trading_day = pPosition->TradingDay;
	position_summary.pre_settlement_price = pPosition->PreSettlementPrice;
	position_summary.settlement_price = pPosition->SettlementPrice;
	position_summary.open_cost = pPosition->OpenCost;
	position_summary.position_cost = pPosition->PositionCost;
	position_summary.position_profit = pPosition->PositionProfit;
	position_summary.close_profit_by_date = pPosition->CloseProfitByDate;
	position_summary.close_profit_by_trade = pPosition->CloseProfitByTrade;
}
void CTPTradeHandler::ConvertPositionDetailCTP2Common(CThostFtdcInvestorPositionDetailField *pPositionDetail, PositionDetailType1 &position_detail)
{
	position_detail.market = g_markets[Market_CTP];
	position_detail.outside_user_id = pPositionDetail->InvestorID;
	position_detail.exchange = pPositionDetail->ExchangeID;
	CTPSplitInstrument(pPositionDetail->InstrumentID, position_detail.symbol, position_detail.contract);
	position_detail.contract_id = position_detail.contract;
	if (pPositionDetail->Direction == THOST_FTDC_D_Buy)
	{
		position_detail.dir = g_order_dir[OrderDir_Long];
	}
	else
	{
		position_detail.dir = g_order_dir[OrderDir_Short];
	}
	position_detail.order_flag1 = ConvertHedgeFlagCTP2Common(pPositionDetail->HedgeFlag);
	position_detail.open_date = pPositionDetail->OpenDate;
	position_detail.trading_day = pPositionDetail->TradingDay;
	position_detail.trade_id = ExtendCTPId(pPositionDetail->InvestorID, pPositionDetail->TradingDay, pPositionDetail->TradeID);
	position_detail.amount = pPositionDetail->Volume;
	position_detail.closed_amount = pPositionDetail->CloseVolume;
	position_detail.closed_money = pPositionDetail->CloseAmount;
	position_detail.pre_settlement_price = pPositionDetail->LastSettlementPrice;
	position_detail.settlement_price = pPositionDetail->SettlementPrice;
	position_detail.open_price = pPositionDetail->OpenPrice;
	position_detail.margin = pPositionDetail->Margin;
	position_detail.margin_rate_by_money = pPositionDetail->MarginRateByMoney;
	position_detail.margin_rate_by_vol = pPositionDetail->MarginRateByVolume;
	position_detail.close_profit_by_date = pPositionDetail->CloseProfitByDate;
	position_detail.close_profit_by_trade = pPositionDetail->CloseProfitByTrade;
	position_detail.position_profit_by_date = pPositionDetail->PositionProfitByDate;
	position_detail.position_profit_by_trade = pPositionDetail->PositionProfitByTrade;
}
void CTPTradeHandler::ConvertTradeAccountCTP2Common(CThostFtdcTradingAccountField *pTradingAccount, TradeAccountType1 &trade_account)
{
	trade_account.market = g_markets[Market_CTP];
	trade_account.outside_user_id = pTradingAccount->AccountID;
	trade_account.pre_balance = pTradingAccount->PreCredit;
	trade_account.pre_balance = pTradingAccount->PreBalance;
	trade_account.pre_margin = pTradingAccount->PreMargin;
	trade_account.interest = pTradingAccount->Interest;
	trade_account.deposit = pTradingAccount->Deposit;
	trade_account.withdraw = pTradingAccount->Withdraw;
	trade_account.credit = pTradingAccount->Credit;
	trade_account.margin = pTradingAccount->CurrMargin;
	trade_account.commission = pTradingAccount->Commission;
	trade_account.close_profit = pTradingAccount->CloseProfit;
	trade_account.position_profit = pTradingAccount->PositionProfit;
	trade_account.frozen_margin = pTradingAccount->FrozenMargin;
	trade_account.frozen_cash = pTradingAccount->FrozenCash;
	trade_account.frozen_commision = pTradingAccount->FrozenCommission;
	trade_account.balance = pTradingAccount->Balance;
	trade_account.available = pTradingAccount->Available;
	trade_account.currency_id = pTradingAccount->CurrencyID;
	trade_account.trading_day = pTradingAccount->TradingDay;
}
void CTPTradeHandler::ConvertProductCTP2Common(CThostFtdcProductField *pProduct, ProductType1 &product)
{
	product.market = g_markets[Market_CTP];
	product.outside_user_id = conf_.user_id;
	product.exchange = pProduct->ExchangeID;
	product.type = ConvertProductTypeCTP2Common(pProduct->ProductClass);
	product.symbol = pProduct->ProductID;
	product.contract = "";
	product.contract_id = "";
	product.vol_multiple = pProduct->VolumeMultiple;
	product.price_tick = pProduct->PriceTick;
}
void CTPTradeHandler::ConvertInstrumentCTP2Common(CThostFtdcInstrumentField *pInstrument, ProductType1 &product)
{
	product.market = g_markets[Market_CTP];
	product.outside_user_id = conf_.user_id;
	product.exchange = pInstrument->ExchangeID;
	product.type = ConvertProductTypeCTP2Common(pInstrument->ProductClass);
	CTPSplitInstrument(pInstrument->InstrumentID, product.symbol, product.contract);
	product.contract_id = product.contract;
	product.vol_multiple = pInstrument->VolumeMultiple;
	product.price_tick = pInstrument->PriceTick;
	product.long_margin_ratio = pInstrument->LongMarginRatio;
	product.short_margin_ratio = pInstrument->ShortMarginRatio;
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
bool CTPTradeHandler::GetAndCleanRecordOrder(Order *p_order, const std::string &user_id, const std::string &order_ref, int front_id, int session_id)
{
	char buf[256] = { 0 };
	snprintf(buf, sizeof(buf), "%s_%s_%d#%d", user_id.c_str(), order_ref.c_str(), front_id, session_id);
	{
		std::unique_lock<std::mutex> lock(wati_deal_order_mtx_);
		auto it = wait_deal_orders_.find(std::string(buf));
		if (it != wait_deal_orders_.end())
		{
			if (p_order)
			{
				*p_order = it->second;
			}
			wait_deal_orders_.erase(buf);
			return true;
		}
		else
		{
			return false;
		}
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
	if (strncmp(order_type, g_order_type[OrderType_Limit], strlen(g_order_type[OrderType_Limit])) == 0)
	{
		return THOST_FTDC_OPT_LimitPrice;
	}
	else if (strncmp(order_type, g_order_type[OrderType_Market], strlen(g_order_type[OrderType_Market])) == 0)
	{
		return THOST_FTDC_OPT_AnyPrice;
	}

	return (char)0;
}
char CTPTradeHandler::ConvertOrderFlag1Common2CTP(const char *order_flag1)
{
	if (strncmp(order_flag1, g_order_flag1[OrderFlag1_Speculation], strlen(g_order_flag1[OrderFlag1_Speculation])) == 0)
	{
		return THOST_FTDC_HF_Speculation;
	}
	else if (strncmp(order_flag1, g_order_flag1[OrderFlag1_Hedge], strlen(g_order_flag1[OrderFlag1_Hedge])) == 0)
	{
		return THOST_FTDC_HF_Hedge;
	}
	else if (strncmp(order_flag1, g_order_flag1[OrderFlag1_Arbitrage], strlen(g_order_flag1[OrderFlag1_Arbitrage])) == 0)
	{
		return THOST_FTDC_HF_Arbitrage;
	}
	else if (strncmp(order_flag1, g_order_flag1[OrderFlag1_Marketmaker], strlen(g_order_flag1[OrderFlag1_Marketmaker])) == 0)
	{
		return THOST_FTDC_HF_MarketMaker;
	}

	return (char)0;
}
bool CTPTradeHandler::ConvertOrderDirCommon2CTP(const char *order_dir, int len, char& action, char& dir)
{
	const char *p_action = nullptr;
	const char *p_dir = nullptr;

	if (!SplitOrderDir(order_dir, len, &p_action, &p_dir))
	{
		return false;
	}

	if (strncmp(p_action, g_order_action[OrderAction_Open], sizeof(g_order_action[OrderAction_Open]) - 1) == 0)
	{
		action = THOST_FTDC_OF_Open;
	}
	else if (strncmp(p_action, g_order_action[OrderAction_Close], sizeof(g_order_action[OrderAction_Close]) - 1) == 0)
	{
		action = THOST_FTDC_OF_Close;
	}
	else if (strncmp(p_action, g_order_action[OrderAction_CloseToday], sizeof(g_order_action[OrderAction_CloseToday]) - 1) == 0)
	{
		action = THOST_FTDC_OF_CloseToday;
	}
	else if (strncmp(p_action, g_order_action[OrderAction_CloseHistory], sizeof(g_order_action[OrderAction_CloseHistory]) - 1) == 0)
	{
		action = THOST_FTDC_OF_CloseYesterday;
	}
	else
	{
		return false;
	}

	if (strncmp(p_dir, g_order_dir[OrderDir_Long], sizeof(g_order_dir[OrderDir_Long]) - 1) == 0)
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
	else if (strncmp(p_dir, g_order_dir[OrderDir_Short], sizeof(g_order_dir[OrderDir_Short]) - 1) == 0)
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
	{
		ret = OrderStatus_PartDealed;
	}break;
	case THOST_FTDC_OST_PartTradedNotQueueing:
	{
		ret = OrderStatus_PartCanceled;
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
		order.order_flag1 = g_order_flag1[OrderFlag1_Speculation];
	}break;
	case THOST_FTDC_HF_Arbitrage:
	{
		order.order_flag1 = g_order_flag1[OrderFlag1_Arbitrage];
	}break;
	case THOST_FTDC_HF_Hedge:
	{
		order.order_flag1 = g_order_flag1[OrderFlag1_Hedge];
	}break;
	case THOST_FTDC_HF_MarketMaker:
	{
		order.order_flag1 = g_order_flag1[OrderFlag1_Marketmaker];
	}break;
	}

	char buf[64] = { 0 };
	if (ctp_dir == THOST_FTDC_D_Buy)
	{
		if (ctp_offset_flag == THOST_FTDC_OF_Open)
		{
			snprintf(buf, sizeof(buf) - 1, "%s_%s", g_order_action[OrderAction_Open], g_order_dir[OrderDir_Long]);
		}
		else if (ctp_offset_flag == THOST_FTDC_OF_CloseToday)
		{
			snprintf(buf, sizeof(buf) - 1, "%s_%s", g_order_action[OrderAction_CloseToday], g_order_dir[OrderDir_Short]);
		}
		else if (ctp_offset_flag == THOST_FTDC_OF_CloseYesterday)
		{
			snprintf(buf, sizeof(buf) - 1, "%s_%s", g_order_action[OrderAction_CloseHistory], g_order_dir[OrderDir_Short]);
		}
		else if (ctp_offset_flag == THOST_FTDC_OF_ForceClose || ctp_offset_flag == THOST_FTDC_OF_ForceOff)
		{
			snprintf(buf, sizeof(buf) - 1, "%s_%s", g_order_action[OrderAction_ForceClose], g_order_dir[OrderDir_Short]);
		}
		else
		{
			snprintf(buf, sizeof(buf) - 1, "%s_%s", g_order_action[OrderAction_Close], g_order_dir[OrderDir_Short]);
		}
	}
	else
	{
		if (ctp_offset_flag == THOST_FTDC_OF_Open)
		{
			snprintf(buf, sizeof(buf) - 1, "%s_%s", g_order_action[OrderAction_Open], g_order_dir[OrderDir_Short]);
		}
		else if (ctp_offset_flag == THOST_FTDC_OF_CloseToday)
		{
			snprintf(buf, sizeof(buf) - 1, "%s_%s", g_order_action[OrderAction_CloseToday], g_order_dir[OrderDir_Long]);
		}
		else if (ctp_offset_flag == THOST_FTDC_OF_CloseYesterday)
		{
			snprintf(buf, sizeof(buf) - 1, "%s_%s", g_order_action[OrderAction_CloseHistory], g_order_dir[OrderDir_Long]);
		}
		else if (ctp_offset_flag == THOST_FTDC_OF_ForceClose || ctp_offset_flag == THOST_FTDC_OF_ForceOff)
		{
			snprintf(buf, sizeof(buf) - 1, "%s_%s", g_order_action[OrderAction_ForceClose], g_order_dir[OrderDir_Long]);
		}
		else
		{
			snprintf(buf, sizeof(buf) - 1, "%s_%s", g_order_action[OrderAction_Close], g_order_dir[OrderDir_Long]);
		}
	}
	order.dir = buf;
}

std::string CTPTradeHandler::ConvertPositionDirCTP2Common(TThostFtdcPosiDirectionType ctp_position_dir)
{
	switch (ctp_position_dir)
	{
	case THOST_FTDC_PD_Long:
		return g_order_dir[OrderDir_Long];
	case THOST_FTDC_PD_Short:
		return g_order_dir[OrderDir_Short];
	case THOST_FTDC_PD_Net: 
	default:
		return g_order_dir[OrderDir_Net];
	}
}
std::string CTPTradeHandler::ConvertHedgeFlagCTP2Common(TThostFtdcHedgeFlagType ctp_hedge_flag)
{
	switch (ctp_hedge_flag)
	{
	case THOST_FTDC_HF_Arbitrage:
		return g_order_flag1[OrderFlag1_Arbitrage];
	case THOST_FTDC_HF_Hedge:
		return g_order_flag1[OrderFlag1_Hedge];
	case THOST_FTDC_HF_MarketMaker:
		return g_order_flag1[OrderFlag1_Marketmaker];
	case THOST_FTDC_HF_Speculation:
	default:
		return g_order_flag1[OrderFlag1_Speculation];
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
std::string CTPTradeHandler::ConvertProductTypeCTP2Common(TThostFtdcProductClassType ctp_product_type)
{
	switch (ctp_product_type)
	{
	case THOST_FTDC_PC_Options:
		return g_product_types[ProductType_Option];
	case THOST_FTDC_PC_Futures:
	default:
		return g_product_types[ProductType_Future];
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
void CTPTradeHandler::SerializeCTPQueryPositionDetail(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcQryInvestorPositionDetailField *pQryPosition)
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
void CTPTradeHandler::SerializeCTPQueryTradeAccount(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcQryTradingAccountField *pTradeingAccount)
{
	char buf[2] = { 0 };

	writer.Key("BrokerID");
	writer.String(pTradeingAccount->BrokerID);

	writer.Key("InvestorID");
	writer.String(pTradeingAccount->InvestorID);

	writer.Key("CurrencyID");
	writer.String(pTradeingAccount->CurrencyID);

	buf[0] = pTradeingAccount->BizType;
	writer.Key("BizType");
	writer.String(buf);

	writer.Key("AccountID");
	writer.String(pTradeingAccount->AccountID);
}
void CTPTradeHandler::SerializeCTPQueryProduct(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcQryProductField *pQryProduct)
{
	char buf[2] = { 0 };

	writer.Key("ProductID");
	writer.String(pQryProduct->ProductID);

	buf[0] = pQryProduct->ProductClass;
	writer.Key("ProductClass");
	writer.String(buf);

	writer.Key("ExchangeID");
	writer.String(pQryProduct->ExchangeID);
}
void CTPTradeHandler::SerializeCTPQueryInstrument(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcQryInstrumentField *pQryInstrument)
{
	char buf[2] = { 0 };

	writer.Key("InstrumentID");
	writer.String(pQryInstrument->InstrumentID);
	
	writer.Key("ExchangeID");
	writer.String(pQryInstrument->ExchangeID);

	writer.Key("ExchangeInstID");
	writer.String(pQryInstrument->ExchangeInstID);

	writer.Key("ProductID");
	writer.String(pQryInstrument->ProductID);
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
void CTPTradeHandler::SerializeCTPPositionDetail(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail)
{
	char buf[2] = { 0 };

	writer.Key("InstrumentID");
	writer.String(pInvestorPositionDetail->InstrumentID);

	writer.Key("BrokerID");
	writer.String(pInvestorPositionDetail->BrokerID);

	writer.Key("InvestorID");
	writer.String(pInvestorPositionDetail->InvestorID);

	buf[0] = pInvestorPositionDetail->HedgeFlag;
	writer.Key("HedgeFlag");
	writer.String(buf);

	buf[0] = pInvestorPositionDetail->Direction;
	writer.Key("Direction");
	writer.String(buf);

	writer.Key("OpenDate");
	writer.String(pInvestorPositionDetail->OpenDate);

	writer.Key("TradeID");
	writer.String(pInvestorPositionDetail->TradeID);

	writer.Key("Volume");
	writer.Int(pInvestorPositionDetail->Volume);

	writer.Key("OpenPrice");
	writer.Double(pInvestorPositionDetail->OpenPrice);

	writer.Key("TradingDay");
	writer.String(pInvestorPositionDetail->TradingDay);

	writer.Key("SettlementID");
	writer.Int(pInvestorPositionDetail->SettlementID);

	buf[0] = pInvestorPositionDetail->TradeType;
	writer.Key("TradeType");
	writer.String(buf);

	writer.Key("CombInstrumentID");
	writer.String(pInvestorPositionDetail->CombInstrumentID);

	writer.Key("ExchangeID");
	writer.String(pInvestorPositionDetail->ExchangeID);

	writer.Key("CloseProfitByDate");
	writer.Double(pInvestorPositionDetail->CloseProfitByDate);

	writer.Key("CloseProfitByTrade");
	writer.Double(pInvestorPositionDetail->CloseProfitByTrade);

	writer.Key("PositionProfitByDate");
	writer.Double(pInvestorPositionDetail->PositionProfitByDate);

	writer.Key("PositionProfitByTrade");
	writer.Double(pInvestorPositionDetail->PositionProfitByTrade);

	writer.Key("Margin");
	writer.Double(pInvestorPositionDetail->Margin);

	writer.Key("ExchMargin");
	writer.Double(pInvestorPositionDetail->ExchMargin);

	writer.Key("MarginRateByMoney");
	writer.Double(pInvestorPositionDetail->MarginRateByMoney);

	writer.Key("MarginRateByVolume");
	writer.Double(pInvestorPositionDetail->MarginRateByVolume);

	writer.Key("LastSettlementPrice");
	writer.Double(pInvestorPositionDetail->LastSettlementPrice);

	writer.Key("SettlementPrice");
	writer.Double(pInvestorPositionDetail->SettlementPrice);

	writer.Key("CloseVolume");
	writer.Int(pInvestorPositionDetail->CloseVolume);

	writer.Key("CloseAmount");
	writer.Double(pInvestorPositionDetail->CloseAmount);

	writer.Key("InvestUnitID");
	writer.String(pInvestorPositionDetail->InvestUnitID);
}
void CTPTradeHandler::SerializeCTPTradingAccount(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcTradingAccountField *pTradingAccount)
{
	char buf[2] = { 0 };

	writer.Key("BrokerID");
	writer.String(pTradingAccount->BrokerID);

	writer.Key("AccountID");
	writer.String(pTradingAccount->AccountID);

	writer.Key("PreMortgage");
	writer.Double(pTradingAccount->PreMortgage);

	writer.Key("PreCredit");
	writer.Double(pTradingAccount->PreCredit);

	writer.Key("PreDeposit");
	writer.Double(pTradingAccount->PreDeposit);

	writer.Key("PreBalance");
	writer.Double(pTradingAccount->PreBalance);

	writer.Key("PreMargin");
	writer.Double(pTradingAccount->PreMargin);

	writer.Key("InterestBase");
	writer.Double(pTradingAccount->InterestBase);

	writer.Key("Interest");
	writer.Double(pTradingAccount->Interest);

	writer.Key("Deposit");
	writer.Double(pTradingAccount->Deposit);

	writer.Key("Withdraw");
	writer.Double(pTradingAccount->Withdraw);

	writer.Key("FrozenMargin");
	writer.Double(pTradingAccount->FrozenMargin);

	writer.Key("FrozenCash");
	writer.Double(pTradingAccount->FrozenCash);

	writer.Key("FrozenCommission");
	writer.Double(pTradingAccount->FrozenCommission);

	writer.Key("CurrMargin");
	writer.Double(pTradingAccount->CurrMargin);

	writer.Key("CashIn");
	writer.Double(pTradingAccount->CashIn);

	writer.Key("Commission");
	writer.Double(pTradingAccount->Commission);

	writer.Key("CloseProfit");
	writer.Double(pTradingAccount->CloseProfit);

	writer.Key("PositionProfit");
	writer.Double(pTradingAccount->PositionProfit);

	writer.Key("Balance");
	writer.Double(pTradingAccount->Balance);

	writer.Key("Available");
	writer.Double(pTradingAccount->Available);

	writer.Key("WithdrawQuota");
	writer.Double(pTradingAccount->WithdrawQuota);

	writer.Key("Reserve");
	writer.Double(pTradingAccount->Reserve);

	writer.Key("TradingDay");
	writer.String(pTradingAccount->TradingDay);

	writer.Key("SettlementID");
	writer.Int(pTradingAccount->SettlementID);

	writer.Key("Credit");
	writer.Double(pTradingAccount->Credit);

	writer.Key("Mortgage");
	writer.Double(pTradingAccount->Mortgage);

	writer.Key("ExchangeMargin");
	writer.Double(pTradingAccount->ExchangeMargin);

	writer.Key("DeliveryMargin");
	writer.Double(pTradingAccount->DeliveryMargin);

	writer.Key("ExchangeDeliveryMargin");
	writer.Double(pTradingAccount->ExchangeDeliveryMargin);

	writer.Key("ReserveBalance");
	writer.Double(pTradingAccount->ReserveBalance);

	writer.Key("CurrencyID");
	writer.String(pTradingAccount->CurrencyID);

	writer.Key("PreFundMortgageIn");
	writer.Double(pTradingAccount->PreFundMortgageIn);

	writer.Key("PreFundMortgageOut");
	writer.Double(pTradingAccount->PreFundMortgageOut);

	writer.Key("FundMortgageIn");
	writer.Double(pTradingAccount->FundMortgageIn);

	writer.Key("FundMortgageOut");
	writer.Double(pTradingAccount->FundMortgageOut);

	writer.Key("FundMortgageAvailable");
	writer.Double(pTradingAccount->FundMortgageAvailable);

	writer.Key("MortgageableFund");
	writer.Double(pTradingAccount->MortgageableFund);

	writer.Key("SpecProductMargin");
	writer.Double(pTradingAccount->SpecProductMargin);

	writer.Key("SpecProductFrozenMargin");
	writer.Double(pTradingAccount->SpecProductFrozenMargin);

	writer.Key("SpecProductCommission");
	writer.Double(pTradingAccount->SpecProductCommission);

	writer.Key("SpecProductFrozenCommission");
	writer.Double(pTradingAccount->SpecProductFrozenCommission);

	writer.Key("SpecProductPositionProfit");
	writer.Double(pTradingAccount->SpecProductPositionProfit);

	writer.Key("SpecProductCloseProfit");
	writer.Double(pTradingAccount->SpecProductCloseProfit);

	writer.Key("SpecProductPositionProfitByAlg");
	writer.Double(pTradingAccount->SpecProductPositionProfitByAlg);

	writer.Key("SpecProductExchangeMargin");
	writer.Double(pTradingAccount->SpecProductExchangeMargin);

	buf[0] = pTradingAccount->BizType;
	writer.Key("BizType");
	writer.String(buf);

	writer.Key("FrozenSwap");
	writer.Double(pTradingAccount->FrozenSwap);

	writer.Key("RemainSwap");
	writer.Double(pTradingAccount->RemainSwap);
}
void CTPTradeHandler::SerializeCTPProduct(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcProductField *pProduct)
{
	char buf[2] = { 0 };

	writer.Key("ProductID");
	writer.String(pProduct->ProductID);

	writer.Key("ProductName");
	writer.String(pProduct->ProductName);

	writer.Key("ExchangeID");
	writer.String(pProduct->ExchangeID);

	buf[0] = pProduct->ProductClass;
	writer.Key("ProductClass");
	writer.String(buf);

	writer.Key("VolumeMultiple");
	writer.Int(pProduct->VolumeMultiple);

	writer.Key("PriceTick");
	writer.Double(pProduct->PriceTick);

	writer.Key("MaxMarketOrderVolume");
	writer.Int(pProduct->MaxMarketOrderVolume);

	writer.Key("MinMarketOrderVolume");
	writer.Int(pProduct->MinMarketOrderVolume);

	writer.Key("MaxLimitOrderVolume");
	writer.Int(pProduct->MaxLimitOrderVolume);

	writer.Key("MinLimitOrderVolume");
	writer.Int(pProduct->MinLimitOrderVolume);

	buf[0] = pProduct->PositionType;
	writer.Key("PositionType");
	writer.String(buf);

	buf[0] = pProduct->PositionDateType;
	writer.Key("PositionDateType");
	writer.String(buf);

	buf[0] = pProduct->CloseDealType;
	writer.Key("CloseDealType");
	writer.String(buf);

	writer.Key("TradeCurrencyID");
	writer.String(pProduct->TradeCurrencyID);

	buf[0] = pProduct->MortgageFundUseRange;
	writer.Key("MortgageFundUseRange");
	writer.String(buf);

	writer.Key("ExchangeProductID");
	writer.String(pProduct->ExchangeProductID);

	writer.Key("UnderlyingMultiple");
	writer.Double(pProduct->UnderlyingMultiple);
}
void CTPTradeHandler::SerializeCTPInstrument(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcInstrumentField *pInstrument)
{
	char buf[2] = { 0 };

	writer.Key("InstrumentID");
	writer.String(pInstrument->InstrumentID);

	writer.Key("ExchangeID");
	writer.String(pInstrument->ExchangeID);

	writer.Key("InstrumentName");
	writer.String(pInstrument->InstrumentName);

	writer.Key("ExchangeInstID");
	writer.String(pInstrument->ExchangeInstID);

	writer.Key("ProductID");
	writer.String(pInstrument->ProductID);

	buf[0] = pInstrument->ProductClass;
	writer.Key("ProductClass");
	writer.String(buf);

	writer.Key("DeliveryYear");
	writer.Int(pInstrument->DeliveryYear);

	writer.Key("DeliveryMonth");
	writer.Int(pInstrument->DeliveryMonth);

	writer.Key("MaxMarketOrderVolume");
	writer.Int(pInstrument->MaxMarketOrderVolume);

	writer.Key("MinMarketOrderVolume");
	writer.Int(pInstrument->MinMarketOrderVolume);

	writer.Key("MaxLimitOrderVolume");
	writer.Int(pInstrument->MaxLimitOrderVolume);

	writer.Key("MinLimitOrderVolume");
	writer.Int(pInstrument->MinLimitOrderVolume);

	writer.Key("VolumeMultiple");
	writer.Int(pInstrument->VolumeMultiple);

	writer.Key("PriceTick");
	writer.Int(pInstrument->PriceTick);

	writer.Key("CreateDate");
	writer.String(pInstrument->CreateDate);

	writer.Key("OpenDate");
	writer.String(pInstrument->OpenDate);

	writer.Key("ExpireDate");
	writer.String(pInstrument->ExpireDate);

	writer.Key("StartDelivDate");
	writer.String(pInstrument->StartDelivDate);

	writer.Key("EndDelivDate");
	writer.String(pInstrument->EndDelivDate);

	buf[0] = pInstrument->InstLifePhase;
	writer.Key("InstLifePhase");
	writer.String(buf);

	writer.Key("IsTrading");
	writer.Int(pInstrument->IsTrading);

	buf[0] = pInstrument->PositionType;
	writer.Key("PositionType");
	writer.String(buf);

	buf[0] = pInstrument->PositionDateType;
	writer.Key("PositionDateType");
	writer.String(buf);

	writer.Key("LongMarginRatio");
	writer.Double(pInstrument->LongMarginRatio);

	writer.Key("ShortMarginRatio");
	writer.Double(pInstrument->ShortMarginRatio);

	buf[0] = pInstrument->MaxMarginSideAlgorithm;
	writer.Key("MaxMarginSideAlgorithm");
	writer.String(buf);

	writer.Key("UnderlyingInstrID");
	writer.String(pInstrument->UnderlyingInstrID);

	writer.Key("StrikePrice");
	writer.Double(pInstrument->StrikePrice);

	buf[0] = pInstrument->OptionsType;
	writer.Key("OptionsType");
	writer.String(buf);

	writer.Key("UnderlyingMultiple");
	writer.Double(pInstrument->UnderlyingMultiple);

	buf[0] = pInstrument->CombinationType;
	writer.Key("CombinationType");
	writer.String(buf);
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
void CTPTradeHandler::OutputPositionDetailQuery(CThostFtdcQryInvestorPositionDetailField *req)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_positiondetailquery");

	writer.Key("data");
	writer.StartObject();
	SerializeCTPQueryPositionDetail(writer, req);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputTradeAccountQuery(CThostFtdcQryTradingAccountField *req)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_tradeaccountquery");

	writer.Key("data");
	writer.StartObject();
	SerializeCTPQueryTradeAccount(writer, req);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputProductQuery(CThostFtdcQryProductField *req)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_productquery");

	writer.Key("data");
	writer.StartObject();
	SerializeCTPQueryProduct(writer, req);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputInstrumentQuery(CThostFtdcQryInstrumentField *req)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_instrumentquery");

	writer.Key("data");
	writer.StartObject();
	SerializeCTPQueryInstrument(writer, req);
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
void CTPTradeHandler::OutputRspPositionDetailQuery(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_rspqrypositiondetail");
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
	if (pInvestorPositionDetail)
	{
		SerializeCTPPositionDetail(writer, pInvestorPositionDetail);
	}
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputRspTradingAccountQuery(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_rspqrytradingaccount");
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
	if (pTradingAccount)
	{
		SerializeCTPTradingAccount(writer, pTradingAccount);
	}
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputRspProductQuery(CThostFtdcProductField *pProduct, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_rspqryproduct");
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
	if (pProduct)
	{
		SerializeCTPProduct(writer, pProduct);
	}
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPTradeHandler::OutputRspInstrumentQuery(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("ctp_rspqryinstrument");
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
	if (pInstrument)
	{
		SerializeCTPInstrument(writer, pInstrument);
	}
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}