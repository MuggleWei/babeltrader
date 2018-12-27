package babeltrader_okex_v3_test

import (
	"fmt"
	"testing"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	okex "github.com/MuggleWei/babel-trader/src/babeltrader-okex-v3"
)

func TestConvertSpotTickerToChannel(t *testing.T) {
	quote := common.MessageQuote{
		Market:      "okex",
		Exchange:    "okex",
		Type:        "spot",
		Symbol:      "BTC-USDT",
		InfoPrimary: "ticker",
	}
	channel, err := okex.ConvertQuoteToChannel(&quote)
	if err != nil {
		t.Error(err.Error())
	}

	if channel != "spot/ticker:BTC-USDT" {
		s := fmt.Sprintf("error channel: %v", channel)
		t.Error(s)
	}
}

func TestConvertSpotKlineToChannel(t *testing.T) {
	quote := common.MessageQuote{
		Market:      "okex",
		Exchange:    "okex",
		Type:        "spot",
		Symbol:      "BTC-USDT",
		InfoPrimary: "kline",
		InfoExtra:   "1m",
	}
	channel, err := okex.ConvertQuoteToChannel(&quote)
	if err != nil {
		t.Error(err.Error())
	}

	if channel != "spot/candle60s:BTC-USDT" {
		s := fmt.Sprintf("error channel: %v", channel)
		t.Error(s)
	}
}

func TestConvertSpotDepthToChannel(t *testing.T) {
	quote := common.MessageQuote{
		Market:      "okex",
		Exchange:    "okex",
		Type:        "spot",
		Symbol:      "BTC-USDT",
		InfoPrimary: "depth",
	}
	channel, err := okex.ConvertQuoteToChannel(&quote)
	if err != nil {
		t.Error(err.Error())
	}

	if channel != "spot/depth5:BTC-USDT" {
		s := fmt.Sprintf("error channel: %v", channel)
		t.Error(s)
	}
}

func TestConvertSpotDepthL2ToChannel(t *testing.T) {
	quote := common.MessageQuote{
		Market:      "okex",
		Exchange:    "okex",
		Type:        "spot",
		Symbol:      "BTC-USDT",
		InfoPrimary: "depthL2",
	}
	channel, err := okex.ConvertQuoteToChannel(&quote)
	if err != nil {
		t.Error(err.Error())
	}

	if channel != "spot/depth:BTC-USDT" {
		s := fmt.Sprintf("error channel: %v", channel)
		t.Error(s)
	}
}

func TestConvertFutureTickerToChannel(t *testing.T) {
	quote := common.MessageQuote{
		Market:      "okex",
		Exchange:    "okex",
		Type:        "future",
		Symbol:      "BTC-USD",
		Contract:    "190329",
		InfoPrimary: "ticker",
	}
	channel, err := okex.ConvertQuoteToChannel(&quote)
	if err != nil {
		t.Error(err.Error())
	}

	if channel != "futures/ticker:BTC-USD-190329" {
		s := fmt.Sprintf("error channel: %v", channel)
		t.Error(s)
	}
}

func TestConvertFutureKlineToChannel(t *testing.T) {
	quote := common.MessageQuote{
		Market:      "okex",
		Exchange:    "okex",
		Type:        "future",
		Symbol:      "BTC-USD",
		Contract:    "190329",
		InfoPrimary: "kline",
		InfoExtra:   "1m",
	}
	channel, err := okex.ConvertQuoteToChannel(&quote)
	if err != nil {
		t.Error(err.Error())
	}

	if channel != "futures/candle60s:BTC-USD-190329" {
		s := fmt.Sprintf("error channel: %v", channel)
		t.Error(s)
	}
}

func TestConvertFutureDepthToChannel(t *testing.T) {
	quote := common.MessageQuote{
		Market:      "okex",
		Exchange:    "okex",
		Type:        "future",
		Symbol:      "BTC-USD",
		Contract:    "190329",
		InfoPrimary: "depth",
	}
	channel, err := okex.ConvertQuoteToChannel(&quote)
	if err != nil {
		t.Error(err.Error())
	}

	if channel != "futures/depth5:BTC-USD-190329" {
		s := fmt.Sprintf("error channel: %v", channel)
		t.Error(s)
	}
}

func TestConvertFutureDepthL2ToChannel(t *testing.T) {
	quote := common.MessageQuote{
		Market:      "okex",
		Exchange:    "okex",
		Type:        "future",
		Symbol:      "BTC-USD",
		Contract:    "190329",
		InfoPrimary: "depthL2",
	}
	channel, err := okex.ConvertQuoteToChannel(&quote)
	if err != nil {
		t.Error(err.Error())
	}

	if channel != "futures/depth:BTC-USD-190329" {
		s := fmt.Sprintf("error channel: %v", channel)
		t.Error(s)
	}
}

func TestConvertFutureSwapTickerToChannel(t *testing.T) {
	quote := common.MessageQuote{
		Market:      "okex",
		Exchange:    "okex",
		Type:        "future",
		Symbol:      "BTC-USD",
		Contract:    "SWAP",
		InfoPrimary: "ticker",
	}
	channel, err := okex.ConvertQuoteToChannel(&quote)
	if err != nil {
		t.Error(err.Error())
	}

	if channel != "swap/ticker:BTC-USD-SWAP" {
		s := fmt.Sprintf("error channel: %v", channel)
		t.Error(s)
	}
}

func TestConvertFutureSwapKlineToChannel(t *testing.T) {
	quote := common.MessageQuote{
		Market:      "okex",
		Exchange:    "okex",
		Type:        "future",
		Symbol:      "BTC-USD",
		Contract:    "SWAP",
		InfoPrimary: "kline",
		InfoExtra:   "1m",
	}
	channel, err := okex.ConvertQuoteToChannel(&quote)
	if err != nil {
		t.Error(err.Error())
	}

	if channel != "swap/candle60s:BTC-USD-SWAP" {
		s := fmt.Sprintf("error channel: %v", channel)
		t.Error(s)
	}
}

func TestConvertFutureSwapDepthToChannel(t *testing.T) {
	quote := common.MessageQuote{
		Market:      "okex",
		Exchange:    "okex",
		Type:        "future",
		Symbol:      "BTC-USD",
		Contract:    "SWAP",
		InfoPrimary: "depth",
	}
	channel, err := okex.ConvertQuoteToChannel(&quote)
	if err != nil {
		t.Error(err.Error())
	}

	if channel != "swap/depth5:BTC-USD-SWAP" {
		s := fmt.Sprintf("error channel: %v", channel)
		t.Error(s)
	}
}

func TestConvertFutureSwapDepthL2ToChannel(t *testing.T) {
	quote := common.MessageQuote{
		Market:      "okex",
		Exchange:    "okex",
		Type:        "future",
		Symbol:      "BTC-USD",
		Contract:    "SWAP",
		InfoPrimary: "depthL2",
	}
	channel, err := okex.ConvertQuoteToChannel(&quote)
	if err != nil {
		t.Error(err.Error())
	}

	if channel != "swap/depth:BTC-USD-SWAP" {
		s := fmt.Sprintf("error channel: %v", channel)
		t.Error(s)
	}
}
