package babeltrader_okex_v3

import (
	"encoding/json"
	"io/ioutil"
	"log"
)

type Config struct {
	RestEndpoint   string   `json:"rest_endpoint"`
	WsEndpoint     string   `json:"ws_endpoint"`
	Key            string   `json:"key"`
	Secret         string   `json:"secret"`
	Passphrase     string   `json:"passphrase"`
	RestTimeoutSec int      `json:"rest_timeout_sec"`
	QuoteSubTopics []string `json:"quote_sub_topics"`
	TradeSubTopics []string `json:"trade_sub_topics"`
}

func LoadConfig(conf_path string) (*Config, error) {
	b, err := ioutil.ReadFile(conf_path)
	if err != nil {
		log.Printf("[Error] failed read config file: %v\n", conf_path)
		return nil, err
	}

	var config Config
	err = json.Unmarshal(b, &config)
	if err != nil {
		log.Printf("[Error] failed parse config file: %v\n", conf_path)
		return nil, err
	}

	return &config, nil
}
