#ifndef XTP_TRADE_CONF_H_
#define XTP_TRADE_CONF_H_

#include <string>

struct XTPTradeConf
{
	uint8_t client_id;
	uint8_t trade_protocol;
	std::string user_id;
	std::string password;
	std::string ip;
	int port;
	std::string key;
	std::string trade_ip;
	int trade_port;
};

bool LoadConfig(const std::string &file_path, XTPTradeConf &conf);

#endif