import time

from trader import Trader

addr = "127.0.0.1:8001"


# addr = "127.0.0.1:8002"


class TraderQueryProduct(Trader):
    def __init__(self):
        Trader.__init__(self)


if __name__ == '__main__':
    trader = TraderQueryProduct()

    ts = time.time()
    trader.query_product(
        qry_id="1",
        user_id="weidaizi",
        market="ctp",
        exchange="SHFE",
        type="future",
        symbol="rb",
        contract="",
    )
    time.sleep(1)
    trader.query_product(
        qry_id="2",
        user_id="weidaizi",
        market="ctp",
        exchange="SHFE",
        type="future",
        symbol="rb",
        contract="1901",
    )

    trader.message_loop()
