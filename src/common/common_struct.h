#ifndef BABELTRADER_KLINE_H_
#define BABELTRADER_KLINE_H_

#include <stdint.h>
#include <string>
#include <vector>

namespace babeltrader
{


//////////////////////////
// enum
enum OrderStatusEnum
{
	OrderStatus_Unknown = 0,
	OrderStatus_PartDealed = 1,
	OrderStatus_AllDealed = 2,
	OrderStatus_Canceled = 3,
	OrderStatus_Canceling = 4,
};

enum OrderSubmitStatusEnum
{
	OrderSubmitStatus_Unknown = 0,
	OrderSubmitStatus_Submitted = 1,
	OrderSubmitStatus_Accepted = 2,
	OrderSubmitStatus_Rejected = 3,
};


//////////////////////////
// quotes

struct Quote
{
	std::string market;			// e.g. okex, bitmex, CTP, XTP, IB ...
	std::string exchange;		// e.g. okex, bitmex, SHFE, SSE, NYMEX ...
	std::string type;			// e.g. spot, future ...
	std::string symbol;			// e.g. btc, btc_usd, rb, cl ...
	std::string contract;		// e.g. this_week, 1901 ...
	std::string contract_id;	// e.g. 20180928, 1901 ...
	std::string info1;			// e.g. ticker, depth, kline, marketdata
	std::string info2;			// e.g. 1m, 1h
};

struct PriceVol
{
	double price;
	int64_t vol;
};

struct MarketData
{
	int64_t ts;
	double last;
	std::vector<PriceVol> bids;
	std::vector<PriceVol> asks;
	double vol;
	double turnover;
	double avg_price;
	double pre_settlement;
	double pre_close;
	double pre_open_interest;
	double settlement;
	double close;
	double open_interest;
	double upper_limit;
	double lower_limit;
	double open;
	double high;
	double low;
	std::string trading_day;
	std::string action_day;
};

struct Kline
{
	int64_t ts;
	double open;
	double high;
	double low;
	double close;
	double vol;
};


//////////////////////////
// trade

struct Order
{
	std::string user_id;
	std::string order_id;
	std::string outside_id;
	std::string client_order_id;
	std::string market;
	std::string exchange;
	std::string type;
	std::string symbol;
	std::string contract;
	std::string contract_id;
	std::string order_type;		// e.g. limit, market
	std::string order_flag1;	// e.g. speculation, hedge, arbitrage
	std::string dir;
	double price;
	double amount;
	double total_price;
	int64_t ts;

	Order()
		: price(0)
		, amount(0)
		, total_price(0)
		, ts(0)
	{}
};

struct OrderStatusNotify
{
	OrderStatusEnum order_status;
	OrderSubmitStatusEnum order_submit_status;
	double amount;
	double dealed_amount;

	OrderStatusNotify()
		: order_status(OrderStatus_Unknown)
		, order_submit_status(OrderSubmitStatus_Unknown)
		, amount(0)
		, dealed_amount(0)
	{}
};

struct OrderDealNotify
{
	double price;
	double amount;
	std::string trading_day;
	std::string trade_id;
	int64_t ts;

	OrderDealNotify()
		: price(0)
		, amount(0)
		, ts(0)
	{}
};

struct OrderQuery
{
	std::string qry_id;
	std::string user_id;
	std::string outside_id;
	std::string market;
	std::string exchange;
	std::string type;
	std::string symbol;
	std::string contract;
	std::string contract_id;
};

struct TradeQuery
{
	std::string qry_id;
	std::string user_id;
	std::string trade_id;
	std::string market;
	std::string exchange;
	std::string type;
	std::string symbol;
	std::string contract;
	std::string contract_id;
};

struct PositionQuery
{
	std::string qry_id;
	std::string user_id;
	std::string market;
	std::string exchange;
	std::string type;
	std::string symbol;
	std::string contract;
	std::string contract_id;
};

struct TradeAccountQuery
{
	std::string qry_id;
	std::string user_id;
	std::string market;
	std::string currency_id;
};

struct PositionSummaryType1
{
	std::string market;
	std::string exchange;
	std::string type;
	std::string symbol;
	std::string contract;
	std::string contract_id;
	std::string dir;					// e.g. net, long, short
	std::string order_flag1;			// e.g. speculation, hedge, arbitrage
	std::string date_type;				// e.g. today, history
	double amount;
	double closed_amount;
	double today_amount;
	double margin;
	double margin_rate_by_money;
	double margin_rate_by_vol;
	double long_frozen;
	double short_frozen;
	double frozen_margin;
	std::string trading_day;
	double pre_settlement_price;
	double settlement_price;
	double open_cost;
	double position_cost;
	double position_profit;
	double close_profit_by_date;
	double close_profit_by_trade;

	PositionSummaryType1()
		: amount(0)
		, closed_amount(0)
		, today_amount(0)
		, margin(0)
		, margin_rate_by_money(0)
		, margin_rate_by_vol(0)
		, long_frozen(0)
		, short_frozen(0)
		, frozen_margin(0)
		, pre_settlement_price(0)
		, settlement_price(0)
		, open_cost(0)
		, position_cost(0)
		, position_profit(0)
		, close_profit_by_date(0)
		, close_profit_by_trade(0)
	{}
};

struct PositionDetailType1
{
	std::string market;
	std::string exchange;
	std::string type;
	std::string symbol;
	std::string contract;
	std::string contract_id;
	std::string dir;					// e.g. net, long, short
	std::string order_flag1;			// e.g. speculation, hedge, arbitrage
	std::string open_date;
	std::string trading_day;
	std::string trade_id;
	double amount;
	double closed_amount;
	double closed_money;
	double pre_settlement_price;
	double settlement_price;
	double open_price;
	double margin;
	double margin_rate_by_money;
	double margin_rate_by_vol;
	double close_profit_by_date;
	double close_profit_by_trade;
	double position_profit_by_date;
	double position_profit_by_trade;

	PositionDetailType1()
		: amount(0)
		, closed_amount(0)
		, closed_money(0)
		, pre_settlement_price(0)
		, settlement_price(0)
		, open_price(0)
		, margin(0)
		, margin_rate_by_money(0)
		, margin_rate_by_vol(0)
		, close_profit_by_date(0)
		, close_profit_by_trade(0)
		, position_profit_by_date(0)
		, position_profit_by_trade(0)
	{}
};

struct TradeAccountType1
{
	std::string outside_user_id;
	double pre_credit;
	double pre_balance;
	double pre_margin;
	double interest;
	double deposit;
	double withdraw;
	double credit;
	double margin;
	double commission;
	double close_profit;
	double position_profit;
	double frozen_margin;
	double frozen_cash;
	double frozen_commision;
	double balance;
	double available;
	std::string currency_id;
	std::string trading_day;

	TradeAccountType1()
		: pre_credit(0)
		, pre_balance(0)
		, pre_margin(0)
		, interest(0)
		, deposit(0)
		, withdraw(0)
		, credit(0)
		, margin(0)
		, commission(0)
		, close_profit(0)
		, position_profit(0)
		, frozen_margin(0)
		, frozen_cash(0)
		, frozen_commision(0)
		, balance(0)
		, available(0)
	{}
};


}

#endif