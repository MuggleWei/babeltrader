package main

import (
	"encoding/json"
	"log"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	"github.com/MuggleWei/cascade"
	"github.com/mitchellh/mapstructure"
)

type fnQuoteCallback func(*common.MessageQuote)

type QuoteService struct {
	Hub       *cascade.Hub
	Callbacks map[string]fnQuoteCallback
}

func NewQuoteService() *QuoteService {
	service := &QuoteService{
		Hub:       nil,
		Callbacks: make(map[string]fnQuoteCallback),
	}

	hub := cascade.NewHub(service, nil, 0)
	service.Hub = hub

	service.RegisterCallbacks()

	return service
}

func (this *QuoteService) Run() {
	addr := "127.0.0.1:6001"
	// addr := "127.0.0.1:6002"

	url := "ws://" + addr + "/ws"
	this.Hub.ConnectAndRun(url, true, 5, nil, func(addr string, err error) {
		log.Printf("[Error] failed dial to %v: %v", addr, err.Error())
	})
}

// Slot callbacks
func (this *QuoteService) OnActive(peer *cascade.Peer) {
	log.Printf("quote service connected: %v\n", peer.Conn.RemoteAddr().String())
}

func (this *QuoteService) OnInactive(peer *cascade.Peer) {
	log.Printf("quote service disconnected: %v\n", peer.Conn.RemoteAddr().String())
}

func (this *QuoteService) OnRead(peer *cascade.Peer, message []byte) {
	var msgs []common.MessageRspCommon
	err := json.Unmarshal(message, &msgs)
	if err != nil {
		log.Printf("failed unmarshal message: %v\n", string(message))
		return
	}

	for _, msg := range msgs {
		if msg.Message != "quote" {
			log.Printf("don't handle message: %v\n", msg.Message)
			return
		}

		var quote common.MessageQuote

		config := &mapstructure.DecoderConfig{TagName: "json", Result: &quote}
		decoder, err := mapstructure.NewDecoder(config)
		if err != nil {
			log.Println("failed new mapstructure decoder")
			return
		}

		err = decoder.Decode(msg.Data)
		if err != nil {
			log.Printf("failed unmarshal quote message: %v\n", msg.Data)
			return
		}

		fn, ok := this.Callbacks[quote.InfoPrimary]
		if !ok {
			log.Printf("failed find callback function for %v\n", quote.InfoPrimary)
			return
		}

		fn(&quote)
	}
}

func (this *QuoteService) OnHubByteMessage(msg *cascade.HubByteMessage) {
}

func (this *QuoteService) OnHubObjectMessage(*cascade.HubObjectMessage) {
}

//////////////////// method ////////////////////
func (this *QuoteService) RegisterCallbacks() {
	this.Callbacks["marketdata"] = this.OnMarketData
	this.Callbacks["kline"] = this.OnKline
	this.Callbacks["orderbook"] = this.OnOrderBook
	this.Callbacks["level2"] = this.OnLevel2
	this.Callbacks["depth"] = this.OnDepth
	this.Callbacks["ticker"] = this.OnTicker
}

func (this *QuoteService) OnMarketData(msg *common.MessageQuote) {
	log.Printf("marketdata: %+v\n", *msg)
}

func (this *QuoteService) OnKline(msg *common.MessageQuote) {
	log.Printf("kline: %+v\n", *msg)
}

func (this *QuoteService) OnOrderBook(msg *common.MessageQuote) {
	log.Printf("orderbook: %+v\n", *msg)
}

func (this *QuoteService) OnLevel2(msg *common.MessageQuote) {
	log.Printf("level2: %+v\n", *msg)
}

func (this *QuoteService) OnDepth(msg *common.MessageQuote) {
	log.Printf("depth: %+v\n", *msg)
}

func (this *QuoteService) OnTicker(msg *common.MessageQuote) {
	log.Printf("ticker: %+v\n", *msg)
}
