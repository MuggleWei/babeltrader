from trader import Trader


class TraderQueryOrder(Trader):
    def __init__(self, addr):
        Trader.__init__(self, addr=addr)

        self.qry_id = "1"

    def on_qryorder(self, msg):
        if msg['error_id'] != 0:
            print("query error: " + msg['error_id'])
            return

        qry_result = msg['data']['data']
        for data in qry_result:
            print('====================')
            print("status: " + str(data['status']))
            print("submit_status: " + str(data['submit_status']))
            print("amount: " + str(data['amount']))
            print("dealed_amount: " + str(data['dealed_amount']))
            print("order: " + str(data['order']))


def TestQueryOrder_CTP(trader):
    trader.query_order(
        qry_id=trader.qry_id,
        user_id="weidaizi",
        outside_id="",
        market="ctp",
        exchange="",
        type="",
        symbol="rb",
        contract="1901",
        contract_id="1901"
    )


def TestQueryOrder_XTP(trader):
    trader.query_order(
        qry_id=trader.qry_id,
        user_id="weidaizi",
        outside_id="",
        market="xtp",
        exchange="",
        type="",
        symbol="600519",
        contract="",
        contract_id=""
    )


if __name__ == '__main__':
    # ctp test
    # addr = "127.0.0.1:8001"
    # fn = TestQueryOrder_CTP

    # xtp test
    addr = "127.0.0.1:8002"
    fn = TestQueryOrder_XTP

    trader = TraderQueryOrder(addr)
    fn(trader)

    trader.message_loop()
