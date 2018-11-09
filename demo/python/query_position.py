from trader import Trader


class TraderQueryPosition(Trader):
    def __init__(self, addr):
        Trader.__init__(self, addr=addr)


def TestQueryPosition_CTP(trader):
    trader.query_position(
        qry_id="1",
        user_id="weidaizi",
        market="ctp",
        exchange="SHFE",
        type="future",
        symbol="rb",
        contract="1901",
        contract_id="1901"
    )


def TestQueryPosition_XTP(trader):
    trader.query_position(
        qry_id="1",
        user_id="weidaizi",
        market="ctp",
        exchange="",
        type="",
        symbol="600519",
        contract="",
        contract_id=""
    )


if __name__ == '__main__':
    # ctp
    addr = "127.0.0.1:8001"
    fn = TestQueryPosition_CTP

    # xtp
    # addr = "127.0.0.1:8002"
    # fn = TestQueryPosition_XTP

    trader = TraderQueryPosition(addr)
    fn(trader)

    trader.message_loop()
