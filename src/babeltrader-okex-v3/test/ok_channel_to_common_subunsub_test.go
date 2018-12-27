package babeltrader_okex_v3_test

import (
	"testing"

	okex "github.com/MuggleWei/babel-trader/src/babeltrader-okex-v3"
)

func TestConvertSpotTicker(t *testing.T) {
	channel := "spot/ticker:BTC-USDT"

	msg, err := okex.ConvertChannelToSubUnsub(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.Market != "okex" {
		t.Error("wrong market")
	}
	if msg.Exchange != "okex" {
		t.Error("wrong exchange")
	}
	if msg.Type != "spot" {
		t.Error("wrong spot")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.InfoPrimary != "ticker" {
		t.Error("wrong info1")
	}
}

func TestConvertSpotKline(t *testing.T) {
	channel := "spot/candle60s:BTC-USDT"

	msg, err := okex.ConvertChannelToSubUnsub(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.Market != "okex" {
		t.Error("wrong market")
	}
	if msg.Exchange != "okex" {
		t.Error("wrong exchange")
	}
	if msg.Type != "spot" {
		t.Error("wrong spot")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.InfoPrimary != "kline" {
		t.Error("wrong info1")
	}
	if msg.InfoExtra != "1m" {
		t.Error("wrong info1")
	}
}

func TestConvertSpotDepth(t *testing.T) {
	channel := "spot/depth5:BTC-USDT"

	msg, err := okex.ConvertChannelToSubUnsub(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.Market != "okex" {
		t.Error("wrong market")
	}
	if msg.Exchange != "okex" {
		t.Error("wrong exchange")
	}
	if msg.Type != "spot" {
		t.Error("wrong spot")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.InfoPrimary != "depth" {
		t.Error("wrong info1")
	}
}

func TestConvertSpotDepthL2(t *testing.T) {
	channel := "spot/depth:BTC-USDT"

	msg, err := okex.ConvertChannelToSubUnsub(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.Market != "okex" {
		t.Error("wrong market")
	}
	if msg.Exchange != "okex" {
		t.Error("wrong exchange")
	}
	if msg.Type != "spot" {
		t.Error("wrong spot")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.InfoPrimary != "depthL2" {
		t.Error("wrong info1")
	}
}

func TestConvertFutureTicker(t *testing.T) {
	channel := "futures/ticker:BTC-USDT-190329"

	msg, err := okex.ConvertChannelToSubUnsub(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.Market != "okex" {
		t.Error("wrong market")
	}
	if msg.Exchange != "okex" {
		t.Error("wrong exchange")
	}
	if msg.Type != "future" {
		t.Error("wrong future")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "190329" {
		t.Error("wrong contract")
	}
	if msg.InfoPrimary != "ticker" {
		t.Error("wrong info1")
	}
}

func TestConvertFutureKline(t *testing.T) {
	channel := "futures/candle60s:BTC-USDT-190329"

	msg, err := okex.ConvertChannelToSubUnsub(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.Market != "okex" {
		t.Error("wrong market")
	}
	if msg.Exchange != "okex" {
		t.Error("wrong exchange")
	}
	if msg.Type != "future" {
		t.Error("wrong future")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "190329" {
		t.Error("wrong contract")
	}
	if msg.InfoPrimary != "kline" {
		t.Error("wrong info1")
	}
	if msg.InfoExtra != "1m" {
		t.Error("wrong info1")
	}
}

func TestConvertFutureDepth(t *testing.T) {
	channel := "futures/depth5:BTC-USDT-190329"

	msg, err := okex.ConvertChannelToSubUnsub(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.Market != "okex" {
		t.Error("wrong market")
	}
	if msg.Exchange != "okex" {
		t.Error("wrong exchange")
	}
	if msg.Type != "future" {
		t.Error("wrong future")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "190329" {
		t.Error("wrong contract")
	}
	if msg.InfoPrimary != "depth" {
		t.Error("wrong info1")
	}
}

func TestConvertFutureDepthL2(t *testing.T) {
	channel := "futures/depth:BTC-USDT-190329"

	msg, err := okex.ConvertChannelToSubUnsub(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.Market != "okex" {
		t.Error("wrong market")
	}
	if msg.Exchange != "okex" {
		t.Error("wrong exchange")
	}
	if msg.Type != "future" {
		t.Error("wrong future")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "190329" {
		t.Error("wrong contract")
	}
	if msg.InfoPrimary != "depthL2" {
		t.Error("wrong info1")
	}
}

func TestConvertSwapTicker(t *testing.T) {
	channel := "swap/ticker:BTC-USDT-SWAP"

	msg, err := okex.ConvertChannelToSubUnsub(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.Market != "okex" {
		t.Error("wrong market")
	}
	if msg.Exchange != "okex" {
		t.Error("wrong exchange")
	}
	if msg.Type != "future" {
		t.Error("wrong future")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "SWAP" {
		t.Error("wrong contract")
	}
	if msg.InfoPrimary != "ticker" {
		t.Error("wrong info1")
	}
}

func TestConvertSwapKline(t *testing.T) {
	channel := "swap/candle60s:BTC-USDT-SWAP"

	msg, err := okex.ConvertChannelToSubUnsub(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.Market != "okex" {
		t.Error("wrong market")
	}
	if msg.Exchange != "okex" {
		t.Error("wrong exchange")
	}
	if msg.Type != "future" {
		t.Error("wrong future")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "SWAP" {
		t.Error("wrong contract")
	}
	if msg.InfoPrimary != "kline" {
		t.Error("wrong info1")
	}
	if msg.InfoExtra != "1m" {
		t.Error("wrong info1")
	}
}

func TestConvertSwapDepth(t *testing.T) {
	channel := "swap/depth5:BTC-USDT-SWAP"

	msg, err := okex.ConvertChannelToSubUnsub(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.Market != "okex" {
		t.Error("wrong market")
	}
	if msg.Exchange != "okex" {
		t.Error("wrong exchange")
	}
	if msg.Type != "future" {
		t.Error("wrong future")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "SWAP" {
		t.Error("wrong contract")
	}
	if msg.InfoPrimary != "depth" {
		t.Error("wrong info1")
	}
}

func TestConvertSwapDepthL2(t *testing.T) {
	channel := "swap/depth:BTC-USDT-SWAP"

	msg, err := okex.ConvertChannelToSubUnsub(channel)
	if err != nil {
		t.Error(err.Error())
	}

	if msg.Market != "okex" {
		t.Error("wrong market")
	}
	if msg.Exchange != "okex" {
		t.Error("wrong exchange")
	}
	if msg.Type != "future" {
		t.Error("wrong future")
	}
	if msg.Symbol != "BTC-USDT" {
		t.Error("wrong symbol")
	}
	if msg.Contract != "SWAP" {
		t.Error("wrong contract")
	}
	if msg.InfoPrimary != "depthL2" {
		t.Error("wrong info1")
	}
}
