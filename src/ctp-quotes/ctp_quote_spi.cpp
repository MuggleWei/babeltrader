#include "ctp_quote_spi.h"

#include <iostream>

#include "glog/logging.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

CTPQuoteHandler::CTPQuoteHandler(CThostFtdcMdApi *api, CTPQuoteConf &conf)
	: api_(api)
	, conf_(conf)
	, req_id_(1)
{}

void CTPQuoteHandler::OnFrontConnected()
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("connected");
	writer.EndObject();
	LOG(INFO) << s.GetString();

	// user login
	CThostFtdcReqUserLoginField req_user_login = { 0 };
	strncpy(req_user_login.BrokerID, conf_.broker_id.c_str(), sizeof(req_user_login.BrokerID) - 1);
	strncpy(req_user_login.UserID, conf_.user_id.c_str(), sizeof(req_user_login.UserID) - 1);
	strncpy(req_user_login.Password, conf_.password.c_str(), sizeof(req_user_login.Password) - 1);
	api_->ReqUserLogin(&req_user_login, req_id_++);
}
void CTPQuoteHandler::OnFrontDisconnected(int nReason)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("disconnected");
	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPQuoteHandler::OnHeartBeatWarning(int nTimeLapse)
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

void CTPQuoteHandler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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

	// sub default topics
	char buf[16][64];
	char* topics[16];
	for (int i = 0; i < 16; i++) {
		topics[i] = buf[i];
	}

	int cnt = 0;
	for (auto topic : conf_.default_sub_topics) {
		memset(buf[cnt], 0, sizeof(buf[cnt]));
		strncpy(buf[cnt++], topic.c_str(), sizeof(buf[0]) - 1);
		if (cnt == 16) {
			api_->SubscribeMarketData(topics, cnt);
			cnt = 0;
		}
	}

	if (cnt != 0) {
		api_->SubscribeMarketData(topics, cnt);
	}
}
void CTPQuoteHandler::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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

void CTPQuoteHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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

void CTPQuoteHandler::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspsubmarketdata");
	writer.Key("req_id");
	writer.Int(nRequestID);
	writer.Key("error_id");
	writer.Int(pRspInfo->ErrorID);
	writer.Key("error_msg");
	writer.String(pRspInfo->ErrorMsg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("InstrumentID");
	writer.String(pSpecificInstrument->InstrumentID);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPQuoteHandler::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("rspunsubmarketdata");
	writer.Key("req_id");
	writer.Int(nRequestID);
	writer.Key("error_id");
	writer.Int(pRspInfo->ErrorID);
	writer.Key("error_msg");
	writer.String(pRspInfo->ErrorMsg);

	writer.Key("data");
	writer.StartObject();
	writer.Key("InstrumentID");
	writer.String(pSpecificInstrument->InstrumentID);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}

void CTPQuoteHandler::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}
void CTPQuoteHandler::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CTPQuoteHandler::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("marketdata");

	writer.Key("data");
	writer.StartObject();
	writer.Key("TradingDay");
	writer.String(pDepthMarketData->TradingDay);
	writer.Key("InstrumentID");
	writer.String(pDepthMarketData->InstrumentID);
	writer.Key("ExchangeID");
	writer.String(pDepthMarketData->ExchangeID);
	writer.Key("ExchangeInstID");
	writer.String(pDepthMarketData->ExchangeInstID);
	writer.Key("LastPrice");
	writer.Double(pDepthMarketData->LastPrice);
	writer.Key("PreSettlementPrice");
	writer.Double(pDepthMarketData->PreSettlementPrice);
	writer.Key("PreClosePrice");
	writer.Double(pDepthMarketData->PreClosePrice);
	writer.Key("PreOpenInterest");
	writer.Double(pDepthMarketData->PreOpenInterest);
	writer.Key("OpenPrice");
	writer.Double(pDepthMarketData->OpenPrice);
	writer.Key("HighestPrice");
	writer.Double(pDepthMarketData->HighestPrice);
	writer.Key("LowestPrice");
	writer.Double(pDepthMarketData->LowestPrice);
	writer.Key("Volume");
	writer.Double(pDepthMarketData->Volume);
	writer.Key("Turnover");
	writer.Double(pDepthMarketData->Turnover);
	writer.Key("OpenInterest");
	writer.Double(pDepthMarketData->OpenInterest);
	writer.Key("ClosePrice");
	writer.Double(pDepthMarketData->ClosePrice);
	writer.Key("SettlementPrice");
	writer.Double(pDepthMarketData->SettlementPrice);
	writer.Key("UpperLimitPrice");
	writer.Double(pDepthMarketData->UpperLimitPrice);
	writer.Key("LowerLimitPrice");
	writer.Double(pDepthMarketData->LowerLimitPrice);
	writer.Key("PreDelta");
	writer.Double(pDepthMarketData->PreDelta);
	writer.Key("CurrDelta");
	writer.Double(pDepthMarketData->CurrDelta);
	writer.Key("UpdateTime");
	writer.String(pDepthMarketData->UpdateTime);
	writer.Key("UpdateMillisec");
	writer.Int(pDepthMarketData->UpdateMillisec);
	writer.Key("BidPrice1");
	writer.Double(pDepthMarketData->BidPrice1);
	writer.Key("BidVolume1");
	writer.Int(pDepthMarketData->BidVolume1);
	writer.Key("AskPrice1");
	writer.Double(pDepthMarketData->AskPrice1);
	writer.Key("AskVolume1");
	writer.Int(pDepthMarketData->AskVolume1);
	writer.Key("BidPrice2");
	writer.Double(pDepthMarketData->BidPrice2);
	writer.Key("BidVolume2");
	writer.Int(pDepthMarketData->BidVolume2);
	writer.Key("AskPrice2");
	writer.Double(pDepthMarketData->AskPrice2);
	writer.Key("AskVolume2");
	writer.Int(pDepthMarketData->AskVolume2);
	writer.Key("BidPrice3");
	writer.Double(pDepthMarketData->BidPrice3);
	writer.Key("BidVolume3");
	writer.Int(pDepthMarketData->BidVolume3);
	writer.Key("AskPrice3");
	writer.Double(pDepthMarketData->AskPrice3);
	writer.Key("AskVolume3");
	writer.Int(pDepthMarketData->AskVolume3);
	writer.Key("BidPrice4");
	writer.Double(pDepthMarketData->BidPrice4);
	writer.Key("BidVolume4");
	writer.Int(pDepthMarketData->BidVolume4);
	writer.Key("AskPrice4");
	writer.Double(pDepthMarketData->AskPrice4);
	writer.Key("AskVolume4");
	writer.Int(pDepthMarketData->AskVolume4);
	writer.Key("BidPrice5");
	writer.Double(pDepthMarketData->BidPrice5);
	writer.Key("BidVolume5");
	writer.Int(pDepthMarketData->BidVolume5);
	writer.Key("AskPrice5");
	writer.Double(pDepthMarketData->AskPrice5);
	writer.Key("AskVolume5");
	writer.Int(pDepthMarketData->AskVolume5);
	writer.Key("AveragePrice");
	writer.Double(pDepthMarketData->AveragePrice);
	writer.Key("ActionDay");
	writer.String(pDepthMarketData->ActionDay);
	writer.EndObject();

	writer.EndObject();
	LOG(INFO) << s.GetString();
}
void CTPQuoteHandler::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) {}