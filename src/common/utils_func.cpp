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


}
