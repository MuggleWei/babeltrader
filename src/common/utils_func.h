#ifndef BABELTRADER_COMMON_FUNC_H_
#define BABELTRADER_COMMON_FUNC_H_

#include <string>
#include <chrono>

namespace babeltrader
{

void CTPSplitInstrument(const char *instrument, std::string &symbol, std::string &contract);
void CTPSplitInstrument(const char *instrument, size_t instrument_len, char* symbol, char *contract);
int64_t CTPGetTimestamp(const char *str_date, const char *str_time, int millisec);

int64_t XTPGetTimestamp(int64_t xtp_ts);

class QuoteTransferMonitor
{
public:
	QuoteTransferMonitor(int64_t step = 10000)
		: step_(step)
	{}

	void start();
	void end(const char *title);

public:
	int64_t total_pkg_;
	int64_t total_elapsed_time_;
	int64_t step_;

	std::chrono::time_point<std::chrono::system_clock> ts_;
};

}

#endif