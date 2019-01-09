package main

import (
	"encoding/json"
	"fmt"
	"log"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	utils "github.com/MuggleWei/babel-trader/src/babeltrader-utils-go"
	"github.com/MuggleWei/cascade"
	"github.com/gorilla/websocket"
)

type ReqCallback func(*cascade.Peer, *common.MessageReqCommon) error

type ClientService struct {
	Hub           *cascade.Hub
	TradeService  *OkexTradeService
	PeerMsgLinker *utils.PeerMsgLinker
	ReqCallbacks  map[string]ReqCallback
}

func NewClientService() *ClientService {
	service := &ClientService{
		Hub:           nil,
		TradeService:  nil,
		PeerMsgLinker: utils.NewPeerMsgLinker(),
		ReqCallbacks:  make(map[string]ReqCallback),
	}

	service.RegisterCallbacks()

	upgrader := websocket.Upgrader{
		ReadBufferSize:  1024 * 20,
		WriteBufferSize: 1024 * 20,
	}

	service.Hub = cascade.NewHub(service, &upgrader, 10240)

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
	for peer := range this.Hub.Peers {
		peer.SendChannel <- msg.Message
	}
}

func (this *ClientService) OnHubObjectMessage(msg *cascade.HubObjectMessage) {
	switch msg.ObjectName {
	// TODO:
	}
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

	// TODO: generate order event message to notify clients

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
