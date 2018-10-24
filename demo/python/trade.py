import json
import time
from datetime import datetime

import websocket
import traceback

addr = "127.0.0.1:8001"
# addr = "127.0.0.1:8002"

def insert_order(ws, user_id, client_order_id,
                 market, exchange, type, symbol, contract, contract_id,
                 order_type, dir, price, amount, total_price, ts):
    utc_ts = datetime.utcfromtimestamp(ts)
    ts_str = utc_ts.strftime("%Y%m%d-%H%M%S")

    ws.send(payload=json.dumps({
        "msg": "insert_order",
        "data": {
            "user_id": user_id,
            "order_id": str(user_id) + ts_str,
            "extra_order_id": "",
            "outside_id": "",
            "client_order_id": client_order_id,
            "market": market,
            "exchange": exchange,
            "type": type,
            "symbol": symbol,
            "contract": contract,
            "contract_id": contract_id,
            "order_type": order_type,
            "dir": dir,
            "price": price,
            "amount": amount,
            "total_price": total_price,
            "ts": ts
        }
    }))

if __name__ == '__main__':
    ws = websocket.create_connection("ws://" + addr + "/ws")
    try:
        ts = time.time()
        insert_order(
            ws=ws,
            user_id="weidaizi",
            client_order_id="20181024-1624-000001",
            market="ctp",
            exchange="",
            type="future",
            symbol="rb",
            contract="1901",
            contract_id="1901",
            order_type="limit",
            dir="open_long",
            price=4190,
            amount=1,
            total_price=0,
            ts=ts
        )
        result = ws.recv()
        result = json.loads(result)
        print(result)
    except Exception as e:
        print(traceback.format_exc())
    ws.close()
