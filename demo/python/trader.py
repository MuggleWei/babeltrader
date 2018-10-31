import json
import traceback
from datetime import datetime

import websocket

addr = "127.0.0.1:8001"


# addr = "127.0.0.1:8002"

class Trader:
    def __init__(self):
        self.rsp_callbacks = {}
        self.rsp_callbacks["confirmorder"] = self.on_confirmorder
        self.rsp_callbacks["orderstatus"] = self.on_orderstatus
        self.rsp_callbacks["orderdeal"] = self.on_orderdeal
        self.rsp_callbacks["error"] = self.on_error

        self.rsp_callbacks["rsp_qryorder"] = self.on_qryorder

        self.ws = websocket.create_connection("ws://" + addr + "/ws")

    def on_confirmorder(self, msg):
        pass

    def on_orderstatus(self, msg):
        pass

    def on_orderdeal(self, msg):
        pass

    def on_error(self, msg):
        print("exception happened: " + str(msg))

    def on_qryorder(self, msg):
        pass

    def insert_order(self, user_id,
                     market, exchange, type, symbol, contract, contract_id,
                     order_type, order_flag1, dir, price, amount, total_price, ts):
        utc_ts = datetime.utcfromtimestamp(ts)
        ts_str = utc_ts.strftime("%Y%m%d-%H%M%S")

        order = json.dumps({
            "msg": "insert_order",
            "data": {
                "user_id": user_id,
                "order_id": str(user_id) + '-' + ts_str,
                "outside_id": "",
                "client_order_id": str(user_id) + '-' + ts_str,
                "market": market,
                "exchange": exchange,
                "type": type,
                "symbol": symbol,
                "contract": contract,
                "contract_id": contract_id,
                "order_type": order_type,
                "order_flag1": order_flag1,
                "dir": dir,
                "price": price,
                "amount": amount,
                "total_price": total_price,
                "ts": ts
            }
        })
        self.ws.send(order)
        print("send insert order: " + str(order))

    def cancel_order(self, user_id, outside_id,
                     market, exchange, type, symbol, contract, contract_id):
        order = json.dumps({
            "msg": "cancel_order",
            "data": {
                "user_id": user_id,
                "outside_id": outside_id,
                "market": market,
                "exchange": exchange,
                "type": type,
                "symbol": symbol,
                "contract": contract,
                "contract_id": contract_id
            }
        })
        self.ws.send(order)
        print("send cancel order: " + str(order))

    def query_order(self, qry_id, user_id, outside_id,
                    market, exchange, type, symbol, contract, contract_id):
        order = json.dumps({
            "msg": "query_order",
            "data": {
                "qry_id": qry_id,
                "user_id": user_id,
                "outside_id": outside_id,
                "market": market,
                "exchange": exchange,
                "type": type,
                "symbol": symbol,
                "contract": contract,
                "contract_id": contract_id
            }
        })
        self.ws.send(order)
        print("send query order: " + str(order))

    def message_loop(self):
        try:
            while True:
                result = self.ws.recv()
                print("receive message: " + result)
                result = json.loads(result)
                fn = self.rsp_callbacks.get(result['msg'])
                if fn:
                    fn(result)
        except Exception as e:
            print(traceback.format_exc())
        self.ws.close()
