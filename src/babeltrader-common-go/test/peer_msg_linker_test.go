package babeltrader_common_go_test

import (
	"testing"
	"time"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	"github.com/MuggleWei/cascade"
)

func TestAddDelPeer(t *testing.T) {
	linker := common.NewPeerMsgLinker()

	err := linker.AddPeer(nil)
	if err != nil {
		t.Error(err.Error())
	}

	err = linker.DelPeer(nil)
	if err != nil {
		t.Error(err.Error())
	}
}

func TestRepeatedAddPeer(t *testing.T) {
	linker := common.NewPeerMsgLinker()

	err := linker.AddPeer(nil)
	if err != nil {
		t.Error(err.Error())
	}

	err = linker.AddPeer(nil)
	if err == nil {
		t.Error("failed repeated add")
	}
}

func TestRepeatedDelPeer(t *testing.T) {
	linker := common.NewPeerMsgLinker()

	err := linker.AddPeer(nil)
	if err != nil {
		t.Error(err.Error())
	}

	err = linker.DelPeer(nil)
	if err != nil {
		t.Error(err.Error())
	}

	err = linker.DelPeer(nil)
	if err == nil {
		t.Error("failed repeated del")
	}
}

func TestCacheGet(t *testing.T) {
	linker := common.NewPeerMsgLinker()

	var peer cascade.Peer

	err := linker.AddPeer(&peer)
	if err != nil {
		t.Error(err.Error())
	}

	linker.CacheMsg(&peer, "a")
	linker.CacheMsg(&peer, "b")
	linker.CacheMsg(&peer, "c")

	p, err := linker.GetMsgPeer("a")
	if err != nil || p != &peer {
		t.Error("failed get msg peer")
	}

	p, err = linker.GetMsgPeer("b")
	if err != nil || p != &peer {
		t.Error("failed get msg peer")
	}

	p, err = linker.GetMsgPeer("c")
	if err != nil || p != &peer {
		t.Error("failed get msg peer")
	}

	err = linker.DelPeer(&peer)
	if err != nil {
		t.Error(err.Error())
	}
}

func TestRepeatedCache(t *testing.T) {
	linker := common.NewPeerMsgLinker()

	var peer cascade.Peer

	err := linker.AddPeer(&peer)
	if err != nil {
		t.Error(err.Error())
	}

	err = linker.CacheMsg(&peer, "a")
	if err != nil {
		t.Error(err.Error())
	}

	err = linker.CacheMsg(&peer, "a")
	if err == nil {
		t.Error("failed find repeated cache error")
	}

	p, err := linker.GetMsgPeer("a")
	if err != nil || p != &peer {
		t.Error("failed get msg peer")
	}

	err = linker.DelPeer(&peer)
	if err != nil {
		t.Error(err.Error())
	}
}

func TestRepeatedGet(t *testing.T) {
	linker := common.NewPeerMsgLinker()

	var peer cascade.Peer

	err := linker.AddPeer(&peer)
	if err != nil {
		t.Error(err.Error())
	}

	err = linker.CacheMsg(&peer, "a")
	if err != nil {
		t.Error(err.Error())
	}

	p, err := linker.GetMsgPeer("a")
	if err != nil || p != &peer {
		t.Error("failed get msg peer")
	}

	p, err = linker.GetMsgPeer("a")
	if err == nil {
		t.Error("failed find repeated get msg peer error")
	}

	err = linker.DelPeer(&peer)
	if err != nil {
		t.Error(err.Error())
	}
}

func TestCleanExpire(t *testing.T) {
	linker := common.NewPeerMsgLinker()

	var peer cascade.Peer

	err := linker.AddPeer(&peer)
	if err != nil {
		t.Error(err.Error())
	}

	linker.CacheMsg(&peer, "a")
	linker.CacheMsg(&peer, "b")
	linker.CacheMsg(&peer, "c")

	time.Sleep(time.Second * 1)

	linker.CleanExpire(0)

	_, err = linker.GetMsgPeer("a")
	if err == nil {
		t.Error("failed find expire msg error")
	}

	_, err = linker.GetMsgPeer("b")
	if err == nil {
		t.Error("failed find expire msg error")
	}

	_, err = linker.GetMsgPeer("c")
	if err == nil {
		t.Error("failed find expire msg error")
	}

	err = linker.DelPeer(&peer)
	if err != nil {
		t.Error(err.Error())
	}
}
