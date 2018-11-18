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

	this.Hub.ConnectAndRun(url, true, 5, nil)
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
	this.Callbacks["confirmorder"] = this.OnConfirmOrder
	this.Callbacks["orderstatus"] = this.OnOrderStatus
	this.Callbacks["orderdeal"] = this.OnOrderDeal
}

func (this *DemoTradeService) OnConfirmOrder(data interface{}) {
	log.Printf("confirmorder: %+v\n", data)
}
func (this *DemoTradeService) OnOrderStatus(data interface{}) {
	log.Printf("orderstatus: %+v\n", data)
}
func (this *DemoTradeService) OnOrderDeal(data interface{}) {
	log.Printf("orderdeal: %+v\n", data)
}

func (this *DemoTradeService) ReqInsertOrder(order *common.MessageOrder) {
	req := common.MessageReqCommon{
		Message: "insert_order",
		Data:    *order,
	}

	msg, err := json.Marshal(req)
	if err != nil {
		log.Printf("failed to marshal message: %v\n", req)
		return
	}

	this.Hub.ByteMessageChannel <- &cascade.HubByteMessage{Peer: nil, Message: msg}
}
