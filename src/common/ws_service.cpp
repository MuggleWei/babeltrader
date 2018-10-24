#include "ws_service.h"

#include <iostream>

#include "glog/logging.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "err.h"

WsService::WsService(QuoteService *quote_service, TradeService *trade_service)
	: quote_(quote_service)
	, trade_(trade_service)
{
	RegisterCallbacks();
}

void WsService::onConnection(uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest &req)
{
	LOG(INFO) << "ws connection: " << ws->getAddress().address << ":" << ws->getAddress().port << ", url: " << req.getUrl().toString() << std::endl;
	if (req.getUrl().toString() != "/ws") {
		ws->close();
	}
}
void WsService::onDisconnection(uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length)
{
	LOG(INFO) << "ws disconnection: " << ws->getAddress().address << ":" << ws->getAddress().port << std::endl;
}
void WsService::onMessage(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode)
{
	rapidjson::Document doc;
	doc.Parse(message, length);

	if (doc.HasParseError()) {
		OnClientMsgError(ws, message, length, 
			BABELTRADER_ERR_WSREQ_FAILED_PARSE, 
			BABELTRADER_ERR_MSG[BABELTRADER_ERR_WSREQ_FAILED_PARSE - BABELTRADER_ERR_BEGIN]);
		return;
	}

	try
	{
		if (!doc["msg"].IsString()) throw std::runtime_error("field \"msg\" need string");

		auto it = callbacks_.find(doc["msg"].GetString());
		if (it != callbacks_.end())
		{
			Dispatch(ws, doc);
		}
		else
		{
			OnClientMsgError(ws, message, length,
				BABELTRADER_ERR_WSREQ_NOT_HANDLE,
				BABELTRADER_ERR_MSG[BABELTRADER_ERR_WSREQ_NOT_HANDLE - BABELTRADER_ERR_BEGIN]);
			OnReqDefault(ws, doc);
		}
	}
	catch (std::exception &e)
	{
		auto error_msg = std::string(
			BABELTRADER_ERR_MSG[BABELTRADER_ERR_WSREQ_FAILED_HANDLE - BABELTRADER_ERR_BEGIN]) + std::string(" - ") + e.what();
		OnClientMsgError(ws, message, length,
			BABELTRADER_ERR_WSREQ_FAILED_HANDLE,
			error_msg.c_str());
	}
}

void WsService::RegisterCallbacks()
{
	default_callback_ = std::bind(&WsService::OnReqDefault, this, std::placeholders::_1, std::placeholders::_2);
	
	callbacks_["insert_order"] = std::bind(&WsService::OnReqInsertOrder, this, std::placeholders::_1, std::placeholders::_2);
	callbacks_["cancel_order"] = std::bind(&WsService::OnReqCancelOrder, this, std::placeholders::_1, std::placeholders::_2);
}
void WsService::Dispatch(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	auto it = callbacks_.find(doc["msg"].GetString());
	if (it != callbacks_.end())
	{
		it->second(ws, doc);
	}
	else
	{
		default_callback_(ws, doc);
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

	ws->send(s.GetString());
}

void WsService::OnReqDefault(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	// do nothing
}
void WsService::OnReqInsertOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	trade_->InsertOrder(ws, doc);
}
void WsService::OnReqCancelOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	trade_->CancelOrder(ws, doc);
}