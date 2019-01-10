package babeltrader_common_go

//////////////////////// common ////////////////////////
/*
common request message
*/
type MessageReqCommon struct {
	Message string      `json:"msg"`
	Data    interface{} `json:"data,omitempty"`
}

/*
common response message
*/
type MessageRspCommon struct {
	Message string      `json:"msg"` // e.g. ticker, depth, order, deal, balances, positions
	ErrId   int64       `json:"error_id"`
	ErrMsg  string      `json:"error_msg,omitempty"`
	Data    interface{} `json:"data,omitempty"`
}

/*
common message quote
*/
type MessageQuote struct {
	Market      string      `json:"market,omitempty"`      // e.g. okex, bitmex, NYMEX, ctp, xtp
	Exchange    string      `json:"exchange,omitempty"`    // e.g. SHFE, SSE, NYMEX, bitmex, okex
	Type        string      `json:"type,omitempty"`        // e.g. spot, future, option
	Symbol      string      `json:"symbol,omitempty"`      // e.g. btc, btc_usd, CL, rb
	Contract    string      `json:"contract,omitempty"`    // e.g. this_week, next_week, quarter, 1810
	ContractId  string      `json:"contract_id,omitempty"` // e.g. 20180928 for coin or repeat Contract for commodity future
	InfoPrimary string      `json:"info1,omitempty"`       // e.g. ticker, depth, kline
	InfoExtra   string      `json:"info2,omitempty"`       // e.g. 1m, 1h, 5
	Data        interface{} `json:"data,omitempty"`        // e.g. MessageQuoteTicker, MessageQuoteDepth
}

//////////////////////// quotes ////////////////////////
/*
marketdata
*/
type MessageQuoteMarketData struct {
	Timestamp       int64       `json:"ts,omitempty"`
	Last            float64     `json:"last,omitempty"`
	Asks            [][]float64 `json:"asks"`
	Bids            [][]float64 `json:"bids"`
	Vol             float64     `json:"vol,omitempty"`
	Turnover        float64     `json:"turnover,omitempty"`
	AvgPrice        float64     `json:"avg_price,omitempty"`
	PreSettlement   float64     `json""pre_settlement,omitempty"`
	PreClose        float64     `json:"pre_close,omitempty"`
	PreOpenInterest float64     `json:"pre_open_interest,omitempty"`
	Settlement      float64     `json:"settlement,omitempty"`
	Close           float64     `json:"close,omitempty"`
	OpenInterest    float64     `json:"open_interest,omitempty"`
	UpperLimit      float64     `json:"upper_limit,omitempty"`
	LowerLimit      float64     `json:"lower_limit,omitempty"`
	Open            float64     `json:"open,omitempty"`
	High            float64     `json:"high,omitempty"`
	Low             float64     `json:"low,omitempty"`
	TradingDay      float64     `json:"trading_day,omitempty"`
	ActionDay       float64     `json:"action_day,omitempty"`
}

/*
kline
*/
type MessageQuoteKLine struct {
	Timestamp int64   `json:"ts"`
	Open      float64 `json:"open"`
	High      float64 `json:"high"`
	Low       float64 `json:"low"`
	Close     float64 `json:"close"`
	Vol       float64 `json:"vol"`
}

/*
orderbook
*/
type MessageQuoteOrderBook struct {
	Timestamp int64       `json:"ts"`
	Last      float64     `json:"last"`
	Vol       float64     `json:"vol"`
	Asks      [][]float64 `json:"asks"`
	Bids      [][]float64 `json:"bids"`
}

/*
level2
*/
type MessageQuoteLevel2 struct {
	Timestamp int64       `json:"ts"`
	Action    string      `json:"action"`
	Data      interface{} `json:"data,omitempty"`
}
type MessageQuoteLevel2Entrust struct {
	ChannelNo int64   `json:"channel_no"`
	Seq       int64   `json:"seq"`
	Price     float64 `json:"price"`
	Vol       float64 `json:"vol"`
	BidNo     int64   `json:"bid_no"`
	AskNo     int64   `json:"ask_no"`
	TradeFlag string  `json:"trade_flag"`
}
type MessageQuoteLevel2Trade struct {
	ChannelNo int64   `json:"channel_no"`
	Seq       int64   `json:"seq"`
	Price     float64 `json:"price"`
	Vol       float64 `json:"vol"`
	Dir       string  `json:"dir"`
	OrderType string  `json:"order_type"`
}

/*
depth
*/
type MessageQuoteDepth struct {
	Asks      [][]float64 `json:"asks"`
	Bids      [][]float64 `json:"bids"`
	Timestamp int64       `json:"ts"`
}

/*
ticker
*/
type MessageQuoteTicker struct {
	Bid       float64 `json:"bid"`
	Ask       float64 `json:"ask"`
	Last      float64 `json:"last"`
	High      float64 `json:"high"`
	Low       float64 `json:"low"`
	Vol       float64 `json:"vol"`
	Timestamp int64   `json:"ts"`
}

/*
subunsub
*/
type MessageSubUnsub struct {
	Market      string `json:"market,omitempty"`      // e.g. okex, bitmex, NYMEX, ctp, xtp
	Exchange    string `json:"exchange,omitempty"`    // e.g. SHFE, SSE, NYMEX, bitmex, okex
	Type        string `json:"type,omitempty"`        // e.g. spot, future, option
	Symbol      string `json:"symbol,omitempty"`      // e.g. btc, btc_usd, CL, rb
	Contract    string `json:"contract,omitempty"`    // e.g. this_week, next_week, quarter, 1810
	ContractId  string `json:"contract_id,omitempty"` // e.g. 20180928 for coin or repeat Contract for commodity future
	InfoPrimary string `json:"info1,omitempty"`       // e.g. ticker, depth, kline
	InfoExtra   string `json:"info2,omitempty"`       // e.g. 1m, 1h, 5
	Subed       int    `json:"subed,omitempty"`
}

//////////////////////// trade ////////////////////////
/*
order
*/
type MessageOrder struct {
	UserId        string  `json:"user_id,omitempty"`
	OrderId       string  `json:"order_id,omitempty"`
	OutsideUserId string  `json:"outside_user_id,omitempty"`
	OutsideId     string  `json:"outside_id,omitempty"`
	ClientOrderId string  `json:"client_order_id,omitempty"`
	Market        string  `json:"market,omitempty"`
	Exchange      string  `json:"exchange,omitempty"`
	ProductType   string  `json:"type,omitempty"`
	Symbol        string  `json:"symbol,omitempty"`
	Contract      string  `json:"contract,omitempty"`
	ContractId    string  `json:"contract_id,omitempty"`
	OrderType     string  `json:"order_type,omitempty"`
	OrderFlag1    string  `json:"order_flag1,omitempty"`
	Dir           string  `json:"dir,omitempty"`
	Price         float64 `json:"price,omitempty"`
	Amount        float64 `json:"amount,omitempty"`
	TotalPrice    float64 `json:"total_price,omitempty"`
	Timestamp     int64   `json:"ts,omitempty"`
}

type MessageQuery struct {
	QueryId             string      `json:"qry_id"`
	CurrencyId          string      `json:"currency_id,omitempty"`
	UserId              string      `json:"user_id,omitempty"`
	OrderId             string      `json:"order_id,omitempty"`
	OutsideUserId       string      `json:"outside_user_id,omitempty"`
	OutsideId           string      `json:"outside_id,omitempty"`
	ClientOrderId       string      `json:"client_order_id,omitempty"`
	TradeId             string      `json:"trade_id,omitempty"`
	PositionSummaryType string      `json:"position_summary_type,omitempty"`
	PositionDetailType  string      `json:"position_detail_type,omitempty"`
	TradeAccountType    string      `json:"trade_account_type,omitempty"`
	ProductType         string      `json:"type,omitempty"`
	Market              string      `json:"market,omitempty"`
	Exchange            string      `json:"exchange,omitempty"`
	Symbol              string      `json:"symbol,omitempty"`
	Contract            string      `json:"contract,omitempty"`
	ContractId          string      `json:"contract_id,omitempty"`
	OrderType           string      `json:"order_type,omitempty"`
	OrderFlag1          string      `json:"order_flag1,omitempty"`
	Dir                 string      `json:"dir,omitempty"`
	Price               float64     `json:"price,omitempty"`
	Amount              float64     `json:"amount,omitempty"`
	TotalPrice          float64     `json:"total_price,omitempty"`
	Timestamp           int64       `json:"ts,omitempty"`
	Data                interface{} `json:"data,omitempty"`
}

type MessageOrderStatus struct {
	Status       int          `json:"status,omitempty"`
	SubmitStatus int          `json:"submit_status,omitempty"`
	Amount       float64      `json:"amount,omitempty"`
	DealedAmount float64      `json:"dealed_amount,omitempty"`
	Order        MessageOrder `json:"order,omitempty"`
}

type MessageOrderDeal struct {
	Price      float64      `json:"price,omitempty"`
	Amount     float64      `json:"amount,omitempty"`
	TradingDay string       `json:"trading_day,omitempty"`
	TradeId    string       `json:"trade_id,omitempty"`
	Timestamp  int64        `json:"ts,omitempty"`
	Order      MessageOrder `json:"order,omitempty"`
}

type MessageOrderTrade struct {
	Status       int          `json:"status,omitempty"`
	Timestamp    int64        `json:"ts,omitempty"`
	Price        float64      `json:"price,omitempty"`
	Amount       float64      `json:"amount,omitempty"`
	DealedAmount float64      `json:"dealed_amount,omitempty"`
	Order        MessageOrder `json:"order,omitempty"`
}

//////////////////////// system ////////////////////////
type MessageHttpHeader struct {
	User  string      `json:"user"`
	Token string      `json:"token"`
	Data  interface{} `json:"data"`
}

type MessageLogin struct {
	User     string `json:"user"`
	Password string `json:"password"`
}
