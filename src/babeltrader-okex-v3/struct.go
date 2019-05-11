package babeltrader_okex_v3

type ReqCommon struct {
	Op   string      `json:"op"`
	Args interface{} `json:"args"`
}

type RspCommon struct {
	Event   string      `json:"event"`
	Table   string      `json:"table"`
	Data    interface{} `json:"data"`
	Success bool        `json:"success"` // for login response
	Channel string      `json:"channel"` // for subscribe
}

type ChannelSplit struct {
	ProductType string
	Info1       string
	Info2       string
	Symbol      string
	Contract    string
}

type Candle struct {
	InstrumentId string   `json:"instrument_id"`
	Candle       []string `json:"candle"`
}

type Ticker struct {
	InstrumentId string `json:"instrument_id"`
	Last         string `json:"last"`
	BestBid      string `json:"best_bid"`
	BestAsk      string `json:"best_ask"`
	Open24H      string `json:"open_24h"`
	High24H      string `json:"high_24h"`
	Low24H       string `json:"low_24h"`
	Vol24H       string `json:"volume_24h"`
	BaseVol24H   string `json:"base_volume_24h"`
	QuoteVol24H  string `json:"quote_volume_24h"`
	Timestamp    string `json:"timestamp"`
}

type FuturesTicker struct {
	InstrumentId string `json:"instrument_id"`
	Last         string `json:"last"`
	BestBid      string `json:"best_bid"`
	BestAsk      string `json:"best_ask"`
	Open24H      string `json:"open_24h"`
	High24H      string `json:"high_24h"`
	Low24H       string `json:"low_24h"`
	Vol24H       string `json:"volume_24h"`
	Timestamp    string `json:"timestamp"`
}

type Depth struct {
	Asks         [][]interface{} `json:"asks"`
	Bids         [][]interface{} `json:"bids"`
	InstrumentId string          `json:"instrument_id"`
	Timestamp    string          `json:"timestamp"`
}

type Order struct {
	ClientOid    string `json:"client_oid,omitempty"`
	InstrumentId string `json:"instrument_id"`
	Type         string `json:"type,omitempty"`
	Side         string `json:"side,omitempty"`
	Price        string `json:"price,omitempty"`
	Size         string `json:"size,omitempty"`
	Notional     string `json:"notional,omitempty"`
	Leverage     string `json:"leverage,omitempty"`
}

type Query struct {
	CurrencyId   string
	InstrumentId string
	ProductType  string
	OrderId      string
}

type OrderRet struct {
	OrderId   string `json:"order_id"`
	ClientOid string `json:"client_oid"`
	ErrId     string `json:"error_code"`
	ErrMsg    string `json:"error_message"`
	Result    bool   `json:"result"`
}

type OrderTrade struct {
	InstrumentId string `json:"instrument_id"`
	OrderId      string `json:"order_id"`
	Price        string `json:"price"`
	Size         string `json:"size"`
	Timestamp    string `json:"timestamp"`

	Notional       string `json:"notional"`
	Side           string `json:"side"`
	Type           string `json:"type"`
	FilledSize     string `json:"filled_size"`
	FilledNotional string `json:"filled_notional"`
	Status         string `json:"state"`
	MarginTrading  string `json:"margin_trading"`

	FilledQty   string `json:"filled_qty"`
	Fee         string `json:"fee"`
	PriceAvg    string `json:"price_avg"`
	ContractVal string `json:"contract_val"`
	Leverage    string `json:"leverage"`
}

type PositionHolding struct {
	MarginMode           string `json:"margin_mode"`
	LongQty              string `json:"long_qty"`
	LongAvailQty         string `json:"long_avail_qty"`
	LongAvgCost          string `json:"long_avg_cost"`
	LongSettlementPrice  string `json:"long_settlement_price"`
	ShortQty             string `json:"short_qty"`
	ShortAvailQty        string `json:"short_avail_qty"`
	ShortAvgCost         string `json:"short_avg_cost"`
	ShortSettlementPrice string `json:"short_settlement_price"`
	InstrumentId         string `json:"instrument_id"`
	Leverage             string `json:"leverage"`
}

type Position struct {
	MarginMode string            `json:"margin_mode"`
	Result     bool              `json:"result"`
	Holding    []PositionHolding `json:"holding"`
}

type TradeAccount struct {
	// futures
	MarginMode        string `json:"margin_mode"`
	TotalAvailBalance string `json:"total_avail_balance"`
	Equity            string `json:"equity"`
	MarginRatio       string `json:"margin_ratio"`

	// spot
	Balance   string `json:"balance"`
	Hold      string `json:"hold"`
	Available string `json:"available"`
}

// callback function type
type OkexMsgCallback func(msg *RspCommon)
