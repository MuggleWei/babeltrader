package babeltrader_okex_v3

type ReqCommon struct {
	Op   string      `json:"op"`
	Args interface{} `json:"args"`
}

type RspCommon struct {
	Event   string      `json:"event"`
	Tabel   string      `json:"table"`
	Data    interface{} `json:"data"`
	Success string      `json:"success"` // for login response
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

// NOTE: fucking okex v3 api, spot/swap ticker's price/vol fields use string, futures's price/vol fields use double?!!! :(
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
