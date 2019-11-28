
#include <iostream>
#include <string>
#include <functional>
#include <signal.h>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#define THREAD_COUNT 4

namespace SigHandler {
	std::function<void(int)> shutdownHandler;
	void signalHandler (int sig) {
		shutdownHandler(sig);
	}
	void assignHandler (boost::asio::io_context& ioc) {
		SigHandler::shutdownHandler = [&] (int sig) {
			ioc.stop();
		};
		struct sigaction sigIntHandler;
		sigIntHandler.sa_handler = SigHandler::signalHandler;
		sigemptyset(&sigIntHandler.sa_mask);
		sigIntHandler.sa_flags = 0;
		sigaction(SIGINT, &sigIntHandler, NULL);

	}
}


void strandPrint (boost::asio::io_context::strand& strand, std::string msg) {
	strand.post([msg] { std::cout << msg << std::endl; });
}


int main (int argc, char* argv[]) {
	int threadCount = THREAD_COUNT;
	bool nonSequential = false;
	if (argc > 1 && std::string("nonsequential").compare(argv[1]) == 0) {
		nonSequential = true;
	}

	boost::asio::io_context ioc(threadCount);
	boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workGuard = boost::asio::make_work_guard(ioc);
	boost::asio::io_context::strand strand(ioc);
	boost::thread_group threadGroup;
	strandPrint(strand, std::to_string(argc));
	for (int i = 0; i < threadCount; i++) {
		strandPrint(strand, "Create " + std::to_string(i));
		threadGroup.create_thread(
			[&ioc, &strand, i] () {
				try {
					strandPrint(strand, "Start " + std::to_string(i));
					ioc.run();
				}
				catch (const std::exception& e) {
					strandPrint(strand, boost::diagnostic_information(e));
				}
			}
		);
	}

	SigHandler::assignHandler(ioc);

	for(int i = 0; i < 10; i++) {
		if (nonSequential) {
			ioc.post([i] { std::cout << i << std::endl; });
		}
		else {
			strand.post([i] { std::cout << i << std::endl; });
		}
	}

	strandPrint(strand, "Join threads");
	strand.post([&workGuard] { workGuard.reset(); });
	threadGroup.join_all();
	std::cout << "Exit" << std::endl;
}
