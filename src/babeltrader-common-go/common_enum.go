package babeltrader_common_go

const (
	OrderStatus_Unknown      = 0
	OrderStatus_PartDealed   = 1
	OrderStatus_AllDealed    = 2
	OrderStatus_Canceled     = 3
	OrderStatus_Canceling    = 4
	OrderStatus_PartCanceled = 5
	OrderStatus_Rejected     = 6
)

const (
	Market_CTP    = "ctp"
	Market_IB     = "ib"
	Market_XTP    = "xtp"
	Market_OKEX   = "okex"
	Market_BITMEX = "bitmex"
)

const (
	Exchange_SHFE   = "SHFE"
	Exchange_CZCE   = "CZCE"
	Exchange_DCE    = "DCE"
	Exchange_CFFEX  = "CFFEX"
	Exchange_INE    = "INE"
	Exchange_SSE    = "SSE"
	Exchange_SZSE   = "SZSE"
	Exchange_CME    = "CME"
	Exchange_CBOT   = "CBOT"
	Exchange_NYMEX  = "NYMEX"
	Exchange_COMEX  = "COMEX"
	Exchange_CBOE   = "CBOE"
	Exchange_OKEX   = "okex"
	Exchange_Bitmex = "bitmex"
)

const (
	ProductType_Future = "future"
	ProductType_Option = "option"
	ProductType_Spot   = "spot"
	ProductType_ETF    = "etf"
	ProductType_IPO    = "ipo"
)

const (
	QuoteInfo1_MarketData = "marketdata"
	QuoteInfo1_Kline      = "kline"
	QuoteInfo1_OrderBook  = "orderbook"
	QuoteInfo1_Level2     = "level2"
	QuoteInfo1_Depth      = "depth"
	QuoteInfo1_DepthL2    = "depthL2"
	QuoteInfo1_Ticker     = "ticker"
)

const (
	QuoteInfo2_1Hour = "1h"
	QuoteInfo2_1Min  = "1m"
)

const (
	OrderType_Limit  = "limit"
	OrderType_Market = "market"
	OrderType_Best   = "best"
)

const (
	OrderFlag1_Speculation = "speculation"
	OrderFlag1_Arbitrage   = "arbitrage"
	OrderFlag1_Hedge       = "hedge"
	OrderFlag1_Marketmaker = "marketmaker"
)

const (
	OrderAction_Open         = "open"
	OrderAction_Close        = "close"
	OrderAction_CloseToday   = "closetoday"
	OrderAction_CloseHistory = "closehistory"
	OrderAction_ForceClose   = "forceclose"
	OrderAction_Buy          = "buy"
	OrderAction_Sell         = "sell"
	OrderAction_Borrow       = "borrow"
	OrderAction_Lend         = "lend"
)

const (
	OrderDir_Net   = "net"
	OrderDir_Long  = "long"
	OrderDir_Short = "short"
)

const (
	AccountType_Normal      = "normal"
	AccountType_Credit      = "credit"
	AccountType_Derivatives = "derivatives"
)

const (
	OrderBookL2Action_Entrust = "entrust"
	OrderBookL2Action_Trade   = "trade"
)

const (
	OrderBookL2TradeFlag_Buy    = "buy"
	OrderBookL2TradeFlag_Sell   = "sell"
	OrderBookL2TradeFlag_Cancel = "cancel"
	OrderBookL2TradeFlag_Deal   = "deal"
)
