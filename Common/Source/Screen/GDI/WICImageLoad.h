#pragma once

#include <vector>
#include <cstdint>
#include "Screen/LKBitmap.h"

struct WICContext;

LKBitmap LoadImageFromMemoryWIC(WICContext& ctx, const void* data, size_t size);
