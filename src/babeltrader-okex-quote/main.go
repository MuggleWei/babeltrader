package main

import (
	"log"
	"net/http"
	"strconv"

	srd "github.com/MuggleWei/go-toy/srd"
	"github.com/gorilla/mux"
	lumberjack "gopkg.in/natefinch/lumberjack.v2"
)

func init() {
	log.SetOutput(&lumberjack.Logger{
		Filename:   "./log/babeltrader-okex-quote.log",
		MaxSize:    100,   // MB
		MaxBackups: 30,    // old files
		MaxAge:     30,    // day
		Compress:   false, // disabled by default
	})
	// log.SetOutput(os.Stdout)
	log.SetFlags(log.LstdFlags | log.LUTC | log.Lmicroseconds | log.Lshortfile)
}

func main() {
	config, err := LoadConfig("config/okex_v3_conf.json")
	if err != nil {
		log.Println("[Error] failed load config")
		panic(err)
	}

	// router
	router := mux.NewRouter()
	initServices(router, config)

	addr := config.Host + ":" + strconv.Itoa(config.Port)
	log.Printf("Listening %v\n", addr)
	http.ListenAndServe(addr, router)
}

func initServices(router *mux.Router, conf *OkexQuoteConfig) {
	// service discovery
	if conf.EnableConsul {
		_, err := initServiceDiscoveryClient(conf)
		if err != nil {
			log.Println("[Error] failed init service discovery client")
			panic(err)
		}
	}

	// create service
	clientService := NewClientService()
	quoteService := NewOkexQuoteService(conf)

	// dependency injection
	clientService.QuoteService = quoteService
	quoteService.ClientService = clientService

	// run
	clientService.Run()
	quoteService.Run()

	router.HandleFunc("/ws", clientService.Hub.OnAccept)
	router.HandleFunc("/topics/get", clientService.TopicsGet)
	router.HandleFunc("/topics/sub", clientService.TopicsSub)
	router.HandleFunc("/topics/unsub", clientService.TopicsUnsub)
}

func initServiceDiscoveryClient(conf *OkexQuoteConfig) (srd.ServiceDiscoveryClient, error) {
	consulAddr := conf.ConsulHost + ":" + strconv.Itoa(conf.ConsulPort)
	serviceDiscoveryClient, err := srd.NewConsulClient(consulAddr)
	if err != nil {
		return nil, err
	}

	registration := srd.ServiceRegistration{
		ID:    conf.ConsulID,
		Name:  conf.ConsulService,
		Addr:  conf.Host,
		Port:  conf.Port,
		Tag:   []string{conf.ConsulTag},
		TTL:   conf.ConsulTTL,
		Check: nil,
	}
	err = serviceDiscoveryClient.Register(&registration)
	if err != nil {
		return nil, err
	}

	return serviceDiscoveryClient, nil
}
