import json
import traceback
from datetime import datetime

import websocket


class Trader:
    def __init__(self, addr):
        self.rsp_callbacks = {}
        self.rsp_callbacks["confirmorder"] = self.on_confirmorder
        self.rsp_callbacks["orderstatus"] = self.on_orderstatus
        self.rsp_callbacks["orderdeal"] = self.on_orderdeal
        self.rsp_callbacks["error"] = self.on_error

        self.rsp_callbacks["rsp_qryorder"] = self.on_qryorder
        self.rsp_callbacks["rsp_qrytrade"] = self.on_qrytrade
        self.rsp_callbacks["rsp_qryposition"] = self.on_qryposition

        self.ws = websocket.create_connection("ws://" + addr + "/ws")

    def on_confirmorder(self, msg):
        pass

    def on_orderstatus(self, msg):
        pass

    def on_orderdeal(self, msg):
        pass

    def on_error(self, msg):
        print("exception happened: " + str(msg))

    def on_qryorder(self, msg):
        pass

    def on_qrytrade(self, msg):
        pass

    def on_qryposition(self, msg):
        pass

    def insert_order(self, user_id,
                     market, exchange, type, symbol, contract, contract_id,
                     order_type, order_flag1, dir, price, amount, total_price, ts):
        utc_ts = datetime.utcfromtimestamp(ts)
        ts_str = utc_ts.strftime("%Y%m%d-%H%M%S")

        order = json.dumps({
            "msg": "insert_order",
            "data": {
                "user_id": user_id,
                "order_id": str(user_id) + '-' + ts_str,
                "outside_id": "",
                "client_order_id": str(user_id) + '-' + ts_str,
                "market": market,
                "exchange": exchange,
                "type": type,
                "symbol": symbol,
                "contract": contract,
                "contract_id": contract_id,
                "order_type": order_type,
                "order_flag1": order_flag1,
                "dir": dir,
                "price": price,
                "amount": amount,
                "total_price": total_price,
                "ts": ts
            }
        })
        self.ws.send(order)
        print("send insert order: " + str(order))

    def cancel_order(self, user_id, outside_id,
                     market, exchange, type, symbol, contract, contract_id):
        order = json.dumps({
            "msg": "cancel_order",
            "data": {
                "user_id": user_id,
                "outside_id": outside_id,
                "market": market,
                "exchange": exchange,
                "type": type,
                "symbol": symbol,
                "contract": contract,
                "contract_id": contract_id
            }
        })
        self.ws.send(order)
        print("send cancel order: " + str(order))

    def query_order(self, qry_id, user_id, outside_id,
                    market, exchange, type, symbol, contract, contract_id):
        order = json.dumps({
            "msg": "query_order",
            "data": {
                "qry_id": qry_id,
                "user_id": user_id,
                "outside_id": outside_id,
                "market": market,
                "exchange": exchange,
                "type": type,
                "symbol": symbol,
                "contract": contract,
                "contract_id": contract_id
            }
        })
        self.ws.send(order)
        print("send query order: " + str(order))

    def query_trade(self, qry_id, user_id, trade_id,
                    market, exchange, type, symbol, contract, contract_id):
        order = json.dumps({
            "msg": "query_trade",
            "data": {
                "qry_id": qry_id,
                "user_id": user_id,
                "trade_id": trade_id,
                "market": market,
                "exchange": exchange,
                "type": type,
                "symbol": symbol,
                "contract": contract,
                "contract_id": contract_id
            }
        })
        self.ws.send(order)
        print("send query trade: " + str(order))

    def query_position(self, qry_id, user_id,
                       market, exchange, type, symbol, contract, contract_id):
        order = json.dumps({
            "msg": "query_position",
            "data": {
                "qry_id": qry_id,
                "user_id": user_id,
                "market": market,
                "exchange": exchange,
                "type": type,
                "symbol": symbol,
                "contract": contract,
                "contract_id": contract_id
            }
        })
        self.ws.send(order)
        print("send query position: " + str(order))

    def query_positiondetail(self, qry_id, user_id,
                             market, exchange, type, symbol, contract, contract_id):
        qry = json.dumps({
            "msg": "query_positiondetail",
            "data": {
                "qry_id": qry_id,
                "user_id": user_id,
                "market": market,
                "exchange": exchange,
                "type": type,
                "symbol": symbol,
                "contract": contract,
                "contract_id": contract_id
            }
        })
        self.ws.send(qry)
        print("send query position detail: " + str(qry))

    def query_tradeaccount(self, qry_id, user_id,
                           market, currency_id=""):
        qry = json.dumps({
            "msg": "query_tradeaccount",
            "data": {
                "qry_id": qry_id,
                "user_id": user_id,
                "market": market,
                "currency_id": currency_id
            }
        })
        self.ws.send(qry)
        print("send query trade account: " + str(qry))

    def query_product(self, qry_id, user_id,
                      market, exchange, type, symbol, contract):
        qry = json.dumps({
            "msg": "query_product",
            "data": {
                "qry_id": qry_id,
                "user_id": user_id,
                "market": market,
                "exchange": exchange,
                "type": type,
                "symbol": symbol,
                "contract": contract,
            }
        })
        self.ws.send(qry)
        print("send query product: " + str(qry))

    def message_loop(self):
        try:
            while True:
                result = self.ws.recv()
                print("receive message: " + result)
                result = json.loads(result)
                fn = self.rsp_callbacks.get(result['msg'])
                if fn:
                    fn(result)
        except Exception as e:
            print(traceback.format_exc())
        self.ws.close()
