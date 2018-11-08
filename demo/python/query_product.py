import time

from trader import Trader


class TraderQueryProduct(Trader):
    def __init__(self):
        addr = "127.0.0.1:8001"
        Trader.__init__(self, addr=addr)


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
