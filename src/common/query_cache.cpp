#include "query_cache.h"

namespace babeltrader
{


void QueryCache::CacheQryOrder(int req_id, uWS::WebSocket<uWS::SERVER>* ws, OrderQuery &order_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	qry_ws_cache_[req_id] = ws;
	qry_order_cache_[req_id] = order_qry;
}
void QueryCache::GetAndClearCacheQryOrder(int req_id, uWS::WebSocket<uWS::SERVER>** ws, OrderQuery *p_order_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	auto it_ws = qry_ws_cache_.find(req_id);
	if (it_ws != qry_ws_cache_.end())
	{
		if (ws)
		{
			*ws = it_ws->second;
		}
		qry_ws_cache_.erase(it_ws);
	}

	auto it_order_qry = qry_order_cache_.find(req_id);
	if (it_order_qry != qry_order_cache_.end())
	{
		if (p_order_qry)
		{
			*p_order_qry = it_order_qry->second;
		}
		qry_order_cache_.erase(it_order_qry);
	}
}

void QueryCache::CacheQryTrade(int req_id, uWS::WebSocket<uWS::SERVER>* ws, TradeQuery &trade_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	qry_ws_cache_[req_id] = ws;
	qry_trade_cache_[req_id] = trade_qry;
}
void QueryCache::GetAndClearCacheQryTrade(int req_id, uWS::WebSocket<uWS::SERVER>** ws, TradeQuery *p_trade_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	auto it_ws = qry_ws_cache_.find(req_id);
	if (it_ws != qry_ws_cache_.end())
	{
		if (ws)
		{
			*ws = it_ws->second;
		}
		qry_ws_cache_.erase(it_ws);
	}

	auto it_trade_qry = qry_trade_cache_.find(req_id);
	if (it_trade_qry != qry_trade_cache_.end())
	{
		if (p_trade_qry)
		{
			*p_trade_qry = it_trade_qry->second;
		}
		qry_trade_cache_.erase(it_trade_qry);
	}
}

void QueryCache::CacheQryPosition(int req_id, uWS::WebSocket<uWS::SERVER>* ws, PositionQuery &position_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	qry_ws_cache_[req_id] = ws;
	qry_position_cache_[req_id] = position_qry;
}
void QueryCache::GetAndCleanCacheQryPosition(int req_id, uWS::WebSocket<uWS::SERVER>** ws, PositionQuery *p_position_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	auto it_ws = qry_ws_cache_.find(req_id);
	if (it_ws != qry_ws_cache_.end())
	{
		if (ws)
		{
			*ws = it_ws->second;
		}
		qry_ws_cache_.erase(it_ws);
	}

	auto it_position_qry = qry_position_cache_.find(req_id);
	if (it_position_qry != qry_position_cache_.end())
	{
		if (p_position_qry)
		{
			*p_position_qry = it_position_qry->second;
		}
		qry_position_cache_.erase(it_position_qry);
	}
}

void QueryCache::CacheQryPositionDetail(int req_id, uWS::WebSocket<uWS::SERVER>* ws, PositionQuery &position_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	qry_ws_cache_[req_id] = ws;
	qry_position_detail_cache_[req_id] = position_qry;
}
void QueryCache::GetAndCleanCacheQryPositionDetail(int req_id, uWS::WebSocket<uWS::SERVER>** ws, PositionQuery *p_position_detail_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	auto it_ws = qry_ws_cache_.find(req_id);
	if (it_ws != qry_ws_cache_.end())
	{
		if (ws)
		{
			*ws = it_ws->second;
		}
		qry_ws_cache_.erase(it_ws);
	}

	auto it_position_detail_qry = qry_position_detail_cache_.find(req_id);
	if (it_position_detail_qry != qry_position_detail_cache_.end())
	{
		if (p_position_detail_qry)
		{
			*p_position_detail_qry = it_position_detail_qry->second;
		}
		qry_position_detail_cache_.erase(it_position_detail_qry);
	}
}

void QueryCache::CacheQryTradeAccount(int req_id, uWS::WebSocket<uWS::SERVER>* ws, TradeAccountQuery &tradeaccount_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	qry_ws_cache_[req_id] = ws;
	qry_trade_account_cache_[req_id] = tradeaccount_qry;
}
void QueryCache::GetAndCleanCacheQryTradeAccount(int req_id, uWS::WebSocket<uWS::SERVER>** ws, TradeAccountQuery *p_tradeaccount_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	auto it_ws = qry_ws_cache_.find(req_id);
	if (it_ws != qry_ws_cache_.end())
	{
		if (ws)
		{
			*ws = it_ws->second;
		}
		qry_ws_cache_.erase(it_ws);
	}

	auto it_trade_account_qry = qry_trade_account_cache_.find(req_id);
	if (it_trade_account_qry != qry_trade_account_cache_.end())
	{
		if (p_tradeaccount_qry)
		{
			*p_tradeaccount_qry = it_trade_account_qry->second;
		}
		qry_trade_account_cache_.erase(it_trade_account_qry);
	}
}

void QueryCache::CacheQryProduct(int req_id, uWS::WebSocket<uWS::SERVER>* ws, ProductQuery &product_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	qry_ws_cache_[req_id] = ws;
	qry_product_cache_[req_id] = product_qry;
}
void QueryCache::GetAndCleanCacheQryProduct(int req_id, uWS::WebSocket<uWS::SERVER>** ws, ProductQuery *p_product_qry)
{
	std::unique_lock<std::mutex> lock(qry_cache_mtx_);
	auto it_ws = qry_ws_cache_.find(req_id);
	if (it_ws != qry_ws_cache_.end())
	{
		if (ws)
		{
			*ws = it_ws->second;
		}
		qry_ws_cache_.erase(it_ws);
	}

	auto it_product_qry = qry_product_cache_.find(req_id);
	if (it_product_qry != qry_product_cache_.end())
	{
		if (p_product_qry)
		{
			*p_product_qry = it_product_qry->second;
		}
		qry_product_cache_.erase(it_product_qry);
	}
}



}