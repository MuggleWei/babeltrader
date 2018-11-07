import time

from trader import Trader


class TraderQueryTrade(Trader):
    def __init__(self):
        addr = "127.0.0.1:8001"
        Trader.__init__(self, addr=addr)

        self.qry_id = "2"

    def on_qrytrade(self, msg):
        if msg['error_id'] != 0:
            print("query error: " + msg['error_id'])
            return

        qry_result = msg['data']['data']
        for data in qry_result:
            print('====================')
            print("price: " + str(data['price']))
            print("amount: " + str(data['amount']))
            print("trading_day: " + str(data['trading_day']))
            print("trade_id: " + str(data['trade_id']))
            print("ts: " + str(data['ts']))
            print("order: " + str(data['order']))


if __name__ == '__main__':
    trader = TraderQueryTrade()

    ts = time.time()
    trader.query_trade(
        qry_id=trader.qry_id,
        user_id="weidaizi",
        trade_id="",
        market="ctp",
        exchange="SHFE",
        type="future",
        symbol="rb",
        contract="1901",
        contract_id="1901"
    )

    trader.message_loop()
