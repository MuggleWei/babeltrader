#ifndef XTP_QUOTE_HANDLER_H_
#define XTP_QUOTE_HANDLER_H_

#include <map>
#include <thread>
#include <vector>

#include "xtp_quote_api.h"
#include "muggle/cpp/tunnel/tunnel.hpp"

#include "common/common_struct.h"
#include "common/ws_service.h"
#include "common/http_service.h"
#include "common/quote_service.h"
#include "common/kline_builder.h"
#include "conf.h"

using namespace babeltrader;

#define XTPQuoteBlockSize 1024

enum XTPQuoteType
{
	XTPQuoteType_MarketData = 0,
	XTPQuoteType_OrderBook,
	XTPQuoteType_TickByTick,
};

struct XTPQuoteBlock
{
	int quote_type;
	char buf[XTPQuoteBlockSize];
#if ENABLE_PERFORMANCE_TEST
	int64_t ts;
#endif
};

class XTPQuoteHandler : public QuoteService, XTP::API::QuoteSpi
{
public:
	XTPQuoteHandler(XTPQuoteConf &conf);

	void run();

public:
	////////////////////////////////////////
	// quote service virtual function
	virtual std::vector<Quote> GetSubTopics(std::vector<bool> &vec_b) override;
	virtual void SubTopic(const Quote &msg) override;
	virtual void UnsubTopic(const Quote &msg) override;

	////////////////////////////////////////
	// spi virtual function
	virtual void OnDisconnected(int reason) override;
	virtual void OnError(XTPRI *error_info) override;

	virtual void OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last) override;
	virtual void OnUnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last) override;
	virtual void OnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last) override;
	virtual void OnUnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last) override;
	virtual void OnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last) override;
	virtual void OnUnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last) override;

	virtual void OnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override;
	virtual void OnUnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override;
	virtual void OnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override;
	virtual void OnUnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override;
	virtual void OnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override;
	virtual void OnUnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override;

	virtual void OnDepthMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count) override;
	virtual void OnOrderBook(XTPOB *order_book) override;
	virtual void OnTickByTick(XTPTBT *tbt_data) override;

private:
	void RunAPI();
	void RunService();

	void Reconn();

	void OutputFrontDisconnected();
	void OutputRspSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last);
	void OutputRspUnsubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last);
	void OutputRspSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last);
	void OutputRspUnsubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last);
	void OutputRspSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last);
	void OutputRspUnsubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last);

	void OutputRspSubAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info);
	void OutputRspUnsubAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info);
	void OutputRspSubAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info);
	void OutputRspUnsubAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info);
	void OutputRspSubAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info);
	void OutputRspUnsubAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info);

	void OutputMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count);
	void OutputOrderBook(XTPOB *order_book);
	void OutputTickByTick(XTPTBT *tbt_data);

	void ConvertMarketData(XTPMD *market_data, Quote &quote, MarketData &md);
	void ConvertOrderBook(XTPOB *xtp_order_book, Quote &quote, OrderBook &order_book);
	void ConvertTickByTick(XTPTBT *tbt_data, Quote &quote, OrderBookLevel2 &level2);

	void SubTopics();

	XTP_EXCHANGE_TYPE ConvertExchangeTypeCommon2XTP(ExchangeEnum exchange);
	ExchangeEnum ConvertExchangeTypeXTP2Common(XTP_EXCHANGE_TYPE exchange_type);

	OrderActionEnum ConvertOrderAction(char side);
	OrderTypeEnum ConvertOrderType(char order_type);
	OrderBookL2TradeFlagEnum ConvertTradeFlag(char trade_flag);

private:
	XTP::API::QuoteApi *api_;
	XTPQuoteConf conf_;

	uWS::Hub uws_hub_;
	WsService ws_service_;
	HttpService http_service_;

	int req_id_;

	std::mutex topic_mtx_;
	std::map<std::string, bool> sub_topics_;
	std::map<std::string, ExchangeEnum> topic_exchange_;
	KlineBuilder kline_builder_;

	muggle::Tunnel<XTPQuoteBlock> quote_tunnel_;
};

#endif