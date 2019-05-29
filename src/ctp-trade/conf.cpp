#include "conf.h"

#include <stdlib.h>

#include "glog/logging.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

bool LoadConfig(const std::string &file_path, CTPTradeConf &conf)
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

		if (doc.HasMember("broker_id") && doc["broker_id"].IsString())
		{
			conf.broker_id = doc["broker_id"].GetString();
		}
		else
		{
			throw(std::runtime_error("can't find 'broker_id' in config file"));
		}

		if (doc.HasMember("user_id") && doc["user_id"].IsString())
		{
			conf.user_id = doc["user_id"].GetString();
		}
		else
		{
			throw(std::runtime_error("can't find 'user_id' in config file"));
		}

		if (doc.HasMember("password") && doc["password"].IsString())
		{
			conf.password = doc["password"].GetString();
		}
		else
		{
			throw(std::runtime_error("can't find 'password' in config file"));
		}

		if (doc.HasMember("trade_addr") && doc["trade_addr"].IsString())
		{
			conf.addr = doc["trade_addr"].GetString();
		}
		else
		{
			throw(std::runtime_error("can't find 'trade_addr' in config file"));
		}

		if (doc.HasMember("trade_listen_ip") && doc["trade_listen_ip"].IsString())
		{
			conf.trade_ip = doc["trade_listen_ip"].GetString();
		}
		else
		{
			throw(std::runtime_error("can't find 'trade_listen_ip' in config file"));
		}

		if (doc.HasMember("trade_listen_port") && doc["trade_listen_port"].IsInt())
		{
			conf.trade_port = doc["trade_listen_port"].GetInt();
		}
		else
		{
			throw(std::runtime_error("can't find 'trade_listen_port' in config file"));
		}

		if (doc.HasMember("product_info") && doc["product_info"].IsString())
		{
			conf.product_info = doc["product_info"].GetString();
		}
		else
		{
			throw(std::runtime_error("can't find 'product_info' in config file"));
		}

		if (doc.HasMember("app_id") && doc["app_id"].IsString())
		{
			conf.app_id = doc["app_id"].GetString();
		}
		else
		{
			throw(std::runtime_error("can't find 'app_id' in config file"));
		}

		if (doc.HasMember("auth_code") && doc["auth_code"].IsString())
		{
			conf.auth_code = doc["auth_code"].GetString();
		}
		else
		{
			throw(std::runtime_error("can't find 'auth_code' in config file"));
		}
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
