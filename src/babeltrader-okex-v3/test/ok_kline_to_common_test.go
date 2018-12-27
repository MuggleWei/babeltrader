package babeltrader_okex_v3_test

import (
	"fmt"
	"testing"
	"time"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	okex "github.com/MuggleWei/babel-trader/src/babeltrader-okex-v3"
)

func TestConvertSpotCandleToQuote(t *testing.T) {
	table := "spot/candle60s"
	candle := okex.Candle{
		Candle:       []string{"2018-12-22T16:27:00.000Z", "3810.8814", "3811.3878", "3804.1902", "3807.3586", "19.21354086"},
		InstrumentId: "BTC-USDT",
	}
	candles := []okex.Candle{candle}

	quoteMessages, err := okex.ConvertCandleToQuotes(table, candles)
	if err != nil {
		t.Error(err.Error())
	}

	if len(quoteMessages) != 1 {
		t.Error("failed get quote messages")
	}

	rsp := quoteMessages[0]
	if rsp.Message != "quote" {
		t.Error("message field is wrong")
	}
	quote, ok := rsp.Data.(common.MessageQuote)
	if !ok {
		t.Error("quote type is wrong")
	}

	if quote.Market != "okex" {
		t.Error("market field is wrong")
	}
	if quote.Exchange != "okex" {
		t.Error("market field is wrong")
	}
	if quote.Type != "spot" {
		t.Error("type field is wrong")
	}
	if quote.Symbol != "BTC-USDT" {
		t.Error("symbol field is wrong")
	}
	if quote.InfoPrimary != "kline" {
		t.Error("info1 field is wrong")
	}
	if quote.InfoExtra != "1m" {
		t.Error("info2 field is wrong")
	}

	kline, ok := quote.Data.(common.MessageQuoteKLine)
	if !ok {
		t.Error("kline field is wrong")
	}

	ts := time.Unix(kline.Timestamp/1000, int64(time.Millisecond)*(kline.Timestamp%1000)).UTC().Format("2006-01-02T15:04:05.000Z")
	open := fmt.Sprintf("%.4f", kline.Open)
	high := fmt.Sprintf("%.4f", kline.High)
	low := fmt.Sprintf("%.4f", kline.Low)
	close_price := fmt.Sprintf("%.4f", kline.Close)

	if ts != candle.Candle[0] {
		t.Errorf("kline timestamp not equal: %v != %v", ts, candle.Candle[0])
	}
	if open != candle.Candle[1] {
		t.Errorf("kline open price not equal: %v != %v", open, candle.Candle[1])
	}
	if high != candle.Candle[2] {
		t.Errorf("kline hgih price not equal: %v != %v", high, candle.Candle[2])
	}
	if low != candle.Candle[3] {
		t.Errorf("kline low price not equal: %v != %v", low, candle.Candle[3])
	}
	if close_price != candle.Candle[4] {
		t.Errorf("kline close price not equal: %v != %v", close_price, candle.Candle[4])
	}
}

func TestConvertFutureCandleToQuote(t *testing.T) {
	table := "futures/candle60s"
	candle := okex.Candle{
		Candle:       []string{"2018-12-22T16:27:00.000Z", "3701.99", "3702.38", "3696.18", "3696.9", "5980", "161.7434738645255"},
		InstrumentId: "BTC-USD-190329",
	}
	candles := []okex.Candle{candle}

	quoteMessages, err := okex.ConvertCandleToQuotes(table, candles)
	if err != nil {
		t.Error(err.Error())
	}

	if len(quoteMessages) != 1 {
		t.Error("failed get quote messages")
	}

	rsp := quoteMessages[0]
	if rsp.Message != "quote" {
		t.Error("message field is wrong")
	}
	quote, ok := rsp.Data.(common.MessageQuote)
	if !ok {
		t.Error("quote type is wrong")
	}

	if quote.Market != "okex" {
		t.Error("market field is wrong")
	}
	if quote.Exchange != "okex" {
		t.Error("market field is wrong")
	}
	if quote.Type != "future" {
		t.Error("type field is wrong")
	}
	if quote.Symbol != "BTC-USD" {
		t.Error("symbol field is wrong")
	}
	if quote.Contract != "190329" {
		t.Error("contract field is wrong")
	}
	if quote.InfoPrimary != "kline" {
		t.Error("info1 field is wrong")
	}

	kline, ok := quote.Data.(common.MessageQuoteKLine)
	if !ok {
		t.Error("kline field is wrong")
	}

	ts := time.Unix(kline.Timestamp/1000, int64(time.Millisecond)*(kline.Timestamp%1000)).UTC().Format("2006-01-02T15:04:05.000Z")
	open := fmt.Sprintf("%.2f", kline.Open)
	high := fmt.Sprintf("%.2f", kline.High)
	low := fmt.Sprintf("%.2f", kline.Low)
	close_price := fmt.Sprintf("%.1f", kline.Close)
	vol := fmt.Sprintf("%d", int64(kline.Vol))

	if ts != candle.Candle[0] {
		t.Errorf("kline timestamp not equal: %v != %v", ts, candle.Candle[0])
	}
	if open != candle.Candle[1] {
		t.Errorf("kline open price not equal: %v != %v", open, candle.Candle[1])
	}
	if high != candle.Candle[2] {
		t.Errorf("kline hgih price not equal: %v != %v", high, candle.Candle[2])
	}
	if low != candle.Candle[3] {
		t.Errorf("kline low price not equal: %v != %v", low, candle.Candle[3])
	}
	if close_price != candle.Candle[4] {
		t.Errorf("kline close price not equal: %v != %v", close_price, candle.Candle[4])
	}
	if vol != candle.Candle[5] {
		t.Errorf("kline vol not equal: %v != %v", vol, candle.Candle[5])
	}
}

func TestConvertSwapCandleToQuote(t *testing.T) {
	table := "swap/candle60s"
	candle := okex.Candle{
		Candle:       []string{"2018-12-27T15:26:00.000Z", "3715.3", "3718", "3715.3", "3718", "47", "1.2648"},
		InstrumentId: "BTC-USD-SWAP",
	}
	candles := []okex.Candle{candle}

	quoteMessages, err := okex.ConvertCandleToQuotes(table, candles)
	if err != nil {
		t.Error(err.Error())
	}

	if len(quoteMessages) != 1 {
		t.Error("failed get quote messages")
	}

	rsp := quoteMessages[0]
	if rsp.Message != "quote" {
		t.Error("message field is wrong")
	}
	quote, ok := rsp.Data.(common.MessageQuote)
	if !ok {
		t.Error("quote type is wrong")
	}

	if quote.Market != "okex" {
		t.Error("market field is wrong")
	}
	if quote.Exchange != "okex" {
		t.Error("market field is wrong")
	}
	if quote.Type != "future" {
		t.Error("type field is wrong")
	}
	if quote.Symbol != "BTC-USD" {
		t.Error("symbol field is wrong")
	}
	if quote.Contract != "SWAP" {
		t.Error("contract field is wrong")
	}
	if quote.InfoPrimary != "kline" {
		t.Error("info1 field is wrong")
	}

	kline, ok := quote.Data.(common.MessageQuoteKLine)
	if !ok {
		t.Error("kline field is wrong")
	}

	ts := time.Unix(kline.Timestamp/1000, int64(time.Millisecond)*(kline.Timestamp%1000)).UTC().Format("2006-01-02T15:04:05.000Z")
	open := fmt.Sprintf("%.1f", kline.Open)
	high := fmt.Sprintf("%.0f", kline.High)
	low := fmt.Sprintf("%.1f", kline.Low)
	close_price := fmt.Sprintf("%.0f", kline.Close)
	vol := fmt.Sprintf("%d", int64(kline.Vol))

	if ts != candle.Candle[0] {
		t.Errorf("kline timestamp not equal: %v != %v", ts, candle.Candle[0])
	}
	if open != candle.Candle[1] {
		t.Errorf("kline open price not equal: %v != %v", open, candle.Candle[1])
	}
	if high != candle.Candle[2] {
		t.Errorf("kline hgih price not equal: %v != %v", high, candle.Candle[2])
	}
	if low != candle.Candle[3] {
		t.Errorf("kline low price not equal: %v != %v", low, candle.Candle[3])
	}
	if close_price != candle.Candle[4] {
		t.Errorf("kline close price not equal: %v != %v", close_price, candle.Candle[4])
	}
	if vol != candle.Candle[5] {
		t.Errorf("kline vol not equal: %v != %v", vol, candle.Candle[5])
	}
}
