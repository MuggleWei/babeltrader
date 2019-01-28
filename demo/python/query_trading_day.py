import threading
import time

from trader import Trader


class TimerTask:
    def __init__(self):
        self.trader = None

    def set_trader(self, trader):
        self.trader = trader

    def time_task(self):
        while True:
            time.sleep(3)
            if self.trader is not None:
                self.trader.time_task()


class TraderQueryTradingDay(Trader):
    def __init__(self, addr):
        Trader.__init__(self, addr=addr)
        self.trading_day = ""

    def time_task(self):
        if len(self.trading_day) == 0:
            self.query_tradingday(qry_id="", market="ctp")

    def on_qrytradingday(self, msg):
        self.trading_day = msg['data']['trading_day']
        print(self.trading_day)


if __name__ == '__main__':
    # ctp test
    addr = "127.0.0.1:8001"

    ts = TimerTask()
    th = threading.Thread(target=ts.time_task)
    th.start()

    while True:
        try:
            trader = TraderQueryTradingDay(addr)
            ts.set_trader(trader=trader)
            trader.message_loop()
        except:
            print("connect failed, try reconnect...")
