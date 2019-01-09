package babeltrader_utils_go_test

import (
	"fmt"
	"testing"
	"time"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	utils "github.com/MuggleWei/babel-trader/src/babeltrader-utils-go"
)

func TestAddOrder(t *testing.T) {
	cacheManager := utils.NewOrderCacheManager()
	order := common.MessageOrder{
		OrderId:   "xxx",
		OutsideId: "yyy",
	}

	err := cacheManager.AddCache(&order)
	if err != nil {
		t.Error(err.Error())
	}

	ret, err := cacheManager.GetOrder(order.OutsideId)
	if err != nil {
		t.Error(err.Error())
	}
	if ret.OutsideId != order.OutsideId {
		s := fmt.Sprintf("outside order id not equal: %v != %v", ret.OutsideId, order.OutsideId)
		t.Error(s)
	}
	if ret.OrderId != order.OrderId {
		t.Error("order id not equal")
	}
}

func TestRepeatedAddOrder(t *testing.T) {
	cacheManager := utils.NewOrderCacheManager()
	order := common.MessageOrder{
		OrderId:   "xxx",
		OutsideId: "yyy",
	}

	err := cacheManager.AddCache(&order)
	if err != nil {
		t.Error(err.Error())
	}

	err = cacheManager.AddCache(&order)
	if err == nil {
		t.Error("failed repeated add")
	}
}

func TestDelExpiredOrder(t *testing.T) {
	cacheManager := utils.NewOrderCacheManager()
	order := common.MessageOrder{
		OrderId:   "xxx",
		OutsideId: "yyy",
	}

	err := cacheManager.AddCache(&order)
	if err != nil {
		t.Error(err.Error())
	}

	time.Sleep(time.Duration(2) * time.Second)
	cacheManager.CleanExpire(1)
	_, err = cacheManager.GetOrder(order.OutsideId)
	if err != nil {
		t.Error(err.Error())
	}

	cacheManager.UpdateOrderStatus(order.OutsideId, common.OrderStatus_AllDealed)
	time.Sleep(time.Duration(2) * time.Second)
	cacheManager.CleanExpire(1)
	_, err = cacheManager.GetOrder(order.OutsideId)
	if err == nil {
		t.Error("failed to clean expired order")
	}
}
