#include "ctp_quote_spi.h"

#include <iostream>

CTPQuoteHandler::CTPQuoteHandler(CThostFtdcMdApi *api, CTPQuoteConf &conf)
	: api_(api)
	, conf_(conf)
	, req_id_(1)
{}

void CTPQuoteHandler::OnFrontConnected()
{
	std::cout << "Front connected" << std::endl;

	// user login
	CThostFtdcReqUserLoginField req_user_login = { 0 };
	strncpy(req_user_login.BrokerID, conf_.broker_id.c_str(), sizeof(req_user_login.BrokerID) - 1);
	strncpy(req_user_login.UserID, conf_.user_id.c_str(), sizeof(req_user_login.UserID) - 1);
	strncpy(req_user_login.Password, conf_.password.c_str(), sizeof(req_user_login.Password) - 1);
	api_->ReqUserLogin(&req_user_login, req_id_++);
}
void CTPQuoteHandler::OnFrontDisconnected(int nReason)
{
	std::cout << "Front disconnected" << std::endl;
}
void CTPQuoteHandler::OnHeartBeatWarning(int nTimeLapse)
{
	std::cout << "Heartbeat warning: " << nTimeLapse << std::endl;
}

void CTPQuoteHandler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "{"
		<< "\"msg\":\"OnRspUserLogin\","
		<< "\"req_id\":" << nRequestID << ","
		<< "\"error_id\":" << pRspInfo->ErrorID << ","
		<< "\"error_msg\":" << "\"" << pRspInfo->ErrorMsg << "\"" << ","
		<< "\"data\":{"
		<< "\"trading_day\":" << "\"" << pRspUserLogin->TradingDay << "\"" << ","
		<< "\"login_time\":" << "\"" << pRspUserLogin->LoginTime << "\"" << ","
		<< "\"broker_id\":" << "\"" << pRspUserLogin->BrokerID << "\"" << ","
		<< "\"user_id\":" << "\"" << pRspUserLogin->UserID << "\"" << ","
		<< "\"system_name\":" << "\"" << pRspUserLogin->SystemName << "\"" << ","
		<< "\"front_id\":" << pRspUserLogin->FrontID << ","
		<< "\"SessionID\":" << pRspUserLogin->SessionID << ","
		<< "\"max_order_ref\":" << "\"" << pRspUserLogin->MaxOrderRef << "\"" << ","
		<< "\"SHFETime\":" << "\"" << pRspUserLogin->SHFETime << "\"" << ","
		<< "\"DCETime\":" << "\"" << pRspUserLogin->DCETime << "\"" << ","
		<< "\"CZCETime\":" << "\"" << pRspUserLogin->CZCETime << "\"" << ","
		<< "\"FFEXTime\":" << "\"" << pRspUserLogin->FFEXTime << "\"" << ","
		<< "\"INETime\":" << "\"" << pRspUserLogin->INETime << "\""
		<< "}"
		<< "}" << std::endl;

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
	std::cout << "{"
		<< "\"msg\":\"OnRspUserLogout\","
		<< "\"req_id\":" << nRequestID << ","
		<< "\"error_id\":" << pRspInfo->ErrorID << ","
		<< "\"error_msg\":" << "\"" << pRspInfo->ErrorMsg << "\"" << ","
		<< "}" << std::endl;
}

void CTPQuoteHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "{"
		<< "\"msg\":\"OnRspError\","
		<< "\"req_id\":" << nRequestID << ","
		<< "\"error_id\":" << pRspInfo->ErrorID << ","
		<< "\"error_msg\":" << "\"" << pRspInfo->ErrorMsg << "\"" << ","
		<< "}" << std::endl;
}

void CTPQuoteHandler::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "{"
		<< "\"msg\":\"OnRspSubMarketData\","
		<< "\"req_id\":" << nRequestID << ","
		<< "\"error_id\":" << pRspInfo->ErrorID << ","
		<< "\"error_msg\":" << "\"" << pRspInfo->ErrorMsg << "\"" << ","
		<< "\"data\":{"
		<< "\"instrument_id\":" << "\"" << pSpecificInstrument->InstrumentID << "\"" << ","
		<< "}"
		<< "}" << std::endl;
}
void CTPQuoteHandler::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "{"
		<< "\"msg\":\"OnRspUnSubMarketData\","
		<< "\"req_id\":" << nRequestID << ","
		<< "\"error_id\":" << pRspInfo->ErrorID << ","
		<< "\"error_msg\":" << "\"" << pRspInfo->ErrorMsg << "\"" << ","
		<< "\"data\":{"
		<< "\"instrument_id\":" << "\"" << pSpecificInstrument->InstrumentID << "\"" << ","
		<< "}"
		<< "}" << std::endl;
}

void CTPQuoteHandler::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}
void CTPQuoteHandler::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CTPQuoteHandler::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	std::cout << "{"
		<< "\"msg\":\"OnRtnDepthMarketData\","
		<< "\"data\":{"
		<< "\"trading_day\":" << "\"" << pDepthMarketData->TradingDay << "\"" << ","
		<< "\"instrument_id\":" << "\"" << pDepthMarketData->InstrumentID << "\"" << ","
		<< "\"exchange\":" << "\"" << pDepthMarketData->ExchangeID << "\"" << ","
		<< "\"exchange_inst_id\":" << "\"" << pDepthMarketData->ExchangeInstID << "\"" << ","
		<< "\"last_price\":" << pDepthMarketData->LastPrice << ","
		<< "\"pre_settlement_price\":" << pDepthMarketData->PreSettlementPrice << ","
		<< "\"pre_close_price\":" << pDepthMarketData->PreClosePrice << ","
		<< "\"pre_open_interest\":" << pDepthMarketData->PreOpenInterest << ","
		<< "\"open_price\":" << pDepthMarketData->OpenPrice << ","
		<< "\"highest_price\":" << pDepthMarketData->HighestPrice << ","
		<< "\"lowest_price\":" << pDepthMarketData->LowestPrice << ","
		<< "\"vol\":" << pDepthMarketData->Volume << ","
		<< "\"turnover\":" << pDepthMarketData->Turnover << ","
		<< "\"open_interest\":" << pDepthMarketData->OpenInterest << ","
		<< "\"close_price\":" << pDepthMarketData->ClosePrice << ","
		<< "\"settlement_price\":" << pDepthMarketData->SettlementPrice << ","
		<< "\"upper_limit_price\":" << pDepthMarketData->UpperLimitPrice << ","
		<< "\"lower_limit_price\":" << pDepthMarketData->LowerLimitPrice << ","
		<< "\"pre_delta\":" << pDepthMarketData->PreDelta << ","
		<< "\"cur_delta\":" << pDepthMarketData->CurrDelta << ","
		<< "\"update_time\":" << "\"" << pDepthMarketData->UpdateTime << "\"" << ","
		<< "\"update_ms\":" << pDepthMarketData->UpdateMillisec << ","
		<< "\"bid_price1\":" << pDepthMarketData->BidPrice1 << ","
		<< "\"bid_vol1\":" << pDepthMarketData->BidVolume1 << ","
		<< "\"ask_price1\":" << pDepthMarketData->AskPrice1 << ","
		<< "\"ask_vol1\":" << pDepthMarketData->AskVolume1 << ","
		<< "\"bid_price2\":" << pDepthMarketData->BidPrice2 << ","
		<< "\"bid_vol2\":" << pDepthMarketData->BidVolume2 << ","
		<< "\"ask_price2\":" << pDepthMarketData->AskPrice2 << ","
		<< "\"ask_vol2\":" << pDepthMarketData->AskVolume2 << ","
		<< "\"bid_price3\":" << pDepthMarketData->BidPrice3 << ","
		<< "\"bid_vol3\":" << pDepthMarketData->BidVolume3 << ","
		<< "\"ask_price3\":" << pDepthMarketData->AskPrice3 << ","
		<< "\"ask_vol3\":" << pDepthMarketData->AskVolume3 << ","
		<< "\"bid_price4\":" << pDepthMarketData->BidPrice4 << ","
		<< "\"bid_vol4\":" << pDepthMarketData->BidVolume4 << ","
		<< "\"ask_price4\":" << pDepthMarketData->AskPrice4 << ","
		<< "\"ask_vol4\":" << pDepthMarketData->AskVolume4 << ","
		<< "\"bid_price5\":" << pDepthMarketData->BidPrice5 << ","
		<< "\"bid_vol5\":" << pDepthMarketData->BidVolume5 << ","
		<< "\"ask_price5\":" << pDepthMarketData->AskPrice5 << ","
		<< "\"ask_vol5\":" << pDepthMarketData->AskVolume5 << ","
		<< "\"avg_price\":" << pDepthMarketData->AveragePrice << ","
		<< "\"action_day\":" << pDepthMarketData->ActionDay << ","
		<< "}"
		<< "}" << std::endl;
}
void CTPQuoteHandler::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) {}