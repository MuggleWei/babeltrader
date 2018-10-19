package main

import (
	"log"
	"os"

	"github.com/gorilla/websocket"
)

func main() {
	// addr := "ws://127.0.0.1:6001/ws"
	addr := "ws://127.0.0.1:6002/ws"

	c, _, err := websocket.DefaultDialer.Dial(addr, nil)
	if err != nil {
		log.Printf("[Error] failed dial to %v: %v", addr, err.Error())
		os.Exit(-1)
	}

	for {
		_, message, err := c.ReadMessage()
		if err != nil {
			if websocket.IsUnexpectedCloseError(err, websocket.CloseGoingAway, websocket.CloseAbnormalClosure) {
				log.Printf("error: %v", err)
			}
			break

		}

		log.Println(string(message))
	}
}
