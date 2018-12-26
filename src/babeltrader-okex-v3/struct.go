package babeltrader_okex_common

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
