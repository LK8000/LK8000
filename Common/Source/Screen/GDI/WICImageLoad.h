#pragma once

#include <vector>
#include <cstdint>
#include "Screen/LKBitmap.h"

struct WICContext;

LKBitmap LoadImageFromMemoryWIC(WICContext& ctx, const std::vector<uint8_t>& data);
