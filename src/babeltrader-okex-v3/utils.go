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
func ConvertChannelToSubUnsub(channel string) (*common.MessageSubUnsub, error) {
	idx := strings.Index(channel, "/")
	if idx <= 0 || idx >= len(channel)-1 {
		s := fmt.Sprintf("failed find '/' in channel %v", channel)
		return nil, errors.New(s)
	}

	idx2 := strings.Index(channel, ":")
	if idx2 <= 0 || idx2 >= len(channel)-1 {
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
		if idx3 <= 0 || idx3 >= len(channel)-1 {
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

func ConvertCandleToQuotes(table string, candles []Candle) ([]common.MessageRspCommon, error) {
	if !strings.HasSuffix(table, "60s") {
		s := fmt.Sprintf("only support 1m kline: %v", table)
		log.Printf("[Warning] %v\n", s)
		return nil, errors.New(s)
	}

	var quotes []common.MessageRspCommon

	idx := strings.Index(table, "/")
	if idx < 0 || idx >= len(table)-1 {
		s := fmt.Sprintf("invalid table name: %v", table)
		log.Printf("[Warning] %v\n", s)
		return nil, errors.New(s)
	}

	productType := table[:idx]
	if productType == "futures" || productType == "swap" {
		productType = "future"
	}
	symbol := ""
	contract := ""
	info1 := "kline"
	info2 := "1m"

	for _, candle := range candles {
		if productType == "future" {
			idx = strings.LastIndex(candle.InstrumentId, "-")
			if idx <= 0 || idx >= len(candle.InstrumentId)-1 {
				s := fmt.Sprintf("invalid instrument id: %v", candle.InstrumentId)
				log.Printf("[Warning] %v\n", s)
				return nil, errors.New(s)
			}
			symbol = candle.InstrumentId[:idx]
			contract = candle.InstrumentId[idx+1:]
		} else if productType == "spot" {
			symbol = candle.InstrumentId
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
				Type:        productType,
				Symbol:      symbol,
				Contract:    contract,
				InfoPrimary: info1,
				InfoExtra:   info2,
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
