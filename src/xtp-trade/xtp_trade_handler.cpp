#include "xtp_trade_handler.h"

#include "glog/logging.h"

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
	// TODO: 
}

XTP_MARKET_TYPE XTPTradeHandler::ConvertExchangeMarketTypeCommon2XTP(const std::string &exchange)
{
	if (exchange == g_exchanges[Exchange_SSE])
	{
		return XTP_MKT_SH_A;
	}
	else if (exchange == g_exchanges[Exchange_SZSE])
	{
		return XTP_MKT_SZ_A;
	}
	return XTP_MKT_UNKNOWN;
}
XTP_PRICE_TYPE XTPTradeHandler::ConvertOrderTypeCommon2XTP(Order &order)
{
	// TODO:
	return XTP_PRICE_TYPE_UNKNOWN;
}