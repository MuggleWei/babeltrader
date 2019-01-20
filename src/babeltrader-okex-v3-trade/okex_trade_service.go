package main

import (
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

func (this *OkexTradeService) QueryOrder(peer *cascade.Peer, qry *common.MessageQuery) (*common.MessageRspCommon, error) {
	okexQry, err := okex.ConvertQryCommon2Okex(qry)
	if err != nil {
		log.Printf("[Error] failed convert query common to okex: %v, %v\n", *qry, err.Error())
		return nil, err
	}

	var okexOrderTrade okex.OrderTrade
	_, err = this.Api.QueryOrder(okexQry, &okexOrderTrade)
	if err != nil {
		log.Printf("[Error] failed query order: %v, %v\n", okexQry, err.Error())
		return nil, err
	}

	rspOrderTrade, err := okex.ConvertOrderTradeOkex2Common(okexQry.ProductType, &okexOrderTrade)
	if err != nil {
		log.Printf("[Error] failed convert query return order trade okex to common: %v, %v\n", okexOrderTrade, err.Error())
		return nil, err
	}

	orderTrade, ok := rspOrderTrade.Data.(common.MessageOrderStatus)
	if !ok {
		s := fmt.Sprintf("failed get order trade from common rsp message: %v\n", rspOrderTrade)
		log.Printf("[Error] %v\n", s)
		return nil, errors.New(s)
	}

	status := []common.MessageOrderStatus{
		common.MessageOrderStatus{
			Status:       orderTrade.Status,
			SubmitStatus: common.OrderStatus_Unknown,
			Amount:       orderTrade.Order.Amount,
			DealedAmount: orderTrade.DealedAmount,
			AvgPrice:     orderTrade.AvgPrice,
			Timestamp:    orderTrade.Timestamp,
			Order:        orderTrade.Order,
		},
	}
	qry.Data = status

	return &common.MessageRspCommon{
		Message: "rsp_qryorder",
		Data:    *qry,
	}, nil
}

func (this *OkexTradeService) QueryPosition(peer *cascade.Peer, qry *common.MessageQuery) (*common.MessageRspCommon, error) {
	okexQry, err := okex.ConvertQryCommon2Okex(qry)
	if err != nil {
		log.Printf("[Error] failed convert query common to okex: %v, %v\n", *qry, err.Error())
		return nil, err
	}

	var position okex.Position
	_, err = this.Api.QueryPosition(okexQry, &position)
	if err != nil {
		log.Printf("[Error] failed query position: %v, %v\n", okexQry, err.Error())
		return nil, err
	}

	qry.PositionSummaryType = "type3"
	qry.Data = position.Holding

	return &common.MessageRspCommon{
		Message: "rsp_qryposition",
		Data:    *qry,
	}, nil
}

func (this *OkexTradeService) QueryAccount(peer *cascade.Peer, qry *common.MessageQuery) (*common.MessageRspCommon, error) {
	okexQry, err := okex.ConvertQryCommon2Okex(qry)
	if err != nil {
		log.Printf("[Error] failed convert query common to okex: %v, %v\n", *qry, err.Error())
		return nil, err
	}

	if okexQry.CurrencyId == "" {
		s := fmt.Sprintf("okex only support query account with single currency")
		log.Printf("[Error] %v\n", s)
		return nil, errors.New(s)
	}

	var account okex.TradeAccount
	_, err = this.Api.QueryAccount(okexQry, &account)
	if err != nil {
		log.Printf("[Error] failed query account: %v, %v\n", okexQry, err.Error())
		return nil, err
	}

	qry.TradeAccountType = "type3"
	qry.Data = account

	return &common.MessageRspCommon{
		Message: "rsp_qrytradeaccount",
		Data:    *qry,
	}, nil
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
	idx := strings.Index(msg.Table, "/")
	productType := msg.Table[:idx]

	var okexTrades []okex.OrderTrade
	err := utils.DecodeInterfaceByJson(msg.Data, &okexTrades)
	if err != nil {
		log.Printf("[Error] failed decode data: %v\n", *msg)
		return
	}

	for _, okexTrade := range okexTrades {
		rsp, err := okex.ConvertOrderTradeOkex2Common(productType, &okexTrade)
		if err != nil {
			log.Printf("[Error] failed convert order trade okex to common: %v\n", okexTrade)
			continue
		}

		this.ClientService.Hub.ObjectMessageChannel <- &cascade.HubObjectMessage{
			Peer:       nil,
			ObjectName: "orderstatus",
			ObjectPtr:  rsp,
		}
	}
}
func (this *OkexTradeService) OnPosition(msg *okex.RspCommon) {
	// TODO:
}
func (this *OkexTradeService) OnAccount(msg *okex.RspCommon) {
	// TODO:
}
