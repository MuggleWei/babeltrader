import time

from trader import Trader


class TraderQueryPositionDetail(Trader):
    def __init__(self):
        addr = "127.0.0.1:8001"
        Trader.__init__(self, addr=addr)

        self.qry_id = "3"


if __name__ == '__main__':
    trader = TraderQueryPositionDetail()

    ts = time.time()
    trader.query_positiondetail(
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
