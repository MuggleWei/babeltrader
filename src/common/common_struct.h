#ifndef BABELTRADER_KLINE_H_
#define BABELTRADER_KLINE_H_

#include <stdint.h>
#include <string>
#include <vector>

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

#endif