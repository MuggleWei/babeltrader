package main

import (
	"fmt"
	"time"

	DemoTrade "github.com/MuggleWei/babel-trader/demo/go/demo_trade"
	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
)

func main() {
	ts := time.Now().Unix()
	orderId := fmt.Sprintf("weidaizi-%v", ts)

	// xtp
	addr := "127.0.0.1:8002"
	order := common.MessageOrder{
		UserId:        "weidaizi",
		OrderId:       orderId,
		ClientOrderId: orderId,
		Market:        "xtp",
		Exchange:      "SSE",
		ProductType:   "spot",
		Symbol:        "600519",
		OrderType:     "limit",
		Dir:           "buy",
		Price:         580,
		Amount:        100,
		Timestamp:     ts,
	}

	service := DemoTrade.NewDemoTradeService()
	service.Run(addr, func() {
		service.ReqInsertOrder(&order)
	})
}
