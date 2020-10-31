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

#if __cplusplus < 201402L
namespace std {
  /**
   *  @brief  Return a reverse iterator pointing to the last element of
   *          the array.
   *  @param  __arr  Array.
   */
  template<typename _Tp, size_t _Nm>
    inline reverse_iterator<_Tp*>
    rbegin(_Tp (&__arr)[_Nm])
    { return reverse_iterator<_Tp*>(__arr + _Nm); }

  /**
   *  @brief  Return a reverse iterator pointing one past the first element of
   *          the array.
   *  @param  __arr  Array.
   */
  template<typename _Tp, size_t _Nm>
    inline reverse_iterator<_Tp*>
    rend(_Tp (&__arr)[_Nm])
    { return reverse_iterator<_Tp*>(__arr); }
}

#endif


#endif // stl_utils_h__
