#include "picojson.h"
#include "tchar.h"
#include "Util/tstring.hpp"

class DeviceSettings final {
public:
  DeviceSettings() = delete;

  explicit DeviceSettings(const TCHAR* name);

  ~DeviceSettings();

  void Commit();

  template<typename T>
  using arithmetic_t =
            std::enable_if_t<std::is_arithmetic_v<T>, std::remove_cv_t<T>>;

  template<typename T>
  arithmetic_t<T> get(const char* key) const {
    const picojson::value& value = data.get(key);
    return value.get<double>();
  }

  template<typename T>
  void set(const char* key, const arithmetic_t<T>& value) {
    auto& object = data.get<picojson::object>();
    object[key] = picojson::value(static_cast<double>(value));
  }

private:
  picojson::value data;
  const tstring file_path;
};
