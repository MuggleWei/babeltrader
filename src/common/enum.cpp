#include "enum.h"

#include <string.h>

#define GET_ENUM_BY_STR(str, g_str, enum_name, enum_max) \
for (int i = 1; i < enum_max; i++) \
{ \
	if (strlen(g_str[i]) == strlen(str) && strncmp(str, g_str[i], strlen(str)) == 0) \
	{ \
		return (enum_name)i; \
	} \
} \
return (enum_name)0;

namespace babeltrader
{

const char *g_markets[Market_Max] = {
	"",
	"ctp",
	"ib",
	"xtp",
	"okex",
	"bitmex"
};
MarketEnum getMarketEnum(const char *market)
{
	GET_ENUM_BY_STR(market, g_markets, MarketEnum, Market_Max)
}

const char *g_exchanges[Exchange_Max] = {
	"",
	"SHFE",
	"CZCE",
	"DCE",
	"CFFEX",
	"INE",
	"SSE",
	"SZSE",
	"CME",
	"CBOT",
	"NYMEX",
	"COMEX",
	"CBOE",
	"okex",
	"bitmex"
};
ExchangeEnum getExchangeEnum(const char *exchange)
{
	GET_ENUM_BY_STR(exchange, g_exchanges, ExchangeEnum, Exchange_Max);
}

const char *g_product_types[ProductType_Max] = {
	"",
	"future",
	"option",
	"spot",
	"etf",
	"ipo"
};
ProductTypeEnum getProductTypeEnum(const char *product_type)
{
	GET_ENUM_BY_STR(product_type, g_product_types, ProductTypeEnum, ProductType_Max);
}

const char *g_quote_info1[QuoteInfo1_Max] = {
	"",
	"marketdata",
	"kline",
	"orderbook",
	"level2",
	"depth",
	"ticker",
};
QuoteInfo1Enum getQuoteInfo1Enum(const char *quote_info1)
{
	GET_ENUM_BY_STR(quote_info1, g_quote_info1, QuoteInfo1Enum, QuoteInfo1_Max);
}

const char *g_quote_info2[QuoteInfo2_Max] = {
	"",
	"1h",
	"1m",
};
QuoteInfo2Enum getQuoteInfo2Enum(const char *quote_info2)
{
	GET_ENUM_BY_STR(quote_info2, g_quote_info2, QuoteInfo2Enum, QuoteInfo2_Max);
}

const char *g_order_type[OrderType_Max] = {
	"",
	"limit",
	"market",
	"best"
};

const char *g_order_flag1[OrderFlag1_Max] = {
	"",
	"speculation",
	"arbitrage",
	"hedge",
	"marketmaker"
};

const char *g_order_action[OrderAction_Max] = {
	"",
	"open",
	"close",
	"closetoday",
	"closehistory",
	"forceclose",
	"buy",
	"sell",
	"borrow",
	"lend"
};

const char *g_order_dir[OrderDir_Max] = {
	"",
	"net",
	"long",
	"short"
};

const char *g_account_type[AccountType_Max] = {
	"",
	"normal",
	"credit",
	"derivatives"
};

const char *g_orderbookl2_action[OrderBookL2Action_Max] = {
	"",
	"entrust",
	"trade"
};

const char *g_orderbookl2_trade_flag[OrderBookL2TradeFlag_Max] = {
	"",
	"buy",
	"sell",
	"cancel",
	"deal"
};

}