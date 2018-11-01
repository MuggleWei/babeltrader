# 交易 WS API

- [连接](#交易连接)  
- [交易结构](#交易结构)  
    - [通用结构](#通用结构)  
    - [订单结构](#订单结构)  
- [请求指令](#请求指令)  
    - [下单](#下单)
    - [撤单](#撤单)
    - [查询订单](#查询订单)
- [应答消息](#应答消息)
    - [上手确认订单接收](#上手确认订单接收)
    - [订单状态变更](#订单状态变更)
    - [订单成交](#订单成交)
    - [查询订单结果](#查询订单结果)

## 交易连接
url: /ws
示例:
```
ws://127.0.0.1:8001/ws
```

## 交易结构
#### 通用结构
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
order_flag1(string): 订单标识 - speculation(投机), hedge(套保), arbitrage(套利), marketmaker(做市商)
dir(string): 订单方向 - buy(买), sell(卖), open_long(开多), open_short(开空), close_long(平多), close_short(平空), closetoday_long(平今多), closetoday_short(平今空), closeyesterday_long(平昨多), closeyesterday_short(平昨空)
price(double): 限价单价格, 当为市价单时, 此字段无效
amount(double/int): 开仓头寸大小
total_price(double): 共开多少价格, 此字段在某些币所的现货交易中有用到
ts(int64): 时间戳
```


## 请求指令
#### 下单
示例:
```
{
    "msg": "insert_order",
    "data": { 订单结构 }
}
```


#### 撤单
示例:
```
{
    "msg": "cancel_order",
    "data": {
        "user_id": "weidaizi",
        "outside_id": "104027_20181031_      256846",
        "market": "ctp",
        "exchange": "SHFE",
        "type": "future",
        "symbol": "rb",
        "contract": "1901",
        "contract_id": "1901"
    }
}
```

字段说明:
```
user_id(string): 用户标识符, 在下单时填入
outside_id(string): 上手订单唯一标识, 在confirmorder中得到
market(string): 市场API - 例如: ctp, xtp, ib, bitmex, okex
exchange(string): 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex
type(string): 主题类型 - spot(现货), future(期货), option(期权)
symbol(string): 符号 - 例如: rb, CL, btc, btc_usdt
contract(string): 合约类型 - 例如: 1901, this_week
contract_id(string): 合约id - 例如: 1901, 20181901
```


#### 查询订单
示例:
```
{
    "msg": "query_order",
    "data": {
        "qry_id": "1",
        "user_id": "weidaizi",
        "outside_id": "104027_20181031_      256846",
        "market": "ctp",
        "exchange": "SHFE",
        "type": "future",
        "symbol": "rb",
        "contract": "1901",
        "contract_id": "1901"
    }
}
```

字段说明:
```
qry_id(string): 查询请求号
user_id(string): 用户标识符, 在下单时填入
outside_id(string): 上手订单唯一标识, 在confirmorder中得到
market(string): 市场API - 例如: ctp, xtp, ib, bitmex, okex
exchange(string): 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex
type(string): 主题类型 - spot(现货), future(期货), option(期权)
symbol(string): 符号 - 例如: rb, CL, btc, btc_usdt
contract(string): 合约类型 - 例如: 1901, this_week
contract_id(string): 合约id - 例如: 1901, 20181901
```

说明:   
当指定了outside_id, 就只查询指定的订单; 没有指定订单信息时, 默认查回整个当日所有符合条件的订单


## 应答消息
#### 上手确认订单接收
消息名: confirmorder  

示例:
```
{
    "msg": "confirmorder",
    "error_id": 0,
    "data": { 订单结构 }
}
```

说明:
此消息代表上手已接收订单，使用此消息，关联上手订单号(outside_id)和系统内订单号, 当上手拒单或者出现错误时, 此结构中的outside_id会为空。

#### 订单状态变更
消息名: orderstatus  

示例:
```
{
    "msg": "orderstatus",
    "error_id": 0,
    "data": {
        "status": 2,
        "submit_status": 1,
        "amount": 5,
        "dealed_amount": 5,
        "order": { 订单结构 }
    }
}
```

字段说明:
```
status(int): 订单状态 - 0(未知), 1(部分成交), 2(完全成交), 3(已撤), 4(撤单中)
submit_status(int): 订单提交状态 - 0(未知), 1(已提交), 2(已接受), 3(已拒绝)
amount(int/double): 订单总数量
dealed_amount(int/double): 订单已成交量
```

说明:
使用此消息时, 需要先检查 error_id 和 status, 当有错误或是订单遭到拒绝时, 此结构的order中, 会有 user_id, order_id, client_order_id。 当订单顺利进行时, 此结构体的order中, 只会有 outside_id。 上手订单号和系统内订单号的关联工作, 需要在收到confirmorder时进行

#### 订单成交
消息名: orderdeal  

示例:
```
{
    "msg": "orderstatus",
    "error_id": 0,
    "data": {
        "price": 4194,
        "amount": 1,
        "trading_day": "20181030",
        "trade_id": "104027_20181030_97398",
        "ts": 1540880723000,
        "order": { 订单结构 }
    }
}
```

字段说明:
```
price(double): 单笔成交价格
amount(int/double): 单笔成交量
trading_day(string): 交易日
trade_id(string): 成交id
ts(int64): 成交时间戳
```

说明:  
返回的订单结构中, 只会有市场, 交易所, 品种, 方向等字段, 下单的价格并不会有


#### 查询订单结果
消息名: rsp_qryorder 

示例:
```
{
    "msg": "rsp_qryorder",
    "error_id": 0,
    "data": {
        "qry_id":"1",
        "user_id":"weidaizi",
        "outside_id":"",
        "market":"ctp",
        "exchange":"SHFE",
        "type":"future",
        "symbol":"rb",
        "contract":"1901",
        "contract_id":"1901"
        "data":[
            {
                "status":3,
                "submit_status":2,
                "amount":1,
                "dealed_amount":0
                "order":{
                    "user_id":"",
                    "order_id":"",
                    "outside_id":"104027_20181031_      342074",
                    "client_order_id":"",
                    "market":"ctp",
                    "exchange":"SHFE",
                    "type":"future",
                    "symbol":"rb",
                    "contract":"1901",
                    "contract_id":"1901",
                    "order_type":"limit",
                    "order_flag1":"speculation",
                    "dir":"open_long",
                    "price":4100.0,
                    "amount":1,
                    "total_price":0.0,
                    "ts":1540968164000.0
                }
            },
            ......
        ]
    }
}
```