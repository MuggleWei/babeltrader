# 交易 WS API

## 交易连接
url: /ws
示例:
```
ws://127.0.0.1:8001/ws
```

## 交易结构:
#### 通用结构:
示例:
```
{
    "msg": "",
    "data": {}
}
```
字段说明:
```
msg(string): 标识消息类型,
data: 根据msg, 对应不同的类型
```


#### 订单结构
示例:
```
{
    "user_id": "daizi",
    "order_id": "20181026160928625-56",
    "outside_id": "201810260003s-868766042#1",
    "client_order_id": "daizi-20181026000001",
    "market": "ctp",
    "exchange": "SHFE",
    "type": "future",
    "symbol": "rb",
    "contract": "1901",
    "contract_id": "1901",
    "order_type": "limit",
    "order_flag1": "speculation",
    "dir": "open_long",
    "price": 4300.0,
    "amount": 1,
    "total_price": 0,
    "ts": 1540533355168,
}
```
字段说明:
```
user_id(string): 用户标识符, 在下单时填入
order_id(string): 系统内部订单标识, 下单时填入, 规则由开发者自定义
outside_id(string): 上手订单唯一标识, 注意, 这里有可能是BabelTrader的拼凑结果
client_order_id(string): 客户订单标识, 由使用开发者系统的客户填入, 规则由开发者自定义
market(string): 市场API - 例如: ctp, xtp, ib, bitmex, okex
exchange(string): 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex
type(string): 主题类型 - spot(现货), future(期货), option(期权)
symbol(string): 符号 - 例如: rb, CL, btc, btc_usdt
contract(string): 合约类型 - 例如: 1901, this_week
contract_id(string): 合约id - 例如: 1901, 20181901
order_type(string): 订单类型 - limit(限价单), market(市价单)
order_flag1(string): 订单标识 - speculation(投机), hedge(套保), arbitrage(套利)
dir(string): 订单方向 - buy(买), sell(卖), open_long(开多), open_short(开空), close_long(平多), close_short(平空), closetoday_long(平今多), closetoday_short(平今空), closeyesterday_long(平昨多), closeyesterday_short(平昨空)
price(double): 限价单价格, 当为市价单时, 此字段无效
amount(double/int): 开仓头寸大小
total_price(double): 共开多少价格, 此字段在某些币所的现货交易中有用到
ts(int64): 时间戳
```


## 交易指令
#### 下单
示例:
```
{
    "msg": "insertorder",
    "data": { 订单结构 }
}
```