/*
 * convert WMM.COF file to magfield.cpp array 
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>


void print(std::ostream& out,  const char* name, int epoch, const double(&array)[13][13]) {

    out << "static const double " << name << epoch << "[13][13] = {\n";
    for (size_t i = 0; i < 13; ++i) {
        out << "  {";
        for (size_t j = 0; j < 13; ++j) {

            out << std::fixed << std::setw(8) << std::setprecision(1);
            out << array[i][j];
            out << (j < 12 ? "," : "");
        }
        out << "}" << (i < 12 ? ",\n" : "\n");
    }
    out << "};\n";
}


int main()
{
    double epoch = 0;
    char model[20] = {};

    double gnm_wmm2025[13][13] = {};
    double hnm_wmm2025[13][13] = {};
    double gtnm_wmm2025[13][13] = {};
    double htnm_wmm2025[13][13] = {};

    try {
        std::ifstream wmm_file("WMM_2025.COF");
        if (wmm_file.is_open())
        {
            std::string line;

            if (std::getline(wmm_file, line)) {
                if (sscanf(line.c_str(), "%lf%20s", &epoch, model) < 2) {
                    throw "invalid header";
                }
            }

            while (std::getline(wmm_file, line)) {
                unsigned n, m;
                double gnm, hnm, dgnm, dhnm;
                if (sscanf(line.c_str(), "%u%u%lf%lf%lf%lf", &n, &m, &gnm, &hnm, &dgnm, &dhnm) == 6) {
                    if (n < 13 && m < 13) {
                        gnm_wmm2025[n][m] = gnm;
                        hnm_wmm2025[n][m] = hnm;
                        gtnm_wmm2025[n][m] = dgnm;
                        htnm_wmm2025[n][m] = dhnm;
                    }
                }
            }
            wmm_file.close();

            print(std::cout, "gnm_wmm", static_cast<int>(epoch), gnm_wmm2025);
            print(std::cout, "hnm_wmm", static_cast<int>(epoch), hnm_wmm2025);
            print(std::cout, "gtnm_wmm", static_cast<int>(epoch), gtnm_wmm2025);
            print(std::cout, "htnm_wmm", static_cast<int>(epoch), htnm_wmm2025);

        }
    }
    catch (const char* error) {
        std::cout << error << "\n";
    }
}
