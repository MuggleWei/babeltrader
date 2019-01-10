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
	if order.ProductType == "future" {
		dir := "0"
		if order.Dir == "open_long" {
			dir = "1"
		} else if order.Dir == "open_short" {
			dir = "2"
		} else if order.Dir == "close_long" {
			dir = "3"
		} else if order.Dir == "close_short" {
			dir = "4"
		} else {
			s := fmt.Sprintf("invalid order dir: %v", order.Dir)
			return "", errors.New(s)
		}
		price := fmt.Sprintf("%v", order.Price)
		size := fmt.Sprintf("%v", order.Amount)
		leverage := "20"
		productType := "futures"
		if order.Contract == "SWAP" {
			leverage = ""
			productType = "swap"
			return "", errors.New("not support okex future swap")
		}

		okexOrder := okex.Order{
			InstrumentId: order.Symbol + "-" + order.Contract,
			Type:         dir,
			Price:        price,
			Size:         size,
			Leverage:     leverage,
		}

		var ret okex.OrderRet
		_, err := this.Api.InsertOrder(&okexOrder, productType, &ret)
		if err != nil {
			return "", err
		}

		return ret.OrderId, nil
	} else if order.ProductType == "spot" {
		okexOrder := okex.Order{
			InstrumentId: order.Symbol,
			Type:         order.OrderType,
			Side:         order.Dir,
		}
		if order.OrderType == "limit" {
			okexOrder.Price = fmt.Sprintf("%v", order.Price)
			okexOrder.Size = fmt.Sprintf("%v", order.Amount)
		} else if order.OrderType == "market" {
			if order.Dir == "buy" {
				okexOrder.Notional = fmt.Sprintf("%v", order.TotalPrice)
			} else {
				okexOrder.Size = fmt.Sprintf("%v", order.Amount)
			}
		}
		var ret okex.OrderRet
		_, err := this.Api.InsertOrder(&okexOrder, "spot", &ret)
		if err != nil {
			return "", err
		}

		return ret.OrderId, nil
	} else {
		s := fmt.Sprintf("not support product type: %v", order.ProductType)
		return "", errors.New(s)
	}
}

func (this *OkexTradeService) CancelOrder(peer *cascade.Peer, order *common.MessageOrder) (string, error) {
	if order.ProductType == "future" {
		okexOrder := okex.Order{
			InstrumentId: order.Symbol + "-" + order.Contract,
		}

		productType := "futures"
		if order.Contract == "SWAP" {
			productType = "swap"
		}

		var ret okex.OrderRet
		_, err := this.Api.CancelOrder(&okexOrder, productType, order.OutsideId, &ret)
		if err != nil {
			return "", err
		}

		return ret.OrderId, nil
	} else if order.ProductType == "spot" {
		okexOrder := okex.Order{
			InstrumentId: order.Symbol,
		}

		var ret okex.OrderRet
		_, err := this.Api.CancelOrder(&okexOrder, "spot", order.OutsideId, &ret)
		if err != nil {
			return "", err
		}

		return ret.OrderId, nil
	} else {
		s := fmt.Sprintf("not support product type: %v", order.ProductType)
		return "", errors.New(s)
	}
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
	// TODO:
}
func (this *OkexTradeService) OnPosition(msg *okex.RspCommon) {
	// TODO:
}
func (this *OkexTradeService) OnAccount(msg *okex.RspCommon) {
	// TODO:
}
