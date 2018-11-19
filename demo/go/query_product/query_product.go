package main

import (
	"log"
	"time"

	DemoTrade "github.com/MuggleWei/babel-trader/demo/go/demo_trade"
	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
)

func main() {
	log.SetFlags(log.LstdFlags | log.LUTC | log.Lmicroseconds | log.Lshortfile)

	// ctp
	addr := "127.0.0.1:8001"
	qry_symbol := common.MessageQuery{
		QueryId:  "1",
		UserId:   "weidaizi",
		Market:   common.Market_CTP,
		Exchange: common.Exchange_SHFE,
		Symbol:   "rb",
	}

	qry_contract := common.MessageQuery{
		QueryId:  "2",
		UserId:   "weidaizi",
		Market:   common.Market_CTP,
		Exchange: common.Exchange_SHFE,
		Symbol:   "rb",
		Contract: "1905",
	}

	service := DemoTrade.NewDemoTradeService()
	service.Run(addr, func() {
		service.ReqQueryProduct(&qry_symbol)
		time.Sleep(time.Second * 3)
		service.ReqQueryProduct(&qry_contract)
	})
}
