import json
import time
from datetime import datetime

import websocket
import traceback

from trader import Trader

addr = "127.0.0.1:8001"


# addr = "127.0.0.1:8002"


class TraderInsertOrder(Trader):
    def __init__(self):
        Trader.__init__(self)
        self.order_map = {}

    def on_confirmorder(self, msg):
        if 'error_id' in msg:
            error_id = msg['error_id']
        else:
            error_id = 0

        outside_id = msg['data']['outside_id']
        order_id = msg['data']['order_id']

        if error_id != 0:
            print("order error: {0} - upstream error id:({1})".format(order_id, error_id))
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
            print("order failed: order({0}) - status: {1}, submit status: {2}".format(order_id, order_status,
                                                                                      order_submit_status))
            return

        order = self.order_map.get(outside_id)
        if order is None:
            print("receive order status from other: " + str(outside_id))
        else:
            print(
                "order status update: order({0}) - status: {1}, submit status: {2}, amount: {3}, dealed amount: {4}".format(
                    order['order_id'], order_status, order_submit_status, msg['data']['amount'], msg['data']['dealed_amount']))

    def on_orderdeal(self, msg):
        outside_id = msg['data']['order']['outside_id']
        order = self.order_map.get(outside_id)
        if order is None:
            print("receive order deal from other: " + str(outside_id))
        else:
            print("order deal: " + str(msg))
            print("original order: " + str(order))


if __name__ == '__main__':
    trader = TraderInsertOrder()

    ts = time.time()
    trader.insert_order(
        user_id="weidaizi",
        market="ctp",
        exchange="SHFE",
        type="future",
        symbol="rb",
        contract="1901",
        contract_id="1901",
        order_type="limit",
        order_flag1="speculation",  # speculation, hedge, arbitrage
        dir="closehistory_long",  # [action: open, close, closetoday, closehistory; dir: long, short] or [buy, sell]
        price=3900,
        amount=1,
        total_price=0,
        ts=ts
    )

    trader.message_loop()
