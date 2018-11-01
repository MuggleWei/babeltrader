#include "ws_service.h"

#include <iostream>
#include <queue>

#include "glog/logging.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "err.h"
#include "converter.h"


namespace babeltrader
{


WsService::WsService(QuoteService *quote_service, TradeService *trade_service)
	: quote_(quote_service)
	, trade_(trade_service)
{
	RegisterCallbacks();

	std::thread th(&WsService::MessageLoop, this);
	th.detach();
}

void WsService::onConnection(uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest &req)
{
	LOG(INFO) << "ws connection: " << ws->getAddress().address << ":" << ws->getAddress().port << ", url: " << req.getUrl().toString() << std::endl;
	if (req.getUrl().toString() != "/ws")
	{
		ws->close();
	} 
	else
	{
		std::unique_lock<std::mutex> lock(ws_mtx_);
		ws_set_.insert(ws);
	}
}
void WsService::onDisconnection(uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length)
{
	LOG(INFO) << "ws disconnection: " << ws->getAddress().address << ":" << ws->getAddress().port << std::endl;
	{
		std::unique_lock<std::mutex> lock(ws_mtx_);
		ws_set_.erase(ws);
	}
}
void WsService::onMessage(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode)
{
	LOG(INFO).write(message, length);

	rapidjson::Document doc;
	doc.Parse(message, length);

	if (doc.HasParseError()) {
		OnClientMsgError(ws, message, length, 
			BABELTRADER_ERR_WSREQ_FAILED_PARSE, 
			BABELTRADER_ERR_MSG[BABELTRADER_ERR_WSREQ_FAILED_PARSE - BABELTRADER_ERR_BEGIN]);
		return;
	}

	int ret = PutMsg(ws, std::move(doc));
	if (ret != 0) {
		OnClientMsgError(ws, message, length,
			BABELTRADER_ERR_WSREQ_FAILED_TUNNEL,
			BABELTRADER_ERR_MSG[BABELTRADER_ERR_WSREQ_FAILED_TUNNEL - BABELTRADER_ERR_BEGIN]);
	}
}

int WsService::PutMsg(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &&doc)
{
	WsTunnelMsg msg(ws, std::move(doc));

	auto ret = msg_tunnel_.Write(std::move(msg));
	if (ret != muggle::TUNNEL_SUCCESS) {
		return -1;
	}

	return 0;
}

void WsService::SendMsgToClient(uWS::WebSocket<uWS::SERVER> *ws, const char *msg)
{
	std::unique_lock<std::mutex> lock(ws_mtx_);
	if (ws_set_.find(ws) != ws_set_.end()) {
		ws->send(msg);
	}
}

void WsService::BroadcastConfirmOrder(uWS::Hub &hub, Order &order, int error_id, const char *error_msg)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("confirmorder");
	writer.Key("error_id");
	writer.Int(error_id);
	// don't response error message, when error_msg is GB2312 may lead json loads error in python
	// writer.Key("error_msg");
	// writer.String(error_msg);

	writer.Key("data");
	writer.StartObject();
	SerializeOrder(writer, order);
	writer.EndObject();

	writer.EndObject();

	LOG(INFO) << s.GetString();

	hub.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void WsService::BroadcastOrderStatus(uWS::Hub &hub, Order &order, OrderStatusNotify &order_status_notify, int error_id, const char *error_msg)
{
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

	hub.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void WsService::BroadcastOrderDeal(uWS::Hub &hub, Order &order, OrderDealNotify &order_deal)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("orderdeal");
	writer.Key("error_id");
	writer.Int(0);

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

	hub.getDefaultGroup<uWS::SERVER>().broadcast(s.GetString(), s.GetLength(), uWS::OpCode::TEXT);
}
void WsService::RspOrderQry(uWS::WebSocket<uWS::SERVER>* ws, OrderQuery &order_qry, std::vector<Order> &orders, std::vector<OrderStatusNotify> &order_status, int error_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rsp_qryorder");
	writer.Key("error_id");
	writer.Int(error_id);

	writer.Key("data");
	writer.StartObject();

	SerializeOrderQuery(writer, order_qry);

	writer.Key("data");
	writer.StartArray();

	for (auto i = 0; i < orders.size(); i++)
	{
		writer.StartObject();
		SerializeOrderStatus(writer, order_status[i]);

		writer.Key("order");
		writer.StartObject();
		SerializeOrder(writer, orders[i]);
		writer.EndObject();  // order end

		writer.EndObject();  // order status end
	}

	writer.EndArray();  // orders

	writer.EndObject();  // data end

	writer.EndObject();  // object end

	LOG(INFO) << s.GetString();

	SendMsgToClient(ws, s.GetString());
}
void WsService::RspTradeQry(uWS::WebSocket<uWS::SERVER>* ws, TradeQuery &trade_qry, std::vector<Order> &orders, std::vector<OrderDealNotify> &order_deal, int error_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rsp_qrytrade");
	writer.Key("error_id");
	writer.Int(error_id);

	writer.Key("data");
	writer.StartObject();

	SerializeTradeQuery(writer, trade_qry);

	writer.Key("data");
	writer.StartArray();

	for (auto i = 0; i < orders.size(); i++)
	{
		writer.StartObject();
		SerializeOrderDeal(writer, order_deal[i]);

		writer.Key("order");
		writer.StartObject();
		SerializeOrder(writer, orders[i]);
		writer.EndObject();  // order end

		writer.EndObject();  // order deal end
	}

	writer.EndArray();  // orders

	writer.EndObject();  // data end

	writer.EndObject();  // object end

	LOG(INFO) << s.GetString();

	SendMsgToClient(ws, s.GetString());
}

void WsService::MessageLoop()
{
	std::queue<WsTunnelMsg> queue;
	while (true) {
		msg_tunnel_.Read(queue, true);
		while (queue.size()) {
			WsTunnelMsg &msg = queue.front();
			Dispatch(msg.ws_, msg.doc_);
			queue.pop();
		}
	}
}

void WsService::RegisterCallbacks()
{
	callbacks_["insert_order"] = std::bind(&WsService::OnReqInsertOrder, this, std::placeholders::_1, std::placeholders::_2);
	callbacks_["cancel_order"] = std::bind(&WsService::OnReqCancelOrder, this, std::placeholders::_1, std::placeholders::_2);
	callbacks_["query_order"] = std::bind(&WsService::OnReqQueryOrder, this, std::placeholders::_1, std::placeholders::_2);
	callbacks_["query_trade"] = std::bind(&WsService::OnReqQueryTrade, this, std::placeholders::_1, std::placeholders::_2);
}
void WsService::Dispatch(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	try
	{
		if (!doc["msg"].IsString()) throw std::runtime_error("field \"msg\" need string");

		auto it = callbacks_.find(doc["msg"].GetString());
		if (it != callbacks_.end())
		{
			it->second(ws, doc);
		}
		else
		{
			OnClientMsgError(ws, doc,
				BABELTRADER_ERR_WSREQ_NOT_HANDLE,
				BABELTRADER_ERR_MSG[BABELTRADER_ERR_WSREQ_NOT_HANDLE - BABELTRADER_ERR_BEGIN]);
		}
	}
	catch (std::exception &e)
	{
		// get error message
		auto error_msg = std::string(
			BABELTRADER_ERR_MSG[BABELTRADER_ERR_WSREQ_FAILED_HANDLE - BABELTRADER_ERR_BEGIN]) + std::string(" - ") + e.what();

		// response error
		OnClientMsgError(ws, doc,
			BABELTRADER_ERR_WSREQ_FAILED_HANDLE,
			error_msg.c_str());
	}
}

void WsService::OnClientMsgError(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, int error_id, const char  *error_msg)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("error");
	writer.Key("error_id");
	writer.Int(error_id);
	writer.Key("error_msg");
	writer.String(error_msg);
	writer.Key("data");
	writer.String(message, length, true);
	writer.EndObject();

	LOG(INFO) << s.GetString();

	{
		std::unique_lock<std::mutex> lock(ws_mtx_);
		if (ws_set_.find(ws) != ws_set_.end()) {
			ws->send(s.GetString());
		}
	}
}
void WsService::OnClientMsgError(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc, int error_id, const char  *error_msg)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("error");
	writer.Key("error_id");
	writer.Int(error_id);
	writer.Key("error_msg");
	writer.String(error_msg);
	writer.Key("data");
	doc.Accept(writer);
	writer.EndObject();

	LOG(INFO) << s.GetString();

	{
		std::unique_lock<std::mutex> lock(ws_mtx_);
		if (ws_set_.find(ws) != ws_set_.end()) {
			ws->send(s.GetString());
		}
	}
}

void WsService::OnReqInsertOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	if (!(doc.HasMember("data") && doc["data"].IsObject())) {
		throw std::runtime_error("field \"data\" need object");
	}

	Order order = ConvertOrderJson2Common(doc["data"]);
	trade_->InsertOrder(ws, order);
}
void WsService::OnReqCancelOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	if (!(doc.HasMember("data") && doc["data"].IsObject())) {
		throw std::runtime_error("field \"data\" need object");
	}

	Order order = ConvertOrderJson2Common(doc["data"]);
	trade_->CancelOrder(ws, order);
}
void WsService::OnReqQueryOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	if (!(doc.HasMember("data") && doc["data"].IsObject())) {
		throw std::runtime_error("field \"data\" need object");
	}

	OrderQuery order_qry = ConvertOrderQueryJson2Common(doc["data"]);
	trade_->QueryOrder(ws, order_qry);
}
void WsService::OnReqQueryTrade(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	if (!(doc.HasMember("data") && doc["data"].IsObject())) {
		throw std::runtime_error("field \"data\" need object");
	}

	TradeQuery trade_qry = ConvertTradeQueryJson2Common(doc["data"]);
	trade_->QueryTrade(ws, trade_qry);
}


}