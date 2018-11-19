package main

import (
	"log"

	DemoTrade "github.com/MuggleWei/babel-trader/demo/go/demo_trade"
	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
)

func main() {
	log.SetFlags(log.LstdFlags | log.LUTC | log.Lmicroseconds | log.Lshortfile)

	// ctp
	addr := "127.0.0.1:8001"
	qry := common.MessageQuery{
		QueryId: "1",
		UserId:  "weidaizi",
		Market:  common.Market_CTP,
	}

	// xtp
	// addr := "127.0.0.1:8002"
	// qry := common.MessageQuery{
	// 	QueryId: "1",
	// 	UserId:  "weidaizi",
	// 	Market:  common.Market_XTP,
	// }

	service := DemoTrade.NewDemoTradeService()
	service.Run(addr, func() {
		service.ReqQueryPosition(&qry)
	})
}
