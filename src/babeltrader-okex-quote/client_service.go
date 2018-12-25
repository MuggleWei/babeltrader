package main

import (
	"log"
	"net/http"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	utils "github.com/MuggleWei/babel-trader/src/babeltrader-utils-go"
	"github.com/MuggleWei/cascade"
	"github.com/gorilla/websocket"
)

type ClientService struct {
	Hub           *cascade.Hub
	QuoteService  *OkexQuoteService
	PeerMsgLinker *utils.PeerMsgLinker
}

func NewClientService() *ClientService {
	service := &ClientService{
		Hub:           nil,
		QuoteService:  nil,
		PeerMsgLinker: utils.NewPeerMsgLinker(),
	}

	upgrader := websocket.Upgrader{
		ReadBufferSize:  1024 * 20,
		WriteBufferSize: 1024 * 20,
	}

	service.Hub = cascade.NewHub(service, &upgrader, 10240)
	go service.Hub.Run()

	return service
}

func (this *ClientService) Run() {
	go this.Hub.Run()
}

///////////////// Slot callbacks /////////////////
func (this *ClientService) OnActive(peer *cascade.Peer) {
	log.Printf("peer active: %v\n", peer.Conn.RemoteAddr().String())
	this.PeerMsgLinker.AddPeer(peer)
}

func (this *ClientService) OnInactive(peer *cascade.Peer) {
	log.Printf("peer inactive: %v\n", peer.Conn.RemoteAddr().String())
	this.PeerMsgLinker.DelPeer(peer)
}

func (this *ClientService) OnRead(peer *cascade.Peer, message []byte) {
	// TODO:
	// utils.DispatchRequests(peer, message, this.ReqCallbacks)
}

func (this *ClientService) OnHubByteMessage(msg *cascade.HubByteMessage) {
	// TODO:
	// utils.DispatchResponse(msg, this.RspCallbacks)
}

func (this *ClientService) OnHubObjectMessage(msg *cascade.HubObjectMessage) {
}

///////////////// Slot callbacks /////////////////
func (this *ClientService) TopicsGet(w http.ResponseWriter, r *http.Request) {
	utils.HttpResponse(w, "topic", 0, "", this.QuoteService.GetSubTopics())
}
func (this *ClientService) TopicsSub(w http.ResponseWriter, r *http.Request) {
	var req []common.MessageQuote
	err := utils.ReadPostBody(r, &req)
	if err != nil {
		utils.HttpResponse(w, "", -1, err.Error(), nil)
		return
	}

	this.QuoteService.SubTopics(req)
	utils.HttpResponse(w, "", 0, "", nil)
}
func (this *ClientService) TopicsUnsub(w http.ResponseWriter, r *http.Request) {
	var req []common.MessageQuote
	err := utils.ReadPostBody(r, &req)
	if err != nil {
		utils.HttpResponse(w, "", -1, err.Error(), nil)
		return
	}

	this.QuoteService.UnsubTopics(req)
	utils.HttpResponse(w, "", 0, "", nil)
}
