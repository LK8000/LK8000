#include <boost/asio.hpp>

/// synchronization (switch from NMEA to command mode and vice versa)
constexpr uint8_t PKT_SYN = 0x16;
/// start command
constexpr uint8_t PKT_STX = 0x02;
/// acknowledged  (CRC OK)
constexpr uint8_t PKT_ACK = 0x06;
/// write flight info and task declaration
constexpr uint8_t PKT_PCWRITE = 0xCA;
/// write Lx class (char LxClass[9] + CRC)
constexpr uint8_t PKT_CCWRITE = 0xD0;

#pragma pack(push, 1)

/// LX flight declaration data (should be compatible with Nano, Colibri and Posigraph)
struct Flight {
	uint8_t flag;	///< can be empty for Nano
	uint16_t oo_id; ///< oficial observer id
	char pilot[19];
	char glider[12];
	char reg_num[8]; ///< aircraft registration
	char cmp_num[4];
	uint8_t cmp_cls; ///< glider class (see @c Class)
	char observer[10];
	uint8_t gpsdatum;
	uint8_t fix_accuracy;
	char gps[60];
};

/// LX task declaration data
struct Task  {
	// auto defined
	uint8_t flag;		///< can be empty for Nano
	int32_t input_time; ///< time of declaration (not important, because timestamp before takeoff is used)
	uint8_t di;			///< day of declaration
	uint8_t mi;			///< month of declaration
	uint8_t yi;			///< year of declaration

	// user defined
	uint8_t fd; ///< intended day of flight [local date]
	uint8_t fm; ///< intended month of flight [local date]
	uint8_t fy; ///< intended  year of flight [local date]

	int16_t taskid; ///< task number of the day (if unused, default is 1)
	char num_of_tp; ///< ! nb of TPs between Start and Finish (not nb of WPs initialized)
	int8_t tpt[12]; ///< waypoint type (see @c WpType)
	int32_t lon[12];
	int32_t lat[12];
	uint8_t name[12][9];
};

struct Decl {
	Flight s_flight;
	Task s_task;
	uint8_t crc;
};

struct Class {
	char name[9];
	uint8_t crc;
};

#pragma pack(pop)

/*
 * to create virtual port :
 *   socat -d -d pty,raw,echo=0 pty,raw,echo=0
 */

int main() {
	try {
		boost::asio::io_service io;
		boost::asio::serial_port serial(io, "/dev/pts/7");

		serial.set_option(boost::asio::serial_port_base::baud_rate(9600));

		Decl decl;
		Class cls;
		size_t size;
		uint8_t *buf;
		uint8_t c;

		while (true) {

			boost::asio::read(serial, boost::asio::buffer(&c, 1));
			switch (c) {
			case PKT_SYN:
				printf("Read : [%02X]\n", c);
				boost::asio::write(serial, boost::asio::buffer(&PKT_ACK, 1));
				printf("Write : [%02X]\n", PKT_ACK);
				break;
			case PKT_STX:
				printf("Read : [%02X]\n", c);
				boost::asio::read(serial, boost::asio::buffer(&c, 1));
				printf("Read : [%02X]\n", c);
				switch (c) {
				case PKT_PCWRITE:
					size = boost::asio::read(serial, boost::asio::buffer(&decl, sizeof(decl)));
					printf("Decl : size[%zu] crc [%02X]\n [", size, decl.crc);
					buf = reinterpret_cast<uint8_t *>(&decl);
					for (unsigned i = 0; i < sizeof(decl); ++i) {
						printf("%02X ", buf[i]);
					}
					printf("]\n");
					boost::asio::write(serial, boost::asio::buffer(&PKT_ACK, 1));
					printf("Write : [%02X]\n", PKT_ACK);
					break;
				case PKT_CCWRITE:
					size = boost::asio::read(serial, boost::asio::buffer(&cls, sizeof(cls)));
					printf("Class : size[%zu] crc [%02X]\n [", size, cls.crc);
					buf = reinterpret_cast<uint8_t *>(&cls);
					for (unsigned i = 0; i < sizeof(cls); ++i) {
						printf("%02X ", buf[i]);
					}
					printf("]\n");
					boost::asio::write(serial, boost::asio::buffer(&PKT_ACK, 1));
					printf("Write : [%02X]\n", (uint8_t)PKT_ACK);
					break;
				}
				break;
			default:
				printf("Read : [%02X]\n", (uint8_t)c);
				break;
			}
		}
	}
	catch (std::exception &e) {
		printf("%s\n", e.what());
	}
	return 0;
}
