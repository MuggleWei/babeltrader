package main

import (
	"log"
	"sync"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	okex "github.com/MuggleWei/babel-trader/src/babeltrader-okex-v3"
	"github.com/MuggleWei/cascade"
)

type OkexTradeService struct {
	Api           *okex.TradeApi
	ClientService *ClientService
	config        *okex.Config
	channels      map[string]bool
	channelMtx    sync.RWMutex
}

func NewOkexTradeService(config *OkexTradeConfig) *OkexTradeService {
	service := &OkexTradeService{
		Api:           okex.NewTradeApi(config.OkexConfig),
		ClientService: nil,
		config:        config.OkexConfig,
		channels:      make(map[string]bool),
	}

	service.Api.SetSpi(service)

	return service
}

func (this *OkexTradeService) Run() {
	err := this.Api.Run()
	if err != nil {
		panic(err)
	}
}

///////////////// req /////////////////
func (this *OkexTradeService) InsertOrder(peer *cascade.Peer, order *common.MessageOrder) (string, error) {
	productType, okexOrder, err := okex.ConvertInsertOrderCommon2Okex(order)
	if err != nil {
		return "", err
	}

	var ret okex.OrderRet
	_, err = this.Api.InsertOrder(okexOrder, productType, &ret)
	if err != nil {
		return "", err
	}

	return ret.OrderId, nil
}

func (this *OkexTradeService) CancelOrder(peer *cascade.Peer, order *common.MessageOrder) (string, error) {
	productType, okexOrder, err := okex.ConvertCancelOrderCommon2Okex(order)
	if err != nil {
		return "", err
	}

	var ret okex.OrderRet
	_, err = this.Api.CancelOrder(okexOrder, productType, order.OutsideId, &ret)
	if err != nil {
		return "", err
	}

	return ret.OrderId, nil
}

///////////////// trade spi /////////////////
func (this *OkexTradeService) OnConnected(peer *cascade.Peer) {
	log.Printf("[Info] okex trade connected: %v\n", peer.Conn.RemoteAddr().String())

	err := this.Api.Login()
	if err != nil {
		log.Printf("[Error] failed invoke trade api login: %v\n", err.Error())
		panic(err)
	}
}
func (this *OkexTradeService) OnDisconnected(peer *cascade.Peer) {
	log.Printf("[Warning] okex trade disconnected: %v\n", peer.Conn.RemoteAddr().String())

	this.channelMtx.Lock()
	defer this.channelMtx.Unlock()
	for channel, _ := range this.channels {
		this.channels[channel] = false
	}
}

func (this *OkexTradeService) OnLogin(msg *okex.RspCommon) {
	log.Printf("[Info] OnLogin: %+v\n", *msg)

	this.channelMtx.Lock()
	defer this.channelMtx.Unlock()
	if len(this.channels) == 0 {
		for _, channel := range this.config.TradeSubTopics {
			this.channels[channel] = false
		}
		this.Api.Subscribe(this.config.TradeSubTopics)
	} else {
		channels := []string{}
		for channel, _ := range this.channels {
			channels = append(channels, channel)
		}
		this.Api.Subscribe(channels)
	}
}
func (this *OkexTradeService) OnError(msg *okex.RspCommon) {
	log.Printf("[Warning] OnError: %+v\n", msg)
}

func (this *OkexTradeService) OnSub(msg *okex.RspCommon) {
	log.Printf("[Info] success sub channel: %v\n", msg.Channel)

	this.channelMtx.Lock()
	defer this.channelMtx.Unlock()
	this.channels[msg.Channel] = true
}
func (this *OkexTradeService) OnUnsub(msg *okex.RspCommon) {
	log.Printf("[Info] success unsub channel: %v\n", msg.Channel)

	this.channelMtx.Lock()
	defer this.channelMtx.Unlock()
	delete(this.channels, msg.Channel)
}

func (this *OkexTradeService) OnOrder(msg *okex.RspCommon) {
}
func (this *OkexTradeService) OnPosition(msg *okex.RspCommon) {
	// TODO:
}
func (this *OkexTradeService) OnAccount(msg *okex.RspCommon) {
	// TODO:
}
