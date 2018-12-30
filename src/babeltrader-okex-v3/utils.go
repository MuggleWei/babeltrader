package babeltrader_okex_v3

import (
	"errors"
	"fmt"
	"log"
	"strconv"
	"strings"
	"time"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
)

///////////////// converter /////////////////
func SplitChannel(channel string) (*ChannelSplit, error) {
	var msg ChannelSplit

	idx := strings.Index(channel, "/")
	if idx <= 0 || idx >= len(channel)-1 {
		s := fmt.Sprintf("failed find '/' in channel %v", channel)
		log.Println(s)
		return nil, errors.New(s)
	}

	idx2 := strings.Index(channel, ":")
	if idx2 == -1 {
		// table format, e.g. futures/depth, futures/ticker, spot/candle60s ...
		msg.ProductType = channel[:idx]
		msg.Info1 = channel[idx+1:]
	} else if idx2 < len(channel)-1 {
		// channel format, e.g. swap/ticker:BTC-USD-SWAP, futures/ticker:BTC-USD-190329 ...
		msg.ProductType = channel[:idx]
		msg.Info1 = channel[idx+1 : idx2]

		if msg.ProductType == "futures" || msg.ProductType == "swap" {
			idx3 := strings.LastIndex(channel, "-")
			if idx3 <= 0 || idx3 >= len(channel)-1 {
				s := fmt.Sprintf("failed find last index '-' in channel %v", channel)
				log.Println(s)
				return nil, errors.New(s)
			}
			msg.Symbol = channel[idx2+1 : idx3]
			msg.Contract = channel[idx3+1:]
		} else {
			msg.Symbol = channel[idx2+1:]
		}
	} else {
		s := fmt.Sprintf("invalid channel format: %v", channel)
		log.Println(s)
		return nil, errors.New(s)
	}

	if msg.ProductType == "futures" || msg.ProductType == "swap" {
		msg.ProductType = "future"
	}

	if strings.HasPrefix(msg.Info1, "candle") {
		if strings.HasSuffix(msg.Info1, "60s") {
			msg.Info2 = "1m"
		} else {
			s := fmt.Sprintf("unsupport channel: %v", channel)
			log.Println(s)
			return nil, errors.New(s)
		}
		msg.Info1 = "kline"
	} else if msg.Info1 == "depth5" {
		msg.Info1 = "depth"
	} else if msg.Info1 == "depth" {
		msg.Info1 = "depthL2"
	}

	return &msg, nil
}

func ConvertChannelToSubUnsub(channel string) (*common.MessageSubUnsub, error) {
	msg, err := SplitChannel(channel)
	if err != nil {
		log.Printf("failed split channel: %v\n", channel)
		return nil, err
	}

	return &common.MessageSubUnsub{
		Market:      "okex",
		Exchange:    "okex",
		Type:        msg.ProductType,
		Symbol:      msg.Symbol,
		Contract:    msg.Contract,
		InfoPrimary: msg.Info1,
		InfoExtra:   msg.Info2,
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

func ConvertCandleToQuotes(table string, candles []Candle) ([]common.MessageRspCommon, error) {
	if !strings.HasSuffix(table, "60s") {
		s := fmt.Sprintf("only support 1m kline: %v", table)
		log.Printf("[Warning] %v\n", s)
		return nil, errors.New(s)
	}

	channelMsg, err := SplitChannel(table)
	if err != nil {
		return nil, err
	}

	var quotes []common.MessageRspCommon

	for _, candle := range candles {
		if channelMsg.ProductType == "future" {
			idx := strings.LastIndex(candle.InstrumentId, "-")
			if idx <= 0 || idx >= len(candle.InstrumentId)-1 {
				s := fmt.Sprintf("invalid instrument id: %v", candle.InstrumentId)
				log.Printf("[Warning] %v\n", s)
				return nil, errors.New(s)
			}
			channelMsg.Symbol = candle.InstrumentId[:idx]
			channelMsg.Contract = candle.InstrumentId[idx+1:]
		} else if channelMsg.ProductType == "spot" {
			channelMsg.Symbol = candle.InstrumentId
		}

		if len(candle.Candle) < 6 {
			s := fmt.Sprintf("invalid kline message: %v", candle.Candle)
			log.Printf("[Warning] %v\n", s)
			return nil, errors.New(s)
		}

		// convert kline fields
		ts, err := time.Parse(time.RFC3339, candle.Candle[0])
		if err != nil {
			log.Printf("[Warning] %v\n", err.Error())
			return nil, err
		}
		open, err := strconv.ParseFloat(candle.Candle[1], 64)
		if err != nil {
			log.Printf("[Warning] %v\n", err.Error())
			return nil, err
		}
		high, err := strconv.ParseFloat(candle.Candle[2], 64)
		if err != nil {
			log.Printf("[Warning] %v\n", err.Error())
			return nil, err
		}
		low, err := strconv.ParseFloat(candle.Candle[3], 64)
		if err != nil {
			log.Printf("[Warning] %v\n", err.Error())
			return nil, err
		}
		close_price, err := strconv.ParseFloat(candle.Candle[4], 64)
		if err != nil {
			log.Printf("[Warning] %v\n", err.Error())
			return nil, err
		}
		vol, err := strconv.ParseFloat(candle.Candle[5], 64)
		if err != nil {
			log.Printf("[Warning] %v\n", err.Error())
			return nil, err
		}

		quote := common.MessageRspCommon{
			Message: "quote",
			Data: common.MessageQuote{
				Market:      "okex",
				Exchange:    "okex",
				Type:        channelMsg.ProductType,
				Symbol:      channelMsg.Symbol,
				Contract:    channelMsg.Contract,
				InfoPrimary: channelMsg.Info1,
				InfoExtra:   channelMsg.Info2,
				Data: common.MessageQuoteKLine{
					Timestamp: ts.Unix()*1000 + int64(ts.Nanosecond())/int64(time.Millisecond),
					Open:      open,
					High:      high,
					Low:       low,
					Close:     close_price,
					Vol:       vol,
				},
			},
		}

		quotes = append(quotes, quote)
	}

	return quotes, nil
}

func ConvertTickerToQuotes(table string, tickers []Ticker) ([]common.MessageRspCommon, error) {
	channelMsg, err := SplitChannel(table)
	if err != nil {
		return nil, err
	}

	var quotes []common.MessageRspCommon

	for _, ticker := range tickers {
		if channelMsg.ProductType == "future" {
			idx := strings.LastIndex(ticker.InstrumentId, "-")
			if idx <= 0 || idx >= len(ticker.InstrumentId)-1 {
				s := fmt.Sprintf("invalid instrument id: %v", ticker.InstrumentId)
				log.Printf("[Warning] %v\n", s)
				return nil, errors.New(s)
			}
			channelMsg.Symbol = ticker.InstrumentId[:idx]
			channelMsg.Contract = ticker.InstrumentId[idx+1:]
		} else if channelMsg.ProductType == "spot" {
			channelMsg.Symbol = ticker.InstrumentId
		}

		ts, err := time.Parse(time.RFC3339, ticker.Timestamp)
		if err != nil {
			log.Printf("[Warning] %v\n", err.Error())
			return nil, err
		}

		last, err := strconv.ParseFloat(ticker.Last, 64)
		if err != nil {
			log.Printf("[Warning] %v\n", err.Error())
			return nil, err
		}

		bid, err := strconv.ParseFloat(ticker.BestBid, 64)
		if err != nil {
			log.Printf("[Warning] %v\n", err.Error())
			return nil, err
		}

		ask, err := strconv.ParseFloat(ticker.BestAsk, 64)
		if err != nil {
			log.Printf("[Warning] %v\n", err.Error())
			return nil, err
		}

		high, err := strconv.ParseFloat(ticker.High24H, 64)
		if err != nil {
			log.Printf("[Warning] %v\n", err.Error())
			return nil, err
		}

		low, err := strconv.ParseFloat(ticker.Low24H, 64)
		if err != nil {
			log.Printf("[Warning] %v\n", err.Error())
			return nil, err
		}

		vol := 0.0
		if channelMsg.ProductType == "spot" {
			vol, err = strconv.ParseFloat(ticker.BaseVol24H, 64)
		} else {
			vol, err = strconv.ParseFloat(ticker.Vol24H, 64)
		}

		quote := common.MessageRspCommon{
			Message: "quote",
			Data: common.MessageQuote{
				Market:      "okex",
				Exchange:    "okex",
				Type:        channelMsg.ProductType,
				Symbol:      channelMsg.Symbol,
				Contract:    channelMsg.Contract,
				InfoPrimary: channelMsg.Info1,
				InfoExtra:   channelMsg.Info2,
				Data: common.MessageQuoteTicker{
					Bid:       bid,
					Ask:       ask,
					Last:      last,
					High:      high,
					Low:       low,
					Vol:       vol,
					Timestamp: ts.Unix()*1000 + int64(ts.Nanosecond())/int64(time.Millisecond),
				},
			},
		}

		quotes = append(quotes, quote)
	}

	return quotes, nil
}
