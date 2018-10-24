#ifndef CTP_QUOTE_CONF_H_
#define CTP_QUOTE_CONF_H_

#include <string>
#include <vector>

struct CTPQuoteConf
{
	std::string broker_id;
	std::string user_id;
	std::string password;
	std::string addr;
	std::string quote_ip;
	int quote_port;
	std::vector<std::string> default_sub_topics;
};

bool LoadConfig(const std::string &file_path, CTPQuoteConf &conf);

#endif