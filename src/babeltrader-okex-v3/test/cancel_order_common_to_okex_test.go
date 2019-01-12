package babeltrader_okex_v3_test

import (
	"testing"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	okex "github.com/MuggleWei/babel-trader/src/babeltrader-okex-v3"
	"github.com/stretchr/testify/assert"
)

func TestConvertCancelOrderCommon2OkexSpot(t *testing.T) {
	order := common.MessageOrder{
		ProductType: common.ProductType_Spot,
		Symbol:      "BTC-USDT",
		OutsideId:   "xxx",
	}

	productType, okexOrder, err := okex.ConvertCancelOrderCommon2Okex(&order)
	if err != nil {
		t.Error(err.Error())
	}
	assert.Equal(t, productType, "spot", "failed convert product type")
	assert.Equal(t, okexOrder.InstrumentId, "BTC-USDT", "failed convert symbol")
}

func TestConvertCancelOrderCommon2OkexFuture(t *testing.T) {
	order := common.MessageOrder{
		ProductType: common.ProductType_Future,
		Symbol:      "BTC-USD",
		Contract:    "190329",
		OutsideId:   "xxx",
	}

	productType, okexOrder, err := okex.ConvertCancelOrderCommon2Okex(&order)
	if err != nil {
		t.Error(err.Error())
	}
	assert.Equal(t, productType, "futures", "failed convert product type")
	assert.Equal(t, okexOrder.InstrumentId, "BTC-USD-190329", "failed convert symbol")
}
