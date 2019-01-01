#!/bin/bash

killall babeltrader-okex-v3-trade

host=127.0.0.1
consul_host=127.0.0.1

nohup ./babeltrader-okex-v3-trade \
	-host=$host \
	-port=8005 \
	-consul.enable=true \
	-consul.host=$consul_host \
	-consul.port=8500 \
	-consul.service=babeltrader-okex-v3-trade \
	-consul.id=babeltrader-okex-v3-trade-1 \
	-consul.ttl=30s \
	&
