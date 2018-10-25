#include "err.h"

const char* BABELTRADER_ERR_MSG[BABELTRADER_ERR_MAX - BABELTRADER_ERR_BEGIN] = {
	"",
	"http request data too long",			// BABELTRADER_ERR_HTTPREQ_TOO_LONG
	"failed parse http request data",		// BABELTRADER_ERR_HTTPREQ_FAILED_PARSE
	"failed parse websocket request data",	// BABELTRADER_ERR_WSREQ_FAILED_PARSE
	"can't find callback handler",			// BABELTRADER_ERR_WSREQ_NOT_HANDLE
	"failed handle message",				// BABELTRADER_ERR_WSREQ_FAILED_HANDLE
	"failed transfer message in tunnel",	// BABELTRADER_ERR_WSREQ_FAILED_TUNNEL
};