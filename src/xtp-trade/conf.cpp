#include "conf.h"

#include <stdlib.h>

#include "glog/logging.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

bool LoadConfig(const std::string &file_path, XTPTradeConf &conf)
{
	FILE *fp = nullptr;
	char *buf = nullptr;

	fp = fopen(file_path.c_str(), "rb");
	if (fp == nullptr) {
		return false;
	}

	bool ret = true;
	try {
		fseek(fp, 0, SEEK_END);
		long cnt = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		buf = (char*)malloc((size_t)cnt + 1);
		memset(buf, 0, cnt + 1);
		auto read_cnt = fread(buf, 1, cnt, fp);
		if (read_cnt != cnt) {
			throw(std::runtime_error("failed read config file"));
		}

		rapidjson::Document doc;
		doc.Parse(buf);

		if (doc.HasParseError()) {
			throw(std::runtime_error("failed parse json from config file"));
		}

		conf.client_id = doc["custom_client_id"].GetUint();
		conf.trade_protocol = doc["trade_protocol"].GetUint();
		conf.user_id = doc["user_id"].GetString();
		conf.password = doc["password"].GetString();
		std::string addr = doc["trade_addr"].GetString();
		auto pos = addr.find(":");
		char buf[32] = { 0 };
		strncpy(buf, addr.c_str(), pos);
		conf.ip = buf;
		strncpy(buf, addr.c_str() + pos + 1, sizeof(buf) - 1);
		conf.port = atoi(buf);
		conf.key = doc["key"].GetString();
		conf.trade_ip = doc["trade_listen_ip"].GetString();
		conf.trade_port = doc["trade_listen_port"].GetInt();
	}
	catch (std::exception e) {
		LOG(ERROR) << e.what();
		ret = false;
	}

	if (buf) {
		free(buf);
	}

	if (fp) {
		fclose(fp);
	}

	return ret;
}