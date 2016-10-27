#include <stdlib.h>
#include <wchar.h>

#include "utils/heapcheck.h"

int _wtoi(const wchar_t *ptr)
{
	return wcstol(ptr, NULL, 10);
}
