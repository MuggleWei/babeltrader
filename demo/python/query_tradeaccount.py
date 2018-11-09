from trader import Trader


class TraderQueryTradeAccount(Trader):
    def __init__(self, addr):
        Trader.__init__(self, addr=addr)


def TestQueryTradeAccount_CTP(trader):
    trader.query_tradeaccount(
        qry_id="1",
        user_id="weidaizi",
        market="ctp"
    )

def TestQueryTradeAccount_XTP(trader):
    trader.query_tradeaccount(
        qry_id="1",
        user_id="weidaizi",
        market="xtp"
    )

if __name__ == '__main__':
    # ctp
    addr = "127.0.0.1:8001"
    fn = TestQueryTradeAccount_CTP

    # xtp
    # addr = "127.0.0.1:8002"
    # fn = TestQueryTradeAccount_XTP

    trader = TraderQueryTradeAccount(addr)
    fn(trader)

    trader.message_loop()
