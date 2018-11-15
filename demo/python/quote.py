import json
import time
import traceback

import websocket

addr = "127.0.0.1:6001"
# addr = "127.0.0.1:6002"

if __name__ == '__main__':
    ws = websocket.create_connection("ws://" + addr + "/ws")
    try:
        while True:
            result = ws.recv()
            print(result)
    except Exception as e:
        print(traceback.format_exc())
    ws.close()
