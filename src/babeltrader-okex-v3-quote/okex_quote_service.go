package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"log"
	"strings"
	"sync"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	okex "github.com/MuggleWei/babel-trader/src/babeltrader-okex-v3"
	utils "github.com/MuggleWei/babel-trader/src/babeltrader-utils-go"
	"github.com/MuggleWei/cascade"
)

type OkexQuoteService struct {
	Api           *okex.QuoteApi
	ClientService *ClientService
	config        *okex.Config
	channels      map[string]bool
	channelMtx    sync.RWMutex
}

func NewOkexQuoteService(config *OkexQuoteConfig) *OkexQuoteService {
	service := &OkexQuoteService{
		Api:           okex.NewQuoteApi(config.OkexConfig),
		ClientService: nil,
		config:        config.OkexConfig,
		channels:      make(map[string]bool),
	}

	service.Api.SetSpi(service)

	return service
}

func (this *OkexQuoteService) Run() {
	err := this.Api.Run()
	if err != nil {
		panic(err)
	}
}

///////////////// quote spi /////////////////
func (this *OkexQuoteService) OnConnected(peer *cascade.Peer) {
	log.Printf("[Info] okex quote connected: %v\n", peer.Conn.RemoteAddr().String())
	this.Api.Subscribe(this.config.QuoteSubTopics)

	this.channelMtx.Lock()
	defer this.channelMtx.Unlock()
	for _, channel := range this.config.QuoteSubTopics {
		this.channels[channel] = false
	}
}
func (this *OkexQuoteService) OnDisconnected(peer *cascade.Peer) {
	log.Printf("[Warning] okex quote disconnected: %v\n", peer.Conn.RemoteAddr().String())

	this.channelMtx.Lock()
	defer this.channelMtx.Unlock()
	this.channels = make(map[string]bool)
}

func (this *OkexQuoteService) OnLogin(msg *okex.RspCommon) {
	log.Printf("[Info] OnLogin")
}
func (this *OkexQuoteService) OnError(msg *okex.RspCommon) {
	log.Printf("[Info] OnError: %+v\n", msg)
}
func (this *OkexQuoteService) OnSub(msg *okex.RspCommon) {
	log.Printf("[Info] success sub channel: %v\n", msg.Channel)

	this.channelMtx.Lock()
	defer this.channelMtx.Unlock()
	this.channels[msg.Channel] = true
}
func (this *OkexQuoteService) OnUnsub(msg *okex.RspCommon) {
	log.Printf("[Info] success unsub channel: %v\n", msg.Channel)

	this.channelMtx.Lock()
	defer this.channelMtx.Unlock()
	delete(this.channels, msg.Channel)
}

func (this *OkexQuoteService) OnKline(msg *okex.RspCommon) {
	// only support 1m kline
	if !strings.HasSuffix(msg.Tabel, "60s") {
		return
	}

	var candles []okex.Candle
	err := utils.DecodeInterfaceByJson(msg.Data, &candles)
	if err != nil {
		log.Printf("[Warning] failed unmarshal candles %v\n", *msg)
		return
	}

	quotes, err := okex.ConvertCandleToQuotes(msg.Tabel, candles)
	if err != nil {
		log.Printf("[Warning] failed convert candles to quotes %v\n", *msg)
		return
	}

	b, err := json.Marshal(quotes)
	if err != nil {
		log.Printf("[Warning] failed marshal quotes %v\n", quotes)
		return
	}

	this.ClientService.Hub.ByteMessageChannel <- &cascade.HubByteMessage{Peer: nil, Message: b}
}
func (this *OkexQuoteService) OnTicker(msg *okex.RspCommon) {
	var tickers []okex.Ticker

	// NOTE: fucking okex v3 api, spot/swap ticker's price/vol fields use string, futures's price/vol fields use double?!!! :(
	if strings.HasPrefix(msg.Tabel, "futures") {
		var futuresTickers []okex.FuturesTicker
		err := utils.DecodeInterfaceByJson(msg.Data, &futuresTickers)
		if err != nil {
			log.Printf("[Warning] failed unmarshal tickers: %v\n", *msg)
			return
		}

		for _, futureTicker := range futuresTickers {
			tickers = append(tickers, okex.Ticker{
				InstrumentId: futureTicker.InstrumentId,
				Last:         fmt.Sprintf("%v", futureTicker.Last),
				BestBid:      fmt.Sprintf("%v", futureTicker.BestBid),
				BestAsk:      fmt.Sprintf("%v", futureTicker.BestAsk),
				Open24H:      fmt.Sprintf("%v", futureTicker.Open24H),
				High24H:      fmt.Sprintf("%v", futureTicker.High24H),
				Low24H:       fmt.Sprintf("%v", futureTicker.Low24H),
				Vol24H:       fmt.Sprintf("%v", futureTicker.Vol24H),
				Timestamp:    futureTicker.Timestamp,
			})
		}
	} else {
		err := utils.DecodeInterfaceByJson(msg.Data, &tickers)
		if err != nil {
			log.Printf("[Warning] failed unmarshal tickers: %v\n", *msg)
			return
		}
	}

	quotes, err := okex.ConvertTickerToQuotes(msg.Tabel, tickers)
	if err != nil {
		log.Printf("[Warning] failed convert tickers to quotes %v\n", *msg)
		return
	}

	b, err := json.Marshal(quotes)
	if err != nil {
		log.Printf("[Warning] failed marshal quotes %v\n", quotes)
		return
	}

	this.ClientService.Hub.ByteMessageChannel <- &cascade.HubByteMessage{Peer: nil, Message: b}
}
func (this *OkexQuoteService) OnDepth(msg *okex.RspCommon) {
	// TODO:
}
func (this *OkexQuoteService) OnDepthL2(msg *okex.RspCommon) {
	// TODO:
}

///////////////// rest /////////////////
func (this *OkexQuoteService) GetSubTopics() []common.MessageSubUnsub {
	this.channelMtx.RLock()
	defer this.channelMtx.RUnlock()

	var topics []common.MessageSubUnsub
	for channel, subed := range this.channels {
		msg, err := okex.ConvertChannelToSubUnsub(channel)
		if err != nil {
			log.Printf("[Error] failed convert channel %v to MessageSubUnsub: %v\n", channel, err.Error())
		}

		if subed {
			msg.Subed = 1
		} else {
			msg.Subed = 0
		}
		topics = append(topics, *msg)
	}

	return topics
}

func (this *OkexQuoteService) SubTopics(msgs []common.MessageQuote) error {
	var channels []string
	for _, msg := range msgs {
		channel, err := okex.ConvertQuoteToChannel(&msg)
		if err != nil {
			return err
		}
		channels = append(channels, channel)
	}

	this.channelMtx.Lock()
	defer this.channelMtx.Unlock()

	for _, channel := range channels {
		subed, ok := this.channels[channel]
		if ok && subed {
			s := fmt.Sprintf("not allowed to sub channel %v repeated", channel)
			return errors.New(s)
		}
	}

	for _, channel := range channels {
		this.channels[channel] = false
	}

	log.Printf("[Info] req sub channels: %v\n", channels)
	return this.Api.Subscribe(channels)
}

func (this *OkexQuoteService) UnsubTopics(msgs []common.MessageQuote) error {
	var channels []string
	for _, msg := range msgs {
		channel, err := okex.ConvertQuoteToChannel(&msg)
		if err != nil {
			return err
		}
		channels = append(channels, channel)
	}

	this.channelMtx.Lock()
	defer this.channelMtx.Unlock()

	for _, channel := range channels {
		_, ok := this.channels[channel]
		if !ok {
			s := fmt.Sprintf("not allowed unsub channel %v, cause not subed", channel)
			return errors.New(s)
		}
	}

	log.Printf("[Info] req unsub channels: %v\n", channels)
	return this.Api.Unsubscribe(channels)
}
