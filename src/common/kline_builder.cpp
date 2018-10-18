#include "kline_builder.h"

#include <time.h>

void KlineBuilder::add(const std::string &key)
{
	if (caches_.find(key) == caches_.end()) {
		KlineCache cache;
		cache.last_update_local_sec = 0;
		caches_[key] = cache;
	}
}
void KlineBuilder::del(const std::string &key)
{
	caches_.erase(key);
}

bool KlineBuilder::updateMarketData(int64_t cur_local_sec, const std::string &key, const MarketData &md, Kline &kline)
{
	// ensure cache exists
	auto it = caches_.find(key);
	if (it == caches_.end()) {
		return false;
	}

	// first time update
	KlineCache &cache = it->second;
	if (cache.last_update_local_sec == 0) {
		cache.start_vol = md.vol;
		cache.last_update_local_sec = cur_local_sec;
		cache.kline.ts = md.ts;
		cache.kline.open = md.last;
		cache.kline.high = md.last;
		cache.kline.low = md.last;
		cache.kline.close = md.last;
		cache.kline.vol = md.vol;

		cache.last_update_local_sec = cur_local_sec;
		return false;
	}

	// update local second
	cache.last_update_local_sec = cur_local_sec;

	// check minute update
	bool minute_update = false;
	struct tm last_tm, curr_tm;
	time_t last_system_ts = cache.kline.ts / 1000;
	time_t curr_system_ts = md.ts / 1000;
#if WIN32
	localtime_s(&last_tm, &last_system_ts);
	localtime_s(&curr_tm, &curr_system_ts);
#else
	localtime_r(&last_system_ts, &last_tm);
	localtime_r(&curr_system_ts, &curr_tm);
#endif
	if (last_tm.tm_min != curr_tm.tm_min ||
		last_tm.tm_hour != curr_tm.tm_hour) {
		minute_update = true;
	}

	// check vol update
	bool val_update = false;
	if (md.last > cache.kline.high) {
		cache.kline.high = md.last;
		val_update = true;
	}
	if (md.last < cache.kline.low) {
		cache.kline.low = md.last;
		val_update = true;
	}

	// update kline
	if (minute_update)
	{
		auto end_vol = cache.kline.vol;
		kline = cache.kline;
		kline.vol = cache.kline.vol - cache.start_vol;

		cache.start_vol = end_vol;
		cache.kline.ts = md.ts;
		cache.kline.open = md.last;
		cache.kline.high = md.last;
		cache.kline.low = md.last;
		cache.kline.close = md.last;
		cache.kline.vol = md.vol;

		return true;
	}

	cache.kline.close = md.last;
	cache.kline.ts = md.ts;
	cache.kline.vol = md.vol;

	if (val_update)
	{
		kline = cache.kline;
		kline.vol = md.vol - cache.start_vol;
		return true;
	}


	return false;
}
