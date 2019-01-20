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
		msg.ProductType = common.ProductType_Future
	}

	if strings.HasPrefix(msg.Info1, "candle") {
		if strings.HasSuffix(msg.Info1, "60s") {
			msg.Info2 = common.QuoteInfo2_1Min
		} else {
			s := fmt.Sprintf("unsupport channel: %v", channel)
			log.Println(s)
			return nil, errors.New(s)
		}
		msg.Info1 = common.QuoteInfo1_Kline
	} else if msg.Info1 == "depth5" {
		msg.Info1 = common.QuoteInfo1_Depth
	} else if msg.Info1 == "depth" {
		msg.Info1 = common.QuoteInfo1_DepthL2
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
	case common.ProductType_Future:
		if quote.Type == common.ProductType_Future && quote.Contract == "SWAP" {
			productType = "swap"
		} else {
			productType = "futures"
		}
	case common.ProductType_Spot:
		productType = "spot"
	default:
		s := fmt.Sprintf("not support type: %v", quote.Type)
		return "", errors.New(s)
	}

	info := ""
	switch quote.InfoPrimary {
	case common.QuoteInfo1_Ticker:
		info = "ticker"
	case common.QuoteInfo1_Depth:
		info = "depth5"
	case common.QuoteInfo1_DepthL2:
		info = "depth"
	case common.QuoteInfo1_Kline:
		if quote.InfoExtra == common.QuoteInfo2_1Min {
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
		if channelMsg.ProductType == common.ProductType_Future {
			idx := strings.LastIndex(candle.InstrumentId, "-")
			if idx <= 0 || idx >= len(candle.InstrumentId)-1 {
				s := fmt.Sprintf("invalid instrument id: %v", candle.InstrumentId)
				log.Printf("[Warning] %v\n", s)
				return nil, errors.New(s)
			}
			channelMsg.Symbol = candle.InstrumentId[:idx]
			channelMsg.Contract = candle.InstrumentId[idx+1:]
		} else if channelMsg.ProductType == common.ProductType_Spot {
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
		if channelMsg.ProductType == common.ProductType_Future {
			idx := strings.LastIndex(ticker.InstrumentId, "-")
			if idx <= 0 || idx >= len(ticker.InstrumentId)-1 {
				s := fmt.Sprintf("invalid instrument id: %v", ticker.InstrumentId)
				log.Printf("[Warning] %v\n", s)
				return nil, errors.New(s)
			}
			channelMsg.Symbol = ticker.InstrumentId[:idx]
			channelMsg.Contract = ticker.InstrumentId[idx+1:]
		} else if channelMsg.ProductType == common.ProductType_Spot {
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
		if channelMsg.ProductType == common.ProductType_Spot {
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

func ConvertDepthToQuotes(table string, depths []Depth) ([]common.MessageRspCommon, error) {
	channelMsg, err := SplitChannel(table)
	if err != nil {
		return nil, err
	}

	var quotes []common.MessageRspCommon

	for _, depth := range depths {
		if channelMsg.ProductType == common.ProductType_Future {
			idx := strings.LastIndex(depth.InstrumentId, "-")
			if idx <= 0 || idx >= len(depth.InstrumentId)-1 {
				s := fmt.Sprintf("invalid instrument id: %v", depth.InstrumentId)
				log.Printf("[Warning] %v\n", s)
				return nil, errors.New(s)
			}
			channelMsg.Symbol = depth.InstrumentId[:idx]
			channelMsg.Contract = depth.InstrumentId[idx+1:]
		} else if channelMsg.ProductType == common.ProductType_Spot {
			channelMsg.Symbol = depth.InstrumentId
		}

		ts, err := time.Parse(time.RFC3339, depth.Timestamp)
		if err != nil {
			log.Printf("[Warning] %v\n", err.Error())
			return nil, err
		}

		asks := [][]float64{}
		bids := [][]float64{}

		if strings.HasPrefix(table, "futures") {
			for _, ask := range depth.Asks {
				asks = append(asks, []float64{ask[0].(float64), ask[1].(float64)})
			}
			for _, bid := range depth.Bids {
				bids = append(bids, []float64{bid[0].(float64), bid[1].(float64)})
			}
		} else {
			for _, ask := range depth.Asks {
				price, err := strconv.ParseFloat(ask[0].(string), 64)
				if err != nil {
					log.Printf("[Warning] %v\n", err.Error())
					return nil, err
				}
				vol, err := strconv.ParseFloat(ask[1].(string), 64)
				if err != nil {
					log.Printf("[Warning] %v\n", err.Error())
					return nil, err
				}
				asks = append(asks, []float64{price, vol})
			}
			for _, bid := range depth.Bids {
				price, err := strconv.ParseFloat(bid[0].(string), 64)
				if err != nil {
					log.Printf("[Warning] %v\n", err.Error())
					return nil, err
				}
				vol, err := strconv.ParseFloat(bid[1].(string), 64)
				if err != nil {
					log.Printf("[Warning] %v\n", err.Error())
					return nil, err
				}
				bids = append(bids, []float64{price, vol})
			}
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
				Data: common.MessageQuoteDepth{
					Asks:      asks,
					Bids:      bids,
					Timestamp: ts.Unix()*1000 + int64(ts.Nanosecond())/int64(time.Millisecond),
				},
			},
		}

		quotes = append(quotes, quote)
	}

	return quotes, nil
}

/*
RETURN: productType, okex.Order, error
*/
func ConvertInsertOrderCommon2Okex(order *common.MessageOrder) (string, *Order, error) {
	if order.ProductType == common.ProductType_Future {
		if order.OrderType == common.OrderType_Market {
			s := fmt.Sprintf("invalid order type '%v' for okex future ", order.OrderType)
			return "", nil, errors.New(s)
		}

		dir := "0"
		if order.Dir == common.OrderAction_Open+"_"+common.OrderDir_Long {
			dir = "1"
		} else if order.Dir == common.OrderAction_Open+"_"+common.OrderDir_Short {
			dir = "2"
		} else if order.Dir == common.OrderAction_Close+"_"+common.OrderDir_Long {
			dir = "3"
		} else if order.Dir == common.OrderAction_Close+"_"+common.OrderDir_Short {
			dir = "4"
		} else {
			s := fmt.Sprintf("invalid order dir: %v", order.Dir)
			return "", nil, errors.New(s)
		}

		price := fmt.Sprintf("%v", order.Price)
		size := fmt.Sprintf("%v", order.Amount)
		leverage := "20"
		productType := "futures"
		if order.Contract == "SWAP" {
			leverage = ""
			productType = "swap"
		} else {
			if order.Leverage > 0 {
				leverage = strconv.Itoa(int(order.Leverage))
			}
		}

		return productType, &Order{
			InstrumentId: order.Symbol + "-" + order.Contract,
			Type:         dir,
			Price:        price,
			Size:         size,
			Leverage:     leverage,
		}, nil
	} else if order.ProductType == common.ProductType_Spot {
		okexOrder := Order{
			InstrumentId: order.Symbol,
			Type:         order.OrderType,
			Side:         order.Dir,
		}
		if order.OrderType == common.OrderType_Limit {
			okexOrder.Price = fmt.Sprintf("%v", order.Price)
			okexOrder.Size = fmt.Sprintf("%v", order.Amount)
		} else if order.OrderType == common.OrderType_Market {
			if order.Dir == common.OrderAction_Buy {
				okexOrder.Notional = fmt.Sprintf("%v", order.TotalPrice)
			} else {
				okexOrder.Size = fmt.Sprintf("%v", order.Amount)
			}
		}

		return "spot", &okexOrder, nil
	} else {
		s := fmt.Sprintf("not support product type: %v", order.ProductType)
		return "", nil, errors.New(s)
	}
}

/*
RETURN: productType, okex.Order, error
*/
func ConvertCancelOrderCommon2Okex(order *common.MessageOrder) (string, *Order, error) {
	if order.ProductType == common.ProductType_Future {
		okexOrder := Order{
			InstrumentId: order.Symbol + "-" + order.Contract,
		}

		productType := "futures"
		if order.Contract == "SWAP" {
			productType = "swap"
		}

		return productType, &okexOrder, nil
	} else if order.ProductType == common.ProductType_Spot {
		okexOrder := Order{
			InstrumentId: order.Symbol,
		}

		return "spot", &okexOrder, nil
	} else {
		s := fmt.Sprintf("not support product type: %v", order.ProductType)
		return "", nil, errors.New(s)
	}
}

func ConvertQryCommon2Okex(qry *common.MessageQuery) (*Query, error) {
	if qry.ProductType == common.ProductType_Future {
		productType := "futures"
		if qry.Contract == "SWAP" {
			productType = "swap"
		}

		return &Query{
			InstrumentId: qry.Symbol + "-" + qry.Contract,
			ProductType:  productType,
			OrderId:      qry.OutsideId,
		}, nil
	} else if qry.ProductType == common.ProductType_Spot {
		return &Query{
			InstrumentId: qry.Symbol,
			ProductType:  "spot",
			OrderId:      qry.OutsideId,
		}, nil
	} else {
		s := fmt.Sprintf("not support product type: %v", qry.ProductType)
		return nil, errors.New(s)
	}
}

func ConvertOrderTradeOkex2Common(productType string, okexTrade *OrderTrade) (*common.MessageRspCommon, error) {
	var err error

	if productType == "spot" {
		// order field
		price := 0.0
		amount := 0.0
		total_price := 0.0
		if okexTrade.Type == "limit" {
			price, err = strconv.ParseFloat(okexTrade.Price, 64)
			if err != nil {
				log.Printf("[Error] %v\n", err.Error())
				return nil, err
			}

			amount, err = strconv.ParseFloat(okexTrade.Size, 64)
			if err != nil {
				log.Printf("[Error] %v\n", err.Error())
				return nil, err
			}
		} else if okexTrade.Type == "market" {
			if okexTrade.Side == "buy" {
				total_price, err = strconv.ParseFloat(okexTrade.Notional, 64)
				if err != nil {
					log.Printf("[Error] %v\n", err.Error())
					return nil, err
				}
			} else if okexTrade.Side == "sell" {
				amount, err = strconv.ParseFloat(okexTrade.Size, 64)
				if err != nil {
					log.Printf("[Error] %v\n", err.Error())
					return nil, err
				}
			}
		}

		// ordertrade field
		status := common.OrderStatus_Unknown
		switch okexTrade.Status {
		case "open":
			status = common.OrderStatus_Unknown
		case "part_filled":
			status = common.OrderStatus_PartDealed
		case "filled":
			status = common.OrderStatus_AllDealed
		case "cancelled":
			status = common.OrderStatus_Canceled
		case "failure":
			status = common.OrderStatus_Rejected
		default:
			s := fmt.Sprintf("invalid spot order status: %v\n", okexTrade.Status)
			log.Printf("[Error] %v", s)
			return nil, errors.New(s)
		}

		ts, err := time.Parse(time.RFC3339, okexTrade.Timestamp)
		if err != nil {
			log.Printf("[Error] %v\n", err.Error())
			return nil, err
		}

		dealedNotional, err := strconv.ParseFloat(okexTrade.FilledNotional, 64)
		if err != nil {
			log.Printf("[Error] %v\n", err.Error())
			return nil, err
		}

		dealedAmount, err := strconv.ParseFloat(okexTrade.FilledSize, 64)
		if err != nil {
			log.Printf("[Error] %v\n", err.Error())
			return nil, err
		}

		avgPrice := 0.0
		if dealedAmount > 0 {
			avgPrice = dealedNotional / dealedAmount
		}

		rsp := common.MessageRspCommon{
			Message: "ordertrade",
			Data: common.MessageOrderTrade{
				Status:       status,
				Timestamp:    ts.Unix()*1000 + int64(ts.Nanosecond())/int64(time.Millisecond),
				DealedAmount: dealedAmount,
				AvgPrice:     avgPrice,
				Order: common.MessageOrder{
					OutsideId:   okexTrade.OrderId,
					Market:      common.Market_OKEX,
					Exchange:    common.Exchange_OKEX,
					ProductType: common.ProductType_Spot,
					Symbol:      okexTrade.InstrumentId,
					OrderType:   okexTrade.Type,
					Dir:         okexTrade.Side,
					Price:       price,
					Amount:      amount,
					TotalPrice:  total_price,
				},
			},
		}

		return &rsp, nil
	} else if productType == "futures" || productType == "swap" {
		// order field
		splitSymbol := strings.LastIndex(okexTrade.InstrumentId, "-")
		if splitSymbol == -1 || splitSymbol >= len(okexTrade.InstrumentId)-1 {
			s := fmt.Sprintf("invalid instrument id for future: %v", okexTrade.InstrumentId)
			log.Printf("[Error] %v\n", s)
			return nil, errors.New(s)
		}

		symbol := okexTrade.InstrumentId[:splitSymbol]
		contract := okexTrade.InstrumentId[splitSymbol+1:]
		orderType := common.OrderType_Limit

		dir := ""
		switch okexTrade.Type {
		case "1":
			dir = common.OrderAction_Open + "_" + common.OrderDir_Long
		case "2":
			dir = common.OrderAction_Open + "_" + common.OrderDir_Short
		case "3":
			dir = common.OrderAction_Close + "_" + common.OrderDir_Long
		case "4":
			dir = common.OrderAction_Close + "_" + common.OrderDir_Short
		default:
			s := fmt.Sprintf("invalid future order dir: %v\n", okexTrade.Type)
			log.Printf("[Error] %v", s)
			return nil, errors.New(s)
		}

		price, err := strconv.ParseFloat(okexTrade.Price, 64)
		if err != nil {
			log.Printf("[Error] %v\n", err.Error())
			return nil, err
		}

		amount, err := strconv.ParseFloat(okexTrade.Size, 64)
		if err != nil {
			log.Printf("[Error] %v\n", err.Error())
			return nil, err
		}

		leverage, err := strconv.ParseFloat(okexTrade.Leverage, 64)

		// ordertrade field
		status := common.OrderStatus_Unknown
		switch okexTrade.Status {
		case "-1":
			status = common.OrderStatus_Canceled
		case "0":
			status = common.OrderStatus_Unknown
		case "1":
			status = common.OrderStatus_PartDealed
		case "2":
			status = common.OrderStatus_AllDealed
		default:
			s := fmt.Sprintf("invalid future order status: %v\n", okexTrade.Status)
			log.Printf("[Error] %v", s)
			return nil, errors.New(s)
		}

		ts, err := time.Parse(time.RFC3339, okexTrade.Timestamp)
		if err != nil {
			log.Printf("[Error] %v\n", err.Error())
			return nil, err
		}

		dealedAmount, err := strconv.ParseFloat(okexTrade.FilledQty, 64)
		if err != nil {
			log.Printf("[Error] %v\n", err.Error())
			return nil, err
		}

		avgPrice, err := strconv.ParseFloat(okexTrade.PriceAvg, 64)
		if err != nil {
			log.Printf("[Error] %v\n", err.Error())
			avgPrice = 0.0
		}

		rsp := common.MessageRspCommon{
			Message: "ordertrade",
			Data: common.MessageOrderTrade{
				Status:       status,
				Timestamp:    ts.Unix()*1000 + int64(ts.Nanosecond())/int64(time.Millisecond),
				DealedAmount: dealedAmount,
				AvgPrice:     avgPrice,
				Order: common.MessageOrder{
					OutsideId:   okexTrade.OrderId,
					Market:      common.Market_OKEX,
					Exchange:    common.Exchange_OKEX,
					ProductType: common.ProductType_Future,
					Symbol:      symbol,
					Contract:    contract,
					OrderType:   orderType,
					Dir:         dir,
					Price:       price,
					Amount:      amount,
					Leverage:    leverage,
				},
			},
		}

		return &rsp, nil
	} else {
		s := fmt.Sprintf("invalid product type: %v", productType)
		return nil, errors.New(s)
	}
}

///////////////// sign /////////////////
func GenWsSign(timestamp, method, requestPath, secret string) (string, error) {
	message := timestamp + strings.ToUpper(method) + requestPath
	return HmacSha256Base64Signer(message, secret)
}
