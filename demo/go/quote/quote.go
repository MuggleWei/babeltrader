package main

import "log"

func main() {
	log.SetFlags(log.LstdFlags | log.LUTC | log.Lmicroseconds | log.Lshortfile)

	service := NewQuoteService()
	service.Run()
}
