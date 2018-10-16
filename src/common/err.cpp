#include "err.h"

const char* BABELTRADER_ERR_MSG[BABELTRADER_ERR_MAX - BABELTRADER_ERR_BEGIN] = {
	"",
	"http request data too long",		// BABELTRADER_ERR_HTTPREQ_TOO_LONG
	"failed parse http request data",	// BABELTRADER_ERR_HTTPREQ_FAILED_PARSE
};