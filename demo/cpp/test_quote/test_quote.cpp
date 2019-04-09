#include <stdint.h>
#include <time.h>
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

void TimeDiff(struct timespec t0, struct timespec t1, struct timespec ts)
{
	static int64_t total_elapsed = 0;
	static int64_t total_pkg = 0;
	const static int64_t step = 1024;

	total_elapsed += (ts.tv_sec - t0.tv_sec) * 1000000000 + ts.tv_nsec - t0.tv_nsec;
	total_pkg += 1;
	if (total_pkg >= step)
	{
		double avg_elapsed_ms = (double)total_elapsed / total_pkg;
		LOG(INFO)
			<< "receive quotes - total pkg: " << total_pkg
			<< ", total use: " << total_elapsed
			<< ", avg elapsed: " << avg_elapsed_ms;
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
		struct timespec ts;
		timespec_get(&ts, TIME_UTC);

		rapidjson::Document doc;
		doc.Parse(message, length);

		if (doc.HasParseError() || !doc.IsArray()) {
			return;
		}

		for (unsigned int i = 0; i < doc.Size(); ++i)
		{
			rapidjson::Value &quote = doc[i];
			if (!(quote.HasMember("data") && quote["data"].IsObject()))
			{
				return;
			}

			struct timespec t0;
			struct timespec t1;
			t0.tv_sec = quote["data"]["t0_s"].GetInt64();
			t0.tv_nsec = quote["data"]["t0_ns"].GetInt64();
			t1.tv_sec = quote["data"]["t1_s"].GetInt64();
			t1.tv_nsec = quote["data"]["t1_ns"].GetInt64();
			TimeDiff(t0, t1, ts);
		}
#else
		LOG(INFO).write(message, length);
#endif
	});

	h.connect(addr, nullptr);
	h.run();

	return 0;
}
