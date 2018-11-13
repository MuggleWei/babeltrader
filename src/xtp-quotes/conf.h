#ifndef XTP_QUOTE_CONF_H_
#define XTP_QUOTE_CONF_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "common/common_struct.h"

using namespace babeltrader;

struct XTPQuoteConf
{
	uint8_t client_id;
	uint8_t quote_protocol;
	std::string user_id;
	std::string password;
	std::string ip;
	int port;
	std::string key;
	std::string quote_ip;
	int quote_port;
	int sub_all;
	int sub_orderbook;
	int sub_l2;
	std::vector<Quote> default_sub_topics;
};

bool LoadConfig(const std::string &file_path, XTPQuoteConf &conf);


#endif