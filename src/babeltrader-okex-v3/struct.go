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

// NOTE: the fucking okex v3 api, spot/swap ticker's price/vol fields use string, futures's price/vol fields use double?!!! :(
type FuturesTicker struct {
	InstrumentId string  `json:"instrument_id"`
	Last         float64 `json:"last"`
	BestBid      float64 `json:"best_bid"`
	BestAsk      float64 `json:"best_ask"`
	Open24H      float64 `json:"open_24h"`
	High24H      float64 `json:"high_24h"`
	Low24H       float64 `json:"low_24h"`
	Vol24H       float64 `json:"volume_24h"`
	Timestamp    string  `json:"timestamp"`
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
	OrderType    string `json:"type,omitempty"` // limit, market
	Side         string `json:"side,omitempty"` // buy, sell,1(open_long), 2(open_short), 3(close_long), 4(close_short)
	Price        string `json:"price,omitempty"`
	Size         string `json:"size,omitempty"`
	Notional     string `json:"notional,omitempty"`
	Leverage     string `json:"leverage,omitempty"`
}

type SpotOrderRet struct {
	OrderId   string `json:"order_id"`
	ClientOid string `json:"client_oid"`
	ErrId     string `json:"error_code"`
	ErrMsg    string `json:"error_message"`
	Result    bool   `json:"result"`
}

// callback function type
type OkexMsgCallback func(msg *RspCommon)
