#include "../../Source/Waypoints/cupx_reader.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

// Display POINTS.CUP
void print_points_cup(const cupx_reader& reader) {
  std::cout << "\n=== Reading POINTS.CUP ===" << std::endl;
  try {
    auto in = reader.read_points_cup();
    std::string content = {
        (std::istreambuf_iterator<char>(&in)),
        (std::istreambuf_iterator<char>())
    };

    std::cout << "--- POINTS.CUP contents ---" << std::endl;
    std::cout << content << std::endl;

    size_t line_count = std::count(content.begin(), content.end(), '\n');
    std::cout << "--- End (" << line_count << " lines) ---" << std::endl;
    std::cout << "✓ POINTS.CUP read successfully" << std::endl;
  }
  catch (const std::exception& e) {
    std::cout << "✗ POINTS.CUP : " << e.what() << std::endl;
  }
}

int main(int argc, char* argv[]) {
  std::cout << "==============================================" << std::endl;
  std::cout << "  POC Reading CUPX with zziplib (memory)" << std::endl;
  std::cout << "==============================================" << std::endl;

  std::string cupx_file = "test.cupx";

  if (argc > 1) {
    cupx_file = argv[1];
  }

  std::cout << "CUPX file: " << cupx_file << "\n" << std::endl;

  try {
    const cupx_reader reader(cupx_file);

    // Read POINTS.CUP
    print_points_cup(reader);

    auto image = reader.read_image("col_croix_fer_1.jpg");
    std::cout << "col_croix_fer_1.jpg found" << std::endl;
  }
  catch (const std::exception& e) {
    std::cerr << "\n❌ Error: " << e.what() << std::endl;
    return 1;
  }

  try {
    const cupx_reader reader(cupx_file);
    auto in = [&reader]() {
      return reader.read_points_cup();
    }();

    std::string content = {
        (std::istreambuf_iterator<char>(&in)),
        (std::istreambuf_iterator<char>())
    };

    std::cout << "--- POINTS.CUP contents ---" << std::endl;
    std::cout << content << std::endl;
  }
  catch (const std::exception& e) {
    std::cerr << "\n❌ Error: " << e.what() << std::endl;
    return 1;
  }

  std::cout << "\n==============================================" << std::endl;
  std::cout << "  Tests completed" << std::endl;
  std::cout << "==============================================" << std::endl;

  return 0;
}
