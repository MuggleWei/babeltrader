#ifndef BABELTRADER_KLINE_H_
#define BABELTRADER_KLINE_H_

#include <stdint.h>
#include <string>

struct Kline
{
	int64_t ts;
	double open;
	double high;
	double low;
	double close;
	double vol;
};

struct SubUnsubMsg
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

#endif