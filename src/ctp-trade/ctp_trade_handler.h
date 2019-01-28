#ifndef CTP_TRADE_HANDLER
#define CTP_TRADE_HANDLER

#include <thread>

#include "ThostFtdcTraderApi.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "common/common_struct.h"
#include "common/ws_service.h"
#include "common/http_service.h"
#include "common/query_cache.h"

#include "conf.h"

using namespace babeltrader;

class CTPTradeHandler : public TradeService, CThostFtdcTraderSpi
{
public:
	CTPTradeHandler(CTPTradeConf &conf);

	void run();

public:
	////////////////////////////////////////
	// trade service virtual function
	virtual void InsertOrder(uWS::WebSocket<uWS::SERVER> *ws, Order &order) override;
	virtual void CancelOrder(uWS::WebSocket<uWS::SERVER> *ws, Order &order) override;
	virtual void QueryOrder(uWS::WebSocket<uWS::SERVER> *ws, OrderQuery &order_query) override;
	virtual void QueryTrade(uWS::WebSocket<uWS::SERVER> *ws, TradeQuery &trade_query) override;
	virtual void QueryPosition(uWS::WebSocket<uWS::SERVER> *ws, PositionQuery &position_query) override;
	virtual void QueryPositionDetail(uWS::WebSocket<uWS::SERVER> *ws, PositionQuery &position_query) override;
	virtual void QueryTradeAccount(uWS::WebSocket<uWS::SERVER> *ws, TradeAccountQuery &tradeaccount_query) override;
	virtual void QueryProduct(uWS::WebSocket<uWS::SERVER> *ws, ProductQuery &query_product) override;
	virtual void QueryTradingDay(uWS::WebSocket<uWS::SERVER> *ws, TradingDayQuery &tradingday_qry) override;

	////////////////////////////////////////
	// spi virtual function
	virtual void OnFrontConnected() override;
	virtual void OnFrontDisconnected(int nReason) override;
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnHeartBeatWarning(int nTimeLapse) override;

	virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) override;
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder) override;
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade) override;

	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspQryProduct(CThostFtdcProductField *pProduct, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

private:
	void RunAPI();
	void RunService();

	void AsyncLoop();

	void OnOrderStatus(TradeBlock &msg);
	void OnOrderDeal(TradeBlock &msg);

	void FillConnectionInfo(const char *tradeing_day, const char *login_time, int front_id, int session_id);
	void ClearConnectionInfo();

	////////////////////////////////////////
	// requests
	void DoAuthenticate();
	void DoLogin();
	void DoSettlementConfirm();

	////////////////////////////////////////
	// convert common struct to ctp struct
	void ConvertInsertOrderCommon2CTP(Order &order, CThostFtdcInputOrderField &req);
	void ConvertCancelOrderCommon2CTP(Order &order, CThostFtdcInputOrderActionField &req);
	void ConvertQueryOrderCommon2CTP(OrderQuery &order_qry, CThostFtdcQryOrderField &req);
	void ConvertQueryTradeCommon2CTP(TradeQuery &trade_qry, CThostFtdcQryTradeField &req);
	void ConvertQueryPositionCommon2CTP(PositionQuery &position_qry, CThostFtdcQryInvestorPositionField &req);
	void ConvertQueryPositionDetailCommon2CTP(PositionQuery &position_qry, CThostFtdcQryInvestorPositionDetailField &req);
	void ConvertQueryTradeAccountCommon2CTP(TradeAccountQuery &tradeaccount_query, CThostFtdcQryTradingAccountField &req);

	////////////////////////////////////////
	// convert ctp struct to common struct 
	void ConvertInsertOrderCTP2Common(CThostFtdcInputOrderField &req, Order &order);
	void ConvertRtnOrderCTP2Common(CThostFtdcOrderField *pOrder, Order &order, OrderStatusNotify &order_status_notify);
	void ConvertRtnTradeCTP2Common(CThostFtdcTradeField *pTrade, Order &order, OrderDealNotify &order_deal);
	void ConvertPositionCTP2Common(CThostFtdcInvestorPositionField *pPosition, PositionSummaryType1 &position_summary);
	void ConvertPositionDetailCTP2Common(CThostFtdcInvestorPositionDetailField *pPositionDetail, PositionDetailType1 &position_detail);
	void ConvertTradeAccountCTP2Common(CThostFtdcTradingAccountField *pTradingAccount, TradeAccountType1 &trade_account);
	void ConvertProductCTP2Common(CThostFtdcProductField *pProduct, ProductType1 &product);
	void ConvertInstrumentCTP2Common(CThostFtdcInstrumentField *pInstrument, ProductType1 &product);

	////////////////////////////////////////
	// order cache
	void RecordOrder(Order &order, const std::string &order_ref, int front_id, int session_id);
	bool GetAndCleanRecordOrder(Order *p_order, const std::string &user_id, const std::string &order_ref, int front_id, int session_id);

	////////////////////////////////////////
	// order map
	void CacheOrderInfoMap(Order &order, OrderStatusNotify &order_status);
	void GetOrderInfoMap(Order &order);
	void GetOrderInfoMap(Order &order, OrderStatusNotify &order_status);
	void ClearOrderInfoMap();

	////////////////////////////////////////
	// field convert
	std::string ExtendCTPId(const char *investor_id, const char *trading_day, const char *ctp_id);
	bool GetCTPIdFromExtend(const char *ext_ctp_id, int len, char *ctp_id, int ctp_id_size);
	
	char ConvertOrderTypeCommon2CTP(const char *order_type);
	char ConvertOrderFlag1Common2CTP(const char *order_flag1);
	bool ConvertOrderDirCommon2CTP(const char *order_dir, int len, char& action, char& dir);

	int ConvertOrderStatusCTP2Common(TThostFtdcOrderStatusType OrderStatus);
	int ConvertOrderSubmitStatusCTP2Common(TThostFtdcOrderSubmitStatusType OrderSubmitStatus);
	void ConvertOrderDirCTP2Common(const char ctp_dir, const char ctp_hedge_flag, const char ctp_offset_flag, Order &order);

	std::string ConvertPositionDirCTP2Common(TThostFtdcPosiDirectionType ctp_position_dir);
	std::string ConvertHedgeFlagCTP2Common(TThostFtdcHedgeFlagType ctp_hedge_flag);
	std::string ConvertDateTypeCTP2Common(TThostFtdcPositionDateType ctp_date_type);
	std::string ConvertProductTypeCTP2Common(TThostFtdcProductClassType ctp_product_type);

	////////////////////////////////////////
	// serialize ctp struct to json
	void SerializeCTPInputOrder(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcInputOrderField *pInputOrder);
	void SerializeCTPActionOrder(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcInputOrderActionField *pActionOrder);
	void SerializeCTPQueryOrder(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcQryOrderField *pQryOrder);
	void SerializeCTPQueryTrade(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcQryTradeField *pQryTrade);
	void SerializeCTPQueryPosition(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcQryInvestorPositionField *pQryPosition);
	void SerializeCTPQueryPositionDetail(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcQryInvestorPositionDetailField *pQryPosition);
	void SerializeCTPQueryTradeAccount(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcQryTradingAccountField *pTradeingAccount);
	void SerializeCTPQueryProduct(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcQryProductField *pQryProduct);
	void SerializeCTPQueryInstrument(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcQryInstrumentField *pQryInstrument);
	void SerializeCTPOrder(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcOrderField *pOrder);
	void SerializeCTPTrade(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcTradeField *pTrade);
	void SerializeCTPPosition(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcInvestorPositionField *pInvestorPosition);
	void SerializeCTPPositionDetail(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail);
	void SerializeCTPTradingAccount(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcTradingAccountField *pTradingAccount);
	void SerializeCTPProduct(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcProductField *pProduct);
	void SerializeCTPInstrument(rapidjson::Writer<rapidjson::StringBuffer> &writer, CThostFtdcInstrumentField *pInstrument);

	////////////////////////////////////////
	// output ctp struct 
	void OutputOrderInsert(CThostFtdcInputOrderField *req);
	void OutputOrderAction(CThostFtdcInputOrderActionField *req);
	void OutputOrderQuery(CThostFtdcQryOrderField *req);
	void OutputTradeQuery(CThostFtdcQryTradeField *req);
	void OutputPositionQuery(CThostFtdcQryInvestorPositionField *req);
	void OutputPositionDetailQuery(CThostFtdcQryInvestorPositionDetailField *req);
	void OutputTradeAccountQuery(CThostFtdcQryTradingAccountField *req);
	void OutputProductQuery(CThostFtdcQryProductField *req);
	void OutputInstrumentQuery(CThostFtdcQryInstrumentField *req);

	void OutputFrontConnected();
	void OutputFrontDisconnected(int reason);
	void OutputRsperror(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputRspSettlementConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);
	void OutputRtnOrder(CThostFtdcOrderField *pOrder);
	void OutputRtnTrade(CThostFtdcTradeField *pTrade);
	void OutputRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputRspOrderQuery(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputRspTradeQuery(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputRspPositionQuery(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputRspPositionDetailQuery(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputRspTradingAccountQuery(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputRspProductQuery(CThostFtdcProductField *pProduct, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OutputRspInstrumentQuery(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

private:
	CThostFtdcTraderApi *api_;
	bool api_ready_;

	CTPTradeConf conf_;

	WsService ws_service_;
	HttpService http_service_;

	int req_id_;
	int order_ref_;
	int order_action_ref_;

	// connection info
	std::mutex conn_info_mtx_;
	std::string ctp_tradeing_day_;
	std::string ctp_login_time_;
	int ctp_front_id_;
	int ctp_session_id_;

	// order recorder
	std::map<std::string, Order> wait_deal_orders_;
	std::mutex wati_deal_order_mtx_;

	// ourside_order_id map
	std::map<std::string, OrderMapInfo> outside_order_maps_;

	muggle::Tunnel<TradeBlock> tunnel_;

	// query cache
	QueryCache qry_cache_;

	// query results
	std::map<int, std::vector<CThostFtdcOrderField>> rsp_qry_order_caches_;
	std::map<int, std::vector<CThostFtdcTradeField>> rsp_qry_trade_caches_;
	std::map<int, std::vector<CThostFtdcInvestorPositionField>> rsp_qry_position_caches_;
	std::map<int, std::vector<CThostFtdcInvestorPositionDetailField>> rsp_qry_position_detail_caches_;
	std::map<int, std::vector<CThostFtdcTradingAccountField>> rsp_qry_trade_account_caches_;
	std::map<int, std::vector<CThostFtdcProductField>> rsp_qry_product_caches_;
	std::map<int, std::vector<CThostFtdcInstrumentField>> rsp_qry_instrument_caches_;
};

#endif