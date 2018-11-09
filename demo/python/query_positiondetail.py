from trader import Trader


class TraderQueryPositionDetail(Trader):
    def __init__(self, addr):
        Trader.__init__(self, addr=addr)

        self.qry_id = "3"


def TestQueryPositionDetail_CTP(trader):
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


def TestQueryPositionDetail_XTP(trader):
    trader.query_positiondetail(
        qry_id=trader.qry_id,
        user_id="weidaizi",
        market="xtp",
        exchange="",
        type="",
        symbol="",
        contract="",
        contract_id=""
    )


if __name__ == '__main__':
    # ctp
    # addr = "127.0.0.1:8001"
    # fn = TestQueryPositionDetail_CTP

    # xtp
    addr = "127.0.0.1:8002"
    fn = TestQueryPositionDetail_XTP

    trader = TraderQueryPositionDetail(addr)
    fn(trader)

    trader.message_loop()
