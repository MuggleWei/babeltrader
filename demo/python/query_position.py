import json
import time
from datetime import datetime

import websocket
import traceback

from trader import Trader

addr = "127.0.0.1:8001"


# addr = "127.0.0.1:8002"


class TraderQueryPosition(Trader):
    def __init__(self):
        Trader.__init__(self)

        self.qry_id = "3"



if __name__ == '__main__':
    trader = TraderQueryPosition()

    ts = time.time()
    trader.query_position(
        qry_id=trader.qry_id,
        user_id="weidaizi",
        market="ctp",
        exchange="SHFE",
        type="future",
        symbol="rb",
        contract="1901",
        contract_id="1901"
    )

    trader.message_loop()
