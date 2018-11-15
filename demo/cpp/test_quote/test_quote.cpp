#include <stdint.h>
#include "common/common_struct.h"
#include "uWS/uWS.h"
#include "rapidjson/document.h"
#include "glog/logging.h"

#if WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

const char *addr = "ws://127.0.0.1:6002/ws";

void TimeDiff(int64_t ts, int64_t cur_ms)
{
	static int64_t total_elapsed = 0;
	static int64_t total_pkg = 0;
	const static int64_t step = 10000;

	total_elapsed += (cur_ms - ts);
	total_pkg += 1;
	if (total_pkg >= step)
	{
		double avg_elapsed_ms = (double)total_elapsed / total_pkg;
		LOG(INFO)
			<< "receive quotes - total pkg: " << total_pkg
			<< ", avg elapsed ms: " << avg_elapsed_ms;
		total_elapsed = 0;
		total_pkg = 0;
	}
}

int main(int argc, char *argv[])
{
	// init glog
	fLI::FLAGS_max_log_size = 100;
	fLI::FLAGS_logbufsecs = 0;
	fLS::FLAGS_log_dir = "./log";
#if WIN32
	CreateDirectoryA("./log/", NULL);
#else
	mkdir("./log/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
	google::SetLogDestination(google::GLOG_INFO, "./log/test_quote.INFO.");
	google::SetLogDestination(google::GLOG_WARNING, "./log/test_quote.WARNING.");
	google::SetLogDestination(google::GLOG_ERROR, "./log/test_quote.ERROR.");
	google::InitGoogleLogging(argv[0]);


	uWS::Hub h;

	h.onMessage([](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
#if ENABLE_PERFORMANCE_TEST
		auto t = std::chrono::system_clock::now().time_since_epoch();
		auto cur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t).count();

		rapidjson::Document doc;
		doc.Parse(message, length);

		if (doc.HasParseError()) {
			return;
		}

		if (!(doc.HasMember("data") && doc["data"].IsObject()))
		{
			return;
		}

		if (doc["data"].HasMember("ts") && doc["data"]["ts"].IsInt64())
		{
			int64_t ts = doc["data"]["ts"].GetInt64();
			TimeDiff(ts, cur_ms);
		}
#else
		LOG(INFO).write(message, length);
#endif
	});

	h.connect(addr, nullptr);
	h.run();

	return 0;
}
