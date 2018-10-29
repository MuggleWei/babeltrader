#ifndef BABELTRADER_QUOTE_SERVICE_H_
#define BABELTRADER_QUOTE_SERVICE_H_

#include <vector>

#include "common/common_struct.h"

namespace babeltrader
{


class QuoteService
{
public:
	virtual std::vector<Quote> GetSubTopics(std::vector<bool> &vec_b) = 0;
	virtual void SubTopic(const Quote &msg) = 0;
	virtual void UnsubTopic(const Quote &msg) = 0;
};


}

#endif