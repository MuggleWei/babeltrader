package babeltrader_utils_go

import (
	"errors"
	"fmt"
	"time"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
)

type orderCache struct {
	order  *common.MessageOrder
	status int
	ts     int64
}

type OrderCacheManager struct {
	OutsideOidMap map[string]*orderCache
}

func NewOrderCacheManager() *OrderCacheManager {
	return &OrderCacheManager{
		OutsideOidMap: make(map[string]*orderCache),
	}
}

func (this *OrderCacheManager) AddCache(order *common.MessageOrder) error {
	if order.OutsideId == "" {
		return errors.New("cache order without outside id")
	}

	_, ok := this.OutsideOidMap[order.OutsideId]
	if ok {
		return errors.New("repeated cache order")
	}

	this.OutsideOidMap[order.OutsideId] = &orderCache{
		order:  order,
		status: common.OrderStatus_Unknown,
		ts:     0,
	}

	return nil
}

func (this *OrderCacheManager) UpdateOrderStatus(outsideOrderId string, status int) error {
	cacheOrder, ok := this.OutsideOidMap[outsideOrderId]
	if !ok {
		s := fmt.Sprintf("could not find order with outside order id: %v", outsideOrderId)
		return errors.New(s)
	}

	cacheOrder.status = status

	if status == common.OrderStatus_AllDealed || status == common.OrderStatus_Canceled || status == common.OrderStatus_PartCanceled || status == common.OrderStatus_Rejected {
		cacheOrder.ts = time.Now().Unix()
	}

	return nil
}

func (this *OrderCacheManager) GetOrder(outsideOid string) (*common.MessageOrder, error) {
	cacheOrder, ok := this.OutsideOidMap[outsideOid]
	if !ok {
		s := fmt.Sprintf("failed find outside order id: %v", outsideOid)
		return nil, errors.New(s)
	}

	return cacheOrder.order, nil
}

func (this *OrderCacheManager) CleanExpire(expireSecond int64) {
	ts := time.Now().Unix()

	var waitDelOutsideOid []string
	for outsideOid, cacheOrder := range this.OutsideOidMap {
		if cacheOrder.ts > 0 && ts-cacheOrder.ts > expireSecond {
			waitDelOutsideOid = append(waitDelOutsideOid, outsideOid)
		}
	}

	for _, outsideOid := range waitDelOutsideOid {
		delete(this.OutsideOidMap, outsideOid)
	}
}
