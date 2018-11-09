# 交易 WS API

- [使用提示](#使用提示)  
- [连接](#交易连接)  
- [交易结构](#交易结构)  
    - [通用结构](#通用结构)  
    - [订单结构](#订单结构)  
    - [订单状态结构](#订单状态结构)
    - [订单成交结构](#订单成交结构)
    - [持仓结构](#持仓结构)
        - [持仓结构类型1 (CTP)](#持仓结构类型1)
        - [持仓结构类型2 (XTP)](#持仓结构类型2)
    - [持仓明细结构](#持仓明细结构)
        - [持仓明细结构类型1 (CTP)](#持仓明细结构类型1)
    - [交易账户结构](#交易账户结构)
        - [交易账户结构类型1 (CTP)](#交易账户结构类型1)
        - [交易账户结构类型2 (XTP)](#交易账户结构类型2)
    - [产品信息结构](#产品信息结构)
        - [产品信息结构类型1 (CTP)](#产品信息结构类型1)
- [请求指令](#请求指令)  
    - [下单](#下单)
    - [撤单](#撤单)
    - [查询订单](#查询订单)
    - [查询成交](#查询成交)
    - [查询持仓](#查询持仓)
    - [查询持仓明细](#查询持仓明细)
    - [查询交易账户](#查询交易账户)
    - [查询产品信息](#查询产品信息)
- [应答消息](#应答消息)
    - [上手确认订单接收](#上手确认订单接收)
    - [订单状态变更](#订单状态变更)
    - [订单成交](#订单成交)
    - [查询订单结果](#查询订单结果)
    - [查询成交结果](#查询成交结果)
    - [查询持仓结果](#查询持仓结果)
    - [查询持仓明细结果](#查询持仓明细结果)
    - [查询交易账户结果](#查询交易账户结果)
    - [查询产品信息结果](#查询产品信息结果)


## 使用提示
1. 由于babel-trader是作为上手服务，其中一些的字段，是为了直接转发方便(比如 user_id, market, type), 而对上手服务并没有太多的实际意义
1. 上手服务, 收到订单之后, 有 上手确认订单接收 消息, 用于关联上下级的订单id
1. 订单结构被用于很多消息, 但并不是每次每个字段都被填满, 取决于消息类型
1. 一般情况下, 查询 订单/成交/持仓/持仓明细/交易账户 只在上下级清算时使用, 不要让交易策略频繁的调用这几个接口。

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
    "outside_user_id":"104027",
    "outside_id": "104027_20181101_      274491",
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
outside_user_id(string): 上手交易账户id
outside_id(string): 上手订单唯一标识, 注意, 这里有可能是BabelTrader的拼凑结果
client_order_id(string): 客户订单标识, 由使用开发者系统的客户填入, 规则由开发者自定义
market(string): 市场API - 例如: ctp, xtp, ib, bitmex, okex
exchange(string): 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex (使用公认的交易所缩写)
type(string): 主题类型 - spot(现货), future(期货), option(期权)
symbol(string): 符号 - 例如: rb, CL, btc, btc_usdt
contract(string): 合约类型 - 例如: 1901, this_week
contract_id(string): 合约id - 例如: 1901, 20181901
order_type(string): 订单类型 - limit(限价单), market(市价单)
order_flag1(string): 订单标识 - speculation(投机), hedge(套保), arbitrage(套利), marketmaker(做市商)
dir(string): 订单方向 - buy(买), sell(卖), open_long(开多), open_short(开空), close_long(平多), close_short(平空), closetoday_long(平今多), closetoday_short(平今空), closehistory_long(平昨多), closehistory_short(平昨空), forceclose_long(强平多), forceclose_short(强平空)
price(double): 限价单价格, 当为市价单时, 此字段无效
amount(double/int): 开仓头寸大小
total_price(double): 共开多少价格, 此字段在某些币所的现货交易中有用到
ts(int64): 时间戳
```

注意:  
- user_id, market, type: 其实在上手当中, 并没有用到这些字段, 只是为了系统内的结构字段统一, 在这里写出来
- outside_id: 这个是上手的订单id, 下单时不填写
- type: 主题类型, 特别注意spot包括了 虚拟货币现货, 股票现货 的意义
- dir: 由 action 和 dir 构成, action为: buy, sell, open, close, closetoday, closehistory, dir为 long, short;  
期货交易: 通常使用 action_dir 的格式, 例如 open_long, closetoday_long  
虚拟货币现货, 股票, etf申赎, 新股申购: 通常单独使用 action, 例如 buy, sell
- price: 当使用市价单时, 请注意, 某些交易所(SHFE, okex合约)不支持市价单


#### 订单状态结构
示例:
```
{
    "status": 2,
    "submit_status": 1,
    "amount": 5,
    "dealed_amount": 5,
    "order": { 订单结构 }
}
```

字段说明:
```
status(int): 订单状态 - 0(未知), 1(部分成交), 2(完全成交), 3(已撤), 4(撤单中), 5(部分撤单), 6(拒单)
submit_status(int): 订单提交状态 - 0(未知), 1(已提交), 2(已接受), 3(已拒绝)
amount(int/double): 订单总数量
dealed_amount(int/double): 订单已成交量
```

#### 订单成交结构
示例:
```
{
    "price": 4194,
    "amount": 1,
    "trading_day": "20181030",
    "trade_id": "104027_20181030_97398",
    "ts": 1540880723000,
    "order": { 订单结构 }
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

#### 持仓结构
注意: 持仓结构根据上手的不同, 将会返回不同的类型, 要根据返回消息中的 position_summary_type 来判断data内是什么结构体

###### 持仓结构类型1
类型: type1
上手: CTP
示例:
```
{
    "market":"ctp",
    "outside_user_id":"104027",
    "exchange":"",
    "type":"",
    "symbol":"rb",
    "contract":"1901",
    "contract_id":"1901",
    "dir":"long",
    "order_flag1":"speculation",
    "date_type":"history",
    "amount":0.0,
    "closed_amount":2.0,
    "today_amount":0.0,
    "margin":0.0,
    "margin_rate_by_money": 0.1,
    "margin_rate_by_vol": 0.0,
    "long_frozen":0.0,
    "short_frozen":0.0,
    "frozen_margin":0.0,
    "trading_day":"20181105",
    "pre_settlement_price":4050.0,
    "settlement_price":4021.0,
    "open_cost":0.0,
    "position_cost":0.0,
    "position_profit":0.0,
    "close_profit_by_date":-650.0,
    "close_profit_by_trade":-900.0
}
```

字段说明:
```
market(string): 市场API - 例如: ctp, xtp, ib, bitmex, okex
outside_user_id(string): 上手交易账户id
exchange(string): 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex (使用公认的交易所缩写)
type(string): 主题类型 - spot(现货), future(期货), option(期权)
symbol(string): 符号 - 例如: rb, CL, btc, btc_usdt
contract(string): 合约类型 - 例如: 1901, this_week
contract_id(string): 合约id - 例如: 1901, 20181901
dir(string): 持仓方向 - long(多头), short(空头)
date_type(string): 持仓的日期类型 - today(今仓), history(昨仓)
amount(double/int): 当前仓位
closed_amount(double/int): 已平仓位
today_amount(double/int): 今仓数量
margin(double): 保证金
margin_rate_by_money(double): 保证金率
margin_rate_by_vol(double): 保证金率(按手数)
long_frozen(double/int): 多头冻结(未成交的单)
short_frozen(double/int): 空头冻结(未成交的单)
frozen_margin(double): 冻结占用保证金
trading_day(string): 交易日
pre_settlement_price(double): 昨日结算价格
settelment_price(double): 结算价
open_cost(double): 开仓成本
position_cost(double): 持仓成本
position_profit(double): 持仓盈亏
close_profit_by_date(double): 盯市平仓盈亏
close_profit_by_trade(double): 逐笔平仓盈亏
```

###### 持仓结构类型2
类型: type2
上手: XTP
示例:
```
{
    "market":"xtp",
    "outside_user_id":"15033731",
    "exchange":"",
    "type":"",
    "symbol":"",
    "dir":"net",
    "amount":100.0,
    "avaliable_amount":0.0,
    "avg_price":580.0,
    "unrealized_profit":0.0,
    "purchase_redeemable_qty":100.0,
    "executable_option":0,
    "lockable_position":0,
    "executable_underlying":0,
    "locked_position":0,
    "usable_locked_position":0
}
```
字段说明:
```
market(string): 市场API - 例如: ctp, xtp, ib, bitmex, okex
outside_user_id(string): 上手交易账户id
exchange(string): 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex (使用公认的交易所缩写)
type(string): 主题类型 - spot(现货), future(期货), option(期权)
symbol(string): 符号 - 例如: rb, CL, btc, btc_usdt
dir(string): 持仓方向 - long(多头), short(空头)
amount(double/int): 当前仓位
avaliable_amount(double/int): 可平仓位
avg_price(double): 开仓均价
unrealized_profit(double): 未实现盈亏
executable_option(int): 可行权合约
lockable_position(int): 可锁定标的
executable_underlying(int): 可行权标的
locked_position(int): 已锁定标的
usable_locked_position(int): 可用已锁定标的
```

#### 持仓明细结构
注意: 持仓结构根据上手的不同, 将会返回不同的类型, 要根据返回的position_detail_type来判断data内是什么结构体

###### 持仓明细结构类型1
类型: type1
上手: CTP
示例:
```
{
    "market":"ctp",
    "outside_user_id":"104027",
    "exchange":"SHFE",
    "type":"",
    "symbol":"rb",
    "contract":"1901",
    "contract_id":"1901",
    "dir":"long",
    "order_flag1":"speculation",
    "open_date":"20181102",
    "trading_day":"20181105",
    "trade_id":"104027_20181105_      150645",
    "amount":0.0,
    "closed_amount":1.0,
    "closed_money":40140.0,
    "pre_settlement_price":4050.0,
    "settlement_price":4014.0,
    "open_price":4062.0,
    "margin":0.0,
    "margin_rate_by_money":0.1,
    "margin_rate_by_vol":0.0,
    "close_profit_by_date":-360.0,
    "close_profit_by_trade":-480.0,
    "position_profit_by_date":0.0,
    "position_profit_by_trade":0.0
}
```

字段说明:
```
market(string): 市场API - 例如: ctp, xtp, ib, bitmex, okex
outside_user_id(string): 上手交易账户id
exchange(string): 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex (使用公认的交易所缩写)
type(string): 主题类型 - spot(现货), future(期货), option(期权)
symbol(string): 符号 - 例如: rb, CL, btc, btc_usdt
contract(string): 合约类型 - 例如: 1901, this_week
contract_id(string): 合约id - 例如: 1901, 20181901
dir(string): 持仓方向 - long(多头), short(空头)
order_flag1(string): 订单标识 - speculation(投机), hedge(套保), arbitrage(套利), marketmaker(做市商)
open_date(string): 开仓日期
trading_day(string): 当前交易日
trade_id(string): 成交编号
amount(double): 当前持仓数量
closed_amount(double): 已平仓数量
closed_money(double): 已平仓金额
pre_settlement_price(double): 昨日结算价
settlement_price(double): 结算价
open_price(double): 开仓价
margin(double): 保证金
margin_rate_by_money(double): 保证金率
margin_rate_by_vol(double): 保证金率(按手数)
close_profit_by_date(double): 平仓盈亏(盯市)
close_profit_by_trade(double): 平仓盈亏(逐笔)
position_profit_by_date(double): 持仓盈亏(盯市)
position_profit_by_trade(double): 持仓盈亏(逐笔)
```

#### 交易账户结构
注意: 资金账户结构根据上手的不同, 将会返回不同的类型, 要根据返回的trade_account_type来判断data内是什么结构体

###### 交易账户结构类型1
类型: type1
上手: CTP
示例:
```
{
    "market": "ctp",
    "outside_user_id":"104027",
    "pre_credit":0.0,
    "pre_balance":1000094.71,
    "pre_margin":8100.0,
    "interest":0.0,
    "deposit":0.0,
    "withdraw":0.0,
    "credit":0.0,
    "margin":8100.0,
    "commission":16.095,
    "close_profit":-650.0,
    "position_profit":-840.0,
    "frozen_margin":0.0,
    "frozen_cash":0.0,
    "frozen_commision":0.0,
    "balance":998588.615,
    "available":990488.615,
    "currency_id":"CNY",
    "trading_day":"20181105"
}
```

字段说明:
```
market(string): 交易市场API
outside_user_id(string): 上手交易账户id
pre_credit(double): 上次信用额度
pre_balance(double): 上次结算准备金
pre_margin(double): 上次占用的保证金
interest(double): 利息收入
deposit(double): 入金金额
withdraw(double): 出金金额
credit(double): 信用额度
margin(double): 当前保证金总额
commission(double): 手续费
close_profit(double): 平仓盈亏
position_profit(double): 持仓盈亏
frozen_margin(double): 冻结保证金
frozen_cash(double): 冻结资金
frozen_commision(double): 冻结手续费
balance(double): 结算准备金
available(double): 可用资金
currency_id(string): 币种代码
trading_day(string): 交易日
```

###### 交易账户结构类型2
类型: type2
上手: XTP
示例:
```
{
    "market":"xtp",
    "outside_user_id":"15033731",
    "account_type":"normal",
    "total_asset":999941826.0,
    "available_cash":999941826.0,
    "securities_asset":0.0,
    "fund_buy_amount":58000.0,
    "fund_buy_fee":174.0,
    "fund_sell_amount":0.0,
    "fund_sell_fee":0.0,
    "withholding_amount":0.0,
    "frozen_margin":0.0,
    "frozen_exec_cash":0.0,
    "frozen_exec_fee":0.0,
    "pay_later":0.0,
    "preadva_pay":0.0,
    "orig_banlance":0.0,
    "banlance":0.0,
    "deposit_withdraw":0.0,
    "trade_netting":0.0,
    "captial_asset":0.0,
    "force_freeze_amount":0.0,
    "preferred_amount":0.0
}
```

字段说明:
```
market(string): 交易市场API
outside_user_id(string): 上手交易账户id
account_type(string): 账户类型 - normal(普通账户), credit(信用账户), derivatives(衍生品账户)
total_asset(double): 总资产 (资金 + 证券资产 + 预扣资金)
available_cash(double): 可用资金
securities_asset(double): 证券资产
fund_buy_amount(double): 累计买入成交证券占用资金
fund_buy_fee(double): 累计买入成交交易费用
fund_sell_amount(double): 累计卖出成交证券所得资金
fund_sell_fee(double): 累计卖出成交交易费用
withholding_amount(double): P系统预扣的资金
frozen_margin(double): 冻结的保证金
frozen_exec_cash(double): 行权冻结资金
frozen_exec_fee(double): 行权费用
pay_later(double): 垫付资金
preadva_pay(double): 预垫付资金
orig_banlance(double): 昨日余额
banlance(double): 当前余额
deposit_withdraw(double): 当天出入金
trade_netting(double): 当日交易资金轧差
captial_asset(double): 资金资产
force_freeze_amount(double): 强锁资金
preferred_amount(double): 可取资金
```

#### 产品信息结构
注意: 资金账户结构根据上手的不同, 将会返回不同的类型, 要根据返回的product_type来判断data内是什么结构体

###### 产品信息结构类型1
类型: type1
上手: CTP
示例:
```
{
    "market":"ctp",
    "outside_user_id":"104027",
    "exchange":"SHFE",
    "type":"future",
    "symbol":"rb",
    "contract":"1901",
    "contract_id":"1901",
    "vol_multiple":10.0,
    "price_tick":1.0,
    "long_margin_ratio":0.1,
    "short_margin_ratio":0.1
}
```

字段说明:
```
market(string): 交易市场API
outside_user_id(string): 上手交易账户id
exchange(string): 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex (使用公认的交易所缩写)
type(string): 主题类型 - spot(现货), future(期货), option(期权)
symbol(string): 符号 - 例如: rb, CL, btc, btc_usdt
contract(string): 合约类型 - 例如: 1901, this_week
contract_id(string): 合约id - 例如: 1901, 20181901
vol_multiple(double): 合约乘数
price_tick(double): 最小变动价
long_margin_ratio(double): 多头保证金比例 (当contract为空时, 此字段无效)
short_margin_ratio(double): 空头保证金比例 (当contract为空时, 此字段无效)
```

## 请求指令
#### 下单
消息名: insert_order  

示例:
```
{
    "msg": "insert_order",
    "data": { 订单结构 }
}
```


#### 撤单
消息名: cancel_order  

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
exchange(string): 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex (使用公认的交易所缩写)
type(string): 主题类型 - spot(现货), future(期货), option(期权)
symbol(string): 符号 - 例如: rb, CL, btc, btc_usdt
contract(string): 合约类型 - 例如: 1901, this_week
contract_id(string): 合约id - 例如: 1901, 20181901
```


#### 查询订单
消息名: query_order  

示例:
```
{
    "msg": "query_order",
    "data": {
        "qry_id": "1",
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
exchange(string): 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex (使用公认的交易所缩写)
type(string): 主题类型 - spot(现货), future(期货), option(期权)
symbol(string): 符号 - 例如: rb, CL, btc, btc_usdt
contract(string): 合约类型 - 例如: 1901, this_week
contract_id(string): 合约id - 例如: 1901, 20181901
```

说明:   
- 当指定了outside_id, 就只查询指定的订单; 没有指定订单信息时, 默认查回整个当日所有符合条件的订单

#### 查询成交
消息名: query_trade

示例:
```
{
    "msg": "query_trade",
    "data": {
        "qry_id": "1",
        "trade_id": "104027_20181101_      118101",
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
trade_id(string): 上手成交号, 在成交消息中得到
market(string): 市场API - 例如: ctp, xtp, ib, bitmex, okex
exchange(string): 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex (使用公认的交易所缩写)
type(string): 主题类型 - spot(现货), future(期货), option(期权)
symbol(string): 符号 - 例如: rb, CL, btc, btc_usdt
contract(string): 合约类型 - 例如: 1901, this_week
contract_id(string): 合约id - 例如: 1901, 20181901
```

#### 查询持仓
消息名: query_position  

示例:
```
{
    "msg": "query_position",
    "data": {
        "qry_id": "1",
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
market(string): 市场API - 例如: ctp, xtp, ib, bitmex, okex
exchange(string): 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex (使用公认的交易所缩写)
type(string): 主题类型 - spot(现货), future(期货), option(期权)
symbol(string): 符号 - 例如: rb, CL, btc, btc_usdt
contract(string): 合约类型 - 例如: 1901, this_week
contract_id(string): 合约id - 例如: 1901, 20181901
```

#### 查询持仓明细
消息名: query_positiondetail  

示例:
```
{
    "msg": "query_positiondetail",
    "data": {
        "qry_id": "1",
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
market(string): 市场API - 例如: ctp, xtp, ib, bitmex, okex
exchange(string): 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex (使用公认的交易所缩写)
type(string): 主题类型 - spot(现货), future(期货), option(期权)
symbol(string): 符号 - 例如: rb, CL, btc, btc_usdt
contract(string): 合约类型 - 例如: 1901, this_week
contract_id(string): 合约id - 例如: 1901, 20181901
```

#### 查询交易账户
消息名: query_tradeaccount  

示例:
```
{
    "msg": "query_tradeaccount",
    "data": {
        "qry_id": "3",
        "market": "ctp",
        "currency_id": ""
    }
}
```

字段说明:
```
qry_id(string): 查询请求号
market(string): 市场API
currency_id(string): 币种代码 (不填会根据对应的市场取默认值)
```

#### 查询产品信息
消息名: query_product

示例:
```
{
    "msg": "query_product",
    "data": {
        "qry_id": "6",
        "market": "ctp",
        "exchange": "SHFE",
        "type": "future",
        "symbol": "rb",
        "contract": "1901"
    }
}
```

字段说明:
```
qry_id(string): 查询请求号
market(string): 市场API
exchange(string): 交易所 - 例如：SHFE, SSE, NYMEX, bitmex, okex (使用公认的交易所缩写)
type(string): 主题类型 - spot(现货), future(期货), option(期权)
symbol(string): 符号 - 例如: rb, CL, btc, btc_usdt
contract(string): 合约类型 - 例如: 1901, this_week
```

说明:
如果是期货品种, 当contract为空时，查的品种信息, 当contract部位空时, 查的是合约信息


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
    "data": { 订单状态结构 }
}
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
    "data": { 订单成交结构 }
    }
}
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
            { 订单状态结构 },
            { 订单状态结构 },
            ......
        ]
    }
}
```

#### 查询成交结果
消息名: rsp_qrytrade 

示例:
```
{
    "msg": "rsp_qrytrade",
    "error_id": 0,
    "data": {
        "qry_id":"2",
        "user_id":"weidaizi",
        "trade_id":"",
        "market":"ctp",
        "exchange":"SHFE",
        "type":"future",
        "symbol":"rb",
        "contract":"1901",
        "contract_id":"1901",
        "data":[
            { 订单成交结构 },
            { 订单成交结构 },
            ......
        ]
    }
}
```

#### 查询持仓结果
消息名: rsp_qryposition

示例:
```
{
    "msg":"rsp_qryposition",
    "error_id":0,
    "data":{
        "qry_id":"3",
        "user_id":"weidaizi",
        "market":"ctp",
        "exchange":"SHFE",
        "type":"future",
        "symbol":"rb",
        "contract":"1901",
        "contract_id":"1901",
        "position_summary_type":"type1",
        "data":
        [
            { 持仓结构 },
            { 持仓结构 },
            ......
        ]
    }
}
```

#### 查询持仓明细结果
消息名: rsp_qrypositiondetail

```
{
    "msg":"rsp_qrypositiondetail",
    "error_id":0,
    "data":
    {
        "qry_id":"",
        "user_id":"",
        "market":"",
        "exchange":"",
        "type":"",
        "symbol":"",
        "contract":"",
        "contract_id":"",
        "position_detail_type":"type1",
        "data":
        [
            { 持仓明细结构 },
            { 持仓明细结构 },
            ......
        ]
    }
}
```

#### 查询交易账户结果
消息名: rsp_qrytradeaccount

示例:
```
{
    "msg":"rsp_qrytradeaccount",
    "error_id":0,
    "data":
    {
        "qry_id":"3",
        "user_id":"",
        "market":"",
        "currency_id":"",
        "trade_account_type":"type1",
        "data":
        [
            { 交易账户结果 },
            { 交易账户结果 },
            ......
        ]
    }
}
```

#### 查询产品信息结果
消息名: rsp_qryproduct

示例:
```
{
    "msg":"rsp_qryproduct",
    "error_id":0,
    "data":
    {
        "qry_id":"2",
        "market":"ctp",
        "exchange":"SHFE",
        "symbol":"rb",
        "contract":"1901",
        "product_type":"type1",
        "data":
        [
            { 产品信息 },
            { 产品信息 },
            ......
        ]
    }
}
```