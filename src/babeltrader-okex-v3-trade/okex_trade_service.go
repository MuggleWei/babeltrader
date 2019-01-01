package main

import (
	"log"

	okex "github.com/MuggleWei/babel-trader/src/babeltrader-okex-v3"
	"github.com/MuggleWei/cascade"
)

type OkexTradeService struct {
	Api           *okex.TradeApi
	ClientService *ClientService
	config        *okex.Config
}

func NewOkexTradeService(config *OkexTradeConfig) *OkexTradeService {
	service := &OkexTradeService{
		Api:           okex.NewTradeApi(config.OkexConfig),
		ClientService: nil,
		config:        config.OkexConfig,
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
}

func (this *OkexTradeService) OnLogin(msg *okex.RspCommon) {
	log.Printf("[Info] OnLogin: %+v\n", *msg)
	// TODO:
}
func (this *OkexTradeService) OnError(msg *okex.RspCommon) {
	// TODO:
}

func (this *OkexTradeService) OnSub(msg *okex.RspCommon) {
	// TODO:
}
func (this *OkexTradeService) OnUnsub(msg *okex.RspCommon) {
	// TODO:
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
