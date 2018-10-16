# 行情 REST API

#### 1.查询所有订阅
method: Get
url: /topics/get
示例：
```
# Request
GET http://127.0.0.1:6888/topics/get

# Response
{
    "msg": "topics",
    "data": [
        {
            "market": "ctp",
            "exchange": "",
            "type": "future",
            "symbol": "rb",
            "contract": "1901",
            "contract_id": "1901",
            "info1": "marketdata",
            "info2": "",
            "subed": 1
        },
        ......
    ]
}
```
返回值说明:
```
market: 市场API - 例如: ctp, xtp, ib, bitmex, okex
exchange: 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex
type: 主题类型 - spot(现货), future(期货), option(期权)
symbol: 符号 - 例如: rb, CL, btc, btc_usdt
contract: 合约类型 - 例如: 1901, this_week
contract_id: 合约id - 例如: 1901, 20181901
info1: 主题信息 - ticker, depth, marketdata, kline
info2: 附加信息 - 1m, 1h
subed: 是否已经处于订阅状态
```
注意：
1. contract和contract_id的存在是因为某些虚拟货币交易所，以周或季度来区分合约，例如：okex的2018年12月季度合约为例，contract为quarter, contract_id为20181228，但是在传统交易所，这两个字段是冗余重复的
1. 返回字段中，一定不为空的是 market, type, symbol, 当为期货合约时，contract也保证不为空。其余字段，是否为空，取决于对应的市场

#### 2. 订阅/退订
method: Post
订阅：url: /topics/sub
退订：url: /topics/unsub
示例:
```
# Request
Post http://127.0.0.1:6888/topics/sub
Post http://127.0.0.1:6888/topics/unsub
{
    "market": "okex",
    "exchange": "okex",
    "type": "future",
    "symbol": "btc",
    "contract": "quarter",
    "contract_id": "",
    "info1": "kline",
    "info2": "1m",
}

# Response
{
    "err_id": 0,
    "err_msg": ""
}
```
返回值说明:
```
err_id: 错误编号, 0为操作成功
err_msg: 错误说明
```
注意：
1. 即使err_id为0，也不一定保证订阅成功，订阅的结果，由ws广播推送
1. BabelTrader的目标是作为上手服务，并没有打算在服务中实现订阅过滤分发，行情一律广播，策略的订阅过滤，应该由中间服务完成。所以最好在config中，配好需要的topic，而订阅与退订，只在合约换月/换季度时，由管理服务或手动进行。
1. 某些市场，kline是由BableTrader生成的，无法单独订阅或退订kline。例如：CTP中，只提供了marketdata，一旦订阅/退订了marketdata，会自动订阅/退订kline。
1. req中，必填项为 market, type, symbol, contract, 当不填info1时, 默认订阅此市场, 所有支持的类型
