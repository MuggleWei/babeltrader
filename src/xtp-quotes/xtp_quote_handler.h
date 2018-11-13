#ifndef XTP_QUOTE_HANDLER_H_
#define XTP_QUOTE_HANDLER_H_

#include <map>
#include <thread>
#include <vector>

#include "xtp_quote_api.h"

#include "common/common_struct.h"
#include "common/ws_service.h"
#include "common/http_service.h"
#include "common/quote_service.h"
#include "common/kline_builder.h"
#include "conf.h"

using namespace babeltrader;

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
	void OutputMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count);
	void OutputOrderBook(XTPOB *order_book);

	void ConvertMarketData(XTPMD *market_data, Quote &quote, MarketData &md);
	void ConvertOrderBook(XTPOB *xtp_order_book, Quote &quote, OrderBook &order_book);

	void BroadcastMarketData(const Quote &quote, const MarketData &md);
	void BroadcastOrderBook(const Quote &quote, const OrderBook &order_book);
	void BroadcastKline(const Quote &quote, const Kline &kline);

	void SubTopics();

	XTP_EXCHANGE_TYPE GetExchangeType(const std::string &exchange);
	const char* ConvertExchangeType2Str(XTP_EXCHANGE_TYPE exchange_type);

private:
	XTP::API::QuoteApi *api_;
	XTPQuoteConf conf_;

	uWS::Hub uws_hub_;
	WsService ws_service_;
	HttpService http_service_;

	int req_id_;

	std::mutex topic_mtx_;
	std::map<std::string, bool> sub_topics_;
	std::map<std::string, XTP_EXCHANGE_TYPE> topic_exchange_;
	KlineBuilder kline_builder_;
};

#endif