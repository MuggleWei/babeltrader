#ifndef CTP_TRADE_CONF_H_
#define CTP_TRADE_CONF_H_

#include <string>

struct CTPTradeConf
{
	std::string broker_id;
	std::string user_id;
	std::string password;
	std::string addr;
	std::string trade_ip;
	int trade_port;
	std::string product_info;
	std::string app_id;
	std::string auth_code;
};

bool LoadConfig(const std::string &file_path, CTPTradeConf &conf);

#endif
