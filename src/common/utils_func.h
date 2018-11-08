#ifndef BABELTRADER_COMMON_FUNC_H_
#define BABELTRADER_COMMON_FUNC_H_

#include <string>

namespace babeltrader
{

void CTPSplitInstrument(const char *instrument, std::string &symbol, std::string &contract);
int64_t CTPGetTimestamp(const char *str_date, const char *str_time, int millisec);

int64_t XTPGetTimestamp(int64_t xtp_ts);

}

#endif