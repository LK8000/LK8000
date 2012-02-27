#ifndef stl_utils_h__
#define stl_utils_h__

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
};

#endif // stl_utils_h__