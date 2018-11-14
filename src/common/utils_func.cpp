#include "utils_func.h"
#include <time.h>
#include <string.h>

namespace babeltrader
{


void CTPSplitInstrument(const char *instrument, std::string &symbol, std::string &contract)
{
	char buf[64] = { 0 };
	const char *p = instrument;
	while (*p) {
		if (*p >= '0' && *p <= '9') {
			break;
		}
		p++;
	}
	auto len = p - instrument;
	strncpy(buf, instrument, len);
	symbol = buf;
	contract = p;
}
void CTPSplitInstrument(const char *instrument, size_t instrument_len, char* symbol, char *contract)
{
	const char *p = instrument;
	while (*p) {
		if (*p >= '0' && *p <= '9') {
			break;
		}
		p++;
	}
	auto len = p - instrument;
	strncpy(symbol, instrument, len);
	strncpy(contract, p, instrument_len - len);
}
int64_t CTPGetTimestamp(const char *str_date, const char *str_time, int millisec)
{
	struct tm time_info = { 0 };
	int date = atoi(str_date);
	time_info.tm_year = date / 10000 - 1900;
	time_info.tm_mon = (date % 10000) / 100 - 1;
	time_info.tm_mday = date % 100;

	if (strlen(str_time) == 8) {
		char buf[16] = { 0 };
		strncpy(buf, str_time, sizeof(buf));
		buf[2] = '\0';
		buf[5] = '\0';
		time_info.tm_hour = atoi(&buf[0]);
		time_info.tm_min = atoi(&buf[3]);
		time_info.tm_sec = atoi(&buf[6]);
	}

	time_t utc_sec = mktime(&time_info);
	return (int64_t)utc_sec * 1000 + (int64_t)millisec;
}

int64_t XTPGetTimestamp(int64_t xtp_ts)
{
	struct tm time_info = { 0 };

	int64_t year = xtp_ts / 10000000000000;
	int64_t month = (xtp_ts / 100000000000) % 100;
	int64_t day = (xtp_ts / 1000000000) % 100;
	int64_t hour = (xtp_ts / 10000000) % 100;
	int64_t minute = (xtp_ts / 100000) % 100;
	int64_t sec = (xtp_ts / 1000) % 100;
	int64_t mill = xtp_ts % 1000;

	time_info.tm_year = year - 1900;
	time_info.tm_mon = month - 1;
	time_info.tm_mday = day;
	time_info.tm_hour = hour;
	time_info.tm_min = minute;
	time_info.tm_sec = sec;

	time_t utc_sec = mktime(&time_info);
	return (int64_t)utc_sec * 1000 + mill;
}

}
