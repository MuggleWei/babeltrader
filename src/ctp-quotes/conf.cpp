#include "conf.h"

#include <stdlib.h>

#include "glog/logging.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

bool LoadConfig(const std::string &file_path, CTPQuoteConf &conf)
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

		if (doc.HasMember("quote_addr") && doc["quote_addr"].IsString())
		{
			conf.addr = doc["quote_addr"].GetString();
		}
		else
		{
			throw(std::runtime_error("can't find 'quote_addr' in config file"));
		}

		if (doc.HasMember("quote_listen_ip") && doc["quote_listen_ip"].IsString())
		{
			conf.quote_ip = doc["quote_listen_ip"].GetString();
		}
		else
		{
			throw(std::runtime_error("can't find 'quote_listen_ip' in config file"));
		}

		if (doc.HasMember("quote_listen_port") && doc["quote_listen_port"].IsInt())
		{
			conf.quote_port = doc["quote_listen_port"].GetInt();
		}
		else
		{
			throw(std::runtime_error("can't find 'quote_listen_port' in config file"));
		}

		if (doc.HasMember("default_sub_topics") && doc["default_sub_topics"].IsArray())
		{
			auto topics = doc["default_sub_topics"].GetArray();
			for (auto i = 0; i < topics.Size(); i++) {
				conf.default_sub_topics.push_back(topics[i].GetString());
			}
		}
	} catch (std::exception e) {
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