#### CTP 配置文件

例子: 参考 config-template 中的 ctp_conf.json 文件

字段说明:
```
broker_id: 对应CTP要求的BrokerID
user_id: 期货账户
password: 密码
trade_addr: 交易前置机地址
quote_addr: 行情前置机地址
trade_listen_ip: BabelTrader-CTP-Trade 服务监听的IP地址
trade_listen_port: BabelTrader-CTP-Trade 服务监听的端口号
quote_listen_ip: BabelTrader-CTP-Quote 服务监听的IP地址
quote_listen_port: BabelTrader-CTP-Quote 服务监听的端口号
default_sub_topics: 默认订阅的行情
product_info: 对应CTP ReqAuthenticate 中的 UserProductInfo 字段
auth_code: 对应CTP ReqAuthenticate 中的 AuthCode 字段
```