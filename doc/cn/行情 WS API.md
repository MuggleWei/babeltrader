# 行情 WS API

- [连接](#行情连接) 
- [注意事项](#推送注意事项) 
- [行情结构](#行情结构) 
    - [通用结构](#通用结构)
    - [marketdata](#marketdata)
    - [kline](#kline)
    - [orderbook](#orderbook)
    - [level2](#level2)
    - [depth](#depth)
    - [ticker](#ticker)
    

## 行情连接
url: /ws
示例:
```
ws://127.0.0.1:6001/ws
```

## 推送注意事项
BabelTrade的设计目的是, 统一上手API接口, 并不做订阅过滤分发和账户权限验证, 行情一律广播推送, 合理的设计, 应该只有内网的中间层能够对接BabelTrader行情接口。想要对订阅的行情做管理, 参照 REST API文档, 或者直接通过配置文件管理。

## 行情结构
#### 通用结构
示例:
```
{
    "msg": "quote",
    "data": {
        "market":"ctp",
        "exchange":"",
        "type":"future",
        "symbol":"rb",
        "contract":"1901",
        "contract_id":"1901",
        "info1":"marketdata",
        "info2":"",
        "data": ...
    }
}
```
字段说明:
```
msg(string): 标识消息类型, 所有行情消息, 此字段都为 quote
market(string): 市场API - 例如: ctp, xtp, ib, bitmex, okex
exchange(string): 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex (使用公认的交易所缩写)
type(string): 主题类型 - spot(现货), future(期货), option(期权)
symbol(string): 符号 - 例如: rb, CL, btc, btc_usdt
contract(string): 合约类型 - 例如: 1901, this_week
contract_id(string): 合约id - 例如: 1901, 20181901
info1(string): 主题信息 - ticker, depth, marketdata, kline
info2(string): 附加信息 - 例如: 1m, 1h
data: 根据info1, 对应不同的类型
```

#### marketdata
说明:
通常在传统的二级市场API中, 推送此类型数据, 其包括了最新价和深度, 以及一些附加的市场信息

示例:
```
{
    "ts":1539755434000,
    "last":4181.0,
    "bids":[[4180.0,990], ...],
    "asks":[[4181.0,13], ...],
    "vol":3188636.0,
    "turnover":13328498480.0,
    "avg_price":4180.0,
    "pre_settlement":4137.0,
    "pre_close":4117.0,
    "pre_open_interest":2692326.0,
    "settlement":1.7976931348623157e308,
    "close":1.7976931348623157e308,
    "open_interest":2848774.0,
    "upper_limit":4426.0,
    "lower_limit":3847.0,
    "open":4176.0,
    "high":4198.0,
    "low":4162.0,
    "trading_day":"20181017",
    "action_day":"20181017"
}
```

字段说明:
```
ts(long): 时间戳, 毫秒为单位
last(double): 最新价格
bids(array): 价,量, 按顺序为买1, 买2 ... 买n, 注意使用时需要判断vol的数量, 为0时, 价格为无效数据
asks(array): 价,量, 按顺序为卖1, 卖2 ... 卖n, 注意使用时需要判断vol的数量, 为0时, 价格为无效数据
vol(double): 本交易日的累计量
turnover(double): 本交易日的成交金额
avg_price(double): 本交易日的均价
pre_settlement(double): 前一交易日结算价
pre_close(double): 前一交易日收盘价
pre_open_interest(double): 前一交易日未平仓量
settlement(double): 当前交易日结算价
close(double): 当前交易日收盘价
open_interest(double): 当前交易日未平仓量
upper_limit(double): 涨停价
lower_limit(double): 跌停价
open(double):开盘价
high(double):当前交易日最高价
low(double):当前交易日最低价
trading_day(string): 交易日
action_day(string): 日期, 注意, 这里不一定是utc时间, 可能是当地时间, CTP使用的就是东八区时间
```

#### kline
说明:
k线数据, 当k线的最高价或最低价发送变化, 或是出现了新的k线时, 才推送此数据

示例:
```
{
    "ts":1539755430000,
    "open":4181.0,
    "high":4182.0,
    "low":4180.0,
    "close":4180.0,
    "vol":3460.0
}
```

字段说明:
```
ts(long): 时间戳, 毫秒为单位
open(double): 开盘价
high(double): 最高价
low(double): 最低价
close(double): 收盘价
vol(double): 量
```

注意:
在传统的交易API中, 通常并没有提供k线数据的推送, 此时的k线数据, 由BabelTrader生成, 只提供1分钟级别的推送


#### orderbook
示例:
```
{
    "ts":1539755434000,
    "last":4181.0,
    "vol":3188636.0,
    "bids":[[4180.0,990], ...],
    "asks":[[4181.0,13], ...]
}
```

字段说明:
```
ts(long): 时间戳, 毫秒为单位
last(double): 最新价格
vol(double): 本交易日的累计量
bids(array): 价,量, 按顺序为买1, 买2 ... 买n, 注意使用时需要判断vol的数量, 为0时, 价格为无效数据
asks(array): 价,量, 按顺序为卖1, 卖2 ... 卖n, 注意使用时需要判断vol的数量, 为0时, 价格为无效数据
```

#### level2
说明: 中国股票市场的level2逐笔数据

示例:
```
{
    "ts":1535439099100,
    "action":"trade",
    "data":{
        "channel_no":1,
        "seq":1262183,
        "price":15.950000000000001,
        "vol":400.0,
        "bid_no":2095191,
        "ask_no":2088161,
        "trade_flag":"buy"
    }
}
```

字段说明:
```
ts(long): 时间戳, 毫秒为单位
action(string): 逐笔信息, data中的字段, 根据此决定  trade - 交易, entrust - 委托

trade:
channel_no(long): 频道
seq(long): 序号
price(double): 成交价格
vol(double): 成交量
bid_no(long): 买方订单号
ask_no(long): 卖方订单号
trade_flag(string): 交易标志位 - buy(主动买), sell(主动卖), cancel(撤单), deal(成交)

entrust:
channel_no(long): 频道
seq(long): 序号
price(double): 委托价格
vol(double): 委托量
dir(string): 方向 - buy(主动买), sell(主动卖), borrow(借入), lend(借出)
order_type(string): 订单类型 - market(市价单), limit(限价单), best(本方最优)
```

#### depth
说明:
深度信息, 通常在虚拟货币交易所中, 分开推送depth和ticker的变化, 而不提供marketdata

示例:
```
{
    "ts": 1534290646254,
    "bids":[[6081.13,162], ...],
    "asks":[[6083.13,30], ...]
}
```

字段说明:
```
ts(long): 时间戳, 毫秒为单位
bids(array): 价,量, 按顺序为买1, 买2 ... 买n, 注意使用时需要判断vol的数量, 为0时, 价格为无效数据
asks(array): 价,量, 按顺序为卖1, 卖2 ... 卖n, 注意使用时需要判断vol的数量, 为0时, 价格为无效数据
```

#### ticker
说明:
基础的行情推送, 通常在虚拟货币交易所, 有此类数据推送

示例:
```
{
    "ts": 1534290647226,
    "bid": 6083.01,
    "ask": 6083.13,
    "last": 6083.01,
    "high": 6180,
    "low": 5755,
    "vol": 6666
}
```

字段说明:
```
ts(long): 时间戳, 毫秒为单位
bid(double): 买一价
ask(double): 卖一价
last(double): 最新价
high(double): 24小时最高价
low(double): 24小时最低价
vol(double): 24小时成交量
```