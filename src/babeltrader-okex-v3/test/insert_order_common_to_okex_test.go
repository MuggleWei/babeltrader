package babeltrader_okex_v3_test

import (
	"testing"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	okex "github.com/MuggleWei/babel-trader/src/babeltrader-okex-v3"

	"github.com/stretchr/testify/assert"
)

func TestConvertInsertOrderCommon2Okex_SpotLimitBuy(t *testing.T) {
	order := common.MessageOrder{
		ProductType: common.ProductType_Spot,
		Symbol:      "BTC-USDT",
		OrderType:   common.OrderType_Limit,
		Dir:         common.OrderAction_Buy,
		Price:       1000,
		Amount:      5,
	}

	productType, okexOrder, err := okex.ConvertInsertOrderCommon2Okex(&order)
	if err != nil {
		t.Error(err.Error())
	}
	assert.Equal(t, productType, "spot", "failed convert product type")
	assert.Equal(t, okexOrder.Side, "buy", "failed convert dir")
	assert.Equal(t, okexOrder.Price, "1000", "failed convert price")
	assert.Equal(t, okexOrder.Size, "5", "failed convert amount")
}

func TestConvertInsertOrderCommon2Okex_SpotLimitSell(t *testing.T) {
	order := common.MessageOrder{
		ProductType: common.ProductType_Spot,
		Symbol:      "BTC-USDT",
		OrderType:   common.OrderType_Limit,
		Dir:         common.OrderAction_Sell,
		Price:       1000,
		Amount:      5,
	}

	productType, okexOrder, err := okex.ConvertInsertOrderCommon2Okex(&order)
	if err != nil {
		t.Error(err.Error())
	}
	assert.Equal(t, productType, "spot", "failed convert product type")
	assert.Equal(t, okexOrder.Side, "sell", "failed convert dir")
	assert.Equal(t, okexOrder.Price, "1000", "failed convert price")
	assert.Equal(t, okexOrder.Size, "5", "failed convert amount")
}

func TestConvertInsertOrderCommon2Okex_SpotMarketBuy(t *testing.T) {
	order := common.MessageOrder{
		ProductType: common.ProductType_Spot,
		Symbol:      "BTC-USDT",
		OrderType:   common.OrderType_Market,
		Dir:         common.OrderAction_Buy,
		TotalPrice:  500,
	}

	productType, okexOrder, err := okex.ConvertInsertOrderCommon2Okex(&order)
	if err != nil {
		t.Error(err.Error())
	}
	assert.Equal(t, productType, "spot", "failed convert product type")
	assert.Equal(t, okexOrder.Side, "buy", "failed convert dir")
	assert.Equal(t, okexOrder.Notional, "500", "failed convert total price")
}

func TestConvertInsertOrderCommon2Okex_SpotMarketSell(t *testing.T) {
	order := common.MessageOrder{
		ProductType: common.ProductType_Spot,
		Symbol:      "BTC-USDT",
		OrderType:   common.OrderType_Market,
		Dir:         common.OrderAction_Sell,
		Amount:      5,
	}

	productType, okexOrder, err := okex.ConvertInsertOrderCommon2Okex(&order)
	if err != nil {
		t.Error(err.Error())
	}
	assert.Equal(t, productType, "spot", "failed convert product type")
	assert.Equal(t, okexOrder.Side, "sell", "failed convert dir")
	assert.Equal(t, okexOrder.Size, "5", "failed convert amount")
}

func TestConvertInsertOrderCommon2Okex_FutureLimitOpenLong(t *testing.T) {
	order := common.MessageOrder{
		ProductType: common.ProductType_Future,
		Symbol:      "BTC-USD",
		Contract:    "190329",
		OrderType:   common.OrderType_Limit,
		Dir:         common.OrderAction_Open + "_" + common.OrderDir_Long,
		Price:       3700,
		Amount:      5,
	}

	productType, okexOrder, err := okex.ConvertInsertOrderCommon2Okex(&order)
	if err != nil {
		t.Error(err.Error())
	}
	assert.Equal(t, productType, "futures", "failed convert product type")
	assert.Equal(t, okexOrder.Type, "1", "failed convert dir")
	assert.Equal(t, okexOrder.Price, "3700", "failed convert price")
	assert.Equal(t, okexOrder.Size, "5", "failed convert amount")
	assert.Equal(t, okexOrder.Leverage, "20", "failed convert leverage")
}

func TestConvertInsertOrderCommon2Okex_FutureLimitOpenShort(t *testing.T) {
	order := common.MessageOrder{
		ProductType: common.ProductType_Future,
		Symbol:      "BTC-USD",
		Contract:    "190329",
		OrderType:   common.OrderType_Limit,
		Dir:         common.OrderAction_Open + "_" + common.OrderDir_Short,
		Price:       3700,
		Amount:      5,
	}

	productType, okexOrder, err := okex.ConvertInsertOrderCommon2Okex(&order)
	if err != nil {
		t.Error(err.Error())
	}
	assert.Equal(t, productType, "futures", "failed convert product type")
	assert.Equal(t, okexOrder.Type, "2", "failed convert dir")
	assert.Equal(t, okexOrder.Price, "3700", "failed convert price")
	assert.Equal(t, okexOrder.Size, "5", "failed convert amount")
	assert.Equal(t, okexOrder.Leverage, "20", "failed convert leverage")
}

func TestConvertInsertOrderCommon2Okex_FutureLimitCloseLong(t *testing.T) {
	order := common.MessageOrder{
		ProductType: common.ProductType_Future,
		Symbol:      "BTC-USD",
		Contract:    "190329",
		OrderType:   common.OrderType_Limit,
		Dir:         common.OrderAction_Close + "_" + common.OrderDir_Long,
		Price:       3700,
		Amount:      5,
	}

	productType, okexOrder, err := okex.ConvertInsertOrderCommon2Okex(&order)
	if err != nil {
		t.Error(err.Error())
	}
	assert.Equal(t, productType, "futures", "failed convert product type")
	assert.Equal(t, okexOrder.Type, "3", "failed convert dir")
	assert.Equal(t, okexOrder.Price, "3700", "failed convert price")
	assert.Equal(t, okexOrder.Size, "5", "failed convert amount")
	assert.Equal(t, okexOrder.Leverage, "20", "failed convert leverage")
}

func TestConvertInsertOrderCommon2Okex_FutureLimitCloseShort(t *testing.T) {
	order := common.MessageOrder{
		ProductType: common.ProductType_Future,
		Symbol:      "BTC-USD",
		Contract:    "190329",
		OrderType:   common.OrderType_Limit,
		Dir:         common.OrderAction_Close + "_" + common.OrderDir_Short,
		Price:       3700,
		Amount:      5,
	}

	productType, okexOrder, err := okex.ConvertInsertOrderCommon2Okex(&order)
	if err != nil {
		t.Error(err.Error())
	}
	assert.Equal(t, productType, "futures", "failed convert product type")
	assert.Equal(t, okexOrder.Type, "4", "failed convert dir")
	assert.Equal(t, okexOrder.Price, "3700", "failed convert price")
	assert.Equal(t, okexOrder.Size, "5", "failed convert amount")
	assert.Equal(t, okexOrder.Leverage, "20", "failed convert leverage")
}
