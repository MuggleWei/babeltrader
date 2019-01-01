package babeltrader_okex_v3

import "github.com/MuggleWei/cascade"

type TradeSpi interface {
	OnConnected(peer *cascade.Peer)
	OnDisconnected(peer *cascade.Peer)

	OnLogin(msg *RspCommon)
	OnError(msg *RspCommon)

	OnSub(msg *RspCommon)
	OnUnsub(msg *RspCommon)

	OnOrder(msg *RspCommon)
	OnPosition(msg *RspCommon)
	OnAccount(msg *RspCommon)
}
