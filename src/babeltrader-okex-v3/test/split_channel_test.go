package babeltrader_okex_v3_test

import (
	"testing"

	okex "github.com/MuggleWei/babel-trader/src/babeltrader-okex-v3"
)

func TestSplitChannelSubUnsubSpotDepth(t *testing.T) {
	channel := "spot/depth:BTC-USDT"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "spot" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "depthL2" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}

func TestSplitChannelSubUnsubSpotDepth5(t *testing.T) {
	channel := "spot/depth5:BTC-USDT"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "spot" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "depth" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}

func TestSplitChannelSubUnsubSpotTicker(t *testing.T) {
	channel := "spot/ticker:BTC-USDT"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "spot" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "ticker" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}

func TestSplitChannelSubUnsubSpotCandle(t *testing.T) {
	channel := "spot/candle60s:BTC-USDT"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "spot" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "kline" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "1m" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}

func TestSplitTableSpotDepth(t *testing.T) {
	channel := "spot/depth"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "spot" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "depthL2" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}

func TestSplitTableSpotDepth5(t *testing.T) {
	channel := "spot/depth5"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "spot" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "depth" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}

func TestSplitTableSpotTicker(t *testing.T) {
	channel := "spot/ticker"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "spot" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "ticker" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}

func TestSplitTableSpotCandle(t *testing.T) {
	channel := "spot/candle60s"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "spot" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "kline" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "1m" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}

func TestSplitChannelSubUnsubFuturesDepth(t *testing.T) {
	channel := "futures/depth:BTC-USD-190329"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "depthL2" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "BTC-USD" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "190329" {
		t.Error("wrong contract")
	}
}

func TestSplitChannelSubUnsubFuturesDepth5(t *testing.T) {
	channel := "futures/depth5:BTC-USD-190329"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "depth" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "BTC-USD" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "190329" {
		t.Error("wrong contract")
	}
}

func TestSplitChannelSubUnsubFuturesTicker(t *testing.T) {
	channel := "futures/ticker:BTC-USD-190329"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "ticker" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "BTC-USD" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "190329" {
		t.Error("wrong contract")
	}
}

func TestSplitChannelSubUnsubFuturesCandle(t *testing.T) {
	channel := "futures/candle60s:BTC-USD-190329"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "kline" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "1m" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "BTC-USD" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "190329" {
		t.Error("wrong contract")
	}
}

func TestSplitTableFuturesDepth(t *testing.T) {
	channel := "futures/depth"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "depthL2" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}

func TestSplitTableFuturesDepth5(t *testing.T) {
	channel := "futures/depth5"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "depth" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}

func TestSplitTableFuturesTicker(t *testing.T) {
	channel := "futures/ticker"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "ticker" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}

func TestSplitTableFutureCandle(t *testing.T) {
	channel := "futures/candle60s"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "kline" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "1m" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}

func TestSplitChannelSubUnsubSwapDepth(t *testing.T) {
	channel := "swap/depth:BTC-USD-SWAP"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "depthL2" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "BTC-USD" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "SWAP" {
		t.Error("wrong contract")
	}
}

func TestSplitChannelSubUnsubSwapDepth5(t *testing.T) {
	channel := "swap/depth5:BTC-USD-SWAP"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "depth" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "BTC-USD" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "SWAP" {
		t.Error("wrong contract")
	}
}

func TestSplitChannelSubUnsubSwapTicker(t *testing.T) {
	channel := "swap/ticker:BTC-USD-SWAP"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "ticker" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "BTC-USD" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "SWAP" {
		t.Error("wrong contract")
	}
}

func TestSplitChannelSubUnsubSwapCandle(t *testing.T) {
	channel := "swap/candle60s:BTC-USD-SWAP"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "kline" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "1m" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "BTC-USD" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "SWAP" {
		t.Error("wrong contract")
	}
}

func TestSplitTableSwapDepth(t *testing.T) {
	channel := "swap/depth"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "depthL2" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}

func TestSplitTableSwapDepth5(t *testing.T) {
	channel := "swap/depth5"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "depth" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}

func TestSplitTableSwapTicker(t *testing.T) {
	channel := "swap/ticker"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "ticker" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}

func TestSplitTableSwapCandle(t *testing.T) {
	channel := "swap/candle60s"

	msg, err := okex.SplitChannel(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.ProductType != "future" {
		t.Error("wrong product type")
	}
	if msg.Info1 != "kline" {
		t.Error("wrong info1")
	}
	if msg.Info2 != "1m" {
		t.Error("wrong info2")
	}
	if msg.Symbol != "" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "" {
		t.Error("wrong contract")
	}
}
