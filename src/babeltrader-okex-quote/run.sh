#!/bin/bash

killall babeltrader-okex-quote

host=127.0.0.1
consul_host=127.0.0.1

nohup ./babeltrader-okex-quote \
	-host=$host \
	-port=6005 \
	-consul.enable=false \
	-consul.host=$consul_host \
	-consul.port=8500 \
	-consul.service=babeltrader-okex-quote \
	-consul.id=babeltrader-okex-quote-1 \
	-consul.ttl=30s \
	&
