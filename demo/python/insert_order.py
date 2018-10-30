import json
import time
from datetime import datetime

import websocket
import traceback

addr = "127.0.0.1:8001"
# addr = "127.0.0.1:8002"


class Connector:
    def __init__(self):
        self.rsp_callbacks = {}
        self.rsp_callbacks["confirmorder"] = self.on_confirmorder
        self.rsp_callbacks["orderstatus"] = self.on_orderstatus
        self.rsp_callbacks["orderdeal"] = self.on_orderdeal
        self.rsp_callbacks["error"] = self.on_error

        self.order_map = {}

        self.ws = websocket.create_connection("ws://" + addr + "/ws")

    def run(self):
        try:
            ts = time.time()
            self.insert_order(
                user_id="weidaizi",
                market="ctp",
                exchange="SHFE",
                type="future",
                symbol="rb",
                contract="1901",
                contract_id="1901",
                order_type="limit",
                order_flag1="speculation",  # speculation, hedge, arbitrage
                dir="open_long",  # [action: open, close, close_today, close_yesterday; dir: long, short] or [buy, sell]
                price=4300,
                amount=2,
                total_price=0,
                ts=ts
            )
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

    def on_confirmorder(self, msg):
        if 'error_id' in msg:
            error_id = msg['error_id']
        else:
            error_id = 0

        outside_id = msg['data']['outside_id']
        order_id = msg['data']['order_id']

        if error_id != 0:
            print("order error: {0} - 上手错误号:({1})".format(order_id, error_id))
            return

        if outside_id == "":
            print("order error: {0}".format(order_id))
            return

        self.order_map[outside_id] = msg['data']
        print("order confirm: {0} - {1}".format(outside_id, order_id))

    def on_orderstatus(self, msg):
        if 'error_id' in msg:
            error_id = msg['error_id']
        else:
            error_id = 0

        outside_id = msg['data']['order']['outside_id']
        order_id = msg['data']['order']['order_id']
        order_status = msg['data']['status']
        order_submit_status = msg['data']['submit_status']

        if outside_id == "":
            print("order failed: order({0}) - status: {1}, submit status: {2}".format(order_id, order_status, order_submit_status))
            return

        order = self.order_map.get(outside_id)
        if order is None:
            print("receive order status from other: " + str(outside_id))
        else:
            print("order status update: order({0}) - status: {1}, submit status: {2}, amount: {3}, dealed amount: {4}".format(
                order_id, order_status, order_submit_status, msg['data']['amount'], msg['data']['dealed_amount']))

    def on_orderdeal(self, msg):
        outside_id = msg['data']['order']['outside_id']
        order = self.order_map.get(outside_id)
        if order is None:
            print("receive order deal from other: " + str(outside_id))
        else:
            print("order deal: " + str(msg))
            print("original order: " + str(order))

    def on_error(self, msg):
        print("exception happened: " + str(msg))


if __name__ == '__main__':
    conn = Connector()
    conn.run()
