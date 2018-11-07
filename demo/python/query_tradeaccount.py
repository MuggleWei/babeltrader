import time

from trader import Trader


class TraderQueryTradeAccount(Trader):
    def __init__(self):
        addr = "127.0.0.1:8001"
        Trader.__init__(self, addr=addr)

        self.qry_id = "3"


if __name__ == '__main__':
    trader = TraderQueryTradeAccount()

    ts = time.time()
    trader.query_tradeaccount(
        qry_id=trader.qry_id,
        user_id="weidaizi",
        market="ctp"
    )

    trader.message_loop()
