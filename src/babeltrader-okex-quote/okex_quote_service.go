package main

import (
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
	log.Printf("okex quote connected: %v\n", peer.Conn.RemoteAddr().String())
	this.Api.Subscribe(this.config.QuoteSubTopics)

	this.channelMtx.Lock()
	defer this.channelMtx.Unlock()
	for _, channel := range this.config.QuoteSubTopics {
		this.channels[channel] = false
	}
}
func (this *OkexQuoteService) OnDisconnected(peer *cascade.Peer) {
	log.Printf("okex quote disconnected: %v\n", peer.Conn.RemoteAddr().String())

	this.channelMtx.Lock()
	defer this.channelMtx.Unlock()
	this.channels = make(map[string]bool)
}

func (this *OkexQuoteService) OnLogin(msg *okex.RspCommon) {
	log.Printf("OnLogin")
}
func (this *OkexQuoteService) OnError(msg *okex.RspCommon) {
	log.Printf("OnError: %+v\n", msg)
}
func (this *OkexQuoteService) OnSub(msg *okex.RspCommon) {
	this.channelMtx.Lock()
	defer this.channelMtx.Unlock()
	this.channels[msg.Channel] = true
}
func (this *OkexQuoteService) OnUnsub(msg *okex.RspCommon) {
	this.channelMtx.Lock()
	defer this.channelMtx.Unlock()
	delete(this.channels, msg.Unsubscribe)
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

func (this *OkexQuoteService) SubTopics([]common.MessageSubUnsub) {
	// TODO:
}

func (this *OkexQuoteService) UnsubTopics([]common.MessageSubUnsub) {
	// TODO:
}
