package main

import (
	"encoding/json"
	"log"
	"net/http"
	"os"
	"time"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
)

func TopicGet(addr string) ([]byte, error) {
	url := "http://" + addr + "/topic/get"
	return common.HttpRequest(http.DefaultClient, "GET", "", url, nil)
}

func TopicSub(addr string, msg *common.MessageSubUnsub) ([]byte, error) {
	url := "http://" + addr + "/topic/sub"
	bytes, err := json.Marshal(msg)
	if err != nil {
		return nil, err
	}
	return common.HttpRequest(http.DefaultClient, "Post", string(bytes), url, nil)
}

func TopicUnsub(addr string, msg *common.MessageSubUnsub) ([]byte, error) {
	url := "http://" + addr + "/topic/unsub"
	bytes, err := json.Marshal(msg)
	if err != nil {
		return nil, err
	}
	return common.HttpRequest(http.DefaultClient, "Post", string(bytes), url, nil)
}

func main() {
	// ctp demo
	// addr := "127.0.0.1:6001"

	// xtp demo
	addr := "127.0.0.1:6002"
	quote := common.MessageSubUnsub{
		Market:   "xtp",
		Exchange: "SSE",
		Type:     "spot",
		Symbol:   "601857",
	}

	// get all topics
	rsp, err := TopicGet(addr)
	if err != nil {
		log.Printf("[Error] failed in /topic/get: %s", err.Error())
		os.Exit(-1)
	}
	log.Printf("%s", string(rsp))

	// sub topic
	rsp, err = TopicSub(addr, &quote)
	if err != nil {
		log.Printf("[Error] failed in /topic/sub: %s", err.Error())
		os.Exit(-1)
	}
	log.Printf("%s", string(rsp))

	time.Sleep(time.Second * 3)

	rsp, err = TopicGet(addr)
	if err != nil {
		log.Printf("[Error] failed in /topic/get: %s", err.Error())
		os.Exit(-1)
	}
	log.Printf("%s", string(rsp))

	// unsub topic
	rsp, err = TopicUnsub(addr, &quote)
	if err != nil {
		log.Printf("[Error] failed in /topic/sub: %s", err.Error())
		os.Exit(-1)
	}
	log.Printf("%s", string(rsp))

	time.Sleep(time.Second * 3)

	rsp, err = TopicGet(addr)
	if err != nil {
		log.Printf("[Error] failed in /topic/get: %s", err.Error())
		os.Exit(-1)
	}
	log.Printf("%s", string(rsp))
}
