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
inline size_t array_size(T (&array)[N])  {
	return N;
}

template<typename T, size_t N>
T* begin(T (&array)[N]) {
	return &array[0];
}

template<typename T, size_t N>
T* end(T (&array)[N]) {
	return &array[N];
}

template<typename T, size_t N>
T* rbegin(T (&array)[N]) {
	return &array[N-1];
}

template<typename T, size_t N>
T* rend(T (&array)[N]) {
	return &array[-1];
}

struct safe_delete {
	template <typename T>
	void operator()(T*& p) {
		if( p) {
			delete p;
			p = 0;
		}
	}

	template <typename K, typename T>
	void operator()(std::pair<K,T*>& p) {
		if( p.second ) {
			delete p.second;
			p.second = 0;
		}
	}
};

struct safe_delete_array {
	template <typename T>
	void operator()(T*& p) {
		if( p) {
			delete[] p;
			p = 0;
		}
	}
};

// Use this instead of std::ptr_fun(&free)
struct safe_free {
	template <typename T>
	void operator()(T*& p) {
		if( p) {
			free(p);
			p = 0;
		}
	}
};

template<typename _Tp>
inline const _Tp& clamp(const _Tp& __val, const _Tp& __min, const _Tp& __max) {
    return std::max(std::min(__val,__max),__min);
}

#endif // stl_utils_h__

