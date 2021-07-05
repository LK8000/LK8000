#include <boost/asio.hpp>
#include <chrono>
#include <thread>
#include <iostream>

/*
 * to create virtual port :
 *    socat -d -d pty,raw,echo=0 pty,raw,echo=0
 */

using namespace std::chrono_literals;
using namespace std::chrono;
using boost::asio::ip::tcp;


//#define SERIAL

constexpr uint8_t PING = 'S';

int main() {
	try {
		boost::asio::io_service io_;


#ifdef SERIAL
		boost::asio::serial_port port(io_, "/dev/rfcomm0");
#else

		tcp::endpoint endpoint_(tcp::v4(), 1234);
		tcp::acceptor acceptor_(io_, endpoint_);
		tcp::socket port(io_);
		acceptor_.accept(port);
#endif

		uint8_t c;

		while (true) {
			auto start = steady_clock::now();

			boost::asio::write(port, boost::asio::buffer(&PING, 1));
			boost::asio::read(port, boost::asio::buffer(&c, 1));

			auto elapsed = duration_cast<milliseconds>(steady_clock::now() - start);

			std::cout << "ping : " << elapsed.count() << "ms" << std::endl;

			std::this_thread::sleep_until(start + 1s);
		}
	}
	catch (std::exception &e) {
		printf("%s\n", e.what());
	}
	return 0;
}
