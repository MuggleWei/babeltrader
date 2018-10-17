#ifndef CTP_QUOTE_HANDLER_H_
#define CTP_QUOTE_HANDLER_H_

#include <map>
#include <thread>
#include <vector>

#include "ThostFtdcMdApi.h"

#include "common/common_struct.h"
#include "common/ws_service.h"
#include "common/http_service.h"
#include "common/quote_service.h"
#include "common/kline_builder.h"
#include "conf.h"

class CTPQuoteHandler : public QuoteService, CThostFtdcMdSpi
{
public:
	CTPQuoteHandler(CTPQuoteConf &conf);

	void run();

public:
	////////////////////////////////////////
	// quote service virtual function
	virtual std::vector<Quote> GetSubTopics(std::vector<bool> &vec_b) override;
	virtual void SubTopic(const Quote &msg) override;
	virtual void UnsubTopic(const Quote &msg) override;

	////////////////////////////////////////
	// spi virtual function
	virtual void OnFrontConnected() override;
	virtual void OnFrontDisconnected(int nReason) override;
	virtual void OnHeartBeatWarning(int nTimeLapse) override;

	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	virtual void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) override;
	virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) override;

private:
	void RunAPI();
	void RunService();

	void OutputFrontConnected();
	void OutputFrontDisconnected();
	void OutputRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputRspUnsubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

	void ConvertMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, Quote &quote, MarketData &md);

	void BroadcastMarketData(const Quote &quote, const MarketData &md);
	void BroadcastKline(const Quote &quote, const Kline &kline);

	void SplitInstrument(const char *instrument, std::string &symbol, std::string &contract);

	int64_t GetUpdateTimeMs(CThostFtdcDepthMarketDataField *pDepthMarketData);

	void SubTopics();

private:
	CThostFtdcMdApi *api_;
	CTPQuoteConf conf_;

	uWS::Hub uws_hub_;
	WsService ws_service_;
	HttpService http_service_;

	int req_id_;

	std::mutex topic_mtx_;
	std::map<std::string, bool> sub_topics_;
	KlineBuilder kline_builder_;
};

#endif