#ifndef BABELTRADER_ENUM_H_
#define BABELTRADER_ENUM_H_

namespace babeltrader
{

enum MarketEnum
{
	Market_Unknown = 0,
	Market_CTP,
	Market_IB,
	Market_XTP,
	Market_OKEX,
	Market_BITMEX,
	Market_Max,
};
extern const char *g_markets[Market_Max];

enum ExchangeEnum
{
	Exchange_Unknown = 0,
	Exchange_SHFE,		// Shanghai Futures Exchange
	Exchange_CZCE,		// Zhengzhou Commodities Exchange
	Exchange_DCE,		// Dalian Commodity Exchange
	Exchange_CFFEX,		// China Financial Futures Exchange
	Exchange_INE,		// Shanghai International Energy Exchange
	Exchange_SSE,		// Shanghai Stock Exchange
	Exchange_SZSE,		// Shenzhen Stock Exchange
	Exchange_CME,		// Chicago Mercantile Exchange
	Exchange_CBOT,		// Chicago Board of Trade
	Exchange_NYMEX,		// New York Mercantile Exchange
	Exchange_COMEX,		// Commodity Exchange
	Exchange_CBOE,		// Chicago Board Options Exchange
	Exchange_OKEX,		// okex
	Exchange_Bitmex,	// bitmex
	Exchange_Max,
};
extern const char *g_exchanges[Exchange_Max];

enum ProductTypeEnum
{
	ProductType_Unknown = 0,
	ProductType_Future,
	ProductType_Option,
	ProductType_Spot,
	ProductType_ETF,
	ProductType_IPO,
	ProductType_Max,
};
extern const char *g_product_types[ProductType_Max];

enum OrderTypeEnum
{
	OrderType_Unknown = 0,
	OrderType_Limit,	// limit price
	OrderType_Market,	// market price
	OrderType_Max,
};
extern const char *g_order_type[OrderType_Max];


enum OrderFlag1Enum
{
	OrderFlag1_Unknown = 0,
	OrderFlag1_Speculation,
	OrderFlag1_Arbitrage,
	OrderFlag1_Hedge,
	OrderFlag1_Marketmaker,
	OrderFlag1_Max,
};
extern const char *g_order_flag1[OrderFlag1_Max];

enum OrderActionEnum
{
	OrderAction_Unknown = 0,
	OrderAction_Open,
	OrderAction_Close,
	OrderAction_CloseToday,
	OrderAction_CloseHistory,
	OrderAction_ForceClose,
	OrderAction_Buy,
	OrderAction_Sell,
	OrderAction_Max,
};
extern const char *g_order_action[OrderAction_Max];

enum OrderDirEnum
{
	OrderDir_Unknown = 0,
	OrderDir_Net,
	OrderDir_Long,
	OrderDir_Short,
	OrderDir_Max,
};
extern const char *g_order_dir[OrderDir_Max];

enum OrderStatusEnum
{
	OrderStatus_Unknown = 0,
	OrderStatus_PartDealed = 1,
	OrderStatus_AllDealed = 2,
	OrderStatus_Canceled = 3,
	OrderStatus_Canceling = 4,
	OrderStatus_PartCanceled = 5,
	OrderStatus_Rejected = 6,
};

enum OrderSubmitStatusEnum
{
	OrderSubmitStatus_Unknown = 0,
	OrderSubmitStatus_Submitted = 1,
	OrderSubmitStatus_Accepted = 2,
	OrderSubmitStatus_Rejected = 3,
};

}


#endif