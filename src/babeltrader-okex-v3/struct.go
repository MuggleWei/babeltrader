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

type Candle struct {
	Candle       []string `json:"candle"`
	InstrumentId string   `json:"instrument_id"`
}
