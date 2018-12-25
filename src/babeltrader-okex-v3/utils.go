package babeltrader_okex_common

import (
	"errors"
	"fmt"
	"strings"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
)

///////////////// converter /////////////////
func ConvertChannelToSubUnsub(channel string) (*common.MessageSubUnsub, error) {
	idx := strings.Index(channel, "/")
	if idx <= 0 || idx >= len(channel) {
		s := fmt.Sprintf("failed find '/' in channel %v", channel)
		return nil, errors.New(s)
	}

	idx2 := strings.Index(channel, ":")
	if idx2 <= 0 || idx2 >= len(channel) {
		s := fmt.Sprintf("failed find ':' in channel %v", channel)
		return nil, errors.New(s)
	}

	productType := channel[:idx]
	info1 := channel[idx+1 : idx2]
	info2 := ""
	if strings.HasPrefix(info1, "candle") {
		if strings.HasSuffix(info1, "60s") {
			info2 = "1m"
		} else {
			s := fmt.Sprintf("unsupport channel: %v", channel)
			return nil, errors.New(s)
		}
		info1 = "kline"
	} else if info1 == "depth5" {
		info1 = "depth"
	} else if info1 == "depth" {
		info1 = "depthL2"
	}
	symbol := ""
	contract := ""

	if productType == "futures" || productType == "swap" {
		productType = "future"
		idx3 := strings.LastIndex(channel, "-")
		if idx3 <= 0 || idx3 >= len(channel) {
			s := fmt.Sprintf("failed find last index '-' in channel %v", channel)
			return nil, errors.New(s)
		}
		symbol = channel[idx2+1 : idx3]
		contract = channel[idx3+1:]
	} else if productType == "spot" {
		symbol = channel[idx2+1:]
	} else {
		s := fmt.Sprintf("unsupport channel: %v", channel)
		return nil, errors.New(s)
	}

	return &common.MessageSubUnsub{
		Market:      "okex",
		Exchange:    "okex",
		Type:        productType,
		Symbol:      symbol,
		Contract:    contract,
		InfoPrimary: info1,
		InfoExtra:   info2,
	}, nil
}

func ConvertQuoteToChannel(quote *common.MessageQuote) (string, error) {
	productType := ""
	switch quote.Type {
	case "future":
		if quote.Type == "future" && quote.Contract == "SWAP" {
			productType = "swap"
		} else {
			productType = "futures"
		}
	case "spot":
		productType = "spot"
	default:
		s := fmt.Sprintf("not support type: %v", quote.Type)
		return "", errors.New(s)
	}

	info := ""
	switch quote.InfoPrimary {
	case "ticker":
		info = "ticker"
	case "depth":
		info = "depth5"
	case "depthL2":
		info = "depth"
	case "kline":
		if quote.InfoExtra == "1m" {
			info = "candle60s"
		} else {
			s := fmt.Sprintf("not support kline with info2: %v", quote.InfoExtra)
			return "", errors.New(s)
		}
	default:
		s := fmt.Sprintf("not support info1: %v", quote.InfoPrimary)
		return "", errors.New(s)
	}

	channel := productType + "/" + info + ":" + quote.Symbol
	if productType == "swap" {
		channel = channel + "-SWAP"
	} else if productType == "futures" {
		channel = channel + "-" + quote.Contract
	}

	return channel, nil
}
