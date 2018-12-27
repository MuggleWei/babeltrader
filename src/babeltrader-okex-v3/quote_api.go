package babeltrader_okex_v3

import (
	"encoding/json"
	"errors"
	"log"
	"strings"
	"time"

	"github.com/MuggleWei/cascade"
)

type QuoteApi struct {
	hub          *cascade.Hub
	peer         *cascade.Peer
	config       *Config
	spi          QuoteSpi
	rspCallbacks map[string]OkexMsgCallback
}

func NewQuoteApi(config *Config) *QuoteApi {
	api := &QuoteApi{
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

func (this *QuoteApi) RegisterCallbacks() {
	this.rspCallbacks["login"] = this.OnLogin
	this.rspCallbacks["error"] = this.OnError
	this.rspCallbacks["subscribe"] = this.OnSub
	this.rspCallbacks["unsubscribe"] = this.OnUnsub
	this.rspCallbacks["candle60s"] = this.OnKline
	this.rspCallbacks["ticker"] = this.OnTicker
	this.rspCallbacks["depth5"] = this.OnDepth
	this.rspCallbacks["depth"] = this.OnDepthL2
}

func (this *QuoteApi) SetSpi(spi QuoteSpi) error {
	if this.spi != nil {
		return errors.New("repeated set spi")
	}

	this.spi = spi

	return nil
}

func (this *QuoteApi) Run() error {
	if this.spi == nil {
		err := errors.New("quote spi is null!!!")
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
func (this *QuoteApi) OnActive(peer *cascade.Peer) {
	this.peer = peer
	this.spi.OnConnected(peer)
}

func (this *QuoteApi) OnInactive(peer *cascade.Peer) {
	this.peer = nil
	this.spi.OnDisconnected(peer)
}

func (this *QuoteApi) OnRead(peer *cascade.Peer, message []byte) {
	if string(message) == "pong" {
		return
	}

	var rsp RspCommon
	err := json.Unmarshal(message, &rsp)
	if err != nil {
		log.Printf("[Warning] failed unmarshal message: %v\n", string(message))
		return
	}

	if rsp.Tabel != "" {
		idx := strings.Index(rsp.Tabel, "/")
		if idx > -1 && idx < len(rsp.Tabel)-1 {
			fn, ok := this.rspCallbacks[rsp.Tabel[idx+1:]]
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

func (this *QuoteApi) OnHubByteMessage(msg *cascade.HubByteMessage) {
	if this.peer != nil {
		this.peer.SendChannel <- msg.Message
	}
}

func (this *QuoteApi) OnHubObjectMessage(msg *cascade.HubObjectMessage) {
}

///////////////// heartbeat /////////////////
func (this *QuoteApi) runHeartbeat() {
	go func() {
		ticker := time.NewTicker(time.Second * 25)
		for _ = range ticker.C {
			this.Ping()
		}
	}()
}

func (this *QuoteApi) Ping() {
	ping := []byte("ping")
	this.hub.ByteMessageChannel <- &cascade.HubByteMessage{Peer: nil, Message: ping}
}

///////////////// requests /////////////////
func (this *QuoteApi) Subscribe(topics []string) error {
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

func (this *QuoteApi) Unsubscribe(topics []string) error {
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
func (this *QuoteApi) OnLogin(msg *RspCommon) {
	this.spi.OnLogin(msg)
}
func (this *QuoteApi) OnError(msg *RspCommon) {
	this.spi.OnError(msg)
}
func (this *QuoteApi) OnSub(msg *RspCommon) {
	this.spi.OnSub(msg)
}
func (this *QuoteApi) OnUnsub(msg *RspCommon) {
	this.spi.OnUnsub(msg)
}
func (this *QuoteApi) OnKline(msg *RspCommon) {
	this.spi.OnKline(msg)
}
func (this *QuoteApi) OnTicker(msg *RspCommon) {
	this.spi.OnTicker(msg)
}
func (this *QuoteApi) OnDepth(msg *RspCommon) {
	this.spi.OnDepth(msg)
}
func (this *QuoteApi) OnDepthL2(msg *RspCommon) {
	this.spi.OnDepthL2(msg)
}
