#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <queue>
#include <iomanip>
#include <sstream>

namespace asio = boost::asio;

class ticker {
    using io_context = asio::io_context;
    using steady_timer = asio::steady_timer;
    using milliseconds = std::chrono::milliseconds;
    using function = std::function<void()>;
    using error_code = boost::system::error_code;

public:
    ticker(io_context &io, milliseconds interval, function&& handler)
            : _timer(io, interval), _interval(interval), _handler(std::move(handler)) {
                
        async_start();
    }

    size_t stop() {
        return _timer.cancel();
    }

private:
    void async_start() {
        _timer.async_wait([&](const error_code ec) {
            on_tick(ec);
        });
    }

    void on_tick(const error_code ec) {
        _handler();
        _timer.expires_from_now(_interval);
        async_start();
    }

    steady_timer _timer;
    milliseconds _interval;
    function _handler;
};


class write_queue {
    using io_context = asio::io_context;
    using serial_port = boost::asio::serial_port;
    using strand = boost::asio::io_service::strand;
    using error_code = boost::system::error_code;

public:
    write_queue(io_context& io, serial_port& serial) : _strand(io), _serial(serial) {}

    void push(std::string&& data) {
        _strand.post([this, data = std::move(data)]() mutable {
            push_handler(std::move(data));
        });
    }

private:
    void write(const std::string& data) {
        async_write(_serial, asio::buffer(data.data(), data.size()), 
                _strand.wrap([&](auto& e, auto bytesTransferred) {
                    write_handler(e, bytesTransferred);
                }));
    }

    void push_handler(std::string&& data) {
        _queue.emplace(data);
        if (_queue.size() > 1 ) {
            // outstanding async_write
            return;
        }
        write(_queue.front());
    }

    void write_handler(const error_code& e, size_t bytesTransferred) {
        _queue.pop();
        if (e) {
            // log error
            return;
        }
        if (!_queue.empty()) {
            write(_queue.front());
        }
    }

    std::queue<std::string> _queue;

    serial_port& _serial;
    strand _strand;
};

uint8_t nmea_crc(const char *text) {
  uint8_t crc = 0U;
  for (const char* c = text; *c; ++c) {
    crc ^= static_cast<uint8_t>(*c);
  }
  return crc;
}

class send_flarm_data {
public:
    explicit send_flarm_data(write_queue& queue) : _queue(queue) {}

    void operator()() {
        const char* s = "$PFLAA,0,-1234,1234,220,2,DD8F12,180,-4.5,30,-1.4,1";
        std:: ostringstream oss;
        oss << s << '*'  << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(nmea_crc(s+1)) << "\r\n";
        _queue.push(oss.str());
    }

private:
    write_queue& _queue;
};

class request_answer {
    using serial_port = boost::asio::serial_port;
    using streambuf = boost::asio::streambuf;
    using error_code = boost::system::error_code;

public:    
    request_answer(write_queue& queue, serial_port& serial) : _queue(queue), _serial(serial) {
        read();
    }

private:
    void read() {
        async_read_until(_serial, data, "\r\n", [&](const error_code& e, std::size_t size) {
            read_handler(e, size);
        });
    }

    void read_handler(const error_code& e, std::size_t size) {
        if (!e) {
            std::istream is(&data);
            std::string line;
            std::getline(is, line);
            if (line.rfind("$PFLAC,S", 0) == 0) {
                line[7] = 'A';
                size_t end = line.find('*');
                if (end != std::string::npos) { 
                    write(line.substr(0, end));
                }
            }
            read();
        }
    }

    void write(std::string&& data) {
        std:: ostringstream oss;
        oss << data << '*' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(nmea_crc(data.c_str()+1)) << "\r\n";
        _queue.push(oss.str());
    }

    streambuf data;
    write_queue& _queue;
    serial_port& _serial;
};

int main() {
    using namespace std::chrono_literals;
    using io_service = boost::asio::io_service;
    using serial_port = boost::asio::serial_port;
    using serial_port_base = boost::asio::serial_port_base;

	try {
		io_service io;

		serial_port serial(io, "/dev/pts/3");
		serial.set_option(serial_port_base::baud_rate(9600));

        write_queue out_queue(io, serial);

        ticker t_nmea_out(io, 500ms, send_flarm_data(out_queue));
        request_answer request(out_queue, serial);

        io.run();
	}
	catch (std::exception &e) {
		printf("%s\n", e.what());
	}
	return 0;
}
