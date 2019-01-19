package main

import (
	"encoding/json"
	"fmt"
	"log"
	"time"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	utils "github.com/MuggleWei/babel-trader/src/babeltrader-utils-go"
	"github.com/MuggleWei/cascade"
	"github.com/gorilla/websocket"
)

type ReqCallback func(*cascade.Peer, *common.MessageReqCommon) error
type RspCallback func(*common.MessageRspCommon)

type ClientService struct {
	Hub               *cascade.Hub
	TradeService      *OkexTradeService
	PeerMsgLinker     *utils.PeerMsgLinker
	OrderCacheManager *utils.OrderCacheManager
	OrderTradeCache   map[string][]*common.MessageOrderTrade
	ReqCallbacks      map[string]ReqCallback
	RspCallbacks      map[string]RspCallback
}

func NewClientService() *ClientService {
	service := &ClientService{
		Hub:               nil,
		TradeService:      nil,
		PeerMsgLinker:     utils.NewPeerMsgLinker(),
		OrderCacheManager: utils.NewOrderCacheManager(),
		OrderTradeCache:   make(map[string][]*common.MessageOrderTrade),
		ReqCallbacks:      make(map[string]ReqCallback),
		RspCallbacks:      make(map[string]RspCallback),
	}

	service.RegisterCallbacks()

	upgrader := websocket.Upgrader{
		ReadBufferSize:  1024 * 20,
		WriteBufferSize: 1024 * 20,
	}

	service.Hub = cascade.NewHub(service, &upgrader, 10240)

	go func() {
		ticker := time.NewTicker(time.Second * 60)
		for _ = range ticker.C {
			service.Hub.ByteMessageChannel <- &cascade.HubByteMessage{Peer: nil, Message: []byte("{\"msg\":\"timer_ticker\"}")}
		}
	}()

	return service
}

func (this *ClientService) RegisterCallbacks() {
	this.ReqCallbacks["insert_order"] = this.InsertOrder
	this.ReqCallbacks["cancel_order"] = this.CancelOrder
	this.ReqCallbacks["query_order"] = this.QueryOrder
	this.ReqCallbacks["query_trade"] = this.QueryTrade
	this.ReqCallbacks["query_position"] = this.QueryPosition
	this.ReqCallbacks["query_tradeaccount"] = this.QueryTradeAccount
	this.ReqCallbacks["query_product"] = this.QueryProduct

	this.RspCallbacks["timer_ticker"] = this.OnTimerTicker
	this.RspCallbacks["confirmorder"] = this.OnConfirmOrder
	this.RspCallbacks["ordertrade"] = this.OnOrderTrade
}

func (this *ClientService) Run() {
	go this.Hub.Run()
}

///////////////// Slot callbacks /////////////////
func (this *ClientService) OnActive(peer *cascade.Peer) {
	log.Printf("peer active: %v\n", peer.Conn.RemoteAddr().String())
	this.PeerMsgLinker.AddPeer(peer)
}

func (this *ClientService) OnInactive(peer *cascade.Peer) {
	log.Printf("peer inactive: %v\n", peer.Conn.RemoteAddr().String())
	this.PeerMsgLinker.DelPeer(peer)
}

func (this *ClientService) OnRead(peer *cascade.Peer, message []byte) {
	log.Printf("[Info] recv message from client: %v\n", string(message))

	var req common.MessageReqCommon
	err := json.Unmarshal(message, &req)
	if err != nil {
		log.Printf("[Warning] failed unmarshal message: %v, %v\n", string(message), err.Error())
		utils.ResponseError(peer, -1, err.Error(), nil)
		return
	}

	fn, ok := this.ReqCallbacks[req.Message]
	if !ok {
		s := fmt.Sprintf("[Warning] failed find callback function for %v", req.Message)
		log.Println(s)
		utils.ResponseError(peer, -1, s, req)
		return
	}

	err = fn(peer, &req)
	if err != nil {
		log.Println(err.Error())
		utils.ResponseError(peer, -1, err.Error(), req)
		return
	}
}

func (this *ClientService) OnHubByteMessage(msg *cascade.HubByteMessage) {
	var rsp common.MessageRspCommon
	err := json.Unmarshal(msg.Message, &rsp)
	if err != nil {
		log.Printf("failed unmarshal message: %v\n", string(msg.Message))
		return
	}

	callback, ok := this.RspCallbacks[rsp.Message]
	if !ok {
		log.Printf("failed find callback of: %v", rsp.Message)
		return
	}

	callback(&rsp)
}

func (this *ClientService) OnHubObjectMessage(msg *cascade.HubObjectMessage) {
	callback, ok := this.RspCallbacks[msg.ObjectName]
	if !ok {
		log.Printf("[Warning] OnHubObjectMessage default handle %v\n", msg.ObjectName)
		return
	}

	rsp := msg.ObjectPtr.(*common.MessageRspCommon)
	callback(rsp)
}

///////////////// req callbacks /////////////////
func (this *ClientService) InsertOrder(peer *cascade.Peer, req *common.MessageReqCommon) error {
	var order common.MessageOrder
	err := utils.DecodeInterfaceByJson(req.Data, &order)
	if err != nil {
		log.Printf("[Warning] failed to decode data: %v, %v\n", *req, err.Error())
		return err
	}

	outsideOrderId, err := this.TradeService.InsertOrder(peer, &order)
	if err != nil {
		log.Printf("[Warning] failed to insert order: %v, %v\n", order, err.Error())
		return err
	}

	// confirm order
	order.OutsideId = outsideOrderId
	rsp := common.MessageRspCommon{
		Message: "confirmorder",
		Data:    order,
	}

	this.Hub.ObjectMessageChannel <- &cascade.HubObjectMessage{
		Peer:       peer,
		ObjectName: "confirmorder",
		ObjectPtr:  &rsp,
	}

	return nil
}
func (this *ClientService) CancelOrder(peer *cascade.Peer, req *common.MessageReqCommon) error {
	var order common.MessageOrder
	err := utils.DecodeInterfaceByJson(req.Data, &order)
	if err != nil {
		log.Printf("[Warning] failed to decode data: %v, %v\n", *req, err.Error())
		return err
	}

	outsideOrderId, err := this.TradeService.CancelOrder(peer, &order)
	if err != nil {
		log.Printf("[Warning] failed to cancel order: %v, %v\n", order, err.Error())
		return err
	}

	log.Printf("[Info] cancel order: %v\n", outsideOrderId)

	return nil
}
func (this *ClientService) QueryOrder(peer *cascade.Peer, req *common.MessageReqCommon) error {
	// TODO:
	return nil
}
func (this *ClientService) QueryTrade(peer *cascade.Peer, req *common.MessageReqCommon) error {
	// TODO:
	return nil
}
func (this *ClientService) QueryPosition(peer *cascade.Peer, req *common.MessageReqCommon) error {
	// TODO:
	return nil
}
func (this *ClientService) QueryTradeAccount(peer *cascade.Peer, req *common.MessageReqCommon) error {
	// TODO:
	return nil
}
func (this *ClientService) QueryProduct(peer *cascade.Peer, req *common.MessageReqCommon) error {
	// TODO:
	return nil
}

///////////////// rsp callbacks /////////////////
func (this *ClientService) OnTimerTicker(*common.MessageRspCommon) {
	this.OrderCacheManager.CleanExpire(60)
	this.PeerMsgLinker.CleanExpire(60)
}

func (this *ClientService) OnConfirmOrder(rsp *common.MessageRspCommon) {
	order, ok := rsp.Data.(common.MessageOrder)
	if !ok {
		log.Printf("[Error] failed get order form MessageRspCommon: %v\n", *rsp)
		return
	}
	this.OrderCacheManager.AddCache(&order)

	b, err := json.Marshal(*rsp)
	if err != nil {
		log.Printf("[Warning] failed marshal message: %v\n", *rsp)
		return
	}
	this.BroadcastMsg(b)

	orderTradeCache, ok := this.OrderTradeCache[order.OutsideId]
	if ok {
		for _, orderTrade := range orderTradeCache {
			orderTrade.Order.UserId = order.UserId
			orderTrade.Order.OrderId = order.OrderId
			orderTrade.Order.ClientOrderId = order.ClientOrderId

			rsp := common.MessageRspCommon{
				Message: "ordertrade",
				Data:    *orderTrade,
			}

			b, err := json.Marshal(rsp)
			if err != nil {
				log.Printf("[Error] failed marshal message: %v\n", rsp)
				continue
			}
			this.BroadcastMsg(b)
		}
		delete(this.OrderTradeCache, order.OutsideId)
	}
}

func (this *ClientService) OnOrderTrade(rsp *common.MessageRspCommon) {
	orderTrade, ok := rsp.Data.(common.MessageOrderTrade)
	if !ok {
		log.Printf("[Error] failed get order trade form MessageRspCommon: %v\n", *rsp)
		return
	}

	cacheOrder, err := this.OrderCacheManager.GetOrder(orderTrade.Order.OutsideId)
	if err != nil {
		_, ok := this.OrderTradeCache[orderTrade.Order.OutsideId]
		if !ok {
			this.OrderTradeCache[orderTrade.Order.OutsideId] = []*common.MessageOrderTrade{&orderTrade}
		} else {
			this.OrderTradeCache[orderTrade.Order.OutsideId] = append(this.OrderTradeCache[orderTrade.Order.OutsideId], &orderTrade)
		}
		log.Printf("[Info] cache order trade: %v\n", orderTrade)
	} else {
		orderTrade.Order.UserId = cacheOrder.UserId
		orderTrade.Order.OrderId = cacheOrder.OrderId
		orderTrade.Order.ClientOrderId = cacheOrder.ClientOrderId

		rsp := common.MessageRspCommon{
			Message: "ordertrade",
			Data:    orderTrade,
		}

		b, err := json.Marshal(rsp)
		if err != nil {
			log.Printf("[Error] failed marshal message: %v\n", rsp)
		}
		this.BroadcastMsg(b)

		this.OrderCacheManager.UpdateOrderStatus(orderTrade.Order.OutsideId, orderTrade.Status)
	}
}

///////////////// utils methods /////////////////
func (this *ClientService) BroadcastMsg(msg []byte) {
	log.Printf("broadcast message: %v\n", string(msg))
	for peer := range this.Hub.Peers {
		peer.SendChannel <- msg
	}
}
