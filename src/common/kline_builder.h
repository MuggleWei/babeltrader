#ifndef BABELTRADER_KLINE_BUILDER_H_
#define BABELTRADER_KLINE_BUILDER_H_

#include <map>
#include <string>

#include "common/common_struct.h"

namespace babeltrader
{


struct KlineCache
{
	int64_t start_vol;
	int64_t last_update_local_sec;
	Kline kline;
};

class KlineBuilder
{
public:
	void add(const std::string &key);
	void del(const std::string &key);

	bool updateMarketData(int64_t cur_local_sec, const std::string &key, const MarketData &md, Kline &kline);

private:
	std::map<std::string, KlineCache> caches_;
};


}

#endif