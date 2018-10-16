#ifndef BABELTRADER_QUOTE_SERVICE_H_
#define BABELTRADER_QUOTE_SERVICE_H_

#include <vector>

#include "common/common_struct.h"

class QuoteService
{
public:
	virtual std::vector<SubUnsubMsg> GetSubTopics(std::vector<bool> &vec_b) = 0;
	virtual void SubTopic(const SubUnsubMsg &msg) = 0;
	virtual void UnsubTopic(const SubUnsubMsg &msg) = 0;
};

#endif