package main

import (
	"log"

	"github.com/MuggleWei/cascade"
	"github.com/gorilla/websocket"
)

type ClientService struct {
	Hub          *cascade.Hub
	TradeService *OkexTradeService
}

func NewClientService() *ClientService {
	service := &ClientService{
		Hub:          nil,
		TradeService: nil,
	}

	upgrader := websocket.Upgrader{
		ReadBufferSize:  1024 * 20,
		WriteBufferSize: 1024 * 20,
	}

	service.Hub = cascade.NewHub(service, &upgrader, 10240)

	return service
}

func (this *ClientService) Run() {
	go this.Hub.Run()
}

///////////////// Slot callbacks /////////////////
func (this *ClientService) OnActive(peer *cascade.Peer) {
	log.Printf("peer active: %v\n", peer.Conn.RemoteAddr().String())
}

func (this *ClientService) OnInactive(peer *cascade.Peer) {
	log.Printf("peer inactive: %v\n", peer.Conn.RemoteAddr().String())
}

func (this *ClientService) OnRead(peer *cascade.Peer, message []byte) {
}

func (this *ClientService) OnHubByteMessage(msg *cascade.HubByteMessage) {
	for peer := range this.Hub.Peers {
		peer.SendChannel <- msg.Message
	}
}

func (this *ClientService) OnHubObjectMessage(msg *cascade.HubObjectMessage) {
}
