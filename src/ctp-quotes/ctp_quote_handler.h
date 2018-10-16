#ifndef CTP_QUOTE_HANDLER_H_
#define CTP_QUOTE_HANDLER_H_

#include "ThostFtdcMdApi.h"

#include "conf.h"
#include "ws_service.h"
#include "http_service.h"

class CTPQuoteHandler : public CThostFtdcMdSpi
{
public:
	CTPQuoteHandler(CTPQuoteConf &conf);

	void run();

public:
	////////////////////////////////////////
	// spi function
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

	void OutputMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);
	std::string SerializeMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

	void SplitInstrument(const char *instrument, std::string &symbol, std::string &contract);

	int64_t GetUpdateTimeMs(CThostFtdcDepthMarketDataField *pDepthMarketData);

private:
	CThostFtdcMdApi *api_;
	CTPQuoteConf &conf_;

	uWS::Hub uws_hub_;
	WsService ws_service_;
	HttpService http_service_;

	int req_id_;
};

#endif