package babeltrader_okex_v3_test

import (
	"encoding/json"
	"testing"
	"time"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	okex "github.com/MuggleWei/babel-trader/src/babeltrader-okex-v3"
	utils "github.com/MuggleWei/babel-trader/src/babeltrader-utils-go"
)

func getTableAndData(message []byte, data interface{}) (string, error) {
	var msg okex.RspCommon
	err := json.Unmarshal(message, &msg)
	if err != nil {
		return "", err
	}

	err = utils.DecodeInterfaceByJson(msg.Data, &data)
	if err != nil {
		return "", err
	}

	return msg.Table, nil
}

func TestConvertSpotDepth5ToQuote(t *testing.T) {
	message := []byte(`{"table":"spot/depth5","data":[{"asks":[["3779.2966","0.01006451",1],["3780.2272","0.581",1],["3780.2377","0.792",1],["3780.2378","1.7999",2],["3780.2379","0.05",1]],"bids":[["3778.3553","0.001",1],["3778.2572","0.4",1],["3778.0171","0.418",1],["3778.0065","0.04426079",1],["3778.0063","0.01",1]],"instrument_id":"BTC-USDT","timestamp":"2018-12-30T15:34:44.707Z"}]}`)

	var depths []okex.Depth

	table, err := getTableAndData(message, &depths)
	if err != nil {
		t.Error(err.Error())
		return
	}

	quotes, err := okex.ConvertDepthToQuotes(table, depths)
	if err != nil {
		t.Error(err.Error())
		return
	}

	if len(quotes) != 1 {
		t.Error("failed get quote messages")
	}

	rsp := quotes[0]
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
	if quote.InfoPrimary != "depth" {
		t.Error("info1 field is wrong")
	}

	depth, ok := quote.Data.(common.MessageQuoteDepth)
	if !ok {
		t.Error("depth field is wrong")
	}

	ts := time.Unix(depth.Timestamp/1000, int64(time.Millisecond)*(depth.Timestamp%1000)).UTC().Format("2006-01-02T15:04:05.000Z")
	asks := depth.Asks
	bids := depth.Bids

	if ts != "2018-12-30T15:34:44.707Z" {
		t.Errorf("depth timestamp not equal: %v != %v", ts, "2018-12-30T15:34:44.707Z")
	}
	if asks[0][0] != 3779.2966 {
		t.Errorf("depth ask1 price not equal: %v != %v", asks[0][0], 3779.2966)
	}
	if asks[0][1] != 0.01006451 {
		t.Errorf("depth ask1 vol not equal: %v != %v", asks[0][1], 0.01006451)
	}
	if asks[1][0] < asks[0][0] {
		t.Errorf("depth asks order wrong: %v", asks)
	}

	if bids[0][0] != 3778.3553 {
		t.Errorf("depth bid1 price not equal: %v != %v", bids[0][0], 3778.3553)
	}
	if bids[0][1] != 0.001 {
		t.Errorf("depth bid1 vol not equal: %v != %v", bids[0][1], 0.001)
	}
	if bids[1][0] > bids[0][0] {
		t.Errorf("depth bids order wrong: %v", bids)
	}
}

func TestConvertFuturesDepth5ToQuote(t *testing.T) {
	message := []byte(`{"table":"futures/depth5","data":[{"asks":[[3690.06,1,0,1],[3690.33,11,0,1],[3690.34,112,0,26],[3690.4,6,0,2],[3691.1,3,0,1]],"bids":[[3689.78,217,0,4],[3689.77,2,0,1],[3689.7,1,0,1],[3689.22,40,0,1],[3689.12,79,0,1]],"instrument_id":"BTC-USD-190329","timestamp":"2018-12-30T15:34:43.817Z"}]}`)

	var depths []okex.Depth

	table, err := getTableAndData(message, &depths)
	if err != nil {
		t.Error(err.Error())
		return
	}

	quotes, err := okex.ConvertDepthToQuotes(table, depths)
	if err != nil {
		t.Error(err.Error())
		return
	}

	if len(quotes) != 1 {
		t.Error("failed get quote messages")
	}

	rsp := quotes[0]
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
		t.Error("symbol field is wrong")
	}
	if quote.InfoPrimary != "depth" {
		t.Error("info1 field is wrong")
	}

	depth, ok := quote.Data.(common.MessageQuoteDepth)
	if !ok {
		t.Error("depth field is wrong")
	}

	ts := time.Unix(depth.Timestamp/1000, int64(time.Millisecond)*(depth.Timestamp%1000)).UTC().Format("2006-01-02T15:04:05.000Z")
	asks := depth.Asks
	bids := depth.Bids

	if ts != "2018-12-30T15:34:43.817Z" {
		t.Errorf("depth timestamp not equal: %v != %v", ts, "2018-12-30T15:34:43.817Z")
	}
	if asks[0][0] != 3690.06 {
		t.Errorf("depth ask1 price not equal: %v != %v", asks[0][0], 3690.06)
	}
	if asks[0][1] != 1 {
		t.Errorf("depth ask1 vol not equal: %v != %v", asks[0][1], 1)
	}
	if asks[1][0] < asks[0][0] {
		t.Errorf("depth asks order wrong: %v", asks)
	}

	if bids[0][0] != 3689.78 {
		t.Errorf("depth bid1 price not equal: %v != %v", bids[0][0], 3689.78)
	}
	if bids[0][1] != 217 {
		t.Errorf("depth bid1 vol not equal: %v != %v", bids[0][1], 217)
	}
	if bids[1][0] > bids[0][0] {
		t.Errorf("depth bids order wrong: %v", bids)
	}
}

func TestConvertSwapDepth5ToQuote(t *testing.T) {
	message := []byte(`{"table":"swap/depth5","data":[{"asks":[["3810.7","3",0,2],["3810.9","21",0,1],["3811","131",0,1],["3812.8","3",0,1],["3812.9","2",0,1]],"bids":[["3803.8","20",0,1],["3803.6","95",0,1],["3802.2","19",0,1],["3801","93",0,1],["3800.9","58",0,1]],"instrument_id":"BTC-USD-SWAP","timestamp":"2018-12-30T15:34:43.183Z"}]}`)

	var depths []okex.Depth

	table, err := getTableAndData(message, &depths)
	if err != nil {
		t.Error(err.Error())
		return
	}

	quotes, err := okex.ConvertDepthToQuotes(table, depths)
	if err != nil {
		t.Error(err.Error())
		return
	}

	if len(quotes) != 1 {
		t.Error("failed get quote messages")
	}

	rsp := quotes[0]
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
		t.Error("symbol field is wrong")
	}
	if quote.InfoPrimary != "depth" {
		t.Error("info1 field is wrong")
	}

	depth, ok := quote.Data.(common.MessageQuoteDepth)
	if !ok {
		t.Error("depth field is wrong")
	}

	ts := time.Unix(depth.Timestamp/1000, int64(time.Millisecond)*(depth.Timestamp%1000)).UTC().Format("2006-01-02T15:04:05.000Z")
	asks := depth.Asks
	bids := depth.Bids

	if ts != "2018-12-30T15:34:43.183Z" {
		t.Errorf("depth timestamp not equal: %v != %v", ts, "2018-12-30T15:34:43.183Z")
	}
	if asks[0][0] != 3810.7 {
		t.Errorf("depth ask1 price not equal: %v != %v", asks[0][0], 3810.7)
	}
	if asks[0][1] != 3 {
		t.Errorf("depth ask1 vol not equal: %v != %v", asks[0][1], 3)
	}
	if asks[1][0] < asks[0][0] {
		t.Errorf("depth asks order wrong: %v", asks)
	}

	if bids[0][0] != 3803.8 {
		t.Errorf("depth bid1 price not equal: %v != %v", bids[0][0], 3803.8)
	}
	if bids[0][1] != 20 {
		t.Errorf("depth bid1 vol not equal: %v != %v", bids[0][1], 20)
	}
	if bids[1][0] > bids[0][0] {
		t.Errorf("depth bids order wrong: %v", bids)
	}
}
