package demo_trade

import (
	"encoding/json"
	"log"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	"github.com/MuggleWei/cascade"
)

type fnMessageCallback func(interface{})

type DemoTradeService struct {
	Hub              *cascade.Hub
	Peer             *cascade.Peer
	CallbackOnActive func()
	Callbacks        map[string]fnMessageCallback
}

func NewDemoTradeService() *DemoTradeService {
	service := &DemoTradeService{
		Hub:              nil,
		CallbackOnActive: nil,
		Callbacks:        make(map[string]fnMessageCallback),
	}

	hub := cascade.NewHub(service, nil, 0)
	service.Hub = hub

	service.RegisterCallbacks()

	return service
}

func (this *DemoTradeService) Run(addr string, callbackOnActive func()) {
	url := "ws://" + addr + "/ws"
	this.CallbackOnActive = callbackOnActive

	this.Hub.ConnectAndRun(url, true, 5, nil, func(addr string, err error) {
		log.Printf("[Error] failed dial to %v: %v", addr, err.Error())
	})
}

// Slot callbacks
func (this *DemoTradeService) OnActive(peer *cascade.Peer) {
	log.Printf("demo trade service connected: %v\n", peer.Conn.RemoteAddr().String())
	this.Peer = peer
	this.CallbackOnActive()
}

func (this *DemoTradeService) OnInactive(peer *cascade.Peer) {
	log.Printf("demo trade disconnected: %v\n", peer.Conn.RemoteAddr().String())
	this.Peer = nil
}

func (this *DemoTradeService) OnRead(peer *cascade.Peer, message []byte) {
	log.Printf("recv message: %v\n", string(message))

	var msg common.MessageRspCommon
	err := json.Unmarshal(message, &msg)
	if err != nil {
		log.Printf("failed unmarshal message: %v\n", string(message))
		return
	}

	fn, ok := this.Callbacks[msg.Message]
	if !ok {
		log.Printf("failed find callback function for %v\n", msg.Message)
		return
	}

	if msg.ErrId != 0 {
		log.Printf("error happend: %v\n", string(message))
		return
	}

	fn(msg.Data)
}

func (this *DemoTradeService) OnHubByteMessage(msg *cascade.HubByteMessage) {
	if this.Peer != nil {
		this.Peer.SendChannel <- msg.Message
	}
}

func (this *DemoTradeService) OnHubObjectMessage(*cascade.HubObjectMessage) {
}

//////////////////// method ////////////////////
func (this *DemoTradeService) RegisterCallbacks() {
	this.Callbacks["error"] = this.OnError
	this.Callbacks["confirmorder"] = this.OnConfirmOrder
	this.Callbacks["orderstatus"] = this.OnOrderStatus
	this.Callbacks["orderdeal"] = this.OnOrderDeal
	this.Callbacks["rsp_qryorder"] = this.OnRspQryOrder
	this.Callbacks["rsp_qrytrade"] = this.OnRspQryTrade
	this.Callbacks["rsp_qryposition"] = this.OnRspQryPosition
	this.Callbacks["rsp_qrypositiondetail"] = this.OnRspQryPositionDetail
	this.Callbacks["rsp_qrytradeaccount"] = this.OnRspQryTradeAccount
	this.Callbacks["rsp_qryproduct"] = this.OnRspQryProduct
}

func (this *DemoTradeService) OnError(data interface{}) {
	s, _ := json.Marshal(data)
	log.Printf("error: %v\n", string(s))
}
func (this *DemoTradeService) OnConfirmOrder(data interface{}) {
	s, _ := json.Marshal(data)
	log.Printf("confirmorder: %v\n", string(s))
}
func (this *DemoTradeService) OnOrderStatus(data interface{}) {
	s, _ := json.Marshal(data)
	log.Printf("orderstatus: %v\n", string(s))
}
func (this *DemoTradeService) OnOrderDeal(data interface{}) {
	s, _ := json.Marshal(data)
	log.Printf("orderdeal: %v\n", string(s))
}

func (this *DemoTradeService) OnRspQryOrder(data interface{}) {
	s, _ := json.Marshal(data)
	log.Printf("rsp_qryorder: %v\n", string(s))
}
func (this *DemoTradeService) OnRspQryTrade(data interface{}) {
	s, _ := json.Marshal(data)
	log.Printf("rsp_qrytrade: %v\n", string(s))
}
func (this *DemoTradeService) OnRspQryPosition(data interface{}) {
	s, _ := json.Marshal(data)
	log.Printf("rsp_qryposition: %v\n", string(s))
}
func (this *DemoTradeService) OnRspQryPositionDetail(data interface{}) {
	s, _ := json.Marshal(data)
	log.Printf("rsp_qrypositiondetail: %v\n", string(s))
}
func (this *DemoTradeService) OnRspQryTradeAccount(data interface{}) {
	s, _ := json.Marshal(data)
	log.Printf("rsp_qrytradeaccount: %v\n", string(s))
}
func (this *DemoTradeService) OnRspQryProduct(data interface{}) {
	s, _ := json.Marshal(data)
	log.Printf("rsp_qryproduct: %v\n", string(s))
}

func (this *DemoTradeService) ReqInsertOrder(order *common.MessageOrder) {
	req := common.MessageReqCommon{
		Message: "insert_order",
		Data:    *order,
	}

	msg, err := json.Marshal(req)
	if err != nil {
		log.Printf("failed to marshal message: %+v\n", req)
		return
	}

	this.Hub.ByteMessageChannel <- &cascade.HubByteMessage{Peer: nil, Message: msg}
}

func (this *DemoTradeService) ReqCancelOrder(order *common.MessageOrder) {
	req := common.MessageReqCommon{
		Message: "cancel_order",
		Data:    *order,
	}

	msg, err := json.Marshal(req)
	if err != nil {
		log.Printf("failed to marshal message: %+v\n", req)
		return
	}

	this.Hub.ByteMessageChannel <- &cascade.HubByteMessage{Peer: nil, Message: msg}
}

func (this *DemoTradeService) DoQuery(req *common.MessageReqCommon) {
	msg, err := json.Marshal(*req)
	if err != nil {
		log.Printf("failed to marshal query message: %+v\n", *req)
		return
	}

	this.Hub.ByteMessageChannel <- &cascade.HubByteMessage{Peer: nil, Message: msg}
}

func (this *DemoTradeService) ReqQueryOrder(qry *common.MessageQuery) {
	req := common.MessageReqCommon{
		Message: "query_order",
		Data:    *qry,
	}
	this.DoQuery(&req)
}
func (this *DemoTradeService) ReqQueryTrade(qry *common.MessageQuery) {
	req := common.MessageReqCommon{
		Message: "query_trade",
		Data:    *qry,
	}
	this.DoQuery(&req)
}
func (this *DemoTradeService) ReqQueryPosition(qry *common.MessageQuery) {
	req := common.MessageReqCommon{
		Message: "query_position",
		Data:    *qry,
	}
	this.DoQuery(&req)
}
func (this *DemoTradeService) ReqQueryPositionDetail(qry *common.MessageQuery) {
	req := common.MessageReqCommon{
		Message: "query_positiondetail",
		Data:    *qry,
	}
	this.DoQuery(&req)
}
func (this *DemoTradeService) ReqQueryTradeAccount(qry *common.MessageQuery) {
	req := common.MessageReqCommon{
		Message: "query_tradeaccount",
		Data:    *qry,
	}
	this.DoQuery(&req)
}
func (this *DemoTradeService) ReqQueryProduct(qry *common.MessageQuery) {
	req := common.MessageReqCommon{
		Message: "query_product",
		Data:    *qry,
	}
	this.DoQuery(&req)
}
