#ifndef BABELTRADER_KLINE_H_
#define BABELTRADER_KLINE_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "common/enum.h"

namespace babeltrader
{

//////////////////////////
// quotes

#define QUOTE_SYMBOL_LEN 16
#define QUOTE_CONTRACT_LEN 16
#define QUOTE_INFO1_LEN 16
#define QUOTE_INFO2_LEN 16

#define BIDASK_MAX_LEN 10
#define QUOTE_DATETIME_LEN 32

#define QuoteBlockSize 1024

struct Quote
{
	uint8_t market;		// MarketEnum
	uint8_t exchange;	// ExchangeEnum
	uint8_t type;		// ProductTypeEnum
	uint8_t info1;		// QuoteInfo1Enum
	char symbol[QUOTE_SYMBOL_LEN];
	char contract[QUOTE_CONTRACT_LEN];
	char contract_id[QUOTE_CONTRACT_LEN];
	uint8_t info2;		// QuoteInfo2Enum
#if ENABLE_PERFORMANCE_TEST
	int64_t ts;
#endif
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
	int bid_ask_len;
	PriceVol bids[BIDASK_MAX_LEN];
	PriceVol asks[BIDASK_MAX_LEN];
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
	char trading_day[QUOTE_DATETIME_LEN];
	char action_day[QUOTE_DATETIME_LEN];
};

struct OrderBook
{
	int64_t ts;
	double last;
	double vol;
	int bid_ask_len;
	PriceVol bids[BIDASK_MAX_LEN];
	PriceVol asks[BIDASK_MAX_LEN];
};

struct OrderBookLevel2Entrust
{
	int64_t channel_no;
	int64_t seq;
	double price;
	double vol;
	OrderActionEnum dir;
	OrderTypeEnum order_type;
};

struct OrderBookLevel2Trade
{
	int64_t channel_no;
	int64_t seq;
	double price;
	double vol;
	int64_t bid_no;
	int64_t ask_no;
	OrderBookL2TradeFlagEnum trade_flag;
};

struct OrderBookLevel2
{
	int64_t ts;
	int64_t seq;
	OrderBookL2Action action;
	OrderBookLevel2Entrust entrust;
	OrderBookLevel2Trade trade;
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

enum QuoteBlockType
{
	QuoteBlockType_MarketData = 0,
	QuoteBlockType_Kline,
	QuoteBlockType_OrderBook,
	QuoteBlockType_Level2,
};

struct QuoteMarketData
{
	uint8_t quote_type;
	Quote quote;
	MarketData market_data;
};

struct QuoteOrderBook
{
	uint8_t quote_type;
	Quote quote;
	OrderBook order_book;
};

struct QuoteOrderBookLevel2
{
	uint8_t quote_type;
	Quote quote;
	OrderBookLevel2 level2;
};

struct QuoteKline
{
	uint8_t quote_type;
	Quote quote;
	Kline kline;
};

struct QuoteBlockCommon
{
	uint8_t quote_type;
	Quote quote;
};

struct QuoteBlock
{
	uint8_t quote_type;
	char buf[QuoteBlockSize];
};

//////////////////////////
// trade

struct Order
{
	std::string user_id;
	std::string order_id;
	std::string outside_user_id;
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

struct ProductQuery
{
	std::string qry_id;
	std::string market;
	std::string exchange;
	std::string symbol;
	std::string contract;
};

struct PositionSummaryType1
{
	std::string market;
	std::string outside_user_id;
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

struct PositionSummaryType2
{
	std::string market;
	std::string outside_user_id;
	std::string exchange;
	std::string type;
	std::string symbol;
	std::string dir;
	double amount;
	double avaliable_amount;
	double avg_price;
	double unrealized_profit;
	double purchase_redeemable_qty;
	int64_t executable_option;
	int64_t lockable_position;
	int64_t executable_underlying;
	int64_t locked_position;
	int64_t usable_locked_position;

	PositionSummaryType2()
		: amount(0)
		, avaliable_amount(0)
		, avg_price(0)
		, unrealized_profit(0)
		, purchase_redeemable_qty(0)
		, executable_option(0)
		, lockable_position(0)
		, executable_underlying(0)
		, locked_position(0)
		, usable_locked_position(0)
	{}
};

struct PositionDetailType1
{
	std::string market;
	std::string outside_user_id;
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

struct PositionDetailType2
{
	char reverse[8];
};

struct TradeAccountType1
{
	std::string market;
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

struct TradeAccountType2
{
	std::string market;
	std::string outside_user_id;
	std::string account_type;
	double total_asset;
	double available_cash;
	double securities_asset;
	double fund_buy_amount;
	double fund_buy_fee;
	double fund_sell_amount;
	double fund_sell_fee;
	double withholding_amount;
	double frozen_margin;
	double frozen_exec_cash;
	double frozen_exec_fee;
	double pay_later;
	double preadva_pay;
	double orig_banlance;
	double banlance;
	double deposit_withdraw;
	double trade_netting;
	double captial_asset;
	double force_freeze_amount;
	double preferred_amount;
};

struct ProductType1
{
	std::string market;
	std::string outside_user_id;
	std::string exchange;
	std::string type;
	std::string symbol;
	std::string contract;
	std::string contract_id;
	double vol_multiple;
	double price_tick;
	double long_margin_ratio;
	double short_margin_ratio;

	ProductType1()
		: vol_multiple(0)
		, price_tick(0)
		, long_margin_ratio(0)
		, short_margin_ratio(0)
	{}
};


}

#endif