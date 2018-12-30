package babeltrader_utils_go

import (
	"encoding/json"
	"fmt"
	"log"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
	cascade "github.com/MuggleWei/cascade"
	"github.com/mitchellh/mapstructure"
)

///////////////// utils functions /////////////////
func DecodeInterfaceByJson(data interface{}, result_pointer interface{}) error {
	config := &mapstructure.DecoderConfig{TagName: "json", Result: result_pointer}
	decoder, err := mapstructure.NewDecoder(config)
	if err != nil {
		log.Printf("[Warning] failed new decoder: %v\n", err.Error())
		return err
	}

	err = decoder.Decode(data)
	if err != nil {
		log.Printf("[Warning] failed decode data: %v, %v\n", data, err.Error())
		return err
	}

	return nil
}

///////////////// response functions /////////////////
func ResponseError(peer *cascade.Peer, errId int64, errMsg string, data interface{}) {
	rsp := common.MessageRspCommon{
		Message: "error",
		ErrId:   errId,
		ErrMsg:  errMsg,
		Data:    data,
	}
	ResponseClient(peer, &rsp)
}
func ResponseClient(peer *cascade.Peer, rsp *common.MessageRspCommon) {
	if rsp != nil {
		b, err := json.Marshal(*rsp)
		if err != nil {
			panic(err)
		}

		peer.SendChannel <- b
	}
}

///////////////// handle requests /////////////////
type ReqCallback func(*cascade.Peer, *common.MessageReqCommon) (*common.MessageRspCommon, error)
type RspCallback func(*common.MessageRspCommon, *cascade.HubByteMessage)

func DispatchRequests(peer *cascade.Peer, message []byte, callbacks map[string]ReqCallback) {
	var req common.MessageReqCommon
	err := json.Unmarshal(message, &req)
	if err != nil {
		errMsg := fmt.Sprintf("failed parse json: %v", string(message))
		log.Printf("[Error] %v\n", errMsg)
		ResponseError(peer, -1, errMsg, nil)
		return
	}

	callback, ok := callbacks[req.Message]
	if !ok {
		errMsg := fmt.Sprintf("failed find callback of: %v", req.Message)
		log.Printf("[Error] %v\n", errMsg)
		ResponseError(peer, -1, errMsg, req)
		return
	}

	log.Printf("%v: %v\n", req.Message, string(message))
	rsp, err := callback(peer, &req)
	if err != nil {
		log.Printf("[Error] %v - %v\n", err, string(message))

		if rsp == nil {
			rsp = &common.MessageRspCommon{
				Message: "error",
				ErrId:   -1,
				Data:    req,
			}
		}
	}

	if rsp != nil {
		ResponseClient(peer, rsp)
	}
}

func DispatchResponse(msg *cascade.HubByteMessage, callbacks map[string]RspCallback) {
	var rsp common.MessageRspCommon
	err := json.Unmarshal(msg.Message, &rsp)
	if err != nil {
		log.Printf("failed unmarshal message: %v\n", string(msg.Message))
		return
	}

	callback, ok := callbacks[rsp.Message]
	if !ok {
		log.Printf("failed find callback of: %v", rsp.Message)
		return
	}

	callback(&rsp, msg)
}
