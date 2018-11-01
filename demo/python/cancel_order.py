import json
import time
from datetime import datetime

import websocket
import traceback

from trader import Trader

addr = "127.0.0.1:8001"


# addr = "127.0.0.1:8002"


class TraderCancelOrder(Trader):
    def __init__(self):
        Trader.__init__(self)

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

        self.cancel_order(
            user_id="weidaizi",
            outside_id=outside_id,
            market=msg['data']['market'],
            exchange=msg['data']['exchange'],
            type=msg['data']['type'],
            symbol=msg['data']['symbol'],
            contract=msg['data']['contract'],
            contract_id=msg['data']['contract_id']
        )


if __name__ == '__main__':
    trader = TraderCancelOrder()

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
        dir="open_long",  # [action: open, close, close_today, close_yesterday; dir: long, short] or [buy, sell]
        price=4000,
        amount=1,
        total_price=0,
        ts=ts
    )

    trader.message_loop()
