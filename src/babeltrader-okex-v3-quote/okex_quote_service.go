package main

import (
	"errors"
	"fmt"
	"log"
	"sync"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	okex "github.com/MuggleWei/babel-trader/src/babeltrader-okex-v3"
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
	// TODO:
}
func (this *OkexQuoteService) OnTicker(msg *okex.RspCommon) {
	// TODO:
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
