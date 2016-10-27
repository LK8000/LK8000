/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#ifndef stl_utils_h__
#define stl_utils_h__

#include <cstddef>
#include <algorithm>

template<typename T, size_t N>
constexpr size_t array_size(T (&array)[N])  {
	return N;
}

struct safe_delete {
	template <typename T>
	void operator()(T*& p) {
		if( p) {
			delete p;
			p = nullptr;
		}
	}

	template <typename K, typename T>
	void operator()(std::pair<K,T*>& p) {
		if( p.second ) {
			delete p.second;
			p.second = nullptr;
		}
	}
};

struct safe_delete_array {
	template <typename T>
	void operator()(T*& p) {
		if( p) {
			delete[] p;
			p = nullptr;
		}
	}
};

// Use this instead of std::ptr_fun(&free)
struct safe_free {
	template <typename T>
	void operator()(T*& p) {
		if( p) {
			free(p);
			p = nullptr;
		}
	}
};

template<typename _Tp>
inline const _Tp& clamp(const _Tp& __val, const _Tp& __min, const _Tp& __max) {
    return std::max(std::min(__val,__max),__min);
}

#endif // stl_utils_h__
