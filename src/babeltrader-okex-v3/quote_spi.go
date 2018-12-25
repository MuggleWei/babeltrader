package babeltrader_okex_common

import "github.com/MuggleWei/cascade"

type OkexMsgCallback func(msg *RspCommon)

type QuoteSpi interface {
	OnConnected(peer *cascade.Peer)
	OnDisconnected(peer *cascade.Peer)

	OnLogin(msg *RspCommon)
	OnError(msg *RspCommon)

	OnSub(msg *RspCommon)
	OnUnsub(msg *RspCommon)

	OnKline(msg *RspCommon)
	OnTicker(msg *RspCommon)
	OnDepth(msg *RspCommon)
	OnDepthL2(msg *RspCommon)
}
