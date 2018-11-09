#ifndef BABELTRADER_QUERY_CACHE_H_
#define BABELTRADER_QUERY_CACHE_H_

#include <map>
#include <thread>
#include <mutex>

#include "uWS/uWS.h"

#include "common/common_struct.h"

namespace babeltrader
{


class QueryCache
{
public:
	void CacheQryOrder(int req_id, uWS::WebSocket<uWS::SERVER>* ws, OrderQuery &order_qry);
	void GetAndClearCacheQryOrder(int req_id, uWS::WebSocket<uWS::SERVER>** ws, OrderQuery *p_order_qry);

	void CacheQryTrade(int req_id, uWS::WebSocket<uWS::SERVER>* ws, TradeQuery &trade_qry);
	void GetAndClearCacheQryTrade(int req_id, uWS::WebSocket<uWS::SERVER>** ws, TradeQuery *p_trade_qry);

	void CacheQryPosition(int req_id, uWS::WebSocket<uWS::SERVER>* ws, PositionQuery &position_qry);
	void GetAndCleanCacheQryPosition(int req_id, uWS::WebSocket<uWS::SERVER>** ws, PositionQuery *p_position_qry);

	void CacheQryPositionDetail(int req_id, uWS::WebSocket<uWS::SERVER>* ws, PositionQuery &position_qry);
	void GetAndCleanCacheQryPositionDetail(int req_id, uWS::WebSocket<uWS::SERVER>** ws, PositionQuery *p_position_detail_qry);

	void CacheQryTradeAccount(int req_id, uWS::WebSocket<uWS::SERVER>* ws, TradeAccountQuery &tradeaccount_qry);
	void GetAndCleanCacheQryTradeAccount(int req_id, uWS::WebSocket<uWS::SERVER>** ws, TradeAccountQuery *p_tradeaccount_qry);

	void CacheQryProduct(int req_id, uWS::WebSocket<uWS::SERVER>* ws, ProductQuery &product_qry);
	void GetAndCleanCacheQryProduct(int req_id, uWS::WebSocket<uWS::SERVER>** ws, ProductQuery *p_product_qry);

private:
	std::mutex qry_cache_mtx_;
	std::map<int, uWS::WebSocket<uWS::SERVER>*> qry_ws_cache_;
	std::map<int, OrderQuery> qry_order_cache_;
	std::map<int, TradeQuery> qry_trade_cache_;
	std::map<int, PositionQuery> qry_position_cache_;
	std::map<int, PositionQuery> qry_position_detail_cache_;
	std::map<int, TradeAccountQuery> qry_trade_account_cache_;
	std::map<int, ProductQuery> qry_product_cache_;
};


}

#endif