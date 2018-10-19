package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"strings"
	"time"
)

type Quote struct {
	Market     string `json:"market"`
	Exchange   string `json:"exchange"`
	Type       string `json:"type"`
	Symbol     string `json:"symbol"`
	Contract   string `json:"contract"`
	ContractId string `json:"contract_id"`
	Info1      string `json:"info1"`
	Info2      string `json:"info2"`
}

func HttpRequest(client *http.Client, reqType string, data string, url string) ([]byte, error) {
	req, _ := http.NewRequest(reqType, url, strings.NewReader(data))

	resp, err := client.Do(req)
	if err != nil {
		return nil, err
	}

	defer resp.Body.Close()

	bodyData, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}

	if resp.StatusCode != 200 {
		return nil, errors.New(fmt.Sprintf("HttpStatusCode:%d ,Desc:%s", resp.StatusCode, string(bodyData)))
	}

	return bodyData, nil
}

func TopicGet(addr string) ([]byte, error) {
	url := "http://" + addr + "/topic/get"
	return HttpRequest(http.DefaultClient, "GET", "", url)
}

func TopicSub(addr string, quote *Quote) ([]byte, error) {
	url := "http://" + addr + "/topic/sub"
	bytes, err := json.Marshal(quote)
	if err != nil {
		return nil, err
	}
	return HttpRequest(http.DefaultClient, "Post", string(bytes), url)
}

func TopicUnsub(addr string, quote *Quote) ([]byte, error) {
	url := "http://" + addr + "/topic/unsub"
	bytes, err := json.Marshal(quote)
	if err != nil {
		return nil, err
	}
	return HttpRequest(http.DefaultClient, "Post", string(bytes), url)
}

func main() {
	// ctp demo
	// addr := "127.0.0.1:6001"

	// xtp demo
	addr := "127.0.0.1:6002"
	quote := Quote{
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
