#include "enum.h"

namespace babeltrader
{ 

const char *g_markets[Market_Max] = {
	"unknown",
	"ctp",
	"xtp",
	"okex",
	"bitmex"
};

const char *g_exchanges[Exchange_Max] = {
	"unknown",
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

const char *g_product_types[ProductType_Max] = {
	"unknown",
	"future",
	"option",
	"spot"
};

const char *g_order_types[OrderType_Max] = {
	"unknown",
	"limit",
	"market"
};


}