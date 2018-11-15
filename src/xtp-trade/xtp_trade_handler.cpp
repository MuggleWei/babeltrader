#include "xtp_trade_handler.h"

#include <string>

#include "glog/logging.h"

#include "common/converter.h"
#include "common/utils_func.h"

XTPTradeHandler::XTPTradeHandler(XTPTradeConf &conf)
	: api_(nullptr)
	, api_ready_(false)
	, xtp_session_id_(0)
	, conf_(conf)
	, ws_service_(nullptr, this)
	, http_service_(nullptr, this)
	, req_id_(1)
	, order_ref_(1)
{}

void XTPTradeHandler::run()
{
	// init ctp api
	RunAPI();

	// run service
	RunService();
}

void XTPTradeHandler::InsertOrder(uWS::WebSocket<uWS::SERVER> *ws, Order &order)
{
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}

	// convert to xtp struct
	XTPOrderInsertInfo req = { 0 };
	ConvertInsertOrderCommon2XTP(order, req);

	// log output
	OutputOrderInsert(req);

	// record order
	RecordOrder(order, req.order_client_id, xtp_session_id_);

	auto ret = api_->InsertOrder(&req, xtp_session_id_);
	if (ret == 0)
	{
		GetAndCleanRecordOrder(nullptr, req.order_client_id, xtp_session_id_);
		ThrowXTPLastError("failed insert order");
	}
}
void XTPTradeHandler::CancelOrder(uWS::WebSocket<uWS::SERVER> *ws, Order &order)
{
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}

	uint64_t order_xtp_id = GetXTPIdFromExtend(order.outside_id.c_str(), order.outside_id.size());
	if (order_xtp_id == 0)
	{
		throw std::runtime_error("invalid xtp order id");
	}

	OutputOrderCancel(order_xtp_id, xtp_session_id_);

	auto ret = api_->CancelOrder(order_xtp_id, xtp_session_id_);
	if (ret == 0)
	{
		ThrowXTPLastError("failed cancel order");
	}
}
void XTPTradeHandler::QueryOrder(uWS::WebSocket<uWS::SERVER> *ws, OrderQuery &order_query)
{
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}

	int ret = 0;
	if (order_query.outside_id.size() > 0)
	{
		uint64_t order_xtp_id = GetXTPIdFromExtend(order_query.outside_id.c_str(), order_query.outside_id.size());
		if (order_xtp_id == 0)
		{
			throw std::runtime_error("invalid xtp order id");
		}

		OutputOrderQuery(order_xtp_id);

		qry_cache_.CacheQryOrder(req_id_, ws, order_query);

		ret = api_->QueryOrderByXTPID(order_xtp_id, xtp_session_id_, req_id_++);
	}
	else
	{
		XTPQueryOrderReq req = { 0 };
		ConvertQueryOrderCommon2XTP(order_query, req);

		OutputOrderQuery(&req);

		qry_cache_.CacheQryOrder(req_id_, ws, order_query);

		ret = api_->QueryOrders(&req, xtp_session_id_, req_id_++);
	}
	
	if (ret != 0)
	{
		qry_cache_.GetAndClearCacheQryOrder(req_id_ - 1, nullptr, nullptr);

		char buf[512];
		snprintf(buf, sizeof(buf) - 1, "failed in QueryOrder, return %d", ret);
		throw std::runtime_error(buf);
	}
}
void XTPTradeHandler::QueryTrade(uWS::WebSocket<uWS::SERVER> *ws, TradeQuery &trade_query)
{
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}

	int ret = 0;
	if (trade_query.trade_id.size() > 0)
	{
		uint64_t trade_xtp_id = GetXTPIdFromExtend(trade_query.trade_id.c_str(), trade_query.trade_id.size());
		if (trade_xtp_id == 0)
		{
			throw std::runtime_error("invalid xtp trade id");
		}

		OutputTradeQuery(trade_xtp_id);

		qry_cache_.CacheQryTrade(req_id_, ws, trade_query);

		ret = api_->QueryTradesByXTPID(trade_xtp_id, xtp_session_id_, req_id_++);
	}
	else
	{
		XTPQueryTraderReq req = { 0 };
		ConvertQueryTradeCommon2XTP(trade_query, req);

		OutputTradeQuery(&req);

		qry_cache_.CacheQryTrade(req_id_, ws, trade_query);

		ret = api_->QueryTrades(&req, xtp_session_id_, req_id_++);
	}

	if (ret != 0)
	{
		qry_cache_.GetAndClearCacheQryTrade(req_id_ - 1, nullptr, nullptr);

		char buf[512];
		snprintf(buf, sizeof(buf) - 1, "failed in QueryTrade, return %d", ret);
		throw std::runtime_error(buf);
	}
}
void XTPTradeHandler::QueryPosition(uWS::WebSocket<uWS::SERVER> *ws, PositionQuery &position_query)
{
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}

	OutputPositionQuery(position_query.symbol.c_str());

	qry_cache_.CacheQryPosition(req_id_, ws, position_query);

	int ret = api_->QueryPosition(position_query.symbol.c_str(), xtp_session_id_, req_id_++);
	if (ret != 0)
	{
		qry_cache_.GetAndCleanCacheQryPosition(req_id_ - 1, nullptr, nullptr);

		char buf[512];
		snprintf(buf, sizeof(buf) - 1, "failed in QueryPosition, return %d", ret);
		throw std::runtime_error(buf);
	}
}
void XTPTradeHandler::QueryPositionDetail(uWS::WebSocket<uWS::SERVER> *ws, PositionQuery &position_query)
{
	throw std::runtime_error("'QueryPositionDetail' not supported in xtp");
}
void XTPTradeHandler::QueryTradeAccount(uWS::WebSocket<uWS::SERVER> *ws, TradeAccountQuery &tradeaccount_query)
{
	if (api_ == nullptr || !api_ready_)
	{
		throw std::runtime_error("trade api not ready yet");
	}

	OutputAssetQuery(xtp_session_id_);

	qry_cache_.CacheQryTradeAccount(req_id_, ws, tradeaccount_query);

	int ret = api_->QueryAsset(xtp_session_id_, req_id_++);
	if (ret != 0)
	{
		qry_cache_.GetAndCleanCacheQryTradeAccount(req_id_ - 1, nullptr, nullptr);

		char buf[512];
		snprintf(buf, sizeof(buf) - 1, "failed in ReqQryTradingAccount, return %d", ret);
		throw std::runtime_error(buf);
	}
}
void XTPTradeHandler::QueryProduct(uWS::WebSocket<uWS::SERVER> *ws, ProductQuery &query_product)
{
	throw std::runtime_error("'QueryProduct' not supported in xtp");
}

void XTPTradeHandler::OnDisconnected(uint64_t session_id, int reason)
{
	api_ready_ = false;

	OutputFrontDisconnected(session_id, reason);

	Reconn();
}
void XTPTradeHandler::OnOrderEvent(XTPOrderInfo *order_info, XTPRI *error_info, uint64_t session_id)
{
	OutputOrderEvent(order_info, error_info, session_id);

	Order order;
	OrderStatusNotify order_status;
	bool ret = GetAndCleanRecordOrder(&order, order_info->order_client_id, session_id);
	ConvertOrderInfoXTP2Common(order_info, order, order_status);
	if (error_info)
	{
		if (ret)
		{
			BroadcastConfirmOrder(order, error_info->error_id, error_info->error_msg);
		}
		BroadcastOrderStatus(order, order_status, error_info->error_id, error_info->error_msg);
	}
	else
	{
		if (ret)
		{
			BroadcastConfirmOrder(order, 0, "");
		}
		BroadcastOrderStatus(order, order_status, 0, "");
	}
}
void XTPTradeHandler::OnTradeEvent(XTPTradeReport *trade_info, uint64_t session_id)
{
	OutputTradeEvent(trade_info, session_id);

	Order order;
	OrderDealNotify order_deal;
	ConvertTradeReportXTP2Common(trade_info, order, order_deal);

	BroadcastOrderDeal(order, order_deal);
}
void XTPTradeHandler::OnQueryOrder(XTPQueryOrderRsp *order_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id)
{
	OutputRspOrderQuery(order_info, error_info, request_id, is_last, session_id);

	auto it = rsp_qry_order_caches_.find(request_id);
	if (it == rsp_qry_order_caches_.end())
	{
		std::vector<XTPQueryOrderRsp> vec;
		rsp_qry_order_caches_[request_id] = std::move(vec);
		it = rsp_qry_order_caches_.find(request_id);
	}
	if (order_info && order_info->order_cancel_xtp_id == 0)
	{
		XTPQueryOrderRsp copy_order;
		memcpy(&copy_order, order_info, sizeof(copy_order));
		it->second.push_back(copy_order);
	}

	if (is_last)
	{
		uWS::WebSocket<uWS::SERVER>* ws;
		OrderQuery order_qry;
		qry_cache_.GetAndClearCacheQryOrder(request_id, &ws, &order_qry);
		std::vector<XTPQueryOrderRsp> &orders = rsp_qry_order_caches_[request_id];

		if (ws)
		{
			std::vector<Order> common_orders;
			std::vector<OrderStatusNotify> common_order_status;
			for (XTPQueryOrderRsp &ctp_order : orders)
			{
				Order order;
				OrderStatusNotify order_status;
				ConvertOrderInfoXTP2Common(&ctp_order, order, order_status);
				common_orders.push_back(std::move(order));
				common_order_status.push_back(std::move(order_status));
			}

			int error_id = 0;
			if (error_info) {
				error_id = error_info->error_id;
			}
			RspOrderQry(ws, order_qry, common_orders, common_order_status, error_id);
		}

		rsp_qry_order_caches_.erase(request_id);
	}
}
void XTPTradeHandler::OnQueryTrade(XTPQueryTradeRsp *trade_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id)
{
	OutputRspTradeQuery(trade_info, error_info, request_id, is_last, session_id);

	auto it = rsp_qry_trade_caches_.find(request_id);
	if (it == rsp_qry_trade_caches_.end())
	{
		std::vector<XTPQueryTradeRsp> vec;
		rsp_qry_trade_caches_[request_id] = std::move(vec);
		it = rsp_qry_trade_caches_.find(request_id);
	}
	if (trade_info)
	{
		XTPQueryTradeRsp copy_trade;
		memcpy(&copy_trade, trade_info, sizeof(copy_trade));
		it->second.push_back(copy_trade);
	}

	if (is_last)
	{
		uWS::WebSocket<uWS::SERVER>* ws;
		TradeQuery trade_qry;
		qry_cache_.GetAndClearCacheQryTrade(request_id, &ws, &trade_qry);
		std::vector<XTPQueryTradeRsp> &trades = rsp_qry_trade_caches_[request_id];

		if (ws)
		{
			std::vector<Order> common_orders;
			std::vector<OrderDealNotify> common_deals;
			for (XTPQueryTradeRsp &ctp_trade : trades)
			{
				Order order;
				OrderDealNotify order_deal;
				ConvertTradeReportXTP2Common(&ctp_trade, order, order_deal);
				common_orders.push_back(std::move(order));
				common_deals.push_back(std::move(order_deal));
			}

			int error_id = 0;
			if (error_info) {
				error_id = error_info->error_id;
			}
			RspTradeQry(ws, trade_qry, common_orders, common_deals, error_id);
		}

		rsp_qry_trade_caches_.erase(request_id);
	}
}
void XTPTradeHandler::OnQueryPosition(XTPQueryStkPositionRsp *position, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id)
{
	OutputRspPositionQuery(position, error_info, request_id, is_last, session_id);

	auto it = rsp_qry_position_caches_.find(request_id);
	if (it == rsp_qry_position_caches_.end())
	{
		std::vector<XTPQueryStkPositionRsp> vec;
		rsp_qry_position_caches_[request_id] = std::move(vec);
		it = rsp_qry_position_caches_.find(request_id);
	}
	if (position)
	{
		XTPQueryStkPositionRsp copy_position;
		memcpy(&copy_position, position, sizeof(copy_position));
		it->second.push_back(copy_position);
	}

	if (is_last)
	{
		uWS::WebSocket<uWS::SERVER>* ws;
		PositionQuery position_qry;
		qry_cache_.GetAndCleanCacheQryPosition(request_id, &ws, &position_qry);
		std::vector<XTPQueryStkPositionRsp> &xtp_positions = rsp_qry_position_caches_[request_id];

		if (ws)
		{
			std::vector<PositionSummaryType2> positions;
			for (XTPQueryStkPositionRsp &xtp_position : xtp_positions)
			{
				PositionSummaryType2 common_position;
				ConvertPositionXTP2Common(&xtp_position, common_position);
				positions.push_back(std::move(common_position));
			}

			int error_id = 0;
			if (error_info) {
				error_id = error_info->error_id;
			}
			RspPositionQryType2(ws, position_qry, positions, error_id);
		}

		rsp_qry_position_caches_.erase(request_id);
	}
}
void XTPTradeHandler::OnQueryAsset(XTPQueryAssetRsp *asset, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id)
{
	OutputRspAssetQuery(asset, error_info, request_id, is_last, session_id);

	auto it = rsp_qry_trade_account_caches_.find(request_id);
	if (it == rsp_qry_trade_account_caches_.end())
	{
		std::vector<XTPQueryAssetRsp> vec;
		rsp_qry_trade_account_caches_[request_id] = std::move(vec);
		it = rsp_qry_trade_account_caches_.find(request_id);
	}
	if (asset)
	{
		XTPQueryAssetRsp copy_trade_account;
		memcpy(&copy_trade_account, asset, sizeof(copy_trade_account));
		it->second.push_back(copy_trade_account);
	}

	if (is_last)
	{
		uWS::WebSocket<uWS::SERVER>* ws;
		TradeAccountQuery trade_account_qry;
		qry_cache_.GetAndCleanCacheQryTradeAccount(request_id, &ws, &trade_account_qry);
		std::vector<XTPQueryAssetRsp> &ctp_trade_accounts = rsp_qry_trade_account_caches_[request_id];

		if (ws)
		{
			std::vector<TradeAccountType2> trade_accounts;
			for (XTPQueryAssetRsp &xtp_trade_account : ctp_trade_accounts)
			{
				TradeAccountType2 trade_account;
				ConvertTradeAccountXTP2Common(&xtp_trade_account, trade_account);
				trade_accounts.push_back(std::move(trade_account));
			}

			int error_id = 0;
			if (error_info) {
				error_id = error_info->error_id;
			}
			RspTradeAccountQryType2(ws, trade_account_qry, trade_accounts, error_id);
		}

		rsp_qry_trade_account_caches_.erase(request_id);
	}
}

void XTPTradeHandler::RunAPI()
{
	api_ = XTP::API::TraderApi::CreateTraderApi(conf_.client_id, "./", XTP_LOG_LEVEL_DEBUG);
	if (!api_)
	{
		auto err = api_->GetApiLastError();
		LOG(ERROR)
			<< "failed init xtp api: "
			<< "(" << err->error_id << ") "
			<< err->error_msg;
		exit(-1);
	}

	api_->SubscribePublicTopic((XTP_TE_RESUME_TYPE)XTP_TERT_QUICK);
	api_->SetSoftwareVersion("v0.0.1");
	api_->SetSoftwareKey(conf_.key.c_str());
	api_->SetHeartBeatInterval(15);
	api_->RegisterSpi(this);

	xtp_session_id_ = api_->Login(conf_.ip.c_str(), conf_.port, conf_.user_id.c_str(), conf_.password.c_str(), (XTP_PROTOCOL_TYPE)conf_.trade_protocol);
	if (xtp_session_id_ > 0)
	{
		LOG(INFO) << "xtp success login: " << conf_.user_id;
	}
	else
	{
		auto err = api_->GetApiLastError();
		LOG(ERROR) << "xtp failed login: " << conf_.user_id << " "
			<< "(" << err->error_id << ") "
			<< err->error_msg;
		exit(-1);
	}

	// generate trading day
	char buf[128] = { 0 };
	time_t t = time(NULL);
	tm *timeinfo = localtime(&t);
	strftime(buf, sizeof(buf) - 1, "%Y%m%d", timeinfo);
	trading_day_ = buf;

	api_ready_ = true;
}
void XTPTradeHandler::RunService()
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

void XTPTradeHandler::Reconn()
{
	do {
		LOG(WARNING) << "xtp reconnect...";
		int ms = 1000;
#if WIN32
		Sleep(ms);
#else
		usleep((double)(ms) * 1000.0);
#endif

		xtp_session_id_ = api_->Login(conf_.ip.c_str(), conf_.port, conf_.user_id.c_str(), conf_.password.c_str(), (XTP_PROTOCOL_TYPE)conf_.trade_protocol);
	} while (xtp_session_id_ <= 0);

	api_ready_ = true;
}

void XTPTradeHandler::ConvertInsertOrderCommon2XTP(Order &order, XTPOrderInsertInfo &req)
{
	req.order_client_id = order_ref_++;
	strncpy(req.ticker, order.symbol.c_str(), sizeof(req.ticker) - 1);
	req.market = ConvertExchangeMarketTypeCommon2XTP(order.exchange);
	req.price = order.price;
	req.quantity = (int64_t)order.amount;
	req.price_type = ConvertOrderTypeCommon2XTP(order.type, order.order_type);
	if (!ConvertOrderDirCommon2XTP(order.type, order.dir, req.side, req.position_effect))
	{
		throw std::runtime_error("failed convert order dir");
	}
}
void XTPTradeHandler::ConvertQueryOrderCommon2XTP(OrderQuery &order_qry, XTPQueryOrderReq &req)
{
	if (order_qry.symbol.size() > 0)
	{
		strncpy(req.ticker, order_qry.symbol.c_str(), sizeof(req.ticker) - 1);
	}
	req.begin_time = 0;
	req.end_time = 0;
}
void XTPTradeHandler::ConvertQueryTradeCommon2XTP(TradeQuery &trade_qry, XTPQueryTraderReq &req)
{
	if (trade_qry.symbol.size() > 0)
	{
		strncpy(req.ticker, trade_qry.symbol.c_str(), sizeof(req.ticker) - 1);
	}
	req.begin_time = 0;
	req.end_time = 0;
}

void XTPTradeHandler::ConvertOrderInfoXTP2Common(XTPOrderInfo *order_info, Order &order, OrderStatusNotify &order_status_notify)
{
	// order
	{
		order.outside_id = ExtendXTPId(conf_.user_id.c_str(), trading_day_.c_str(), order_info->order_xtp_id);
		order.market = g_markets[Market_XTP];
		order.outside_user_id = conf_.user_id;
		order.exchange = ConvertMarketTypeExchangeXTP2Common(order_info->market);
		order.symbol = order_info->ticker;
		order.dir = ConvertOrderDirXTP2Common(order_info);

		order.price = order_info->price;
		order.amount = order_info->quantity;
		order.total_price = 0;

		order.ts = XTPGetTimestamp(order_info->insert_time);
	}
	
	// status
	{
		order_status_notify.order_status = ConvertOrderStatusXTP2Common(order_info->order_status);
		order_status_notify.order_submit_status = ConvertOrderSubmitStatusXTP2Common(order_info->order_submit_status);
		order_status_notify.amount = order_info->qty_left + order_info->qty_traded;
		order_status_notify.dealed_amount = order_info->qty_traded;
	}
}
void XTPTradeHandler::ConvertTradeReportXTP2Common(XTPTradeReport *trade_info, Order &order, OrderDealNotify &order_deal)
{
	// order
	{
		order.outside_id = ExtendXTPId(conf_.user_id.c_str(), trading_day_.c_str(), trade_info->order_xtp_id);
		order.market = g_markets[Market_XTP];
		order.outside_user_id = conf_.user_id;
		order.exchange = ConvertMarketTypeExchangeXTP2Common(trade_info->market);
		order.symbol = trade_info->ticker;
		order.dir = ConvertTradeDirXTP2Common(trade_info);

		order.price = trade_info->price;
		order.amount = trade_info->quantity;
		order.total_price = 0;

		order.ts = 0;
	}

	// status
	{
		order_deal.price = trade_info->price;
		order_deal.amount = trade_info->quantity;
		order_deal.trading_day = "";
		order_deal.trade_id = ExtendXTPId(conf_.user_id.c_str(), trading_day_.c_str(), trade_info->report_index);
		order_deal.ts = XTPGetTimestamp(trade_info->trade_time);
	}
}
void XTPTradeHandler::ConvertPositionXTP2Common(XTPQueryStkPositionRsp *pPosition, PositionSummaryType2 &position_summary)
{
	position_summary.market = g_markets[Market_XTP];
	position_summary.outside_user_id = conf_.user_id;
	position_summary.exchange = "";
	position_summary.type = "";
	position_summary.symbol = pPosition->ticker;
	position_summary.dir = ConvertPositionDirXTP2Common(pPosition->position_direction);
	position_summary.amount = pPosition->total_qty;
	position_summary.avaliable_amount = pPosition->sellable_qty;
	position_summary.avg_price = pPosition->avg_price;
	position_summary.unrealized_profit = pPosition->unrealized_pnl;
	position_summary.purchase_redeemable_qty = pPosition->purchase_redeemable_qty;
	position_summary.executable_option = pPosition->executable_option;
	position_summary.lockable_position = pPosition->lockable_position;
	position_summary.executable_underlying = pPosition->executable_underlying;
	position_summary.locked_position = pPosition->locked_position;
	position_summary.usable_locked_position = pPosition->usable_locked_position;
}
void XTPTradeHandler::ConvertTradeAccountXTP2Common(XTPQueryAssetRsp *asset, TradeAccountType2 &trade_account)
{
	trade_account.market = g_markets[Market_XTP];
	trade_account.outside_user_id = conf_.user_id;
	trade_account.account_type = ConvertAccountTypeXTP2Common(asset->account_type);
	trade_account.total_asset = asset->total_asset;
	trade_account.available_cash = asset->buying_power;
	trade_account.securities_asset = asset->security_asset;
	trade_account.fund_buy_amount = asset->fund_buy_amount;
	trade_account.fund_buy_fee = asset->fund_buy_fee;
	trade_account.fund_sell_amount = asset->fund_sell_amount;
	trade_account.fund_sell_fee = asset->fund_sell_fee;
	trade_account.withholding_amount = asset->withholding_amount;
	trade_account.frozen_margin = asset->frozen_margin;
	trade_account.frozen_exec_cash = asset->frozen_exec_cash;
	trade_account.frozen_exec_fee = asset->frozen_exec_fee;
	trade_account.pay_later = asset->pay_later;
	trade_account.preadva_pay = asset->preadva_pay;
	trade_account.orig_banlance = asset->orig_banlance;
	trade_account.banlance = asset->banlance;
	trade_account.deposit_withdraw = asset->deposit_withdraw;
	trade_account.trade_netting = asset->trade_netting;
	trade_account.captial_asset = asset->captial_asset;
	trade_account.force_freeze_amount = asset->force_freeze_amount;
	trade_account.preferred_amount = asset->preferred_amount;
}

std::string XTPTradeHandler::ExtendXTPId(const char *investor_id, const char *trading_day, uint64_t xtp_id)
{
	if (xtp_id == 0)
	{
		return "";
	}

	char buf[256] = { 0 };
	snprintf(buf, sizeof(buf), "%s_%s_%llu", investor_id, trading_day, (unsigned long long)xtp_id);
	return std::string(buf);
}
uint64_t XTPTradeHandler::GetXTPIdFromExtend(const char *ext_ctp_id, int len)
{
	const char *p = ext_ctp_id + len - 1;
	int xtp_id_len = 0;
	for (xtp_id_len = 0; xtp_id_len <len; xtp_id_len++)
	{
		if (*(p - xtp_id_len) == '_')
		{
			break;
		}
	}

	if (xtp_id_len == 0 || xtp_id_len >= len)
	{
		return 0;
	}

	uint64_t ret = 0;
	try
	{
		ret = std::stoull(p - xtp_id_len + 1);
	}
	catch (std::exception &e)
	{}
	
	return ret;
}

XTP_MARKET_TYPE XTPTradeHandler::ConvertExchangeMarketTypeCommon2XTP(const std::string &exchange)
{
	if (strncmp(exchange.c_str(), g_exchanges[Exchange_SSE], strlen(g_exchanges[Exchange_SSE])) == 0)
	{
		return XTP_MKT_SH_A;
	}
	else if (strncmp(exchange.c_str(), g_exchanges[Exchange_SZSE], strlen(g_exchanges[Exchange_SZSE])) == 0)
	{
		return XTP_MKT_SZ_A;
	}
	return XTP_MKT_UNKNOWN;
}
XTP_PRICE_TYPE XTPTradeHandler::ConvertOrderTypeCommon2XTP(const std::string &product_type, const std::string &order_type)
{
	if (strncmp(order_type.c_str(), g_order_type[OrderType_Limit], strlen(g_order_type[OrderType_Limit])) == 0)
	{
		return XTP_PRICE_LIMIT;
	}
	else if (strncmp(order_type.c_str(), g_order_type[OrderType_Market], strlen(g_order_type[OrderType_Market])) == 0)
	{
		if (strncmp(product_type.c_str(), g_product_types[ProductType_Spot], strlen(g_product_types[ProductType_Spot])) == 0)
		{
			return XTP_PRICE_BEST5_OR_CANCEL;
		}
		else
		{
			return XTP_PRICE_LIMIT;
		}
	}
	return XTP_PRICE_TYPE_UNKNOWN;
}
XTP_BUSINESS_TYPE XTPTradeHandler::ConvertProductTypeCommon2XTP(const std::string &product_type)
{
	if (strncmp(product_type.c_str(), g_product_types[ProductType_Spot], strlen(g_product_types[ProductType_Spot])) == 0)
	{
		return XTP_BUSINESS_TYPE_CASH;
	}
	else if (strncmp(product_type.c_str(), g_product_types[ProductType_Option], strlen(g_product_types[ProductType_Option])) == 0)
	{
		return XTP_BUSINESS_TYPE_OPTION;
	}
	else if (strncmp(product_type.c_str(), g_product_types[ProductType_ETF], strlen(g_product_types[ProductType_ETF])) == 0)
	{
		return XTP_BUSINESS_TYPE_ETF;
	}
	else if (strncmp(product_type.c_str(), g_product_types[ProductType_IPO], strlen(g_product_types[ProductType_IPO])) == 0)
	{
		return XTP_BUSINESS_TYPE_IPOS;
	}
	else
	{
		return XTP_BUSINESS_TYPE_UNKNOWN;
	}
}
bool XTPTradeHandler::ConvertOrderDirCommon2XTP(const std::string &product_type, const std::string &dir, XTP_SIDE_TYPE &xtp_side, XTP_POSITION_EFFECT_TYPE &xtp_position_effect)
{
	const char *p_action = nullptr;
	const char *p_dir = nullptr;
	if (!SplitOrderDir(dir.c_str(), dir.size(), &p_action, &p_dir))
	{
		return false;
	}

	// dir
	if (strncmp(product_type.c_str(), g_product_types[ProductType_Spot], strlen(g_product_types[ProductType_Spot])) == 0)
	{
		xtp_side = ConvertOrderSideCommon2XTP_Spot(p_dir, p_action);
		xtp_position_effect = ConvertOrderPositionEffectCommon2XTP_Spot(p_dir, p_action);
	}
	else if (strncmp(product_type.c_str(), g_product_types[ProductType_ETF], strlen(g_product_types[ProductType_ETF])) == 0)
	{
		xtp_side = ConvertOrderSideCommon2XTP_ETF(p_dir, p_action);
		xtp_position_effect = ConvertOrderPositionEffectCommon2XTP_Spot(p_dir, p_action);
	}
	else if (strncmp(product_type.c_str(), g_product_types[ProductType_IPO], strlen(g_product_types[ProductType_IPO])) == 0)
	{
		xtp_side = ConvertOrderSideCommon2XTP_IPO(p_dir, p_action);
		xtp_position_effect = ConvertOrderPositionEffectCommon2XTP_Spot(p_dir, p_action);
	}
	else
	{
		xtp_side = XTP_SIDE_UNKNOWN;
	}

	if (xtp_side == XTP_SIDE_UNKNOWN || xtp_position_effect == XTP_POSITION_EFFECT_UNKNOWN)
	{
		return false;
	}	

	return true;
}
XTP_SIDE_TYPE XTPTradeHandler::ConvertOrderSideCommon2XTP_Spot(const char *p_dir, const char *p_action)
{
	if (strncmp(p_action, g_order_action[OrderAction_Buy], strlen(g_order_action[OrderAction_Buy])) == 0)
	{
		return XTP_SIDE_BUY;
	}
	else if (strncmp(p_action, g_order_action[OrderAction_Sell], strlen(g_order_action[OrderAction_Sell])) == 0)
	{
		return XTP_SIDE_SELL;
	}
	else
	{
		return XTP_SIDE_UNKNOWN;
	}
}
XTP_SIDE_TYPE XTPTradeHandler::ConvertOrderSideCommon2XTP_ETF(const char *p_dir, const char *p_action)
{
	if (strncmp(p_action, g_order_action[OrderAction_Buy], strlen(g_order_action[OrderAction_Buy])) == 0)
	{
		return XTP_SIDE_PURCHASE;
	}
	else if (strncmp(p_action, g_order_action[OrderAction_Sell], strlen(g_order_action[OrderAction_Sell])) == 0)
	{
		return XTP_SIDE_REDEMPTION;
	}
	else
	{
		return XTP_SIDE_UNKNOWN;
	}
}
XTP_SIDE_TYPE XTPTradeHandler::ConvertOrderSideCommon2XTP_IPO(const char *p_dir, const char *p_action)
{
	if (strncmp(p_action, g_order_action[OrderAction_Buy], strlen(g_order_action[OrderAction_Buy])) == 0)
	{
		return XTP_SIDE_BUY;
	}
	else
	{
		return XTP_SIDE_UNKNOWN;
	}
}
XTP_POSITION_EFFECT_TYPE XTPTradeHandler::ConvertOrderPositionEffectCommon2XTP_Spot(const char *p_dir, const char *p_action)
{
	return XTP_POSITION_EFFECT_INIT;
}

std::string XTPTradeHandler::ConvertMarketTypeExchangeXTP2Common(XTP_MARKET_TYPE market)
{
	switch (market)
	{
	case XTP_MKT_SZ_A:
		return g_exchanges[Exchange_SZSE];
	case XTP_MKT_SH_A:
		return g_exchanges[Exchange_SSE];
	}
	return g_exchanges[Exchange_Unknown];
}
std::string XTPTradeHandler::ConvertOrderDirXTP2Common(XTPOrderInfo *order_info)
{
	switch (order_info->side)
	{
	case XTP_SIDE_BUY:
	case XTP_SIDE_PURCHASE:
		return g_order_action[OrderAction_Buy];
	case XTP_SIDE_SELL:
	case XTP_SIDE_REDEMPTION:
		return g_order_action[OrderAction_Sell];
	}
	return g_order_action[OrderAction_Unknown];
}
std::string XTPTradeHandler::ConvertTradeDirXTP2Common(XTPTradeReport *trade_info)
{
	switch (trade_info->side)
	{
	case XTP_SIDE_BUY:
	case XTP_SIDE_PURCHASE:
		return g_order_action[OrderAction_Buy];
	case XTP_SIDE_SELL:
	case XTP_SIDE_REDEMPTION:
		return g_order_action[OrderAction_Sell];
	}
	return g_order_action[OrderAction_Unknown];
}
std::string XTPTradeHandler::ConvertPositionDirXTP2Common(XTP_POSITION_DIRECTION_TYPE dir)
{
	switch (dir)
	{
	case XTP_POSITION_DIRECTION_NET:
		return g_order_dir[OrderDir_Net];
	case XTP_POSITION_DIRECTION_LONG:
		return g_order_dir[OrderDir_Long];
	case XTP_POSITION_DIRECTION_SHORT:
		return g_order_dir[OrderDir_Short];
	}
	return g_order_dir[OrderDir_Unknown];
}
OrderStatusEnum XTPTradeHandler::ConvertOrderStatusXTP2Common(XTP_ORDER_STATUS_TYPE order_status)
{
	OrderStatusEnum ret = OrderStatus_Unknown;
	switch (order_status)
	{
	case XTP_ORDER_STATUS_ALLTRADED:
	{
		ret = OrderStatus_AllDealed;
	}break;
	case XTP_ORDER_STATUS_PARTTRADEDQUEUEING:
	{
		ret = OrderStatus_PartDealed;
	}break;
	case XTP_ORDER_STATUS_PARTTRADEDNOTQUEUEING:
	{
		ret = OrderStatus_PartCanceled;
	}break;
	case XTP_ORDER_STATUS_CANCELED:
	{
		ret = OrderStatus_Canceled;
	}break;
	case XTP_ORDER_STATUS_REJECTED:
	{
		ret = OrderStatus_Rejected;
	}break;
	}

	return ret;
}
OrderSubmitStatusEnum XTPTradeHandler::ConvertOrderSubmitStatusXTP2Common(XTP_ORDER_SUBMIT_STATUS_TYPE order_submit_status)
{
	OrderSubmitStatusEnum ret = OrderSubmitStatus_Unknown;
	switch (order_submit_status)
	{
	case XTP_ORDER_SUBMIT_STATUS_INSERT_SUBMITTED:
	case XTP_ORDER_SUBMIT_STATUS_CANCEL_SUBMITTED:
		ret = OrderSubmitStatus_Submitted;
		break;
	case XTP_ORDER_SUBMIT_STATUS_INSERT_ACCEPTED:
	case XTP_ORDER_SUBMIT_STATUS_CANCEL_ACCEPTED:
		ret = OrderSubmitStatus_Accepted;
		break;
	case XTP_ORDER_SUBMIT_STATUS_INSERT_REJECTED:
	case XTP_ORDER_SUBMIT_STATUS_CANCEL_REJECTED:
		ret = OrderSubmitStatus_Rejected;
		break;
	}
	return ret;
}
std::string XTPTradeHandler::ConvertAccountTypeXTP2Common(XTP_ACCOUNT_TYPE account_type)
{
	switch (account_type)
	{
	case XTP_ACCOUNT_NORMAL:
		return g_account_type[AccountType_Normal];
	case XTP_ACCOUNT_CREDIT:
		return g_account_type[AccountType_Credit];
	case XTP_ACCOUNT_DERIVE:
		return g_account_type[AccountType_Derivatives];
	}
	return g_account_type[AccountType_Unknown];
}

void XTPTradeHandler::RecordOrder(Order &order, uint32_t order_ref, uint64_t session_id)
{
	char buf[256] = { 0 };
	snprintf(buf, sizeof(buf), "%s_%d_%d", conf_.user_id.c_str(), order_ref, session_id);
	{
		std::unique_lock<std::mutex> lock(wati_deal_order_mtx_);
		wait_deal_orders_[std::string(buf)] = order;
	}
}
bool XTPTradeHandler::GetAndCleanRecordOrder(Order *p_order, uint32_t order_ref, int session_id)
{
	char buf[256] = { 0 };
	snprintf(buf, sizeof(buf), "%s_%d_%d", conf_.user_id.c_str(), order_ref, session_id);
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

void XTPTradeHandler::ThrowXTPLastError(const char *tip_msg)
{
	char buf[512];
	XTPRI *ret = api_->GetApiLastError();
	if (ret)
	{
		snprintf(buf, sizeof(buf) - 1, "%s: xtp error: (%d) %s", tip_msg, ret->error_id, ret->error_msg);
	}
	else
	{
		snprintf(buf, sizeof(buf) - 1, "%s: xtp error: (%d) %s", tip_msg, 0, "");
	}
	throw std::runtime_error(buf);
}

void XTPTradeHandler::SerializeXTPOrderInsert(rapidjson::Writer<rapidjson::StringBuffer> &writer, XTPOrderInsertInfo &req)
{
	char buf[2] = { 0 };

	writer.Key("order_xtp_id");
	writer.Uint64(req.order_xtp_id);

	writer.Key("order_client_id");
	writer.Uint(req.order_client_id);

	writer.Key("ticker");
	writer.String(req.ticker);

	writer.Key("market");
	writer.Int((int)req.market);

	writer.Key("price");
	writer.Double(req.price);

	writer.Key("stop_price");
	writer.Double(req.stop_price);

	writer.Key("quantity");
	writer.Int64(req.quantity);

	writer.Key("price_type");
	writer.Int((int)req.price_type);

	writer.Key("side");
	writer.Uint((uint8_t)req.side);

	writer.Key("position_effect");
	writer.Uint((uint8_t)req.position_effect);

	writer.Key("business_type");
	writer.Int((int)req.business_type);
}
void XTPTradeHandler::SerializeXTPOrderInfo(rapidjson::Writer<rapidjson::StringBuffer> &writer, XTPOrderInfo *order_info)
{
	char buf[2] = { 0 };

	writer.Key("order_xtp_id");
	writer.Uint64(order_info->order_xtp_id);

	writer.Key("order_client_id");
	writer.Uint(order_info->order_client_id);

	writer.Key("order_cancel_client_id");
	writer.Uint(order_info->order_cancel_client_id);

	writer.Key("order_cancel_xtp_id");
	writer.Uint64(order_info->order_cancel_xtp_id);

	writer.Key("ticker");
	writer.String(order_info->ticker);

	writer.Key("market");
	writer.Int((int)order_info->market);

	writer.Key("price");
	writer.Double(order_info->price);

	writer.Key("quantity");
	writer.Int64(order_info->quantity);

	writer.Key("price_type");
	writer.Int((int)order_info->price_type);

	writer.Key("side");
	writer.Int((int)order_info->side);

	writer.Key("position_effect");
	writer.Int((int)order_info->position_effect);

	writer.Key("business_type");
	writer.Int((int)order_info->business_type);

	writer.Key("qty_traded");
	writer.Int64(order_info->qty_traded);

	writer.Key("qty_left");
	writer.Int64(order_info->qty_left);

	writer.Key("insert_time");
	writer.Int64(order_info->insert_time);

	writer.Key("update_time");
	writer.Int64(order_info->update_time);

	writer.Key("cancel_time");
	writer.Int64(order_info->cancel_time);

	writer.Key("trade_amount");
	writer.Double(order_info->trade_amount);

	writer.Key("order_local_id");
	writer.String(order_info->order_local_id);

	writer.Key("order_status");
	writer.Int((int)order_info->order_status);

	writer.Key("order_submit_status");
	writer.Int((int)order_info->order_submit_status);

	buf[0] = order_info->order_type;
	writer.Key("order_type");
	writer.String(buf);
}
void XTPTradeHandler::SerializeXTPTradeReport(rapidjson::Writer<rapidjson::StringBuffer> &writer, XTPTradeReport *trade_info)
{
	char buf[2] = { 0 };

	writer.Key("order_xtp_id");
	writer.Uint64(trade_info->order_xtp_id);

	writer.Key("order_client_id");
	writer.Uint(trade_info->order_client_id);

	writer.Key("ticker");
	writer.String(trade_info->ticker);

	writer.Key("market");
	writer.Int((int)trade_info->market);

	writer.Key("local_order_id");
	writer.Uint64(trade_info->local_order_id);

	writer.Key("exec_id");
	writer.String(trade_info->exec_id);

	writer.Key("price");
	writer.Double(trade_info->price);

	writer.Key("quantity");
	writer.Int64(trade_info->quantity);

	writer.Key("trade_time");
	writer.Int64(trade_info->trade_time);

	writer.Key("trade_amount");
	writer.Double(trade_info->trade_amount);

	writer.Key("report_index");
	writer.Uint64(trade_info->report_index);

	writer.Key("order_exch_id");
	writer.String(trade_info->order_exch_id);

	buf[0] = trade_info->trade_type;
	writer.Key("trade_type");
	writer.String(buf);

	writer.Key("side");
	writer.Int((int)trade_info->side);

	writer.Key("position_effect");
	writer.Int((int)trade_info->position_effect);

	writer.Key("business_type");
	writer.Int((int)trade_info->business_type);

	writer.Key("branch_pbu");
	writer.String(trade_info->branch_pbu);
}
void XTPTradeHandler::SerializeXTPQueryOrder(rapidjson::Writer<rapidjson::StringBuffer> &writer, XTPQueryOrderReq *qry)
{
	writer.Key("ticker");
	writer.String(qry->ticker);
	writer.Key("begin_time");
	writer.Int64(qry->begin_time);
	writer.Key("end_time");
	writer.Int64(qry->end_time);
}
void XTPTradeHandler::SerializeXTPQueryTrade(rapidjson::Writer<rapidjson::StringBuffer> &writer, XTPQueryTraderReq *qry)
{
	writer.Key("ticker");
	writer.String(qry->ticker);
	writer.Key("begin_time");
	writer.Int64(qry->begin_time);
	writer.Key("end_time");
	writer.Int64(qry->end_time);
}
void XTPTradeHandler::SerializeXTPPosition(rapidjson::Writer<rapidjson::StringBuffer> &writer, XTPQueryStkPositionRsp *rsp)
{
	writer.Key("ticker");
	writer.String(rsp->ticker);

	writer.Key("ticker_name");
	writer.String(rsp->ticker_name);

	writer.Key("market");
	writer.Int((int)rsp->market);

	writer.Key("total_qty");
	writer.Int64(rsp->total_qty);

	writer.Key("sellable_qty");
	writer.Int64(rsp->sellable_qty);

	writer.Key("avg_price");
	writer.Double(rsp->avg_price);

	writer.Key("unrealized_pnl");
	writer.Double(rsp->unrealized_pnl);

	writer.Key("yesterday_position");
	writer.Int64(rsp->yesterday_position);

	writer.Key("purchase_redeemable_qty");
	writer.Int64(rsp->purchase_redeemable_qty);

	writer.Key("position_direction");
	writer.Int((int)rsp->position_direction);

	writer.Key("executable_option");
	writer.Int64(rsp->executable_option);

	writer.Key("lockable_position");
	writer.Int64(rsp->lockable_position);

	writer.Key("executable_underlying");
	writer.Int64(rsp->executable_underlying);

	writer.Key("locked_position");
	writer.Int64(rsp->locked_position);

	writer.Key("usable_locked_position");
	writer.Int64(rsp->usable_locked_position);
}
void XTPTradeHandler::SerializeXTPAsset(rapidjson::Writer<rapidjson::StringBuffer> &writer, XTPQueryAssetRsp *rsp)
{
	writer.Key("total_asset");
	writer.Double(rsp->total_asset);
	writer.Key("buying_power");
	writer.Double(rsp->buying_power);
	writer.Key("security_asset");
	writer.Double(rsp->security_asset);
	writer.Key("fund_buy_amount");
	writer.Double(rsp->fund_buy_amount);
	writer.Key("fund_buy_fee");
	writer.Double(rsp->fund_buy_fee);
	writer.Key("fund_sell_amount");
	writer.Double(rsp->fund_sell_amount);
	writer.Key("fund_sell_fee");
	writer.Double(rsp->fund_sell_fee);
	writer.Key("withholding_amount");
	writer.Double(rsp->withholding_amount);
	writer.Key("account_type");
	writer.Int((int)rsp->account_type);
	writer.Key("frozen_margin");
	writer.Double(rsp->frozen_margin);
	writer.Key("frozen_exec_cash");
	writer.Double(rsp->frozen_exec_cash);
	writer.Key("pay_later");
	writer.Double(rsp->pay_later);
	writer.Key("preadva_pay");
	writer.Double(rsp->preadva_pay);
	writer.Key("orig_banlance");
	writer.Double(rsp->orig_banlance);
	writer.Key("banlance");
	writer.Double(rsp->banlance);
	writer.Key("deposit_withdraw");
	writer.Double(rsp->deposit_withdraw);
	writer.Key("trade_netting");
	writer.Double(rsp->trade_netting);
	writer.Key("captial_asset");
	writer.Double(rsp->captial_asset);
	writer.Key("force_freeze_amount");
	writer.Double(rsp->force_freeze_amount);
	writer.Key("preferred_amount");
	writer.Double(rsp->preferred_amount);
}

void XTPTradeHandler::OutputFrontDisconnected(uint64_t session_id, int reason)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("xtp_frontdisconnect");

	writer.Key("data");
	writer.StartObject();
	writer.Key("session_id");
	writer.Uint64(session_id);
	writer.Key("reason");
	writer.Int(reason);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void XTPTradeHandler::OutputOrderInsert(XTPOrderInsertInfo &req)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("xtp_orderinsert");

	writer.Key("data");
	writer.StartObject();
	SerializeXTPOrderInsert(writer, req);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void XTPTradeHandler::OutputOrderEvent(XTPOrderInfo *order_info, XTPRI *error_info, uint64_t session_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("xtp_orderevent");
	if (error_info)
	{
		writer.Key("error_id");
		writer.Int(error_info->error_id);
		writer.Key("error_msg");
		writer.String(error_info->error_msg);
	}
	else
	{
		writer.Key("error_id");
		writer.Int(0);
	}
	

	writer.Key("data");
	writer.StartObject();
	if (order_info)
	{
		SerializeXTPOrderInfo(writer, order_info);
	}
	writer.EndObject();	// end data
	writer.EndObject();	// end object
	LOG(INFO) << s.GetString();
}
void XTPTradeHandler::OutputTradeEvent(XTPTradeReport *trade_info, uint64_t session_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("xtp_tradeevent");

	writer.Key("data");
	writer.StartObject();
	if (trade_info)
	{
		SerializeXTPTradeReport(writer, trade_info);
	}
	writer.EndObject();	// end data
	writer.EndObject();	// end object
	LOG(INFO) << s.GetString();
}
void XTPTradeHandler::OutputOrderCancel(uint64_t order_xtp_id, uint64_t session_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("xtp_ordercancel");

	writer.Key("data");
	writer.StartObject();
	writer.Key("order_xtp_id");
	writer.Uint64(order_xtp_id);
	writer.Key("session_id");
	writer.Uint64(session_id);
	writer.EndObject();	// end data
	writer.EndObject();	// end object
	LOG(INFO) << s.GetString();
}
void XTPTradeHandler::OutputOrderQuery(uint64_t order_xtp_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("xtp_orderquery");

	writer.Key("data");
	writer.StartObject();
	writer.Key("order_xtp_id");
	writer.Uint64(order_xtp_id);
	writer.EndObject();	// end data
	writer.EndObject();	// end object
	LOG(INFO) << s.GetString();
}
void XTPTradeHandler::OutputOrderQuery(XTPQueryOrderReq *req)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("xtp_orderquery");

	writer.Key("data");
	writer.StartObject();
	if (req)
	{
		SerializeXTPQueryOrder(writer, req);
	}
	writer.EndObject();	// end data
	writer.EndObject();	// end object
	LOG(INFO) << s.GetString();
}
void XTPTradeHandler::OutputTradeQuery(uint64_t trade_xtp_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("xtp_tradequery");

	writer.Key("data");
	writer.StartObject();
	writer.Key("trade_xtp_id");
	writer.Uint64(trade_xtp_id);
	writer.EndObject();	// end data
	writer.EndObject();	// end object
	LOG(INFO) << s.GetString();
}
void XTPTradeHandler::OutputTradeQuery(XTPQueryTraderReq *req)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("xtp_tradequery");

	writer.Key("data");
	writer.StartObject();
	if (req)
	{
		SerializeXTPQueryTrade(writer, req);
	}
	writer.EndObject();	// end data
	writer.EndObject();	// end object
	LOG(INFO) << s.GetString();
}
void XTPTradeHandler::OutputPositionQuery(const std::string &symbol)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("xtp_positionquery");

	writer.Key("data");
	writer.StartObject();
	writer.Key("ticker");
	writer.String(symbol.c_str());
	writer.EndObject();	// end data
	writer.EndObject();	// end object
	LOG(INFO) << s.GetString();
}
void XTPTradeHandler::OutputAssetQuery(uint64_t session_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("xtp_assetquery");

	writer.Key("data");
	writer.StartObject();
	writer.Key("session_id");
	writer.Uint64(session_id);
	writer.EndObject();	// end data
	writer.EndObject();	// end object
	LOG(INFO) << s.GetString();
}

void XTPTradeHandler::OutputRspOrderQuery(XTPQueryOrderRsp *order_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("xtp_rspqryorder");
	writer.Key("req_id");
	writer.Int(request_id);
	writer.Key("is_last");
	writer.Bool(is_last);
	if (error_info)
	{
		writer.Key("error_id");
		writer.Int(error_info->error_id);
		writer.Key("error_msg");
		writer.String(error_info->error_msg);
	}
	else
	{
		writer.Key("error_id");
		writer.Int(0);
	}

	writer.Key("data");
	writer.StartObject();
	if (order_info)
	{
		SerializeXTPOrderInfo(writer, order_info);
	}
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void XTPTradeHandler::OutputRspTradeQuery(XTPQueryTradeRsp *trade_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("xtp_rspqrytrade");
	writer.Key("req_id");
	writer.Int(request_id);
	writer.Key("is_last");
	writer.Bool(is_last);
	if (error_info)
	{
		writer.Key("error_id");
		writer.Int(error_info->error_id);
		writer.Key("error_msg");
		writer.String(error_info->error_msg);
	}
	else
	{
		writer.Key("error_id");
		writer.Int(0);
	}

	writer.Key("data");
	writer.StartObject();
	if (trade_info)
	{
		SerializeXTPTradeReport(writer, trade_info);
	}
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void XTPTradeHandler::OutputRspPositionQuery(XTPQueryStkPositionRsp *position, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("xtp_rspqryposition");
	writer.Key("req_id");
	writer.Int(request_id);
	writer.Key("is_last");
	writer.Bool(is_last);
	if (error_info)
	{
		writer.Key("error_id");
		writer.Int(error_info->error_id);
		writer.Key("error_msg");
		writer.String(error_info->error_msg);
	}
	else
	{
		writer.Key("error_id");
		writer.Int(0);
	}

	writer.Key("data");
	writer.StartObject();
	if (position)
	{
		SerializeXTPPosition(writer, position);
	}
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void XTPTradeHandler::OutputRspAssetQuery(XTPQueryAssetRsp *asset, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("xtp_rspqryasset");
	writer.Key("req_id");
	writer.Int(request_id);
	writer.Key("is_last");
	writer.Bool(is_last);
	if (error_info)
	{
		writer.Key("error_id");
		writer.Int(error_info->error_id);
		writer.Key("error_msg");
		writer.String(error_info->error_msg);
	}
	else
	{
		writer.Key("error_id");
		writer.Int(0);
	}

	writer.Key("data");
	writer.StartObject();
	if (asset)
	{
		SerializeXTPAsset(writer, asset);
	}
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}