package babeltrader_utils_go

import (
	"bytes"
	"errors"
	"fmt"
	"log"
	"time"

	cascade "github.com/MuggleWei/cascade"
)

type innerPeer struct {
	peer *cascade.Peer
	ts   int64
}

type PeerMsgLinker struct {
	msgPeer  map[string]innerPeer
	peerMsgs map[*cascade.Peer]map[string]bool
	// mtx      sync.Mutex
}

func NewPeerMsgLinker() *PeerMsgLinker {
	return &PeerMsgLinker{
		msgPeer:  make(map[string]innerPeer),
		peerMsgs: make(map[*cascade.Peer]map[string]bool),
	}
}

func (this *PeerMsgLinker) AddPeer(peer *cascade.Peer) error {
	//	this.mtx.Lock()
	//	defer this.mtx.Unlock()

	_, ok := this.peerMsgs[peer]
	if ok {
		return errors.New("repeated add peer")
	}

	this.peerMsgs[peer] = make(map[string]bool)

	return nil
}

func (this *PeerMsgLinker) DelPeer(peer *cascade.Peer) error {
	//	this.mtx.Lock()
	//	defer this.mtx.Unlock()

	msgs, ok := this.peerMsgs[peer]
	if !ok {
		return errors.New("delete peer not exists")
	}

	for k, _ := range msgs {
		delete(this.msgPeer, k)
	}
	delete(this.peerMsgs, peer)

	return nil
}

func (this *PeerMsgLinker) CacheMsg(peer *cascade.Peer, msg string) error {
	//	this.mtx.Lock()
	//	defer this.mtx.Unlock()

	_, ok := this.msgPeer[msg]
	if ok {
		return errors.New("repeated id message")
	}

	msgs, ok := this.peerMsgs[peer]
	if !ok {
		return errors.New("peer not exists")
	}

	msgs[msg] = true
	this.msgPeer[msg] = innerPeer{
		peer: peer,
		ts:   time.Now().Unix(),
	}

	return nil
}

func (this *PeerMsgLinker) GetMsgPeer(msg string) (*cascade.Peer, error) {
	//	this.mtx.Lock()
	//	defer this.mtx.Unlock()

	ipeer, ok := this.msgPeer[msg]
	if !ok {
		return nil, errors.New("failed find peer")
	}

	msgs, ok := this.peerMsgs[ipeer.peer]
	if ok {
		delete(msgs, msg)
	}
	delete(this.msgPeer, msg)

	return ipeer.peer, nil
}

func (this *PeerMsgLinker) CleanExpire(expireSecond int64) {
	ts := time.Now().Unix()

	//	this.mtx.Lock()
	//	defer this.mtx.Unlock()

	var waitDelMsgs []string
	for k, v := range this.msgPeer {
		if ts-v.ts > expireSecond {
			waitDelMsgs = append(waitDelMsgs, k)
			msgs, ok := this.peerMsgs[v.peer]
			if ok {
				delete(msgs, k)
			}
		}
	}

	for _, msg := range waitDelMsgs {
		delete(this.msgPeer, msg)
	}
}

func (this *PeerMsgLinker) OutputMsgPeer() {
	var buffer bytes.Buffer
	buffer.WriteString("current msg peer: \n")
	for k, v := range this.msgPeer {
		s := fmt.Sprintf("    %v: %v\n", k, v.peer.Conn.RemoteAddr().String())
		buffer.WriteString(s)
	}
	log.Printf("%v", buffer.String())
}
