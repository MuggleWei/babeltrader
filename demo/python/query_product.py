import time

from trader import Trader


class TraderQueryProduct(Trader):
    def __init__(self, addr):
        Trader.__init__(self, addr=addr)


if __name__ == '__main__':
    addr = "127.0.0.1:8001"

    trader = TraderQueryProduct(addr)

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
    time.sleep(3)
    trader.query_product(
        qry_id="3",
        user_id="weidaizi",
        market="ctp",
        exchange="",
        type="",
        symbol="",
        contract=""
    )

    trader.message_loop()
