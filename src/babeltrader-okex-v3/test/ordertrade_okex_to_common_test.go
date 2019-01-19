package babeltrader_okex_v3_test

import (
	"encoding/json"
	"errors"
	"fmt"
	"strings"
	"testing"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	okex "github.com/MuggleWei/babel-trader/src/babeltrader-okex-v3"
	utils "github.com/MuggleWei/babel-trader/src/babeltrader-utils-go"
	"github.com/stretchr/testify/assert"
)

func ParseJsonToOrderTrades(jsonStr string) (string, []okex.OrderTrade, error) {
	var rsp okex.RspCommon
	err := json.Unmarshal([]byte(jsonStr), &rsp)
	if err != nil {
		return "", nil, err
	}

	idx := strings.Index(rsp.Table, "/")
	if idx < 0 && idx >= len(rsp.Table)-1 {
		s := fmt.Sprintf("failed split table %v", rsp.Table)
		return "", nil, errors.New(s)
	}
	productType := rsp.Table[:idx]

	var okexTrades []okex.OrderTrade
	err = utils.DecodeInterfaceByJson(rsp.Data, &okexTrades)
	if err != nil {
		return "", nil, err
	}

	return productType, okexTrades, nil
}

func TestConvertOrderTradeOkexToCommon_Spot(t *testing.T) {
	jsonStr := `{"table":"spot/order","data":[{"filled_notional":"0","filled_size":"0","instrument_id":"BTC-USDT","margin_trading":"1","notional":"","order_id":"2151181205905408","price":"30","side":"buy","size":"0.01","status":"cancelled","timestamp":"2019-01-15T13:55:29.000Z","type":"limit"}]}`

	productType, okexTrades, err := ParseJsonToOrderTrades(jsonStr)
	if err != nil {
		t.Error(err.Error())
	}

	for _, okexTrade := range okexTrades {
		rsp, err := okex.ConvertOrderTradeOkex2Common(productType, &okexTrade)
		if err != nil {
			t.Error(err.Error())
			return
		}

		assert.Equal(t, "ordertrade", rsp.Message, "message id is wrong")

		orderTrade := rsp.Data.(common.MessageOrderTrade)
		assert.Equal(t, common.OrderStatus_Canceled, orderTrade.Status, "status is wrong")
		assert.Equal(t, int64(1547560529000), orderTrade.Timestamp, "ts is wrong")
		assert.Equal(t, 0.0, orderTrade.DealedAmount, "dealed amount is wrong")
		assert.Equal(t, 0.0, orderTrade.AvgPrice, "avg price is wrong")

		assert.Equal(t, "2151181205905408", orderTrade.Order.OutsideId, "outside id is wrong")
		assert.Equal(t, common.Market_OKEX, orderTrade.Order.Market, "market is wrong")
		assert.Equal(t, common.Exchange_OKEX, orderTrade.Order.Exchange, "market is wrong")
		assert.Equal(t, common.ProductType_Spot, orderTrade.Order.ProductType, "market is wrong")
		assert.Equal(t, "BTC-USDT", orderTrade.Order.Symbol, "symbol is wrong")
		assert.Equal(t, common.OrderType_Limit, orderTrade.Order.OrderType, "order type is wrong")
		assert.Equal(t, common.OrderAction_Buy, orderTrade.Order.Dir, "dir is wrong")
		assert.Equal(t, 30.0, orderTrade.Order.Price, "price is wrong")
		assert.Equal(t, 0.01, orderTrade.Order.Amount, "amount is wrong")
	}
}

func TestConvertOrderTradeOkexToCommon_Future(t *testing.T) {
	jsonStr := `{"table":"futures/order","data":[{"leverage":"20","size":"1","filled_qty":"0","price":"3700.0","fee":"0","contract_val":"100","price_avg":"0.0","type":"2","instrument_id":"BTC-USD-190329","order_id":"2151193271902208","timestamp":"2019-01-15T13:56:44.000Z","status":"-1"}]}`

	productType, okexTrades, err := ParseJsonToOrderTrades(jsonStr)
	if err != nil {
		t.Error(err.Error())
	}

	for _, okexTrade := range okexTrades {
		rsp, err := okex.ConvertOrderTradeOkex2Common(productType, &okexTrade)
		if err != nil {
			t.Error(err.Error())
			return
		}

		assert.Equal(t, "ordertrade", rsp.Message, "message id is wrong")
		orderTrade := rsp.Data.(common.MessageOrderTrade)
		assert.Equal(t, common.OrderStatus_Canceled, orderTrade.Status, "status is wrong")
		assert.Equal(t, int64(1547560604000), orderTrade.Timestamp, "ts is wrong")
		assert.Equal(t, 0.0, orderTrade.DealedAmount, "dealed amount is wrong")
		assert.Equal(t, 0.0, orderTrade.AvgPrice, "avg price is wrong")

		assert.Equal(t, "2151193271902208", orderTrade.Order.OutsideId, "outside id is wrong")
		assert.Equal(t, common.Market_OKEX, orderTrade.Order.Market, "market is wrong")
		assert.Equal(t, common.Exchange_OKEX, orderTrade.Order.Exchange, "market is wrong")
		assert.Equal(t, common.ProductType_Future, orderTrade.Order.ProductType, "market is wrong")
		assert.Equal(t, "BTC-USD", orderTrade.Order.Symbol, "symbol is wrong")
		assert.Equal(t, "190329", orderTrade.Order.Contract, "contract is wrong")
		assert.Equal(t, common.OrderType_Limit, orderTrade.Order.OrderType, "order type is wrong")
		assert.Equal(t, common.OrderAction_Open+"_"+common.OrderDir_Short, orderTrade.Order.Dir, "dir is wrong")
		assert.Equal(t, 3700.0, orderTrade.Order.Price, "price is wrong")
		assert.Equal(t, 1.0, orderTrade.Order.Amount, "amount is wrong")
		assert.Equal(t, 20.0, orderTrade.Order.Leverage, "leverage is wrong")
	}
}
