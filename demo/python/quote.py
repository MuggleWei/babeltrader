import json
import time
import traceback

import websocket

# addr = "127.0.0.1:6001"
addr = "127.0.0.1:6002"

total_elapsed = 0
total_pkg = 0
step = 100000


def print_elapsed_ms(result, receive_ts):
    global total_pkg
    global total_elapsed
    global step

    msg = json.loads(result)
    quote = msg['data']
    if 'ts' in msg['data']:
        elpased_ms = int(round(receive_ts * 1000)) - msg['data']['ts']
        total_elapsed += elpased_ms
        total_pkg += 1
        if total_pkg >= step:
            print("total pkg: " + str(total_pkg) + ", avg elapsed ms: " + str(total_elapsed / total_pkg))
            total_elapsed = 0
            total_pkg = 0


if __name__ == '__main__':
    ws = websocket.create_connection("ws://" + addr + "/ws")
    try:
        while True:
            result = ws.recv()
            ts = time.time()
            # print(result)
            print_elapsed_ms(result=result, receive_ts=ts)
    except Exception as e:
        print(traceback.format_exc())
    ws.close()
