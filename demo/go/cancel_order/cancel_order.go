package main

import (
	"log"
	"time"

	DemoTrade "github.com/MuggleWei/babel-trader/demo/go/demo_trade"
	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
)

func main() {
	log.SetFlags(log.LstdFlags | log.LUTC | log.Lmicroseconds | log.Lshortfile)

	ts := time.Now().Unix()

	// // ctp
	// addr := "127.0.0.1:8001"
	// order := common.MessageOrder{
	// 	UserId:      "weidaizi",
	// 	OutsideId:   "104027_20181112_      244440",
	// 	Market:      common.Market_CTP,
	// 	Exchange:    common.Exchange_SHFE,
	// 	ProductType: common.ProductType_Future,
	// 	Symbol:      "rb",
	// 	Contract:    "1902",
	// 	Timestamp:   ts,
	// }

	// xtp
	addr := "127.0.0.1:8002"
	order := common.MessageOrder{
		UserId:      "weidaizi",
		OutsideId:   "15033731_20181118_36567563152394204",
		Market:      common.Market_XTP,
		Exchange:    common.Exchange_SSE,
		ProductType: common.ProductType_Spot,
		Symbol:      "600519",
		Timestamp:   ts,
	}

	service := DemoTrade.NewDemoTradeService()
	service.Run(addr, func() {
		service.ReqCancelOrder(&order)
	})

}
