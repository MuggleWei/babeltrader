package main

import (
	"fmt"
	"log"
	"time"

	DemoTrade "github.com/MuggleWei/babel-trader/demo/go/demo_trade"
	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
)

func main() {
	log.SetFlags(log.LstdFlags | log.LUTC | log.Lmicroseconds | log.Lshortfile)

	ts := time.Now().Unix()
	orderId := fmt.Sprintf("weidaizi-%v", ts)

	// // ctp
	// addr := "127.0.0.1:8001"
	// order := common.MessageOrder{
	// 	UserId:        "weidaizi",
	// 	OrderId:       orderId,
	// 	ClientOrderId: orderId,
	// 	Market:        common.Market_CTP,
	// 	Exchange:      common.Exchange_SHFE,
	// 	ProductType:   common.ProductType_Future,
	// 	Symbol:        "rb",
	// 	Contract:      "1902",
	// 	OrderType:     common.OrderType_Limit,
	// 	OrderFlag1:    common.OrderFlag1_Speculation,
	// 	Dir:           common.OrderAction_Open + "_" + common.OrderDir_Short,
	// 	Price:         3800,
	// 	Timestamp:     ts,
	// }

	// xtp
	addr := "127.0.0.1:8002"
	order := common.MessageOrder{
		UserId:        "weidaizi",
		OrderId:       orderId,
		ClientOrderId: orderId,
		Market:        common.Market_XTP,
		Exchange:      common.Exchange_SSE,
		ProductType:   common.ProductType_Spot,
		Symbol:        "600519",
		OrderType:     common.OrderType_Limit,
		Dir:           common.OrderAction_Buy,
		Price:         580,
		Amount:        200,
		Timestamp:     ts,
	}

	service := DemoTrade.NewDemoTradeService()
	service.Run(addr, func() {
		service.ReqInsertOrder(&order)
	})
}
