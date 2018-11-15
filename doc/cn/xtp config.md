#### XTP 配置文件

例子: 参考 config-template 中的 xtp_conf.json 文件

字段说明:
```
custom_client_id: 自定义的客户id
quote_protocol: BabelTrader-XTP-Trade使用的协议, 1 - TCP, 2 - UDP
trade_protocol: BabelTrader-XTP-Quote使用的协议, 1 - TCP, 2 - UDP
user_id: 账户
password: 密码
trade_addr: 交易前置机地址
quote_addr: 行情前置机地址
trade_listen_ip: BabelTrader-XTP-Trade 服务监听的IP地址
trade_listen_port: BabelTrader-XTP-Trade 服务监听的端口号
quote_listen_ip: BabelTrader-XTP-Quote 服务监听的IP地址
quote_listen_port: BabelTrader-XTP-Quote 服务监听的端口号
sub_all: 是否订阅全市场行情, 0 - 否, 1 - 是(若为是, 则default_sub_topics字段无效, xtp的外围测试环境不支持全市场订阅)
sub_orderbook: 是否订阅orderbook 0 - 否, 1 - 是 (默认只订阅marketdata)
sub_Level2: 是否订阅level2逐笔 0 - 否, 1 - 是 (默认只订阅marketdata, 注意, 不要同时订阅全市场的level2行情, 当前的推送效率无法承担)
default_sub_topics: 默认订阅的行情
```
