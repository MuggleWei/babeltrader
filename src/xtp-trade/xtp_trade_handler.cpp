#include "xtp_trade_handler.h"

#include "glog/logging.h"

#include "common/converter.h"

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
		Order ret_order;
		GetAndCleanRecordOrder(ret_order, req.order_client_id, xtp_session_id_);
		ThrowXTPLastError("failed insert order");
	}
}
void XTPTradeHandler::CancelOrder(uWS::WebSocket<uWS::SERVER> *ws, Order &order)
{}
void XTPTradeHandler::QueryOrder(uWS::WebSocket<uWS::SERVER> *ws, OrderQuery &order_query)
{}
void XTPTradeHandler::QueryTrade(uWS::WebSocket<uWS::SERVER> *ws, TradeQuery &trade_query)
{}
void XTPTradeHandler::QueryPosition(uWS::WebSocket<uWS::SERVER> *ws, PositionQuery &position_query)
{}
void XTPTradeHandler::QueryPositionDetail(uWS::WebSocket<uWS::SERVER> *ws, PositionQuery &position_query)
{}
void XTPTradeHandler::QueryTradeAccount(uWS::WebSocket<uWS::SERVER> *ws, TradeAccountQuery &tradeaccount_query)
{}
void XTPTradeHandler::QueryProduct(uWS::WebSocket<uWS::SERVER> *ws, ProductQuery &query_product)
{}


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


void XTPTradeHandler::RecordOrder(Order &order, uint32_t order_ref, uint64_t session_id)
{
	char buf[256] = { 0 };
	snprintf(buf, sizeof(buf), "%s_%d_%d", conf_.user_id.c_str(), order_ref, session_id);
	{
		std::unique_lock<std::mutex> lock(wati_deal_order_mtx_);
		wait_deal_orders_[std::string(buf)] = order;
	}
}
bool XTPTradeHandler::GetAndCleanRecordOrder(Order &order, uint32_t order_ref, int session_id)
{
	char buf[256] = { 0 };
	snprintf(buf, sizeof(buf), "%s_%d_%d", conf_.user_id.c_str(), order_ref, session_id);
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