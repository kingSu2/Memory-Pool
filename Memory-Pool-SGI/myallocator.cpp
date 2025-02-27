#include "myallocator.h"

template <typename T>
T* myallocato<T>::allocator(size_t __n) {
	void*T* allocate(size_t __n) {
		__n *= sizeof(T);
		void* __ret = 0;

		if (__n > (size_t)_MAX_BYTES) {
			__ret = malloc_alloc::allocate(__n);
		}
		else {
			_Obj* volatile* __my_free_list = _S_free_list + _S_freelist_index(__n);

			std::lock_guard<std::mutex> guard(mtx);

			_Obj* __result = *__my_free_list;
			if (__result == 0)
				__ret = _S_refill(_S_round_up(__n));
			else {
				*__my_free_list = __result->_M_free_list_link;
				__ret = __result;
			}
		}
		return (T*)__ret;
	}

template<typename T>
	void myallocator<T>::deallocate(void* __p, size_t __n) {
		if (__n > (size_t)_MAX_BYTES)
			malloc_alloc::deallocate(__p, __n);
		else {
			_Obj* volatile* __my_free_list
				= _S_free_list + _S_freelist_index(__n);
			_Obj* __q = (_Obj*)__p;

			std::lock_guard<std::mutex> guard(mtx);

			__q->_M_free_list_link = *__my_free_list;
			*__my_free_list = __q;
		}
	}

template<typename T>
	void* myallocator<T>::reallocate(void* __p, size_t __old_sz, size_t __new_sz) {
		void* __result;
		size_t __copy_sz;

		if (__old_sz > (size_t)_MAX_BYTES && __new_sz > (size_t)_MAX_BYTES) {
			return(realloc(__p, __new_sz));
		}
		if (_S_round_up(__old_sz) == _S_round_up(__new_sz)) return(__p);
		__result = allocate(__new_sz);
		__copy_sz = __new_sz > __old_sz ? __old_sz : __new_sz;
		memcpy(__result, __p, __copy_sz);
		deallocate(__p, __old_sz);
		return(__result);
	}
}

template <typename T>
void* myallocato<T>::_S_refill(size_t __n) {
	int __nobjs = 20;

	char* __chunk = _S_chunk_alloc(__n, __nobjs);
	_Obj* volatile* __my_free_list;
	_Obj* __result;
	_Obj* __current_obj;
	_Obj* __next_obj;
	int __i;

	if (1 == __nobjs) return(__chunk);	//����chunk��ռ�ֻ��һ������
	__my_free_list = _S_free_list + _S_freelist_index(__n);

	/* Build free list in chunk */
	__result = (_Obj*)__chunk;
	*__my_free_list = __next_obj = (_Obj*)(__chunk + __n);
	for (__i = 1; ; __i++) {
		__current_obj = __next_obj;
		__next_obj = (_Obj*)((char*)__next_obj + __n);
		if (__nobjs - 1 == __i) {
			__current_obj->_M_free_list_link = 0;
			break;
		}
		else {
			__current_obj->_M_free_list_link = __next_obj;
		}
	}
	return(__result);
}
