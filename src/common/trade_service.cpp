#include "trade_service.h"

#include "glog/logging.h"

#include "converter.h"
#include "ws_service.h"

namespace babeltrader
{

void TradeService::OnReqInsertOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	if (!(doc.HasMember("data") && doc["data"].IsObject())) {
		throw std::runtime_error("field \"data\" need object");
	}

	Order order = ConvertOrderJson2Common(doc["data"]);
	this->InsertOrder(ws, order);
}
void TradeService::OnReqCancelOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	if (!(doc.HasMember("data") && doc["data"].IsObject())) {
		throw std::runtime_error("field \"data\" need object");
	}

	Order order = ConvertOrderJson2Common(doc["data"]);
	this->CancelOrder(ws, order);
}
void TradeService::OnReqQueryOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	if (!(doc.HasMember("data") && doc["data"].IsObject())) {
		throw std::runtime_error("field \"data\" need object");
	}

	OrderQuery order_qry = ConvertOrderQueryJson2Common(doc["data"]);
	this->QueryOrder(ws, order_qry);
}
void TradeService::OnReqQueryTrade(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	if (!(doc.HasMember("data") && doc["data"].IsObject())) {
		throw std::runtime_error("field \"data\" need object");
	}

	TradeQuery trade_qry = ConvertTradeQueryJson2Common(doc["data"]);
	this->QueryTrade(ws, trade_qry);
}
void TradeService::OnReqQueryPosition(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	if (!(doc.HasMember("data") && doc["data"].IsObject())) {
		throw std::runtime_error("field \"data\" need object");
	}

	PositionQuery position_qry = ConvertPositionQueryJson2Common(doc["data"]);
	this->QueryPosition(ws, position_qry);
}
void TradeService::OnReqQueryPositionDetail(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	if (!(doc.HasMember("data") && doc["data"].IsObject())) {
		throw std::runtime_error("field \"data\" need object");
	}

	PositionQuery position_qry = ConvertPositionQueryJson2Common(doc["data"]);
	this->QueryPositionDetail(ws, position_qry);
}
void TradeService::OnReqQueryTradeAccount(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	if (!(doc.HasMember("data") && doc["data"].IsObject())) {
		throw std::runtime_error("field \"data\" need object");
	}

	TradeAccountQuery trade_account_qry = ConvertTradeAccountJson2Common(doc["data"]);
	this->QueryTradeAccount(ws, trade_account_qry);
}
void TradeService::OnReqQueryProduct(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	if (!(doc.HasMember("data") && doc["data"].IsObject())) {
		throw std::runtime_error("field \"data\" need object");
	}

	ProductQuery product_qry = ConvertProductQueryJson2Common(doc["data"]);
	this->QueryProduct(ws, product_qry);
}


void TradeService::BroadcastConfirmOrder(uWS::Hub &hub, Order &order, int error_id, const char *error_msg)
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
void TradeService::BroadcastOrderStatus(uWS::Hub &hub, Order &order, OrderStatusNotify &order_status_notify, int error_id, const char *error_msg)
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
void TradeService::BroadcastOrderDeal(uWS::Hub &hub, Order &order, OrderDealNotify &order_deal)
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
void TradeService::RspOrderQry(uWS::WebSocket<uWS::SERVER>* ws, OrderQuery &order_qry, std::vector<Order> &orders, std::vector<OrderStatusNotify> &order_status, int error_id)
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

	ws_service_->SendMsgToClient(ws, s.GetString());
}
void TradeService::RspTradeQry(uWS::WebSocket<uWS::SERVER>* ws, TradeQuery &trade_qry, std::vector<Order> &orders, std::vector<OrderDealNotify> &order_deal, int error_id)
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

	ws_service_->SendMsgToClient(ws, s.GetString());
}

void TradeService::RspPositionQryType1(uWS::WebSocket<uWS::SERVER>* ws, PositionQuery &position_qry, std::vector<PositionSummaryType1> &positions, int error_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rsp_qryposition");
	writer.Key("error_id");
	writer.Int(error_id);

	writer.Key("data");
	writer.StartObject();

	SerializePositionQuery(writer, position_qry);

	writer.Key("position_summary_type");
	writer.String("type1");

	writer.Key("data");
	writer.StartArray();

	for (auto i = 0; i < positions.size(); i++)
	{
		writer.StartObject();
		SerializePositionSummaryType1(writer, positions[i]);
		writer.EndObject();
	}

	writer.EndArray();  // positions

	writer.EndObject();  // data end

	writer.EndObject();  // object end

	LOG(INFO) << s.GetString();

	ws_service_->SendMsgToClient(ws, s.GetString());
}
void TradeService::RspPositionDetailQryType1(uWS::WebSocket<uWS::SERVER>* ws, PositionQuery &position_qry, std::vector<PositionDetailType1> &positions, int error_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rsp_qrypositiondetail");
	writer.Key("error_id");
	writer.Int(error_id);

	writer.Key("data");
	writer.StartObject();

	SerializePositionQuery(writer, position_qry);

	writer.Key("position_detail_type");
	writer.String("type1");

	writer.Key("data");
	writer.StartArray();

	for (auto i = 0; i < positions.size(); i++)
	{
		writer.StartObject();
		SerializePositionDetailType1(writer, positions[i]);
		writer.EndObject();
	}

	writer.EndArray();  // positions

	writer.EndObject();  // data end

	writer.EndObject();  // object end

	LOG(INFO) << s.GetString();

	ws_service_->SendMsgToClient(ws, s.GetString());
}
void TradeService::RspTradeAccountQryType1(uWS::WebSocket<uWS::SERVER>* ws, TradeAccountQuery &tradeaccount_qry, std::vector<TradeAccountType1> &trade_accounts, int error_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rsp_qrytradeaccount");
	writer.Key("error_id");
	writer.Int(error_id);

	writer.Key("data");
	writer.StartObject();

	SerializeTradeAccountQuery(writer, tradeaccount_qry);

	writer.Key("trade_account_type");
	writer.String("type1");

	writer.Key("data");
	writer.StartArray();

	for (auto i = 0; i < trade_accounts.size(); i++)
	{
		writer.StartObject();
		SerializeTradeAccountType1(writer, trade_accounts[i]);
		writer.EndObject();
	}

	writer.EndArray();  // positions

	writer.EndObject();  // data end

	writer.EndObject();  // object end

	LOG(INFO) << s.GetString();

	ws_service_->SendMsgToClient(ws, s.GetString());
}
void TradeService::RspProductQryType1(uWS::WebSocket<uWS::SERVER>* ws, ProductQuery &product_qry, std::vector<ProductType1> &product_types, int error_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rsp_qryproduct");
	writer.Key("error_id");
	writer.Int(error_id);

	writer.Key("data");
	writer.StartObject();

	SerializeProductQuery(writer, product_qry);

	writer.Key("product_type");
	writer.String("type1");

	writer.Key("data");
	writer.StartArray();

	for (auto i = 0; i < product_types.size(); i++)
	{
		writer.StartObject();
		SerializeProductType1(writer, product_types[i]);
		writer.EndObject();
	}

	writer.EndArray();  // positions

	writer.EndObject();  // data end

	writer.EndObject();  // object end

	LOG(INFO) << s.GetString();

	ws_service_->SendMsgToClient(ws, s.GetString());
}

void TradeService::RspPositionQryType2(uWS::WebSocket<uWS::SERVER>* ws, PositionQuery &position_qry, std::vector<PositionSummaryType2> &positions, int error_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rsp_qryposition");
	writer.Key("error_id");
	writer.Int(error_id);

	writer.Key("data");
	writer.StartObject();

	SerializePositionQuery(writer, position_qry);

	writer.Key("position_summary_type");
	writer.String("type2");

	writer.Key("data");
	writer.StartArray();

	for (auto i = 0; i < positions.size(); i++)
	{
		writer.StartObject();
		SerializePositionSummaryType2(writer, positions[i]);
		writer.EndObject();
	}

	writer.EndArray();  // positions

	writer.EndObject();  // data end

	writer.EndObject();  // object end

	LOG(INFO) << s.GetString();

	ws_service_->SendMsgToClient(ws, s.GetString());
}
void TradeService::RspTradeAccountQryType2(uWS::WebSocket<uWS::SERVER>* ws, TradeAccountQuery &tradeaccount_qry, std::vector<TradeAccountType2> &trade_accounts, int error_id)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rsp_qrytradeaccount");
	writer.Key("error_id");
	writer.Int(error_id);

	writer.Key("data");
	writer.StartObject();

	SerializeTradeAccountQuery(writer, tradeaccount_qry);

	writer.Key("trade_account_type");
	writer.String("type2");

	writer.Key("data");
	writer.StartArray();

	for (auto i = 0; i < trade_accounts.size(); i++)
	{
		writer.StartObject();
		SerializeTradeAccountType2(writer, trade_accounts[i]);
		writer.EndObject();
	}

	writer.EndArray();  // positions

	writer.EndObject();  // data end

	writer.EndObject();  // object end

	LOG(INFO) << s.GetString();

	ws_service_->SendMsgToClient(ws, s.GetString());
}


}