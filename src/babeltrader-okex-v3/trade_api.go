package babeltrader_okex_v3

import (
	"encoding/json"
	"errors"
	"log"
	"strconv"
	"strings"
	"time"

	"github.com/MuggleWei/cascade"
)

type TradeApi struct {
	hub          *cascade.Hub
	peer         *cascade.Peer
	config       *Config
	spi          TradeSpi
	rspCallbacks map[string]OkexMsgCallback
}

func NewTradeApi(config *Config) *TradeApi {
	api := &TradeApi{
		hub:          nil,
		peer:         nil,
		config:       config,
		spi:          nil,
		rspCallbacks: make(map[string]OkexMsgCallback),
	}

	api.hub = cascade.NewHub(api, nil, 0)
	api.RegisterCallbacks()

	return api
}

func (this *TradeApi) RegisterCallbacks() {
	this.rspCallbacks["login"] = this.OnLogin
	this.rspCallbacks["error"] = this.OnError
	this.rspCallbacks["subscribe"] = this.OnSub
	this.rspCallbacks["unsubscribe"] = this.OnUnsub
	this.rspCallbacks["account"] = this.OnAccount
	this.rspCallbacks["position"] = this.OnPosition
	this.rspCallbacks["order"] = this.OnOrder
}

func (this *TradeApi) SetSpi(spi TradeSpi) error {
	if this.spi != nil {
		return errors.New("repeated set spi")
	}

	this.spi = spi

	return nil
}

func (this *TradeApi) Run() error {
	if this.spi == nil {
		err := errors.New("trade spi is null!!!")
		return err
	}

	this.runHeartbeat()

	go func() {
		url := this.config.WsEndpoint
		this.hub.ConnectAndRun(url, true, 15, nil, func(addr string, err error) {
			log.Printf("[Error] failed dial to %v: %v", addr, err.Error())
		})
	}()

	return nil
}

///////////////// Slot callbacks /////////////////
func (this *TradeApi) OnActive(peer *cascade.Peer) {
	this.peer = peer
	this.spi.OnConnected(peer)
}

func (this *TradeApi) OnInactive(peer *cascade.Peer) {
	this.peer = nil
	this.spi.OnDisconnected(peer)
}

func (this *TradeApi) OnRead(peer *cascade.Peer, message []byte) {
	if string(message) == "pong" {
		return
	}

	var rsp RspCommon
	err := json.Unmarshal(message, &rsp)
	if err != nil {
		log.Printf("[Warning] failed unmarshal message: %v, %v\n", string(message), err.Error())
		return
	}

	if rsp.Table != "" {
		idx := strings.Index(rsp.Table, "/")
		if idx > -1 && idx < len(rsp.Table)-1 {
			fn, ok := this.rspCallbacks[rsp.Table[idx+1:]]
			if ok {
				fn(&rsp)
			} else {
				log.Printf("[Warning] failed find callback function for message: %v\n", string(message))
			}
		} else {
			log.Printf("[Warning] not recognized message: %v\n", string(message))
		}
	} else if rsp.Event != "" {
		fn, ok := this.rspCallbacks[rsp.Event]
		if ok {
			fn(&rsp)
		} else {
			log.Printf("[Warning] failed find callback function for message: %v\n", string(message))
		}
	} else {
		log.Printf("[Warning] not recognized message: %v\n", string(message))
	}
}

func (this *TradeApi) OnHubByteMessage(msg *cascade.HubByteMessage) {
	if this.peer != nil {
		this.peer.SendChannel <- msg.Message
	}
}

func (this *TradeApi) OnHubObjectMessage(msg *cascade.HubObjectMessage) {
}

///////////////// heartbeat /////////////////
func (this *TradeApi) runHeartbeat() {
	go func() {
		ticker := time.NewTicker(time.Second * 25)
		for _ = range ticker.C {
			this.Ping()
		}
	}()
}

func (this *TradeApi) Ping() {
	ping := []byte("ping")
	this.hub.ByteMessageChannel <- &cascade.HubByteMessage{Peer: nil, Message: ping}
}

///////////////// requests /////////////////
func (this *TradeApi) Login() error {
	ts := time.Now().Unix()
	timestamp := strconv.FormatInt(ts, 10)
	sign, err := GenWsSign(timestamp, "GET", "/users/self/verify", this.config.Secret)
	if err != nil {
		log.Printf("[Warning] failed generate sign: %v\n", err.Error())
		return err
	}

	req := ReqCommon{
		Op:   "login",
		Args: []string{this.config.Key, this.config.Passphrase, timestamp, sign},
	}

	b, err := json.Marshal(req)
	if err != nil {
		return err
	}

	this.hub.ByteMessageChannel <- &cascade.HubByteMessage{Peer: nil, Message: b}
	return nil
}

func (this *TradeApi) Subscribe(topics []string) error {
	req := ReqCommon{
		Op:   "subscribe",
		Args: topics,
	}

	b, err := json.Marshal(req)
	if err != nil {
		return err
	}

	this.hub.ByteMessageChannel <- &cascade.HubByteMessage{Peer: nil, Message: b}
	return nil
}

func (this *TradeApi) Unsubscribe(topics []string) error {
	req := ReqCommon{
		Op:   "unsubscribe",
		Args: topics,
	}

	b, err := json.Marshal(req)
	if err != nil {
		return err
	}

	this.hub.ByteMessageChannel <- &cascade.HubByteMessage{Peer: nil, Message: b}
	return nil
}

///////////////// response and quotes /////////////////
func (this *TradeApi) OnLogin(msg *RspCommon) {
	this.spi.OnLogin(msg)
}
func (this *TradeApi) OnError(msg *RspCommon) {
	this.spi.OnError(msg)
}
func (this *TradeApi) OnSub(msg *RspCommon) {
	this.spi.OnSub(msg)
}
func (this *TradeApi) OnUnsub(msg *RspCommon) {
	this.spi.OnUnsub(msg)
}
func (this *TradeApi) OnAccount(msg *RspCommon) {
	this.spi.OnAccount(msg)
}
func (this *TradeApi) OnPosition(msg *RspCommon) {
	this.spi.OnPosition(msg)
}
func (this *TradeApi) OnOrder(msg *RspCommon) {
	this.spi.OnOrder(msg)
}
