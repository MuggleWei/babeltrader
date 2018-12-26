package main

import (
	"flag"
	"time"

	okex "github.com/MuggleWei/babel-trader/src/babeltrader-okex-v3"
)

type OkexQuoteConfig struct {
	// service
	Host string
	Port int

	// consul
	EnableConsul  bool
	ConsulHost    string
	ConsulPort    int
	ConsulService string
	ConsulID      string
	ConsulTag     string
	ConsulTTL     time.Duration

	// okex config
	OkexConfig *okex.Config
}

func LoadConfig(conf_path string) (*OkexQuoteConfig, error) {
	// load flag config
	host := flag.String("host", "127.0.0.1", "bind host")
	port := flag.Int("port", 6003, "listen port")

	enableConsul := flag.Bool("consul.enable", false, "enable use consul")
	consulHost := flag.String("consul.host", "127.0.0.1", "consul host")
	consulPort := flag.Int("consul.port", 8500, "consul port")
	consulService := flag.String("consul.service", "babeltrader-okex-quote", "service name")
	consulID := flag.String("consul.id", "babeltrader-okex-quote-0", "service id")
	consulTag := flag.String("consul.tag", "", "consul service tags")
	consulTTL := flag.String("consul.ttl", "3s", "consul ttl")

	flag.Parse()

	ttl, err := time.ParseDuration(*consulTTL)
	if err != nil {
		return nil, err
	}
	if ttl < time.Second {
		ttl = time.Second
	}

	// load okex config
	okexConfig, err := okex.LoadConfig(conf_path)
	if err != nil {
		return nil, err
	}

	return &OkexQuoteConfig{
		Host: *host,
		Port: *port,

		EnableConsul:  *enableConsul,
		ConsulHost:    *consulHost,
		ConsulPort:    *consulPort,
		ConsulService: *consulService,
		ConsulID:      *consulID,
		ConsulTag:     *consulTag,
		ConsulTTL:     ttl,

		OkexConfig: okexConfig,
	}, nil
}
