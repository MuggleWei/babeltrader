package babeltrader_okex_v3_test

import (
	"fmt"
	"testing"
	"time"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	okex "github.com/MuggleWei/babel-trader/src/babeltrader-okex-v3"
)

func TestConvertSpotTickerToQuote(t *testing.T) {
	table := "spot/ticker"
	tickers := []okex.Ticker{
		okex.Ticker{
			InstrumentId: "BTC-USDT",
			Last:         "3773.6417",
			BestBid:      "3773.6523",
			BestAsk:      "3774.7527",
			Open24H:      "3813.1247",
			High24H:      "3880",
			Low24H:       "3659.2804",
			BaseVol24H:   "39365.1185632",
			QuoteVol24H:  "148245304.02456281",
			Timestamp:    "2018-12-30T14:53:20.732Z",
		},
	}

	quoteMessages, err := okex.ConvertTickerToQuotes(table, tickers)
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
	if quote.InfoPrimary != "ticker" {
		t.Error("info1 field is wrong")
	}

	ticker, ok := quote.Data.(common.MessageQuoteTicker)
	if !ok {
		t.Error("ticker field is wrong")
	}

	ts := time.Unix(ticker.Timestamp/1000, int64(time.Millisecond)*(ticker.Timestamp%1000)).UTC().Format("2006-01-02T15:04:05.000Z")
	last := fmt.Sprintf("%.4f", ticker.Last)
	bid := fmt.Sprintf("%.4f", ticker.Bid)
	ask := fmt.Sprintf("%.4f", ticker.Ask)
	high := fmt.Sprintf("%.0f", ticker.High)
	low := fmt.Sprintf("%.4f", ticker.Low)
	vol := fmt.Sprintf("%.7f", ticker.Vol)

	if ts != tickers[0].Timestamp {
		t.Errorf("ticker timestamp not equal: %v != %v", ts, tickers[0].Timestamp)
	}
	if last != tickers[0].Last {
		t.Errorf("ticker last not equal: %v != %v", last, tickers[0].Last)
	}
	if bid != tickers[0].BestBid {
		t.Errorf("ticker bid not equal: %v != %v", bid, tickers[0].BestBid)
	}
	if ask != tickers[0].BestAsk {
		t.Errorf("ticker ask not equal: %v != %v", ask, tickers[0].BestAsk)
	}
	if high != tickers[0].High24H {
		t.Errorf("ticker high not equal: %v != %v", high, tickers[0].High24H)
	}
	if low != tickers[0].Low24H {
		t.Errorf("ticker low not equal: %v != %v", low, tickers[0].Low24H)
	}
	if vol != tickers[0].BaseVol24H {
		t.Errorf("ticker vol not equal: %v != %v", vol, tickers[0].BaseVol24H)
	}
}

func TestConvertFuturesTickerToQuote(t *testing.T) {
	table := "futures/ticker"
	tickers := []okex.Ticker{
		okex.Ticker{
			InstrumentId: "BTC-USD-190329",
			Last:         "3688.72",
			BestBid:      "3687.96",
			BestAsk:      "3688.9",
			Open24H:      "",
			High24H:      "3788.0",
			Low24H:       "3551.7",
			Vol24H:       "8580040",
			Timestamp:    "2018-12-30T14:53:18.155Z",
		},
	}

	quoteMessages, err := okex.ConvertTickerToQuotes(table, tickers)
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
	if quote.InfoPrimary != "ticker" {
		t.Error("info1 field is wrong")
	}

	ticker, ok := quote.Data.(common.MessageQuoteTicker)
	if !ok {
		t.Error("ticker field is wrong")
	}

	ts := time.Unix(ticker.Timestamp/1000, int64(time.Millisecond)*(ticker.Timestamp%1000)).UTC().Format("2006-01-02T15:04:05.000Z")
	last := fmt.Sprintf("%.2f", ticker.Last)
	bid := fmt.Sprintf("%.2f", ticker.Bid)
	ask := fmt.Sprintf("%.1f", ticker.Ask)
	high := fmt.Sprintf("%.1f", ticker.High)
	low := fmt.Sprintf("%.1f", ticker.Low)
	vol := fmt.Sprintf("%.0f", ticker.Vol)

	if ts != tickers[0].Timestamp {
		t.Errorf("ticker timestamp not equal: %v != %v", ts, tickers[0].Timestamp)
	}
	if last != tickers[0].Last {
		t.Errorf("ticker last not equal: %v != %v", last, tickers[0].Last)
	}
	if bid != tickers[0].BestBid {
		t.Errorf("ticker bid not equal: %v != %v", bid, tickers[0].BestBid)
	}
	if ask != tickers[0].BestAsk {
		t.Errorf("ticker ask not equal: %v != %v", ask, tickers[0].BestAsk)
	}
	if high != tickers[0].High24H {
		t.Errorf("ticker high not equal: %v != %v", high, tickers[0].High24H)
	}
	if low != tickers[0].Low24H {
		t.Errorf("ticker low not equal: %v != %v", low, tickers[0].Low24H)
	}
	if vol != tickers[0].Vol24H {
		t.Errorf("ticker vol not equal: %v != %v", vol, tickers[0].Vol24H)
	}
}

func TestConvertSwapTickerToQuote(t *testing.T) {
	table := "swap/ticker"
	tickers := []okex.Ticker{
		okex.Ticker{
			InstrumentId: "BTC-USD-SWAP",
			Last:         "3802.1",
			BestBid:      "3800.9",
			BestAsk:      "3803.3",
			Open24H:      "",
			High24H:      "3950",
			Low24H:       "3676",
			Vol24H:       "370907",
			Timestamp:    "2018-12-30T14:53:08.410Z",
		},
	}

	quoteMessages, err := okex.ConvertTickerToQuotes(table, tickers)
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
	if quote.InfoPrimary != "ticker" {
		t.Error("info1 field is wrong")
	}

	ticker, ok := quote.Data.(common.MessageQuoteTicker)
	if !ok {
		t.Error("ticker field is wrong")
	}

	ts := time.Unix(ticker.Timestamp/1000, int64(time.Millisecond)*(ticker.Timestamp%1000)).UTC().Format("2006-01-02T15:04:05.000Z")
	last := fmt.Sprintf("%.1f", ticker.Last)
	bid := fmt.Sprintf("%.1f", ticker.Bid)
	ask := fmt.Sprintf("%.1f", ticker.Ask)
	high := fmt.Sprintf("%.0f", ticker.High)
	low := fmt.Sprintf("%.0f", ticker.Low)
	vol := fmt.Sprintf("%.0f", ticker.Vol)

	if ts != tickers[0].Timestamp {
		t.Errorf("ticker timestamp not equal: %v != %v", ts, tickers[0].Timestamp)
	}
	if last != tickers[0].Last {
		t.Errorf("ticker last not equal: %v != %v", last, tickers[0].Last)
	}
	if bid != tickers[0].BestBid {
		t.Errorf("ticker bid not equal: %v != %v", bid, tickers[0].BestBid)
	}
	if ask != tickers[0].BestAsk {
		t.Errorf("ticker ask not equal: %v != %v", ask, tickers[0].BestAsk)
	}
	if high != tickers[0].High24H {
		t.Errorf("ticker high not equal: %v != %v", high, tickers[0].High24H)
	}
	if low != tickers[0].Low24H {
		t.Errorf("ticker low not equal: %v != %v", low, tickers[0].Low24H)
	}
	if vol != tickers[0].Vol24H {
		t.Errorf("ticker vol not equal: %v != %v", vol, tickers[0].Vol24H)
	}
}
