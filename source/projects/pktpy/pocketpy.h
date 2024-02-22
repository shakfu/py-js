/*
 *  Copyright (c) 2023 blueloveTH
 *  Distributed Under The MIT License
 *  https://github.com/pocketpy/pocketpy
 */

#ifndef POCKETPY_H
#define POCKETPY_H


#ifdef PK_USER_CONFIG_H

#include "user_config.h"

#else

/*************** feature settings ***************/

// Whether to compile os-related modules or not
#ifndef PK_ENABLE_OS                // can be overrided by cmake
#define PK_ENABLE_OS                0
#endif

// Enable this if you are working with multi-threading (experimental)
// This triggers necessary locks to make the VM thread-safe
#ifndef PK_ENABLE_THREAD            // can be overrided by cmake
#define PK_ENABLE_THREAD            0
#endif

// GC min threshold
#ifndef PK_GC_MIN_THRESHOLD         // can be overrided by cmake
#define PK_GC_MIN_THRESHOLD         32768
#endif

// Whether to use `std::function` to do bindings or not
// By default, functions to be binded must be a C function pointer without capture
// However, someone thinks it's not convenient.
// By setting this to 1, capturing lambdas can be binded,
// but it's slower and may cause severe "code bloat", also needs more time to compile.
#define PK_ENABLE_STD_FUNCTION      0

/*************** debug settings ***************/

// Enable this may help you find bugs
#define PK_DEBUG_EXTRA_CHECK        0

// Do not edit the following settings unless you know what you are doing
#define PK_DEBUG_NO_BUILTINS        0
#define PK_DEBUG_DIS_EXEC           0
#define PK_DEBUG_CEVAL_STEP         0
#define PK_DEBUG_FULL_EXCEPTION     0
#define PK_DEBUG_MEMORY_POOL        0
#define PK_DEBUG_NO_MEMORY_POOL     0
#define PK_DEBUG_NO_AUTO_GC         0
#define PK_DEBUG_GC_STATS           0

/*************** internal settings ***************/

// This is the maximum size of the value stack in void* units
// The actual size in bytes equals `sizeof(void*) * PK_VM_STACK_SIZE`
#define PK_VM_STACK_SIZE            32768

// This is the maximum number of local variables in a function
// (not recommended to change this / it should be less than 200)
#define PK_MAX_CO_VARNAMES          64

// Hash table load factor (smaller ones mean less collision but more memory)
// For class instance
#define PK_INST_ATTR_LOAD_FACTOR    0.67
// For class itself
#define PK_TYPE_ATTR_LOAD_FACTOR    0.5

#ifdef _WIN32
    #define PK_PLATFORM_SEP '\\'
#else
    #define PK_PLATFORM_SEP '/'
#endif

#ifdef _MSC_VER
#pragma warning (disable:4267)
#pragma warning (disable:4100)
#pragma warning (disable:4244)
#pragma warning (disable:4996)
#endif

#ifdef _MSC_VER
#define PK_ENABLE_COMPUTED_GOTO		0
#define PK_UNREACHABLE()			__assume(0);
#else
#define PK_ENABLE_COMPUTED_GOTO		1
#define PK_UNREACHABLE()			__builtin_unreachable();
#endif


#if PK_DEBUG_CEVAL_STEP && defined(PK_ENABLE_COMPUTED_GOTO)
#undef PK_ENABLE_COMPUTED_GOTO
#endif

#endif


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    //define something for Windows (32-bit and 64-bit, this part is common)
    #define PK_EXPORT __declspec(dllexport)
    #define PK_SYS_PLATFORM     0
#elif __EMSCRIPTEN__
    #define PK_EXPORT
    #define PK_SYS_PLATFORM     1
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
        // iOS, tvOS, or watchOS Simulator
        #define PK_SYS_PLATFORM     2
    #elif TARGET_OS_IPHONE
        // iOS, tvOS, or watchOS device
        #define PK_SYS_PLATFORM     2
    #elif TARGET_OS_MAC
        #define PK_SYS_PLATFORM     3
    #else
    #   error "Unknown Apple platform"
    #endif
    #define PK_EXPORT __attribute__((visibility("default")))
#elif __ANDROID__
    #define PK_EXPORT __attribute__((visibility("default")))
    #define PK_SYS_PLATFORM     4
#elif __linux__
    #define PK_EXPORT __attribute__((visibility("default")))
    #define PK_SYS_PLATFORM     5
#else
    #define PK_EXPORT
    #define PK_SYS_PLATFORM     6
#endif




#include <cmath>
#include <cstring>

#include <stdexcept>
#include <vector>
#include <string>
#include <chrono>
#include <string_view>
#include <memory>
#include <iostream>
#include <map>
#include <set>
#include <algorithm>
#include <variant>
#include <type_traits>
#include <random>
#include <deque>

#define PK_VERSION				"1.4.1"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

/*******************************************************************************/
#if PK_ENABLE_STD_FUNCTION
#include <functional>
#endif
/*******************************************************************************/

#if PK_ENABLE_THREAD
#define PK_THREAD_LOCAL thread_local
#include <mutex>

struct GIL {
	inline static std::mutex _mutex;
    explicit GIL() { _mutex.lock(); }
    ~GIL() { _mutex.unlock(); }
};
#define PK_GLOBAL_SCOPE_LOCK() GIL _lock;

#else
#define PK_THREAD_LOCAL
#define PK_GLOBAL_SCOPE_LOCK()
#endif

/*******************************************************************************/

#define PK_UNUSED(x) (void)(x)

#define PK_LOCAL_STATIC static

namespace pkpy{

namespace std = ::std;

template <size_t T>
struct NumberTraits;

template <>
struct NumberTraits<4> {
	using int_t = int32_t;
	using float_t = float;

	static constexpr int_t kMaxSmallInt = (1 << 28) - 1;
	static constexpr int_t kMinSmallInt = - (1 << 28);
	static constexpr float_t kEpsilon = (float_t)1e-4;
};

template <>
struct NumberTraits<8> {
	using int_t = int64_t;
	using float_t = double;

	static constexpr int_t kMaxSmallInt = (1ll << 60) - 1;
	static constexpr int_t kMinSmallInt = - (1ll << 60);
	static constexpr float_t kEpsilon = (float_t)1e-8;
};

using Number = NumberTraits<sizeof(void*)>;
using i64 = int64_t;		// always 64-bit
using f64 = Number::float_t;

template<size_t T>
union BitsCvtImpl;

template<>
union BitsCvtImpl<4>{
	NumberTraits<4>::int_t _int;
	NumberTraits<4>::float_t _float;

	// 1 + 8 + 23
	int sign() const { return _int >> 31; }
	unsigned int exp() const { return (_int >> 23) & 0b1111'1111; }
	uint64_t mantissa() const { return _int & 0x7fffff; }

	void set_exp(int exp) { _int = (_int & 0x807f'ffff) | (exp << 23); }
	void set_sign(int sign) { _int = (_int & 0x7fff'ffff) | (sign << 31); }
	void zero_mantissa() { _int &= 0xff80'0000; }

	static constexpr int C0 = 127;	// 2^7 - 1
	static constexpr int C1 = -62;	// 2 - 2^6
	static constexpr int C2 = 63;	// 2^6 - 1
	static constexpr NumberTraits<4>::int_t C3 = 0b1011'1111'1111'1111'1111'1111'1111'1111;
	static constexpr int C4 = 0b11111111;

	BitsCvtImpl(NumberTraits<4>::float_t val): _float(val) {}
	BitsCvtImpl(NumberTraits<4>::int_t val): _int(val) {}
};

template<>
union BitsCvtImpl<8>{
	NumberTraits<8>::int_t _int;
	NumberTraits<8>::float_t _float;

	// 1 + 11 + 52
	int sign() const { return _int >> 63; }
	unsigned int exp() const { return (_int >> 52) & 0b0111'1111'1111; }
	uint64_t mantissa() const { return _int & 0xfffffffffffff; }

	void set_exp(uint64_t exp) { _int = (_int & 0x800f'ffff'ffff'ffff) | (exp << 52); }
	void set_sign(uint64_t sign) { _int = (_int & 0x7fff'ffff'ffff'ffff) | (sign << 63); }
	void zero_mantissa() { _int &= 0xfff0'0000'0000'0000; }

	static constexpr int C0 = 1023;	// 2^10 - 1
	static constexpr int C1 = -510;	// 2 - 2^9
	static constexpr int C2 = 511;	// 2^9 - 1
	static constexpr NumberTraits<8>::int_t C3 = 0b1011'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111;
	static constexpr int C4 = 0b11111111111;

	BitsCvtImpl(NumberTraits<8>::float_t val): _float(val) {}
	BitsCvtImpl(NumberTraits<8>::int_t val): _int(val) {}
};

using BitsCvt = BitsCvtImpl<sizeof(void*)>;

static_assert(sizeof(i64) == 8);
static_assert(sizeof(Number::float_t) == sizeof(void*));
static_assert(sizeof(Number::int_t) == sizeof(void*));
static_assert(sizeof(BitsCvt) == sizeof(void*));
static_assert(std::numeric_limits<f64>::is_iec559);

struct Dummy { }; // for special objects: True, False, None, Ellipsis, etc.
struct DummyInstance { };
struct DummyModule { };
struct NoReturn { };
struct Discarded { };

struct Type {
	int index;
	constexpr Type(int index): index(index) {}
	bool operator==(Type other) const { return this->index == other.index; }
	bool operator!=(Type other) const { return this->index != other.index; }
	operator int() const { return this->index; }
};

#define PK_LAMBDA(x) ([](VM* vm, ArgsView args) { return x; })
#define PK_VAR_LAMBDA(x) ([](VM* vm, ArgsView args) { return VAR(x); })
#define PK_ACTION(x) ([](VM* vm, ArgsView args) { x; return vm->None; })

#ifdef POCKETPY_H
#define PK_FATAL_ERROR() throw std::runtime_error( "L" + std::to_string(__LINE__) + " FATAL_ERROR()!");
#else
#define PK_FATAL_ERROR() throw std::runtime_error( __FILE__ + std::string(":") + std::to_string(__LINE__) + " FATAL_ERROR()!");
#endif

#define PK_ASSERT(x) if(!(x)) PK_FATAL_ERROR();

struct PyObject;
#define PK_BITS(p) (reinterpret_cast<Number::int_t>(p))

inline PyObject* tag_float(f64 val){
	BitsCvt decomposed(val);
	// std::cout << "tagging: " << val << std::endl;
	int sign = decomposed.sign();
	int exp_7b = decomposed.exp() - BitsCvt::C0;
	if(exp_7b < BitsCvt::C1){
		exp_7b = BitsCvt::C1 - 1;	// -63 + 63 = 0
		decomposed.zero_mantissa();
	}else if(exp_7b > BitsCvt::C2){
		exp_7b = BitsCvt::C2 + 1;	// 64 + 63 = 127
		if(!std::isnan(val)) decomposed.zero_mantissa();
	}
	decomposed.set_exp(exp_7b + BitsCvt::C2);
	decomposed._int = (decomposed._int << 1) | 0b01;
	decomposed.set_sign(sign);
	return reinterpret_cast<PyObject*>(decomposed._int);
}

inline f64 untag_float(PyObject* val){
	BitsCvt decomposed(reinterpret_cast<Number::int_t>(val));
	// std::cout << "untagging: " << val << std::endl;
	decomposed._int = (decomposed._int >> 1) & BitsCvt::C3;
	unsigned int exp_7b = decomposed.exp();
	if(exp_7b == 0) return 0.0f;
	if(exp_7b == BitsCvt::C0){
		decomposed.set_exp(BitsCvt::C4);
		return decomposed._float;
	}
	decomposed.set_exp(exp_7b - BitsCvt::C2 + BitsCvt::C0);
	return decomposed._float;
}

// is_pod<> for c++17 and c++20
template<typename T>
struct is_pod {
	static constexpr bool value = std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;
};

#define PK_ALWAYS_PASS_BY_POINTER(T) \
	T(const T&) = delete; \
	T& operator=(const T&) = delete; \
	T(T&&) = delete; \
	T& operator=(T&&) = delete;

inline const char* kPlatformStrings[] = {
    "win32",        // 0
    "emscripten",   // 1
    "ios",          // 2
    "darwin",       // 3
    "android",      // 4
    "linux",        // 5
    "unknown"       // 6
};

} // namespace pkpy



namespace pkpy{

void* pool64_alloc(size_t);
void pool64_dealloc(void*);

void* pool128_alloc(size_t);
void pool128_dealloc(void*);

template<typename T>
void* pool64_alloc(){
    return pool64_alloc(sizeof(T));
}

template<typename T>
void* pool128_alloc(){
    return pool128_alloc(sizeof(T));
}

void pools_shrink_to_fit();

};  // namespace pkpy

namespace pkpy{

struct LinkedListNode{
    LinkedListNode* prev;
    LinkedListNode* next;
};

template<typename T>
struct DoubleLinkedList{
    static_assert(std::is_base_of_v<LinkedListNode, T>);
    int _size;
    LinkedListNode head;
    LinkedListNode tail;
    
    DoubleLinkedList(): _size(0){
        head.prev = nullptr;
        head.next = &tail;
        tail.prev = &head;
        tail.next = nullptr;
    }

    void push_back(T* node){
        node->prev = tail.prev;
        node->next = &tail;
        tail.prev->next = node;
        tail.prev = node;
        _size++;
    }

    void push_front(T* node){
        node->prev = &head;
        node->next = head.next;
        head.next->prev = node;
        head.next = node;
        _size++;
    }

    void pop_back(){
#if PK_DEBUG_MEMORY_POOL
        if(empty()) throw std::runtime_error("DoubleLinkedList::pop_back() called on empty list");
#endif
        tail.prev->prev->next = &tail;
        tail.prev = tail.prev->prev;
        _size--;
    }

    void pop_front(){
#if PK_DEBUG_MEMORY_POOL
        if(empty()) throw std::runtime_error("DoubleLinkedList::pop_front() called on empty list");
#endif
        head.next->next->prev = &head;
        head.next = head.next->next;
        _size--;
    }

    T* back() const {
#if PK_DEBUG_MEMORY_POOL
        if(empty()) throw std::runtime_error("DoubleLinkedList::back() called on empty list");
#endif
        return static_cast<T*>(tail.prev);
    }

    T* front() const {
#if PK_DEBUG_MEMORY_POOL
        if(empty()) throw std::runtime_error("DoubleLinkedList::front() called on empty list");
#endif
        return static_cast<T*>(head.next);
    }

    void erase(T* node){
#if PK_DEBUG_MEMORY_POOL
        if(empty()) throw std::runtime_error("DoubleLinkedList::erase() called on empty list");
        LinkedListNode* n = head.next;
        while(n != &tail){
            if(n == node) break;
            n = n->next;
        }
        if(n != node) throw std::runtime_error("DoubleLinkedList::erase() called on node not in the list");
#endif
        node->prev->next = node->next;
        node->next->prev = node->prev;
        _size--;
    }

    // void move_all_back(DoubleLinkedList<T>& other){
    //     if(other.empty()) return;
    //     other.tail.prev->next = &tail;
    //     tail.prev->next = other.head.next;
    //     other.head.next->prev = tail.prev;
    //     tail.prev = other.tail.prev;
    //     _size += other._size;
    //     other.head.next = &other.tail;
    //     other.tail.prev = &other.head;
    //     other._size = 0;
    // }

    bool empty() const {
#if PK_DEBUG_MEMORY_POOL
        if(size() == 0){
            if(head.next != &tail || tail.prev != &head){
                throw std::runtime_error("DoubleLinkedList::size() returned 0 but the list is not empty");
            }
            return true;
        }
#endif
        return _size == 0;
    }

    int size() const { return _size; }

    template<typename Func>
    void apply(Func func){
        LinkedListNode* p = head.next;
        while(p != &tail){
            LinkedListNode* next = p->next;
            func(static_cast<T*>(p));
            p = next;
        }
    }
};

template<int __BlockSize=128>
struct MemoryPool{
    static const size_t __MaxBlocks = 256*1024 / __BlockSize;
    static const size_t __MinArenaCount = PK_GC_MIN_THRESHOLD*100 / (256*1024);

    struct Block{
        void* arena;
        char data[__BlockSize];
    };

    struct Arena: LinkedListNode{
        Block _blocks[__MaxBlocks];
        Block* _free_list[__MaxBlocks];
        int _free_list_size;
        
        Arena(): _free_list_size(__MaxBlocks) {
            for(int i=0; i<__MaxBlocks; i++){
                _blocks[i].arena = this;
                _free_list[i] = &_blocks[i];
            }
        }

        bool empty() const { return _free_list_size == 0; }
        bool full() const { return _free_list_size == __MaxBlocks; }

        size_t allocated_size() const{
            return __BlockSize * (__MaxBlocks - _free_list_size);
        }

        Block* alloc(){
#if PK_DEBUG_MEMORY_POOL
            if(empty()) throw std::runtime_error("Arena::alloc() called on empty arena");
#endif
            _free_list_size--;
            return _free_list[_free_list_size];
        }

        void dealloc(Block* block){
#if PK_DEBUG_MEMORY_POOL
            if(full()) throw std::runtime_error("Arena::dealloc() called on full arena");
#endif
            _free_list[_free_list_size] = block;
            _free_list_size++;
        }
    };

    MemoryPool() = default;
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
    MemoryPool(MemoryPool&&) = delete;
    MemoryPool& operator=(MemoryPool&&) = delete;

    DoubleLinkedList<Arena> _arenas;
    DoubleLinkedList<Arena> _empty_arenas;

    void* alloc(size_t size){
        PK_GLOBAL_SCOPE_LOCK();
#if PK_DEBUG_NO_MEMORY_POOL
        return malloc(size);
#endif
        if(size > __BlockSize){
            void* p = malloc(sizeof(void*) + size);
            memset(p, 0, sizeof(void*));
            return (char*)p + sizeof(void*);
        }

        if(_arenas.empty()){
            // std::cout << _arenas.size() << ',' << _empty_arenas.size() << ',' << _full_arenas.size() << std::endl;
            _arenas.push_back(new Arena());
        }
        Arena* arena = _arenas.back();
        void* p = arena->alloc()->data;
        if(arena->empty()){
            _arenas.pop_back();
            _empty_arenas.push_back(arena);
        }
        return p;
    }

    void dealloc(void* p){
        PK_GLOBAL_SCOPE_LOCK();
#if PK_DEBUG_NO_MEMORY_POOL
        free(p);
        return;
#endif
#if PK_DEBUG_MEMORY_POOL
        if(p == nullptr) throw std::runtime_error("MemoryPool::dealloc() called on nullptr");
#endif
        Block* block = (Block*)((char*)p - sizeof(void*));
        if(block->arena == nullptr){
            free(block);
        }else{
            Arena* arena = (Arena*)block->arena;
            if(arena->empty()){
                _empty_arenas.erase(arena);
                _arenas.push_front(arena);
                arena->dealloc(block);
            }else{
                arena->dealloc(block);
            }
        }
    }

    void shrink_to_fit(){
        PK_GLOBAL_SCOPE_LOCK();
        if(_arenas.size() < __MinArenaCount) return;
        _arenas.apply([this](Arena* arena){
            if(arena->full()){
                _arenas.erase(arena);
                delete arena;
            }
        });
    }

    ~MemoryPool(){
        _arenas.apply([](Arena* arena){ delete arena; });
        _empty_arenas.apply([](Arena* arena){ delete arena; });
    }
};

static MemoryPool<64> pool64;
static MemoryPool<128> pool128;

void* pool64_alloc(size_t size){ return pool64.alloc(size); }
void pool64_dealloc(void* p){ pool64.dealloc(p); }

void* pool128_alloc(size_t size){ return pool128.alloc(size); }
void pool128_dealloc(void* p){ pool128.dealloc(p); }

void pools_shrink_to_fit(){
    pool64.shrink_to_fit();
    pool128.shrink_to_fit();
}

}


namespace pkpy{

template<typename T>
struct pod_vector{
    static_assert(64 % sizeof(T) == 0);
    static_assert(is_pod<T>::value);
    static constexpr int N = 64 / sizeof(T);
    static_assert(N >= 4);
    int _size;
    int _capacity;
    T* _data;

    pod_vector(): _size(0), _capacity(N) {
        _data = (T*)pool64_alloc(_capacity * sizeof(T));
    }

    pod_vector(int size): _size(size), _capacity(std::max(N, size)) {
        _data = (T*)pool64_alloc(_capacity * sizeof(T));
    }

    pod_vector(const pod_vector& other): _size(other._size), _capacity(other._capacity) {
        _data = (T*)pool64_alloc(_capacity * sizeof(T));
        memcpy(_data, other._data, sizeof(T) * _size);
    }

    pod_vector(pod_vector&& other) noexcept {
        _size = other._size;
        _capacity = other._capacity;
        _data = other._data;
        other._data = nullptr;
    }

    pod_vector& operator=(pod_vector&& other) noexcept {
        if(_data!=nullptr) pool64_dealloc(_data);
        _size = other._size;
        _capacity = other._capacity;
        _data = other._data;
        other._data = nullptr;
        return *this;
    }

    // remove copy assignment
    pod_vector& operator=(const pod_vector& other) = delete;

    template<typename __ValueT>
    void push_back(__ValueT&& t) {
        if (_size == _capacity) reserve(_capacity*2);
        _data[_size++] = std::forward<__ValueT>(t);
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (_size == _capacity) reserve(_capacity*2);
        new (&_data[_size++]) T(std::forward<Args>(args)...);
    }

    void reserve(int cap){
        if(cap <= _capacity) return;
        _capacity = cap;
        T* old_data = _data;
        _data = (T*)pool64_alloc(_capacity * sizeof(T));
        if(old_data != nullptr){
            memcpy(_data, old_data, sizeof(T) * _size);
            pool64_dealloc(old_data);
        }
    }

    void pop_back() { _size--; }
    T popx_back() { T t = std::move(_data[_size-1]); _size--; return t; }
    
    void extend(const pod_vector& other){
        for(int i=0; i<other.size(); i++) push_back(other[i]);
    }

    void extend(const T* begin, const T* end){
        for(auto it=begin; it!=end; it++) push_back(*it);
    }

    T& operator[](int index) { return _data[index]; }
    const T& operator[](int index) const { return _data[index]; }

    T* begin() { return _data; }
    T* end() { return _data + _size; }
    const T* begin() const { return _data; }
    const T* end() const { return _data + _size; }
    T& back() { return _data[_size - 1]; }
    const T& back() const { return _data[_size - 1]; }

    bool empty() const { return _size == 0; }
    int size() const { return _size; }
    T* data() { return _data; }
    const T* data() const { return _data; }
    void clear() { _size=0; }

    template<typename __ValueT>
    void insert(int i, __ValueT&& val){
        if (_size == _capacity) reserve(_capacity*2);
        for(int j=_size; j>i; j--) _data[j] = _data[j-1];
        _data[i] = std::forward<__ValueT>(val);
        _size++;
    }

    void erase(int i){
        for(int j=i; j<_size-1; j++) _data[j] = _data[j+1];
        _size--;
    }

    void reverse(){
        std::reverse(_data, _data+_size);
    }

    void resize(int size){
        if(size > _capacity) reserve(size);
        _size = size;
    }

    std::pair<T*, int> detach() noexcept {
        T* p = _data;
        int size = _size;
        _data = nullptr;
        _size = 0;
        return {p, size};
    }

    ~pod_vector() {
        if(_data != nullptr) pool64_dealloc(_data);
    }
};


template <typename T, typename Container=std::vector<T>>
class stack{
	Container vec;
public:
	void push(const T& t){ vec.push_back(t); }
	void push(T&& t){ vec.push_back(std::move(t)); }
    template<typename... Args>
    void emplace(Args&&... args){
        vec.emplace_back(std::forward<Args>(args)...);
    }
	void pop(){ vec.pop_back(); }
	void clear(){ vec.clear(); }
	bool empty() const { return vec.empty(); }
	size_t size() const { return vec.size(); }
	T& top(){ return vec.back(); }
	const T& top() const { return vec.back(); }
	T popx(){ T t = std::move(vec.back()); vec.pop_back(); return t; }
    void reserve(int n){ vec.reserve(n); }
	Container& data() { return vec; }
};

template <typename T, typename Container=std::vector<T>>
class stack_no_copy: public stack<T, Container>{
public:
    stack_no_copy() = default;
    stack_no_copy(const stack_no_copy& other) = delete;
    stack_no_copy& operator=(const stack_no_copy& other) = delete;
    stack_no_copy(stack_no_copy&& other) noexcept = default;
    stack_no_copy& operator=(stack_no_copy&& other) noexcept = default;
};

} // namespace pkpy


namespace pkpy {

int utf8len(unsigned char c, bool suppress=false);
struct SStream;

struct Str{
    int size;
    bool is_ascii;
    char* data;
    char _inlined[24];

    bool is_inlined() const { return data == _inlined; }

    Str();
    Str(int size, bool is_ascii);
    Str(const std::string& s);
    Str(std::string_view s);
    Str(const char* s);
    Str(const char* s, int len);
    Str(std::pair<char *, int>);
    Str(const Str& other);
    Str(Str&& other);

    operator std::string_view() const { return sv(); }

    const char* begin() const { return data; }
    const char* end() const { return data + size; }
    char operator[](int idx) const { return data[idx]; }
    int length() const { return size; }
    bool empty() const { return size == 0; }
    size_t hash() const{ return std::hash<std::string_view>()(sv()); }

    Str& operator=(const Str& other);
    Str operator+(const Str& other) const;
    friend Str operator+(const char* p, const Str& str);
    Str operator+(const char* p) const;

    bool operator==(const std::string_view other) const;
    bool operator!=(const std::string_view other) const;
    bool operator<(const std::string_view other) const;
    friend bool operator<(const std::string_view other, const Str& str);

    bool operator==(const char* p) const;
    bool operator!=(const char* p) const;

    bool operator==(const Str& other) const;
    bool operator!=(const Str& other) const;
    bool operator<(const Str& other) const;
    bool operator>(const Str& other) const;
    bool operator<=(const Str& other) const;
    bool operator>=(const Str& other) const;

    ~Str();

    friend std::ostream& operator<<(std::ostream& os, const Str& str);

    Str substr(int start, int len) const;
    Str substr(int start) const;
    const char* c_str() const;
    std::string_view sv() const;
    std::string str() const;
    Str strip(bool left, bool right, const Str& chars) const;
    Str strip(bool left=true, bool right=true) const;
    Str lstrip() const { return strip(true, false); }
    Str rstrip() const { return strip(false, true); }
    Str lower() const;
    Str upper() const;
    Str escape(bool single_quote=true) const;
    void escape_(SStream& ss, bool single_quote=true) const;
    int index(const Str& sub, int start=0) const;
    Str replace(char old, char new_) const;
    Str replace(const Str& old, const Str& new_, int count=-1) const;
    std::vector<std::string_view> split(const Str& sep) const;
    std::vector<std::string_view> split(char sep) const;
    int count(const Str& sub) const;

    /*************unicode*************/
    int _unicode_index_to_byte(int i) const;
    int _byte_index_to_unicode(int n) const;
    Str u8_getitem(int i) const;
    Str u8_slice(int start, int stop, int step) const;
    int u8_length() const;
};

struct StrName {
    uint16_t index;
    StrName();
    explicit StrName(uint16_t index);
    StrName(const char* s);
    StrName(const Str& s);
    std::string_view sv() const;
    const char* c_str() const;
    bool empty() const { return index == 0; }

    Str escape() const;

    bool operator==(const StrName& other) const noexcept {
        return this->index == other.index;
    }

    bool operator!=(const StrName& other) const noexcept {
        return this->index != other.index;
    }

    bool operator<(const StrName& other) const noexcept {
        return this->index < other.index;
    }

    bool operator>(const StrName& other) const noexcept {
        return this->index > other.index;
    }

    static bool is_valid(int index);
    static StrName get(std::string_view s);
    static std::map<std::string, uint16_t, std::less<>>& _interned();
    static std::map<uint16_t, std::string>& _r_interned();
    static uint32_t _pesudo_random_index;
};

struct SStream{
    PK_ALWAYS_PASS_BY_POINTER(SStream)
    // pod_vector<T> is allocated by pool64 so the buffer can be moved into Str without a copy
    pod_vector<char> buffer;
    int _precision = -1;

    bool empty() const { return buffer.empty(); }
    void setprecision(int precision) { _precision = precision; }

    SStream(){}
    SStream(int guess_size){ buffer.reserve(guess_size); }

    Str str();

    SStream& operator<<(const Str&);
    SStream& operator<<(const char*);
    SStream& operator<<(int);
    SStream& operator<<(size_t);
    SStream& operator<<(i64);
    SStream& operator<<(f64);
    SStream& operator<<(const std::string&);
    SStream& operator<<(std::string_view);
    SStream& operator<<(char);
    SStream& operator<<(StrName);

    void write_hex(unsigned char, bool non_zero=false);
    void write_hex(void*);
    void write_hex(i64);
};

template<typename... Args>
Str _S(Args&&... args) {
    SStream ss;
    (ss << ... << args);
    return ss.str();
}

struct CString{
	const char* ptr;
	CString(const char* ptr): ptr(ptr) {}
    operator const char*() const { return ptr; }
};

// unary operators
const StrName __repr__ = StrName::get("__repr__");
const StrName __str__ = StrName::get("__str__");
const StrName __hash__ = StrName::get("__hash__");      // unused
const StrName __len__ = StrName::get("__len__");
const StrName __iter__ = StrName::get("__iter__");
const StrName __next__ = StrName::get("__next__");      // unused
const StrName __neg__ = StrName::get("__neg__");        // unused
const StrName __bool__ = StrName::get("__bool__");      // unused
// logical operators
const StrName __eq__ = StrName::get("__eq__");
const StrName __lt__ = StrName::get("__lt__");
const StrName __le__ = StrName::get("__le__");
const StrName __gt__ = StrName::get("__gt__");
const StrName __ge__ = StrName::get("__ge__");
const StrName __contains__ = StrName::get("__contains__");
// binary operators
const StrName __add__ = StrName::get("__add__");
const StrName __radd__ = StrName::get("__radd__");
const StrName __sub__ = StrName::get("__sub__");
const StrName __rsub__ = StrName::get("__rsub__");
const StrName __mul__ = StrName::get("__mul__");
const StrName __rmul__ = StrName::get("__rmul__");
const StrName __truediv__ = StrName::get("__truediv__");
const StrName __floordiv__ = StrName::get("__floordiv__");
const StrName __mod__ = StrName::get("__mod__");
const StrName __pow__ = StrName::get("__pow__");
const StrName __matmul__ = StrName::get("__matmul__");
const StrName __lshift__ = StrName::get("__lshift__");
const StrName __rshift__ = StrName::get("__rshift__");
const StrName __and__ = StrName::get("__and__");
const StrName __or__ = StrName::get("__or__");
const StrName __xor__ = StrName::get("__xor__");
const StrName __invert__ = StrName::get("__invert__");
// indexer
const StrName __getitem__ = StrName::get("__getitem__");
const StrName __setitem__ = StrName::get("__setitem__");
const StrName __delitem__ = StrName::get("__delitem__");

// specials
const StrName __new__ = StrName::get("__new__");
const StrName __init__ = StrName::get("__init__");
const StrName __call__ = StrName::get("__call__");
const StrName __divmod__ = StrName::get("__divmod__");
const StrName __enter__ = StrName::get("__enter__");
const StrName __exit__ = StrName::get("__exit__");
const StrName __name__ = StrName::get("__name__");
const StrName __all__ = StrName::get("__all__");
const StrName __package__ = StrName::get("__package__");
const StrName __path__ = StrName::get("__path__");
const StrName __class__ = StrName::get("__class__");

const StrName pk_id_add = StrName::get("add");
const StrName pk_id_set = StrName::get("set");
const StrName pk_id_long = StrName::get("long");
const StrName pk_id_complex = StrName::get("complex");

#define DEF_SNAME(name) const static StrName name(#name)

} // namespace pkpy
namespace pkpy {

int utf8len(unsigned char c, bool suppress){
    if((c & 0b10000000) == 0) return 1;
    if((c & 0b11100000) == 0b11000000) return 2;
    if((c & 0b11110000) == 0b11100000) return 3;
    if((c & 0b11111000) == 0b11110000) return 4;
    if((c & 0b11111100) == 0b11111000) return 5;
    if((c & 0b11111110) == 0b11111100) return 6;
    if(!suppress) throw std::runtime_error("invalid utf8 char: " + std::to_string(c));
    return 0;
}

#define PK_STR_ALLOCATE()                                   \
        if(this->size < sizeof(this->_inlined)){            \
            this->data = this->_inlined;                    \
        }else{                                              \
            this->data = (char*)pool64_alloc(this->size+1); \
        }

#define PK_STR_COPY_INIT(__s)  \
        for(int i=0; i<this->size; i++){                    \
            this->data[i] = __s[i];                         \
            if(!isascii(__s[i])) is_ascii = false;          \
        }                                                   \
        this->data[this->size] = '\0';

    Str::Str(): size(0), is_ascii(true), data(_inlined) {
        _inlined[0] = '\0';
    }

    Str::Str(int size, bool is_ascii): size(size), is_ascii(is_ascii) {
        PK_STR_ALLOCATE()
    }

    Str::Str(const std::string& s): size(s.size()), is_ascii(true) {
        PK_STR_ALLOCATE()
        PK_STR_COPY_INIT(s)
    }

    Str::Str(std::string_view s): size(s.size()), is_ascii(true) {
        PK_STR_ALLOCATE()
        PK_STR_COPY_INIT(s)
    }

    Str::Str(const char* s): size(strlen(s)), is_ascii(true) {
        PK_STR_ALLOCATE()
        PK_STR_COPY_INIT(s)
    }

    Str::Str(const char* s, int len): size(len), is_ascii(true) {
        PK_STR_ALLOCATE()
        PK_STR_COPY_INIT(s)
    }

    Str::Str(std::pair<char *, int> detached): size(detached.second), is_ascii(true) {
        this->data = detached.first;
        for(int i=0; i<size; i++){
            if(!isascii(data[i])){ is_ascii = false; break; }
        }
        PK_ASSERT(data[size] == '\0');
    }

    Str::Str(const Str& other): size(other.size), is_ascii(other.is_ascii) {
        PK_STR_ALLOCATE()
        memcpy(data, other.data, size);
        data[size] = '\0';
    }

    Str::Str(Str&& other): size(other.size), is_ascii(other.is_ascii) {
        if(other.is_inlined()){
            data = _inlined;
            for(int i=0; i<size; i++) _inlined[i] = other._inlined[i];
            data[size] = '\0';
        }else{
            data = other.data;
            // zero out `other`
            other.data = other._inlined;
            other.data[0] = '\0';
            other.size = 0;
        }
    }

    Str operator+(const char* p, const Str& str){
        Str other(p);
        return other + str;
    }

    std::ostream& operator<<(std::ostream& os, const Str& str){
        return os << str.sv();
    }

    bool operator<(const std::string_view other, const Str& str){
        return other < str.sv();
    }

    Str& Str::operator=(const Str& other){
        if(!is_inlined()) pool64_dealloc(data);
        size = other.size;
        is_ascii = other.is_ascii;
        PK_STR_ALLOCATE()
        memcpy(data, other.data, size);
        data[size] = '\0';
        return *this;
    }

    Str Str::operator+(const Str& other) const {
        Str ret(size + other.size, is_ascii && other.is_ascii);
        memcpy(ret.data, data, size);
        memcpy(ret.data + size, other.data, other.size);
        ret.data[ret.size] = '\0';
        return ret;
    }

    Str Str::operator+(const char* p) const {
        Str other(p);
        return *this + other;
    }

    bool Str::operator==(const Str& other) const {
        if(size != other.size) return false;
        return memcmp(data, other.data, size) == 0;
    }

    bool Str::operator!=(const Str& other) const {
        if(size != other.size) return true;
        return memcmp(data, other.data, size) != 0;
    }

    bool Str::operator==(const std::string_view other) const {
        if(size != (int)other.size()) return false;
        return memcmp(data, other.data(), size) == 0;
    }

    bool Str::operator!=(const std::string_view other) const {
        if(size != (int)other.size()) return true;
        return memcmp(data, other.data(), size) != 0;
    }

    bool Str::operator==(const char* p) const {
        return *this == std::string_view(p);
    }

    bool Str::operator!=(const char* p) const {
        return *this != std::string_view(p);
    }

    bool Str::operator<(const Str& other) const {
        return this->sv() < other.sv();
    }

    bool Str::operator<(const std::string_view other) const {
        return this->sv() < other;
    }

    bool Str::operator>(const Str& other) const {
        return this->sv() > other.sv();
    }

    bool Str::operator<=(const Str& other) const {
        return this->sv() <= other.sv();
    }

    bool Str::operator>=(const Str& other) const {
        return this->sv() >= other.sv();
    }

    Str::~Str(){
        if(!is_inlined()) pool64_dealloc(data);
    }

    Str Str::substr(int start, int len) const {
        Str ret(len, is_ascii);
        memcpy(ret.data, data + start, len);
        ret.data[len] = '\0';
        return ret;
    }

    Str Str::substr(int start) const {
        return substr(start, size - start);
    }

    const char* Str::c_str() const{
        return data;
    }

    std::string_view Str::sv() const {
        return std::string_view(data, size);
    }

    std::string Str::str() const {
        return std::string(data, size);
    }

    Str Str::strip(bool left, bool right, const Str& chars) const {
        int L = 0;
        int R = u8_length();
        if(left){
            while(L < R && chars.index(u8_getitem(L)) != -1) L++;
        }
        if(right){
            while(L < R && chars.index(u8_getitem(R-1)) != -1) R--;
        }
        return u8_slice(L, R, 1);
    }

    Str Str::strip(bool left, bool right) const {
        if(is_ascii){
            int L = 0;
            int R = size;
            if(left){
                while(L < R && (data[L] == ' ' || data[L] == '\t' || data[L] == '\n' || data[L] == '\r')) L++;
            }
            if(right){
                while(L < R && (data[R-1] == ' ' || data[R-1] == '\t' || data[R-1] == '\n' || data[R-1] == '\r')) R--;
            }
            return substr(L, R - L);
        }else{
            return strip(left, right, " \t\n\r");
        }
    }

    Str Str::lower() const{
        std::string copy(data, size);
        std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c){
            if('A' <= c && c <= 'Z') return c + ('a' - 'A');
            return (int)c;
        });
        return Str(copy);
    }

    Str Str::upper() const{
        std::string copy(data, size);
        std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c){
            if('a' <= c && c <= 'z') return c - ('a' - 'A');
            return (int)c;
        });
        return Str(copy);
    }

    Str Str::escape(bool single_quote) const{
        SStream ss;
        escape_(ss, single_quote);
        return ss.str();
    }

    void Str::escape_(SStream& ss, bool single_quote) const {
        ss << (single_quote ? '\'' : '"');
        for (int i=0; i<length(); i++) {
            char c = this->operator[](i);
            switch (c) {
                case '"':
                    if(!single_quote) ss << '\\';
                    ss << '"';
                    break;
                case '\'':
                    if(single_quote) ss << '\\';
                    ss << '\'';
                    break;
                case '\\': ss << '\\' << '\\'; break;
                case '\n': ss << "\\n"; break;
                case '\r': ss << "\\r"; break;
                case '\t': ss << "\\t"; break;
                case '\b': ss << "\\b"; break;
                default:
                    if ('\x00' <= c && c <= '\x1f') {
                        ss << "\\x"; // << std::hex << std::setw(2) << std::setfill('0') << (int)c;
                        ss << "0123456789abcdef"[c >> 4];
                        ss << "0123456789abcdef"[c & 0xf];
                    } else {
                        ss << c;
                    }
            }
        }
        ss << (single_quote ? '\'' : '"');
    }

    int Str::index(const Str& sub, int start) const {
        auto p = std::search(data + start, data + size, sub.data, sub.data + sub.size);
        if(p == data + size) return -1;
        return p - data;
    }

    Str Str::replace(char old, char new_) const{
        Str copied = *this;
        for(int i=0; i<copied.size; i++){
            if(copied.data[i] == old) copied.data[i] = new_;
        }
        return copied;
    }

    Str Str::replace(const Str& old, const Str& new_, int count) const {
        SStream ss;
        int start = 0;
        while(true){
            int i = index(old, start);
            if(i == -1) break;
            ss << substr(start, i - start);
            ss << new_;
            start = i + old.size;
            if(count != -1 && --count == 0) break;
        }
        ss << substr(start, size - start);
        return ss.str();
    }


    int Str::_unicode_index_to_byte(int i) const{
        if(is_ascii) return i;
        int j = 0;
        while(i > 0){
            j += utf8len(data[j]);
            i--;
        }
        return j;
    }

    int Str::_byte_index_to_unicode(int n) const{
        if(is_ascii) return n;
        int cnt = 0;
        for(int i=0; i<n; i++){
            if((data[i] & 0xC0) != 0x80) cnt++;
        }
        return cnt;
    }

    Str Str::u8_getitem(int i) const{
        i = _unicode_index_to_byte(i);
        return substr(i, utf8len(data[i]));
    }

    Str Str::u8_slice(int start, int stop, int step) const{
        SStream ss;
        if(is_ascii){
            for(int i=start; step>0?i<stop:i>stop; i+=step) ss << data[i];
        }else{
            for(int i=start; step>0?i<stop:i>stop; i+=step) ss << u8_getitem(i);
        }
        return ss.str();
    }

    int Str::u8_length() const {
        return _byte_index_to_unicode(size);
    }

    std::vector<std::string_view> Str::split(const Str& sep) const{
        std::vector<std::string_view> result;
        std::string_view tmp;
        int start = 0;
        while(true){
            int i = index(sep, start);
            if(i == -1) break;
            tmp = sv().substr(start, i - start);
            if(!tmp.empty()) result.push_back(tmp);
            start = i + sep.size;
        }
        tmp = sv().substr(start, size - start);
        if(!tmp.empty()) result.push_back(tmp);
        return result;
    }

    std::vector<std::string_view> Str::split(char sep) const{
        std::vector<std::string_view> result;
        int i = 0;
        for(int j = 0; j < size; j++){
            if(data[j] == sep){
                if(j > i) result.emplace_back(data+i, j-i);
                i = j + 1;
                continue;
            }
        }
        if(size > i) result.emplace_back(data+i, size-i);
        return result;
    }

    int Str::count(const Str& sub) const{
        if(sub.empty()) return size + 1;
        int cnt = 0;
        int start = 0;
        while(true){
            int i = index(sub, start);
            if(i == -1) break;
            cnt++;
            start = i + sub.size;
        }
        return cnt;
    }

    std::map<std::string, uint16_t, std::less<>>& StrName::_interned(){
        static std::map<std::string, uint16_t, std::less<>> interned;
        return interned;
    }

    std::map<uint16_t, std::string>& StrName::_r_interned(){
        static std::map<uint16_t, std::string> r_interned;
        return r_interned;
    }

    uint32_t StrName::_pesudo_random_index = 0;

    StrName StrName::get(std::string_view s){
        auto it = _interned().find(s);
        if(it != _interned().end()) return StrName(it->second);
        // generate new index
        // https://github.com/python/cpython/blob/3.12/Objects/dictobject.c#L175
        uint16_t index = ((_pesudo_random_index*5) + 1) & 65535;
        if(index == 0) throw std::runtime_error("StrName index overflow");
        _interned()[std::string(s)] = index;
        if(is_valid(index)) throw std::runtime_error("StrName index conflict");
        _r_interned()[index] = std::string(s);
        _pesudo_random_index = index;
        return StrName(index);
    }

    Str StrName::escape() const {
        return Str(sv()).escape();
    }

    bool StrName::is_valid(int index) {
        return _r_interned().find(index) != _r_interned().end();
    }

    StrName::StrName(): index(0) {}
    StrName::StrName(uint16_t index): index(index) {}
    StrName::StrName(const char* s): index(get(s).index) {}
    StrName::StrName(const Str& s){
        index = get(s.sv()).index;
    }

    std::string_view StrName::sv() const {
        const std::string& str = _r_interned()[index];
        return std::string_view(str);
    }

    const char* StrName::c_str() const{
        const std::string& str = _r_interned()[index];
        return str.c_str();
    }

    Str SStream::str(){
        // after this call, the buffer is no longer valid
        buffer.reserve(buffer.size() + 1);  // allocate one more byte for '\0'
        buffer[buffer.size()] = '\0';       // set '\0'
        return Str(buffer.detach());
    }

    SStream& SStream::operator<<(const Str& s){
        buffer.extend(s.begin(), s.end());
        return *this;
    }

    SStream& SStream::operator<<(const char* s){
        buffer.extend(s, s + strlen(s));
        return *this;
    }

    SStream& SStream::operator<<(const std::string& s){
        buffer.extend(s.data(), s.data() + s.size());
        return *this;
    }

    SStream& SStream::operator<<(std::string_view s){
        buffer.extend(s.data(), s.data() + s.size());
        return *this;
    }

    SStream& SStream::operator<<(char c){
        buffer.push_back(c);
        return *this;
    }

    SStream& SStream::operator<<(StrName sn){
        return *this << sn.sv();
    }

    SStream& SStream::operator<<(size_t val){
        // size_t could be out of range of `i64`, use `std::to_string` instead
        return (*this) << std::to_string(val);
    }

    SStream& SStream::operator<<(int val){
        return (*this) << static_cast<i64>(val);
    }

    SStream& SStream::operator<<(i64 val){
        // str(-2**64).__len__() == 21
        buffer.reserve(buffer.size() + 24);
        if(val == 0){
            buffer.push_back('0');
            return *this;
        }
        if(val < 0){
            buffer.push_back('-');
            val = -val;
        }
        char* begin = buffer.end();
        while(val){
            buffer.push_back('0' + val % 10);
            val /= 10;
        }
        std::reverse(begin, buffer.end());
        return *this;
    }

    SStream& SStream::operator<<(f64 val){
        if(std::isinf(val) || std::isnan(val)){
            return (*this) << std::to_string(val);
        }
        char b[32];
        if(_precision == -1){
            int prec = std::numeric_limits<f64>::max_digits10-1;
            snprintf(b, sizeof(b), "%.*g", prec, val);
        }else{
            int prec = _precision;
            snprintf(b, sizeof(b), "%.*f", prec, val);
        }
        (*this) << b;
        if(std::all_of(b+1, b+strlen(b), isdigit)) (*this) << ".0";
        return *this;
    }

    void SStream::write_hex(unsigned char c, bool non_zero){
        unsigned char high = c >> 4;
        unsigned char low = c & 0xf;
        if(non_zero){
            if(high) (*this) << "0123456789abcdef"[high];
            if(high || low) (*this) << "0123456789abcdef"[low];
        }else{
            (*this) << "0123456789abcdef"[high];
            (*this) << "0123456789abcdef"[low];
        }
    }

    void SStream::write_hex(void* p){
        if(p == nullptr){
            (*this) << "0x0";
            return;
        }
        (*this) << "0x";
        uintptr_t p_t = reinterpret_cast<uintptr_t>(p);
        bool non_zero = true;
        for(int i=sizeof(void*)-1; i>=0; i--){
            unsigned char cpnt = (p_t >> (i * 8)) & 0xff;
            write_hex(cpnt, non_zero);
            if(cpnt != 0) non_zero = false;
        }
    }

    void SStream::write_hex(i64 val){
        if(val == 0){
            (*this) << "0x0";
            return;
        }
        if(val < 0){
            (*this) << "-";
            val = -val;
        }
        (*this) << "0x";
        bool non_zero = true;
        for(int i=56; i>=0; i-=8){
            unsigned char cpnt = (val >> i) & 0xff;
            write_hex(cpnt, non_zero);
            if(cpnt != 0) non_zero = false;
        }
    }

#undef PK_STR_ALLOCATE
#undef PK_STR_COPY_INIT

} // namespace pkpy


namespace pkpy {

using List = pod_vector<PyObject*>;

struct Tuple {
    PyObject** _args;
    PyObject* _inlined[3];
    int _size;

    Tuple(int n);
    Tuple(const Tuple& other);
    Tuple(Tuple&& other) noexcept;
    Tuple(List&& other) noexcept;
    ~Tuple();

    Tuple(PyObject*, PyObject*);
    Tuple(PyObject*, PyObject*, PyObject*);

    bool is_inlined() const { return _args == _inlined; }
    PyObject*& operator[](int i){ return _args[i]; }
    PyObject* operator[](int i) const { return _args[i]; }

    int size() const { return _size; }

    PyObject** begin() const { return _args; }
    PyObject** end() const { return _args + _size; }
};

// a lightweight view for function args, it does not own the memory
struct ArgsView{
    PyObject** _begin;
    PyObject** _end;

    ArgsView(PyObject** begin, PyObject** end) : _begin(begin), _end(end) {}
    ArgsView(const Tuple& t) : _begin(t.begin()), _end(t.end()) {}

    PyObject** begin() const { return _begin; }
    PyObject** end() const { return _end; }
    int size() const { return _end - _begin; }
    bool empty() const { return _begin == _end; }
    PyObject* operator[](int i) const { return _begin[i]; }

    List to_list() const;
    Tuple to_tuple() const;
};

}   // namespace pkpy
namespace pkpy {

Tuple::Tuple(int n){
    if(n <= 3){
        this->_args = _inlined;
    }else{
        this->_args = (PyObject**)pool64_alloc(n * sizeof(void*));
    }
    this->_size = n;
}

Tuple::Tuple(const Tuple& other): Tuple(other._size){
    for(int i=0; i<_size; i++) _args[i] = other._args[i];
}

Tuple::Tuple(Tuple&& other) noexcept {
    _size = other._size;
    if(other.is_inlined()){
        _args = _inlined;
        for(int i=0; i<_size; i++) _args[i] = other._args[i];
    }else{
        _args = other._args;
        other._args = other._inlined;
        other._size = 0;
    }
}

Tuple::Tuple(List&& other) noexcept {
    _size = other.size();
    _args = other._data;
    other._data = nullptr;
}

Tuple::Tuple(PyObject* _0, PyObject* _1): Tuple(2){
    _args[0] = _0;
    _args[1] = _1;
}

Tuple::Tuple(PyObject* _0, PyObject* _1, PyObject* _2): Tuple(3){
    _args[0] = _0;
    _args[1] = _1;
    _args[2] = _2;
}

Tuple::~Tuple(){ if(!is_inlined()) pool64_dealloc(_args); }

List ArgsView::to_list() const{
    List ret(size());
    for(int i=0; i<size(); i++) ret[i] = _begin[i];
    return ret;
}

Tuple ArgsView::to_tuple() const{
    Tuple ret(size());
    for(int i=0; i<size(); i++) ret[i] = _begin[i];
    return ret;
}

}   // namespace pkpy


namespace pkpy{

template<typename T>
constexpr T default_invalid_value(){
    if constexpr(std::is_pointer_v<T>) return nullptr;
    else if constexpr(std::is_same_v<int, T>) return -1;
    else return Discarded();
}

#define PK_SMALL_NAME_DICT_CAPACITY 8
#define PK_SMALL_NAME_DICT_LOOP(B) for(int i=0; i<PK_SMALL_NAME_DICT_CAPACITY; i++) { B }

template<typename V>
struct SmallNameDict{
    using K = StrName;
    static_assert(is_pod<V>::value);

    bool _is_small;
    uint16_t _size;
    K _keys[PK_SMALL_NAME_DICT_CAPACITY];
    V _values[PK_SMALL_NAME_DICT_CAPACITY];

    SmallNameDict(): _is_small(true), _size(0) {}

    bool try_set(K key, V val){
        PK_SMALL_NAME_DICT_LOOP(
            if(_keys[i] == key){ _values[i] = val; return true; }
        )

        if(_size == PK_SMALL_NAME_DICT_CAPACITY) return false;
        if(_keys[_size].empty()){
            _keys[_size] = key;
            _values[_size] = val;
            _size++;
            return true;
        }

        PK_SMALL_NAME_DICT_LOOP(
            if(_keys[i].empty()){
                _keys[i] = key;
                _values[i] = val;
                _size++;
                return true;
            }
        )
        PK_UNREACHABLE()
    }

    V try_get(K key) const {
        PK_SMALL_NAME_DICT_LOOP(
            if(_keys[i] == key) return _values[i];
        )
        return default_invalid_value<V>();
    }

    V* try_get_2(K key) {
        PK_SMALL_NAME_DICT_LOOP(
            if(_keys[i] == key) return &_values[i];
        )
        return nullptr;
    }

    bool del(K key){
        PK_SMALL_NAME_DICT_LOOP(
            if(_keys[i] == key){ _keys[i] = StrName(); _size--; return true; }
        )
        return false;
    }

    template<typename Func>
    void apply(Func func) const {
        PK_SMALL_NAME_DICT_LOOP(
            if(!_keys[i].empty()) func(_keys[i], _values[i]);
        )
    }

    void clear(){
        PK_SMALL_NAME_DICT_LOOP(
            _keys[i] = StrName();
        )
        _size = 0;
    }

    uint16_t size() const { return _size; }
    uint16_t capacity() const { return PK_SMALL_NAME_DICT_CAPACITY; }
};

template<typename T>
struct LargeNameDict {
    PK_ALWAYS_PASS_BY_POINTER(LargeNameDict)

    using Item = std::pair<StrName, T>;
    static constexpr uint16_t kInitialCapacity = 32;
    static_assert(is_pod<T>::value);

    bool _is_small;
    float _load_factor;
    uint16_t _size;

    uint16_t _capacity;
    uint16_t _critical_size;
    uint16_t _mask;

    Item* _items;

#define HASH_PROBE_1(key, ok, i)            \
ok = false;                                 \
i = key.index & _mask;                      \
while(!_items[i].first.empty()) {           \
    if(_items[i].first == (key)) { ok = true; break; }  \
    i = (i + 1) & _mask;                                \
}

#define HASH_PROBE_0 HASH_PROBE_1

    LargeNameDict(float load_factor=PK_INST_ATTR_LOAD_FACTOR): _is_small(false), _load_factor(load_factor), _size(0) {
        _set_capacity_and_alloc_items(kInitialCapacity);
    }

    ~LargeNameDict(){ free(_items); }

    uint16_t size() const { return _size; }
    uint16_t capacity() const { return _capacity; }

    void _set_capacity_and_alloc_items(uint16_t val){
        _capacity = val;
        _critical_size = val * _load_factor;
        _mask = val - 1;

        _items = (Item*)malloc(_capacity * sizeof(Item));
        memset(_items, 0, _capacity * sizeof(Item));
    }

    void set(StrName key, T val){
        bool ok; uint16_t i;
        HASH_PROBE_1(key, ok, i);
        if(!ok) {
            _size++;
            if(_size > _critical_size){
                _rehash_2x();
                HASH_PROBE_1(key, ok, i);
            }
            _items[i].first = key;
        }
        _items[i].second = val;
    }

    void _rehash_2x(){
        Item* old_items = _items;
        uint16_t old_capacity = _capacity;
        _set_capacity_and_alloc_items(_capacity * 2);
        for(uint16_t i=0; i<old_capacity; i++){
            if(old_items[i].first.empty()) continue;
            bool ok; uint16_t j;
            HASH_PROBE_1(old_items[i].first, ok, j);
            if(ok) PK_FATAL_ERROR();
            _items[j] = old_items[i];
        }
        free(old_items);
    }

    T try_get(StrName key) const{
        bool ok; uint16_t i;
        HASH_PROBE_0(key, ok, i);
        if(!ok) return default_invalid_value<T>();
        return _items[i].second;
    }

    T* try_get_2(StrName key) {
        bool ok; uint16_t i;
        HASH_PROBE_0(key, ok, i);
        if(!ok) return nullptr;
        return &_items[i].second;
    }

    T try_get_likely_found(StrName key) const{
        uint16_t i = key.index & _mask;
        if(_items[i].first == key) return _items[i].second;
        i = (i + 1) & _mask;
        if(_items[i].first == key) return _items[i].second;
        return try_get(key);
    }

    T* try_get_2_likely_found(StrName key) {
        uint16_t i = key.index & _mask;
        if(_items[i].first == key) return &_items[i].second;
        i = (i + 1) & _mask;
        if(_items[i].first == key) return &_items[i].second;
        return try_get_2(key);
    }

    bool del(StrName key){
        bool ok; uint16_t i;
        HASH_PROBE_0(key, ok, i);
        if(!ok) return false;
        _items[i].first = StrName();
        _items[i].second = nullptr;
        _size--;
        // tidy
        uint16_t pre_z = i;
        uint16_t z = (i + 1) & _mask;
        while(!_items[z].first.empty()){
            uint16_t h = _items[z].first.index & _mask;
            if(h != i) break;
            std::swap(_items[pre_z], _items[z]);
            pre_z = z;
            z = (z + 1) & _mask;
        }
        return true;
    }

    template<typename __Func>
    void apply(__Func func) const {
        for(uint16_t i=0; i<_capacity; i++){
            if(_items[i].first.empty()) continue;
            func(_items[i].first, _items[i].second);
        }
    }

    std::vector<StrName> keys() const {
        std::vector<StrName> v;
        for(uint16_t i=0; i<_capacity; i++){
            if(_items[i].first.empty()) continue;
            v.push_back(_items[i].first);
        }
        return v;
    }

    void clear(){
        for(uint16_t i=0; i<_capacity; i++){
            _items[i].first = StrName();
            _items[i].second = nullptr;
        }
        _size = 0;
    }
#undef HASH_PROBE_0
#undef HASH_PROBE_1
};

template<typename V>
struct NameDictImpl{
    PK_ALWAYS_PASS_BY_POINTER(NameDictImpl)

    union{
        SmallNameDict<V> _small;
        LargeNameDict<V> _large;
    };

    NameDictImpl(): _small() {}
    NameDictImpl(float load_factor): _large(load_factor) {}

    bool is_small() const{
        const bool* p = reinterpret_cast<const bool*>(this);
        return *p;
    }

    void set(StrName key, V val){
        if(is_small()){
            bool ok = _small.try_set(key, val);
            if(!ok){
                SmallNameDict<V> copied(_small);
                // move to large name dict
                new (&_large) LargeNameDict<V>();
                copied.apply([&](StrName key, V val){
                    _large.set(key, val);
                });
                _large.set(key, val);
            }
        }else{
            _large.set(key, val);
        }
    }

    uint16_t size() const{ return is_small() ?_small.size() : _large.size(); }
    uint16_t capacity() const{ return is_small() ?_small.capacity() : _large.capacity(); }
    V try_get(StrName key) const { return is_small() ?_small.try_get(key) : _large.try_get(key); }
    V* try_get_2(StrName key) { return is_small() ?_small.try_get_2(key) : _large.try_get_2(key); }
    bool del(StrName key){ return is_small() ?_small.del(key) : _large.del(key); }

    V try_get_likely_found(StrName key) const { return is_small() ?_small.try_get(key) : _large.try_get_likely_found(key); }
    V* try_get_2_likely_found(StrName key) { return is_small() ?_small.try_get_2(key) : _large.try_get_2_likely_found(key); }

    bool contains(StrName key) const { return try_get(key) != default_invalid_value<V>(); }

    V operator[](StrName key) const {
        V val = try_get_likely_found(key);
        if(val == default_invalid_value<V>()){
            throw std::runtime_error(_S("NameDict key not found: ", key.escape()).str());
        }
        return val;
    }

    void clear(){
        if(is_small()) _small.clear();
        else _large.clear();
    }

    template<typename Func>
    void apply(Func func) const {
        if(is_small()) _small.apply(func);
        else _large.apply(func);
    }

    std::vector<StrName> keys() const{
        std::vector<StrName> v;
        apply([&](StrName key, V val){
            v.push_back(key);
        });
        return v;
    }

    std::vector<std::pair<StrName, V>> items() const{
        std::vector<std::pair<StrName, V>> v;
        apply([&](StrName key, V val){
            v.push_back({key, val});
        });
        return v;
    }

    ~NameDictImpl(){
        if(!is_small()) _large.~LargeNameDict<V>();
    }
};

using NameDict = NameDictImpl<PyObject*>;
using NameDict_ = std::shared_ptr<NameDict>;
using NameDictInt = NameDictImpl<int>;

static_assert(sizeof(NameDict) <= 128);

} // namespace pkpy
namespace pkpy{
}   // namespace pkpy


namespace pkpy{

struct NeedMoreLines {
    NeedMoreLines(bool is_compiling_class) : is_compiling_class(is_compiling_class) {}
    bool is_compiling_class;
};

struct HandledException {};
struct UnhandledException {};
struct ToBeRaisedException {};

enum CompileMode {
    EXEC_MODE,
    EVAL_MODE,
    REPL_MODE,
    JSON_MODE,
    CELL_MODE
};

struct SourceData {
    PK_ALWAYS_PASS_BY_POINTER(SourceData)

    Str filename;
    CompileMode mode;

    Str source;
    std::vector<const char*> line_starts;
    
    SourceData(std::string_view source, const Str& filename, CompileMode mode);
    SourceData(const Str& filename, CompileMode mode);
    std::pair<const char*,const char*> _get_line(int lineno) const;
    Str snapshot(int lineno, const char* cursor, std::string_view name) const;
};

struct ExceptionLine{
    std::shared_ptr<SourceData> src;
    int lineno;
    const char* cursor;
    std::string name;

    Str snapshot() const { return src->snapshot(lineno, cursor, name); }

    ExceptionLine(std::shared_ptr<SourceData> src, int lineno, const char* cursor, std::string_view name):
        src(src), lineno(lineno), cursor(cursor), name(name) {}
};

struct Exception {
    StrName type;
    Str msg;
    bool is_re;

    int _ip_on_error;
    void* _code_on_error;

    PyObject* _self;    // weak reference
    
    stack<ExceptionLine> stacktrace;
    Exception(StrName type): type(type), is_re(true), _ip_on_error(-1), _code_on_error(nullptr), _self(nullptr) {}

    PyObject* self() const{
        PK_ASSERT(_self != nullptr);
        return _self;
    }

    template<typename... Args>
    void st_push(Args&&... args){
        if(stacktrace.size() >= 8) return;
        stacktrace.emplace(std::forward<Args>(args)...);
    }

    Str summary() const;
};

}   // namespace pkpy
namespace pkpy{

    SourceData::SourceData(std::string_view source, const Str& filename, CompileMode mode): filename(filename), mode(mode) {
        int index = 0;
        // Skip utf8 BOM if there is any.
        if (strncmp(source.data(), "\xEF\xBB\xBF", 3) == 0) index += 3;
        // Drop all '\r'
        SStream ss(source.size() + 1);
        while(index < source.size()){
            if(source[index] != '\r') ss << source[index];
            index++;
        }
        this->source = ss.str();
        line_starts.push_back(this->source.c_str());
    }

    SourceData::SourceData(const Str& filename, CompileMode mode): filename(filename), mode(mode) {
        line_starts.push_back(this->source.c_str());
    }

    std::pair<const char*,const char*> SourceData::_get_line(int lineno) const {
        if(lineno == -1) return {nullptr, nullptr};
        lineno -= 1;
        if(lineno < 0) lineno = 0;
        const char* _start = line_starts.at(lineno);
        const char* i = _start;
        // max 300 chars
        while(*i != '\n' && *i != '\0' && i-_start < 300) i++;
        return {_start, i};
    }

    Str SourceData::snapshot(int lineno, const char* cursor, std::string_view name) const{
        SStream ss;
        ss << "  " << "File \"" << filename << "\", line " << lineno;
        if(!name.empty()) ss << ", in " << name;
        if(!source.empty()){
            ss << '\n';
            std::pair<const char*,const char*> pair = _get_line(lineno);
            Str line = "<?>";
            int removed_spaces = 0;
            if(pair.first && pair.second){
                line = Str(pair.first, pair.second-pair.first).lstrip();
                removed_spaces = pair.second - pair.first - line.length();
                if(line.empty()) line = "<?>";
            }
            ss << "    " << line;
            if(cursor && line != "<?>" && cursor >= pair.first && cursor <= pair.second){
                auto column = cursor - pair.first - removed_spaces;
                if(column >= 0) ss << "\n    " << std::string(column, ' ') << "^";
            }
        }
        return ss.str();
    }

    Str Exception::summary() const {
        stack<ExceptionLine> st(stacktrace);
        SStream ss;
        if(is_re) ss << "Traceback (most recent call last):\n";
        while(!st.empty()) {
            ss << st.top().snapshot() << '\n';
            st.pop();
        }
        // TODO: allow users to override the behavior
        if (!msg.empty()) ss << type.sv() << ": " << msg;
        else ss << type.sv();
        return ss.str();
    }

}   // namespace pkpy


namespace pkpy {
    
struct Frame;
class VM;

#if PK_ENABLE_STD_FUNCTION
using NativeFuncC = std::function<PyObject*(VM*, ArgsView)>;
#else
typedef PyObject* (*NativeFuncC)(VM*, ArgsView);
#endif

enum class BindType{
    DEFAULT,
    STATICMETHOD,
    CLASSMETHOD,
};

struct BoundMethod {
    PyObject* self;
    PyObject* func;
    BoundMethod(PyObject* self, PyObject* func) : self(self), func(func) {}
};

struct StaticMethod{
    PyObject* func;
    StaticMethod(PyObject* func) : func(func) {}
};

struct ClassMethod{
    PyObject* func;
    ClassMethod(PyObject* func) : func(func) {}
};

struct Property{
    PyObject* getter;
    PyObject* setter;
    Str signature;
    Property(PyObject* getter, PyObject* setter, Str signature) : getter(getter), setter(setter), signature(signature) {}
};

struct Range {
    i64 start = 0;
    i64 stop = -1;
    i64 step = 1;
};

struct StarWrapper{
    int level;      // either 1 or 2
    PyObject* obj;
    StarWrapper(int level, PyObject* obj) : level(level), obj(obj) {}
};

struct Bytes{
    unsigned char* _data;
    int _size;

    int size() const noexcept { return _size; }
    int operator[](int i) const noexcept { return (int)_data[i]; }
    const unsigned char* data() const noexcept { return _data; }

    bool operator==(const Bytes& rhs) const;
    bool operator!=(const Bytes& rhs) const;

    Str str() const noexcept { return Str((char*)_data, _size); }
    std::string_view sv() const noexcept { return std::string_view((char*)_data, _size); }

    Bytes() : _data(nullptr), _size(0) {}
    Bytes(unsigned char* p, int size): _data(p), _size(size) {}
    Bytes(const Str& str): Bytes(str.sv()) {}
    operator bool() const noexcept { return _data != nullptr; }

    Bytes(const std::vector<unsigned char>& v);
    Bytes(std::string_view sv);
    Bytes(const Bytes& rhs);
    Bytes(Bytes&& rhs) noexcept;
    Bytes& operator=(Bytes&& rhs) noexcept;
    Bytes& operator=(const Bytes& rhs) = delete;
    std::pair<unsigned char*, int> detach() noexcept;

    ~Bytes(){ delete[] _data;}
};

using Super = std::pair<PyObject*, Type>;

struct Slice {
    PyObject* start;
    PyObject* stop;
    PyObject* step;
    Slice(PyObject* start, PyObject* stop, PyObject* step) : start(start), stop(stop), step(step) {}
};

struct GCHeader {
    bool enabled;   // whether this object is managed by GC
    bool marked;    // whether this object is marked
    GCHeader() : enabled(true), marked(false) {}
};

struct PyObject{
    GCHeader gc;
    Type type;
    NameDict* _attr;

    bool is_attr_valid() const noexcept { return _attr != nullptr; }
    NameDict& attr() noexcept { return *_attr; }
    PyObject* attr(StrName name) const noexcept { return (*_attr)[name]; }

    // PyObject* operator[](StrName name) const noexcept { return (*_attr)[name]; }

    virtual void _obj_gc_mark() = 0;
    virtual void* _value_ptr() = 0;

    PyObject(Type type) : type(type), _attr(nullptr) {}

    virtual ~PyObject();

    void _enable_instance_dict() {
        _attr = new(pool128_alloc<NameDict>()) NameDict();
    }

    void _enable_instance_dict(float lf){
        _attr = new(pool128_alloc<NameDict>()) NameDict(lf);
    }
};

struct PySignalObject: PyObject {
    PySignalObject() : PyObject(0) {
        gc.enabled = false;
    }
    void _obj_gc_mark() override {}
    void* _value_ptr() override { return nullptr; }
};

inline PyObject* const PY_NULL = new PySignalObject();
inline PyObject* const PY_OP_CALL = new PySignalObject();
inline PyObject* const PY_OP_YIELD = new PySignalObject();

const int kTpIntIndex = 2;
const int kTpFloatIndex = 3;

inline bool is_tagged(PyObject* p) noexcept { return (PK_BITS(p) & 0b11) != 0b00; }
inline bool is_small_int(PyObject* p) noexcept { return (PK_BITS(p) & 0b11) == 0b10; }
inline bool is_heap_int(PyObject* p) noexcept { return !is_tagged(p) && p->type.index == kTpIntIndex; }
inline bool is_float(PyObject* p) noexcept { return (PK_BITS(p) & 1) == 1; }    // 01 or 11
inline bool is_int(PyObject* p) noexcept { return is_small_int(p) || is_heap_int(p); }

inline bool is_type(PyObject* obj, Type type) {
#if PK_DEBUG_EXTRA_CHECK
    if(obj == nullptr) throw std::runtime_error("is_type() called with nullptr");
#endif
    switch(type.index){
        case kTpIntIndex: return is_int(obj);
        case kTpFloatIndex: return is_float(obj);
        default: return !is_tagged(obj) && obj->type == type;
    }
}

inline bool is_non_tagged_type(PyObject* obj, Type type) {
#if PK_DEBUG_EXTRA_CHECK
    if(obj == nullptr) throw std::runtime_error("is_non_tagged_type() called with nullptr");
#endif
    return !is_tagged(obj) && obj->type == type;
}

template <typename, typename=void> struct has_gc_marker : std::false_type {};
template <typename T> struct has_gc_marker<T, std::void_t<decltype(&T::_gc_mark)>> : std::true_type {};

template <typename T>
struct Py_ final: PyObject {
    T _value;
    void _obj_gc_mark() override {
        if constexpr (has_gc_marker<T>::value) {
            _value._gc_mark();
        }
    }

    void* _value_ptr() override { return &_value; }
    
    template <typename... Args>
    Py_(Type type, Args&&... args) : PyObject(type), _value(std::forward<Args>(args)...) { }
};

struct MappingProxy{
    PyObject* obj;
    MappingProxy(PyObject* obj) : obj(obj) {}
    NameDict& attr() noexcept { return obj->attr(); }
};

#define PK_OBJ_GET(T, obj) (((Py_<T>*)(obj))->_value)

#define PK_OBJ_MARK(obj) \
    if(!is_tagged(obj) && !(obj)->gc.marked) {                      \
        (obj)->gc.marked = true;                                    \
        (obj)->_obj_gc_mark();                                      \
        if((obj)->is_attr_valid()) gc_mark_namedict((obj)->attr()); \
    }

inline void gc_mark_namedict(NameDict& t){
    if(t.size() == 0) return;
    t.apply([](StrName name, PyObject* obj){
        PK_OBJ_MARK(obj);
    });
}

StrName _type_name(VM* vm, Type type);

template <typename, typename=void> struct is_py_class : std::false_type {};
template <typename T> struct is_py_class<T, std::void_t<decltype(T::_type)>> : std::true_type {};

template<typename T> T to_void_p(VM*, PyObject*);

template<typename __T>
__T py_cast(VM* vm, PyObject* obj) {
    using T = std::decay_t<__T>;
    if constexpr(std::is_enum_v<T>){
        return (__T)py_cast<i64>(vm, obj);
    }else if constexpr(std::is_pointer_v<T>){
        return to_void_p<T>(vm, obj);
    }else if constexpr(is_py_class<T>::value){
        T::_check_type(vm, obj);
        return PK_OBJ_GET(T, obj);
    }else {
        return Discarded();
    }
}

template<typename __T>
__T _py_cast(VM* vm, PyObject* obj) {
    using T = std::decay_t<__T>;
    if constexpr(std::is_enum_v<T>){
        return (__T)_py_cast<i64>(vm, obj);
    }else if constexpr(std::is_pointer_v<__T>){
        return to_void_p<__T>(vm, obj);
    }else if constexpr(is_py_class<T>::value){
        return PK_OBJ_GET(T, obj);
    }else {
        return Discarded();
    }
}

#define VAR(x) py_var(vm, x)
#define CAST(T, x) py_cast<T>(vm, x)
#define _CAST(T, x) _py_cast<T>(vm, x)

#define CAST_F(x) py_cast<f64>(vm, x)
#define CAST_DEFAULT(T, x, default_value) (x != vm->None) ? py_cast<T>(vm, x) : (default_value)

/*****************************************************************/
template<>
struct Py_<i64> final: PyObject {
    i64 _value;
    Py_(Type type, i64 val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {}
    void* _value_ptr() override { return &_value; }
};

inline bool try_cast_int(PyObject* obj, i64* val) noexcept {
    if(is_small_int(obj)){
        *val = PK_BITS(obj) >> 2;
        return true;
    }else if(is_heap_int(obj)){
        *val = PK_OBJ_GET(i64, obj);
        return true;
    }else{
        return false;
    }
}

template<>
struct Py_<List> final: PyObject {
    List _value;
    Py_(Type type, List&& val): PyObject(type), _value(std::move(val)) {}
    Py_(Type type, const List& val): PyObject(type), _value(val) {}

    void _obj_gc_mark() override {
        for(PyObject* obj: _value) PK_OBJ_MARK(obj);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<Tuple> final: PyObject {
    Tuple _value;
    Py_(Type type, Tuple&& val): PyObject(type), _value(std::move(val)) {}
    Py_(Type type, const Tuple& val): PyObject(type), _value(val) {}

    void _obj_gc_mark() override {
        for(PyObject* obj: _value) PK_OBJ_MARK(obj);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<MappingProxy> final: PyObject {
    MappingProxy _value;
    Py_(Type type, MappingProxy val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.obj);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<BoundMethod> final: PyObject {
    BoundMethod _value;
    Py_(Type type, BoundMethod val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.self);
        PK_OBJ_MARK(_value.func);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<StarWrapper> final: PyObject {
    StarWrapper _value;
    Py_(Type type, StarWrapper val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.obj);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<StaticMethod> final: PyObject {
    StaticMethod _value;
    Py_(Type type, StaticMethod val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.func);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<ClassMethod> final: PyObject {
    ClassMethod _value;
    Py_(Type type, ClassMethod val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.func);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<Property> final: PyObject {
    Property _value;
    Py_(Type type, Property val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.getter);
        PK_OBJ_MARK(_value.setter);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<Slice> final: PyObject {
    Slice _value;
    Py_(Type type, Slice val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.start);
        PK_OBJ_MARK(_value.stop);
        PK_OBJ_MARK(_value.step);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<Super> final: PyObject {
    Super _value;
    template<typename... Args>
    Py_(Type type, Args&&... args): PyObject(type), _value(std::forward<Args>(args)...) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.first);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<DummyInstance> final: PyObject {
    Py_(Type type): PyObject(type) {
        _enable_instance_dict();
    }
    void _obj_gc_mark() override {}
    void* _value_ptr() override { return nullptr; }
};

template<>
struct Py_<Type> final: PyObject {
    Type _value;
    Py_(Type type, Type val): PyObject(type), _value(val) {
        _enable_instance_dict(PK_TYPE_ATTR_LOAD_FACTOR);
    }
    void _obj_gc_mark() override {}
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<DummyModule> final: PyObject {
    Py_(Type type): PyObject(type) {
        _enable_instance_dict(PK_TYPE_ATTR_LOAD_FACTOR);
    }
    void _obj_gc_mark() override {}
    void* _value_ptr() override { return nullptr; }
};

}   // namespace pkpy
namespace pkpy{
    PyObject::~PyObject() {
        if(_attr == nullptr) return;
        _attr->~NameDict();
        pool128_dealloc(_attr);
    }

    bool Bytes::operator==(const Bytes& rhs) const{
        if(_size != rhs._size) return false;
        for(int i=0; i<_size; i++) if(_data[i] != rhs._data[i]) return false;
        return true;
    }
    bool Bytes::operator!=(const Bytes& rhs) const{ return !(*this == rhs); }

    Bytes::Bytes(const std::vector<unsigned char>& v){
        _data = new unsigned char[v.size()];
        _size = v.size();
        for(int i=0; i<_size; i++) _data[i] = v[i];
    }
    Bytes::Bytes(std::string_view sv){
        _data = new unsigned char[sv.size()];
        _size = sv.size();
        for(int i=0; i<_size; i++) _data[i] = sv[i];
    }

    // copy constructor
    Bytes::Bytes(const Bytes& rhs){
        _data = new unsigned char[rhs._size];
        _size = rhs._size;
        for(int i=0; i<_size; i++) _data[i] = rhs._data[i];
    }

    // move constructor
    Bytes::Bytes(Bytes&& rhs) noexcept {
        _data = rhs._data;
        _size = rhs._size;
        rhs._data = nullptr;
        rhs._size = 0;
    }

    // move assignment
    Bytes& Bytes::operator=(Bytes&& rhs) noexcept {
        delete[] _data;
        _data = rhs._data;
        _size = rhs._size;
        rhs._data = nullptr;
        rhs._size = 0;
        return *this;
    }

    std::pair<unsigned char*, int> Bytes::detach() noexcept {
        unsigned char* p = _data;
        int size = _size;
        _data = nullptr;
        _size = 0;
        return {p, size};
    }

}   // namespace pkpy


namespace pkpy{

struct Dict{
    struct Item{
        PyObject* first;
        PyObject* second;
    };

    struct ItemNode{
        int prev;
        int next;
    };

    static constexpr int __Capacity = 8;
    static constexpr float __LoadFactor = 0.67f;
    static_assert(sizeof(Item) * __Capacity <= 128);
    static_assert(sizeof(ItemNode) * __Capacity <= 64);

    VM* vm;
    int _capacity;
    int _mask;
    int _size;
    int _critical_size;
    int _head_idx;          // for order preserving
    int _tail_idx;          // for order preserving
    Item* _items;
    ItemNode* _nodes;       // for order preserving

    Dict(VM* vm);
    Dict(Dict&& other);
    Dict(const Dict& other);
    Dict& operator=(const Dict&) = delete;
    Dict& operator=(Dict&&) = delete;

    int size() const { return _size; }

    void _probe_0(PyObject* key, bool& ok, int& i) const;
    void _probe_1(PyObject* key, bool& ok, int& i) const;

    void set(PyObject* key, PyObject* val);
    void _rehash();

    PyObject* try_get(PyObject* key) const;

    bool contains(PyObject* key) const;
    bool erase(PyObject* key);
    void update(const Dict& other);

    template<typename __Func>
    void apply(__Func f) const {
        int i = _head_idx;
        while(i != -1){
            f(_items[i].first, _items[i].second);
            i = _nodes[i].next;
        }
    }

    Tuple keys() const;
    Tuple values() const;
    void clear();
    ~Dict();

    void _gc_mark() const;
};

} // namespace pkpy
namespace pkpy{

    Dict::Dict(VM* vm): vm(vm), _capacity(__Capacity),
            _mask(__Capacity-1),
            _size(0), _critical_size(__Capacity*__LoadFactor+0.5f), _head_idx(-1), _tail_idx(-1){
        _items = (Item*)pool128_alloc(_capacity * sizeof(Item));
        memset(_items, 0, _capacity * sizeof(Item));
        _nodes = (ItemNode*)pool64_alloc(_capacity * sizeof(ItemNode));
        memset(_nodes, -1, _capacity * sizeof(ItemNode));
    }

    Dict::Dict(Dict&& other){
        vm = other.vm;
        _capacity = other._capacity;
        _mask = other._mask;
        _size = other._size;
        _critical_size = other._critical_size;
        _head_idx = other._head_idx;
        _tail_idx = other._tail_idx;
        _items = other._items;
        _nodes = other._nodes;
        other._items = nullptr;
        other._nodes = nullptr;
    }

    Dict::Dict(const Dict& other){
        vm = other.vm;
        _capacity = other._capacity;
        _mask = other._mask;
        _size = other._size;
        _critical_size = other._critical_size;
        _head_idx = other._head_idx;
        _tail_idx = other._tail_idx;
        _items = (Item*)pool128_alloc(_capacity * sizeof(Item));
        memcpy(_items, other._items, _capacity * sizeof(Item));
        _nodes = (ItemNode*)pool64_alloc(_capacity * sizeof(ItemNode));
        memcpy(_nodes, other._nodes, _capacity * sizeof(ItemNode));
    }

    void Dict::set(PyObject* key, PyObject* val){
        // do possible rehash
        if(_size+1 > _critical_size) _rehash();
        bool ok; int i;
        _probe_1(key, ok, i);
        if(!ok) {
            _size++;
            _items[i].first = key;

            // append to tail
            if(_size == 0+1){
                _head_idx = i;
                _tail_idx = i;
            }else{
                _nodes[i].prev = _tail_idx;
                _nodes[_tail_idx].next = i;
                _tail_idx = i;
            }
        }
        _items[i].second = val;
    }

    void Dict::_rehash(){
        Item* old_items = _items;
        ItemNode* old_nodes = _nodes;
        int old_head_idx = _head_idx;

        _capacity *= 2;
        _mask = _capacity - 1;
        _size = 0;
        _critical_size = _capacity*__LoadFactor+0.5f;
        _head_idx = -1;
        _tail_idx = -1;
        
        _items = (Item*)pool128_alloc(_capacity * sizeof(Item));
        memset(_items, 0, _capacity * sizeof(Item));
        _nodes = (ItemNode*)pool64_alloc(_capacity * sizeof(ItemNode));
        memset(_nodes, -1, _capacity * sizeof(ItemNode));

        // copy old items to new dict
        int i = old_head_idx;
        while(i != -1){
            set(old_items[i].first, old_items[i].second);
            i = old_nodes[i].next;
        }
        pool128_dealloc(old_items);
        pool64_dealloc(old_nodes);
    }


    PyObject* Dict::try_get(PyObject* key) const{
        bool ok; int i;
        _probe_0(key, ok, i);
        if(!ok) return nullptr;
        return _items[i].second;
    }

    bool Dict::contains(PyObject* key) const{
        bool ok; int i;
        _probe_0(key, ok, i);
        return ok;
    }

    bool Dict::erase(PyObject* key){
        bool ok; int i;
        _probe_0(key, ok, i);
        if(!ok) return false;
        _items[i].first = nullptr;
        // _items[i].second = PY_DELETED_SLOT;  // do not change .second if it is not NULL, it means the slot is occupied by a deleted item
        _size--;

        if(_size == 0){
            _head_idx = -1;
            _tail_idx = -1;
        }else{
            if(_head_idx == i){
                _head_idx = _nodes[i].next;
                _nodes[_head_idx].prev = -1;
            }else if(_tail_idx == i){
                _tail_idx = _nodes[i].prev;
                _nodes[_tail_idx].next = -1;
            }else{
                _nodes[_nodes[i].prev].next = _nodes[i].next;
                _nodes[_nodes[i].next].prev = _nodes[i].prev;
            }
        }
        _nodes[i].prev = -1;
        _nodes[i].next = -1;
        return true;
    }

    void Dict::update(const Dict& other){
        other.apply([&](PyObject* k, PyObject* v){ set(k, v); });
    }

    Tuple Dict::keys() const{
        Tuple t(_size);
        int i = _head_idx;
        int j = 0;
        while(i != -1){
            t[j++] = _items[i].first;
            i = _nodes[i].next;
        }
        PK_ASSERT(j == _size);
        return t;
    }

    Tuple Dict::values() const{
        Tuple t(_size);
        int i = _head_idx;
        int j = 0;
        while(i != -1){
            t[j++] = _items[i].second;
            i = _nodes[i].next;
        }
        PK_ASSERT(j == _size);
        return t;
    }

    void Dict::clear(){
        _size = 0;
        _head_idx = -1;
        _tail_idx = -1;
        memset(_items, 0, _capacity * sizeof(Item));
        memset(_nodes, -1, _capacity * sizeof(ItemNode));
    }

    Dict::~Dict(){
        if(_items==nullptr) return;
        pool128_dealloc(_items);
        pool64_dealloc(_nodes);
    }

    void Dict::_gc_mark() const{
        apply([](PyObject* k, PyObject* v){
            PK_OBJ_MARK(k);
            PK_OBJ_MARK(v);
        });
    }

}   // namespace pkpy


namespace pkpy{

enum NameScope { NAME_LOCAL, NAME_GLOBAL, NAME_GLOBAL_UNKNOWN };

enum Opcode: uint8_t {
    #define OPCODE(name) OP_##name,
    
#ifdef OPCODE

/**************************/
OPCODE(NO_OP)
/**************************/
OPCODE(POP_TOP)
OPCODE(DUP_TOP)
OPCODE(ROT_TWO)
OPCODE(ROT_THREE)
OPCODE(PRINT_EXPR)
/**************************/
OPCODE(LOAD_CONST)
OPCODE(LOAD_NONE)
OPCODE(LOAD_TRUE)
OPCODE(LOAD_FALSE)
OPCODE(LOAD_INTEGER)
OPCODE(LOAD_ELLIPSIS)
OPCODE(LOAD_FUNCTION)
OPCODE(LOAD_NULL)
/**************************/
OPCODE(LOAD_FAST)
OPCODE(LOAD_NAME)
OPCODE(LOAD_NONLOCAL)
OPCODE(LOAD_GLOBAL)
OPCODE(LOAD_ATTR)
OPCODE(LOAD_CLASS_GLOBAL)
OPCODE(LOAD_METHOD)
OPCODE(LOAD_SUBSCR)

OPCODE(STORE_FAST)
OPCODE(STORE_NAME)
OPCODE(STORE_GLOBAL)
OPCODE(STORE_ATTR)
OPCODE(STORE_SUBSCR)

OPCODE(DELETE_FAST)
OPCODE(DELETE_NAME)
OPCODE(DELETE_GLOBAL)
OPCODE(DELETE_ATTR)
OPCODE(DELETE_SUBSCR)
/**************************/
OPCODE(BUILD_LONG)
OPCODE(BUILD_IMAG)
OPCODE(BUILD_BYTES)
OPCODE(BUILD_TUPLE)
OPCODE(BUILD_LIST)
OPCODE(BUILD_DICT)
OPCODE(BUILD_SET)
OPCODE(BUILD_SLICE)
OPCODE(BUILD_STRING)
/**************************/
OPCODE(BUILD_TUPLE_UNPACK)
OPCODE(BUILD_LIST_UNPACK)
OPCODE(BUILD_DICT_UNPACK)
OPCODE(BUILD_SET_UNPACK)
/**************************/
OPCODE(BINARY_TRUEDIV)
OPCODE(BINARY_POW)

OPCODE(BINARY_ADD)
OPCODE(BINARY_SUB)
OPCODE(BINARY_MUL)
OPCODE(BINARY_FLOORDIV)
OPCODE(BINARY_MOD)

OPCODE(COMPARE_LT)
OPCODE(COMPARE_LE)
OPCODE(COMPARE_EQ)
OPCODE(COMPARE_NE)
OPCODE(COMPARE_GT)
OPCODE(COMPARE_GE)

OPCODE(BITWISE_LSHIFT)
OPCODE(BITWISE_RSHIFT)
OPCODE(BITWISE_AND)
OPCODE(BITWISE_OR)
OPCODE(BITWISE_XOR)

OPCODE(BINARY_MATMUL)

OPCODE(IS_OP)
OPCODE(CONTAINS_OP)
/**************************/
OPCODE(JUMP_ABSOLUTE)
OPCODE(JUMP_ABSOLUTE_TOP)
OPCODE(POP_JUMP_IF_FALSE)
OPCODE(POP_JUMP_IF_TRUE)
OPCODE(JUMP_IF_TRUE_OR_POP)
OPCODE(JUMP_IF_FALSE_OR_POP)
OPCODE(SHORTCUT_IF_FALSE_OR_POP)
OPCODE(LOOP_CONTINUE)
OPCODE(LOOP_BREAK)
OPCODE(GOTO)
/**************************/
OPCODE(FSTRING_EVAL)
OPCODE(REPR)
OPCODE(CALL)
OPCODE(CALL_TP)
OPCODE(RETURN_VALUE)
OPCODE(YIELD_VALUE)
/**************************/
OPCODE(LIST_APPEND)
OPCODE(DICT_ADD)
OPCODE(SET_ADD)
/**************************/
OPCODE(UNARY_NEGATIVE)
OPCODE(UNARY_NOT)
OPCODE(UNARY_STAR)
OPCODE(UNARY_INVERT)
/**************************/
OPCODE(GET_ITER)
OPCODE(FOR_ITER)
/**************************/
OPCODE(IMPORT_PATH)
OPCODE(POP_IMPORT_STAR)
/**************************/
OPCODE(UNPACK_SEQUENCE)
OPCODE(UNPACK_EX)
/**************************/
OPCODE(BEGIN_CLASS)
OPCODE(END_CLASS)
OPCODE(STORE_CLASS_ATTR)
OPCODE(BEGIN_CLASS_DECORATION)
OPCODE(END_CLASS_DECORATION)
OPCODE(ADD_CLASS_ANNOTATION)
/**************************/
OPCODE(WITH_ENTER)
OPCODE(WITH_EXIT)
/**************************/
OPCODE(EXCEPTION_MATCH)
OPCODE(RAISE)
OPCODE(RAISE_ASSERT)
OPCODE(RE_RAISE)
OPCODE(POP_EXCEPTION)
/**************************/
OPCODE(FORMAT_STRING)
/**************************/
OPCODE(INC_FAST)
OPCODE(DEC_FAST)
OPCODE(INC_GLOBAL)
OPCODE(DEC_GLOBAL)
#endif
#undef OPCODE
};

inline const char* OP_NAMES[] = {
    #define OPCODE(name) #name,
    
#ifdef OPCODE

/**************************/
OPCODE(NO_OP)
/**************************/
OPCODE(POP_TOP)
OPCODE(DUP_TOP)
OPCODE(ROT_TWO)
OPCODE(ROT_THREE)
OPCODE(PRINT_EXPR)
/**************************/
OPCODE(LOAD_CONST)
OPCODE(LOAD_NONE)
OPCODE(LOAD_TRUE)
OPCODE(LOAD_FALSE)
OPCODE(LOAD_INTEGER)
OPCODE(LOAD_ELLIPSIS)
OPCODE(LOAD_FUNCTION)
OPCODE(LOAD_NULL)
/**************************/
OPCODE(LOAD_FAST)
OPCODE(LOAD_NAME)
OPCODE(LOAD_NONLOCAL)
OPCODE(LOAD_GLOBAL)
OPCODE(LOAD_ATTR)
OPCODE(LOAD_CLASS_GLOBAL)
OPCODE(LOAD_METHOD)
OPCODE(LOAD_SUBSCR)

OPCODE(STORE_FAST)
OPCODE(STORE_NAME)
OPCODE(STORE_GLOBAL)
OPCODE(STORE_ATTR)
OPCODE(STORE_SUBSCR)

OPCODE(DELETE_FAST)
OPCODE(DELETE_NAME)
OPCODE(DELETE_GLOBAL)
OPCODE(DELETE_ATTR)
OPCODE(DELETE_SUBSCR)
/**************************/
OPCODE(BUILD_LONG)
OPCODE(BUILD_IMAG)
OPCODE(BUILD_BYTES)
OPCODE(BUILD_TUPLE)
OPCODE(BUILD_LIST)
OPCODE(BUILD_DICT)
OPCODE(BUILD_SET)
OPCODE(BUILD_SLICE)
OPCODE(BUILD_STRING)
/**************************/
OPCODE(BUILD_TUPLE_UNPACK)
OPCODE(BUILD_LIST_UNPACK)
OPCODE(BUILD_DICT_UNPACK)
OPCODE(BUILD_SET_UNPACK)
/**************************/
OPCODE(BINARY_TRUEDIV)
OPCODE(BINARY_POW)

OPCODE(BINARY_ADD)
OPCODE(BINARY_SUB)
OPCODE(BINARY_MUL)
OPCODE(BINARY_FLOORDIV)
OPCODE(BINARY_MOD)

OPCODE(COMPARE_LT)
OPCODE(COMPARE_LE)
OPCODE(COMPARE_EQ)
OPCODE(COMPARE_NE)
OPCODE(COMPARE_GT)
OPCODE(COMPARE_GE)

OPCODE(BITWISE_LSHIFT)
OPCODE(BITWISE_RSHIFT)
OPCODE(BITWISE_AND)
OPCODE(BITWISE_OR)
OPCODE(BITWISE_XOR)

OPCODE(BINARY_MATMUL)

OPCODE(IS_OP)
OPCODE(CONTAINS_OP)
/**************************/
OPCODE(JUMP_ABSOLUTE)
OPCODE(JUMP_ABSOLUTE_TOP)
OPCODE(POP_JUMP_IF_FALSE)
OPCODE(POP_JUMP_IF_TRUE)
OPCODE(JUMP_IF_TRUE_OR_POP)
OPCODE(JUMP_IF_FALSE_OR_POP)
OPCODE(SHORTCUT_IF_FALSE_OR_POP)
OPCODE(LOOP_CONTINUE)
OPCODE(LOOP_BREAK)
OPCODE(GOTO)
/**************************/
OPCODE(FSTRING_EVAL)
OPCODE(REPR)
OPCODE(CALL)
OPCODE(CALL_TP)
OPCODE(RETURN_VALUE)
OPCODE(YIELD_VALUE)
/**************************/
OPCODE(LIST_APPEND)
OPCODE(DICT_ADD)
OPCODE(SET_ADD)
/**************************/
OPCODE(UNARY_NEGATIVE)
OPCODE(UNARY_NOT)
OPCODE(UNARY_STAR)
OPCODE(UNARY_INVERT)
/**************************/
OPCODE(GET_ITER)
OPCODE(FOR_ITER)
/**************************/
OPCODE(IMPORT_PATH)
OPCODE(POP_IMPORT_STAR)
/**************************/
OPCODE(UNPACK_SEQUENCE)
OPCODE(UNPACK_EX)
/**************************/
OPCODE(BEGIN_CLASS)
OPCODE(END_CLASS)
OPCODE(STORE_CLASS_ATTR)
OPCODE(BEGIN_CLASS_DECORATION)
OPCODE(END_CLASS_DECORATION)
OPCODE(ADD_CLASS_ANNOTATION)
/**************************/
OPCODE(WITH_ENTER)
OPCODE(WITH_EXIT)
/**************************/
OPCODE(EXCEPTION_MATCH)
OPCODE(RAISE)
OPCODE(RAISE_ASSERT)
OPCODE(RE_RAISE)
OPCODE(POP_EXCEPTION)
/**************************/
OPCODE(FORMAT_STRING)
/**************************/
OPCODE(INC_FAST)
OPCODE(DEC_FAST)
OPCODE(INC_GLOBAL)
OPCODE(DEC_GLOBAL)
#endif
#undef OPCODE
};

struct Bytecode{
    uint8_t op;
    uint16_t arg;
};

enum class CodeBlockType {
    NO_BLOCK,
    FOR_LOOP,
    WHILE_LOOP,
    CONTEXT_MANAGER,
    TRY_EXCEPT,
};

inline const uint8_t BC_NOARG = 0;
inline const int BC_KEEPLINE = -1;

struct CodeBlock {
    CodeBlockType type;
    int parent;         // parent index in blocks
    int base_stack_size; // this is used for exception handling
    int start;          // start index of this block in codes, inclusive
    int end;            // end index of this block in codes, exclusive
    int end2;           // ...

    CodeBlock(CodeBlockType type, int parent, int base_stack_size, int start):
        type(type), parent(parent), base_stack_size(base_stack_size), start(start), end(-1), end2(-1) {}

    int get_break_end() const{
        if(end2 != -1) return end2;
        return end;
    }
};

struct CodeObject;
struct FuncDecl;
using CodeObject_ = std::shared_ptr<CodeObject>;
using FuncDecl_ = std::shared_ptr<FuncDecl>;

struct CodeObject {
    std::shared_ptr<SourceData> src;
    Str name;
    bool is_generator = false;

    std::vector<Bytecode> codes;
    std::vector<int> iblocks;    // block index for each bytecode
    std::vector<int> lines;     // line number for each bytecode
    List consts;
    std::vector<StrName> varnames;      // local variables
    NameDictInt varnames_inv;
    std::vector<CodeBlock> blocks = { CodeBlock(CodeBlockType::NO_BLOCK, -1, 0, 0) };
    NameDictInt labels;
    std::vector<FuncDecl_> func_decls;

    int start_line;
    int end_line;

    const CodeBlock& _get_block_codei(int codei) const{
        return blocks[iblocks[codei]];
    }

    CodeObject(std::shared_ptr<SourceData> src, const Str& name);
    void _gc_mark() const;
};

struct FuncDecl {
    struct KwArg {
        int index;              // index in co->varnames
        StrName key;            // name of this argument
        PyObject* value;        // default value
    };
    CodeObject_ code;           // code object of this function
    std::vector<int> args;      // indices in co->varnames
    std::vector<KwArg> kwargs;  // indices in co->varnames
    int starred_arg = -1;       // index in co->varnames, -1 if no *arg
    int starred_kwarg = -1;     // index in co->varnames, -1 if no **kwarg
    bool nested = false;        // whether this function is nested

    Str signature;              // signature of this function
    Str docstring;              // docstring of this function
    bool is_simple;

    NameDictInt kw_to_index;

    void add_kwarg(int index, StrName key, PyObject* value){
        kw_to_index.set(key, index);
        kwargs.push_back({index, key, value});
    }
    
    void _gc_mark() const;
};

struct UserData{
    char data[12];
    bool empty;

    UserData(): empty(true) {}
    template<typename T>
    UserData(T t): empty(false){
        static_assert(std::is_trivially_copyable_v<T>);
        static_assert(sizeof(T) <= sizeof(data));
        memcpy(data, &t, sizeof(T));
    }

    template <typename T>
    T get() const{
        static_assert(std::is_trivially_copyable_v<T>);
        static_assert(sizeof(T) <= sizeof(data));
#if PK_DEBUG_EXTRA_CHECK
        PK_ASSERT(!empty);
#endif
        return reinterpret_cast<const T&>(data);
    }
};

struct NativeFunc {
    NativeFuncC f;

    // old style argc-based call
    int argc;

    // new style decl-based call
    FuncDecl_ decl;

    UserData _userdata;

    void set_userdata(UserData data) {
        if(!_userdata.empty && !data.empty){
            // override is not supported
            throw std::runtime_error("userdata already set");
        }
        _userdata = data;
    }

    NativeFunc(NativeFuncC f, int argc, bool method);
    NativeFunc(NativeFuncC f, FuncDecl_ decl);

    void check_size(VM* vm, ArgsView args) const;
    PyObject* call(VM* vm, ArgsView args) const;
};

struct Function{
    FuncDecl_ decl;
    PyObject* _module;  // weak ref
    PyObject* _class;   // weak ref
    NameDict_ _closure;

    explicit Function(FuncDecl_ decl, PyObject* _module, PyObject* _class, NameDict_ _closure):
        decl(decl), _module(_module), _class(_class), _closure(_closure) {}
};

template<>
struct Py_<Function> final: PyObject {
    Function _value;
    template<typename... Args>
    Py_(Type type, Args&&... args): PyObject(type), _value(std::forward<Args>(args)...) {
        // _enable_instance_dict();
    }
    void _obj_gc_mark() override {
        _value.decl->_gc_mark();
        if(_value._closure != nullptr) gc_mark_namedict(*_value._closure);
    }

    void* _value_ptr() override {
        return &_value;
    }
};

template<>
struct Py_<NativeFunc> final: PyObject {
    NativeFunc _value;
    template<typename... Args>
    Py_(Type type, Args&&... args): PyObject(type), _value(std::forward<Args>(args)...) {
        // _enable_instance_dict();
    }
    void _obj_gc_mark() override {
        if(_value.decl != nullptr){
            _value.decl->_gc_mark();
        }
    }
    void* _value_ptr() override {
        return &_value;
    }
};

template<typename T>
T lambda_get_userdata(PyObject** p){
    if(p[-1] != PY_NULL) return PK_OBJ_GET(NativeFunc, p[-1])._userdata.get<T>();
    else return PK_OBJ_GET(NativeFunc, p[-2])._userdata.get<T>();
}

} // namespace pkpy
namespace pkpy{

    CodeObject::CodeObject(std::shared_ptr<SourceData> src, const Str& name):
        src(src), name(name), start_line(-1), end_line(-1) {}

    void CodeObject::_gc_mark() const {
        for(PyObject* v : consts) PK_OBJ_MARK(v);
        for(auto& decl: func_decls) decl->_gc_mark();
    }

    NativeFunc::NativeFunc(NativeFuncC f, int argc, bool method){
        this->f = f;
        this->argc = argc;
        if(argc != -1) this->argc += (int)method;
    }

    NativeFunc::NativeFunc(NativeFuncC f, FuncDecl_ decl){
        this->f = f;
        this->argc = -1;
        this->decl = decl;
    }
}   // namespace pkpy


namespace pkpy{

// weak reference fast locals
struct FastLocals{
    // this is a weak reference
    const NameDictInt* varnames_inv;
    PyObject** a;

    int size() const{ return varnames_inv->size();}

    PyObject*& operator[](int i){ return a[i]; }
    PyObject* operator[](int i) const { return a[i]; }

    FastLocals(const CodeObject* co, PyObject** a): varnames_inv(&co->varnames_inv), a(a) {}
    FastLocals(const FastLocals& other): varnames_inv(other.varnames_inv), a(other.a) {}

    PyObject** try_get_name(StrName name);
    NameDict_ to_namedict();

    PyObject** begin() const { return a; }
    PyObject** end() const { return a + size(); }
};

template<size_t MAX_SIZE>
struct ValueStackImpl {
    // We allocate extra MAX_SIZE/128 places to keep `_sp` valid when `is_overflow() == true`.
    PyObject* _begin[MAX_SIZE + MAX_SIZE/128];
    PyObject** _sp;
    PyObject** _max_end;

    static constexpr size_t max_size() { return MAX_SIZE; }

    ValueStackImpl(): _sp(_begin), _max_end(_begin + MAX_SIZE) {}

    PyObject*& top(){ return _sp[-1]; }
    PyObject* top() const { return _sp[-1]; }
    PyObject*& second(){ return _sp[-2]; }
    PyObject* second() const { return _sp[-2]; }
    PyObject*& third(){ return _sp[-3]; }
    PyObject* third() const { return _sp[-3]; }
    PyObject*& peek(int n){ return _sp[-n]; }
    PyObject* peek(int n) const { return _sp[-n]; }
    void push(PyObject* v){ *_sp++ = v; }
    void pop(){ --_sp; }
    PyObject* popx(){ return *--_sp; }
    ArgsView view(int n){ return ArgsView(_sp-n, _sp); }
    void shrink(int n){ _sp -= n; }
    int size() const { return _sp - _begin; }
    bool empty() const { return _sp == _begin; }
    PyObject** begin() { return _begin; }
    PyObject** end() { return _sp; }
    void reset(PyObject** sp) {
#if PK_DEBUG_EXTRA_CHECK
        if(sp < _begin || sp > _begin + MAX_SIZE) PK_FATAL_ERROR();
#endif
        _sp = sp;
    }
    void clear() { _sp = _begin; }
    bool is_overflow() const { return _sp >= _max_end; }

    PyObject* operator[](int i) const { return _begin[i]; }
    PyObject*& operator[](int i) { return _begin[i]; }
    
    ValueStackImpl(const ValueStackImpl&) = delete;
    ValueStackImpl(ValueStackImpl&&) = delete;
    ValueStackImpl& operator=(const ValueStackImpl&) = delete;
    ValueStackImpl& operator=(ValueStackImpl&&) = delete;
};

using ValueStack = ValueStackImpl<PK_VM_STACK_SIZE>;

struct Frame {
    int _ip = -1;
    int _next_ip = 0;
    ValueStack* _s;
    // This is for unwinding only, use `actual_sp_base()` for value stack access
    PyObject** _sp_base;

    const CodeObject* co;
    PyObject* _module;
    PyObject* _callable;    // weak ref
    FastLocals _locals;

    NameDict& f_globals() noexcept { return _module->attr(); }
    
    PyObject* f_closure_try_get(StrName name);

    Frame(ValueStack* _s, PyObject** p0, const CodeObject* co, PyObject* _module, PyObject* _callable)
            : _s(_s), _sp_base(p0), co(co), _module(_module), _callable(_callable), _locals(co, p0) { }

    Frame(ValueStack* _s, PyObject** p0, const CodeObject* co, PyObject* _module, PyObject* _callable, FastLocals _locals)
            : _s(_s), _sp_base(p0), co(co), _module(_module), _callable(_callable), _locals(_locals) { }

    Frame(ValueStack* _s, PyObject** p0, const CodeObject_& co, PyObject* _module)
            : _s(_s), _sp_base(p0), co(co.get()), _module(_module), _callable(nullptr), _locals(co.get(), p0) {}

    Bytecode next_bytecode() {
        _ip = _next_ip++;
#if PK_DEBUG_EXTRA_CHECK
        if(_ip >= co->codes.size()) PK_FATAL_ERROR();
#endif
        return co->codes[_ip];
    }

    PyObject** actual_sp_base() const { return _locals.a; }
    int stack_size() const { return _s->_sp - actual_sp_base(); }
    ArgsView stack_view() const { return ArgsView(actual_sp_base(), _s->_sp); }

    void jump_abs(int i){ _next_ip = i; }
    bool jump_to_exception_handler();
    int _exit_block(int i);
    void jump_abs_break(int target);

    void _gc_mark() const {
        PK_OBJ_MARK(_module);
        co->_gc_mark();
        // Frame could be stored in a generator, so mark _callable for safety
        if(_callable != nullptr) PK_OBJ_MARK(_callable);
    }
};

}; // namespace pkpy
namespace pkpy{
    PyObject** FastLocals::try_get_name(StrName name){
        int index = varnames_inv->try_get(name);
        if(index == -1) return nullptr;
        return &a[index];
    }

    NameDict_ FastLocals::to_namedict(){
        NameDict_ dict = std::make_shared<NameDict>();
        varnames_inv->apply([&](StrName name, int index){
            PyObject* value = a[index];
            if(value != PY_NULL) dict->set(name, value);
        });
        return dict;
    }

    PyObject* Frame::f_closure_try_get(StrName name){
        if(_callable == nullptr) return nullptr;
        Function& fn = PK_OBJ_GET(Function, _callable);
        if(fn._closure == nullptr) return nullptr;
        return fn._closure->try_get(name);
    }

    bool Frame::jump_to_exception_handler(){
        // try to find a parent try block
        int block = co->iblocks[_ip];
        while(block >= 0){
            if(co->blocks[block].type == CodeBlockType::TRY_EXCEPT) break;
            block = co->blocks[block].parent;
        }
        if(block < 0) return false;
        PyObject* obj = _s->popx();         // pop exception object
        // get the stack size of the try block
        int _stack_size = co->blocks[block].base_stack_size;
        if(stack_size() < _stack_size) throw std::runtime_error(_S("invalid state: ", stack_size(), '<', _stack_size).str());
        _s->reset(actual_sp_base() + _locals.size() + _stack_size);          // rollback the stack   
        _s->push(obj);                                      // push exception object
        _next_ip = co->blocks[block].end;
        return true;
    }

    int Frame::_exit_block(int i){
        auto type = co->blocks[i].type;
        if(type==CodeBlockType::FOR_LOOP || type==CodeBlockType::CONTEXT_MANAGER) _s->pop();
        return co->blocks[i].parent;
    }

    void Frame::jump_abs_break(int target){
        int i = co->iblocks[_ip];
        _next_ip = target;
        if(_next_ip >= co->codes.size()){
            while(i>=0) i = _exit_block(i);
        }else{
            // BUG (solved)
            // for i in range(4):
            //     _ = 0
            // # if there is no op here, the block check will fail
            // while i: --i
            int next_block = co->iblocks[target];
            while(i>=0 && i!=next_block) i = _exit_block(i);
            if(i!=next_block) throw std::runtime_error("invalid jump");
        }
    }

}   // namespace pkpy


namespace pkpy {

struct LineRecord{
    int line;
    i64 hits;
    clock_t time;

    LineRecord(): line(-1), hits(0), time(0) {}
    bool is_valid() const { return line != -1; }
};

struct LineProfiler{
    clock_t prev_time;
    LineRecord* prev_record;
    int prev_line;

    // filename -> records
    std::map<std::string_view, std::vector<LineRecord>> records;

    std::set<FuncDecl*> functions;

    void begin();
    void _step(Frame* frame);
    void _step_end();
    void end();
    Str stats();
};

} // namespace pkpy

namespace pkpy{

static std::string left_pad(std::string s, int width){
    int n = width - s.size();
    if(n <= 0) return s;
    return std::string(n, ' ') + s;
}

static std::string to_string_1f(f64 x){
    char buf[32];
    snprintf(buf, 32, "%.1f", x);
    return buf;
}

void LineProfiler::begin(){
    prev_time = 0;
    prev_record = nullptr;
    prev_line = -1;
    records.clear();
}

void LineProfiler::_step(Frame *frame){
    std::string_view filename = frame->co->src->filename.sv();
    int line = frame->co->lines[frame->_ip];
    // std::string_view function = frame->co->name.sv();

    if(prev_record == nullptr){
        prev_time = clock();
    }else{
        _step_end();
    }

    std::vector<LineRecord>& file_records = records[filename];
    if(file_records.empty()){
        int total_lines = frame->co->src->line_starts.size();
        file_records.resize(total_lines + 1);
        for(int i=1; i<=total_lines; i++){
            file_records[i].line = i;
        }
    }
    prev_record = &file_records.at(line);
}

void LineProfiler::_step_end(){
    clock_t now = clock();
    clock_t delta = now - prev_time;
    prev_time = now;
    if(prev_record->line != prev_line){
        prev_record->hits++;
        prev_line = prev_record->line;
    }
    prev_record->time += delta;
}

void LineProfiler::end(){
    _step_end();
}

Str LineProfiler::stats(){
    SStream ss;
    for(FuncDecl* decl: functions){
        int start_line = decl->code->start_line;
        int end_line = decl->code->end_line;
        if(start_line == -1 || end_line == -1) continue;
        std::string_view filename = decl->code->src->filename.sv();
        std::vector<LineRecord>& file_records = records[filename];
        if(file_records.empty()) continue;
        clock_t total_time = 0;
        for(int line = start_line; line <= end_line; line++){
            total_time += file_records.at(line).time;
        }
        ss << "Total time: " << (f64)total_time / CLOCKS_PER_SEC << "s\n";
        ss << "File: " << filename << "\n";
        ss << "Function: " << decl->code->name << " at line " << start_line << "\n";
        ss << "Line #      Hits         Time  Per Hit   % Time  Line Contents\n";
        ss << "==============================================================\n";
        for(int line = start_line; line <= end_line; line++){
            const LineRecord& record = file_records.at(line);
            if(!record.is_valid()) continue;
            ss << left_pad(std::to_string(line), 6);
            if(record.hits == 0){
                ss << std::string(10 + 13 + 9 + 9, ' ');
            }else{
                ss << left_pad(std::to_string(record.hits), 10);
                ss << left_pad(std::to_string(record.time), 13);
                ss << left_pad(std::to_string(record.time / record.hits), 9);
                if(total_time == 0){
                    ss << left_pad("0.0", 9);
                }else{
                    ss << left_pad(to_string_1f(record.time * (f64)100 / total_time), 9);
                }
            }
            // line_content
            auto [_0, _1] = decl->code->src->_get_line(line);
            ss << "  " << std::string_view(_0, _1-_0) << "\n";
        }
        ss << "\n";
    }
    return ss.str();
}

}   // namespace pkpy


namespace pkpy {
struct ManagedHeap{
    std::vector<PyObject*> _no_gc;
    std::vector<PyObject*> gen;
    VM* vm;
    void (*_gc_on_delete)(VM*, PyObject*) = nullptr;
    void (*_gc_marker_ex)(VM*) = nullptr;

    ManagedHeap(VM* vm): vm(vm) {}
    
    int gc_threshold = PK_GC_MIN_THRESHOLD;
    int gc_counter = 0;

    /********************/
    int _gc_lock_counter = 0;
    struct ScopeLock{
        ManagedHeap* heap;
        ScopeLock(ManagedHeap* heap): heap(heap){
            heap->_gc_lock_counter++;
        }
        ~ScopeLock(){
            heap->_gc_lock_counter--;
        }
    };

    ScopeLock gc_scope_lock(){
        return ScopeLock(this);
    }
    /********************/

    template<typename T, typename... Args>
    PyObject* gcnew(Type type, Args&&... args){
        using __T = Py_<std::decay_t<T>>;
        // https://github.com/pocketpy/pocketpy/issues/94#issuecomment-1594784476
        PyObject* obj = new(pool64_alloc<__T>()) Py_<std::decay_t<T>>(type, std::forward<Args>(args)...);
        gen.push_back(obj);
        gc_counter++;
        return obj;
    }

    template<typename T, typename... Args>
    PyObject* _new(Type type, Args&&... args){
        using __T = Py_<std::decay_t<T>>;
        // https://github.com/pocketpy/pocketpy/issues/94#issuecomment-1594784476
        PyObject* obj = new(pool64_alloc<__T>()) Py_<std::decay_t<T>>(type, std::forward<Args>(args)...);
        obj->gc.enabled = false;
        _no_gc.push_back(obj);
        return obj;
    }

#if PK_DEBUG_GC_STATS
    inline static std::map<Type, int> deleted;
#endif

    int sweep();
    void _auto_collect();
    bool _should_auto_collect() const { return gc_counter >= gc_threshold; }
    int collect();
    void mark();
    ~ManagedHeap();
};

}   // namespace pkpy

namespace pkpy{

    int ManagedHeap::sweep(){
        std::vector<PyObject*> alive;
        for(PyObject* obj: gen){
            if(obj->gc.marked){
                obj->gc.marked = false;
                alive.push_back(obj);
            }else{
#if PK_DEBUG_GC_STATS
                deleted[obj->type] += 1;
#endif
                if(_gc_on_delete) _gc_on_delete(vm, obj);
                obj->~PyObject();
                pool64_dealloc(obj);
            }
        }

        // clear _no_gc marked flag
        for(PyObject* obj: _no_gc) obj->gc.marked = false;

        int freed = gen.size() - alive.size();

#if PK_DEBUG_GC_STATS
        for(auto& [type, count]: deleted){
            std::cout << "GC: " << _type_name(vm, type).sv() << "=" << count << std::endl;
        }
        std::cout << "GC: " << alive.size() << "/" << gen.size() << " (" << freed << " freed)" << std::endl;
        deleted.clear();
#endif
        gen.clear();
        gen.swap(alive);
        // clean up pools
        pools_shrink_to_fit();
        return freed;
    }

    void ManagedHeap::_auto_collect(){
#if !PK_DEBUG_NO_AUTO_GC
        if(_gc_lock_counter > 0) return;
        gc_counter = 0;
        collect();
        gc_threshold = gen.size() * 2;
        if(gc_threshold < PK_GC_MIN_THRESHOLD) gc_threshold = PK_GC_MIN_THRESHOLD;
#endif
    }

    int ManagedHeap::collect(){
        PK_ASSERT(_gc_lock_counter == 0)
        mark();
        int freed = sweep();
        return freed;
    }

    ManagedHeap::~ManagedHeap(){
        for(PyObject* obj: _no_gc) { obj->~PyObject(); pool64_dealloc(obj); }
        for(PyObject* obj: gen) { obj->~PyObject(); pool64_dealloc(obj); }
    }


void FuncDecl::_gc_mark() const{
    code->_gc_mark();
    for(int i=0; i<kwargs.size(); i++) PK_OBJ_MARK(kwargs[i].value);
}

}   // namespace pkpy


namespace pkpy{

/* Stack manipulation macros */
// https://github.com/python/cpython/blob/3.9/Python/ceval.c#L1123
#define TOP()             (s_data.top())
#define SECOND()          (s_data.second())
#define THIRD()           (s_data.third())
#define PEEK(n)           (s_data.peek(n))
#define STACK_SHRINK(n)   (s_data.shrink(n))
#define PUSH(v)           (s_data.push(v))
#define POP()             (s_data.pop())
#define POPX()            (s_data.popx())
#define STACK_VIEW(n)     (s_data.view(n))

#define DEF_NATIVE_2(ctype, ptype)                                      \
    template<> inline ctype py_cast<ctype>(VM* vm, PyObject* obj) {     \
        vm->check_non_tagged_type(obj, vm->ptype);                      \
        return PK_OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> inline ctype _py_cast<ctype>(VM* vm, PyObject* obj) {    \
        PK_UNUSED(vm);                                                  \
        return PK_OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> inline ctype& py_cast<ctype&>(VM* vm, PyObject* obj) {   \
        vm->check_non_tagged_type(obj, vm->ptype);                      \
        return PK_OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> inline ctype& _py_cast<ctype&>(VM* vm, PyObject* obj) {  \
        PK_UNUSED(vm);                                                  \
        return PK_OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    inline PyObject* py_var(VM* vm, const ctype& value) { return vm->heap.gcnew<ctype>(vm->ptype, value);}     \
    inline PyObject* py_var(VM* vm, ctype&& value) { return vm->heap.gcnew<ctype>(vm->ptype, std::move(value));}


typedef PyObject* (*BinaryFuncC)(VM*, PyObject*, PyObject*);

struct PyTypeInfo{
    PyObject* obj;      // never be garbage collected
    Type base;
    PyObject* mod;      // never be garbage collected
    StrName name;
    bool subclass_enabled;

    std::vector<StrName> annotated_fields;

    // cached special methods
    // unary operators
    PyObject* (*m__repr__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__str__)(VM* vm, PyObject*) = nullptr;
    i64 (*m__hash__)(VM* vm, PyObject*) = nullptr;
    i64 (*m__len__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__iter__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__next__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__neg__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__bool__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__invert__)(VM* vm, PyObject*) = nullptr;

    BinaryFuncC m__eq__ = nullptr;
    BinaryFuncC m__lt__ = nullptr;
    BinaryFuncC m__le__ = nullptr;
    BinaryFuncC m__gt__ = nullptr;
    BinaryFuncC m__ge__ = nullptr;
    BinaryFuncC m__contains__ = nullptr;

    // binary operators
    BinaryFuncC m__add__ = nullptr;
    BinaryFuncC m__sub__ = nullptr;
    BinaryFuncC m__mul__ = nullptr;
    BinaryFuncC m__truediv__ = nullptr;
    BinaryFuncC m__floordiv__ = nullptr;
    BinaryFuncC m__mod__ = nullptr;
    BinaryFuncC m__pow__ = nullptr;
    BinaryFuncC m__matmul__ = nullptr;

    BinaryFuncC m__lshift__ = nullptr;
    BinaryFuncC m__rshift__ = nullptr;
    BinaryFuncC m__and__ = nullptr;
    BinaryFuncC m__or__ = nullptr;
    BinaryFuncC m__xor__ = nullptr;

    // indexer
    PyObject* (*m__getitem__)(VM* vm, PyObject*, PyObject*) = nullptr;
    void (*m__setitem__)(VM* vm, PyObject*, PyObject*, PyObject*) = nullptr;
    void (*m__delitem__)(VM* vm, PyObject*, PyObject*) = nullptr;

    // attributes
    void (*m__setattr__)(VM* vm, PyObject*, StrName, PyObject*) = nullptr;
    PyObject* (*m__getattr__)(VM* vm, PyObject*, StrName) = nullptr;
    bool (*m__delattr__)(VM* vm, PyObject*, StrName) = nullptr;

};

struct FrameId{
    std::vector<pkpy::Frame>* data;
    int index;
    FrameId(std::vector<pkpy::Frame>* data, int index) : data(data), index(index) {}
    Frame* operator->() const { return &data->operator[](index); }
    Frame* get() const { return &data->operator[](index); }
};

typedef void(*PrintFunc)(const char*, int);

class VM {
    PK_ALWAYS_PASS_BY_POINTER(VM)
    
    VM* vm;     // self reference for simplify code
public:
    ManagedHeap heap;
    ValueStack s_data;
    stack< Frame > callstack;
    std::vector<PyTypeInfo> _all_types;
    
    NameDict _modules;                                 // loaded modules
    std::map<StrName, Str> _lazy_modules;              // lazy loaded modules

    struct{
        PyObject* error;
        stack<ArgsView> s_view;
    } _c;

    PyObject* None;
    PyObject* True;
    PyObject* False;
    PyObject* NotImplemented;   // unused
    PyObject* Ellipsis;
    PyObject* builtins;         // builtins module
    PyObject* StopIteration;
    PyObject* _main;            // __main__ module

    PyObject* _last_exception;  // last exception
    PyObject* _curr_class;      // current class being defined

    // this is for repr() recursion detection (no need to mark)
    std::set<PyObject*> _repr_recursion_set;

    // cached code objects for FSTRING_EVAL
    std::map<std::string_view, CodeObject_> _cached_codes;

    void (*_ceval_on_step)(VM*, Frame*, Bytecode bc) = nullptr;

    LineProfiler* _profiler = nullptr;

    PrintFunc _stdout;
    PrintFunc _stderr;
    unsigned char* (*_import_handler)(const char*, int, int*);

    // for quick access
    static constexpr Type tp_object=0, tp_type=1;
    static constexpr Type tp_int=kTpIntIndex, tp_float=kTpFloatIndex, tp_bool=4, tp_str=5;
    static constexpr Type tp_list=6, tp_tuple=7;
    static constexpr Type tp_slice=8, tp_range=9, tp_module=10;
    static constexpr Type tp_function=11, tp_native_func=12, tp_bound_method=13;
    
    static constexpr Type tp_super=14, tp_exception=15, tp_bytes=16, tp_mappingproxy=17;
    static constexpr Type tp_dict=18, tp_property=19, tp_star_wrapper=20;
    static constexpr Type tp_staticmethod=21, tp_classmethod=22;

    PyObject* cached_object__new__;

    const bool enable_os;

    VM(bool enable_os=true);

    FrameId top_frame();
    void _pop_frame();

    PyObject* py_str(PyObject* obj);
    PyObject* py_repr(PyObject* obj);
    PyObject* py_json(PyObject* obj);
    PyObject* py_iter(PyObject* obj);

    PyObject* find_name_in_mro(Type cls, StrName name);
    bool isinstance(PyObject* obj, Type base);
    bool issubclass(Type cls, Type base);

    CodeObject_ compile(std::string_view source, const Str& filename, CompileMode mode, bool unknown_global_scope=false);
    PyObject* exec(std::string_view source, Str filename, CompileMode mode, PyObject* _module=nullptr);
    PyObject* exec(std::string_view source);
    PyObject* eval(std::string_view source);

    template<typename ...Args>
    PyObject* _exec(Args&&... args){
        callstack.emplace(&s_data, s_data._sp, std::forward<Args>(args)...);
        return _run_top_frame();
    }

    void _push_varargs(){ }
    void _push_varargs(PyObject* _0){ PUSH(_0); }
    void _push_varargs(PyObject* _0, PyObject* _1){ PUSH(_0); PUSH(_1); }
    void _push_varargs(PyObject* _0, PyObject* _1, PyObject* _2){ PUSH(_0); PUSH(_1); PUSH(_2); }
    void _push_varargs(PyObject* _0, PyObject* _1, PyObject* _2, PyObject* _3){ PUSH(_0); PUSH(_1); PUSH(_2); PUSH(_3); }

    virtual void stdout_write(const Str& s){
        _stdout(s.data, s.size);
    }

    virtual void stderr_write(const Str& s){
        _stderr(s.data, s.size);
    }

    template<typename... Args>
    PyObject* call(PyObject* callable, Args&&... args){
        PUSH(callable);
        PUSH(PY_NULL);
        _push_varargs(args...);
        return vectorcall(sizeof...(args));
    }

    template<typename... Args>
    PyObject* call_method(PyObject* self, PyObject* callable, Args&&... args){
        PUSH(callable);
        PUSH(self);
        _push_varargs(args...);
        return vectorcall(sizeof...(args));
    }

    template<typename... Args>
    PyObject* call_method(PyObject* self, StrName name, Args&&... args){
        PyObject* callable = get_unbound_method(self, name, &self);
        return call_method(self, callable, args...);
    }

    PyObject* new_type_object(PyObject* mod, StrName name, Type base, bool subclass_enabled=true);
    Type _new_type_object(StrName name, Type base=0, bool subclass_enabled=false);
    const PyTypeInfo* _inst_type_info(PyObject* obj);

#define BIND_UNARY_SPECIAL(name)                                                        \
    void bind##name(Type type, PyObject* (*f)(VM*, PyObject*)){                         \
        _all_types[type].m##name = f;                                                   \
        PyObject* nf = bind_method<0>(_t(type), #name, [](VM* vm, ArgsView args){       \
            return lambda_get_userdata<PyObject*(*)(VM*, PyObject*)>(args.begin())(vm, args[0]);\
        });                                                                             \
        PK_OBJ_GET(NativeFunc, nf).set_userdata(f);                                        \
    }

    BIND_UNARY_SPECIAL(__repr__)
    BIND_UNARY_SPECIAL(__str__)
    BIND_UNARY_SPECIAL(__iter__)
    BIND_UNARY_SPECIAL(__next__)
    BIND_UNARY_SPECIAL(__neg__)
    BIND_UNARY_SPECIAL(__bool__)
    BIND_UNARY_SPECIAL(__invert__)

    void bind__hash__(Type type, i64 (*f)(VM* vm, PyObject*));
    void bind__len__(Type type, i64 (*f)(VM* vm, PyObject*));
#undef BIND_UNARY_SPECIAL


#define BIND_BINARY_SPECIAL(name)                                                       \
    void bind##name(Type type, BinaryFuncC f){                                          \
        _all_types[type].m##name = f;                                                   \
        PyObject* nf = bind_method<1>(type, #name, [](VM* vm, ArgsView args){           \
            return lambda_get_userdata<BinaryFuncC>(args.begin())(vm, args[0], args[1]);\
        });                                                                             \
        PK_OBJ_GET(NativeFunc, nf).set_userdata(f);                                     \
    }

    BIND_BINARY_SPECIAL(__eq__)
    BIND_BINARY_SPECIAL(__lt__)
    BIND_BINARY_SPECIAL(__le__)
    BIND_BINARY_SPECIAL(__gt__)
    BIND_BINARY_SPECIAL(__ge__)
    BIND_BINARY_SPECIAL(__contains__)

    BIND_BINARY_SPECIAL(__add__)
    BIND_BINARY_SPECIAL(__sub__)
    BIND_BINARY_SPECIAL(__mul__)
    BIND_BINARY_SPECIAL(__truediv__)
    BIND_BINARY_SPECIAL(__floordiv__)
    BIND_BINARY_SPECIAL(__mod__)
    BIND_BINARY_SPECIAL(__pow__)
    BIND_BINARY_SPECIAL(__matmul__)

    BIND_BINARY_SPECIAL(__lshift__)
    BIND_BINARY_SPECIAL(__rshift__)
    BIND_BINARY_SPECIAL(__and__)
    BIND_BINARY_SPECIAL(__or__)
    BIND_BINARY_SPECIAL(__xor__)

#undef BIND_BINARY_SPECIAL

    void bind__getitem__(Type type, PyObject* (*f)(VM*, PyObject*, PyObject*));
    void bind__setitem__(Type type, void (*f)(VM*, PyObject*, PyObject*, PyObject*));
    void bind__delitem__(Type type, void (*f)(VM*, PyObject*, PyObject*));

    bool py_eq(PyObject* lhs, PyObject* rhs);
    // new in v1.2.9
    bool py_lt(PyObject* lhs, PyObject* rhs);
    bool py_le(PyObject* lhs, PyObject* rhs);
    bool py_gt(PyObject* lhs, PyObject* rhs);
    bool py_ge(PyObject* lhs, PyObject* rhs);
    bool py_ne(PyObject* lhs, PyObject* rhs) { return !py_eq(lhs, rhs); }

    template<int ARGC, typename __T>
    PyObject* bind_constructor(__T&& type, NativeFuncC fn) {
        static_assert(ARGC==-1 || ARGC>=1);
        return bind_func<ARGC>(std::forward<__T>(type), "__new__", fn);
    }

    template<typename T, typename __T>
    PyObject* bind_default_constructor(__T&& type) {
        return bind_constructor<1>(std::forward<__T>(type), [](VM* vm, ArgsView args){
            return vm->heap.gcnew<T>(PK_OBJ_GET(Type, args[0]), T());
        });
    }

    template<typename T, typename __T>
    PyObject* bind_notimplemented_constructor(__T&& type) {
        return bind_constructor<-1>(std::forward<__T>(type), [](VM* vm, ArgsView args){
            PK_UNUSED(args);
            vm->NotImplementedError();
            return vm->None;
        });
    }

    int normalized_index(int index, int size);
    PyObject* py_next(PyObject* obj);
    bool py_callable(PyObject* obj);
    
    /***** Error Reporter *****/
    void _raise(bool re_raise=false);

    void _builtin_error(StrName type);
    void _builtin_error(StrName type, PyObject* arg);
    void _builtin_error(StrName type, const Str& msg);

    void StackOverflowError() { _builtin_error("StackOverflowError"); }
    void IOError(const Str& msg) { _builtin_error("IOError", msg); }
    void NotImplementedError(){ _builtin_error("NotImplementedError"); }
    void TypeError(const Str& msg){ _builtin_error("TypeError", msg); }
    void IndexError(const Str& msg){ _builtin_error("IndexError", msg); }
    void ValueError(const Str& msg){ _builtin_error("ValueError", msg); }
    void RuntimeError(const Str& msg){ _builtin_error("RuntimeError", msg); }
    void ZeroDivisionError(const Str& msg){ _builtin_error("ZeroDivisionError", msg); }
    void ZeroDivisionError(){ _builtin_error("ZeroDivisionError", "division by zero"); }
    void NameError(StrName name){ _builtin_error("NameError", _S("name ", name.escape() + " is not defined")); }
    void UnboundLocalError(StrName name){ _builtin_error("UnboundLocalError", _S("local variable ", name.escape() + " referenced before assignment")); }
    void KeyError(PyObject* obj){ _builtin_error("KeyError", obj); }
    void ImportError(const Str& msg){ _builtin_error("ImportError", msg); }

    void BinaryOptError(const char* op, PyObject* _0, PyObject* _1) {
        StrName name_0 = _type_name(vm, _tp(_0));
        StrName name_1 = _type_name(vm, _tp(_1));
        TypeError(_S("unsupported operand type(s) for ", op, ": ", name_0.escape(), " and ", name_1.escape()));
    }

    void AttributeError(PyObject* obj, StrName name){
        if(isinstance(obj, vm->tp_type)){
            _builtin_error("AttributeError", _S("type object ", _type_name(vm, PK_OBJ_GET(Type, obj)).escape(), " has no attribute ", name.escape()));
        }else{
            _builtin_error("AttributeError", _S(_type_name(vm, _tp(obj)).escape(), " object has no attribute ", name.escape()));
        }
    }
    void AttributeError(const Str& msg){ _builtin_error("AttributeError", msg); }

    void check_type(PyObject* obj, Type type){
        if(is_type(obj, type)) return;
        TypeError("expected " + _type_name(vm, type).escape() + ", got " + _type_name(vm, _tp(obj)).escape());
    }

    void check_non_tagged_type(PyObject* obj, Type type){
        if(is_non_tagged_type(obj, type)) return;
        TypeError("expected " + _type_name(vm, type).escape() + ", got " + _type_name(vm, _tp(obj)).escape());
    }

    PyObject* _t(Type t){
        return _all_types[t.index].obj;
    }

    Type _tp(PyObject* obj){
        if(is_int(obj)) return tp_int;
        if(is_float(obj)) return tp_float;
        return obj->type;
    }

    PyObject* _t(PyObject* obj){
        return _all_types[_tp(obj).index].obj;
    }

    struct ImportContext{
        std::vector<Str> pending;
        std::vector<bool> pending_is_init;   // a.k.a __init__.py
        struct Temp{
            ImportContext* ctx;
            Temp(ImportContext* ctx, Str name, bool is_init) : ctx(ctx){
                ctx->pending.push_back(name);
                ctx->pending_is_init.push_back(is_init);
            }
            ~Temp(){
                ctx->pending.pop_back();
                ctx->pending_is_init.pop_back();
            }
        };

        Temp scope(Str name, bool is_init){
            return {this, name, is_init};
        }
    };

    ImportContext _import_context;
    PyObject* py_import(Str path, bool throw_err=true);
    ~VM();

#if PK_DEBUG_CEVAL_STEP
    void _log_s_data(const char* title = nullptr);
#endif
    void _unpack_as_list(ArgsView args, List& list);
    void _unpack_as_dict(ArgsView args, Dict& dict);
    PyObject* vectorcall(int ARGC, int KWARGC=0, bool op_call=false);
    PyObject* py_negate(PyObject* obj);
    bool py_bool(PyObject* obj);
    i64 py_hash(PyObject* obj);
    PyObject* py_list(PyObject*);
    PyObject* new_module(Str name, Str package="");
    Str disassemble(CodeObject_ co);
    void init_builtin_types();
    PyObject* getattr(PyObject* obj, StrName name, bool throw_err=true);
    void delattr(PyObject* obj, StrName name);
    PyObject* get_unbound_method(PyObject* obj, StrName name, PyObject** self, bool throw_err=true, bool fallback=false);
    void parse_int_slice(const Slice& s, int length, int& start, int& stop, int& step);
    PyObject* _format_string(Str, PyObject*);
    void setattr(PyObject* obj, StrName name, PyObject* value);
    template<int ARGC>
    PyObject* bind_method(Type, Str, NativeFuncC);
    template<int ARGC>
    PyObject* bind_method(PyObject*, Str, NativeFuncC);
    template<int ARGC>
    PyObject* bind_func(PyObject*, Str, NativeFuncC, UserData userdata={}, BindType bt=BindType::DEFAULT);
    void _error(PyObject*);
    PyObject* _run_top_frame();
    void post_init();
    PyObject* _py_generator(Frame&& frame, ArgsView buffer);
    void _prepare_py_call(PyObject**, ArgsView, ArgsView, const FuncDecl_&);
    // new style binding api
    PyObject* bind(PyObject*, const char*, const char*, NativeFuncC, UserData userdata={}, BindType bt=BindType::DEFAULT);
    PyObject* bind(PyObject*, const char*, NativeFuncC, UserData userdata={}, BindType bt=BindType::DEFAULT);
    PyObject* bind_property(PyObject*, Str, NativeFuncC fget, NativeFuncC fset=nullptr);
};

DEF_NATIVE_2(Str, tp_str)
DEF_NATIVE_2(List, tp_list)
DEF_NATIVE_2(Tuple, tp_tuple)
DEF_NATIVE_2(Function, tp_function)
DEF_NATIVE_2(NativeFunc, tp_native_func)
DEF_NATIVE_2(BoundMethod, tp_bound_method)
DEF_NATIVE_2(Range, tp_range)
DEF_NATIVE_2(Slice, tp_slice)
DEF_NATIVE_2(Exception, tp_exception)
DEF_NATIVE_2(Bytes, tp_bytes)
DEF_NATIVE_2(MappingProxy, tp_mappingproxy)
DEF_NATIVE_2(Dict, tp_dict)
DEF_NATIVE_2(Property, tp_property)
DEF_NATIVE_2(StarWrapper, tp_star_wrapper)
DEF_NATIVE_2(StaticMethod, tp_staticmethod)
DEF_NATIVE_2(ClassMethod, tp_classmethod)

#undef DEF_NATIVE_2

#define PY_CAST_INT(T)                                  \
template<> inline T py_cast<T>(VM* vm, PyObject* obj){  \
    if(is_small_int(obj)) return (T)(PK_BITS(obj) >> 2);    \
    if(is_heap_int(obj)) return (T)PK_OBJ_GET(i64, obj);    \
    vm->check_type(obj, vm->tp_int);                        \
    return 0;                                               \
}                                                           \
template<> inline T _py_cast<T>(VM* vm, PyObject* obj){     \
    PK_UNUSED(vm);                                          \
    if(is_small_int(obj)) return (T)(PK_BITS(obj) >> 2);    \
    return (T)PK_OBJ_GET(i64, obj);                         \
}

PY_CAST_INT(char)
PY_CAST_INT(short)
PY_CAST_INT(int)
PY_CAST_INT(long)
PY_CAST_INT(long long)
PY_CAST_INT(unsigned char)
PY_CAST_INT(unsigned short)
PY_CAST_INT(unsigned int)
PY_CAST_INT(unsigned long)
PY_CAST_INT(unsigned long long)

template<> inline float py_cast<float>(VM* vm, PyObject* obj){
    if(is_float(obj)) return untag_float(obj);
    i64 bits;
    if(try_cast_int(obj, &bits)) return (float)bits;
    vm->TypeError("expected 'int' or 'float', got " + _type_name(vm, vm->_tp(obj)).escape());
    return 0;
}
template<> inline float _py_cast<float>(VM* vm, PyObject* obj){
    return py_cast<float>(vm, obj);
}
template<> inline double py_cast<double>(VM* vm, PyObject* obj){
    if(is_float(obj)) return untag_float(obj);
    i64 bits;
    if(try_cast_int(obj, &bits)) return (float)bits;
    vm->TypeError("expected 'int' or 'float', got " + _type_name(vm, vm->_tp(obj)).escape());
    return 0;
}
template<> inline double _py_cast<double>(VM* vm, PyObject* obj){
    return py_cast<double>(vm, obj);
}

#define PY_VAR_INT(T)                                       \
    inline PyObject* py_var(VM* vm, T _val){                \
        i64 val = static_cast<i64>(_val);                   \
        if(val >= Number::kMinSmallInt && val <= Number::kMaxSmallInt){     \
            val = (val << 2) | 0b10;                        \
            return reinterpret_cast<PyObject*>(val);        \
        }else{                                              \
            return vm->heap.gcnew<i64>(vm->tp_int, val);    \
        }                                                   \
    }

PY_VAR_INT(char)
PY_VAR_INT(short)
PY_VAR_INT(int)
PY_VAR_INT(long)
PY_VAR_INT(long long)
PY_VAR_INT(unsigned char)
PY_VAR_INT(unsigned short)
PY_VAR_INT(unsigned int)
PY_VAR_INT(unsigned long)
PY_VAR_INT(unsigned long long)
#undef PY_VAR_INT

inline PyObject* py_var(VM* vm, float _val){
    PK_UNUSED(vm);
    return tag_float(static_cast<f64>(_val));
}

inline PyObject* py_var(VM* vm, double _val){
    PK_UNUSED(vm);
    return tag_float(static_cast<f64>(_val));
}

inline PyObject* py_var(VM* vm, bool val){
    return val ? vm->True : vm->False;
}

template<> inline bool py_cast<bool>(VM* vm, PyObject* obj){
    if(obj == vm->True) return true;
    if(obj == vm->False) return false;
    vm->check_non_tagged_type(obj, vm->tp_bool);
    return false;
}
template<> inline bool _py_cast<bool>(VM* vm, PyObject* obj){
    return obj == vm->True;
}

template<> inline CString py_cast<CString>(VM* vm, PyObject* obj){
    vm->check_non_tagged_type(obj, vm->tp_str);
    return PK_OBJ_GET(Str, obj).c_str();
}

template<> inline CString _py_cast<CString>(VM* vm, PyObject* obj){
    return PK_OBJ_GET(Str, obj).c_str();
}

inline PyObject* py_var(VM* vm, const char* val){
    return VAR(Str(val));
}

template<>
inline const char* py_cast<const char*>(VM* vm, PyObject* obj){
    if(obj == vm->None) return nullptr;
    vm->check_non_tagged_type(obj, vm->tp_str);
    return PK_OBJ_GET(Str, obj).c_str();
}

template<>
inline const char* _py_cast<const char*>(VM* vm, PyObject* obj){
    return PK_OBJ_GET(Str, obj).c_str();
}

inline PyObject* py_var(VM* vm, std::string val){
    return VAR(Str(val));
}

inline PyObject* py_var(VM* vm, std::string_view val){
    return VAR(Str(val));
}

inline PyObject* py_var(VM* vm, NoReturn val){
    PK_UNUSED(val);
    return vm->None;
}

template<int ARGC>
PyObject* VM::bind_method(Type type, Str name, NativeFuncC fn) {
    PyObject* nf = VAR(NativeFunc(fn, ARGC, true));
    _t(type)->attr().set(name, nf);
    return nf;
}

template<int ARGC>
PyObject* VM::bind_method(PyObject* obj, Str name, NativeFuncC fn) {
    check_non_tagged_type(obj, tp_type);
    return bind_method<ARGC>(PK_OBJ_GET(Type, obj), name, fn);
}

template<int ARGC>
PyObject* VM::bind_func(PyObject* obj, Str name, NativeFuncC fn, UserData userdata, BindType bt) {
    PyObject* nf = VAR(NativeFunc(fn, ARGC, false));
    PK_OBJ_GET(NativeFunc, nf).set_userdata(userdata);
    switch(bt){
        case BindType::DEFAULT: break;
        case BindType::STATICMETHOD: nf = VAR(StaticMethod(nf)); break;
        case BindType::CLASSMETHOD: nf = VAR(ClassMethod(nf)); break;
    }
    obj->attr().set(name, nf);
    return nf;
}

/***************************************************/

template<typename T>
PyObject* PyArrayGetItem(VM* vm, PyObject* obj, PyObject* index){
    static_assert(std::is_same_v<T, List> || std::is_same_v<T, Tuple>);
    const T& self = _CAST(T&, obj);

    if(is_non_tagged_type(index, vm->tp_slice)){
        const Slice& s = _CAST(Slice&, index);
        int start, stop, step;
        vm->parse_int_slice(s, self.size(), start, stop, step);
        List new_list;
        for(int i=start; step>0?i<stop:i>stop; i+=step) new_list.push_back(self[i]);
        return VAR(T(std::move(new_list)));
    }

    int i = CAST(int, index);
    i = vm->normalized_index(i, self.size());
    return self[i];
}
}   // namespace pkpy
namespace pkpy{

    struct JsonSerializer{
        VM* vm;
        PyObject* root;
        SStream ss;

        JsonSerializer(VM* vm, PyObject* root) : vm(vm), root(root) {}

        template<typename T>
        void write_array(T& arr){
            ss << '[';
            for(int i=0; i<arr.size(); i++){
                if(i != 0) ss << ", ";
                write_object(arr[i]);
            }
            ss << ']';
        }

        void write_dict(Dict& dict){
            ss << '{';
            bool first = true;
            dict.apply([&](PyObject* k, PyObject* v){
                if(!first) ss << ", ";
                first = false;
                if(!is_non_tagged_type(k, vm->tp_str)){
                    vm->TypeError(_S("json keys must be string, got ", _type_name(vm, vm->_tp(k))));
                }
                ss << _CAST(Str&, k).escape(false) << ": ";
                write_object(v);
            });
            ss << '}';
        }

        void write_object(PyObject* obj){
            Type obj_t = vm->_tp(obj);
            if(obj == vm->None){
                ss << "null";
            }else if(obj_t == vm->tp_int){
                ss << _CAST(i64, obj);
            }else if(obj_t == vm->tp_float){
                f64 val = _CAST(f64, obj);
                if(std::isinf(val) || std::isnan(val)) vm->ValueError("cannot jsonify 'nan' or 'inf'");
                ss << val;
            }else if(obj_t == vm->tp_bool){
                ss << (obj == vm->True ? "true" : "false");
            }else if(obj_t == vm->tp_str){
                _CAST(Str&, obj).escape_(ss, false);
            }else if(obj_t == vm->tp_list){
                write_array<List>(_CAST(List&, obj));
            }else if(obj_t == vm->tp_tuple){
                write_array<Tuple>(_CAST(Tuple&, obj));
            }else if(obj_t == vm->tp_dict){
                write_dict(_CAST(Dict&, obj));
            }else{
                vm->TypeError(_S("unrecognized type ", _type_name(vm, obj_t).escape()));
            }
        }

        Str serialize(){
            auto _lock = vm->heap.gc_scope_lock();
            write_object(root);
            return ss.str();
        }
    };

    VM::VM(bool enable_os) : heap(this), enable_os(enable_os) {
        this->vm = this;
        this->_c.error = nullptr;
        _stdout = [](const char* buf, int size) {
            std::cout.write(buf, size);
        };
        _stderr = [](const char* buf, int size) {
            std::cerr.write(buf, size);
        };
        callstack.reserve(8);
        _main = nullptr;
        _last_exception = nullptr;
        _import_handler = [](const char* name_p, int name_size, int* out_size) -> unsigned char*{
            PK_UNUSED(name_p);
            PK_UNUSED(name_size);
            PK_UNUSED(out_size);
            return nullptr;
        };
        init_builtin_types();
    }

    PyObject* VM::py_str(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__str__) return ti->m__str__(this, obj);
        PyObject* self;
        PyObject* f = get_unbound_method(obj, __str__, &self, false);
        if(self != PY_NULL) return call_method(self, f);
        return py_repr(obj);
    }

    PyObject* VM::py_repr(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__repr__) return ti->m__repr__(this, obj);
        return call_method(obj, __repr__);
    }

    PyObject* VM::py_json(PyObject* obj){
        auto j = JsonSerializer(this, obj);
        return VAR(j.serialize());
    }

    PyObject* VM::py_iter(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__iter__) return ti->m__iter__(this, obj);
        PyObject* self;
        PyObject* iter_f = get_unbound_method(obj, __iter__, &self, false);
        if(self != PY_NULL) return call_method(self, iter_f);
        TypeError(_type_name(vm, _tp(obj)).escape() + " object is not iterable");
        return nullptr;
    }

    FrameId VM::top_frame(){
#if PK_DEBUG_EXTRA_CHECK
        if(callstack.empty()) PK_FATAL_ERROR();
#endif
        return FrameId(&callstack.data(), callstack.size()-1);
    }

    void VM::_pop_frame(){
        Frame* frame = &callstack.top();
        s_data.reset(frame->_sp_base);
        callstack.pop();
    }

    PyObject* VM::find_name_in_mro(Type cls, StrName name){
        PyObject* val;
        do{
            val = _t(cls)->attr().try_get(name);
            if(val != nullptr) return val;
            cls = _all_types[cls].base;
            if(cls.index == -1) break;
        }while(true);
        return nullptr;
    }

    bool VM::isinstance(PyObject* obj, Type base){
        return issubclass(_tp(obj), base);
    }

    bool VM::issubclass(Type cls, Type base){
        do{
            if(cls == base) return true;
            Type next = _all_types[cls].base;
            if(next.index == -1) break;
            cls = next;
        }while(true);
        return false;
    }

    PyObject* VM::exec(std::string_view source, Str filename, CompileMode mode, PyObject* _module){
        if(_module == nullptr) _module = _main;
        try {
            CodeObject_ code = compile(source, filename, mode);
#if PK_DEBUG_DIS_EXEC
            if(_module == _main) std::cout << disassemble(code) << '\n';
#endif
            return _exec(code, _module);
        }catch (const Exception& e){
            stderr_write(e.summary() + "\n");
        }
#if !PK_DEBUG_FULL_EXCEPTION
        catch(const std::exception& e) {
            Str msg = "An std::exception occurred! It could be a bug.\n";
            msg = msg + e.what() + "\n";
            stderr_write(msg);
        }
        catch(NeedMoreLines){
            throw;
        }
        catch(...) {
            Str msg = "An unknown exception occurred! It could be a bug. Please report it to @blueloveTH on GitHub.\n";
            stderr_write(msg);
        }
#endif
        callstack.clear();
        s_data.clear();
        return nullptr;
    }

    PyObject* VM::exec(std::string_view source){
        return exec(source, "main.py", EXEC_MODE);
    }

    PyObject* VM::eval(std::string_view source){
        return exec(source, "<eval>", EVAL_MODE);
    }

    PyObject* VM::new_type_object(PyObject* mod, StrName name, Type base, bool subclass_enabled){
        PyObject* obj = heap._new<Type>(tp_type, _all_types.size());
        const PyTypeInfo& base_info = _all_types[base];
        if(!base_info.subclass_enabled){
            TypeError(_S("type ", base_info.name.escape(), " is not `subclass_enabled`"));
        }
        PyTypeInfo info{
            obj,
            base,
            mod,
            name,
            subclass_enabled,
        };
        _all_types.push_back(info);
        return obj;
    }

    Type VM::_new_type_object(StrName name, Type base, bool subclass_enabled) {
        PyObject* obj = new_type_object(nullptr, name, base, subclass_enabled);
        return PK_OBJ_GET(Type, obj);
    }

    const PyTypeInfo* VM::_inst_type_info(PyObject* obj){
        if(is_int(obj)) return &_all_types[tp_int];
        if(is_float(obj)) return &_all_types[tp_float];
        return &_all_types[obj->type];
    }

    bool VM::py_eq(PyObject* lhs, PyObject* rhs){
        if(lhs == rhs) return true;
        const PyTypeInfo* ti = _inst_type_info(lhs);
        PyObject* res;
        if(ti->m__eq__){
            res = ti->m__eq__(this, lhs, rhs);
            if(res != vm->NotImplemented) return res == vm->True;
        }
        res = call_method(lhs, __eq__, rhs);
        if(res != vm->NotImplemented) return res == vm->True;

        ti = _inst_type_info(rhs);
        if(ti->m__eq__){
            res = ti->m__eq__(this, rhs, lhs);
            if(res != vm->NotImplemented) return res == vm->True;
        }
        res = call_method(rhs, __eq__, lhs);
        if(res != vm->NotImplemented) return res == vm->True;
        return false;
    }

    int VM::normalized_index(int index, int size){
        if(index < 0) index += size;
        if(index < 0 || index >= size){
            IndexError(std::to_string(index) + " not in [0, " + std::to_string(size) + ")");
        }
        return index;
    }

    PyObject* VM::py_next(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__next__) return ti->m__next__(this, obj);
        return call_method(obj, __next__);
    }

    bool VM::py_callable(PyObject* obj){
        Type cls = vm->_tp(obj);
        switch(cls.index){
            case VM::tp_function.index: return vm->True;
            case VM::tp_native_func.index: return vm->True;
            case VM::tp_bound_method.index: return vm->True;
            case VM::tp_type.index: return vm->True;
        }
        return vm->find_name_in_mro(cls, __call__) != nullptr;
    }

    PyObject* VM::py_import(Str path, bool throw_err){
        if(path.empty()) vm->ValueError("empty module name");
        static auto f_join = [](const std::vector<std::string_view>& cpnts){
            SStream ss;
            for(int i=0; i<cpnts.size(); i++){
                if(i != 0) ss << ".";
                ss << cpnts[i];
            }
            return Str(ss.str());
        };

        if(path[0] == '.'){
            if(_import_context.pending.empty()){
                ImportError("relative import outside of package");
            }
            Str curr_path = _import_context.pending.back();
            bool curr_is_init = _import_context.pending_is_init.back();
            // convert relative path to absolute path
            std::vector<std::string_view> cpnts = curr_path.split('.');
            int prefix = 0;     // how many dots in the prefix
            for(int i=0; i<path.length(); i++){
                if(path[i] == '.') prefix++;
                else break;
            }
            if(prefix > cpnts.size()) ImportError("attempted relative import beyond top-level package");
            path = path.substr(prefix);     // remove prefix
            for(int i=(int)curr_is_init; i<prefix; i++) cpnts.pop_back();
            if(!path.empty()) cpnts.push_back(path.sv());
            path = f_join(cpnts);
        }

        PK_ASSERT(path.begin()[0] != '.' && path.end()[-1] != '.');

        // check existing module
        StrName name(path);
        PyObject* ext_mod = _modules.try_get(name);
        if(ext_mod != nullptr) return ext_mod;

        std::vector<std::string_view> path_cpnts = path.split('.');
        // check circular import
        if(_import_context.pending.size() > 128){
            ImportError("maximum recursion depth exceeded while importing");
        }

        // try import
        Str filename = path.replace('.', PK_PLATFORM_SEP) + ".py";
        Str source;
        bool is_init = false;
        auto it = _lazy_modules.find(name);
        if(it == _lazy_modules.end()){
            int out_size;
            unsigned char* out = _import_handler(filename.data, filename.size, &out_size);
            if(out == nullptr){
                filename = path.replace('.', PK_PLATFORM_SEP).str() + PK_PLATFORM_SEP + "__init__.py";
                is_init = true;
                out = _import_handler(filename.data, filename.size, &out_size);
            }
            if(out == nullptr){
                if(throw_err) ImportError(_S("module ", path.escape(), " not found"));
                else return nullptr;
            }
            PK_ASSERT(out_size >= 0)
            source = Str(std::string_view((char*)out, out_size));
            free(out);
        }else{
            source = it->second;
            _lazy_modules.erase(it);
        }
        auto _ = _import_context.scope(path, is_init);
        CodeObject_ code = compile(source, filename, EXEC_MODE);

        Str name_cpnt = path_cpnts.back();
        path_cpnts.pop_back();
        PyObject* new_mod = new_module(name_cpnt, f_join(path_cpnts));
        _exec(code, new_mod);
        return new_mod;
    }

    VM::~VM() {
        callstack.clear();
        s_data.clear();
        _all_types.clear();
        _modules.clear();
        _lazy_modules.clear();
    }

PyObject* VM::py_negate(PyObject* obj){
    const PyTypeInfo* ti = _inst_type_info(obj);
    if(ti->m__neg__) return ti->m__neg__(this, obj);
    return call_method(obj, __neg__);
}

bool VM::py_bool(PyObject* obj){
    if(obj == vm->True) return true;
    if(obj == vm->False) return false;
    if(obj == None) return false;
    if(is_int(obj)) return _CAST(i64, obj) != 0;
    if(is_float(obj)) return _CAST(f64, obj) != 0.0;
    PyObject* self;
    PyObject* len_f = get_unbound_method(obj, __len__, &self, false);
    if(self != PY_NULL){
        PyObject* ret = call_method(self, len_f);
        return CAST(i64, ret) > 0;
    }
    return true;
}

PyObject* VM::py_list(PyObject* it){
    auto _lock = heap.gc_scope_lock();
    it = py_iter(it);
    List list;
    PyObject* obj = py_next(it);
    while(obj != StopIteration){
        list.push_back(obj);
        obj = py_next(it);
    }
    return VAR(std::move(list));
}



void VM::parse_int_slice(const Slice& s, int length, int& start, int& stop, int& step){
    auto clip = [](int value, int min, int max){
        if(value < min) return min;
        if(value > max) return max;
        return value;
    };
    if(s.step == None) step = 1;
    else step = CAST(int, s.step);
    if(step == 0) ValueError("slice step cannot be zero");
    if(step > 0){
        if(s.start == None){
            start = 0;
        }else{
            start = CAST(int, s.start);
            if(start < 0) start += length;
            start = clip(start, 0, length);
        }
        if(s.stop == None){
            stop = length;
        }else{
            stop = CAST(int, s.stop);
            if(stop < 0) stop += length;
            stop = clip(stop, 0, length);
        }
    }else{
        if(s.start == None){
            start = length - 1;
        }else{
            start = CAST(int, s.start);
            if(start < 0) start += length;
            start = clip(start, -1, length - 1);
        }
        if(s.stop == None){
            stop = -1;
        }else{
            stop = CAST(int, s.stop);
            if(stop < 0) stop += length;
            stop = clip(stop, -1, length - 1);
        }
    }
}

i64 VM::py_hash(PyObject* obj){
    // https://docs.python.org/3.10/reference/datamodel.html#object.__hash__
    const PyTypeInfo* ti = _inst_type_info(obj);
    if(ti->m__hash__) return ti->m__hash__(this, obj);

    PyObject* self;
    PyObject* f = get_unbound_method(obj, __hash__, &self, false);
    if(f != nullptr){
        PyObject* ret = call_method(self, f);
        return CAST(i64, ret);
    }
    // if it is trivial `object`, return PK_BITS
    if(ti == &_all_types[tp_object]) return PK_BITS(obj);
    // otherwise, we check if it has a custom __eq__ other than object.__eq__
    bool has_custom_eq = false;
    if(ti->m__eq__) has_custom_eq = true;
    else{
        f = get_unbound_method(obj, __eq__, &self, false);
        has_custom_eq = f != _t(tp_object)->attr(__eq__);
    }
    if(has_custom_eq){
        TypeError(_S("unhashable type: ", ti->name.escape()));
        PK_UNREACHABLE()
    }else{
        return PK_BITS(obj);
    }
}

PyObject* VM::_format_string(Str spec, PyObject* obj){
    if(spec.empty()) return py_str(obj);
    char type;
    switch(spec.end()[-1]){
        case 'f': case 'd': case 's':
            type = spec.end()[-1];
            spec = spec.substr(0, spec.length() - 1);
            break;
        default: type = ' '; break;
    }

    char pad_c = ' ';
    for(char c: std::string_view("0-=*#@!~")){
        if(spec[0] == c){
            pad_c = c;
            spec = spec.substr(1);
            break;
        }
    }
    char align;
    if(spec[0] == '^'){
        align = '^';
        spec = spec.substr(1);
    }else if(spec[0] == '>'){
        align = '>';
        spec = spec.substr(1);
    }else if(spec[0] == '<'){
        align = '<';
        spec = spec.substr(1);
    }else{
        if(is_int(obj) || is_float(obj)) align = '>';
        else align = '<';
    }

    int dot = spec.index(".");
    int width, precision;
    try{
        if(dot >= 0){
            if(dot == 0){
                width = -1;
            }else{
                width = std::stoi(spec.substr(0, dot).str());
            }
            precision = std::stoi(spec.substr(dot+1).str());
        }else{
            width = std::stoi(spec.str());
            precision = -1;
        }
    }catch(...){
        ValueError("invalid format specifer");
    }

    if(type != 'f' && dot >= 0) ValueError("precision not allowed in the format specifier");
    Str ret;
    if(type == 'f'){
        f64 val = CAST(f64, obj);
        if(precision < 0) precision = 6;
        SStream ss;
        ss.setprecision(precision);
        ss << val;
        ret = ss.str();
    }else if(type == 'd'){
        ret = std::to_string(CAST(i64, obj));
    }else if(type == 's'){
        ret = CAST(Str&, obj);
    }else{
        ret = CAST(Str&, py_str(obj));
    }
    if(width != -1 && width > ret.length()){
        int pad = width - ret.length();
        if(align == '>' || align == '<'){
            std::string padding(pad, pad_c);
            if(align == '>') ret = padding.c_str() + ret;
            else ret = ret + padding.c_str();
        }else{  // ^
            int pad_left = pad / 2;
            int pad_right = pad - pad_left;
            std::string padding_left(pad_left, pad_c);
            std::string padding_right(pad_right, pad_c);
            ret = padding_left.c_str() + ret + padding_right.c_str();
        }
    }
    return VAR(ret);
}

PyObject* VM::new_module(Str name, Str package) {
    PyObject* obj = heap._new<DummyModule>(tp_module);
    obj->attr().set(__name__, VAR(name));
    obj->attr().set(__package__, VAR(package));
    // convert to fullname
    if(!package.empty()) name = package + "." + name;
    obj->attr().set(__path__, VAR(name));

    // we do not allow override in order to avoid memory leak
    // it is because Module objects are not garbage collected
    if(_modules.contains(name)){
        throw std::runtime_error(_S("module ", name.escape(), " already exists").str());
    }
    // set it into _modules
    _modules.set(name, obj);
    return obj;
}

static std::string _opcode_argstr(VM* vm, Bytecode byte, const CodeObject* co){
    std::string argStr = std::to_string(byte.arg);
    switch(byte.op){
        case OP_LOAD_CONST: case OP_FORMAT_STRING: case OP_IMPORT_PATH:
            if(vm != nullptr){
                argStr += _S(" (", CAST(Str, vm->py_repr(co->consts[byte.arg])), ")").sv();
            }
            break;
        case OP_LOAD_NAME: case OP_LOAD_GLOBAL: case OP_LOAD_NONLOCAL: case OP_STORE_GLOBAL:
        case OP_LOAD_ATTR: case OP_LOAD_METHOD: case OP_STORE_ATTR: case OP_DELETE_ATTR:
        case OP_BEGIN_CLASS: case OP_GOTO:
        case OP_DELETE_GLOBAL: case OP_INC_GLOBAL: case OP_DEC_GLOBAL: case OP_STORE_CLASS_ATTR:
            argStr += _S(" (", StrName(byte.arg).sv(), ")").sv();
            break;
        case OP_LOAD_FAST: case OP_STORE_FAST: case OP_DELETE_FAST: case OP_INC_FAST: case OP_DEC_FAST:
            argStr += _S(" (", co->varnames[byte.arg].sv(), ")").sv();
            break;
        case OP_LOAD_FUNCTION:
            argStr += _S(" (", co->func_decls[byte.arg]->code->name, ")").sv();
            break;
    }
    return argStr;
}

Str VM::disassemble(CodeObject_ co){
    auto pad = [](const Str& s, const int n){
        if(s.length() >= n) return s.substr(0, n);
        return s + std::string(n - s.length(), ' ');
    };

    std::vector<int> jumpTargets;
    for(auto byte : co->codes){
        if(byte.op == OP_JUMP_ABSOLUTE || byte.op == OP_POP_JUMP_IF_FALSE || byte.op == OP_SHORTCUT_IF_FALSE_OR_POP || byte.op == OP_FOR_ITER){
            jumpTargets.push_back(byte.arg);
        }
        if(byte.op == OP_GOTO){
            // TODO: pre-compute jump targets for OP_GOTO
            int* target = co->labels.try_get_2_likely_found(StrName(byte.arg));
            if(target != nullptr) jumpTargets.push_back(*target);
        }
    }
    SStream ss;
    int prev_line = -1;
    for(int i=0; i<co->codes.size(); i++){
        const Bytecode& byte = co->codes[i];
        Str line = std::to_string(co->lines[i]);
        if(co->lines[i] == prev_line) line = "";
        else{
            if(prev_line != -1) ss << "\n";
            prev_line = co->lines[i];
        }

        std::string pointer;
        if(std::find(jumpTargets.begin(), jumpTargets.end(), i) != jumpTargets.end()){
            pointer = "-> ";
        }else{
            pointer = "   ";
        }
        ss << pad(line, 8) << pointer << pad(std::to_string(i), 3);
        ss << " " << pad(OP_NAMES[byte.op], 25) << " ";
        // ss << pad(byte.arg == -1 ? "" : std::to_string(byte.arg), 5);
        std::string argStr = _opcode_argstr(this, byte, co.get());
        ss << argStr;
        // ss << pad(argStr, 40);      // may overflow
        // ss << co->blocks[byte.block].type;
        if(i != co->codes.size() - 1) ss << '\n';
    }

    for(auto& decl: co->func_decls){
        ss << "\n\n" << "Disassembly of " << decl->code->name << ":\n";
        ss << disassemble(decl->code);
    }
    ss << "\n";
    return Str(ss.str());
}

#if PK_DEBUG_CEVAL_STEP
void VM::_log_s_data(const char* title) {
    if(_main == nullptr) return;
    if(callstack.empty()) return;
    SStream ss;
    if(title) ss << title << " | ";
    std::map<PyObject**, int> sp_bases;
    for(Frame& f: callstack.data()){
        if(f._sp_base == nullptr) PK_FATAL_ERROR();
        sp_bases[f._sp_base] += 1;
    }
    FrameId frame = top_frame();
    int line = frame->co->lines[frame->_ip];
    ss << frame->co->name << ":" << line << " [";
    for(PyObject** p=s_data.begin(); p!=s_data.end(); p++){
        ss << std::string(sp_bases[p], '|');
        if(sp_bases[p] > 0) ss << " ";
        PyObject* obj = *p;
        if(obj == nullptr) ss << "(nil)";
        else if(obj == PY_NULL) ss << "NULL";
        else if(is_int(obj)) ss << CAST(i64, obj);
        else if(is_float(obj)) ss << CAST(f64, obj);
        else if(is_type(obj, tp_str)) ss << CAST(Str, obj).escape();
        else if(obj == None) ss << "None";
        else if(obj == True) ss << "True";
        else if(obj == False) ss << "False";
        else if(is_type(obj, tp_function)){
            auto& f = CAST(Function&, obj);
            ss << f.decl->code->name << "(...)";
        } else if(is_type(obj, tp_type)){
            Type t = PK_OBJ_GET(Type, obj);
            ss << "<class " + _all_types[t].name.escape() + ">";
        } else if(is_type(obj, tp_list)){
            auto& t = CAST(List&, obj);
            ss << "list(size=" << t.size() << ")";
        } else if(is_type(obj, tp_tuple)){
            auto& t = CAST(Tuple&, obj);
            ss << "tuple(size=" << t.size() << ")";
        } else ss << "(" << _type_name(this, obj->type) << ")";
        ss << ", ";
    }
    std::string output = ss.str();
    if(!s_data.empty()) {
        output.pop_back(); output.pop_back();
    }
    output.push_back(']');
    Bytecode byte = frame->co->codes[frame->_ip];
    std::cout << output << " " << OP_NAMES[byte.op] << " " << _opcode_argstr(nullptr, byte, frame->co) << std::endl;
}
#endif

void VM::init_builtin_types(){
    _all_types.push_back({heap._new<Type>(Type(1), Type(0)), -1, nullptr, "object", true});
    _all_types.push_back({heap._new<Type>(Type(1), Type(1)), 0, nullptr, "type", false});

    if(tp_int != _new_type_object("int")) exit(-3);
    if((tp_float != _new_type_object("float"))) exit(-3);

    if(tp_bool != _new_type_object("bool")) exit(-3);
    if(tp_str != _new_type_object("str")) exit(-3);
    if(tp_list != _new_type_object("list")) exit(-3);
    if(tp_tuple != _new_type_object("tuple")) exit(-3);

    if(tp_slice != _new_type_object("slice")) exit(-3);
    if(tp_range != _new_type_object("range")) exit(-3);
    if(tp_module != _new_type_object("module")) exit(-3);
    if(tp_function != _new_type_object("function")) exit(-3);
    if(tp_native_func != _new_type_object("native_func")) exit(-3);
    if(tp_bound_method != _new_type_object("bound_method")) exit(-3);

    if(tp_super != _new_type_object("super")) exit(-3);
    if(tp_exception != _new_type_object("Exception", 0, true)) exit(-3);
    if(tp_bytes != _new_type_object("bytes")) exit(-3);
    if(tp_mappingproxy != _new_type_object("mappingproxy")) exit(-3);
    if(tp_dict != _new_type_object("dict")) exit(-3);
    if(tp_property != _new_type_object("property")) exit(-3);
    if(tp_star_wrapper != _new_type_object("_star_wrapper")) exit(-3);

    if(tp_staticmethod != _new_type_object("staticmethod")) exit(-3);
    if(tp_classmethod != _new_type_object("classmethod")) exit(-3);

    // SyntaxError and IndentationError must be created here
    Type tp_syntax_error = _new_type_object("SyntaxError", tp_exception, true);
    Type tp_indentation_error = _new_type_object("IndentationError", tp_syntax_error, true);

    this->None = heap._new<Dummy>(_new_type_object("NoneType"));
    this->NotImplemented = heap._new<Dummy>(_new_type_object("NotImplementedType"));
    this->Ellipsis = heap._new<Dummy>(_new_type_object("ellipsis"));
    this->True = heap._new<Dummy>(tp_bool);
    this->False = heap._new<Dummy>(tp_bool);
    this->StopIteration = heap._new<Dummy>(_new_type_object("StopIterationType"));

    this->builtins = new_module("builtins");
    
    // setup public types
    builtins->attr().set("type", _t(tp_type));
    builtins->attr().set("object", _t(tp_object));
    builtins->attr().set("bool", _t(tp_bool));
    builtins->attr().set("int", _t(tp_int));
    builtins->attr().set("float", _t(tp_float));
    builtins->attr().set("str", _t(tp_str));
    builtins->attr().set("list", _t(tp_list));
    builtins->attr().set("tuple", _t(tp_tuple));
    builtins->attr().set("range", _t(tp_range));
    builtins->attr().set("bytes", _t(tp_bytes));
    builtins->attr().set("dict", _t(tp_dict));
    builtins->attr().set("property", _t(tp_property));
    builtins->attr().set("StopIteration", StopIteration);
    builtins->attr().set("NotImplemented", NotImplemented);
    builtins->attr().set("slice", _t(tp_slice));
    builtins->attr().set("Exception", _t(tp_exception));
    builtins->attr().set("SyntaxError", _t(tp_syntax_error));
    builtins->attr().set("IndentationError", _t(tp_indentation_error));

    post_init();
    this->_main = new_module("__main__");
}

// `heap.gc_scope_lock();` needed before calling this function
void VM::_unpack_as_list(ArgsView args, List& list){
    for(PyObject* obj: args){
        if(is_non_tagged_type(obj, tp_star_wrapper)){
            const StarWrapper& w = _CAST(StarWrapper&, obj);
            // maybe this check should be done in the compile time
            if(w.level != 1) TypeError("expected level 1 star wrapper");
            PyObject* _0 = py_iter(w.obj);
            PyObject* _1 = py_next(_0);
            while(_1 != StopIteration){
                list.push_back(_1);
                _1 = py_next(_0);
            }
        }else{
            list.push_back(obj);
        }
    }
}

// `heap.gc_scope_lock();` needed before calling this function
void VM::_unpack_as_dict(ArgsView args, Dict& dict){
    for(PyObject* obj: args){
        if(is_non_tagged_type(obj, tp_star_wrapper)){
            const StarWrapper& w = _CAST(StarWrapper&, obj);
            // maybe this check should be done in the compile time
            if(w.level != 2) TypeError("expected level 2 star wrapper");
            const Dict& other = CAST(Dict&, w.obj);
            dict.update(other);
        }else{
            const Tuple& t = CAST(Tuple&, obj);
            if(t.size() != 2) TypeError("expected tuple of length 2");
            dict.set(t[0], t[1]);
        }
    }
}


void VM::_prepare_py_call(PyObject** buffer, ArgsView args, ArgsView kwargs, const FuncDecl_& decl){
    const CodeObject* co = decl->code.get();
    int co_nlocals = co->varnames.size();
    int decl_argc = decl->args.size();

    if(args.size() < decl_argc){
        vm->TypeError(_S(
            co->name, "() takes ", decl_argc, " positional arguments but ", args.size(), " were given"
        ));
    }

    int i = 0;
    // prepare args
    for(int index: decl->args) buffer[index] = args[i++];
    // set extra varnames to PY_NULL
    for(int j=i; j<co_nlocals; j++) buffer[j] = PY_NULL;
    // prepare kwdefaults
    for(auto& kv: decl->kwargs) buffer[kv.index] = kv.value;
    
    // handle *args
    if(decl->starred_arg != -1){
        ArgsView vargs(args.begin() + i, args.end());
        buffer[decl->starred_arg] = VAR(vargs.to_tuple());
        i += vargs.size();
    }else{
        // kwdefaults override
        for(auto& kv: decl->kwargs){
            if(i >= args.size()) break;
            buffer[kv.index] = args[i++];
        }
        if(i < args.size()) TypeError(_S("too many arguments", " (", decl->code->name, ')'));
    }
    
    PyObject* vkwargs;
    if(decl->starred_kwarg != -1){
        vkwargs = VAR(Dict(this));
        buffer[decl->starred_kwarg] = vkwargs;
    }else{
        vkwargs = nullptr;
    }

    for(int j=0; j<kwargs.size(); j+=2){
        StrName key(CAST(int, kwargs[j]));
        int index = decl->kw_to_index.try_get_likely_found(key);
        // if key is an explicit key, set as local variable
        if(index >= 0){
            buffer[index] = kwargs[j+1];
        }else{
            // otherwise, set as **kwargs if possible
            if(vkwargs == nullptr){
                TypeError(_S(key.escape(), " is an invalid keyword argument for ", co->name, "()"));
            }else{
                Dict& dict = _CAST(Dict&, vkwargs);
                dict.set(VAR(key.sv()), kwargs[j+1]);
            }
        }
    }
}

PyObject* VM::vectorcall(int ARGC, int KWARGC, bool op_call){
    PyObject** p1 = s_data._sp - KWARGC*2;
    PyObject** p0 = p1 - ARGC - 2;
    // [callable, <self>, args..., kwargs...]
    //      ^p0                    ^p1      ^_sp
    PyObject* callable = p1[-(ARGC + 2)];
    bool method_call = p1[-(ARGC + 1)] != PY_NULL;

    // handle boundmethod, do a patch
    if(is_non_tagged_type(callable, tp_bound_method)){
        if(method_call) PK_FATAL_ERROR();
        BoundMethod& bm = PK_OBJ_GET(BoundMethod, callable);
        callable = bm.func;      // get unbound method
        p1[-(ARGC + 2)] = bm.func;
        p1[-(ARGC + 1)] = bm.self;
        method_call = true;
        // [unbound, self, args..., kwargs...]
    }

    ArgsView args(p1 - ARGC - int(method_call), p1);
    ArgsView kwargs(p1, s_data._sp);

    PyObject** _base = args.begin();
    PyObject* buffer[PK_MAX_CO_VARNAMES];

    if(is_non_tagged_type(callable, tp_native_func)){
        const auto& f = PK_OBJ_GET(NativeFunc, callable);
        PyObject* ret;
        if(f.decl != nullptr){
            int co_nlocals = f.decl->code->varnames.size();
            _prepare_py_call(buffer, args, kwargs, f.decl);
            // copy buffer back to stack
            s_data.reset(_base + co_nlocals);
            for(int j=0; j<co_nlocals; j++) _base[j] = buffer[j];
            ret = f.call(vm, ArgsView(s_data._sp - co_nlocals, s_data._sp));
        }else{
            if(KWARGC != 0) TypeError("old-style native_func does not accept keyword arguments");
            f.check_size(this, args);
            ret = f.call(this, args);
        }
        s_data.reset(p0);
        return ret;
    }

    if(is_non_tagged_type(callable, tp_function)){
        /*****************_py_call*****************/
        // callable must be a `function` object
        if(s_data.is_overflow()) StackOverflowError();

        const Function& fn = PK_OBJ_GET(Function, callable);
        const FuncDecl_& decl = fn.decl;
        const CodeObject* co = decl->code.get();
        int co_nlocals = co->varnames.size();
        if(decl->is_simple){
            if(args.size() != decl->args.size()){
                TypeError(_S(
                    co->name, "() takes ", decl->args.size(), " positional arguments but ", args.size(), " were given"
                ));
            }
            if(!kwargs.empty()){
                TypeError(_S(co->name, "() takes no keyword arguments"));
            }
            s_data.reset(_base + co_nlocals);
            int i = 0;
            // prepare args
            for(int index: decl->args) _base[index] = args[i++];
            // set extra varnames to PY_NULL
            for(int j=i; j<co_nlocals; j++) _base[j] = PY_NULL;
            goto __FAST_CALL;
        }

        _prepare_py_call(buffer, args, kwargs, decl);
        
        if(co->is_generator){
            s_data.reset(p0);
            return _py_generator(
                Frame(&s_data, nullptr, co, fn._module, callable),
                ArgsView(buffer, buffer + co_nlocals)
            );
        }

        // copy buffer back to stack
        s_data.reset(_base + co_nlocals);
        for(int j=0; j<co_nlocals; j++) _base[j] = buffer[j];

__FAST_CALL:
        callstack.emplace(&s_data, p0, co, fn._module, callable, FastLocals(co, args.begin()));
        if(op_call) return PY_OP_CALL;
        return _run_top_frame();
        /*****************_py_call*****************/
    }

    if(is_non_tagged_type(callable, tp_type)){
        if(method_call) PK_FATAL_ERROR();
        // [type, NULL, args..., kwargs...]
        PyObject* new_f = find_name_in_mro(PK_OBJ_GET(Type, callable), __new__);
        PyObject* obj;
#if PK_DEBUG_EXTRA_CHECK
        PK_ASSERT(new_f != nullptr);
#endif
        if(new_f == cached_object__new__) {
            // fast path for object.__new__
            Type t = PK_OBJ_GET(Type, callable);
            obj = vm->heap.gcnew<DummyInstance>(t);
        }else{
            PUSH(new_f);
            PUSH(PY_NULL);
            PUSH(callable);    // cls
            for(PyObject* o: args) PUSH(o);
            for(PyObject* o: kwargs) PUSH(o);
            // if obj is not an instance of callable, the behavior is undefined
            obj = vectorcall(ARGC+1, KWARGC);
        }

        // __init__
        PyObject* self;
        callable = get_unbound_method(obj, __init__, &self, false);
        if (self != PY_NULL) {
            // replace `NULL` with `self`
            p1[-(ARGC + 2)] = callable;
            p1[-(ARGC + 1)] = self;
            // [init_f, self, args..., kwargs...]
            vectorcall(ARGC, KWARGC);
            // We just discard the return value of `__init__`
            // in cpython it raises a TypeError if the return value is not None
        }else{
            // manually reset the stack
            s_data.reset(p0);
        }
        return obj;
    }

    // handle `__call__` overload
    PyObject* self;
    PyObject* call_f = get_unbound_method(callable, __call__, &self, false);
    if(self != PY_NULL){
        p1[-(ARGC + 2)] = call_f;
        p1[-(ARGC + 1)] = self;
        // [call_f, self, args..., kwargs...]
        return vectorcall(ARGC, KWARGC, false);
    }
    TypeError(_type_name(vm, _tp(callable)).escape() + " object is not callable");
    PK_UNREACHABLE()
}

void VM::delattr(PyObject *_0, StrName _name){
    const PyTypeInfo* ti = _inst_type_info(_0);
    if(ti->m__delattr__ && ti->m__delattr__(this, _0, _name)) return;
    if(is_tagged(_0) || !_0->is_attr_valid()) TypeError("cannot delete attribute");
    if(!_0->attr().del(_name)) AttributeError(_0, _name);
}

// https://docs.python.org/3/howto/descriptor.html#invocation-from-an-instance
PyObject* VM::getattr(PyObject* obj, StrName name, bool throw_err){
    Type objtype(0);
    // handle super() proxy
    if(is_non_tagged_type(obj, tp_super)){
        const Super& super = PK_OBJ_GET(Super, obj);
        obj = super.first;
        objtype = super.second;
    }else{
        objtype = _tp(obj);
    }
    PyObject* cls_var = find_name_in_mro(objtype, name);
    if(cls_var != nullptr){
        // handle descriptor
        if(is_non_tagged_type(cls_var, tp_property)){
            const Property& prop = PK_OBJ_GET(Property, cls_var);
            return call(prop.getter, obj);
        }
    }
    // handle instance __dict__
    if(!is_tagged(obj) && obj->is_attr_valid()){
        PyObject* val;
        if(obj->type == tp_type){
            val = find_name_in_mro(PK_OBJ_GET(Type, obj), name);
            if(val != nullptr){
                if(is_tagged(val)) return val;
                if(val->type == tp_staticmethod) return PK_OBJ_GET(StaticMethod, val).func;
                if(val->type == tp_classmethod) return VAR(BoundMethod(obj, PK_OBJ_GET(ClassMethod, val).func));
                return val;
            }
        }else{
            val = obj->attr().try_get_likely_found(name);
            if(val != nullptr) return val;
        }
    }
    if(cls_var != nullptr){
        // bound method is non-data descriptor
        if(!is_tagged(cls_var)){
            switch(cls_var->type){
                case tp_function.index:
                    return VAR(BoundMethod(obj, cls_var));
                case tp_native_func.index:
                    return VAR(BoundMethod(obj, cls_var));
                case tp_staticmethod.index:
                    return PK_OBJ_GET(StaticMethod, cls_var).func;
                case tp_classmethod.index:
                    return VAR(BoundMethod(_t(objtype), PK_OBJ_GET(ClassMethod, cls_var).func));
            }
        }
        return cls_var;
    }

    const PyTypeInfo* ti = &_all_types[objtype];
    if(ti->m__getattr__){
        PyObject* ret = ti->m__getattr__(this, obj, name);
        if(ret) return ret;
    }

    if(throw_err) AttributeError(obj, name);
    return nullptr;
}

// used by OP_LOAD_METHOD
// try to load a unbound method (fallback to `getattr` if not found)
PyObject* VM::get_unbound_method(PyObject* obj, StrName name, PyObject** self, bool throw_err, bool fallback){
    *self = PY_NULL;
    Type objtype(0);
    // handle super() proxy
    if(is_non_tagged_type(obj, tp_super)){
        const Super& super = PK_OBJ_GET(Super, obj);
        obj = super.first;
        objtype = super.second;
    }else{
        objtype = _tp(obj);
    }
    PyObject* cls_var = find_name_in_mro(objtype, name);

    if(fallback){
        if(cls_var != nullptr){
            // handle descriptor
            if(is_non_tagged_type(cls_var, tp_property)){
                const Property& prop = PK_OBJ_GET(Property, cls_var);
                return call(prop.getter, obj);
            }
        }
        // handle instance __dict__
        if(!is_tagged(obj) && obj->is_attr_valid()){
            PyObject* val;
            if(obj->type == tp_type){
                val = find_name_in_mro(PK_OBJ_GET(Type, obj), name);
                if(val != nullptr){
                    if(is_tagged(val)) return val;
                    if(val->type == tp_staticmethod) return PK_OBJ_GET(StaticMethod, val).func;
                    if(val->type == tp_classmethod) return VAR(BoundMethod(obj, PK_OBJ_GET(ClassMethod, val).func));
                    return val;
                }
            }else{
                val = obj->attr().try_get_likely_found(name);
                if(val != nullptr) return val;
            }
        }
    }

    if(cls_var != nullptr){
        if(!is_tagged(cls_var)){
            switch(cls_var->type){
                case tp_function.index:
                    *self = obj;
                    break;
                case tp_native_func.index:
                    *self = obj;
                    break;
                case tp_staticmethod.index:
                    *self = PY_NULL;
                    return PK_OBJ_GET(StaticMethod, cls_var).func;
                case tp_classmethod.index:
                    *self = _t(objtype);
                    return PK_OBJ_GET(ClassMethod, cls_var).func;
            }
        }
        return cls_var;
    }

    const PyTypeInfo* ti = &_all_types[objtype];
    if(fallback && ti->m__getattr__){
        PyObject* ret = ti->m__getattr__(this, obj, name);
        if(ret) return ret;
    }

    if(throw_err) AttributeError(obj, name);
    return nullptr;
}

void VM::setattr(PyObject* obj, StrName name, PyObject* value){
    Type objtype(0);
    // handle super() proxy
    if(is_non_tagged_type(obj, tp_super)){
        Super& super = PK_OBJ_GET(Super, obj);
        obj = super.first;
        objtype = super.second;
    }else{
        objtype = _tp(obj);
    }
    PyObject* cls_var = find_name_in_mro(objtype, name);
    if(cls_var != nullptr){
        // handle descriptor
        if(is_non_tagged_type(cls_var, tp_property)){
            const Property& prop = _CAST(Property&, cls_var);
            if(prop.setter != vm->None){
                call(prop.setter, obj, value);
            }else{
                TypeError(_S("readonly attribute: ", name.escape()));
            }
            return;
        }
    }

    const PyTypeInfo* ti = &_all_types[objtype];
    if(ti->m__setattr__){
        ti->m__setattr__(this, obj, name, value);
        return;
    }

    // handle instance __dict__
    if(is_tagged(obj) || !obj->is_attr_valid()) TypeError("cannot set attribute");
    obj->attr().set(name, value);
}

PyObject* VM::bind(PyObject* obj, const char* sig, NativeFuncC fn, UserData userdata, BindType bt){
    return bind(obj, sig, nullptr, fn, userdata, bt);
}

PyObject* VM::bind(PyObject* obj, const char* sig, const char* docstring, NativeFuncC fn, UserData userdata, BindType bt){
    CodeObject_ co;
    try{
        // fn(a, b, *c, d=1) -> None
        co = compile(_S("def ", sig, " : pass"), "<bind>", EXEC_MODE);
    }catch(const Exception&){
        throw std::runtime_error("invalid signature: " + std::string(sig));
    }
    if(co->func_decls.size() != 1){
        throw std::runtime_error("expected 1 function declaration");
    }
    FuncDecl_ decl = co->func_decls[0];
    decl->signature = Str(sig);
    if(docstring != nullptr){
        decl->docstring = Str(docstring).strip();
    }
    PyObject* f_obj = VAR(NativeFunc(fn, decl));
    PK_OBJ_GET(NativeFunc, f_obj).set_userdata(userdata);

    switch(bt){
        case BindType::STATICMETHOD:
            f_obj = VAR(StaticMethod(f_obj));
            break;
        case BindType::CLASSMETHOD:
            f_obj = VAR(ClassMethod(f_obj));
            break;
        case BindType::DEFAULT:
            break;
    }
    if(obj != nullptr) obj->attr().set(decl->code->name, f_obj);
    return f_obj;
}

PyObject* VM::bind_property(PyObject* obj, Str name, NativeFuncC fget, NativeFuncC fset){
    PyObject* _0 = heap.gcnew<NativeFunc>(tp_native_func, fget, 1, false);
    PyObject* _1 = vm->None;
    if(fset != nullptr) _1 = heap.gcnew<NativeFunc>(tp_native_func, fset, 2, false);
    Str signature = name;
    int pos = name.index(":");
    if(pos > 0) name = name.substr(0, pos).strip();
    PyObject* prop = VAR(Property(_0, _1, signature));
    obj->attr().set(name, prop);
    return prop;
}

void VM::_builtin_error(StrName type){ _error(call(builtins->attr(type))); }
void VM::_builtin_error(StrName type, PyObject* arg){ _error(call(builtins->attr(type), arg)); }
void VM::_builtin_error(StrName type, const Str& msg){ _builtin_error(type, VAR(msg)); }

void VM::_error(PyObject* e_obj){
    PK_ASSERT(isinstance(e_obj, tp_exception))
    Exception& e = PK_OBJ_GET(Exception, e_obj);
    if(callstack.empty()){
        e.is_re = false;
        throw e;
    }
    PUSH(e_obj);
    _raise();
}

void VM::_raise(bool re_raise){
    Frame* frame = top_frame().get();
    Exception& e = PK_OBJ_GET(Exception, s_data.top());
    if(!re_raise){
        e._ip_on_error = frame->_ip;
        e._code_on_error = (void*)frame->co;
    }
    bool ok = frame->jump_to_exception_handler();

    int actual_ip = frame->_ip;
    if(e._ip_on_error >= 0 && e._code_on_error == (void*)frame->co) actual_ip = e._ip_on_error;
    int current_line = frame->co->lines[actual_ip];         // current line
    auto current_f_name = frame->co->name.sv();             // current function name
    if(frame->_callable == nullptr) current_f_name = "";    // not in a function
    e.st_push(frame->co->src, current_line, nullptr, current_f_name);

    if(ok) throw HandledException();
    else throw UnhandledException();
}

void ManagedHeap::mark() {
    for(PyObject* obj: _no_gc) PK_OBJ_MARK(obj);
    for(auto& frame : vm->callstack.data()) frame._gc_mark();
    for(PyObject* obj: vm->s_data) PK_OBJ_MARK(obj);
    for(auto [_, co]: vm->_cached_codes) co->_gc_mark();
    if(vm->_last_exception) PK_OBJ_MARK(vm->_last_exception);
    if(vm->_curr_class) PK_OBJ_MARK(vm->_curr_class);
    if(vm->_c.error != nullptr) PK_OBJ_MARK(vm->_c.error);
    if(_gc_marker_ex) _gc_marker_ex(vm);
}

StrName _type_name(VM *vm, Type type){
    return vm->_all_types[type].name;
}


void VM::bind__getitem__(Type type, PyObject* (*f)(VM*, PyObject*, PyObject*)){
    _all_types[type].m__getitem__ = f;
    PyObject* nf = bind_method<1>(type, "__getitem__", [](VM* vm, ArgsView args){
        return lambda_get_userdata<PyObject*(*)(VM*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1]);
    });
    PK_OBJ_GET(NativeFunc, nf).set_userdata(f);
}

void VM::bind__setitem__(Type type, void (*f)(VM*, PyObject*, PyObject*, PyObject*)){
    _all_types[type].m__setitem__ = f;
    PyObject* nf = bind_method<2>(type, "__setitem__", [](VM* vm, ArgsView args){
        lambda_get_userdata<void(*)(VM* vm, PyObject*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1], args[2]);
        return vm->None;
    });
    PK_OBJ_GET(NativeFunc, nf).set_userdata(f);
}

void VM::bind__delitem__(Type type, void (*f)(VM*, PyObject*, PyObject*)){
    _all_types[type].m__delitem__ = f;
    PyObject* nf = bind_method<1>(type, "__delitem__", [](VM* vm, ArgsView args){
        lambda_get_userdata<void(*)(VM*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1]);
        return vm->None;
    });
    PK_OBJ_GET(NativeFunc, nf).set_userdata(f);
}

void VM::bind__hash__(Type type, i64 (*f)(VM*, PyObject*)){
    PyObject* obj = _t(type);
    _all_types[type].m__hash__ = f;
    PyObject* nf = bind_method<0>(obj, "__hash__", [](VM* vm, ArgsView args){
        i64 ret = lambda_get_userdata<i64(*)(VM*, PyObject*)>(args.begin())(vm, args[0]);
        return VAR(ret);
    });
    PK_OBJ_GET(NativeFunc, nf).set_userdata(f);
}

void VM::bind__len__(Type type, i64 (*f)(VM*, PyObject*)){
    PyObject* obj = _t(type);
    _all_types[type].m__len__ = f;
    PyObject* nf = bind_method<0>(obj, "__len__", [](VM* vm, ArgsView args){
        i64 ret = lambda_get_userdata<i64(*)(VM*, PyObject*)>(args.begin())(vm, args[0]);
        return VAR(ret);
    });
    PK_OBJ_GET(NativeFunc, nf).set_userdata(f);
}

void Dict::_probe_0(PyObject *key, bool &ok, int &i) const{
    ok = false;
    i64 hash = vm->py_hash(key);
    i = hash & _mask;
    // std::cout << CAST(Str, vm->py_repr(key)) << " " << hash << " " << i << std::endl;
    for(int j=0; j<_capacity; j++) {
        if(_items[i].first != nullptr){
            if(vm->py_eq(_items[i].first, key)) { ok = true; break; }
        }else{
            if(_items[i].second == nullptr) break;
        }
        // https://github.com/python/cpython/blob/3.8/Objects/dictobject.c#L166
        i = ((5*i) + 1) & _mask;
        // std::cout << CAST(Str, vm->py_repr(key)) << " next: " << i << std::endl;
    }
}

void Dict::_probe_1(PyObject *key, bool &ok, int &i) const{
    ok = false;
    i = vm->py_hash(key) & _mask;
    while(_items[i].first != nullptr) {
        if(vm->py_eq(_items[i].first, key)) { ok = true; break; }
        // https://github.com/python/cpython/blob/3.8/Objects/dictobject.c#L166
        i = ((5*i) + 1) & _mask;
    }
}

void NativeFunc::check_size(VM* vm, ArgsView args) const{
    if(args.size() != argc && argc != -1) {
        vm->TypeError(_S("expected ", argc, " arguments, got ", args.size()));
    }
}

PyObject* NativeFunc::call(VM *vm, ArgsView args) const {
    return f(vm, args);
}

}   // namespace pkpy


// dummy header for ceval.cpp
namespace pkpy{

#define BINARY_F_COMPARE(func, op, rfunc)                           \
        PyObject* ret;                                              \
        const PyTypeInfo* _ti = _inst_type_info(_0);                \
        if(_ti->m##func){                               \
            ret = _ti->m##func(this, _0, _1);           \
        }else{                                          \
            PyObject* self;                                                     \
            PyObject* _2 = get_unbound_method(_0, func, &self, false);          \
            if(_2 != nullptr) ret = call_method(self, _2, _1);                  \
            else ret = NotImplemented;                                          \
        }                                                                       \
        if(ret == NotImplemented){                                              \
            PyObject* self;                                                     \
            PyObject* _2 = get_unbound_method(_1, rfunc, &self, false);         \
            if(_2 != nullptr) ret = call_method(self, _2, _0);                  \
            else BinaryOptError(op, _0, _1);                                    \
            if(ret == NotImplemented) BinaryOptError(op, _0, _1);               \
        }


bool VM::py_lt(PyObject* _0, PyObject* _1){
    BINARY_F_COMPARE(__lt__, "<", __gt__);
    return ret == True;
}

bool VM::py_le(PyObject* _0, PyObject* _1){
    BINARY_F_COMPARE(__le__, "<=", __ge__);
    return ret == True;
}

bool VM::py_gt(PyObject* _0, PyObject* _1){
    BINARY_F_COMPARE(__gt__, ">", __lt__);
    return ret == True;
}

bool VM::py_ge(PyObject* _0, PyObject* _1){
    BINARY_F_COMPARE(__ge__, ">=", __le__);
    return ret == True;
}

#undef BINARY_F_COMPARE

// static i64 _py_sint(PyObject* obj) noexcept {
//     return (i64)(PK_BITS(obj) >> 2);
// }

PyObject* VM::_run_top_frame(){
    FrameId frame = top_frame();
    const int base_id = frame.index;
    bool need_raise = false;

    while(true){
#if PK_DEBUG_EXTRA_CHECK
        if(frame.index < base_id) PK_FATAL_ERROR();
#endif
        try{
            if(need_raise){ need_raise = false; _raise(); }
/**********************************************************************/
/* NOTE: 
 * Be aware of accidental gc!
 * DO NOT leave any strong reference of PyObject* in the C stack
 */
{

#define CEVAL_STEP_CALLBACK() \
    if(_ceval_on_step) _ceval_on_step(this, frame.get(), byte); \
    if(_profiler) _profiler->_step(frame.get());

#define DISPATCH_OP_CALL() { frame = top_frame(); goto __NEXT_FRAME; }
__NEXT_FRAME:
    Bytecode byte = frame->next_bytecode();
    CEVAL_STEP_CALLBACK();

    // cache
    const CodeObject* co = frame->co;
    const auto& co_consts = co->consts;

#if PK_ENABLE_COMPUTED_GOTO
static void* OP_LABELS[] = {
    #define OPCODE(name) &&CASE_OP_##name,
    
#ifdef OPCODE

/**************************/
OPCODE(NO_OP)
/**************************/
OPCODE(POP_TOP)
OPCODE(DUP_TOP)
OPCODE(ROT_TWO)
OPCODE(ROT_THREE)
OPCODE(PRINT_EXPR)
/**************************/
OPCODE(LOAD_CONST)
OPCODE(LOAD_NONE)
OPCODE(LOAD_TRUE)
OPCODE(LOAD_FALSE)
OPCODE(LOAD_INTEGER)
OPCODE(LOAD_ELLIPSIS)
OPCODE(LOAD_FUNCTION)
OPCODE(LOAD_NULL)
/**************************/
OPCODE(LOAD_FAST)
OPCODE(LOAD_NAME)
OPCODE(LOAD_NONLOCAL)
OPCODE(LOAD_GLOBAL)
OPCODE(LOAD_ATTR)
OPCODE(LOAD_CLASS_GLOBAL)
OPCODE(LOAD_METHOD)
OPCODE(LOAD_SUBSCR)

OPCODE(STORE_FAST)
OPCODE(STORE_NAME)
OPCODE(STORE_GLOBAL)
OPCODE(STORE_ATTR)
OPCODE(STORE_SUBSCR)

OPCODE(DELETE_FAST)
OPCODE(DELETE_NAME)
OPCODE(DELETE_GLOBAL)
OPCODE(DELETE_ATTR)
OPCODE(DELETE_SUBSCR)
/**************************/
OPCODE(BUILD_LONG)
OPCODE(BUILD_IMAG)
OPCODE(BUILD_BYTES)
OPCODE(BUILD_TUPLE)
OPCODE(BUILD_LIST)
OPCODE(BUILD_DICT)
OPCODE(BUILD_SET)
OPCODE(BUILD_SLICE)
OPCODE(BUILD_STRING)
/**************************/
OPCODE(BUILD_TUPLE_UNPACK)
OPCODE(BUILD_LIST_UNPACK)
OPCODE(BUILD_DICT_UNPACK)
OPCODE(BUILD_SET_UNPACK)
/**************************/
OPCODE(BINARY_TRUEDIV)
OPCODE(BINARY_POW)

OPCODE(BINARY_ADD)
OPCODE(BINARY_SUB)
OPCODE(BINARY_MUL)
OPCODE(BINARY_FLOORDIV)
OPCODE(BINARY_MOD)

OPCODE(COMPARE_LT)
OPCODE(COMPARE_LE)
OPCODE(COMPARE_EQ)
OPCODE(COMPARE_NE)
OPCODE(COMPARE_GT)
OPCODE(COMPARE_GE)

OPCODE(BITWISE_LSHIFT)
OPCODE(BITWISE_RSHIFT)
OPCODE(BITWISE_AND)
OPCODE(BITWISE_OR)
OPCODE(BITWISE_XOR)

OPCODE(BINARY_MATMUL)

OPCODE(IS_OP)
OPCODE(CONTAINS_OP)
/**************************/
OPCODE(JUMP_ABSOLUTE)
OPCODE(JUMP_ABSOLUTE_TOP)
OPCODE(POP_JUMP_IF_FALSE)
OPCODE(POP_JUMP_IF_TRUE)
OPCODE(JUMP_IF_TRUE_OR_POP)
OPCODE(JUMP_IF_FALSE_OR_POP)
OPCODE(SHORTCUT_IF_FALSE_OR_POP)
OPCODE(LOOP_CONTINUE)
OPCODE(LOOP_BREAK)
OPCODE(GOTO)
/**************************/
OPCODE(FSTRING_EVAL)
OPCODE(REPR)
OPCODE(CALL)
OPCODE(CALL_TP)
OPCODE(RETURN_VALUE)
OPCODE(YIELD_VALUE)
/**************************/
OPCODE(LIST_APPEND)
OPCODE(DICT_ADD)
OPCODE(SET_ADD)
/**************************/
OPCODE(UNARY_NEGATIVE)
OPCODE(UNARY_NOT)
OPCODE(UNARY_STAR)
OPCODE(UNARY_INVERT)
/**************************/
OPCODE(GET_ITER)
OPCODE(FOR_ITER)
/**************************/
OPCODE(IMPORT_PATH)
OPCODE(POP_IMPORT_STAR)
/**************************/
OPCODE(UNPACK_SEQUENCE)
OPCODE(UNPACK_EX)
/**************************/
OPCODE(BEGIN_CLASS)
OPCODE(END_CLASS)
OPCODE(STORE_CLASS_ATTR)
OPCODE(BEGIN_CLASS_DECORATION)
OPCODE(END_CLASS_DECORATION)
OPCODE(ADD_CLASS_ANNOTATION)
/**************************/
OPCODE(WITH_ENTER)
OPCODE(WITH_EXIT)
/**************************/
OPCODE(EXCEPTION_MATCH)
OPCODE(RAISE)
OPCODE(RAISE_ASSERT)
OPCODE(RE_RAISE)
OPCODE(POP_EXCEPTION)
/**************************/
OPCODE(FORMAT_STRING)
/**************************/
OPCODE(INC_FAST)
OPCODE(DEC_FAST)
OPCODE(INC_GLOBAL)
OPCODE(DEC_GLOBAL)
#endif
#undef OPCODE
};

#define DISPATCH() { byte = frame->next_bytecode(); CEVAL_STEP_CALLBACK(); goto *OP_LABELS[byte.op];}
#define TARGET(op) CASE_OP_##op:
goto *OP_LABELS[byte.op];

#else
#define TARGET(op) case OP_##op:
#define DISPATCH() { byte = frame->next_bytecode(); CEVAL_STEP_CALLBACK(); goto __NEXT_STEP;}

__NEXT_STEP:;
#if PK_DEBUG_CEVAL_STEP
    _log_s_data();
#endif
    switch (byte.op)
    {
#endif
    TARGET(NO_OP) DISPATCH();
    /*****************************************/
    TARGET(POP_TOP) POP(); DISPATCH();
    TARGET(DUP_TOP) PUSH(TOP()); DISPATCH();
    TARGET(ROT_TWO) std::swap(TOP(), SECOND()); DISPATCH();
    TARGET(ROT_THREE){
        PyObject* _0 = TOP();
        TOP() = SECOND();
        SECOND() = THIRD();
        THIRD() = _0;
    } DISPATCH();
    TARGET(PRINT_EXPR){
        if(TOP() != None) stdout_write(CAST(Str&, py_repr(TOP())) + "\n");
        POP();
    } DISPATCH();
    /*****************************************/
    TARGET(LOAD_CONST)
        if(heap._should_auto_collect()) heap._auto_collect();
        PUSH(co_consts[byte.arg]);
        DISPATCH();
    TARGET(LOAD_NONE)       PUSH(None); DISPATCH();
    TARGET(LOAD_TRUE)       PUSH(True); DISPATCH();
    TARGET(LOAD_FALSE)      PUSH(False); DISPATCH();
    TARGET(LOAD_INTEGER)    PUSH(VAR((int16_t)byte.arg)); DISPATCH();
    TARGET(LOAD_ELLIPSIS)   PUSH(Ellipsis); DISPATCH();
    TARGET(LOAD_FUNCTION) {
        FuncDecl_ decl = co->func_decls[byte.arg];
        PyObject* obj;
        if(decl->nested){
            NameDict_ captured = frame->_locals.to_namedict();
            obj = VAR(Function(decl, frame->_module, nullptr, captured));
            captured->set(decl->code->name, obj);
        }else{
            obj = VAR(Function(decl, frame->_module, nullptr, nullptr));
        }
        PUSH(obj);
    } DISPATCH();
    TARGET(LOAD_NULL) PUSH(PY_NULL); DISPATCH();
    /*****************************************/
    TARGET(LOAD_FAST) {
        if(heap._should_auto_collect()) heap._auto_collect();
        PyObject* _0 = frame->_locals[byte.arg];
        if(_0 == PY_NULL) vm->UnboundLocalError(co->varnames[byte.arg]);
        PUSH(_0);
    } DISPATCH();
    TARGET(LOAD_NAME) {
        StrName _name(byte.arg);
        PyObject** slot = frame->_locals.try_get_name(_name);
        if(slot != nullptr) {
            if(*slot == PY_NULL) vm->UnboundLocalError(_name);
            PUSH(*slot);
            DISPATCH();
        }
        PyObject* _0 = frame->f_closure_try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = frame->f_globals().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = vm->builtins->attr().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        vm->NameError(_name);
    } DISPATCH();
    TARGET(LOAD_NONLOCAL) {
        if(heap._should_auto_collect()) heap._auto_collect();
        StrName _name(byte.arg);
        PyObject* _0 = frame->f_closure_try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = frame->f_globals().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = vm->builtins->attr().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        vm->NameError(_name);
    } DISPATCH();
    TARGET(LOAD_GLOBAL){
        if(heap._should_auto_collect()) heap._auto_collect();
        StrName _name(byte.arg);
        PyObject* _0 = frame->f_globals().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = vm->builtins->attr().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        vm->NameError(_name);
    } DISPATCH();
    TARGET(LOAD_ATTR){
        TOP() = getattr(TOP(), StrName(byte.arg));
    } DISPATCH();
    TARGET(LOAD_CLASS_GLOBAL){
        PK_ASSERT(_curr_class != nullptr);
        StrName _name(byte.arg);
        PyObject* _0 = getattr(_curr_class, _name, false);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        // load global if attribute not found
        _0 = frame->f_globals().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = vm->builtins->attr().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        vm->NameError(_name);
    } DISPATCH();
    TARGET(LOAD_METHOD){
        PyObject* _0;
        TOP() = get_unbound_method(TOP(), StrName(byte.arg), &_0, true, true);
        PUSH(_0);
    }DISPATCH();
    TARGET(LOAD_SUBSCR){
        PyObject* _1 = POPX();    // b
        PyObject* _0 = TOP();     // a
        auto _ti = _inst_type_info(_0);
        if(_ti->m__getitem__){
            TOP() = _ti->m__getitem__(this, _0, _1);
        }else{
            TOP() = call_method(_0, __getitem__, _1);
        }
    } DISPATCH();
    TARGET(STORE_FAST)
        frame->_locals[byte.arg] = POPX();
        DISPATCH();
    TARGET(STORE_NAME){
        StrName _name(byte.arg);
        PyObject* _0 = POPX();
        if(frame->_callable != nullptr){
            PyObject** slot = frame->_locals.try_get_name(_name);
            if(slot == nullptr) vm->UnboundLocalError(_name);
            *slot = _0;
        }else{
            frame->f_globals().set(_name, _0);
        }
    } DISPATCH();
    TARGET(STORE_GLOBAL)
        frame->f_globals().set(StrName(byte.arg), POPX());
        DISPATCH();
    TARGET(STORE_ATTR) {
        PyObject* _0 = TOP();         // a
        PyObject* _1 = SECOND();      // val
        setattr(_0, StrName(byte.arg), _1);
        STACK_SHRINK(2);
    } DISPATCH();
    TARGET(STORE_SUBSCR){
        PyObject* _2 = POPX();        // b
        PyObject* _1 = POPX();        // a
        PyObject* _0 = POPX();        // val
        auto _ti = _inst_type_info(_1);
        if(_ti->m__setitem__){
            _ti->m__setitem__(this, _1, _2, _0);
        }else{
            call_method(_1, __setitem__, _2, _0);
        }
    }DISPATCH();
    TARGET(DELETE_FAST){
        PyObject* _0 = frame->_locals[byte.arg];
        if(_0 == PY_NULL) vm->UnboundLocalError(co->varnames[byte.arg]);
        frame->_locals[byte.arg] = PY_NULL;
    }DISPATCH();
    TARGET(DELETE_NAME){
        StrName _name(byte.arg);
        if(frame->_callable != nullptr){
            PyObject** slot = frame->_locals.try_get_name(_name);
            if(slot == nullptr) vm->UnboundLocalError(_name);
            *slot = PY_NULL;
        }else{
            if(!frame->f_globals().del(_name)) vm->NameError(_name);
        }
    } DISPATCH();
    TARGET(DELETE_GLOBAL){
        StrName _name(byte.arg);
        if(!frame->f_globals().del(_name)) vm->NameError(_name);
    }DISPATCH();
    TARGET(DELETE_ATTR){
        PyObject* _0 = POPX();
        delattr(_0, StrName(byte.arg));
    } DISPATCH();
    TARGET(DELETE_SUBSCR){
        PyObject* _1 = POPX();
        PyObject* _0 = POPX();
        auto _ti = _inst_type_info(_0);
        if(_ti->m__delitem__){
            _ti->m__delitem__(this, _0, _1);
        }else{
            call_method(_0, __delitem__, _1);
        }
    }DISPATCH();
    /*****************************************/
    TARGET(BUILD_LONG) {
        PyObject* _0 = builtins->attr().try_get_likely_found(pk_id_long);
        if(_0 == nullptr) AttributeError(builtins, pk_id_long);
        TOP() = call(_0, TOP());
    } DISPATCH();
    TARGET(BUILD_IMAG) {
        PyObject* _0 = builtins->attr().try_get_likely_found(pk_id_complex);
        if(_0 == nullptr) AttributeError(builtins, pk_id_long);
        TOP() = call(_0, VAR(0), TOP());
    } DISPATCH();
    TARGET(BUILD_BYTES) {
        const Str& s = CAST(Str&, TOP());
        unsigned char* p = new unsigned char[s.size];
        memcpy(p, s.data, s.size);
        TOP() = VAR(Bytes(p, s.size));
    } DISPATCH();
    TARGET(BUILD_TUPLE){
        PyObject* _0 = VAR(STACK_VIEW(byte.arg).to_tuple());
        STACK_SHRINK(byte.arg);
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_LIST){
        PyObject* _0 = VAR(STACK_VIEW(byte.arg).to_list());
        STACK_SHRINK(byte.arg);
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_DICT){
        if(byte.arg == 0){
            PUSH(VAR(Dict(this)));
            DISPATCH();
        }
        PyObject* _0 = VAR(STACK_VIEW(byte.arg).to_list());
        _0 = call(_t(tp_dict), _0);
        STACK_SHRINK(byte.arg);
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_SET){
        PyObject* _0 = VAR(STACK_VIEW(byte.arg).to_list());
        _0 = call(builtins->attr(pk_id_set), _0);
        STACK_SHRINK(byte.arg);
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_SLICE){
        PyObject* _2 = POPX();    // step
        PyObject* _1 = POPX();    // stop
        PyObject* _0 = POPX();    // start
        PUSH(VAR(Slice(_0, _1, _2)));
    } DISPATCH();
    TARGET(BUILD_STRING) {
        SStream ss;
        ArgsView view = STACK_VIEW(byte.arg);
        for(PyObject* obj : view) ss << CAST(Str&, py_str(obj));
        STACK_SHRINK(byte.arg);
        PUSH(VAR(ss.str()));
    } DISPATCH();
    /*****************************************/
    TARGET(BUILD_TUPLE_UNPACK) {
        auto _lock = heap.gc_scope_lock();
        List list;
        _unpack_as_list(STACK_VIEW(byte.arg), list);
        STACK_SHRINK(byte.arg);
        PyObject* _0 = VAR(Tuple(std::move(list)));
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_LIST_UNPACK) {
        auto _lock = heap.gc_scope_lock();
        List list;
        _unpack_as_list(STACK_VIEW(byte.arg), list);
        STACK_SHRINK(byte.arg);
        PyObject* _0 = VAR(std::move(list));
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_DICT_UNPACK) {
        auto _lock = heap.gc_scope_lock();
        Dict dict(this);
        _unpack_as_dict(STACK_VIEW(byte.arg), dict);
        STACK_SHRINK(byte.arg);
        PyObject* _0 = VAR(std::move(dict));
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_SET_UNPACK) {
        auto _lock = heap.gc_scope_lock();
        List list;
        _unpack_as_list(STACK_VIEW(byte.arg), list);
        STACK_SHRINK(byte.arg);
        PyObject* _0 = VAR(std::move(list));
        _0 = call(builtins->attr(pk_id_set), _0);
        PUSH(_0);
    } DISPATCH();
    /*****************************************/
#define BINARY_OP_SPECIAL(func)                         \
        _1 = POPX();                                    \
        _0 = TOP();                                     \
        _ti = _inst_type_info(_0);                      \
        if(_ti->m##func){                               \
            TOP() = _ti->m##func(this, _0, _1);         \
        }else{                                          \
            PyObject* self;                                         \
            PyObject* _2 = get_unbound_method(_0, func, &self, false);        \
            if(_2 != nullptr) TOP() = call_method(self, _2, _1);    \
            else TOP() = NotImplemented;                            \
        }

#define BINARY_OP_RSPECIAL(op, func)                                \
        if(TOP() == NotImplemented){                                \
            PyObject* self;                                         \
            PyObject* _2 = get_unbound_method(_1, func, &self, false);        \
            if(_2 != nullptr) TOP() = call_method(self, _2, _0);    \
            else BinaryOptError(op, _0, _1);                        \
            if(TOP() == NotImplemented) BinaryOptError(op, _0, _1); \
        }

    TARGET(BINARY_TRUEDIV){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__truediv__);
        if(TOP() == NotImplemented) BinaryOptError("/", _0, _1);
    } DISPATCH();
    TARGET(BINARY_POW){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__pow__);
        if(TOP() == NotImplemented) BinaryOptError("**", _0, _1);
    } DISPATCH();
    TARGET(BINARY_ADD){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__add__);
        BINARY_OP_RSPECIAL("+", __radd__);
    } DISPATCH()
    TARGET(BINARY_SUB){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__sub__);
        BINARY_OP_RSPECIAL("-", __rsub__);
    } DISPATCH()
    TARGET(BINARY_MUL){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__mul__);
        BINARY_OP_RSPECIAL("*", __rmul__);
    } DISPATCH()
    TARGET(BINARY_FLOORDIV){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__floordiv__);
        if(TOP() == NotImplemented) BinaryOptError("//", _0, _1);
    } DISPATCH()
    TARGET(BINARY_MOD){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__mod__);
        if(TOP() == NotImplemented) BinaryOptError("%", _0, _1);
    } DISPATCH()
    TARGET(COMPARE_LT){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        TOP() = VAR(py_lt(_0, _1));
    } DISPATCH()
    TARGET(COMPARE_LE){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        TOP() = VAR(py_le(_0, _1));
    } DISPATCH()
    TARGET(COMPARE_EQ){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        TOP() = VAR(py_eq(_0, _1));
    } DISPATCH()
    TARGET(COMPARE_NE){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        TOP() = VAR(!py_eq(_0, _1));
    } DISPATCH()
    TARGET(COMPARE_GT){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        TOP() = VAR(py_gt(_0, _1));
    } DISPATCH()
    TARGET(COMPARE_GE){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        TOP() = VAR(py_ge(_0, _1));
    } DISPATCH()
    TARGET(BITWISE_LSHIFT){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__lshift__);
        if(TOP() == NotImplemented) BinaryOptError("<<", _0, _1);
    } DISPATCH()
    TARGET(BITWISE_RSHIFT){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__rshift__);
        if(TOP() == NotImplemented) BinaryOptError(">>", _0, _1);
    } DISPATCH()
    TARGET(BITWISE_AND){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__and__);
        if(TOP() == NotImplemented) BinaryOptError("&", _0, _1);
    } DISPATCH()
    TARGET(BITWISE_OR){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__or__);
        if(TOP() == NotImplemented) BinaryOptError("|", _0, _1);
    } DISPATCH()
    TARGET(BITWISE_XOR){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__xor__);
        if(TOP() == NotImplemented) BinaryOptError("^", _0, _1);
    } DISPATCH()
    TARGET(BINARY_MATMUL){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__matmul__);
        if(TOP() == NotImplemented) BinaryOptError("@", _0, _1);
    } DISPATCH();

#undef BINARY_OP_SPECIAL

    TARGET(IS_OP){
        PyObject* _1 = POPX();    // rhs
        PyObject* _0 = TOP();     // lhs
        TOP() = VAR(static_cast<bool>((_0==_1) ^ byte.arg));
    } DISPATCH();
    TARGET(CONTAINS_OP){
        // a in b -> b __contains__ a
        auto _ti = _inst_type_info(TOP());
        PyObject* _0;
        if(_ti->m__contains__){
            _0 = _ti->m__contains__(this, TOP(), SECOND());
        }else{
            _0 = call_method(TOP(), __contains__, SECOND());
        }
        POP();
        TOP() = VAR(static_cast<bool>((int)CAST(bool, _0) ^ byte.arg));
    } DISPATCH();
    /*****************************************/
    TARGET(JUMP_ABSOLUTE)
        frame->jump_abs(byte.arg);
        DISPATCH();
    TARGET(JUMP_ABSOLUTE_TOP)
        frame->jump_abs(_CAST(uint16_t, POPX()));
        DISPATCH();
    TARGET(POP_JUMP_IF_FALSE){
        if(!py_bool(TOP())) frame->jump_abs(byte.arg);
        POP();
    } DISPATCH();
    TARGET(POP_JUMP_IF_TRUE){
        if(py_bool(TOP())) frame->jump_abs(byte.arg);
        POP();
    } DISPATCH();
    TARGET(JUMP_IF_TRUE_OR_POP){
        if(py_bool(TOP())) frame->jump_abs(byte.arg);
        else POP();
    } DISPATCH();
    TARGET(JUMP_IF_FALSE_OR_POP){
        if(!py_bool(TOP())) frame->jump_abs(byte.arg);
        else POP();
    } DISPATCH();
    TARGET(SHORTCUT_IF_FALSE_OR_POP){
        if(!py_bool(TOP())){                // [b, False]
            STACK_SHRINK(2);                // []
            PUSH(vm->False);                // [False]
            frame->jump_abs(byte.arg);
        } else POP();                       // [b]
    } DISPATCH();
    TARGET(LOOP_CONTINUE)
        frame->jump_abs(byte.arg);
        DISPATCH();
    TARGET(LOOP_BREAK)
        frame->jump_abs_break(byte.arg);
        DISPATCH();
    TARGET(GOTO) {
        StrName _name(byte.arg);
        int index = co->labels.try_get_likely_found(_name);
        if(index < 0) RuntimeError(_S("label ", _name.escape(), " not found"));
        frame->jump_abs_break(index);
    } DISPATCH();
    /*****************************************/
    TARGET(FSTRING_EVAL){
        PyObject* _0 = co_consts[byte.arg];
        std::string_view string = CAST(Str&, _0).sv();
        auto it = _cached_codes.find(string);
        CodeObject_ code;
        if(it == _cached_codes.end()){
            code = vm->compile(string, "<eval>", EVAL_MODE, true);
            _cached_codes[string] = code;
        }else{
            code = it->second;
        }
        _0 = vm->_exec(code.get(), frame->_module, frame->_callable, frame->_locals);
        PUSH(_0);
    } DISPATCH();
    TARGET(REPR)
        TOP() = py_repr(TOP());
        DISPATCH();
    TARGET(CALL){
        PyObject* _0 = vectorcall(
            byte.arg & 0xFF,          // ARGC
            (byte.arg>>8) & 0xFF,     // KWARGC
            true
        );
        if(_0 == PY_OP_CALL) DISPATCH_OP_CALL();
        PUSH(_0);
    } DISPATCH();
    TARGET(CALL_TP){
        PyObject* _0;
        PyObject* _1;
        PyObject* _2;
        // [callable, <self>, args: tuple, kwargs: dict | NULL]
        if(byte.arg){
            _2 = POPX();
            _1 = POPX();
            for(PyObject* obj: _CAST(Tuple&, _1)) PUSH(obj);
            _CAST(Dict&, _2).apply([this](PyObject* k, PyObject* v){
                PUSH(VAR(StrName(CAST(Str&, k)).index));
                PUSH(v);
            });
            _0 = vectorcall(
                _CAST(Tuple&, _1).size(),   // ARGC
                _CAST(Dict&, _2).size(),    // KWARGC
                true
            );
        }else{
            // no **kwargs
            _1 = POPX();
            for(PyObject* obj: _CAST(Tuple&, _1)) PUSH(obj);
            _0 = vectorcall(
                _CAST(Tuple&, _1).size(),   // ARGC
                0,                          // KWARGC
                true
            );
        }
        if(_0 == PY_OP_CALL) DISPATCH_OP_CALL();
        PUSH(_0);
    } DISPATCH();
    TARGET(RETURN_VALUE){
        PyObject* _0 = byte.arg == BC_NOARG ? POPX() : None;
        _pop_frame();
        if(frame.index == base_id){       // [ frameBase<- ]
            return _0;
        }else{
            frame = top_frame();
            PUSH(_0);
            goto __NEXT_FRAME;
        }
    } DISPATCH();
    TARGET(YIELD_VALUE)
        return PY_OP_YIELD;
    /*****************************************/
    TARGET(LIST_APPEND){
        PyObject* _0 = POPX();
        CAST(List&, SECOND()).push_back(_0);
    } DISPATCH();
    TARGET(DICT_ADD) {
        PyObject* _0 = POPX();
        Tuple& t = CAST(Tuple&, _0);
        call_method(SECOND(), __setitem__, t[0], t[1]);
    } DISPATCH();
    TARGET(SET_ADD){
        PyObject* _0 = POPX();
        call_method(SECOND(), pk_id_add, _0);
    } DISPATCH();
    /*****************************************/
    TARGET(UNARY_NEGATIVE)
        TOP() = py_negate(TOP());
        DISPATCH();
    TARGET(UNARY_NOT){
        PyObject* _0 = TOP();
        if(_0==True) TOP()=False;
        else if(_0==False) TOP()=True;
        else TOP() = VAR(!py_bool(_0));
    } DISPATCH();
    TARGET(UNARY_STAR)
        TOP() = VAR(StarWrapper(byte.arg, TOP()));
        DISPATCH();
    TARGET(UNARY_INVERT){
        PyObject* _0;
        auto _ti = _inst_type_info(TOP());
        if(_ti->m__invert__) _0 = _ti->m__invert__(this, TOP());
        else _0 = call_method(TOP(), __invert__);
        TOP() = _0;
    } DISPATCH();
    /*****************************************/
    TARGET(GET_ITER)
        TOP() = py_iter(TOP());
        DISPATCH();
    TARGET(FOR_ITER){
        PyObject* _0 = py_next(TOP());
        if(_0 != StopIteration){
            PUSH(_0);
        }else{
            frame->jump_abs_break(byte.arg);
        }
    } DISPATCH();
    /*****************************************/
    TARGET(IMPORT_PATH){
        PyObject* _0 = co_consts[byte.arg];
        PUSH(py_import(CAST(Str&, _0)));
    } DISPATCH();
    TARGET(POP_IMPORT_STAR) {
        PyObject* _0 = POPX();        // pop the module
        PyObject* _1 = _0->attr().try_get(__all__);
        StrName _name;
        if(_1 != nullptr){
            for(PyObject* key: CAST(List&, _1)){
                _name = StrName::get(CAST(Str&, key).sv());
                PyObject* value = _0->attr().try_get_likely_found(_name);
                if(value == nullptr){
                    ImportError(_S("cannot import name ", _name.escape()));
                }else{
                    frame->f_globals().set(_name, value);
                }
            }
        }else{
            for(auto& [name, value]: _0->attr().items()){
                std::string_view s = name.sv();
                if(s.empty() || s[0] == '_') continue;
                frame->f_globals().set(name, value);
            }
        }
    } DISPATCH();
    /*****************************************/
    TARGET(UNPACK_SEQUENCE){
        auto _lock = heap.gc_scope_lock();  // lock the gc via RAII!!
        PyObject* _0 = py_iter(POPX());
        for(int i=0; i<byte.arg; i++){
            PyObject* _1 = py_next(_0);
            if(_1 == StopIteration) ValueError("not enough values to unpack");
            PUSH(_1);
        }
        if(py_next(_0) != StopIteration) ValueError("too many values to unpack");
    } DISPATCH();
    TARGET(UNPACK_EX) {
        auto _lock = heap.gc_scope_lock();  // lock the gc via RAII!!
        PyObject* _0 = py_iter(POPX());
        PyObject* _1;
        for(int i=0; i<byte.arg; i++){
            _1 = py_next(_0);
            if(_1 == StopIteration) ValueError("not enough values to unpack");
            PUSH(_1);
        }
        List extras;
        while(true){
            _1 = py_next(_0);
            if(_1 == StopIteration) break;
            extras.push_back(_1);
        }
        PUSH(VAR(extras));
    } DISPATCH();
    /*****************************************/
    TARGET(BEGIN_CLASS){
        StrName _name(byte.arg);
        PyObject* _0 = POPX();   // super
        if(_0 == None) _0 = _t(tp_object);
        check_non_tagged_type(_0, tp_type);
        _curr_class = new_type_object(frame->_module, _name, PK_OBJ_GET(Type, _0));
    } DISPATCH();
    TARGET(END_CLASS) {
        PK_ASSERT(_curr_class != nullptr);
        StrName _name(byte.arg);
        frame->_module->attr().set(_name, _curr_class);
        _curr_class = nullptr;
    } DISPATCH();
    TARGET(STORE_CLASS_ATTR){
        PK_ASSERT(_curr_class != nullptr);
        StrName _name(byte.arg);
        PyObject* _0 = POPX();
        if(is_non_tagged_type(_0, tp_function)){
            PK_OBJ_GET(Function, _0)._class = _curr_class;
        }
        _curr_class->attr().set(_name, _0);
    } DISPATCH();
    TARGET(BEGIN_CLASS_DECORATION){
        PUSH(_curr_class);
    } DISPATCH();
    TARGET(END_CLASS_DECORATION){
        _curr_class = POPX();
    } DISPATCH();
    TARGET(ADD_CLASS_ANNOTATION) {
        PK_ASSERT(_curr_class != nullptr);
        StrName _name(byte.arg);
        Type type = PK_OBJ_GET(Type, _curr_class);
        _all_types[type].annotated_fields.push_back(_name);
    } DISPATCH();
    /*****************************************/
    TARGET(WITH_ENTER)
        PUSH(call_method(TOP(), __enter__));
        DISPATCH();
    TARGET(WITH_EXIT)
        call_method(TOP(), __exit__);
        POP();
        DISPATCH();
    /*****************************************/
    TARGET(EXCEPTION_MATCH) {
        PyObject* assumed_type = POPX();
        check_non_tagged_type(assumed_type, tp_type);
        PyObject* e_obj = TOP();
        bool ok = isinstance(e_obj, PK_OBJ_GET(Type, assumed_type));
        PUSH(VAR(ok));
    } DISPATCH();
    TARGET(RAISE) {
        if(is_non_tagged_type(TOP(), tp_type)){
            TOP() = call(TOP());
        }
        if(!isinstance(TOP(), tp_exception)){
            _builtin_error("TypeError", "exceptions must derive from Exception");
        }
        _error(POPX());
    } DISPATCH();
    TARGET(RAISE_ASSERT)
        if(byte.arg){
            PyObject* _0 = py_str(POPX());
            _builtin_error("AssertionError", CAST(Str, _0));
        }else{
            _builtin_error("AssertionError");
        }
        DISPATCH();
    TARGET(RE_RAISE) _raise(true); DISPATCH();
    TARGET(POP_EXCEPTION) _last_exception = POPX(); DISPATCH();
    /*****************************************/
    TARGET(FORMAT_STRING) {
        PyObject* _0 = POPX();
        const Str& spec = CAST(Str&, co_consts[byte.arg]);
        PUSH(_format_string(spec, _0));
    } DISPATCH();
    /*****************************************/
    TARGET(INC_FAST){
        PyObject** p = &frame->_locals[byte.arg];
        if(*p == PY_NULL) vm->NameError(co->varnames[byte.arg]);
        *p = VAR(CAST(i64, *p) + 1);
    } DISPATCH();
    TARGET(DEC_FAST){
        PyObject** p = &frame->_locals[byte.arg];
        if(*p == PY_NULL) vm->NameError(co->varnames[byte.arg]);
        *p = VAR(CAST(i64, *p) - 1);
    } DISPATCH();
    TARGET(INC_GLOBAL){
        StrName _name(byte.arg);
        PyObject** p = frame->f_globals().try_get_2_likely_found(_name);
        if(p == nullptr) vm->NameError(_name);
        *p = VAR(CAST(i64, *p) + 1);
    } DISPATCH();
    TARGET(DEC_GLOBAL){
        StrName _name(byte.arg);
        PyObject** p = frame->f_globals().try_get_2_likely_found(_name);
        if(p == nullptr) vm->NameError(_name);
        *p = VAR(CAST(i64, *p) - 1);
    } DISPATCH();

#if !PK_ENABLE_COMPUTED_GOTO
        static_assert(OP_DEC_GLOBAL == 112);
        case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: case 123: case 124: case 125: case 126: case 127: case 128: case 129: case 130: case 131: case 132: case 133: case 134: case 135: case 136: case 137: case 138: case 139: case 140: case 141: case 142: case 143: case 144: case 145: case 146: case 147: case 148: case 149: case 150: case 151: case 152: case 153: case 154: case 155: case 156: case 157: case 158: case 159: case 160: case 161: case 162: case 163: case 164: case 165: case 166: case 167: case 168: case 169: case 170: case 171: case 172: case 173: case 174: case 175: case 176: case 177: case 178: case 179: case 180: case 181: case 182: case 183: case 184: case 185: case 186: case 187: case 188: case 189: case 190: case 191: case 192: case 193: case 194: case 195: case 196: case 197: case 198: case 199: case 200: case 201: case 202: case 203: case 204: case 205: case 206: case 207: case 208: case 209: case 210: case 211: case 212: case 213: case 214: case 215: case 216: case 217: case 218: case 219: case 220: case 221: case 222: case 223: case 224: case 225: case 226: case 227: case 228: case 229: case 230: case 231: case 232: case 233: case 234: case 235: case 236: case 237: case 238: case 239: case 240: case 241: case 242: case 243: case 244: case 245: case 246: case 247: case 248: case 249: case 250: case 251: case 252: case 253: case 254: case 255: PK_UNREACHABLE() break;
    }
#endif
}

#undef DISPATCH
#undef TARGET
#undef DISPATCH_OP_CALL
#undef CEVAL_STEP_CALLBACK
/**********************************************************************/
            PK_UNREACHABLE()
        }catch(HandledException){
            continue;
        }catch(UnhandledException){
            PyObject* e_obj = POPX();
            Exception& _e = PK_OBJ_GET(Exception, e_obj);
            _pop_frame();
            if(callstack.empty()){
#if PK_DEBUG_FULL_EXCEPTION
                std::cerr << _e.summary() << std::endl;
#endif
                throw _e;
            }
            frame = top_frame();
            PUSH(e_obj);
            if(frame.index < base_id) throw ToBeRaisedException();
            need_raise = true;
        }catch(ToBeRaisedException){
            need_raise = true;
        }
    }
}

#undef TOP
#undef SECOND
#undef THIRD
#undef PEEK
#undef STACK_SHRINK
#undef PUSH
#undef POP
#undef POPX
#undef STACK_VIEW

#undef DISPATCH
#undef TARGET
#undef DISPATCH_OP_CALL

} // namespace pkpy



namespace pkpy{

typedef uint8_t TokenIndex;

constexpr const char* kTokens[] = {
    "is not", "not in", "yield from",
    "@eof", "@eol", "@sof",
    "@id", "@num", "@str", "@fstr", "@long", "@bytes", "@imag",
    "@indent", "@dedent",
    /*****************************************/
    "+", "+=", "-", "-=",   // (INPLACE_OP - 1) can get '=' removed
    "*", "*=", "/", "/=", "//", "//=", "%", "%=",
    "&", "&=", "|", "|=", "^", "^=", 
    "<<", "<<=", ">>", ">>=",
    /*****************************************/
    ".", ",", ":", ";", "#", "(", ")", "[", "]", "{", "}",
    "**", "=", ">", "<", "..", "...", "->", "@", "==", "!=", ">=", "<=",
    "++", "--", "~",
    /** KW_BEGIN **/
    "class", "import", "as", "def", "lambda", "pass", "del", "from", "with", "yield",
    "None", "in", "is", "and", "or", "not", "True", "False", "global", "try", "except", "finally",
    "while", "for", "if", "elif", "else", "break", "continue", "return", "assert", "raise"
};

using TokenValue = std::variant<std::monostate, i64, f64, Str>;
const TokenIndex kTokenCount = sizeof(kTokens) / sizeof(kTokens[0]);

constexpr TokenIndex TK(const char token[]) {
    for(int k=0; k<kTokenCount; k++){
        const char* i = kTokens[k];
        const char* j = token;
        while(*i && *j && *i == *j) { i++; j++;}
        if(*i == *j) return k;
    }
    return 255;
}

#define TK_STR(t) kTokens[t]
const std::map<std::string_view, TokenIndex> kTokenKwMap = [](){
    std::map<std::string_view, TokenIndex> map;
    for(int k=TK("class"); k<kTokenCount; k++) map[kTokens[k]] = k;
    return map;
}();

struct Token{
  TokenIndex type;
  const char* start;
  int length;
  int line;
  int brackets_level;
  TokenValue value;

  Str str() const { return Str(start, length);}
  std::string_view sv() const { return std::string_view(start, length);}

  // Str info() const {
  //   SStream ss;
  //   ss << line << ": " << TK_STR(type) << " '" << (
  //       sv()=="\n" ? "\\n" : sv()
  //   ) << "'";
  //   return ss.str();
  // }
};

// https://docs.python.org/3/reference/expressions.html#operator-precedence
enum Precedence {
  PREC_LOWEST,
  PREC_LAMBDA,        // lambda
  PREC_TERNARY,       // ?:
  PREC_LOGICAL_OR,    // or
  PREC_LOGICAL_AND,   // and
  PREC_LOGICAL_NOT,   // not
  /* https://docs.python.org/3/reference/expressions.html#comparisons
   * Unlike C, all comparison operations in Python have the same priority,
   * which is lower than that of any arithmetic, shifting or bitwise operation.
   * Also unlike C, expressions like a < b < c have the interpretation that is conventional in mathematics.
   */
  PREC_COMPARISION,   // < > <= >= != ==, in / is / is not / not in
  PREC_BITWISE_OR,    // |
  PREC_BITWISE_XOR,   // ^
  PREC_BITWISE_AND,   // &
  PREC_BITWISE_SHIFT, // << >>
  PREC_TERM,          // + -
  PREC_FACTOR,        // * / % // @
  PREC_UNARY,         // - not ~
  PREC_EXPONENT,      // **
  PREC_PRIMARY,       // f() x[] a.b 1:2
  PREC_HIGHEST,
};

enum StringType { NORMAL_STRING, RAW_STRING, F_STRING, NORMAL_BYTES };

struct Lexer {
    VM* vm;
    std::shared_ptr<SourceData> src;
    const char* token_start;
    const char* curr_char;
    int current_line = 1;
    std::vector<Token> nexts;
    stack<int> indents;
    int brackets_level = 0;
    bool used = false;

    char peekchar() const{ return *curr_char; }
    bool match_n_chars(int n, char c0);
    bool match_string(const char* s);
    int eat_spaces();

    bool eat_indentation();
    char eatchar();
    char eatchar_include_newline();
    int eat_name();
    void skip_line_comment();
    bool matchchar(char c);
    void add_token(TokenIndex type, TokenValue value={});
    void add_token_2(char c, TokenIndex one, TokenIndex two);
    Str eat_string_until(char quote, bool raw);
    void eat_string(char quote, StringType type);

    void eat_number();
    bool lex_one_token();

    /***** Error Reporter *****/
    void throw_err(StrName type, Str msg);
    void throw_err(StrName type, Str msg, int lineno, const char* cursor);
    void SyntaxError(Str msg){ throw_err("SyntaxError", msg); }
    void SyntaxError(){ throw_err("SyntaxError", "invalid syntax"); }
    void IndentationError(Str msg){ throw_err("IndentationError", msg); }
    Lexer(VM* vm, std::shared_ptr<SourceData> src);
    std::vector<Token> run();
};

bool parse_int(std::string_view text, i64* out, int base);

} // namespace pkpy

namespace pkpy{


const uint32_t kLoRangeA[] = {170,186,443,448,660,1488,1519,1568,1601,1646,1649,1749,1774,1786,1791,1808,1810,1869,1969,1994,2048,2112,2144,2208,2230,2308,2365,2384,2392,2418,2437,2447,2451,2474,2482,2486,2493,2510,2524,2527,2544,2556,2565,2575,2579,2602,2610,2613,2616,2649,2654,2674,2693,2703,2707,2730,2738,2741,2749,2768,2784,2809,2821,2831,2835,2858,2866,2869,2877,2908,2911,2929,2947,2949,2958,2962,2969,2972,2974,2979,2984,2990,3024,3077,3086,3090,3114,3133,3160,3168,3200,3205,3214,3218,3242,3253,3261,3294,3296,3313,3333,3342,3346,3389,3406,3412,3423,3450,3461,3482,3507,3517,3520,3585,3634,3648,3713,3716,3718,3724,3749,3751,3762,3773,3776,3804,3840,3904,3913,3976,4096,4159,4176,4186,4193,4197,4206,4213,4238,4352,4682,4688,4696,4698,4704,4746,4752,4786,4792,4800,4802,4808,4824,4882,4888,4992,5121,5743,5761,5792,5873,5888,5902,5920,5952,5984,5998,6016,6108,6176,6212,6272,6279,6314,6320,6400,6480,6512,6528,6576,6656,6688,6917,6981,7043,7086,7098,7168,7245,7258,7401,7406,7413,7418,8501,11568,11648,11680,11688,11696,11704,11712,11720,11728,11736,12294,12348,12353,12447,12449,12543,12549,12593,12704,12784,13312,19968,40960,40982,42192,42240,42512,42538,42606,42656,42895,42999,43003,43011,43015,43020,43072,43138,43250,43259,43261,43274,43312,43360,43396,43488,43495,43514,43520,43584,43588,43616,43633,43642,43646,43697,43701,43705,43712,43714,43739,43744,43762,43777,43785,43793,43808,43816,43968,44032,55216,55243,63744,64112,64285,64287,64298,64312,64318,64320,64323,64326,64467,64848,64914,65008,65136,65142,65382,65393,65440,65474,65482,65490,65498,65536,65549,65576,65596,65599,65616,65664,66176,66208,66304,66349,66370,66384,66432,66464,66504,66640,66816,66864,67072,67392,67424,67584,67592,67594,67639,67644,67647,67680,67712,67808,67828,67840,67872,67968,68030,68096,68112,68117,68121,68192,68224,68288,68297,68352,68416,68448,68480,68608,68864,69376,69415,69424,69600,69635,69763,69840,69891,69956,69968,70006,70019,70081,70106,70108,70144,70163,70272,70280,70282,70287,70303,70320,70405,70415,70419,70442,70450,70453,70461,70480,70493,70656,70727,70751,70784,70852,70855,71040,71128,71168,71236,71296,71352,71424,71680,71935,72096,72106,72161,72163,72192,72203,72250,72272,72284,72349,72384,72704,72714,72768,72818,72960,72968,72971,73030,73056,73063,73066,73112,73440,73728,74880,77824,82944,92160,92736,92880,92928,93027,93053,93952,94032,94208,100352,110592,110928,110948,110960,113664,113776,113792,113808,123136,123214,123584,124928,126464,126469,126497,126500,126503,126505,126516,126521,126523,126530,126535,126537,126539,126541,126545,126548,126551,126553,126555,126557,126559,126561,126564,126567,126572,126580,126585,126590,126592,126603,126625,126629,126635,131072,173824,177984,178208,183984,194560};
const uint32_t kLoRangeB[] = {170,186,443,451,660,1514,1522,1599,1610,1647,1747,1749,1775,1788,1791,1808,1839,1957,1969,2026,2069,2136,2154,2228,2237,2361,2365,2384,2401,2432,2444,2448,2472,2480,2482,2489,2493,2510,2525,2529,2545,2556,2570,2576,2600,2608,2611,2614,2617,2652,2654,2676,2701,2705,2728,2736,2739,2745,2749,2768,2785,2809,2828,2832,2856,2864,2867,2873,2877,2909,2913,2929,2947,2954,2960,2965,2970,2972,2975,2980,2986,3001,3024,3084,3088,3112,3129,3133,3162,3169,3200,3212,3216,3240,3251,3257,3261,3294,3297,3314,3340,3344,3386,3389,3406,3414,3425,3455,3478,3505,3515,3517,3526,3632,3635,3653,3714,3716,3722,3747,3749,3760,3763,3773,3780,3807,3840,3911,3948,3980,4138,4159,4181,4189,4193,4198,4208,4225,4238,4680,4685,4694,4696,4701,4744,4749,4784,4789,4798,4800,4805,4822,4880,4885,4954,5007,5740,5759,5786,5866,5880,5900,5905,5937,5969,5996,6000,6067,6108,6210,6264,6276,6312,6314,6389,6430,6509,6516,6571,6601,6678,6740,6963,6987,7072,7087,7141,7203,7247,7287,7404,7411,7414,7418,8504,11623,11670,11686,11694,11702,11710,11718,11726,11734,11742,12294,12348,12438,12447,12538,12543,12591,12686,12730,12799,19893,40943,40980,42124,42231,42507,42527,42539,42606,42725,42895,42999,43009,43013,43018,43042,43123,43187,43255,43259,43262,43301,43334,43388,43442,43492,43503,43518,43560,43586,43595,43631,43638,43642,43695,43697,43702,43709,43712,43714,43740,43754,43762,43782,43790,43798,43814,43822,44002,55203,55238,55291,64109,64217,64285,64296,64310,64316,64318,64321,64324,64433,64829,64911,64967,65019,65140,65276,65391,65437,65470,65479,65487,65495,65500,65547,65574,65594,65597,65613,65629,65786,66204,66256,66335,66368,66377,66421,66461,66499,66511,66717,66855,66915,67382,67413,67431,67589,67592,67637,67640,67644,67669,67702,67742,67826,67829,67861,67897,68023,68031,68096,68115,68119,68149,68220,68252,68295,68324,68405,68437,68466,68497,68680,68899,69404,69415,69445,69622,69687,69807,69864,69926,69956,70002,70006,70066,70084,70106,70108,70161,70187,70278,70280,70285,70301,70312,70366,70412,70416,70440,70448,70451,70457,70461,70480,70497,70708,70730,70751,70831,70853,70855,71086,71131,71215,71236,71338,71352,71450,71723,71935,72103,72144,72161,72163,72192,72242,72250,72272,72329,72349,72440,72712,72750,72768,72847,72966,72969,73008,73030,73061,73064,73097,73112,73458,74649,75075,78894,83526,92728,92766,92909,92975,93047,93071,94026,94032,100343,101106,110878,110930,110951,111355,113770,113788,113800,113817,123180,123214,123627,125124,126467,126495,126498,126500,126503,126514,126519,126521,126523,126530,126535,126537,126539,126543,126546,126548,126551,126553,126555,126557,126559,126562,126564,126570,126578,126583,126588,126590,126601,126619,126627,126633,126651,173782,177972,178205,183969,191456,195101};

std::set<char> kValidChars = {
    '0','1','2','3','4','5','6','7','8','9',
    // a-f
    'a','b','c','d','e','f',
    // A-Z
    'A','B','C','D','E','F',
    // other valid chars
    '.', 'L', 'x', 'b', 'o', 'j'
};

static bool is_unicode_Lo_char(uint32_t c) {
    // open a hole for carrot
    if(c == U'🥕') return true;
    auto index = std::lower_bound(kLoRangeA, kLoRangeA + 476, c) - kLoRangeA;
    if(c == kLoRangeA[index]) return true;
    index -= 1;
    if(index < 0) return false;
    return c >= kLoRangeA[index] && c <= kLoRangeB[index];
}

    bool Lexer::match_n_chars(int n, char c0){
        const char* c = curr_char;
        for(int i=0; i<n; i++){
            if(*c == '\0') return false;
            if(*c != c0) return false;
            c++;
        }
        for(int i=0; i<n; i++) eatchar_include_newline();
        return true;
    }

    bool Lexer::match_string(const char* s){
        int s_len = strlen(s);
        bool ok = strncmp(curr_char, s, s_len) == 0;
        if(ok) for(int i=0; i<s_len; i++) eatchar_include_newline();
        return ok;
    }

    int Lexer::eat_spaces(){
        int count = 0;
        while (true) {
            switch (peekchar()) {
                case ' ' : count+=1; break;
                case '\t': count+=4; break;
                default: return count;
            }
            eatchar();
        }
    }

    bool Lexer::eat_indentation(){
        if(brackets_level > 0) return true;
        int spaces = eat_spaces();
        if(peekchar() == '#') skip_line_comment();
        if(peekchar() == '\0' || peekchar() == '\n') return true;
        // https://docs.python.org/3/reference/lexical_analysis.html#indentation
        if(spaces > indents.top()){
            indents.push(spaces);
            nexts.push_back(Token{TK("@indent"), token_start, 0, current_line, brackets_level});
        } else if(spaces < indents.top()){
            while(spaces < indents.top()){
                indents.pop();
                nexts.push_back(Token{TK("@dedent"), token_start, 0, current_line, brackets_level});
            }
            if(spaces != indents.top()){
                return false;
            }
        }
        return true;
    }

    char Lexer::eatchar() {
        char c = peekchar();
        if(c == '\n') throw std::runtime_error("eatchar() cannot consume a newline");
        curr_char++;
        return c;
    }

    char Lexer::eatchar_include_newline() {
        char c = peekchar();
        curr_char++;
        if (c == '\n'){
            current_line++;
            src->line_starts.push_back(curr_char);
        }
        return c;
    }

    int Lexer::eat_name() {
        curr_char--;
        while(true){
            unsigned char c = peekchar();
            int u8bytes = utf8len(c, true);
            if(u8bytes == 0) return 1;
            if(u8bytes == 1){
                if(isalpha(c) || c=='_' || isdigit(c)) {
                    curr_char++;
                    continue;
                }else{
                    break;
                }
            }
            // handle multibyte char
            std::string u8str(curr_char, u8bytes);
            if(u8str.size() != u8bytes) return 2;
            uint32_t value = 0;
            for(int k=0; k < u8bytes; k++){
                uint8_t b = u8str[k];
                if(k==0){
                    if(u8bytes == 2) value = (b & 0b00011111) << 6;
                    else if(u8bytes == 3) value = (b & 0b00001111) << 12;
                    else if(u8bytes == 4) value = (b & 0b00000111) << 18;
                }else{
                    value |= (b & 0b00111111) << (6*(u8bytes-k-1));
                }
            }
            if(is_unicode_Lo_char(value)) curr_char += u8bytes;
            else break;
        }

        int length = (int)(curr_char - token_start);
        if(length == 0) return 3;
        std::string_view name(token_start, length);

        if(src->mode == JSON_MODE){
            if(name == "true"){
                add_token(TK("True"));
            } else if(name == "false"){
                add_token(TK("False"));
            } else if(name == "null"){
                add_token(TK("None"));
            } else {
                return 4;
            }
            return 0;
        }

        if(kTokenKwMap.count(name)){
            add_token(kTokenKwMap.at(name));
        } else {
            add_token(TK("@id"));
        }
        return 0;
    }

    void Lexer::skip_line_comment() {
        char c;
        while ((c = peekchar()) != '\0') {
            if (c == '\n') return;
            eatchar();
        }
    }
    
    bool Lexer::matchchar(char c) {
        if (peekchar() != c) return false;
        eatchar_include_newline();
        return true;
    }

    void Lexer::add_token(TokenIndex type, TokenValue value) {
        switch(type){
            case TK("{"): case TK("["): case TK("("): brackets_level++; break;
            case TK(")"): case TK("]"): case TK("}"): brackets_level--; break;
        }
        auto token = Token{
            type,
            token_start,
            (int)(curr_char - token_start),
            current_line - ((type == TK("@eol")) ? 1 : 0),
            brackets_level,
            value
        };
        // handle "not in", "is not", "yield from"
        if(!nexts.empty()){
            auto& back = nexts.back();
            if(back.type == TK("not") && type == TK("in")){
                back.type = TK("not in");
                return;
            }
            if(back.type == TK("is") && type == TK("not")){
                back.type = TK("is not");
                return;
            }
            if(back.type == TK("yield") && type == TK("from")){
                back.type = TK("yield from");
                return;
            }
            nexts.push_back(token);
        }
    }

    void Lexer::add_token_2(char c, TokenIndex one, TokenIndex two) {
        if (matchchar(c)) add_token(two);
        else add_token(one);
    }

    Str Lexer::eat_string_until(char quote, bool raw) {
        bool quote3 = match_n_chars(2, quote);
        std::vector<char> buff;
        while (true) {
            char c = eatchar_include_newline();
            if (c == quote){
                if(quote3 && !match_n_chars(2, quote)){
                    buff.push_back(c);
                    continue;
                }
                break;
            }
            if (c == '\0'){
                if(quote3 && src->mode == REPL_MODE){
                    throw NeedMoreLines(false);
                }
                SyntaxError("EOL while scanning string literal");
            }
            if (c == '\n'){
                if(!quote3) SyntaxError("EOL while scanning string literal");
                else{
                    buff.push_back(c);
                    continue;
                }
            }
            if (!raw && c == '\\') {
                switch (eatchar_include_newline()) {
                    case '"':  buff.push_back('"');  break;
                    case '\'': buff.push_back('\''); break;
                    case '\\': buff.push_back('\\'); break;
                    case 'n':  buff.push_back('\n'); break;
                    case 'r':  buff.push_back('\r'); break;
                    case 't':  buff.push_back('\t'); break;
                    case 'b':  buff.push_back('\b'); break;
                    case 'x': {
                        char hex[3] = {eatchar(), eatchar(), '\0'};
                        size_t parsed;
                        char code;
                        try{
                            code = (char)std::stoi(hex, &parsed, 16);
                        }catch(...){
                            SyntaxError("invalid hex char");
                        }
                        if (parsed != 2) SyntaxError("invalid hex char");
                        buff.push_back(code);
                    } break;
                    default: SyntaxError("invalid escape char");
                }
            } else {
                buff.push_back(c);
            }
        }
        return Str(buff.data(), buff.size());
    }

    void Lexer::eat_string(char quote, StringType type) {
        Str s = eat_string_until(quote, type == RAW_STRING);
        if(type == F_STRING){
            add_token(TK("@fstr"), s);
            return;
        }
        if(type == NORMAL_BYTES){
            add_token(TK("@bytes"), s);
            return;
        }
        add_token(TK("@str"), s);
    }

    void Lexer::eat_number() {
        const char* i = token_start;
        while(kValidChars.count(*i)) i++;
        std::string_view text(token_start, i - token_start);
        this->curr_char = i;

        if(text[0] != '.'){
            // try long
            if(i[-1] == 'L'){
                add_token(TK("@long"));
                return;
            }
            // try integer
            i64 int_out;
            if(parse_int(text, &int_out, -1)){
                add_token(TK("@num"), int_out);
                return;
            }
        }

        // try float
        double float_out;
        char* p_end;
        try{
            float_out = std::strtod(text.data(), &p_end);
        }catch(...){
            SyntaxError("invalid number literal");
        }
        
        if(p_end == text.data() + text.size()){
            add_token(TK("@num"), (f64)float_out);
            return;
        }

        if(i[-1] == 'j' && p_end == text.data() + text.size() - 1){
            add_token(TK("@imag"), (f64)float_out);
            return;
        }

        SyntaxError("invalid number literal");
    }

    bool Lexer::lex_one_token() {
        while (peekchar() != '\0') {
            token_start = curr_char;
            char c = eatchar_include_newline();
            switch (c) {
                case '\'': case '"': eat_string(c, NORMAL_STRING); return true;
                case '#': skip_line_comment(); break;
                case '~': add_token(TK("~")); return true;
                case '{': add_token(TK("{")); return true;
                case '}': add_token(TK("}")); return true;
                case ',': add_token(TK(",")); return true;
                case ':': add_token(TK(":")); return true;
                case ';': add_token(TK(";")); return true;
                case '(': add_token(TK("(")); return true;
                case ')': add_token(TK(")")); return true;
                case '[': add_token(TK("[")); return true;
                case ']': add_token(TK("]")); return true;
                case '@': add_token(TK("@")); return true;
                case '\\': {
                    // line continuation character
                    char c = eatchar_include_newline();
                    if (c != '\n'){
                        if(src->mode == REPL_MODE && c == '\0') throw NeedMoreLines(false);
                        SyntaxError("expected newline after line continuation character");
                    }
                    eat_spaces();
                    return true;
                }
                case '%': add_token_2('=', TK("%"), TK("%=")); return true;
                case '&': add_token_2('=', TK("&"), TK("&=")); return true;
                case '|': add_token_2('=', TK("|"), TK("|=")); return true;
                case '^': add_token_2('=', TK("^"), TK("^=")); return true;
                case '.': {
                    if(matchchar('.')) {
                        if(matchchar('.')) {
                            add_token(TK("..."));
                        } else {
                            add_token(TK(".."));
                        }
                    } else {
                        char next_char = peekchar();
                        if(next_char >= '0' && next_char <= '9'){
                            eat_number();
                        }else{
                            add_token(TK("."));
                        }
                    }
                    return true;
                }
                case '=': add_token_2('=', TK("="), TK("==")); return true;
                case '+':
                    if(matchchar('+')){
                        add_token(TK("++"));
                    }else{
                        add_token_2('=', TK("+"), TK("+="));
                    }
                    return true;
                case '>': {
                    if(matchchar('=')) add_token(TK(">="));
                    else if(matchchar('>')) add_token_2('=', TK(">>"), TK(">>="));
                    else add_token(TK(">"));
                    return true;
                }
                case '<': {
                    if(matchchar('=')) add_token(TK("<="));
                    else if(matchchar('<')) add_token_2('=', TK("<<"), TK("<<="));
                    else add_token(TK("<"));
                    return true;
                }
                case '-': {
                    if(matchchar('-')){
                        add_token(TK("--"));
                    }else{
                        if(matchchar('=')) add_token(TK("-="));
                        else if(matchchar('>')) add_token(TK("->"));
                        else add_token(TK("-"));
                    }
                    return true;
                }
                case '!':
                    if(matchchar('=')) add_token(TK("!="));
                    else SyntaxError("expected '=' after '!'");
                    break;
                case '*':
                    if (matchchar('*')) {
                        add_token(TK("**"));  // '**'
                    } else {
                        add_token_2('=', TK("*"), TK("*="));
                    }
                    return true;
                case '/':
                    if(matchchar('/')) {
                        add_token_2('=', TK("//"), TK("//="));
                    } else {
                        add_token_2('=', TK("/"), TK("/="));
                    }
                    return true;
                case ' ': case '\t': eat_spaces(); break;
                case '\n': {
                    add_token(TK("@eol"));
                    if(!eat_indentation()) IndentationError("unindent does not match any outer indentation level");
                    return true;
                }
                default: {
                    if(c == 'f'){
                        if(matchchar('\'')) {eat_string('\'', F_STRING); return true;}
                        if(matchchar('"')) {eat_string('"', F_STRING); return true;}
                    }else if(c == 'r'){
                        if(matchchar('\'')) {eat_string('\'', RAW_STRING); return true;}
                        if(matchchar('"')) {eat_string('"', RAW_STRING); return true;}
                    }else if(c == 'b'){
                        if(matchchar('\'')) {eat_string('\'', NORMAL_BYTES); return true;}
                        if(matchchar('"')) {eat_string('"', NORMAL_BYTES); return true;}
                    }
                    if (c >= '0' && c <= '9') {
                        eat_number();
                        return true;
                    }
                    switch (eat_name())
                    {
                        case 0: break;
                        case 1: SyntaxError("invalid char: " + std::string(1, c)); break;
                        case 2: SyntaxError("invalid utf8 sequence: " + std::string(1, c)); break;
                        case 3: SyntaxError("@id contains invalid char"); break;
                        case 4: SyntaxError("invalid JSON token"); break;
                        default: PK_FATAL_ERROR();
                    }
                    return true;
                }
            }
        }

        token_start = curr_char;
        while(indents.size() > 1){
            indents.pop();
            add_token(TK("@dedent"));
            return true;
        }
        add_token(TK("@eof"));
        return false;
    }

    void Lexer::throw_err(StrName type, Str msg){
        int lineno = current_line;
        const char* cursor = curr_char;
        if(peekchar() == '\n'){
            lineno--;
            cursor--;
        }
        throw_err(type, msg, lineno, cursor);
    }

    Lexer::Lexer(VM* vm, std::shared_ptr<SourceData> src) : vm(vm), src(src) {
        this->token_start = src->source.c_str();
        this->curr_char = src->source.c_str();
        this->nexts.push_back(Token{TK("@sof"), token_start, 0, current_line, brackets_level});
        this->indents.push(0);
    }

    std::vector<Token> Lexer::run() {
        PK_ASSERT(!used)
        used = true;
        while (lex_one_token());
        return std::move(nexts);
    }

bool parse_int(std::string_view text, i64* out, int base){
  *out = 0;

  const auto f_startswith_2 = [](std::string_view t, const char* prefix) -> bool{
    if(t.length() < 2) return false;
    return t[0] == prefix[0] && t[1] == prefix[1];
  };

  if(base == -1){
    if(f_startswith_2(text, "0b")) base = 2;
    else if(f_startswith_2(text, "0o")) base = 8;
    else if(f_startswith_2(text, "0x")) base = 16;
    else base = 10;
  }

  if(base == 10){
    // 10-base  12334
    if(text.length() == 0) return false;
    for(char c : text){
      if(c >= '0' && c <= '9'){
        *out = (*out * 10) + (c - '0');
        if(*out < 0) return false;      // overflow
      }else{
        return false;
      }
    }
    return true;
  }else if(base == 2){
    // 2-base   0b101010
    if(f_startswith_2(text, "0b")) text.remove_prefix(2);
    if(text.length() == 0) return false;
    for(char c : text){
      if(c == '0' || c == '1'){
        *out = (*out << 1) | (c - '0');
        if(*out < 0) return false;      // overflow
      }else{
        return false;
      }
    }
    return true;
  }else if(base == 8){
    // 8-base   0o123
    if(f_startswith_2(text, "0o")) text.remove_prefix(2);
    if(text.length() == 0) return false;
    for(char c : text){
      if(c >= '0' && c <= '7'){
        *out = (*out << 3) | (c - '0');
        if(*out < 0) return false;      // overflow
      }else{
        return false;
      }
    }
    return true;
  }else if(base == 16){
    // 16-base  0x123
    if(f_startswith_2(text, "0x")) text.remove_prefix(2);
    if(text.length() == 0) return false;
    for(char c : text){
      if(c >= '0' && c <= '9'){
        *out = (*out << 4) | (c - '0');
        if(*out < 0) return false;      // overflow
      }else if(c >= 'a' && c <= 'f'){
        *out = (*out << 4) | (c - 'a' + 10);
        if(*out < 0) return false;      // overflow
      }else if(c >= 'A' && c <= 'F'){
        *out = (*out << 4) | (c - 'A' + 10);
        if(*out < 0) return false;      // overflow
      }else{
        return false;
      }
    }
    return true;
  }
  return false;
}

}   // namespace pkpy



namespace pkpy{

struct CodeEmitContext;
struct Expr;
typedef std::unique_ptr<Expr> Expr_;

struct Expr{
    int line = 0;
    virtual ~Expr() = default;
    virtual void emit_(CodeEmitContext* ctx) = 0;
    virtual bool is_literal() const { return false; }
    virtual bool is_json_object() const { return false; }
    virtual bool is_attrib() const { return false; }
    virtual bool is_compare() const { return false; }
    virtual int star_level() const { return 0; }
    virtual bool is_tuple() const { return false; }
    virtual bool is_name() const { return false; }
    bool is_starred() const { return star_level() > 0; }

    // for OP_DELETE_XXX
    [[nodiscard]] virtual bool emit_del(CodeEmitContext* ctx) {
        PK_UNUSED(ctx);
        return false;
    }

    // for OP_STORE_XXX
    [[nodiscard]] virtual bool emit_store(CodeEmitContext* ctx) {
        PK_UNUSED(ctx);
        return false;
    }
};

struct CodeEmitContext{
    VM* vm;
    FuncDecl_ func;     // optional
    CodeObject_ co;     // 1 CodeEmitContext <=> 1 CodeObject_
    // some bugs on MSVC (error C2280) when using std::vector<Expr_>
    // so we use stack_no_copy instead
    stack_no_copy<Expr_> s_expr;
    int level;
    std::set<Str> global_names;
    CodeEmitContext(VM* vm, CodeObject_ co, int level): vm(vm), co(co), level(level) {}

    int curr_block_i = 0;
    bool is_compiling_class = false;
    int base_stack_size = 0;

    std::map<void*, int> _co_consts_nonstring_dedup_map;
    std::map<std::string, int, std::less<>> _co_consts_string_dedup_map;

    int get_loop() const;
    CodeBlock* enter_block(CodeBlockType type);
    void exit_block();
    void emit_expr();   // clear the expression stack and generate bytecode
    int emit_(Opcode opcode, uint16_t arg, int line);
    void patch_jump(int index);
    bool add_label(StrName name);
    int add_varname(StrName name);
    int add_const(PyObject*);
    int add_const_string(std::string_view);
    int add_func_decl(FuncDecl_ decl);
    void emit_store_name(NameScope scope, StrName name, int line);
};

struct NameExpr: Expr{
    StrName name;
    NameScope scope;
    NameExpr(StrName name, NameScope scope): name(name), scope(scope) {}
    void emit_(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
    bool is_name() const override { return true; }
};

struct InvertExpr: Expr{
    Expr_ child;
    InvertExpr(Expr_&& child): child(std::move(child)) {}
    void emit_(CodeEmitContext* ctx) override;
};

struct StarredExpr: Expr{
    int level;
    Expr_ child;
    StarredExpr(int level, Expr_&& child): level(level), child(std::move(child)) {}
    int star_level() const override { return level; }
    void emit_(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
};

struct NotExpr: Expr{
    Expr_ child;
    NotExpr(Expr_&& child): child(std::move(child)) {}
    void emit_(CodeEmitContext* ctx) override;
};

struct AndExpr: Expr{
    Expr_ lhs;
    Expr_ rhs;
    void emit_(CodeEmitContext* ctx) override;
};

struct OrExpr: Expr{
    Expr_ lhs;
    Expr_ rhs;
    void emit_(CodeEmitContext* ctx) override;
};

// [None, True, False, ...]
struct Literal0Expr: Expr{
    TokenIndex token;
    Literal0Expr(TokenIndex token): token(token) {}
    bool is_json_object() const override { return true; }

    void emit_(CodeEmitContext* ctx) override;
};

struct LongExpr: Expr{
    Str s;
    LongExpr(const Str& s): s(s) {}
    void emit_(CodeEmitContext* ctx) override;
};

struct BytesExpr: Expr{
    Str s;
    BytesExpr(const Str& s): s(s) {}
    void emit_(CodeEmitContext* ctx) override;
};

struct ImagExpr: Expr{
    f64 value;
    ImagExpr(f64 value): value(value) {}
    void emit_(CodeEmitContext* ctx) override;
};

// @num, @str which needs to invoke OP_LOAD_CONST
struct LiteralExpr: Expr{
    TokenValue value;
    LiteralExpr(TokenValue value): value(value) {}
    void emit_(CodeEmitContext* ctx) override;
    bool is_literal() const override { return true; }
    bool is_json_object() const override { return true; }
};

struct NegatedExpr: Expr{
    Expr_ child;
    NegatedExpr(Expr_&& child): child(std::move(child)) {}
    void emit_(CodeEmitContext* ctx) override;
    bool is_json_object() const override { return child->is_literal(); }
};

struct SliceExpr: Expr{
    Expr_ start;
    Expr_ stop;
    Expr_ step;
    void emit_(CodeEmitContext* ctx) override;
};

struct DictItemExpr: Expr{
    Expr_ key;      // maybe nullptr if it is **kwargs
    Expr_ value;
    int star_level() const override { return value->star_level(); }
    void emit_(CodeEmitContext* ctx) override;
};

struct SequenceExpr: Expr{
    std::vector<Expr_> items;
    SequenceExpr(std::vector<Expr_>&& items): items(std::move(items)) {}
    virtual Opcode opcode() const = 0;

    void emit_(CodeEmitContext* ctx) override {
        for(auto& item: items) item->emit_(ctx);
        ctx->emit_(opcode(), items.size(), line);
    }
};

struct ListExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    Opcode opcode() const override {
        for(auto& e: items) if(e->is_starred()) return OP_BUILD_LIST_UNPACK;
        return OP_BUILD_LIST;
    }

    bool is_json_object() const override { return true; }
};

struct DictExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    Opcode opcode() const override {
        for(auto& e: items) if(e->is_starred()) return OP_BUILD_DICT_UNPACK;
        return OP_BUILD_DICT;
    }

    bool is_json_object() const override { return true; }
};

struct SetExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    Opcode opcode() const override {
        for(auto& e: items) if(e->is_starred()) return OP_BUILD_SET_UNPACK;
        return OP_BUILD_SET;
    }
};

struct TupleExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    bool is_tuple() const override { return true; }
    Opcode opcode() const override {
        for(auto& e: items) if(e->is_starred()) return OP_BUILD_TUPLE_UNPACK;
        return OP_BUILD_TUPLE;
    }

    bool emit_store(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
};

struct CompExpr: Expr{
    Expr_ expr;       // loop expr
    Expr_ vars;       // loop vars
    Expr_ iter;       // loop iter
    Expr_ cond;       // optional if condition

    virtual Opcode op0() = 0;
    virtual Opcode op1() = 0;

    void emit_(CodeEmitContext* ctx) override;
};

struct ListCompExpr: CompExpr{
    Opcode op0() override { return OP_BUILD_LIST; }
    Opcode op1() override { return OP_LIST_APPEND; }
};

struct DictCompExpr: CompExpr{
    Opcode op0() override { return OP_BUILD_DICT; }
    Opcode op1() override { return OP_DICT_ADD; }
};

struct SetCompExpr: CompExpr{
    Opcode op0() override { return OP_BUILD_SET; }
    Opcode op1() override { return OP_SET_ADD; }
};

struct LambdaExpr: Expr{
    FuncDecl_ decl;

    LambdaExpr(FuncDecl_ decl): decl(decl) {}

    void emit_(CodeEmitContext* ctx) override {
        int index = ctx->add_func_decl(decl);
        ctx->emit_(OP_LOAD_FUNCTION, index, line);
    }
};

struct FStringExpr: Expr{
    Str src;
    FStringExpr(const Str& src): src(src) {}
    void _load_simple_expr(CodeEmitContext* ctx, Str expr);
    void emit_(CodeEmitContext* ctx) override;
};

struct SubscrExpr: Expr{
    Expr_ a;
    Expr_ b;
    void emit_(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
};

struct AttribExpr: Expr{
    Expr_ a;
    StrName b;
    AttribExpr(Expr_ a, StrName b): a(std::move(a)), b(b) {}

    void emit_(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
    void emit_method(CodeEmitContext* ctx);
    bool is_attrib() const override { return true; }
};

struct CallExpr: Expr{
    Expr_ callable;
    std::vector<Expr_> args;
    // **a will be interpreted as a special keyword argument: {"**": a}
    std::vector<std::pair<Str, Expr_>> kwargs;
    void emit_(CodeEmitContext* ctx) override;
};

struct GroupedExpr: Expr{
    Expr_ a;
    GroupedExpr(Expr_&& a): a(std::move(a)) {}

    void emit_(CodeEmitContext* ctx) override{
        a->emit_(ctx);
    }

    bool emit_del(CodeEmitContext* ctx) override {
        return a->emit_del(ctx);
    }

    bool emit_store(CodeEmitContext* ctx) override {
        return a->emit_store(ctx);
    }
};

struct BinaryExpr: Expr{
    TokenIndex op;
    Expr_ lhs;
    Expr_ rhs;
    bool is_compare() const override;
    void _emit_compare(CodeEmitContext* ctx, std::vector<int>& jmps);
    void emit_(CodeEmitContext* ctx) override;
};


struct TernaryExpr: Expr{
    Expr_ cond;
    Expr_ true_expr;
    Expr_ false_expr;
    void emit_(CodeEmitContext* ctx) override;
};


} // namespace pkpy
namespace pkpy{

    inline bool is_imm_int(i64 v){
        return v >= INT16_MIN && v <= INT16_MAX;
    }

    inline bool is_identifier(std::string_view s){
        if(s.empty()) return false;
        if(!isalpha(s[0]) && s[0] != '_') return false;
        for(char c: s) if(!isalnum(c) && c != '_') return false;
        return true;
    }

    int CodeEmitContext::get_loop() const {
        int index = curr_block_i;
        while(index >= 0){
            if(co->blocks[index].type == CodeBlockType::FOR_LOOP) break;
            if(co->blocks[index].type == CodeBlockType::WHILE_LOOP) break;
            index = co->blocks[index].parent;
        }
        return index;
    }

    CodeBlock* CodeEmitContext::enter_block(CodeBlockType type){
        if(type==CodeBlockType::FOR_LOOP || type==CodeBlockType::CONTEXT_MANAGER) base_stack_size++;
        co->blocks.push_back(CodeBlock(
            type, curr_block_i, base_stack_size, (int)co->codes.size()
        ));
        curr_block_i = co->blocks.size()-1;
        return &co->blocks[curr_block_i];
    }

    void CodeEmitContext::exit_block(){
        auto curr_type = co->blocks[curr_block_i].type;
        if(curr_type == CodeBlockType::FOR_LOOP || curr_type==CodeBlockType::CONTEXT_MANAGER) base_stack_size--;
        co->blocks[curr_block_i].end = co->codes.size();
        curr_block_i = co->blocks[curr_block_i].parent;
        if(curr_block_i < 0) PK_FATAL_ERROR();

        if(curr_type == CodeBlockType::FOR_LOOP){
            // add a no op here to make block check work
            emit_(OP_NO_OP, BC_NOARG, BC_KEEPLINE);
        }
    }

    // clear the expression stack and generate bytecode
    void CodeEmitContext::emit_expr(){
        if(s_expr.size() != 1) throw std::runtime_error("s_expr.size() != 1");
        Expr_ expr = s_expr.popx();
        expr->emit_(this);
    }

    int CodeEmitContext::emit_(Opcode opcode, uint16_t arg, int line) {
        co->codes.push_back(Bytecode{(uint8_t)opcode, arg});
        co->iblocks.push_back(curr_block_i);
        co->lines.push_back(line);
        int i = co->codes.size() - 1;
        if(line==BC_KEEPLINE){
            if(i>=1) co->lines[i] = co->lines[i-1];
            else co->lines[i] = 1;
        }
        return i;
    }

    void CodeEmitContext::patch_jump(int index) {
        int target = co->codes.size();
        co->codes[index].arg = target;
    }

    bool CodeEmitContext::add_label(StrName name){
        if(co->labels.contains(name)) return false;
        co->labels.set(name, co->codes.size());
        return true;
    }

    int CodeEmitContext::add_varname(StrName name){
        // PK_MAX_CO_VARNAMES will be checked when pop_context(), not here
        int index = co->varnames_inv.try_get(name);
        if(index >= 0) return index;
        co->varnames.push_back(name);
        index = co->varnames.size() - 1;
        co->varnames_inv.set(name, index);
        return index;
    }

    int CodeEmitContext::add_const_string(std::string_view key){
        auto it = _co_consts_string_dedup_map.find(key);
        if(it != _co_consts_string_dedup_map.end()){
            return it->second;
        }else{
            co->consts.push_back(VAR(key));
            int index = co->consts.size() - 1;
            _co_consts_string_dedup_map[std::string(key)] = index;
            return index;
        }
    }

    int CodeEmitContext::add_const(PyObject* v){
        if(is_non_tagged_type(v, vm->tp_str)){
            // warning: should use add_const_string() instead
            return add_const_string(PK_OBJ_GET(Str, v).sv());
        }else{
            // non-string deduplication
            auto it = _co_consts_nonstring_dedup_map.find(v);
            if(it != _co_consts_nonstring_dedup_map.end()){
                return it->second;
            }else{
                co->consts.push_back(v);
                int index = co->consts.size() - 1;
                _co_consts_nonstring_dedup_map[v] = index;
                return index;
            }
        }
        PK_UNREACHABLE()
    }

    int CodeEmitContext::add_func_decl(FuncDecl_ decl){
        co->func_decls.push_back(decl);
        return co->func_decls.size() - 1;
    }

    void CodeEmitContext::emit_store_name(NameScope scope, StrName name, int line){
        switch(scope){
            case NAME_LOCAL:
                emit_(OP_STORE_FAST, add_varname(name), line);
                break;
            case NAME_GLOBAL:
                emit_(OP_STORE_GLOBAL, StrName(name).index, line);
                break;
            case NAME_GLOBAL_UNKNOWN:
                emit_(OP_STORE_NAME, StrName(name).index, line);
                break;
            default: PK_FATAL_ERROR(); break;
        }
    }


    void NameExpr::emit_(CodeEmitContext* ctx) {
        int index = ctx->co->varnames_inv.try_get(name);
        if(scope == NAME_LOCAL && index >= 0){
            ctx->emit_(OP_LOAD_FAST, index, line);
        }else{
            Opcode op = ctx->level <= 1 ? OP_LOAD_GLOBAL : OP_LOAD_NONLOCAL;
            if(ctx->is_compiling_class && scope == NAME_GLOBAL){
                // if we are compiling a class, we should use OP_LOAD_ATTR_GLOBAL instead of OP_LOAD_GLOBAL
                // this supports @property.setter
                op = OP_LOAD_CLASS_GLOBAL;
                // exec()/eval() won't work with OP_LOAD_ATTR_GLOBAL in class body
            }else{
                // we cannot determine the scope when calling exec()/eval()
                if(scope == NAME_GLOBAL_UNKNOWN) op = OP_LOAD_NAME;
            }
            ctx->emit_(op, StrName(name).index, line);
        }
    }

    bool NameExpr::emit_del(CodeEmitContext* ctx) {
        switch(scope){
            case NAME_LOCAL:
                ctx->emit_(OP_DELETE_FAST, ctx->add_varname(name), line);
                break;
            case NAME_GLOBAL:
                ctx->emit_(OP_DELETE_GLOBAL, StrName(name).index, line);
                break;
            case NAME_GLOBAL_UNKNOWN:
                ctx->emit_(OP_DELETE_NAME, StrName(name).index, line);
                break;
            default: PK_FATAL_ERROR(); break;
        }
        return true;
    }

    bool NameExpr::emit_store(CodeEmitContext* ctx) {
        if(ctx->is_compiling_class){
            ctx->emit_(OP_STORE_CLASS_ATTR, name.index, line);
            return true;
        }
        ctx->emit_store_name(scope, name, line);
        return true;
    }

    void InvertExpr::emit_(CodeEmitContext* ctx) {
        child->emit_(ctx);
        ctx->emit_(OP_UNARY_INVERT, BC_NOARG, line);
    }

    void StarredExpr::emit_(CodeEmitContext* ctx) {
        child->emit_(ctx);
        ctx->emit_(OP_UNARY_STAR, level, line);
    }

    bool StarredExpr::emit_store(CodeEmitContext* ctx) {
        if(level != 1) return false;
        // simply proxy to child
        return child->emit_store(ctx);
    }

    void NotExpr::emit_(CodeEmitContext* ctx) {
        child->emit_(ctx);
        ctx->emit_(OP_UNARY_NOT, BC_NOARG, line);
    }

    void AndExpr::emit_(CodeEmitContext* ctx) {
        lhs->emit_(ctx);
        int patch = ctx->emit_(OP_JUMP_IF_FALSE_OR_POP, BC_NOARG, line);
        rhs->emit_(ctx);
        ctx->patch_jump(patch);
    }

    void OrExpr::emit_(CodeEmitContext* ctx) {
        lhs->emit_(ctx);
        int patch = ctx->emit_(OP_JUMP_IF_TRUE_OR_POP, BC_NOARG, line);
        rhs->emit_(ctx);
        ctx->patch_jump(patch);
    }

    void Literal0Expr::emit_(CodeEmitContext* ctx){
        switch (token) {
            case TK("None"):    ctx->emit_(OP_LOAD_NONE, BC_NOARG, line); break;
            case TK("True"):    ctx->emit_(OP_LOAD_TRUE, BC_NOARG, line); break;
            case TK("False"):   ctx->emit_(OP_LOAD_FALSE, BC_NOARG, line); break;
            case TK("..."):     ctx->emit_(OP_LOAD_ELLIPSIS, BC_NOARG, line); break;
            default: PK_FATAL_ERROR();
        }
    }

    void LongExpr::emit_(CodeEmitContext* ctx) {
        ctx->emit_(OP_LOAD_CONST, ctx->add_const_string(s.sv()), line);
        ctx->emit_(OP_BUILD_LONG, BC_NOARG, line);
    }

    void ImagExpr::emit_(CodeEmitContext* ctx) {
        VM* vm = ctx->vm;
        ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR(value)), line);
        ctx->emit_(OP_BUILD_IMAG, BC_NOARG, line);
    }

    void BytesExpr::emit_(CodeEmitContext* ctx) {
        ctx->emit_(OP_LOAD_CONST, ctx->add_const_string(s.sv()), line);
        ctx->emit_(OP_BUILD_BYTES, BC_NOARG, line);
    }

    void LiteralExpr::emit_(CodeEmitContext* ctx) {
        VM* vm = ctx->vm;
        if(std::holds_alternative<i64>(value)){
            i64 _val = std::get<i64>(value);
            if(is_imm_int(_val)){
                ctx->emit_(OP_LOAD_INTEGER, (uint16_t)_val, line);
                return;
            }
            ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR(_val)), line);
            return;
        }
        if(std::holds_alternative<f64>(value)){
            f64 _val = std::get<f64>(value);
            ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR(_val)), line);
            return;
        }
        if(std::holds_alternative<Str>(value)){
            std::string_view key = std::get<Str>(value).sv();
            ctx->emit_(OP_LOAD_CONST, ctx->add_const_string(key), line);
            return;
        }
    }

    void NegatedExpr::emit_(CodeEmitContext* ctx){
        VM* vm = ctx->vm;
        // if child is a int of float, do constant folding
        if(child->is_literal()){
            LiteralExpr* lit = static_cast<LiteralExpr*>(child.get());
            if(std::holds_alternative<i64>(lit->value)){
                i64 _val = -std::get<i64>(lit->value);
                if(is_imm_int(_val)){
                    ctx->emit_(OP_LOAD_INTEGER, (uint16_t)_val, line);
                }else{
                    ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR(_val)), line);
                }
                return;
            }
            if(std::holds_alternative<f64>(lit->value)){
                f64 _val = -std::get<f64>(lit->value);
                ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR(_val)), line);
                return;
            }
        }
        child->emit_(ctx);
        ctx->emit_(OP_UNARY_NEGATIVE, BC_NOARG, line);
    }


    void SliceExpr::emit_(CodeEmitContext* ctx){
        if(start){
            start->emit_(ctx);
        }else{
            ctx->emit_(OP_LOAD_NONE, BC_NOARG, line);
        }

        if(stop){
            stop->emit_(ctx);
        }else{
            ctx->emit_(OP_LOAD_NONE, BC_NOARG, line);
        }

        if(step){
            step->emit_(ctx);
        }else{
            ctx->emit_(OP_LOAD_NONE, BC_NOARG, line);
        }

        ctx->emit_(OP_BUILD_SLICE, BC_NOARG, line);
    }

    void DictItemExpr::emit_(CodeEmitContext* ctx) {
        if(is_starred()){
            PK_ASSERT(key == nullptr);
            value->emit_(ctx);
        }else{
            value->emit_(ctx);
            key->emit_(ctx);     // reverse order
            ctx->emit_(OP_BUILD_TUPLE, 2, line);
        }
    }

    bool TupleExpr::emit_store(CodeEmitContext* ctx) {
        // TOS is an iterable
        // items may contain StarredExpr, we should check it
        int starred_i = -1;
        for(int i=0; i<items.size(); i++){
            if(!items[i]->is_starred()) continue;
            if(starred_i == -1) starred_i = i;
            else return false;  // multiple StarredExpr not allowed
        }

        if(starred_i == -1){
            Bytecode& prev = ctx->co->codes.back();
            if(prev.op == OP_BUILD_TUPLE && prev.arg == items.size()){
                // build tuple and unpack it is meaningless
                prev.op = OP_NO_OP;
                prev.arg = BC_NOARG;
            }else{
                ctx->emit_(OP_UNPACK_SEQUENCE, items.size(), line);
            }
        }else{
            // starred assignment target must be in a tuple
            if(items.size() == 1) return false;
            // starred assignment target must be the last one (differ from cpython)
            if(starred_i != items.size()-1) return false;
            // a,*b = [1,2,3]
            // stack is [1,2,3] -> [1,[2,3]]
            ctx->emit_(OP_UNPACK_EX, items.size()-1, line);
        }
        // do reverse emit
        for(int i=items.size()-1; i>=0; i--){
            bool ok = items[i]->emit_store(ctx);
            if(!ok) return false;
        }
        return true;
    }

    bool TupleExpr::emit_del(CodeEmitContext* ctx){
        for(auto& e: items){
            bool ok = e->emit_del(ctx);
            if(!ok) return false;
        }
        return true;
    }

    void CompExpr::emit_(CodeEmitContext* ctx){
        ctx->emit_(op0(), 0, line);
        iter->emit_(ctx);
        ctx->emit_(OP_GET_ITER, BC_NOARG, BC_KEEPLINE);
        ctx->enter_block(CodeBlockType::FOR_LOOP);
        ctx->emit_(OP_FOR_ITER, BC_NOARG, BC_KEEPLINE);
        bool ok = vars->emit_store(ctx);
        // this error occurs in `vars` instead of this line, but...nevermind
        PK_ASSERT(ok);  // TODO: raise a SyntaxError instead
        if(cond){
            cond->emit_(ctx);
            int patch = ctx->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, BC_KEEPLINE);
            expr->emit_(ctx);
            ctx->emit_(op1(), BC_NOARG, BC_KEEPLINE);
            ctx->patch_jump(patch);
        }else{
            expr->emit_(ctx);
            ctx->emit_(op1(), BC_NOARG, BC_KEEPLINE);
        }
        ctx->emit_(OP_LOOP_CONTINUE, ctx->get_loop(), BC_KEEPLINE);
        ctx->exit_block();
    }


    void FStringExpr::_load_simple_expr(CodeEmitContext* ctx, Str expr){
        bool repr = false;
        if(expr.size>=2 && expr.end()[-2]=='!'){
            switch(expr.end()[-1]){
                case 'r': repr = true; expr = expr.substr(0, expr.size-2); break;
                case 's': repr = false; expr = expr.substr(0, expr.size-2); break;
                default: break;     // nothing happens
            }
        }
        // name or name.name
        bool is_fastpath = false;
        if(is_identifier(expr.sv())){
            ctx->emit_(OP_LOAD_NAME, StrName(expr.sv()).index, line);
            is_fastpath = true;
        }else{
            int dot = expr.index(".");
            if(dot > 0){
                std::string_view a = expr.sv().substr(0, dot);
                std::string_view b = expr.sv().substr(dot+1);
                if(is_identifier(a) && is_identifier(b)){
                    ctx->emit_(OP_LOAD_NAME, StrName(a).index, line);
                    ctx->emit_(OP_LOAD_ATTR, StrName(b).index, line);
                    is_fastpath = true;
                }
            }
        }
        
        if(!is_fastpath){
            int index = ctx->add_const_string(expr.sv());
            ctx->emit_(OP_FSTRING_EVAL, index, line);
        }

        if(repr){
            ctx->emit_(OP_REPR, BC_NOARG, line);
        }
    }

    void FStringExpr::emit_(CodeEmitContext* ctx){
        int i = 0;              // left index
        int j = 0;              // right index
        int count = 0;          // how many string parts
        bool flag = false;      // true if we are in a expression

        const char* fmt_valid_chars = "0-=*#@!~" "<>^" ".fds" "0123456789";
        PK_LOCAL_STATIC const std::set<char> fmt_valid_char_set(fmt_valid_chars, fmt_valid_chars + strlen(fmt_valid_chars));

        while(j < src.size){
            if(flag){
                if(src[j] == '}'){
                    // add expression
                    Str expr = src.substr(i, j-i);
                    // BUG: ':' is not a format specifier in f"{stack[2:]}"
                    int conon = expr.index(":");
                    if(conon >= 0){
                        Str spec = expr.substr(conon+1);
                        // filter some invalid spec
                        bool ok = true;
                        for(char c: spec) if(!fmt_valid_char_set.count(c)){ ok = false; break; }
                        if(ok){
                            _load_simple_expr(ctx, expr.substr(0, conon));
                            ctx->emit_(OP_FORMAT_STRING, ctx->add_const_string(spec.sv()), line);
                        }else{
                            // ':' is not a spec indicator
                            _load_simple_expr(ctx, expr);
                        }
                    }else{
                        _load_simple_expr(ctx, expr);
                    }
                    flag = false;
                    count++;
                }
            }else{
                if(src[j] == '{'){
                    // look at next char
                    if(j+1 < src.size && src[j+1] == '{'){
                        // {{ -> {
                        j++;
                        ctx->emit_(OP_LOAD_CONST, ctx->add_const_string("{"), line);
                        count++;
                    }else{
                        // { -> }
                        flag = true;
                        i = j+1;
                    }
                }else if(src[j] == '}'){
                    // look at next char
                    if(j+1 < src.size && src[j+1] == '}'){
                        // }} -> }
                        j++;
                        ctx->emit_(OP_LOAD_CONST, ctx->add_const_string("}"), line);
                        count++;
                    }else{
                        // } -> error
                        // throw std::runtime_error("f-string: unexpected }");
                        // just ignore
                    }
                }else{
                    // literal
                    i = j;
                    while(j < src.size && src[j] != '{' && src[j] != '}') j++;
                    Str literal = src.substr(i, j-i);
                    ctx->emit_(OP_LOAD_CONST, ctx->add_const_string(literal.sv()), line);
                    count++;
                    continue;   // skip j++
                }
            }
            j++;
        }

        if(flag){
            // literal
            Str literal = src.substr(i, src.size-i);
            ctx->emit_(OP_LOAD_CONST, ctx->add_const_string(literal.sv()), line);
            count++;
        }

        ctx->emit_(OP_BUILD_STRING, count, line);
    }


    void SubscrExpr::emit_(CodeEmitContext* ctx){
        a->emit_(ctx);
        b->emit_(ctx);
        ctx->emit_(OP_LOAD_SUBSCR, BC_NOARG, line);
    }

    bool SubscrExpr::emit_del(CodeEmitContext* ctx){
        a->emit_(ctx);
        b->emit_(ctx);
        ctx->emit_(OP_DELETE_SUBSCR, BC_NOARG, line);
        return true;
    }

    bool SubscrExpr::emit_store(CodeEmitContext* ctx){
        a->emit_(ctx);
        b->emit_(ctx);
        ctx->emit_(OP_STORE_SUBSCR, BC_NOARG, line);
        return true;
    }

    void AttribExpr::emit_(CodeEmitContext* ctx){
        a->emit_(ctx);
        ctx->emit_(OP_LOAD_ATTR, b.index, line);
    }

    bool AttribExpr::emit_del(CodeEmitContext* ctx) {
        a->emit_(ctx);
        ctx->emit_(OP_DELETE_ATTR, b.index, line);
        return true;
    }

    bool AttribExpr::emit_store(CodeEmitContext* ctx){
        a->emit_(ctx);
        ctx->emit_(OP_STORE_ATTR, b.index, line);
        return true;
    }

    void AttribExpr::emit_method(CodeEmitContext* ctx) {
        a->emit_(ctx);
        ctx->emit_(OP_LOAD_METHOD, b.index, line);
    }

    void CallExpr::emit_(CodeEmitContext* ctx) {
        bool vargs = false;
        bool vkwargs = false;
        for(auto& arg: args) if(arg->is_starred()) vargs = true;
        for(auto& item: kwargs) if(item.second->is_starred()) vkwargs = true;

        // if callable is a AttrExpr, we should try to use `fast_call` instead of use `boundmethod` proxy
        if(callable->is_attrib()){
            auto p = static_cast<AttribExpr*>(callable.get());
            p->emit_method(ctx);    // OP_LOAD_METHOD
        }else{
            callable->emit_(ctx);
            ctx->emit_(OP_LOAD_NULL, BC_NOARG, BC_KEEPLINE);
        }

        if(vargs || vkwargs){
            for(auto& item: args) item->emit_(ctx);
            ctx->emit_(OP_BUILD_TUPLE_UNPACK, (uint16_t)args.size(), line);

            if(!kwargs.empty()){
                for(auto& item: kwargs){
                    if(item.second->is_starred()){
                        PK_ASSERT(item.second->star_level() == 2)
                        item.second->emit_(ctx);
                    }else{
                        // k=v
                        int index = ctx->add_const_string(item.first.sv());
                        ctx->emit_(OP_LOAD_CONST, index, line);
                        item.second->emit_(ctx);
                        ctx->emit_(OP_BUILD_TUPLE, 2, line);
                    }
                }
                ctx->emit_(OP_BUILD_DICT_UNPACK, (int)kwargs.size(), line);
                ctx->emit_(OP_CALL_TP, 1, line);
            }else{
                ctx->emit_(OP_CALL_TP, 0, line);
            }
        }else{
            // vectorcall protocal
            for(auto& item: args) item->emit_(ctx);
            for(auto& item: kwargs){
                uint16_t index = StrName(item.first.sv()).index;
                ctx->emit_(OP_LOAD_INTEGER, index, line);
                item.second->emit_(ctx);
            }
            int KWARGC = kwargs.size();
            int ARGC = args.size();
            ctx->emit_(OP_CALL, (KWARGC<<8)|ARGC, line);
        }
    }


    bool BinaryExpr::is_compare() const {
        switch(op){
            case TK("<"): case TK("<="): case TK("=="):
            case TK("!="): case TK(">"): case TK(">="): return true;
            default: return false;
        }
    }

    void BinaryExpr::_emit_compare(CodeEmitContext* ctx, std::vector<int>& jmps){
        if(lhs->is_compare()){
            static_cast<BinaryExpr*>(lhs.get())->_emit_compare(ctx, jmps);
        }else{
            lhs->emit_(ctx); // [a]
        }
        rhs->emit_(ctx); // [a, b]
        ctx->emit_(OP_DUP_TOP, BC_NOARG, line);      // [a, b, b]
        ctx->emit_(OP_ROT_THREE, BC_NOARG, line);    // [b, a, b]
        switch(op){
            case TK("<"):   ctx->emit_(OP_COMPARE_LT, BC_NOARG, line);  break;
            case TK("<="):  ctx->emit_(OP_COMPARE_LE, BC_NOARG, line);  break;
            case TK("=="):  ctx->emit_(OP_COMPARE_EQ, BC_NOARG, line);  break;
            case TK("!="):  ctx->emit_(OP_COMPARE_NE, BC_NOARG, line);  break;
            case TK(">"):   ctx->emit_(OP_COMPARE_GT, BC_NOARG, line);  break;
            case TK(">="):  ctx->emit_(OP_COMPARE_GE, BC_NOARG, line);  break;
            default: PK_UNREACHABLE()
        }
        // [b, RES]
        int index = ctx->emit_(OP_SHORTCUT_IF_FALSE_OR_POP, BC_NOARG, line);
        jmps.push_back(index);
    }

    void BinaryExpr::emit_(CodeEmitContext* ctx) {
        std::vector<int> jmps;
        if(is_compare() && lhs->is_compare()){
            // (a < b) < c
            static_cast<BinaryExpr*>(lhs.get())->_emit_compare(ctx, jmps);
            // [b, RES]
        }else{
            // (1 + 2) < c
            lhs->emit_(ctx);
        }

        rhs->emit_(ctx);
        switch (op) {
            case TK("+"):   ctx->emit_(OP_BINARY_ADD, BC_NOARG, line);  break;
            case TK("-"):   ctx->emit_(OP_BINARY_SUB, BC_NOARG, line);  break;
            case TK("*"):   ctx->emit_(OP_BINARY_MUL, BC_NOARG, line);  break;
            case TK("/"):   ctx->emit_(OP_BINARY_TRUEDIV, BC_NOARG, line);  break;
            case TK("//"):  ctx->emit_(OP_BINARY_FLOORDIV, BC_NOARG, line);  break;
            case TK("%"):   ctx->emit_(OP_BINARY_MOD, BC_NOARG, line);  break;
            case TK("**"):  ctx->emit_(OP_BINARY_POW, BC_NOARG, line);  break;

            case TK("<"):   ctx->emit_(OP_COMPARE_LT, BC_NOARG, line);  break;
            case TK("<="):  ctx->emit_(OP_COMPARE_LE, BC_NOARG, line);  break;
            case TK("=="):  ctx->emit_(OP_COMPARE_EQ, BC_NOARG, line);  break;
            case TK("!="):  ctx->emit_(OP_COMPARE_NE, BC_NOARG, line);  break;
            case TK(">"):   ctx->emit_(OP_COMPARE_GT, BC_NOARG, line);  break;
            case TK(">="):  ctx->emit_(OP_COMPARE_GE, BC_NOARG, line);  break;

            case TK("in"):      ctx->emit_(OP_CONTAINS_OP, 0, line);   break;
            case TK("not in"):  ctx->emit_(OP_CONTAINS_OP, 1, line);   break;
            case TK("is"):      ctx->emit_(OP_IS_OP, 0, line);         break;
            case TK("is not"):  ctx->emit_(OP_IS_OP, 1, line);         break;

            case TK("<<"):  ctx->emit_(OP_BITWISE_LSHIFT, BC_NOARG, line);  break;
            case TK(">>"):  ctx->emit_(OP_BITWISE_RSHIFT, BC_NOARG, line);  break;
            case TK("&"):   ctx->emit_(OP_BITWISE_AND, BC_NOARG, line);  break;
            case TK("|"):   ctx->emit_(OP_BITWISE_OR, BC_NOARG, line);  break;
            case TK("^"):   ctx->emit_(OP_BITWISE_XOR, BC_NOARG, line);  break;

            case TK("@"):   ctx->emit_(OP_BINARY_MATMUL, BC_NOARG, line);  break;
            default: PK_FATAL_ERROR();
        }

        for(int i: jmps) ctx->patch_jump(i);
    }

    void TernaryExpr::emit_(CodeEmitContext* ctx){
        cond->emit_(ctx);
        int patch = ctx->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, cond->line);
        true_expr->emit_(ctx);
        int patch_2 = ctx->emit_(OP_JUMP_ABSOLUTE, BC_NOARG, true_expr->line);
        ctx->patch_jump(patch);
        false_expr->emit_(ctx);
        ctx->patch_jump(patch_2);
    }

}   // namespace pkpy


namespace pkpy{

class Compiler;
typedef void (Compiler::*PrattCallback)();

struct PrattRule{
    PrattCallback prefix;
    PrattCallback infix;
    Precedence precedence;
};

class Compiler {
    PK_ALWAYS_PASS_BY_POINTER(Compiler)

    inline static PrattRule rules[kTokenCount];
    std::unique_ptr<Lexer> lexer;
    stack<CodeEmitContext> contexts;
    VM* vm;
    bool unknown_global_scope;     // for eval/exec() call
    bool used;
    // for parsing token stream
    int i = 0;
    std::vector<Token> tokens;

    const Token& prev() const{ return tokens.at(i-1); }
    const Token& curr() const{ return tokens.at(i); }
    const Token& next() const{ return tokens.at(i+1); }
    const Token& err() const{
        if(i >= tokens.size()) return prev();
        return curr();
    }
    void advance(int delta=1) { i += delta; }

    CodeEmitContext* ctx() { return &contexts.top(); }
    CompileMode mode() const{ return lexer->src->mode; }
    NameScope name_scope() const;
    CodeObject_ push_global_context();
    FuncDecl_ push_f_context(Str name);
    void pop_context();

    static void init_pratt_rules();

    bool match(TokenIndex expected);
    void consume(TokenIndex expected);
    bool match_newlines_repl();

    bool match_newlines(bool repl_throw=false);
    bool match_end_stmt();
    void consume_end_stmt();

    /*************************************************/
    void EXPR();
    void EXPR_TUPLE(bool allow_slice=false);
    Expr_ EXPR_VARS();  // special case for `for loop` and `comp`

    template <typename T, typename... Args>
    std::unique_ptr<T> make_expr(Args&&... args) {
        std::unique_ptr<T> expr = std::make_unique<T>(std::forward<Args>(args)...);
        expr->line = prev().line;
        return expr;
    }

    template<typename T>
    void _consume_comp(Expr_ expr){
        static_assert(std::is_base_of<CompExpr, T>::value);
        std::unique_ptr<CompExpr> ce = make_expr<T>();
        ce->expr = std::move(expr);
        ce->vars = EXPR_VARS();
        consume(TK("in"));
        parse_expression(PREC_TERNARY + 1);
        ce->iter = ctx()->s_expr.popx();
        match_newlines_repl();
        if(match(TK("if"))){
            parse_expression(PREC_TERNARY + 1);
            ce->cond = ctx()->s_expr.popx();
        }
        ctx()->s_expr.push(std::move(ce));
        match_newlines_repl();
    }

    void exprLiteral();
    void exprLong();
    void exprImag();
    void exprBytes();
    void exprFString();
    void exprLambda();
    void exprOr();
    void exprAnd();
    void exprTernary();
    void exprBinaryOp();
    void exprNot();
    void exprUnaryOp();
    void exprGroup();
    void exprList();
    void exprMap();
    void exprCall();
    void exprName();
    void exprAttrib();
    void exprSlice0();
    void exprSlice1();
    void exprSubscr();
    void exprLiteral0();

    void compile_block_body(void (Compiler::*callback)()=nullptr);
    void compile_normal_import();
    void compile_from_import();
    bool is_expression(bool allow_slice=false);
    void parse_expression(int precedence, bool allow_slice=false);
    void compile_if_stmt();
    void compile_while_loop();
    void compile_for_loop();
    void compile_try_except();
    void compile_decorated();

    bool try_compile_assignment();
    void compile_stmt();
    void consume_type_hints();
    void _add_decorators(const std::vector<Expr_>& decorators);
    void compile_class(const std::vector<Expr_>& decorators={});
    void _compile_f_args(FuncDecl_ decl, bool enable_type_hints);
    void compile_function(const std::vector<Expr_>& decorators={});

    PyObject* to_object(const TokenValue& value);
    PyObject* read_literal();

    void SyntaxError(Str msg){ lexer->throw_err("SyntaxError", msg, err().line, err().start); }
    void SyntaxError(){ lexer->throw_err("SyntaxError", "invalid syntax", err().line, err().start); }
    void IndentationError(Str msg){ lexer->throw_err("IndentationError", msg, err().line, err().start); }

public:
    Compiler(VM* vm, std::string_view source, const Str& filename, CompileMode mode, bool unknown_global_scope=false);
    CodeObject_ compile();
};

} // namespace pkpy
namespace pkpy{

    NameScope Compiler::name_scope() const {
        auto s = contexts.size()>1 ? NAME_LOCAL : NAME_GLOBAL;
        if(unknown_global_scope && s == NAME_GLOBAL) s = NAME_GLOBAL_UNKNOWN;
        return s;
    }

    CodeObject_ Compiler::push_global_context(){
        CodeObject_ co = std::make_shared<CodeObject>(lexer->src, lexer->src->filename);
        co->start_line = i==0 ? 1 : prev().line;
        contexts.push(CodeEmitContext(vm, co, contexts.size()));
        return co;
    }

    FuncDecl_ Compiler::push_f_context(Str name){
        FuncDecl_ decl = std::make_shared<FuncDecl>();
        decl->code = std::make_shared<CodeObject>(lexer->src, name);
        decl->code->start_line = i==0 ? 1 : prev().line;
        decl->nested = name_scope() == NAME_LOCAL;
        contexts.push(CodeEmitContext(vm, decl->code, contexts.size()));
        contexts.top().func = decl;
        return decl;
    }

    void Compiler::pop_context(){
        if(!ctx()->s_expr.empty()){
            throw std::runtime_error("!ctx()->s_expr.empty()");
        }
        // add a `return None` in the end as a guard
        // previously, we only do this if the last opcode is not a return
        // however, this is buggy...since there may be a jump to the end (out of bound) even if the last opcode is a return
        ctx()->emit_(OP_RETURN_VALUE, 1, BC_KEEPLINE);
        // find the last valid token
        int j = i-1;
        while(tokens[j].type == TK("@eol") || tokens[j].type == TK("@dedent") || tokens[j].type == TK("@eof")) j--;
        ctx()->co->end_line = tokens[j].line;

        // some check here
        std::vector<Bytecode>& codes = ctx()->co->codes;
        if(ctx()->co->varnames.size() > PK_MAX_CO_VARNAMES){
            SyntaxError("maximum number of local variables exceeded");
        }
        if(ctx()->co->consts.size() > 65535){
            SyntaxError("maximum number of constants exceeded");
        }
        if(codes.size() > 65535 && ctx()->co->src->mode != JSON_MODE){
            // json mode does not contain jump instructions, so it is safe to ignore this check
            SyntaxError("maximum number of opcodes exceeded");
        }
        // pre-compute LOOP_BREAK and LOOP_CONTINUE and FOR_ITER
        for(int i=0; i<codes.size(); i++){
            Bytecode& bc = codes[i];
            if(bc.op == OP_LOOP_CONTINUE){
                bc.arg = ctx()->co->blocks[bc.arg].start;
            }else if(bc.op == OP_LOOP_BREAK){
                bc.arg = ctx()->co->blocks[bc.arg].get_break_end();
            }else if(bc.op == OP_FOR_ITER){
                bc.arg = ctx()->co->_get_block_codei(i).end;
            }
        }
        // pre-compute func->is_simple
        FuncDecl_ func = contexts.top().func;
        if(func){
            func->is_simple = true;
            if(func->code->is_generator) func->is_simple = false;
            if(func->kwargs.size() > 0) func->is_simple = false;
            if(func->starred_arg >= 0) func->is_simple = false;
            if(func->starred_kwarg >= 0) func->is_simple = false;
        }
        contexts.pop();
    }

    void Compiler::init_pratt_rules(){
        PK_LOCAL_STATIC unsigned int count = 0;
        if(count > 0) return;
        count += 1;
// http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
#define PK_METHOD(name) &Compiler::name
#define PK_NO_INFIX nullptr, PREC_LOWEST
        for(TokenIndex i=0; i<kTokenCount; i++) rules[i] = { nullptr, PK_NO_INFIX };
        rules[TK(".")] =        { nullptr,                  PK_METHOD(exprAttrib),         PREC_PRIMARY };
        rules[TK("(")] =        { PK_METHOD(exprGroup),     PK_METHOD(exprCall),           PREC_PRIMARY };
        rules[TK("[")] =        { PK_METHOD(exprList),      PK_METHOD(exprSubscr),         PREC_PRIMARY };
        rules[TK("{")] =        { PK_METHOD(exprMap),       PK_NO_INFIX };
        rules[TK("%")] =        { nullptr,                  PK_METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("+")] =        { nullptr,                  PK_METHOD(exprBinaryOp),       PREC_TERM };
        rules[TK("-")] =        { PK_METHOD(exprUnaryOp),   PK_METHOD(exprBinaryOp),       PREC_TERM };
        rules[TK("*")] =        { PK_METHOD(exprUnaryOp),   PK_METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("~")] =        { PK_METHOD(exprUnaryOp),   nullptr,                    PREC_UNARY };
        rules[TK("/")] =        { nullptr,                  PK_METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("//")] =       { nullptr,                  PK_METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("**")] =       { PK_METHOD(exprUnaryOp),   PK_METHOD(exprBinaryOp),       PREC_EXPONENT };
        rules[TK(">")] =        { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("<")] =        { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("==")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("!=")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK(">=")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("<=")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("in")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("is")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("<<")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK(">>")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK("&")] =        { nullptr,               PK_METHOD(exprBinaryOp),       PREC_BITWISE_AND };
        rules[TK("|")] =        { nullptr,               PK_METHOD(exprBinaryOp),       PREC_BITWISE_OR };
        rules[TK("^")] =        { nullptr,               PK_METHOD(exprBinaryOp),       PREC_BITWISE_XOR };
        rules[TK("@")] =        { nullptr,               PK_METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("if")] =       { nullptr,               PK_METHOD(exprTernary),        PREC_TERNARY };
        rules[TK("not in")] =   { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("is not")] =   { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("and") ] =     { nullptr,               PK_METHOD(exprAnd),            PREC_LOGICAL_AND };
        rules[TK("or")] =       { nullptr,               PK_METHOD(exprOr),             PREC_LOGICAL_OR };
        rules[TK("not")] =      { PK_METHOD(exprNot),       nullptr,                    PREC_LOGICAL_NOT };
        rules[TK("True")] =     { PK_METHOD(exprLiteral0),  PK_NO_INFIX };
        rules[TK("False")] =    { PK_METHOD(exprLiteral0),  PK_NO_INFIX };
        rules[TK("None")] =     { PK_METHOD(exprLiteral0),  PK_NO_INFIX };
        rules[TK("...")] =      { PK_METHOD(exprLiteral0),  PK_NO_INFIX };
        rules[TK("lambda")] =   { PK_METHOD(exprLambda),    PK_NO_INFIX };
        rules[TK("@id")] =      { PK_METHOD(exprName),      PK_NO_INFIX };
        rules[TK("@num")] =     { PK_METHOD(exprLiteral),   PK_NO_INFIX };
        rules[TK("@str")] =     { PK_METHOD(exprLiteral),   PK_NO_INFIX };
        rules[TK("@fstr")] =    { PK_METHOD(exprFString),   PK_NO_INFIX };
        rules[TK("@long")] =    { PK_METHOD(exprLong),      PK_NO_INFIX };
        rules[TK("@imag")] =    { PK_METHOD(exprImag),      PK_NO_INFIX };
        rules[TK("@bytes")] =   { PK_METHOD(exprBytes),     PK_NO_INFIX };
        rules[TK(":")] =        { PK_METHOD(exprSlice0),    PK_METHOD(exprSlice1),      PREC_PRIMARY };
        
#undef PK_METHOD
#undef PK_NO_INFIX
    }

    bool Compiler::match(TokenIndex expected) {
        if (curr().type != expected) return false;
        advance();
        return true;
    }

    void Compiler::consume(TokenIndex expected) {
        if (!match(expected)){
            SyntaxError(
                _S("expected '", TK_STR(expected), "', got '", TK_STR(curr().type), "'")
            );
        }
    }

    bool Compiler::match_newlines_repl(){
        return match_newlines(mode()==REPL_MODE);
    }

    bool Compiler::match_newlines(bool repl_throw) {
        bool consumed = false;
        if (curr().type == TK("@eol")) {
            while (curr().type == TK("@eol")) advance();
            consumed = true;
        }
        if (repl_throw && curr().type == TK("@eof")){
            throw NeedMoreLines(ctx()->is_compiling_class);
        }
        return consumed;
    }

    bool Compiler::match_end_stmt() {
        if (match(TK(";"))) { match_newlines(); return true; }
        if (match_newlines() || curr().type == TK("@eof")) return true;
        if (curr().type == TK("@dedent")) return true;
        return false;
    }

    void Compiler::consume_end_stmt() {
        if (!match_end_stmt()) SyntaxError("expected statement end");
    }

    void Compiler::EXPR() {
        parse_expression(PREC_LOWEST+1);
    }

    void Compiler::EXPR_TUPLE(bool allow_slice) {
        parse_expression(PREC_LOWEST+1, allow_slice);
        if(!match(TK(","))) return;
        // tuple expression
        std::vector<Expr_> items;
        items.push_back(ctx()->s_expr.popx());
        do {
            if(curr().brackets_level) match_newlines_repl();
            if(!is_expression(allow_slice)) break;
            parse_expression(PREC_LOWEST+1, allow_slice);
            items.push_back(ctx()->s_expr.popx());
            if(curr().brackets_level) match_newlines_repl();
        } while(match(TK(",")));
        ctx()->s_expr.push(make_expr<TupleExpr>(std::move(items)));
    }

    // special case for `for loop` and `comp`
    Expr_ Compiler::EXPR_VARS(){
        std::vector<Expr_> items;
        do {
            consume(TK("@id"));
            items.push_back(make_expr<NameExpr>(prev().str(), name_scope()));
        } while(match(TK(",")));
        if(items.size()==1) return std::move(items[0]);
        return make_expr<TupleExpr>(std::move(items));
    }

    void Compiler::exprLiteral(){
        ctx()->s_expr.push(make_expr<LiteralExpr>(prev().value));
    }

    void Compiler::exprLong(){
        ctx()->s_expr.push(make_expr<LongExpr>(prev().str()));
    }

    void Compiler::exprImag(){
        ctx()->s_expr.push(make_expr<ImagExpr>(std::get<f64>(prev().value)));
    }

    void Compiler::exprBytes(){
        ctx()->s_expr.push(make_expr<BytesExpr>(std::get<Str>(prev().value)));
    }

    void Compiler::exprFString(){
        ctx()->s_expr.push(make_expr<FStringExpr>(std::get<Str>(prev().value)));
    }

    void Compiler::exprLambda(){
        FuncDecl_ decl = push_f_context("<lambda>");
        auto e = make_expr<LambdaExpr>(decl);
        if(!match(TK(":"))){
            _compile_f_args(e->decl, false);
            consume(TK(":"));
        }
        // https://github.com/pocketpy/pocketpy/issues/37
        parse_expression(PREC_LAMBDA + 1);
        ctx()->emit_expr();
        ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
        pop_context();
        ctx()->s_expr.push(std::move(e));
    }

    void Compiler::exprOr(){
        auto e = make_expr<OrExpr>();
        e->lhs = ctx()->s_expr.popx();
        parse_expression(PREC_LOGICAL_OR + 1);
        e->rhs = ctx()->s_expr.popx();
        ctx()->s_expr.push(std::move(e));
    }
    
    void Compiler::exprAnd(){
        auto e = make_expr<AndExpr>();
        e->lhs = ctx()->s_expr.popx();
        parse_expression(PREC_LOGICAL_AND + 1);
        e->rhs = ctx()->s_expr.popx();
        ctx()->s_expr.push(std::move(e));
    }
    
    void Compiler::exprTernary(){
        auto e = make_expr<TernaryExpr>();
        e->true_expr = ctx()->s_expr.popx();
        // cond
        parse_expression(PREC_TERNARY + 1);
        e->cond = ctx()->s_expr.popx();
        consume(TK("else"));
        // if false
        parse_expression(PREC_TERNARY + 1);
        e->false_expr = ctx()->s_expr.popx();
        ctx()->s_expr.push(std::move(e));
    }
    
    void Compiler::exprBinaryOp(){
        auto e = make_expr<BinaryExpr>();
        e->op = prev().type;
        e->lhs = ctx()->s_expr.popx();
        parse_expression(rules[e->op].precedence + 1);
        e->rhs = ctx()->s_expr.popx();
        ctx()->s_expr.push(std::move(e));
    }

    void Compiler::exprNot() {
        parse_expression(PREC_LOGICAL_NOT + 1);
        ctx()->s_expr.push(make_expr<NotExpr>(ctx()->s_expr.popx()));
    }
    
    void Compiler::exprUnaryOp(){
        TokenIndex op = prev().type;
        parse_expression(PREC_UNARY + 1);
        switch(op){
            case TK("-"):
                ctx()->s_expr.push(make_expr<NegatedExpr>(ctx()->s_expr.popx()));
                break;
            case TK("~"):
                ctx()->s_expr.push(make_expr<InvertExpr>(ctx()->s_expr.popx()));
                break;
            case TK("*"):
                ctx()->s_expr.push(make_expr<StarredExpr>(1, ctx()->s_expr.popx()));
                break;
            case TK("**"):
                ctx()->s_expr.push(make_expr<StarredExpr>(2, ctx()->s_expr.popx()));
                break;
            default: PK_FATAL_ERROR();
        }
    }

    void Compiler::exprGroup(){
        match_newlines_repl();
        EXPR_TUPLE();   // () is just for change precedence
        match_newlines_repl();
        consume(TK(")"));
        if(ctx()->s_expr.top()->is_tuple()) return;
        Expr_ g = make_expr<GroupedExpr>(ctx()->s_expr.popx());
        ctx()->s_expr.push(std::move(g));
    }

    void Compiler::exprList() {
        int line = prev().line;
        std::vector<Expr_> items;
        do {
            match_newlines_repl();
            if (curr().type == TK("]")) break;
            EXPR();
            items.push_back(ctx()->s_expr.popx());
            match_newlines_repl();
            if(items.size()==1 && match(TK("for"))){
                _consume_comp<ListCompExpr>(std::move(items[0]));
                consume(TK("]"));
                return;
            }
            match_newlines_repl();
        } while (match(TK(",")));
        consume(TK("]"));
        auto e = make_expr<ListExpr>(std::move(items));
        e->line = line;     // override line
        ctx()->s_expr.push(std::move(e));
    }

    void Compiler::exprMap() {
        bool parsing_dict = false;  // {...} may be dict or set
        std::vector<Expr_> items;
        do {
            match_newlines_repl();
            if (curr().type == TK("}")) break;
            EXPR();
            int star_level = ctx()->s_expr.top()->star_level();
            if(star_level==2 || curr().type == TK(":")){
                parsing_dict = true;
            }
            if(parsing_dict){
                auto dict_item = make_expr<DictItemExpr>();
                if(star_level == 2){
                    dict_item->key = nullptr;
                    dict_item->value = ctx()->s_expr.popx();
                }else{
                    consume(TK(":"));
                    EXPR();
                    dict_item->key = ctx()->s_expr.popx();
                    dict_item->value = ctx()->s_expr.popx();
                }
                items.push_back(std::move(dict_item));
            }else{
                items.push_back(ctx()->s_expr.popx());
            }
            match_newlines_repl();
            if(items.size()==1 && match(TK("for"))){
                if(parsing_dict) _consume_comp<DictCompExpr>(std::move(items[0]));
                else _consume_comp<SetCompExpr>(std::move(items[0]));
                consume(TK("}"));
                return;
            }
            match_newlines_repl();
        } while (match(TK(",")));
        consume(TK("}"));
        if(items.size()==0 || parsing_dict){
            auto e = make_expr<DictExpr>(std::move(items));
            ctx()->s_expr.push(std::move(e));
        }else{
            auto e = make_expr<SetExpr>(std::move(items));
            ctx()->s_expr.push(std::move(e));
        }
    }

    void Compiler::exprCall() {
        auto e = make_expr<CallExpr>();
        e->callable = ctx()->s_expr.popx();
        do {
            match_newlines_repl();
            if (curr().type==TK(")")) break;
            if(curr().type==TK("@id") && next().type==TK("=")) {
                consume(TK("@id"));
                Str key = prev().str();
                consume(TK("="));
                EXPR();
                e->kwargs.push_back({key, ctx()->s_expr.popx()});
            } else{
                EXPR();
                if(ctx()->s_expr.top()->star_level() == 2){
                    // **kwargs
                    e->kwargs.push_back({"**", ctx()->s_expr.popx()});
                }else{
                    // positional argument
                    if(!e->kwargs.empty()) SyntaxError("positional argument follows keyword argument");
                    e->args.push_back(ctx()->s_expr.popx());
                }
            }
            match_newlines_repl();
        } while (match(TK(",")));
        consume(TK(")"));
        if(e->args.size() > 32767) SyntaxError("too many positional arguments");
        if(e->kwargs.size() > 32767) SyntaxError("too many keyword arguments");
        ctx()->s_expr.push(std::move(e));
    }

    void Compiler::exprName(){
        Str name = prev().str();
        NameScope scope = name_scope();
        if(ctx()->global_names.count(name)){
            scope = NAME_GLOBAL;
        }
        ctx()->s_expr.push(make_expr<NameExpr>(name, scope));
    }

    void Compiler::exprAttrib() {
        consume(TK("@id"));
        ctx()->s_expr.push(
            make_expr<AttribExpr>(ctx()->s_expr.popx(), StrName::get(prev().sv()))
        );
    }

    void Compiler::exprSlice0() {
        auto slice = make_expr<SliceExpr>();
        if(is_expression()){        // :<stop>
            EXPR();
            slice->stop = ctx()->s_expr.popx();
            // try optional step
            if(match(TK(":"))){     // :<stop>:<step>
                EXPR();
                slice->step = ctx()->s_expr.popx();
            }
        }else if(match(TK(":"))){
            if(is_expression()){    // ::<step>
                EXPR();
                slice->step = ctx()->s_expr.popx();
            }   // else ::
        }   // else :
        ctx()->s_expr.push(std::move(slice));
    }

    void Compiler::exprSlice1() {
        auto slice = make_expr<SliceExpr>();
        slice->start = ctx()->s_expr.popx();
        if(is_expression()){        // <start>:<stop>
            EXPR();
            slice->stop = ctx()->s_expr.popx();
            // try optional step
            if(match(TK(":"))){     // <start>:<stop>:<step>
                EXPR();
                slice->step = ctx()->s_expr.popx();
            }
        }else if(match(TK(":"))){   // <start>::<step>
            EXPR();
            slice->step = ctx()->s_expr.popx();
        }   // else <start>:
        ctx()->s_expr.push(std::move(slice));
    }
    
    void Compiler::exprSubscr() {
        auto e = make_expr<SubscrExpr>();
        match_newlines_repl();
        e->a = ctx()->s_expr.popx();        // a
        EXPR_TUPLE(true);
        e->b = ctx()->s_expr.popx();        // a[<expr>]
        match_newlines_repl();
        consume(TK("]"));
        ctx()->s_expr.push(std::move(e));
    }

    void Compiler::exprLiteral0() {
        ctx()->s_expr.push(make_expr<Literal0Expr>(prev().type));
    }

    void Compiler::compile_block_body(void (Compiler::*callback)()) {
        if(callback == nullptr) callback = &Compiler::compile_stmt;
        consume(TK(":"));
        if(curr().type!=TK("@eol") && curr().type!=TK("@eof")){
            compile_stmt();     // inline block
            return;
        }
        if(!match_newlines(mode()==REPL_MODE)){
            SyntaxError("expected a new line after ':'");
        }
        consume(TK("@indent"));
        while (curr().type != TK("@dedent")) {
            match_newlines();
            (this->*callback)();
            match_newlines();
        }
        consume(TK("@dedent"));
    }

    // import a [as b]
    // import a [as b], c [as d]
    void Compiler::compile_normal_import() {
        do {
            consume(TK("@id"));
            Str name = prev().str();
            ctx()->emit_(OP_IMPORT_PATH, ctx()->add_const_string(name.sv()), prev().line);
            if (match(TK("as"))) {
                consume(TK("@id"));
                name = prev().str();
            }
            ctx()->emit_store_name(name_scope(), StrName(name), prev().line);
        } while (match(TK(",")));
        consume_end_stmt();
    }

    // from a import b [as c], d [as e]
    // from a.b import c [as d]
    // from . import a [as b]
    // from .a import b [as c]
    // from ..a import b [as c]
    // from .a.b import c [as d]
    // from xxx import *
    void Compiler::compile_from_import() {
        int dots = 0;

        while(true){
            switch(curr().type){
                case TK("."): dots+=1; break;
                case TK(".."): dots+=2; break;
                case TK("..."): dots+=3; break;
                default: goto __EAT_DOTS_END;
            }
            advance();
        }
__EAT_DOTS_END:
        SStream ss;
        for(int i=0; i<dots; i++) ss << '.';

        if(dots > 0){
            // @id is optional if dots > 0
            if(match(TK("@id"))){
                ss << prev().sv();
                while (match(TK("."))) {
                    consume(TK("@id"));
                    ss << "." << prev().sv();
                }
            }
        }else{
            // @id is required if dots == 0
            consume(TK("@id"));
            ss << prev().sv();
            while (match(TK("."))) {
                consume(TK("@id"));
                ss << "." << prev().sv();
            }
        }

        ctx()->emit_(OP_IMPORT_PATH, ctx()->add_const_string(ss.str().sv()), prev().line);
        consume(TK("import"));

        if (match(TK("*"))) {
            if(name_scope() != NAME_GLOBAL) SyntaxError("from <module> import * can only be used in global scope");
            // pop the module and import __all__
            ctx()->emit_(OP_POP_IMPORT_STAR, BC_NOARG, prev().line);
            consume_end_stmt();
            return;
        }

        do {
            ctx()->emit_(OP_DUP_TOP, BC_NOARG, BC_KEEPLINE);
            consume(TK("@id"));
            Str name = prev().str();
            ctx()->emit_(OP_LOAD_ATTR, StrName(name).index, prev().line);
            if (match(TK("as"))) {
                consume(TK("@id"));
                name = prev().str();
            }
            ctx()->emit_store_name(name_scope(), StrName(name), prev().line);
        } while (match(TK(",")));
        ctx()->emit_(OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
        consume_end_stmt();
    }

    bool Compiler::is_expression(bool allow_slice){
        PrattCallback prefix = rules[curr().type].prefix;
        return prefix != nullptr && (allow_slice || curr().type!=TK(":"));
    }

    void Compiler::parse_expression(int precedence, bool allow_slice) {
        PrattCallback prefix = rules[curr().type].prefix;
        if (prefix==nullptr || (curr().type==TK(":") && !allow_slice)){
            SyntaxError(Str("expected an expression, got ") + TK_STR(curr().type));
        }
        advance();
        (this->*prefix)();
        while (rules[curr().type].precedence >= precedence && (allow_slice || curr().type!=TK(":"))) {
            TokenIndex op = curr().type;
            advance();
            PrattCallback infix = rules[op].infix;
            PK_ASSERT(infix != nullptr);
            (this->*infix)();
        }
    }

    void Compiler::compile_if_stmt() {
        EXPR();   // condition
        ctx()->emit_expr();
        int patch = ctx()->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, prev().line);
        compile_block_body();
        if (match(TK("elif"))) {
            int exit_patch = ctx()->emit_(OP_JUMP_ABSOLUTE, BC_NOARG, prev().line);
            ctx()->patch_jump(patch);
            compile_if_stmt();
            ctx()->patch_jump(exit_patch);
        } else if (match(TK("else"))) {
            int exit_patch = ctx()->emit_(OP_JUMP_ABSOLUTE, BC_NOARG, prev().line);
            ctx()->patch_jump(patch);
            compile_block_body();
            ctx()->patch_jump(exit_patch);
        } else {
            ctx()->patch_jump(patch);
        }
    }

    void Compiler::compile_while_loop() {
        CodeBlock* block = ctx()->enter_block(CodeBlockType::WHILE_LOOP);
        EXPR();   // condition
        ctx()->emit_expr();
        int patch = ctx()->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, prev().line);
        compile_block_body();
        ctx()->emit_(OP_LOOP_CONTINUE, ctx()->get_loop(), BC_KEEPLINE);
        ctx()->patch_jump(patch);
        ctx()->exit_block();
        // optional else clause
        if (match(TK("else"))) {
            compile_block_body();
            block->end2 = ctx()->co->codes.size();
        }
    }

    void Compiler::compile_for_loop() {
        Expr_ vars = EXPR_VARS();
        consume(TK("in"));
        EXPR_TUPLE(); ctx()->emit_expr();
        ctx()->emit_(OP_GET_ITER, BC_NOARG, BC_KEEPLINE);
        CodeBlock* block = ctx()->enter_block(CodeBlockType::FOR_LOOP);
        ctx()->emit_(OP_FOR_ITER, BC_NOARG, BC_KEEPLINE);
        bool ok = vars->emit_store(ctx());
        if(!ok) SyntaxError();  // this error occurs in `vars` instead of this line, but...nevermind
        compile_block_body();
        ctx()->emit_(OP_LOOP_CONTINUE, ctx()->get_loop(), BC_KEEPLINE);
        ctx()->exit_block();
        // optional else clause
        if (match(TK("else"))) {
            compile_block_body();
            block->end2 = ctx()->co->codes.size();
        }
    }

    void Compiler::compile_try_except() {
        ctx()->enter_block(CodeBlockType::TRY_EXCEPT);
        compile_block_body();
        std::vector<int> patches = {
            ctx()->emit_(OP_JUMP_ABSOLUTE, BC_NOARG, BC_KEEPLINE)
        };
        ctx()->exit_block();

        int finally_entry = -1;
        if(curr().type != TK("finally")){
            do {
                StrName as_name;
                consume(TK("except"));
                if(is_expression()){
                    EXPR();      // push assumed type on to the stack
                    ctx()->emit_expr();
                    ctx()->emit_(OP_EXCEPTION_MATCH, BC_NOARG, prev().line);
                    if(match(TK("as"))){
                        consume(TK("@id"));
                        as_name = StrName(prev().sv());
                    }
                }else{
                    ctx()->emit_(OP_LOAD_TRUE, BC_NOARG, BC_KEEPLINE);
                }
                int patch = ctx()->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, BC_KEEPLINE);
                // on match
                if(!as_name.empty()){
                    ctx()->emit_(OP_DUP_TOP, BC_NOARG, BC_KEEPLINE);
                    ctx()->emit_store_name(name_scope(), as_name, BC_KEEPLINE);
                }
                // pop the exception 
                ctx()->emit_(OP_POP_EXCEPTION, BC_NOARG, BC_KEEPLINE);
                compile_block_body();
                patches.push_back(ctx()->emit_(OP_JUMP_ABSOLUTE, BC_NOARG, BC_KEEPLINE));
                ctx()->patch_jump(patch);
            }while(curr().type == TK("except"));
        }

        if(match(TK("finally"))){
            int patch = ctx()->emit_(OP_JUMP_ABSOLUTE, BC_NOARG, BC_KEEPLINE);
            finally_entry = ctx()->co->codes.size();
            compile_block_body();
            ctx()->emit_(OP_JUMP_ABSOLUTE_TOP, BC_NOARG, BC_KEEPLINE);
            ctx()->patch_jump(patch);
        }
        // no match, re-raise
        if(finally_entry != -1){
            ctx()->emit_(OP_LOAD_INTEGER, (uint16_t)ctx()->co->codes.size()+2, BC_KEEPLINE);
            ctx()->emit_(OP_JUMP_ABSOLUTE, finally_entry, BC_KEEPLINE);
        }
        ctx()->emit_(OP_RE_RAISE, BC_NOARG, BC_KEEPLINE);

        // no exception or no match, jump to the end
        for (int patch : patches) ctx()->patch_jump(patch);
        if(finally_entry != -1){
            ctx()->emit_(OP_LOAD_INTEGER, (uint16_t)ctx()->co->codes.size()+2, BC_KEEPLINE);
            ctx()->emit_(OP_JUMP_ABSOLUTE, finally_entry, BC_KEEPLINE);
        }
    }

    void Compiler::compile_decorated(){
        std::vector<Expr_> decorators;
        do{
            EXPR();
            decorators.push_back(ctx()->s_expr.popx());
            if(!match_newlines_repl()) SyntaxError();
        }while(match(TK("@")));

        if(match(TK("class"))){
            compile_class(decorators);
        }else{
            consume(TK("def"));
            compile_function(decorators);
        }
    }

    bool Compiler::try_compile_assignment(){
        switch (curr().type) {
            case TK("+="): case TK("-="): case TK("*="): case TK("/="): case TK("//="): case TK("%="):
            case TK("<<="): case TK(">>="): case TK("&="): case TK("|="): case TK("^="): {
                Expr* lhs_p = ctx()->s_expr.top().get();
                if(lhs_p->is_starred()) SyntaxError();
                if(ctx()->is_compiling_class) SyntaxError("can't use inplace operator in class definition");
                advance();
                auto e = make_expr<BinaryExpr>();
                e->op = prev().type - 1; // -1 to remove =
                e->lhs = ctx()->s_expr.popx();
                EXPR_TUPLE();
                e->rhs = ctx()->s_expr.popx();
                if(e->is_starred()) SyntaxError();
                e->emit_(ctx());
                bool ok = lhs_p->emit_store(ctx());
                if(!ok) SyntaxError();
            } return true;
            case TK("="): {
                int n = 0;
                while(match(TK("="))){
                    EXPR_TUPLE();
                    n += 1;
                }
                // stack size is n+1
                Expr_ val = ctx()->s_expr.popx();
                val->emit_(ctx());
                for(int j=1; j<n; j++) ctx()->emit_(OP_DUP_TOP, BC_NOARG, BC_KEEPLINE);
                for(int j=0; j<n; j++){
                    auto e = ctx()->s_expr.popx();
                    if(e->is_starred()) SyntaxError();
                    bool ok = e->emit_store(ctx());
                    if(!ok) SyntaxError();
                }
            } return true;
            default: return false;
        }
    }

    void Compiler::compile_stmt() {
        if(match(TK("class"))){
            compile_class();
            return;
        }
        advance();
        int kw_line = prev().line;  // backup line number
        int curr_loop_block = ctx()->get_loop();
        switch(prev().type){
            case TK("break"):
                if (curr_loop_block < 0) SyntaxError("'break' outside loop");
                ctx()->emit_(OP_LOOP_BREAK, curr_loop_block, kw_line);
                consume_end_stmt();
                break;
            case TK("continue"):
                if (curr_loop_block < 0) SyntaxError("'continue' not properly in loop");
                ctx()->emit_(OP_LOOP_CONTINUE, curr_loop_block, kw_line);
                consume_end_stmt();
                break;
            case TK("yield"): 
                if (contexts.size() <= 1) SyntaxError("'yield' outside function");
                EXPR_TUPLE(); ctx()->emit_expr();
                // if yield present, mark the function as generator
                ctx()->co->is_generator = true;
                ctx()->emit_(OP_YIELD_VALUE, BC_NOARG, kw_line);
                consume_end_stmt();
                break;
            case TK("yield from"):
                if (contexts.size() <= 1) SyntaxError("'yield from' outside function");
                EXPR_TUPLE(); ctx()->emit_expr();
                // if yield from present, mark the function as generator
                ctx()->co->is_generator = true;
                ctx()->emit_(OP_GET_ITER, BC_NOARG, kw_line);
                ctx()->enter_block(CodeBlockType::FOR_LOOP);
                ctx()->emit_(OP_FOR_ITER, BC_NOARG, BC_KEEPLINE);
                ctx()->emit_(OP_YIELD_VALUE, BC_NOARG, BC_KEEPLINE);
                ctx()->emit_(OP_LOOP_CONTINUE, ctx()->get_loop(), BC_KEEPLINE);
                ctx()->exit_block();
                consume_end_stmt();
                break;
            case TK("return"):
                if (contexts.size() <= 1) SyntaxError("'return' outside function");
                if(match_end_stmt()){
                    ctx()->emit_(OP_RETURN_VALUE, 1, kw_line);
                }else{
                    EXPR_TUPLE(); ctx()->emit_expr();
                    // check if it is a generator
                    if(ctx()->co->is_generator) SyntaxError("'return' with argument inside generator function");
                    consume_end_stmt();
                    ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, kw_line);
                }
                break;
            /*************************************************/
            case TK("if"): compile_if_stmt(); break;
            case TK("while"): compile_while_loop(); break;
            case TK("for"): compile_for_loop(); break;
            case TK("import"): compile_normal_import(); break;
            case TK("from"): compile_from_import(); break;
            case TK("def"): compile_function(); break;
            case TK("@"): compile_decorated(); break;
            case TK("try"): compile_try_except(); break;
            case TK("pass"): consume_end_stmt(); break;
            /*************************************************/
            case TK("++"):{
                consume(TK("@id"));
                StrName name(prev().sv());
                NameScope scope = name_scope();
                bool is_global = ctx()->global_names.count(name.sv());
                if(is_global) scope = NAME_GLOBAL;
                switch(scope){
                    case NAME_LOCAL:
                        ctx()->emit_(OP_INC_FAST, ctx()->add_varname(name), prev().line);
                        break;
                    case NAME_GLOBAL:
                        ctx()->emit_(OP_INC_GLOBAL, name.index, prev().line);
                        break;
                    default: SyntaxError(); break;
                }
                consume_end_stmt();
                break;
            }
            case TK("--"):{
                consume(TK("@id"));
                StrName name(prev().sv());
                switch(name_scope()){
                    case NAME_LOCAL:
                        ctx()->emit_(OP_DEC_FAST, ctx()->add_varname(name), prev().line);
                        break;
                    case NAME_GLOBAL:
                        ctx()->emit_(OP_DEC_GLOBAL, name.index, prev().line);
                        break;
                    default: SyntaxError(); break;
                }
                consume_end_stmt();
                break;
            }
            case TK("assert"):{
                EXPR();    // condition
                ctx()->emit_expr();
                int index = ctx()->emit_(OP_POP_JUMP_IF_TRUE, BC_NOARG, kw_line);
                int has_msg = 0;
                if(match(TK(","))){
                    EXPR();    // message
                    ctx()->emit_expr();
                    has_msg = 1;
                }
                ctx()->emit_(OP_RAISE_ASSERT, has_msg, kw_line);
                ctx()->patch_jump(index);
                consume_end_stmt();
                break;
            }
            case TK("global"):
                do {
                    consume(TK("@id"));
                    ctx()->global_names.insert(prev().str());
                } while (match(TK(",")));
                consume_end_stmt();
                break;
            case TK("raise"): {
                EXPR(); ctx()->emit_expr();
                ctx()->emit_(OP_RAISE, BC_NOARG, kw_line);
                consume_end_stmt();
            } break;
            case TK("del"): {
                EXPR_TUPLE();
                Expr_ e = ctx()->s_expr.popx();
                bool ok = e->emit_del(ctx());
                if(!ok) SyntaxError();
                consume_end_stmt();
            } break;
            case TK("with"): {
                EXPR();    // [ <expr> ]
                ctx()->emit_expr();
                ctx()->enter_block(CodeBlockType::CONTEXT_MANAGER);
                Expr_ as_name;
                if(match(TK("as"))){
                    consume(TK("@id"));
                    as_name = make_expr<NameExpr>(prev().str(), name_scope());
                }
                ctx()->emit_(OP_WITH_ENTER, BC_NOARG, prev().line);
                // [ <expr> <expr>.__enter__() ]
                if(as_name){
                    bool ok = as_name->emit_store(ctx());
                    if(!ok) SyntaxError();
                }else{
                    ctx()->emit_(OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
                }
                compile_block_body();
                ctx()->emit_(OP_WITH_EXIT, BC_NOARG, prev().line);
                ctx()->exit_block();
            } break;
            /*************************************************/
            case TK("=="): {
                consume(TK("@id"));
                if(mode()!=EXEC_MODE) SyntaxError("'label' is only available in EXEC_MODE");
                bool ok = ctx()->add_label(prev().str());
                consume(TK("=="));
                if(!ok) SyntaxError("label " + prev().str().escape() + " already exists");
                consume_end_stmt();
            } break;
            case TK("->"):
                consume(TK("@id"));
                if(mode()!=EXEC_MODE) SyntaxError("'goto' is only available in EXEC_MODE");
                ctx()->emit_(OP_GOTO, StrName(prev().sv()).index, prev().line);
                consume_end_stmt();
                break;
            /*************************************************/
            // handle dangling expression or assignment
            default: {
                advance(-1);    // do revert since we have pre-called advance() at the beginning
                EXPR_TUPLE();

                bool is_typed_name = false;     // e.g. x: int
                // eat variable's type hint if it is a single name
                if(ctx()->s_expr.top()->is_name()){
                    if(match(TK(":"))){
                        consume_type_hints();
                        is_typed_name = true;

                        if(ctx()->is_compiling_class){
                            NameExpr* ne = static_cast<NameExpr*>(ctx()->s_expr.top().get());
                            ctx()->emit_(OP_ADD_CLASS_ANNOTATION, ne->name.index, BC_KEEPLINE);
                        }
                    }
                }
                if(!try_compile_assignment()){
                    if(!ctx()->s_expr.empty() && ctx()->s_expr.top()->is_starred()){
                        SyntaxError();
                    }
                    if(!is_typed_name){
                        ctx()->emit_expr();
                        if((mode()==CELL_MODE || mode()==REPL_MODE) && name_scope()==NAME_GLOBAL){
                            ctx()->emit_(OP_PRINT_EXPR, BC_NOARG, BC_KEEPLINE);
                        }else{
                            ctx()->emit_(OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
                        }
                    }else{
                        PK_ASSERT(ctx()->s_expr.size() == 1)
                        ctx()->s_expr.pop();
                    }
                }
                consume_end_stmt();
            }
        }
    }

    void Compiler::consume_type_hints(){
        EXPR();
        ctx()->s_expr.pop();
    }

    void Compiler::_add_decorators(const std::vector<Expr_>& decorators){
        // [obj]
        for(auto it=decorators.rbegin(); it!=decorators.rend(); ++it){
            (*it)->emit_(ctx());                                    // [obj, f]
            ctx()->emit_(OP_ROT_TWO, BC_NOARG, (*it)->line);        // [f, obj]
            ctx()->emit_(OP_LOAD_NULL, BC_NOARG, BC_KEEPLINE);      // [f, obj, NULL]
            ctx()->emit_(OP_ROT_TWO, BC_NOARG, BC_KEEPLINE);        // [obj, NULL, f]
            ctx()->emit_(OP_CALL, 1, (*it)->line);                  // [obj]
        }
    }

    void Compiler::compile_class(const std::vector<Expr_>& decorators){
        consume(TK("@id"));
        int namei = StrName(prev().sv()).index;
        Expr_ base = nullptr;
        if(match(TK("("))){
            if(is_expression()){
                EXPR();
                base = ctx()->s_expr.popx();
            }
            consume(TK(")"));
        }
        if(base == nullptr){
            ctx()->emit_(OP_LOAD_NONE, BC_NOARG, prev().line);
        }else {
            base->emit_(ctx());
        }
        ctx()->emit_(OP_BEGIN_CLASS, namei, BC_KEEPLINE);

        for(auto& c: this->contexts.data()){
            if(c.is_compiling_class){
                SyntaxError("nested class is not allowed");
            }
        }
        ctx()->is_compiling_class = true;
        compile_block_body();
        ctx()->is_compiling_class = false;

        if(!decorators.empty()){
            ctx()->emit_(OP_BEGIN_CLASS_DECORATION, BC_NOARG, BC_KEEPLINE);
            _add_decorators(decorators);
            ctx()->emit_(OP_END_CLASS_DECORATION, BC_NOARG, BC_KEEPLINE);
        }

        ctx()->emit_(OP_END_CLASS, namei, BC_KEEPLINE);
    }

    void Compiler::_compile_f_args(FuncDecl_ decl, bool enable_type_hints){
        int state = 0;      // 0 for args, 1 for *args, 2 for k=v, 3 for **kwargs
        do {
            if(state > 3) SyntaxError();
            if(state == 3) SyntaxError("**kwargs should be the last argument");
            match_newlines();
            if(match(TK("*"))){
                if(state < 1) state = 1;
                else SyntaxError("*args should be placed before **kwargs");
            }
            else if(match(TK("**"))){
                state = 3;
            }
            consume(TK("@id"));
            StrName name = prev().str();

            // check duplicate argument name
            for(int j: decl->args){
                if(decl->code->varnames[j] == name) {
                    SyntaxError("duplicate argument name");
                }
            }
            for(auto& kv: decl->kwargs){
                if(decl->code->varnames[kv.index] == name){
                    SyntaxError("duplicate argument name");
                }
            }
            if(decl->starred_arg!=-1 && decl->code->varnames[decl->starred_arg] == name){
                SyntaxError("duplicate argument name");
            }
            if(decl->starred_kwarg!=-1 && decl->code->varnames[decl->starred_kwarg] == name){
                SyntaxError("duplicate argument name");
            }

            // eat type hints
            if(enable_type_hints && match(TK(":"))) consume_type_hints();
            if(state == 0 && curr().type == TK("=")) state = 2;
            int index = ctx()->add_varname(name);
            switch (state)
            {
                case 0:
                    decl->args.push_back(index);
                    break;
                case 1:
                    decl->starred_arg = index;
                    state+=1;
                    break;
                case 2: {
                    consume(TK("="));
                    PyObject* value = read_literal();
                    if(value == nullptr){
                        SyntaxError(Str("default argument must be a literal"));
                    }
                    decl->add_kwarg(index, name, value);
                } break;
                case 3:
                    decl->starred_kwarg = index;
                    state+=1;
                    break;
            }
        } while (match(TK(",")));
    }

    void Compiler::compile_function(const std::vector<Expr_>& decorators){
        const char* _start = curr().start;
        consume(TK("@id"));
        Str decl_name = prev().str();
        FuncDecl_ decl = push_f_context(decl_name);
        consume(TK("("));
        if (!match(TK(")"))) {
            _compile_f_args(decl, true);
            consume(TK(")"));
        }
        if(match(TK("->"))) consume_type_hints();
        const char* _end = curr().start;
        decl->signature = Str(_start, _end-_start);
        compile_block_body();
        pop_context();

        PyObject* docstring = nullptr;
        if(decl->code->codes.size()>=2 && decl->code->codes[0].op == OP_LOAD_CONST && decl->code->codes[1].op == OP_POP_TOP){
            PyObject* c = decl->code->consts[decl->code->codes[0].arg];
            if(is_type(c, vm->tp_str)){
                decl->code->codes[0].op = OP_NO_OP;
                decl->code->codes[1].op = OP_NO_OP;
                docstring = c;
            }
        }
        if(docstring != nullptr){
            decl->docstring = PK_OBJ_GET(Str, docstring);
        }
        ctx()->emit_(OP_LOAD_FUNCTION, ctx()->add_func_decl(decl), prev().line);

        _add_decorators(decorators);

        if(!ctx()->is_compiling_class){
            auto e = make_expr<NameExpr>(decl_name, name_scope());
            e->emit_store(ctx());
        }else{
            int index = StrName(decl_name).index;
            ctx()->emit_(OP_STORE_CLASS_ATTR, index, prev().line);
        }
    }

    PyObject* Compiler::to_object(const TokenValue& value){
        PyObject* obj = nullptr;
        if(std::holds_alternative<i64>(value)){
            obj = VAR(std::get<i64>(value));
        }
        if(std::holds_alternative<f64>(value)){
            obj = VAR(std::get<f64>(value));
        }
        if(std::holds_alternative<Str>(value)){
            obj = VAR(std::get<Str>(value));
        }
        PK_ASSERT(obj != nullptr)
        return obj;
    }

    PyObject* Compiler::read_literal(){
        advance();
        switch(prev().type){
            case TK("-"): {
                consume(TK("@num"));
                PyObject* val = to_object(prev().value);
                return vm->py_negate(val);
            }
            case TK("@num"): return to_object(prev().value);
            case TK("@str"): return to_object(prev().value);
            case TK("True"): return VAR(true);
            case TK("False"): return VAR(false);
            case TK("None"): return vm->None;
            case TK("..."): return vm->Ellipsis;
            case TK("("): {
                List cpnts;
                while(true) {
                    cpnts.push_back(read_literal());
                    if(curr().type == TK(")")) break;
                    consume(TK(","));
                    if(curr().type == TK(")")) break;
                }
                consume(TK(")"));
                return VAR(Tuple(std::move(cpnts)));
            }
            default: break;
        }
        return nullptr;
    }

    Compiler::Compiler(VM* vm, std::string_view source, const Str& filename, CompileMode mode, bool unknown_global_scope){
        this->vm = vm;
        this->used = false;
        this->unknown_global_scope = unknown_global_scope;
        this->lexer = std::make_unique<Lexer>(
            vm, std::make_shared<SourceData>(source, filename, mode)
        );
        init_pratt_rules();
    }


    CodeObject_ Compiler::compile(){
        PK_ASSERT(!used)
        used = true;

        tokens = lexer->run();
        CodeObject_ code = push_global_context();

        advance();          // skip @sof, so prev() is always valid
        match_newlines();   // skip possible leading '\n'

        if(mode()==EVAL_MODE) {
            EXPR_TUPLE(); ctx()->emit_expr();
            consume(TK("@eof"));
            ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
            pop_context();
            return code;
        }else if(mode()==JSON_MODE){
            EXPR();
            Expr_ e = ctx()->s_expr.popx();
            if(!e->is_json_object()) SyntaxError("expect a JSON object, literal or array");
            consume(TK("@eof"));
            e->emit_(ctx());
            ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
            pop_context();
            return code;
        }

        while (!match(TK("@eof"))) {
            compile_stmt();
            match_newlines();
        }
        pop_context();
        return code;
    }

    // TODO: refactor this
    void Lexer::throw_err(StrName type, Str msg, int lineno, const char* cursor){
        PyObject* e_obj = vm->call(vm->builtins->attr(type), VAR(msg));
        Exception& e = PK_OBJ_GET(Exception, e_obj);
        e.st_push(src, lineno, cursor, "");
        throw e;
    }
}   // namespace pkpy


namespace pkpy{
    
class REPL {
protected:
    int need_more_lines = 0;
    std::string buffer;
    VM* vm;
public:
    REPL(VM* vm);
    bool input(std::string line);
};

} // namespace pkpy
namespace pkpy {
    REPL::REPL(VM* vm) : vm(vm){
        vm->stdout_write("pocketpy " PK_VERSION " (" __DATE__ ", " __TIME__ ") ");
        vm->stdout_write(_S("[", sizeof(void*)*8, " bit] on ", kPlatformStrings[PK_SYS_PLATFORM], "\n"));
        vm->stdout_write("https://github.com/pocketpy/pocketpy" "\n");
        vm->stdout_write("Type \"exit()\" to exit." "\n");
    }

    bool REPL::input(std::string line){
        CompileMode mode = REPL_MODE;
        if(need_more_lines){
            buffer += line;
            buffer += '\n';
            int n = buffer.size();
            if(n>=need_more_lines){
                for(int i=buffer.size()-need_more_lines; i<buffer.size(); i++){
                    // no enough lines
                    if(buffer[i] != '\n') return true;
                }
                need_more_lines = 0;
                line = buffer;
                buffer.clear();
                mode = CELL_MODE;
            }else{
                return true;
            }
        }
        
        try{
            vm->exec(line, "<stdin>", mode);
        }catch(NeedMoreLines ne){
            buffer += line;
            buffer += '\n';
            need_more_lines = ne.is_compiling_class ? 3 : 2;
            if (need_more_lines) return true;
        }
        return false;
    }

}

// generated by prebuild.py

#include <map>
#include <string>

namespace pkpy{
    inline static std::map<std::string, const char*> kPythonLibs = {
        {"bisect", "\x22\x22\x22\x42\x69\x73\x65\x63\x74\x69\x6f\x6e\x20\x61\x6c\x67\x6f\x72\x69\x74\x68\x6d\x73\x2e\x22\x22\x22\x0a\x0a\x64\x65\x66\x20\x69\x6e\x73\x6f\x72\x74\x5f\x72\x69\x67\x68\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x3d\x30\x2c\x20\x68\x69\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x49\x6e\x73\x65\x72\x74\x20\x69\x74\x65\x6d\x20\x78\x20\x69\x6e\x20\x6c\x69\x73\x74\x20\x61\x2c\x20\x61\x6e\x64\x20\x6b\x65\x65\x70\x20\x69\x74\x20\x73\x6f\x72\x74\x65\x64\x20\x61\x73\x73\x75\x6d\x69\x6e\x67\x20\x61\x20\x69\x73\x20\x73\x6f\x72\x74\x65\x64\x2e\x0a\x0a\x20\x20\x20\x20\x49\x66\x20\x78\x20\x69\x73\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x69\x6e\x20\x61\x2c\x20\x69\x6e\x73\x65\x72\x74\x20\x69\x74\x20\x74\x6f\x20\x74\x68\x65\x20\x72\x69\x67\x68\x74\x20\x6f\x66\x20\x74\x68\x65\x20\x72\x69\x67\x68\x74\x6d\x6f\x73\x74\x20\x78\x2e\x0a\x0a\x20\x20\x20\x20\x4f\x70\x74\x69\x6f\x6e\x61\x6c\x20\x61\x72\x67\x73\x20\x6c\x6f\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x30\x29\x20\x61\x6e\x64\x20\x68\x69\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x6c\x65\x6e\x28\x61\x29\x29\x20\x62\x6f\x75\x6e\x64\x20\x74\x68\x65\x0a\x20\x20\x20\x20\x73\x6c\x69\x63\x65\x20\x6f\x66\x20\x61\x20\x74\x6f\x20\x62\x65\x20\x73\x65\x61\x72\x63\x68\x65\x64\x2e\x0a\x20\x20\x20\x20\x22\x22\x22\x0a\x0a\x20\x20\x20\x20\x6c\x6f\x20\x3d\x20\x62\x69\x73\x65\x63\x74\x5f\x72\x69\x67\x68\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x2c\x20\x68\x69\x29\x0a\x20\x20\x20\x20\x61\x2e\x69\x6e\x73\x65\x72\x74\x28\x6c\x6f\x2c\x20\x78\x29\x0a\x0a\x64\x65\x66\x20\x62\x69\x73\x65\x63\x74\x5f\x72\x69\x67\x68\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x3d\x30\x2c\x20\x68\x69\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x52\x65\x74\x75\x72\x6e\x20\x74\x68\x65\x20\x69\x6e\x64\x65\x78\x20\x77\x68\x65\x72\x65\x20\x74\x6f\x20\x69\x6e\x73\x65\x72\x74\x20\x69\x74\x65\x6d\x20\x78\x20\x69\x6e\x20\x6c\x69\x73\x74\x20\x61\x2c\x20\x61\x73\x73\x75\x6d\x69\x6e\x67\x20\x61\x20\x69\x73\x20\x73\x6f\x72\x74\x65\x64\x2e\x0a\x0a\x20\x20\x20\x20\x54\x68\x65\x20\x72\x65\x74\x75\x72\x6e\x20\x76\x61\x6c\x75\x65\x20\x69\x20\x69\x73\x20\x73\x75\x63\x68\x20\x74\x68\x61\x74\x20\x61\x6c\x6c\x20\x65\x20\x69\x6e\x20\x61\x5b\x3a\x69\x5d\x20\x68\x61\x76\x65\x20\x65\x20\x3c\x3d\x20\x78\x2c\x20\x61\x6e\x64\x20\x61\x6c\x6c\x20\x65\x20\x69\x6e\x0a\x20\x20\x20\x20\x61\x5b\x69\x3a\x5d\x20\x68\x61\x76\x65\x20\x65\x20\x3e\x20\x78\x2e\x20\x20\x53\x6f\x20\x69\x66\x20\x78\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x61\x70\x70\x65\x61\x72\x73\x20\x69\x6e\x20\x74\x68\x65\x20\x6c\x69\x73\x74\x2c\x20\x61\x2e\x69\x6e\x73\x65\x72\x74\x28\x78\x29\x20\x77\x69\x6c\x6c\x0a\x20\x20\x20\x20\x69\x6e\x73\x65\x72\x74\x20\x6a\x75\x73\x74\x20\x61\x66\x74\x65\x72\x20\x74\x68\x65\x20\x72\x69\x67\x68\x74\x6d\x6f\x73\x74\x20\x78\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x74\x68\x65\x72\x65\x2e\x0a\x0a\x20\x20\x20\x20\x4f\x70\x74\x69\x6f\x6e\x61\x6c\x20\x61\x72\x67\x73\x20\x6c\x6f\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x30\x29\x20\x61\x6e\x64\x20\x68\x69\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x6c\x65\x6e\x28\x61\x29\x29\x20\x62\x6f\x75\x6e\x64\x20\x74\x68\x65\x0a\x20\x20\x20\x20\x73\x6c\x69\x63\x65\x20\x6f\x66\x20\x61\x20\x74\x6f\x20\x62\x65\x20\x73\x65\x61\x72\x63\x68\x65\x64\x2e\x0a\x20\x20\x20\x20\x22\x22\x22\x0a\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x6f\x20\x3c\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x27\x6c\x6f\x20\x6d\x75\x73\x74\x20\x62\x65\x20\x6e\x6f\x6e\x2d\x6e\x65\x67\x61\x74\x69\x76\x65\x27\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x69\x20\x69\x73\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x69\x20\x3d\x20\x6c\x65\x6e\x28\x61\x29\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6c\x6f\x20\x3c\x20\x68\x69\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6d\x69\x64\x20\x3d\x20\x28\x6c\x6f\x2b\x68\x69\x29\x2f\x2f\x32\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x78\x20\x3c\x20\x61\x5b\x6d\x69\x64\x5d\x3a\x20\x68\x69\x20\x3d\x20\x6d\x69\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x20\x6c\x6f\x20\x3d\x20\x6d\x69\x64\x2b\x31\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x0a\x0a\x64\x65\x66\x20\x69\x6e\x73\x6f\x72\x74\x5f\x6c\x65\x66\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x3d\x30\x2c\x20\x68\x69\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x49\x6e\x73\x65\x72\x74\x20\x69\x74\x65\x6d\x20\x78\x20\x69\x6e\x20\x6c\x69\x73\x74\x20\x61\x2c\x20\x61\x6e\x64\x20\x6b\x65\x65\x70\x20\x69\x74\x20\x73\x6f\x72\x74\x65\x64\x20\x61\x73\x73\x75\x6d\x69\x6e\x67\x20\x61\x20\x69\x73\x20\x73\x6f\x72\x74\x65\x64\x2e\x0a\x0a\x20\x20\x20\x20\x49\x66\x20\x78\x20\x69\x73\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x69\x6e\x20\x61\x2c\x20\x69\x6e\x73\x65\x72\x74\x20\x69\x74\x20\x74\x6f\x20\x74\x68\x65\x20\x6c\x65\x66\x74\x20\x6f\x66\x20\x74\x68\x65\x20\x6c\x65\x66\x74\x6d\x6f\x73\x74\x20\x78\x2e\x0a\x0a\x20\x20\x20\x20\x4f\x70\x74\x69\x6f\x6e\x61\x6c\x20\x61\x72\x67\x73\x20\x6c\x6f\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x30\x29\x20\x61\x6e\x64\x20" "\x68\x69\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x6c\x65\x6e\x28\x61\x29\x29\x20\x62\x6f\x75\x6e\x64\x20\x74\x68\x65\x0a\x20\x20\x20\x20\x73\x6c\x69\x63\x65\x20\x6f\x66\x20\x61\x20\x74\x6f\x20\x62\x65\x20\x73\x65\x61\x72\x63\x68\x65\x64\x2e\x0a\x20\x20\x20\x20\x22\x22\x22\x0a\x0a\x20\x20\x20\x20\x6c\x6f\x20\x3d\x20\x62\x69\x73\x65\x63\x74\x5f\x6c\x65\x66\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x2c\x20\x68\x69\x29\x0a\x20\x20\x20\x20\x61\x2e\x69\x6e\x73\x65\x72\x74\x28\x6c\x6f\x2c\x20\x78\x29\x0a\x0a\x0a\x64\x65\x66\x20\x62\x69\x73\x65\x63\x74\x5f\x6c\x65\x66\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x3d\x30\x2c\x20\x68\x69\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x52\x65\x74\x75\x72\x6e\x20\x74\x68\x65\x20\x69\x6e\x64\x65\x78\x20\x77\x68\x65\x72\x65\x20\x74\x6f\x20\x69\x6e\x73\x65\x72\x74\x20\x69\x74\x65\x6d\x20\x78\x20\x69\x6e\x20\x6c\x69\x73\x74\x20\x61\x2c\x20\x61\x73\x73\x75\x6d\x69\x6e\x67\x20\x61\x20\x69\x73\x20\x73\x6f\x72\x74\x65\x64\x2e\x0a\x0a\x20\x20\x20\x20\x54\x68\x65\x20\x72\x65\x74\x75\x72\x6e\x20\x76\x61\x6c\x75\x65\x20\x69\x20\x69\x73\x20\x73\x75\x63\x68\x20\x74\x68\x61\x74\x20\x61\x6c\x6c\x20\x65\x20\x69\x6e\x20\x61\x5b\x3a\x69\x5d\x20\x68\x61\x76\x65\x20\x65\x20\x3c\x20\x78\x2c\x20\x61\x6e\x64\x20\x61\x6c\x6c\x20\x65\x20\x69\x6e\x0a\x20\x20\x20\x20\x61\x5b\x69\x3a\x5d\x20\x68\x61\x76\x65\x20\x65\x20\x3e\x3d\x20\x78\x2e\x20\x20\x53\x6f\x20\x69\x66\x20\x78\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x61\x70\x70\x65\x61\x72\x73\x20\x69\x6e\x20\x74\x68\x65\x20\x6c\x69\x73\x74\x2c\x20\x61\x2e\x69\x6e\x73\x65\x72\x74\x28\x78\x29\x20\x77\x69\x6c\x6c\x0a\x20\x20\x20\x20\x69\x6e\x73\x65\x72\x74\x20\x6a\x75\x73\x74\x20\x62\x65\x66\x6f\x72\x65\x20\x74\x68\x65\x20\x6c\x65\x66\x74\x6d\x6f\x73\x74\x20\x78\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x74\x68\x65\x72\x65\x2e\x0a\x0a\x20\x20\x20\x20\x4f\x70\x74\x69\x6f\x6e\x61\x6c\x20\x61\x72\x67\x73\x20\x6c\x6f\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x30\x29\x20\x61\x6e\x64\x20\x68\x69\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x6c\x65\x6e\x28\x61\x29\x29\x20\x62\x6f\x75\x6e\x64\x20\x74\x68\x65\x0a\x20\x20\x20\x20\x73\x6c\x69\x63\x65\x20\x6f\x66\x20\x61\x20\x74\x6f\x20\x62\x65\x20\x73\x65\x61\x72\x63\x68\x65\x64\x2e\x0a\x20\x20\x20\x20\x22\x22\x22\x0a\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x6f\x20\x3c\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x27\x6c\x6f\x20\x6d\x75\x73\x74\x20\x62\x65\x20\x6e\x6f\x6e\x2d\x6e\x65\x67\x61\x74\x69\x76\x65\x27\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x69\x20\x69\x73\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x69\x20\x3d\x20\x6c\x65\x6e\x28\x61\x29\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6c\x6f\x20\x3c\x20\x68\x69\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6d\x69\x64\x20\x3d\x20\x28\x6c\x6f\x2b\x68\x69\x29\x2f\x2f\x32\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x61\x5b\x6d\x69\x64\x5d\x20\x3c\x20\x78\x3a\x20\x6c\x6f\x20\x3d\x20\x6d\x69\x64\x2b\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x20\x68\x69\x20\x3d\x20\x6d\x69\x64\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x0a\x0a\x23\x20\x43\x72\x65\x61\x74\x65\x20\x61\x6c\x69\x61\x73\x65\x73\x0a\x62\x69\x73\x65\x63\x74\x20\x3d\x20\x62\x69\x73\x65\x63\x74\x5f\x72\x69\x67\x68\x74\x0a\x69\x6e\x73\x6f\x72\x74\x20\x3d\x20\x69\x6e\x73\x6f\x72\x74\x5f\x72\x69\x67\x68\x74\x0a" },
        {"builtins", "\x69\x6d\x70\x6f\x72\x74\x20\x73\x79\x73\x20\x61\x73\x20\x5f\x73\x79\x73\x0a\x69\x6d\x70\x6f\x72\x74\x20\x6f\x70\x65\x72\x61\x74\x6f\x72\x20\x61\x73\x20\x5f\x6f\x70\x65\x72\x61\x74\x6f\x72\x0a\x0a\x64\x65\x66\x20\x70\x72\x69\x6e\x74\x28\x2a\x61\x72\x67\x73\x2c\x20\x73\x65\x70\x3d\x27\x20\x27\x2c\x20\x65\x6e\x64\x3d\x27\x5c\x6e\x27\x29\x3a\x0a\x20\x20\x20\x20\x73\x20\x3d\x20\x73\x65\x70\x2e\x6a\x6f\x69\x6e\x28\x5b\x73\x74\x72\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x61\x72\x67\x73\x5d\x29\x0a\x20\x20\x20\x20\x5f\x73\x79\x73\x2e\x73\x74\x64\x6f\x75\x74\x2e\x77\x72\x69\x74\x65\x28\x73\x20\x2b\x20\x65\x6e\x64\x29\x0a\x0a\x64\x65\x66\x20\x5f\x6d\x69\x6e\x6d\x61\x78\x5f\x72\x65\x64\x75\x63\x65\x28\x6f\x70\x2c\x20\x61\x72\x67\x73\x2c\x20\x6b\x65\x79\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x6b\x65\x79\x20\x69\x73\x20\x4e\x6f\x6e\x65\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x61\x72\x67\x73\x29\x20\x3d\x3d\x20\x32\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x72\x67\x73\x5b\x30\x5d\x20\x69\x66\x20\x6f\x70\x28\x61\x72\x67\x73\x5b\x30\x5d\x2c\x20\x61\x72\x67\x73\x5b\x31\x5d\x29\x20\x65\x6c\x73\x65\x20\x61\x72\x67\x73\x5b\x31\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6b\x65\x79\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x78\x3a\x20\x78\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x61\x72\x67\x73\x29\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x54\x79\x70\x65\x45\x72\x72\x6f\x72\x28\x27\x65\x78\x70\x65\x63\x74\x65\x64\x20\x31\x20\x61\x72\x67\x75\x6d\x65\x6e\x74\x73\x2c\x20\x67\x6f\x74\x20\x30\x27\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x61\x72\x67\x73\x29\x20\x3d\x3d\x20\x31\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x72\x67\x73\x20\x3d\x20\x61\x72\x67\x73\x5b\x30\x5d\x0a\x20\x20\x20\x20\x61\x72\x67\x73\x20\x3d\x20\x69\x74\x65\x72\x28\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x6e\x65\x78\x74\x28\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x72\x65\x73\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x27\x61\x72\x67\x73\x20\x69\x73\x20\x61\x6e\x20\x65\x6d\x70\x74\x79\x20\x73\x65\x71\x75\x65\x6e\x63\x65\x27\x29\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x54\x72\x75\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x20\x3d\x20\x6e\x65\x78\x74\x28\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x72\x65\x61\x6b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x70\x28\x6b\x65\x79\x28\x69\x29\x2c\x20\x6b\x65\x79\x28\x72\x65\x73\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x69\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x6d\x69\x6e\x28\x2a\x61\x72\x67\x73\x2c\x20\x6b\x65\x79\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x6d\x69\x6e\x6d\x61\x78\x5f\x72\x65\x64\x75\x63\x65\x28\x5f\x6f\x70\x65\x72\x61\x74\x6f\x72\x2e\x6c\x74\x2c\x20\x61\x72\x67\x73\x2c\x20\x6b\x65\x79\x29\x0a\x0a\x64\x65\x66\x20\x6d\x61\x78\x28\x2a\x61\x72\x67\x73\x2c\x20\x6b\x65\x79\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x6d\x69\x6e\x6d\x61\x78\x5f\x72\x65\x64\x75\x63\x65\x28\x5f\x6f\x70\x65\x72\x61\x74\x6f\x72\x2e\x67\x74\x2c\x20\x61\x72\x67\x73\x2c\x20\x6b\x65\x79\x29\x0a\x0a\x64\x65\x66\x20\x61\x6c\x6c\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x69\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x54\x72\x75\x65\x0a\x0a\x64\x65\x66\x20\x61\x6e\x79\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x54\x72\x75\x65\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x0a\x64\x65\x66\x20\x65\x6e\x75\x6d\x65\x72\x61\x74\x65\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x2c\x20\x73\x74\x61\x72\x74\x3d\x30\x29\x3a\x0a\x20\x20\x20\x20\x6e\x20\x3d\x20\x73\x74\x61\x72\x74\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x6e\x2c\x20\x65\x6c\x65\x6d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x2b\x2b\x6e\x0a\x0a\x64\x65\x66\x20\x73\x75\x6d\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74" "\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x20\x2b\x3d\x20\x69\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x6d\x61\x70\x28\x66\x2c\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x66\x28\x69\x29\x0a\x0a\x64\x65\x66\x20\x66\x69\x6c\x74\x65\x72\x28\x66\x2c\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x66\x28\x69\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x69\x0a\x0a\x64\x65\x66\x20\x7a\x69\x70\x28\x61\x2c\x20\x62\x29\x3a\x0a\x20\x20\x20\x20\x61\x20\x3d\x20\x69\x74\x65\x72\x28\x61\x29\x0a\x20\x20\x20\x20\x62\x20\x3d\x20\x69\x74\x65\x72\x28\x62\x29\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x54\x72\x75\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x69\x20\x3d\x20\x6e\x65\x78\x74\x28\x61\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x62\x69\x20\x3d\x20\x6e\x65\x78\x74\x28\x62\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x61\x69\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x20\x6f\x72\x20\x62\x69\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x72\x65\x61\x6b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x61\x69\x2c\x20\x62\x69\x0a\x0a\x64\x65\x66\x20\x72\x65\x76\x65\x72\x73\x65\x64\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x61\x20\x3d\x20\x6c\x69\x73\x74\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x0a\x20\x20\x20\x20\x61\x2e\x72\x65\x76\x65\x72\x73\x65\x28\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x0a\x0a\x64\x65\x66\x20\x73\x6f\x72\x74\x65\x64\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x2c\x20\x6b\x65\x79\x3d\x4e\x6f\x6e\x65\x2c\x20\x72\x65\x76\x65\x72\x73\x65\x3d\x46\x61\x6c\x73\x65\x29\x3a\x0a\x20\x20\x20\x20\x61\x20\x3d\x20\x6c\x69\x73\x74\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x0a\x20\x20\x20\x20\x61\x2e\x73\x6f\x72\x74\x28\x6b\x65\x79\x3d\x6b\x65\x79\x2c\x20\x72\x65\x76\x65\x72\x73\x65\x3d\x72\x65\x76\x65\x72\x73\x65\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x0a\x0a\x23\x23\x23\x23\x23\x20\x73\x74\x72\x20\x23\x23\x23\x23\x23\x0a\x64\x65\x66\x20\x5f\x5f\x66\x6f\x72\x6d\x61\x74\x5f\x73\x74\x72\x69\x6e\x67\x28\x73\x65\x6c\x66\x3a\x20\x73\x74\x72\x2c\x20\x2a\x61\x72\x67\x73\x2c\x20\x2a\x2a\x6b\x77\x61\x72\x67\x73\x29\x20\x2d\x3e\x20\x73\x74\x72\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x74\x6f\x6b\x65\x6e\x69\x7a\x65\x53\x74\x72\x69\x6e\x67\x28\x73\x3a\x20\x73\x74\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6f\x6b\x65\x6e\x73\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x4c\x2c\x20\x52\x20\x3d\x20\x30\x2c\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6d\x6f\x64\x65\x20\x3d\x20\x4e\x6f\x6e\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x75\x72\x41\x72\x67\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x6c\x6f\x6f\x6b\x69\x6e\x67\x46\x6f\x72\x4b\x77\x6f\x72\x64\x20\x3d\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x28\x52\x3c\x6c\x65\x6e\x28\x73\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x75\x72\x43\x68\x61\x72\x20\x3d\x20\x73\x5b\x52\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x65\x78\x74\x43\x68\x61\x72\x20\x3d\x20\x73\x5b\x52\x2b\x31\x5d\x20\x69\x66\x20\x52\x2b\x31\x3c\x6c\x65\x6e\x28\x73\x29\x20\x65\x6c\x73\x65\x20\x27\x27\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x49\x6e\x76\x61\x6c\x69\x64\x20\x63\x61\x73\x65\x20\x31\x3a\x20\x73\x74\x72\x61\x79\x20\x27\x7d\x27\x20\x65\x6e\x63\x6f\x75\x6e\x74\x65\x72\x65\x64\x2c\x20\x65\x78\x61\x6d\x70\x6c\x65\x3a\x20\x22\x41\x42\x43\x44\x20\x45\x46\x47\x48\x20\x7b\x6e\x61\x6d\x65\x7d\x20\x49\x4a\x4b\x4c\x7d\x22\x2c\x20\x22\x48\x65\x6c\x6c\x6f\x20\x7b\x76\x76\x7d\x7d\x22\x2c\x20\x22\x48\x45\x4c\x4c\x4f\x20\x7b\x30\x7d\x20\x57\x4f\x52\x4c\x44\x7d\x22\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x63\x75\x72\x43\x68\x61\x72\x20\x3d\x3d\x20\x27\x7d\x27\x20\x61\x6e\x64\x20\x6e\x65\x78\x74\x43\x68\x61\x72\x20\x21\x3d\x20\x27\x7d\x27\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x22\x53\x69\x6e\x67\x6c\x65\x20\x27\x7d\x27\x20\x65\x6e\x63\x6f\x75\x6e\x74\x65\x72\x65\x64\x20\x69\x6e\x20\x66\x6f\x72\x6d\x61\x74\x20\x73\x74\x72\x69\x6e\x67\x22\x29\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x56\x61\x6c\x69\x64\x20\x43\x61\x73\x65\x20\x31\x3a\x20\x45\x73\x63\x61\x70\x69\x6e\x67\x20\x63\x61" "\x73\x65\x2c\x20\x77\x65\x20\x65\x73\x63\x61\x70\x65\x20\x22\x7b\x7b\x20\x6f\x72\x20\x22\x7d\x7d\x22\x20\x74\x6f\x20\x62\x65\x20\x22\x7b\x22\x20\x6f\x72\x20\x22\x7d\x22\x2c\x20\x65\x78\x61\x6d\x70\x6c\x65\x3a\x20\x22\x7b\x7b\x7d\x7d\x22\x2c\x20\x22\x7b\x7b\x4d\x79\x20\x4e\x61\x6d\x65\x20\x69\x73\x20\x7b\x30\x7d\x7d\x7d\x22\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x28\x63\x75\x72\x43\x68\x61\x72\x20\x3d\x3d\x20\x27\x7b\x27\x20\x61\x6e\x64\x20\x6e\x65\x78\x74\x43\x68\x61\x72\x20\x3d\x3d\x20\x27\x7b\x27\x29\x20\x6f\x72\x20\x28\x63\x75\x72\x43\x68\x61\x72\x20\x3d\x3d\x20\x27\x7d\x27\x20\x61\x6e\x64\x20\x6e\x65\x78\x74\x43\x68\x61\x72\x20\x3d\x3d\x20\x27\x7d\x27\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x28\x4c\x3c\x52\x29\x3a\x20\x23\x20\x56\x61\x6c\x69\x64\x20\x43\x61\x73\x65\x20\x31\x2e\x31\x3a\x20\x6d\x61\x6b\x65\x20\x73\x75\x72\x65\x20\x77\x65\x20\x61\x72\x65\x20\x6e\x6f\x74\x20\x61\x64\x64\x69\x6e\x67\x20\x65\x6d\x70\x74\x79\x20\x73\x74\x72\x69\x6e\x67\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6f\x6b\x65\x6e\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x5b\x4c\x3a\x52\x5d\x29\x20\x23\x20\x61\x64\x64\x20\x74\x68\x65\x20\x73\x74\x72\x69\x6e\x67\x20\x62\x65\x66\x6f\x72\x65\x20\x74\x68\x65\x20\x65\x73\x63\x61\x70\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6f\x6b\x65\x6e\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x63\x75\x72\x43\x68\x61\x72\x29\x20\x23\x20\x56\x61\x6c\x69\x64\x20\x43\x61\x73\x65\x20\x31\x2e\x32\x3a\x20\x61\x64\x64\x20\x74\x68\x65\x20\x65\x73\x63\x61\x70\x65\x20\x63\x68\x61\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x4c\x20\x3d\x20\x52\x2b\x32\x20\x23\x20\x6d\x6f\x76\x65\x20\x74\x68\x65\x20\x6c\x65\x66\x74\x20\x70\x6f\x69\x6e\x74\x65\x72\x20\x74\x6f\x20\x74\x68\x65\x20\x6e\x65\x78\x74\x20\x63\x68\x61\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x52\x20\x3d\x20\x52\x2b\x32\x20\x23\x20\x6d\x6f\x76\x65\x20\x74\x68\x65\x20\x72\x69\x67\x68\x74\x20\x70\x6f\x69\x6e\x74\x65\x72\x20\x74\x6f\x20\x74\x68\x65\x20\x6e\x65\x78\x74\x20\x63\x68\x61\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6f\x6e\x74\x69\x6e\x75\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x56\x61\x6c\x69\x64\x20\x43\x61\x73\x65\x20\x32\x3a\x20\x52\x65\x67\x75\x6c\x61\x72\x20\x63\x6f\x6d\x6d\x61\x6e\x64\x20\x6c\x69\x6e\x65\x20\x61\x72\x67\x20\x63\x61\x73\x65\x3a\x20\x65\x78\x61\x6d\x70\x6c\x65\x3a\x20\x20\x22\x41\x42\x43\x44\x20\x45\x46\x47\x48\x20\x7b\x7d\x20\x49\x4a\x4b\x4c\x22\x2c\x20\x22\x7b\x7d\x22\x2c\x20\x22\x48\x45\x4c\x4c\x4f\x20\x7b\x7d\x20\x57\x4f\x52\x4c\x44\x22\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x63\x75\x72\x43\x68\x61\x72\x20\x3d\x3d\x20\x27\x7b\x27\x20\x61\x6e\x64\x20\x6e\x65\x78\x74\x43\x68\x61\x72\x20\x3d\x3d\x20\x27\x7d\x27\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6d\x6f\x64\x65\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x20\x61\x6e\x64\x20\x6d\x6f\x64\x65\x20\x21\x3d\x20\x27\x61\x75\x74\x6f\x27\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x49\x6e\x76\x61\x6c\x69\x64\x20\x63\x61\x73\x65\x20\x32\x3a\x20\x6d\x69\x78\x69\x6e\x67\x20\x61\x75\x74\x6f\x6d\x61\x74\x69\x63\x20\x61\x6e\x64\x20\x6d\x61\x6e\x75\x61\x6c\x20\x66\x69\x65\x6c\x64\x20\x73\x70\x65\x63\x69\x66\x69\x63\x61\x74\x69\x6f\x6e\x73\x20\x2d\x2d\x20\x65\x78\x61\x6d\x70\x6c\x65\x3a\x20\x22\x41\x42\x43\x44\x20\x45\x46\x47\x48\x20\x7b\x6e\x61\x6d\x65\x7d\x20\x49\x4a\x4b\x4c\x20\x7b\x7d\x22\x2c\x20\x22\x48\x65\x6c\x6c\x6f\x20\x7b\x76\x76\x7d\x20\x7b\x7d\x22\x2c\x20\x22\x48\x45\x4c\x4c\x4f\x20\x7b\x30\x7d\x20\x57\x4f\x52\x4c\x44\x20\x7b\x7d\x22\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x22\x43\x61\x6e\x6e\x6f\x74\x20\x73\x77\x69\x74\x63\x68\x20\x66\x72\x6f\x6d\x20\x6d\x61\x6e\x75\x61\x6c\x20\x66\x69\x65\x6c\x64\x20\x6e\x75\x6d\x62\x65\x72\x69\x6e\x67\x20\x74\x6f\x20\x61\x75\x74\x6f\x6d\x61\x74\x69\x63\x20\x66\x69\x65\x6c\x64\x20\x73\x70\x65\x63\x69\x66\x69\x63\x61\x74\x69\x6f\x6e\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6d\x6f\x64\x65\x20\x3d\x20\x27\x61\x75\x74\x6f\x27\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x28\x4c\x3c\x52\x29\x3a\x20\x23\x20\x56\x61\x6c\x69\x64\x20\x43\x61\x73\x65\x20\x32\x2e\x31\x3a\x20\x6d\x61\x6b\x65\x20\x73\x75\x72\x65\x20\x77\x65\x20\x61\x72\x65\x20\x6e\x6f\x74\x20\x61\x64" "\x64\x69\x6e\x67\x20\x65\x6d\x70\x74\x79\x20\x73\x74\x72\x69\x6e\x67\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6f\x6b\x65\x6e\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x5b\x4c\x3a\x52\x5d\x29\x20\x23\x20\x61\x64\x64\x20\x74\x68\x65\x20\x73\x74\x72\x69\x6e\x67\x20\x62\x65\x66\x6f\x72\x65\x20\x74\x68\x65\x20\x73\x70\x65\x63\x69\x61\x6c\x20\x6d\x61\x72\x6b\x65\x72\x20\x66\x6f\x72\x20\x74\x68\x65\x20\x61\x72\x67\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6f\x6b\x65\x6e\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x22\x7b\x22\x2b\x73\x74\x72\x28\x63\x75\x72\x41\x72\x67\x29\x2b\x22\x7d\x22\x29\x20\x23\x20\x56\x61\x6c\x69\x64\x20\x43\x61\x73\x65\x20\x32\x2e\x32\x3a\x20\x61\x64\x64\x20\x74\x68\x65\x20\x73\x70\x65\x63\x69\x61\x6c\x20\x6d\x61\x72\x6b\x65\x72\x20\x66\x6f\x72\x20\x74\x68\x65\x20\x61\x72\x67\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x75\x72\x41\x72\x67\x2b\x3d\x31\x20\x23\x20\x69\x6e\x63\x72\x65\x6d\x65\x6e\x74\x20\x74\x68\x65\x20\x61\x72\x67\x20\x70\x6f\x73\x69\x74\x69\x6f\x6e\x2c\x20\x74\x68\x69\x73\x20\x77\x69\x6c\x6c\x20\x62\x65\x20\x75\x73\x65\x64\x20\x66\x6f\x72\x20\x72\x65\x66\x65\x72\x65\x6e\x63\x69\x6e\x67\x20\x74\x68\x65\x20\x61\x72\x67\x20\x6c\x61\x74\x65\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x4c\x20\x3d\x20\x52\x2b\x32\x20\x23\x20\x6d\x6f\x76\x65\x20\x74\x68\x65\x20\x6c\x65\x66\x74\x20\x70\x6f\x69\x6e\x74\x65\x72\x20\x74\x6f\x20\x74\x68\x65\x20\x6e\x65\x78\x74\x20\x63\x68\x61\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x52\x20\x3d\x20\x52\x2b\x32\x20\x23\x20\x6d\x6f\x76\x65\x20\x74\x68\x65\x20\x72\x69\x67\x68\x74\x20\x70\x6f\x69\x6e\x74\x65\x72\x20\x74\x6f\x20\x74\x68\x65\x20\x6e\x65\x78\x74\x20\x63\x68\x61\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6f\x6e\x74\x69\x6e\x75\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x56\x61\x6c\x69\x64\x20\x43\x61\x73\x65\x20\x33\x3a\x20\x4b\x65\x79\x2d\x77\x6f\x72\x64\x20\x61\x72\x67\x20\x63\x61\x73\x65\x3a\x20\x65\x78\x61\x6d\x70\x6c\x65\x3a\x20\x22\x41\x42\x43\x44\x20\x45\x46\x47\x48\x20\x7b\x6e\x61\x6d\x65\x7d\x20\x49\x4a\x4b\x4c\x22\x2c\x20\x22\x48\x65\x6c\x6c\x6f\x20\x7b\x76\x76\x7d\x22\x2c\x20\x22\x48\x45\x4c\x4c\x4f\x20\x7b\x6e\x61\x6d\x65\x7d\x20\x57\x4f\x52\x4c\x44\x22\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x28\x63\x75\x72\x43\x68\x61\x72\x20\x3d\x3d\x20\x27\x7b\x27\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6d\x6f\x64\x65\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x20\x61\x6e\x64\x20\x6d\x6f\x64\x65\x20\x21\x3d\x20\x27\x6d\x61\x6e\x75\x61\x6c\x27\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x23\x20\x49\x6e\x76\x61\x6c\x69\x64\x20\x63\x61\x73\x65\x20\x32\x3a\x20\x6d\x69\x78\x69\x6e\x67\x20\x61\x75\x74\x6f\x6d\x61\x74\x69\x63\x20\x61\x6e\x64\x20\x6d\x61\x6e\x75\x61\x6c\x20\x66\x69\x65\x6c\x64\x20\x73\x70\x65\x63\x69\x66\x69\x63\x61\x74\x69\x6f\x6e\x73\x20\x2d\x2d\x20\x65\x78\x61\x6d\x70\x6c\x65\x3a\x20\x22\x41\x42\x43\x44\x20\x45\x46\x47\x48\x20\x7b\x7d\x20\x49\x4a\x4b\x4c\x20\x7b\x6e\x61\x6d\x65\x7d\x22\x2c\x20\x22\x48\x65\x6c\x6c\x6f\x20\x7b\x7d\x20\x7b\x31\x7d\x22\x2c\x20\x22\x48\x45\x4c\x4c\x4f\x20\x7b\x7d\x20\x57\x4f\x52\x4c\x44\x20\x7b\x6e\x61\x6d\x65\x7d\x22\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x22\x43\x61\x6e\x6e\x6f\x74\x20\x73\x77\x69\x74\x63\x68\x20\x66\x72\x6f\x6d\x20\x61\x75\x74\x6f\x6d\x61\x74\x69\x63\x20\x66\x69\x65\x6c\x64\x20\x73\x70\x65\x63\x69\x66\x69\x63\x61\x74\x69\x6f\x6e\x20\x74\x6f\x20\x6d\x61\x6e\x75\x61\x6c\x20\x66\x69\x65\x6c\x64\x20\x6e\x75\x6d\x62\x65\x72\x69\x6e\x67\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6d\x6f\x64\x65\x20\x3d\x20\x27\x6d\x61\x6e\x75\x61\x6c\x27\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x28\x4c\x3c\x52\x29\x3a\x20\x23\x20\x56\x61\x6c\x69\x64\x20\x63\x61\x73\x65\x20\x33\x2e\x31\x3a\x20\x6d\x61\x6b\x65\x20\x73\x75\x72\x65\x20\x77\x65\x20\x61\x72\x65\x20\x6e\x6f\x74\x20\x61\x64\x64\x69\x6e\x67\x20\x65\x6d\x70\x74\x79\x20\x73\x74\x72\x69\x6e\x67\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6f\x6b\x65\x6e\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x5b\x4c\x3a\x52\x5d\x29\x20\x23\x20\x61\x64\x64\x20\x74\x68\x65\x20\x73\x74\x72\x69\x6e\x67\x20\x62\x65\x66\x6f\x72" "\x65\x20\x74\x68\x65\x20\x73\x70\x65\x63\x69\x61\x6c\x20\x6d\x61\x72\x6b\x65\x72\x20\x66\x6f\x72\x20\x74\x68\x65\x20\x61\x72\x67\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x57\x65\x20\x6c\x6f\x6f\x6b\x20\x66\x6f\x72\x20\x74\x68\x65\x20\x65\x6e\x64\x20\x6f\x66\x20\x74\x68\x65\x20\x6b\x65\x79\x77\x6f\x72\x64\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6b\x77\x4c\x20\x3d\x20\x52\x20\x23\x20\x4b\x65\x79\x77\x6f\x72\x64\x20\x6c\x65\x66\x74\x20\x70\x6f\x69\x6e\x74\x65\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6b\x77\x52\x20\x3d\x20\x52\x2b\x31\x20\x23\x20\x4b\x65\x79\x77\x6f\x72\x64\x20\x72\x69\x67\x68\x74\x20\x70\x6f\x69\x6e\x74\x65\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x28\x6b\x77\x52\x3c\x6c\x65\x6e\x28\x73\x29\x20\x61\x6e\x64\x20\x73\x5b\x6b\x77\x52\x5d\x21\x3d\x27\x7d\x27\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x5b\x6b\x77\x52\x5d\x20\x3d\x3d\x20\x27\x7b\x27\x3a\x20\x23\x20\x49\x6e\x76\x61\x6c\x69\x64\x20\x63\x61\x73\x65\x20\x33\x3a\x20\x73\x74\x72\x61\x79\x20\x27\x7b\x27\x20\x65\x6e\x63\x6f\x75\x6e\x74\x65\x72\x65\x64\x2c\x20\x65\x78\x61\x6d\x70\x6c\x65\x3a\x20\x22\x41\x42\x43\x44\x20\x45\x46\x47\x48\x20\x7b\x6e\x7b\x61\x6d\x65\x7d\x20\x49\x4a\x4b\x4c\x20\x7b\x22\x2c\x20\x22\x48\x65\x6c\x6c\x6f\x20\x7b\x76\x76\x7b\x7d\x7d\x22\x2c\x20\x22\x48\x45\x4c\x4c\x4f\x20\x7b\x30\x7d\x20\x57\x4f\x52\x7b\x4c\x44\x7d\x22\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x22\x55\x6e\x65\x78\x70\x65\x63\x74\x65\x64\x20\x27\x7b\x27\x20\x69\x6e\x20\x66\x69\x65\x6c\x64\x20\x6e\x61\x6d\x65\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6b\x77\x52\x20\x2b\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x56\x61\x6c\x69\x64\x20\x63\x61\x73\x65\x20\x33\x2e\x32\x3a\x20\x57\x65\x20\x68\x61\x76\x65\x20\x73\x75\x63\x63\x65\x73\x73\x66\x75\x6c\x6c\x79\x20\x66\x6f\x75\x6e\x64\x20\x74\x68\x65\x20\x65\x6e\x64\x20\x6f\x66\x20\x74\x68\x65\x20\x6b\x65\x79\x77\x6f\x72\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6b\x77\x52\x3c\x6c\x65\x6e\x28\x73\x29\x20\x61\x6e\x64\x20\x73\x5b\x6b\x77\x52\x5d\x20\x3d\x3d\x20\x27\x7d\x27\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6f\x6b\x65\x6e\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x5b\x6b\x77\x4c\x3a\x6b\x77\x52\x2b\x31\x5d\x29\x20\x23\x20\x61\x64\x64\x20\x74\x68\x65\x20\x73\x70\x65\x63\x69\x61\x6c\x20\x6d\x61\x72\x6b\x65\x72\x20\x66\x6f\x72\x20\x74\x68\x65\x20\x61\x72\x67\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x4c\x20\x3d\x20\x6b\x77\x52\x2b\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x52\x20\x3d\x20\x6b\x77\x52\x2b\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x49\x6e\x76\x61\x6c\x69\x64\x20\x63\x61\x73\x65\x20\x34\x3a\x20\x57\x65\x20\x64\x69\x64\x6e\x27\x74\x20\x66\x69\x6e\x64\x20\x74\x68\x65\x20\x65\x6e\x64\x20\x6f\x66\x20\x74\x68\x65\x20\x6b\x65\x79\x77\x6f\x72\x64\x2c\x20\x74\x68\x72\x6f\x77\x20\x65\x72\x72\x6f\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x22\x45\x78\x70\x65\x63\x74\x65\x64\x20\x27\x7d\x27\x20\x62\x65\x66\x6f\x72\x65\x20\x65\x6e\x64\x20\x6f\x66\x20\x73\x74\x72\x69\x6e\x67\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6f\x6e\x74\x69\x6e\x75\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x52\x20\x3d\x20\x52\x2b\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x56\x61\x6c\x69\x64\x20\x63\x61\x73\x65\x20\x34\x3a\x20\x57\x65\x20\x68\x61\x76\x65\x20\x72\x65\x61\x63\x68\x65\x64\x20\x74\x68\x65\x20\x65\x6e\x64\x20\x6f\x66\x20\x74\x68\x65\x20\x73\x74\x72\x69\x6e\x67\x2c\x20\x61\x64\x64\x20\x74\x68\x65\x20\x72\x65\x6d\x61\x69\x6e\x69\x6e\x67\x20\x73\x74\x72\x69\x6e\x67\x20\x74\x6f\x20\x74\x68\x65\x20\x74\x6f\x6b\x65\x6e\x73\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x4c\x3c\x52\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6f\x6b\x65\x6e\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x5b\x4c\x3a\x52\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20" "\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x70\x72\x69\x6e\x74\x28\x74\x6f\x6b\x65\x6e\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x74\x6f\x6b\x65\x6e\x73\x0a\x0a\x20\x20\x20\x20\x74\x6f\x6b\x65\x6e\x73\x20\x3d\x20\x74\x6f\x6b\x65\x6e\x69\x7a\x65\x53\x74\x72\x69\x6e\x67\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x61\x72\x67\x4d\x61\x70\x20\x3d\x20\x7b\x7d\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x2c\x20\x61\x20\x69\x6e\x20\x65\x6e\x75\x6d\x65\x72\x61\x74\x65\x28\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x72\x67\x4d\x61\x70\x5b\x73\x74\x72\x28\x69\x29\x5d\x20\x3d\x20\x61\x0a\x20\x20\x20\x20\x66\x69\x6e\x61\x6c\x5f\x74\x6f\x6b\x65\x6e\x73\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x74\x20\x69\x6e\x20\x74\x6f\x6b\x65\x6e\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x5b\x30\x5d\x20\x3d\x3d\x20\x27\x7b\x27\x20\x61\x6e\x64\x20\x74\x5b\x2d\x31\x5d\x20\x3d\x3d\x20\x27\x7d\x27\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6b\x65\x79\x20\x3d\x20\x74\x5b\x31\x3a\x2d\x31\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x72\x67\x4d\x61\x70\x56\x61\x6c\x20\x3d\x20\x61\x72\x67\x4d\x61\x70\x2e\x67\x65\x74\x28\x6b\x65\x79\x2c\x20\x4e\x6f\x6e\x65\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6b\x77\x61\x72\x67\x73\x56\x61\x6c\x20\x3d\x20\x6b\x77\x61\x72\x67\x73\x2e\x67\x65\x74\x28\x6b\x65\x79\x2c\x20\x4e\x6f\x6e\x65\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x61\x72\x67\x4d\x61\x70\x56\x61\x6c\x20\x69\x73\x20\x4e\x6f\x6e\x65\x20\x61\x6e\x64\x20\x6b\x77\x61\x72\x67\x73\x56\x61\x6c\x20\x69\x73\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x22\x4e\x6f\x20\x61\x72\x67\x20\x66\x6f\x75\x6e\x64\x20\x66\x6f\x72\x20\x74\x6f\x6b\x65\x6e\x3a\x20\x22\x2b\x74\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x61\x72\x67\x4d\x61\x70\x56\x61\x6c\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x69\x6e\x61\x6c\x5f\x74\x6f\x6b\x65\x6e\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x74\x72\x28\x61\x72\x67\x4d\x61\x70\x56\x61\x6c\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x69\x6e\x61\x6c\x5f\x74\x6f\x6b\x65\x6e\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x74\x72\x28\x6b\x77\x61\x72\x67\x73\x56\x61\x6c\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x69\x6e\x61\x6c\x5f\x74\x6f\x6b\x65\x6e\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x74\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x27\x2e\x6a\x6f\x69\x6e\x28\x66\x69\x6e\x61\x6c\x5f\x74\x6f\x6b\x65\x6e\x73\x29\x0a\x0a\x73\x74\x72\x2e\x66\x6f\x72\x6d\x61\x74\x20\x3d\x20\x5f\x5f\x66\x6f\x72\x6d\x61\x74\x5f\x73\x74\x72\x69\x6e\x67\x0a\x64\x65\x6c\x20\x5f\x5f\x66\x6f\x72\x6d\x61\x74\x5f\x73\x74\x72\x69\x6e\x67\x0a\x0a\x0a\x64\x65\x66\x20\x68\x65\x6c\x70\x28\x6f\x62\x6a\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x61\x73\x61\x74\x74\x72\x28\x6f\x62\x6a\x2c\x20\x27\x5f\x5f\x66\x75\x6e\x63\x5f\x5f\x27\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x62\x6a\x20\x3d\x20\x6f\x62\x6a\x2e\x5f\x5f\x66\x75\x6e\x63\x5f\x5f\x0a\x20\x20\x20\x20\x70\x72\x69\x6e\x74\x28\x6f\x62\x6a\x2e\x5f\x5f\x73\x69\x67\x6e\x61\x74\x75\x72\x65\x5f\x5f\x29\x0a\x20\x20\x20\x20\x70\x72\x69\x6e\x74\x28\x6f\x62\x6a\x2e\x5f\x5f\x64\x6f\x63\x5f\x5f\x29\x0a\x0a\x64\x65\x66\x20\x63\x6f\x6d\x70\x6c\x65\x78\x28\x2a\x61\x72\x67\x73\x2c\x20\x2a\x2a\x6b\x77\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x69\x6d\x70\x6f\x72\x74\x20\x63\x6d\x61\x74\x68\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x63\x6d\x61\x74\x68\x2e\x63\x6f\x6d\x70\x6c\x65\x78\x28\x2a\x61\x72\x67\x73\x2c\x20\x2a\x2a\x6b\x77\x61\x72\x67\x73\x29\x0a\x0a\x64\x65\x66\x20\x6c\x6f\x6e\x67\x28\x2a\x61\x72\x67\x73\x2c\x20\x2a\x2a\x6b\x77\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x69\x6d\x70\x6f\x72\x74\x20\x5f\x6c\x6f\x6e\x67\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x6c\x6f\x6e\x67\x2e\x6c\x6f\x6e\x67\x28\x2a\x61\x72\x67\x73\x2c\x20\x2a\x2a\x6b\x77\x61\x72\x67\x73\x29\x0a\x0a\x0a\x23\x20\x62\x75\x69\x6c\x74\x69\x6e\x20\x65\x78\x63\x65\x70\x74\x69\x6f\x6e\x73\x0a\x63\x6c\x61\x73\x73\x20\x53\x74\x61\x63\x6b\x4f\x76\x65\x72\x66\x6c\x6f\x77\x45\x72\x72\x6f\x72\x28\x45\x78\x63\x65\x70\x74\x69\x6f\x6e\x29\x3a\x20\x70\x61\x73\x73\x0a\x63\x6c\x61\x73\x73\x20\x49\x4f\x45\x72\x72\x6f\x72\x28\x45\x78\x63\x65\x70\x74\x69\x6f\x6e\x29\x3a\x20\x70\x61\x73\x73\x0a\x63\x6c\x61\x73\x73\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x45\x72\x72\x6f\x72\x28\x45\x78\x63" "\x65\x70\x74\x69\x6f\x6e\x29\x3a\x20\x70\x61\x73\x73\x0a\x63\x6c\x61\x73\x73\x20\x54\x79\x70\x65\x45\x72\x72\x6f\x72\x28\x45\x78\x63\x65\x70\x74\x69\x6f\x6e\x29\x3a\x20\x70\x61\x73\x73\x0a\x63\x6c\x61\x73\x73\x20\x49\x6e\x64\x65\x78\x45\x72\x72\x6f\x72\x28\x45\x78\x63\x65\x70\x74\x69\x6f\x6e\x29\x3a\x20\x70\x61\x73\x73\x0a\x63\x6c\x61\x73\x73\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x45\x78\x63\x65\x70\x74\x69\x6f\x6e\x29\x3a\x20\x70\x61\x73\x73\x0a\x63\x6c\x61\x73\x73\x20\x52\x75\x6e\x74\x69\x6d\x65\x45\x72\x72\x6f\x72\x28\x45\x78\x63\x65\x70\x74\x69\x6f\x6e\x29\x3a\x20\x70\x61\x73\x73\x0a\x63\x6c\x61\x73\x73\x20\x5a\x65\x72\x6f\x44\x69\x76\x69\x73\x69\x6f\x6e\x45\x72\x72\x6f\x72\x28\x45\x78\x63\x65\x70\x74\x69\x6f\x6e\x29\x3a\x20\x70\x61\x73\x73\x0a\x63\x6c\x61\x73\x73\x20\x4e\x61\x6d\x65\x45\x72\x72\x6f\x72\x28\x45\x78\x63\x65\x70\x74\x69\x6f\x6e\x29\x3a\x20\x70\x61\x73\x73\x0a\x63\x6c\x61\x73\x73\x20\x55\x6e\x62\x6f\x75\x6e\x64\x4c\x6f\x63\x61\x6c\x45\x72\x72\x6f\x72\x28\x45\x78\x63\x65\x70\x74\x69\x6f\x6e\x29\x3a\x20\x70\x61\x73\x73\x0a\x63\x6c\x61\x73\x73\x20\x41\x74\x74\x72\x69\x62\x75\x74\x65\x45\x72\x72\x6f\x72\x28\x45\x78\x63\x65\x70\x74\x69\x6f\x6e\x29\x3a\x20\x70\x61\x73\x73\x0a\x63\x6c\x61\x73\x73\x20\x49\x6d\x70\x6f\x72\x74\x45\x72\x72\x6f\x72\x28\x45\x78\x63\x65\x70\x74\x69\x6f\x6e\x29\x3a\x20\x70\x61\x73\x73\x0a\x63\x6c\x61\x73\x73\x20\x41\x73\x73\x65\x72\x74\x69\x6f\x6e\x45\x72\x72\x6f\x72\x28\x45\x78\x63\x65\x70\x74\x69\x6f\x6e\x29\x3a\x20\x70\x61\x73\x73\x0a\x0a\x63\x6c\x61\x73\x73\x20\x4b\x65\x79\x45\x72\x72\x6f\x72\x28\x45\x78\x63\x65\x70\x74\x69\x6f\x6e\x29\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x3d\x2e\x2e\x2e\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x6b\x65\x79\x20\x3d\x20\x6b\x65\x79\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6b\x65\x79\x20\x69\x73\x20\x2e\x2e\x2e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x75\x70\x65\x72\x28\x29\x2e\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x75\x70\x65\x72\x28\x29\x2e\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x72\x65\x70\x72\x28\x6b\x65\x79\x29\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x73\x74\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x6b\x65\x79\x20\x69\x73\x20\x2e\x2e\x2e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x27\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x74\x72\x28\x73\x65\x6c\x66\x2e\x6b\x65\x79\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x6b\x65\x79\x20\x69\x73\x20\x2e\x2e\x2e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x4b\x65\x79\x45\x72\x72\x6f\x72\x28\x29\x27\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x27\x4b\x65\x79\x45\x72\x72\x6f\x72\x28\x7b\x73\x65\x6c\x66\x2e\x6b\x65\x79\x21\x72\x7d\x29\x27\x0a" },
        {"cmath", "\x69\x6d\x70\x6f\x72\x74\x20\x6d\x61\x74\x68\x0a\x0a\x63\x6c\x61\x73\x73\x20\x63\x6f\x6d\x70\x6c\x65\x78\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x72\x65\x61\x6c\x2c\x20\x69\x6d\x61\x67\x3d\x30\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x72\x65\x61\x6c\x20\x3d\x20\x66\x6c\x6f\x61\x74\x28\x72\x65\x61\x6c\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x69\x6d\x61\x67\x20\x3d\x20\x66\x6c\x6f\x61\x74\x28\x69\x6d\x61\x67\x29\x0a\x0a\x20\x20\x20\x20\x40\x70\x72\x6f\x70\x65\x72\x74\x79\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x72\x65\x61\x6c\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x72\x65\x61\x6c\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x40\x70\x72\x6f\x70\x65\x72\x74\x79\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x6d\x61\x67\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x69\x6d\x61\x67\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6f\x6e\x6a\x75\x67\x61\x74\x65\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x63\x6f\x6d\x70\x6c\x65\x78\x28\x73\x65\x6c\x66\x2e\x72\x65\x61\x6c\x2c\x20\x2d\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x20\x3d\x20\x5b\x27\x28\x27\x2c\x20\x73\x74\x72\x28\x73\x65\x6c\x66\x2e\x72\x65\x61\x6c\x29\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x20\x3e\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x27\x2b\x27\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x74\x72\x28\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x27\x6a\x29\x27\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x27\x2e\x6a\x6f\x69\x6e\x28\x73\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x63\x6f\x6d\x70\x6c\x65\x78\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x72\x65\x61\x6c\x20\x3d\x3d\x20\x6f\x74\x68\x65\x72\x2e\x72\x65\x61\x6c\x20\x61\x6e\x64\x20\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x20\x3d\x3d\x20\x6f\x74\x68\x65\x72\x2e\x69\x6d\x61\x67\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x6e\x20\x28\x69\x6e\x74\x2c\x20\x66\x6c\x6f\x61\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x72\x65\x61\x6c\x20\x3d\x3d\x20\x6f\x74\x68\x65\x72\x20\x61\x6e\x64\x20\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x20\x3d\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x61\x64\x64\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x63\x6f\x6d\x70\x6c\x65\x78\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x63\x6f\x6d\x70\x6c\x65\x78\x28\x73\x65\x6c\x66\x2e\x72\x65\x61\x6c\x20\x2b\x20\x6f\x74\x68\x65\x72\x2e\x72\x65\x61\x6c\x2c\x20\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x20\x2b\x20\x6f\x74\x68\x65\x72\x2e\x69\x6d\x61\x67\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x6e\x20\x28\x69\x6e\x74\x2c\x20\x66\x6c\x6f\x61\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x63\x6f\x6d\x70\x6c\x65\x78\x28\x73\x65\x6c\x66\x2e\x72\x65\x61\x6c\x20\x2b\x20\x6f\x74\x68\x65\x72\x2c\x20\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x61\x64\x64\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x61\x64\x64\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x63\x6f\x6d\x70\x6c\x65\x78\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x63\x6f\x6d\x70\x6c\x65\x78\x28\x73\x65\x6c\x66\x2e\x72\x65" "\x61\x6c\x20\x2d\x20\x6f\x74\x68\x65\x72\x2e\x72\x65\x61\x6c\x2c\x20\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x20\x2d\x20\x6f\x74\x68\x65\x72\x2e\x69\x6d\x61\x67\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x6e\x20\x28\x69\x6e\x74\x2c\x20\x66\x6c\x6f\x61\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x63\x6f\x6d\x70\x6c\x65\x78\x28\x73\x65\x6c\x66\x2e\x72\x65\x61\x6c\x20\x2d\x20\x6f\x74\x68\x65\x72\x2c\x20\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x63\x6f\x6d\x70\x6c\x65\x78\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x63\x6f\x6d\x70\x6c\x65\x78\x28\x6f\x74\x68\x65\x72\x2e\x72\x65\x61\x6c\x20\x2d\x20\x73\x65\x6c\x66\x2e\x72\x65\x61\x6c\x2c\x20\x6f\x74\x68\x65\x72\x2e\x69\x6d\x61\x67\x20\x2d\x20\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x6e\x20\x28\x69\x6e\x74\x2c\x20\x66\x6c\x6f\x61\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x63\x6f\x6d\x70\x6c\x65\x78\x28\x6f\x74\x68\x65\x72\x20\x2d\x20\x73\x65\x6c\x66\x2e\x72\x65\x61\x6c\x2c\x20\x2d\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6d\x75\x6c\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x63\x6f\x6d\x70\x6c\x65\x78\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x63\x6f\x6d\x70\x6c\x65\x78\x28\x73\x65\x6c\x66\x2e\x72\x65\x61\x6c\x20\x2a\x20\x6f\x74\x68\x65\x72\x2e\x72\x65\x61\x6c\x20\x2d\x20\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x20\x2a\x20\x6f\x74\x68\x65\x72\x2e\x69\x6d\x61\x67\x2c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x72\x65\x61\x6c\x20\x2a\x20\x6f\x74\x68\x65\x72\x2e\x69\x6d\x61\x67\x20\x2b\x20\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x20\x2a\x20\x6f\x74\x68\x65\x72\x2e\x72\x65\x61\x6c\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x6e\x20\x28\x69\x6e\x74\x2c\x20\x66\x6c\x6f\x61\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x63\x6f\x6d\x70\x6c\x65\x78\x28\x73\x65\x6c\x66\x2e\x72\x65\x61\x6c\x20\x2a\x20\x6f\x74\x68\x65\x72\x2c\x20\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x20\x2a\x20\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x6d\x75\x6c\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x6d\x75\x6c\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x70\x6f\x77\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x69\x6e\x74\x20\x7c\x20\x66\x6c\x6f\x61\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x6e\x20\x28\x69\x6e\x74\x2c\x20\x66\x6c\x6f\x61\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x63\x6f\x6d\x70\x6c\x65\x78\x28\x73\x65\x6c\x66\x2e\x5f\x5f\x61\x62\x73\x5f\x5f\x28\x29\x20\x2a\x2a\x20\x6f\x74\x68\x65\x72\x20\x2a\x20\x6d\x61\x74\x68\x2e\x63\x6f\x73\x28\x6f\x74\x68\x65\x72\x20\x2a\x20\x70\x68\x61\x73\x65\x28\x73\x65\x6c\x66\x29\x29\x2c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x61\x62\x73\x5f\x5f\x28\x29\x20\x2a\x2a\x20\x6f\x74\x68\x65\x72\x20\x2a\x20\x6d\x61\x74\x68\x2e\x73\x69\x6e\x28\x6f\x74\x68\x65\x72\x20\x2a\x20\x70\x68\x61\x73\x65\x28\x73\x65\x6c\x66\x29\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x61\x62\x73\x5f\x5f\x28\x73\x65\x6c\x66\x29\x20\x2d\x3e\x20\x66\x6c\x6f\x61\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6d\x61\x74\x68\x2e\x73\x71\x72\x74\x28\x73\x65\x6c\x66\x2e\x72\x65\x61\x6c\x20\x2a\x2a\x20\x32\x20\x2b\x20\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x20\x2a\x2a\x20\x32\x29" "\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6e\x65\x67\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x63\x6f\x6d\x70\x6c\x65\x78\x28\x2d\x73\x65\x6c\x66\x2e\x72\x65\x61\x6c\x2c\x20\x2d\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x68\x61\x73\x68\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x68\x61\x73\x68\x28\x28\x73\x65\x6c\x66\x2e\x72\x65\x61\x6c\x2c\x20\x73\x65\x6c\x66\x2e\x69\x6d\x61\x67\x29\x29\x0a\x0a\x0a\x23\x20\x43\x6f\x6e\x76\x65\x72\x73\x69\x6f\x6e\x73\x20\x74\x6f\x20\x61\x6e\x64\x20\x66\x72\x6f\x6d\x20\x70\x6f\x6c\x61\x72\x20\x63\x6f\x6f\x72\x64\x69\x6e\x61\x74\x65\x73\x0a\x0a\x64\x65\x66\x20\x70\x68\x61\x73\x65\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6d\x61\x74\x68\x2e\x61\x74\x61\x6e\x32\x28\x7a\x2e\x69\x6d\x61\x67\x2c\x20\x7a\x2e\x72\x65\x61\x6c\x29\x0a\x0a\x64\x65\x66\x20\x70\x6f\x6c\x61\x72\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x7a\x2e\x5f\x5f\x61\x62\x73\x5f\x5f\x28\x29\x2c\x20\x70\x68\x61\x73\x65\x28\x7a\x29\x0a\x0a\x64\x65\x66\x20\x72\x65\x63\x74\x28\x72\x3a\x20\x66\x6c\x6f\x61\x74\x2c\x20\x70\x68\x69\x3a\x20\x66\x6c\x6f\x61\x74\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x20\x2a\x20\x6d\x61\x74\x68\x2e\x63\x6f\x73\x28\x70\x68\x69\x29\x20\x2b\x20\x72\x20\x2a\x20\x6d\x61\x74\x68\x2e\x73\x69\x6e\x28\x70\x68\x69\x29\x20\x2a\x20\x31\x6a\x0a\x0a\x23\x20\x50\x6f\x77\x65\x72\x20\x61\x6e\x64\x20\x6c\x6f\x67\x61\x72\x69\x74\x68\x6d\x69\x63\x20\x66\x75\x6e\x63\x74\x69\x6f\x6e\x73\x0a\x0a\x64\x65\x66\x20\x65\x78\x70\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6d\x61\x74\x68\x2e\x65\x78\x70\x28\x7a\x2e\x72\x65\x61\x6c\x29\x20\x2a\x20\x72\x65\x63\x74\x28\x31\x2c\x20\x7a\x2e\x69\x6d\x61\x67\x29\x0a\x0a\x64\x65\x66\x20\x6c\x6f\x67\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x2c\x20\x62\x61\x73\x65\x3d\x32\x2e\x37\x31\x38\x32\x38\x31\x38\x32\x38\x34\x35\x39\x30\x34\x35\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6d\x61\x74\x68\x2e\x6c\x6f\x67\x28\x7a\x2e\x5f\x5f\x61\x62\x73\x5f\x5f\x28\x29\x2c\x20\x62\x61\x73\x65\x29\x20\x2b\x20\x70\x68\x61\x73\x65\x28\x7a\x29\x20\x2a\x20\x31\x6a\x0a\x0a\x64\x65\x66\x20\x6c\x6f\x67\x31\x30\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x67\x28\x7a\x2c\x20\x31\x30\x29\x0a\x0a\x64\x65\x66\x20\x73\x71\x72\x74\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x7a\x20\x2a\x2a\x20\x30\x2e\x35\x0a\x0a\x23\x20\x54\x72\x69\x67\x6f\x6e\x6f\x6d\x65\x74\x72\x69\x63\x20\x66\x75\x6e\x63\x74\x69\x6f\x6e\x73\x0a\x0a\x64\x65\x66\x20\x61\x63\x6f\x73\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x2d\x31\x6a\x20\x2a\x20\x6c\x6f\x67\x28\x7a\x20\x2b\x20\x73\x71\x72\x74\x28\x7a\x20\x2a\x20\x7a\x20\x2d\x20\x31\x29\x29\x0a\x0a\x64\x65\x66\x20\x61\x73\x69\x6e\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x2d\x31\x6a\x20\x2a\x20\x6c\x6f\x67\x28\x31\x6a\x20\x2a\x20\x7a\x20\x2b\x20\x73\x71\x72\x74\x28\x31\x20\x2d\x20\x7a\x20\x2a\x20\x7a\x29\x29\x0a\x0a\x64\x65\x66\x20\x61\x74\x61\x6e\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x31\x6a\x20\x2f\x20\x32\x20\x2a\x20\x6c\x6f\x67\x28\x28\x31\x20\x2d\x20\x31\x6a\x20\x2a\x20\x7a\x29\x20\x2f\x20\x28\x31\x20\x2b\x20\x31\x6a\x20\x2a\x20\x7a\x29\x29\x0a\x0a\x64\x65\x66\x20\x63\x6f\x73\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x65\x78\x70\x28\x7a\x29\x20\x2b\x20\x65\x78\x70\x28\x2d\x7a\x29\x29\x20\x2f\x20\x32\x0a\x0a\x64\x65\x66\x20\x73\x69\x6e\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x65\x78\x70\x28\x7a\x29\x20\x2d\x20\x65\x78\x70\x28\x2d\x7a\x29\x29\x20\x2f\x20\x28\x32\x20\x2a\x20\x31\x6a\x29\x0a\x0a\x64\x65\x66\x20\x74\x61\x6e\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x69\x6e\x28\x7a\x29\x20\x2f\x20\x63\x6f\x73\x28\x7a\x29\x0a\x0a\x23\x20\x48\x79\x70\x65\x72\x62\x6f\x6c\x69\x63\x20\x66\x75\x6e\x63\x74\x69\x6f\x6e\x73\x0a\x0a\x64\x65\x66\x20\x61\x63\x6f\x73\x68\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x67\x28\x7a\x20\x2b\x20\x73\x71\x72\x74\x28\x7a\x20\x2a\x20\x7a\x20\x2d\x20\x31\x29\x29\x0a\x0a\x64\x65\x66\x20\x61\x73\x69\x6e\x68\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x67\x28\x7a\x20\x2b\x20\x73\x71\x72\x74\x28\x7a\x20\x2a\x20\x7a\x20\x2b\x20\x31\x29\x29\x0a\x0a\x64\x65\x66\x20\x61\x74\x61\x6e\x68" "\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x31\x20\x2f\x20\x32\x20\x2a\x20\x6c\x6f\x67\x28\x28\x31\x20\x2b\x20\x7a\x29\x20\x2f\x20\x28\x31\x20\x2d\x20\x7a\x29\x29\x0a\x0a\x64\x65\x66\x20\x63\x6f\x73\x68\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x65\x78\x70\x28\x7a\x29\x20\x2b\x20\x65\x78\x70\x28\x2d\x7a\x29\x29\x20\x2f\x20\x32\x0a\x0a\x64\x65\x66\x20\x73\x69\x6e\x68\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x65\x78\x70\x28\x7a\x29\x20\x2d\x20\x65\x78\x70\x28\x2d\x7a\x29\x29\x20\x2f\x20\x32\x0a\x0a\x64\x65\x66\x20\x74\x61\x6e\x68\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x69\x6e\x68\x28\x7a\x29\x20\x2f\x20\x63\x6f\x73\x68\x28\x7a\x29\x0a\x0a\x23\x20\x43\x6c\x61\x73\x73\x69\x66\x69\x63\x61\x74\x69\x6f\x6e\x20\x66\x75\x6e\x63\x74\x69\x6f\x6e\x73\x0a\x0a\x64\x65\x66\x20\x69\x73\x66\x69\x6e\x69\x74\x65\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6d\x61\x74\x68\x2e\x69\x73\x66\x69\x6e\x69\x74\x65\x28\x7a\x2e\x72\x65\x61\x6c\x29\x20\x61\x6e\x64\x20\x6d\x61\x74\x68\x2e\x69\x73\x66\x69\x6e\x69\x74\x65\x28\x7a\x2e\x69\x6d\x61\x67\x29\x0a\x0a\x64\x65\x66\x20\x69\x73\x69\x6e\x66\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6d\x61\x74\x68\x2e\x69\x73\x69\x6e\x66\x28\x7a\x2e\x72\x65\x61\x6c\x29\x20\x6f\x72\x20\x6d\x61\x74\x68\x2e\x69\x73\x69\x6e\x66\x28\x7a\x2e\x69\x6d\x61\x67\x29\x0a\x0a\x64\x65\x66\x20\x69\x73\x6e\x61\x6e\x28\x7a\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6d\x61\x74\x68\x2e\x69\x73\x6e\x61\x6e\x28\x7a\x2e\x72\x65\x61\x6c\x29\x20\x6f\x72\x20\x6d\x61\x74\x68\x2e\x69\x73\x6e\x61\x6e\x28\x7a\x2e\x69\x6d\x61\x67\x29\x0a\x0a\x64\x65\x66\x20\x69\x73\x63\x6c\x6f\x73\x65\x28\x61\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x2c\x20\x62\x3a\x20\x63\x6f\x6d\x70\x6c\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6d\x61\x74\x68\x2e\x69\x73\x63\x6c\x6f\x73\x65\x28\x61\x2e\x72\x65\x61\x6c\x2c\x20\x62\x2e\x72\x65\x61\x6c\x29\x20\x61\x6e\x64\x20\x6d\x61\x74\x68\x2e\x69\x73\x63\x6c\x6f\x73\x65\x28\x61\x2e\x69\x6d\x61\x67\x2c\x20\x62\x2e\x69\x6d\x61\x67\x29\x0a\x0a\x23\x20\x43\x6f\x6e\x73\x74\x61\x6e\x74\x73\x0a\x0a\x70\x69\x20\x3d\x20\x6d\x61\x74\x68\x2e\x70\x69\x0a\x65\x20\x3d\x20\x6d\x61\x74\x68\x2e\x65\x0a\x74\x61\x75\x20\x3d\x20\x32\x20\x2a\x20\x70\x69\x0a\x69\x6e\x66\x20\x3d\x20\x6d\x61\x74\x68\x2e\x69\x6e\x66\x0a\x69\x6e\x66\x6a\x20\x3d\x20\x63\x6f\x6d\x70\x6c\x65\x78\x28\x30\x2c\x20\x69\x6e\x66\x29\x0a\x6e\x61\x6e\x20\x3d\x20\x6d\x61\x74\x68\x2e\x6e\x61\x6e\x0a\x6e\x61\x6e\x6a\x20\x3d\x20\x63\x6f\x6d\x70\x6c\x65\x78\x28\x30\x2c\x20\x6e\x61\x6e\x29\x0a" },
        {"collections", "\x64\x65\x66\x20\x43\x6f\x75\x6e\x74\x65\x72\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x61\x20\x3d\x20\x7b\x7d\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x78\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x78\x20\x69\x6e\x20\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x5b\x78\x5d\x20\x2b\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x5b\x78\x5d\x20\x3d\x20\x31\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x0a\x0a\x63\x6c\x61\x73\x73\x20\x64\x65\x66\x61\x75\x6c\x74\x64\x69\x63\x74\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x29\x20\x2d\x3e\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x20\x3d\x20\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x20\x3d\x20\x7b\x7d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x65\x74\x69\x74\x65\x6d\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6b\x65\x79\x20\x6e\x6f\x74\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x6b\x65\x79\x5d\x20\x3d\x20\x73\x65\x6c\x66\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x6b\x65\x79\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x73\x65\x74\x69\x74\x65\x6d\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x2c\x20\x76\x61\x6c\x75\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x6b\x65\x79\x5d\x20\x3d\x20\x76\x61\x6c\x75\x65\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x64\x65\x6c\x69\x74\x65\x6d\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x64\x65\x6c\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x6b\x65\x79\x5d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x20\x2d\x3e\x20\x73\x74\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x22\x64\x65\x66\x61\x75\x6c\x74\x64\x69\x63\x74\x28\x7b\x73\x65\x6c\x66\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x7d\x2c\x20\x7b\x73\x65\x6c\x66\x2e\x5f\x61\x7d\x29\x22\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x5f\x5f\x6f\x3a\x20\x6f\x62\x6a\x65\x63\x74\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x69\x73\x69\x6e\x73\x74\x61\x6e\x63\x65\x28\x5f\x5f\x6f\x2c\x20\x64\x65\x66\x61\x75\x6c\x74\x64\x69\x63\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x20\x21\x3d\x20\x5f\x5f\x6f\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x20\x3d\x3d\x20\x5f\x5f\x6f\x2e\x5f\x61\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x74\x65\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x74\x65\x72\x28\x73\x65\x6c\x66\x2e\x5f\x61\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x63\x6f\x6e\x74\x61\x69\x6e\x73\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6b\x65\x79\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x2e\x5f\x61\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x6b\x65\x79\x73\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x6b\x65\x79\x73\x28\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x76\x61\x6c\x75\x65\x73\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x76\x61\x6c\x75\x65\x73\x28\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x74\x65\x6d\x73\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x69\x74\x65\x6d\x73\x28\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x70\x6f\x70\x28\x73\x65\x6c" "\x66\x2c\x20\x2a\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x70\x6f\x70\x28\x2a\x61\x72\x67\x73\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6c\x65\x61\x72\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x63\x6c\x65\x61\x72\x28\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6f\x70\x79\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x65\x77\x5f\x64\x64\x20\x3d\x20\x64\x65\x66\x61\x75\x6c\x74\x64\x69\x63\x74\x28\x73\x65\x6c\x66\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x65\x77\x5f\x64\x64\x2e\x5f\x61\x20\x3d\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x63\x6f\x70\x79\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6e\x65\x77\x5f\x64\x64\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x67\x65\x74\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x2c\x20\x64\x65\x66\x61\x75\x6c\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x67\x65\x74\x28\x6b\x65\x79\x2c\x20\x64\x65\x66\x61\x75\x6c\x74\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x75\x70\x64\x61\x74\x65\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x75\x70\x64\x61\x74\x65\x28\x6f\x74\x68\x65\x72\x29\x0a" },
        {"colorsys", "\x22\x22\x22\x43\x6f\x6e\x76\x65\x72\x73\x69\x6f\x6e\x20\x66\x75\x6e\x63\x74\x69\x6f\x6e\x73\x20\x62\x65\x74\x77\x65\x65\x6e\x20\x52\x47\x42\x20\x61\x6e\x64\x20\x6f\x74\x68\x65\x72\x20\x63\x6f\x6c\x6f\x72\x20\x73\x79\x73\x74\x65\x6d\x73\x2e\x0a\x0a\x54\x68\x69\x73\x20\x6d\x6f\x64\x75\x6c\x65\x73\x20\x70\x72\x6f\x76\x69\x64\x65\x73\x20\x74\x77\x6f\x20\x66\x75\x6e\x63\x74\x69\x6f\x6e\x73\x20\x66\x6f\x72\x20\x65\x61\x63\x68\x20\x63\x6f\x6c\x6f\x72\x20\x73\x79\x73\x74\x65\x6d\x20\x41\x42\x43\x3a\x0a\x0a\x20\x20\x72\x67\x62\x5f\x74\x6f\x5f\x61\x62\x63\x28\x72\x2c\x20\x67\x2c\x20\x62\x29\x20\x2d\x2d\x3e\x20\x61\x2c\x20\x62\x2c\x20\x63\x0a\x20\x20\x61\x62\x63\x5f\x74\x6f\x5f\x72\x67\x62\x28\x61\x2c\x20\x62\x2c\x20\x63\x29\x20\x2d\x2d\x3e\x20\x72\x2c\x20\x67\x2c\x20\x62\x0a\x0a\x41\x6c\x6c\x20\x69\x6e\x70\x75\x74\x73\x20\x61\x6e\x64\x20\x6f\x75\x74\x70\x75\x74\x73\x20\x61\x72\x65\x20\x74\x72\x69\x70\x6c\x65\x73\x20\x6f\x66\x20\x66\x6c\x6f\x61\x74\x73\x20\x69\x6e\x20\x74\x68\x65\x20\x72\x61\x6e\x67\x65\x20\x5b\x30\x2e\x30\x2e\x2e\x2e\x31\x2e\x30\x5d\x0a\x28\x77\x69\x74\x68\x20\x74\x68\x65\x20\x65\x78\x63\x65\x70\x74\x69\x6f\x6e\x20\x6f\x66\x20\x49\x20\x61\x6e\x64\x20\x51\x2c\x20\x77\x68\x69\x63\x68\x20\x63\x6f\x76\x65\x72\x73\x20\x61\x20\x73\x6c\x69\x67\x68\x74\x6c\x79\x20\x6c\x61\x72\x67\x65\x72\x20\x72\x61\x6e\x67\x65\x29\x2e\x0a\x49\x6e\x70\x75\x74\x73\x20\x6f\x75\x74\x73\x69\x64\x65\x20\x74\x68\x65\x20\x76\x61\x6c\x69\x64\x20\x72\x61\x6e\x67\x65\x20\x6d\x61\x79\x20\x63\x61\x75\x73\x65\x20\x65\x78\x63\x65\x70\x74\x69\x6f\x6e\x73\x20\x6f\x72\x20\x69\x6e\x76\x61\x6c\x69\x64\x20\x6f\x75\x74\x70\x75\x74\x73\x2e\x0a\x0a\x53\x75\x70\x70\x6f\x72\x74\x65\x64\x20\x63\x6f\x6c\x6f\x72\x20\x73\x79\x73\x74\x65\x6d\x73\x3a\x0a\x52\x47\x42\x3a\x20\x52\x65\x64\x2c\x20\x47\x72\x65\x65\x6e\x2c\x20\x42\x6c\x75\x65\x20\x63\x6f\x6d\x70\x6f\x6e\x65\x6e\x74\x73\x0a\x59\x49\x51\x3a\x20\x4c\x75\x6d\x69\x6e\x61\x6e\x63\x65\x2c\x20\x43\x68\x72\x6f\x6d\x69\x6e\x61\x6e\x63\x65\x20\x28\x75\x73\x65\x64\x20\x62\x79\x20\x63\x6f\x6d\x70\x6f\x73\x69\x74\x65\x20\x76\x69\x64\x65\x6f\x20\x73\x69\x67\x6e\x61\x6c\x73\x29\x0a\x48\x4c\x53\x3a\x20\x48\x75\x65\x2c\x20\x4c\x75\x6d\x69\x6e\x61\x6e\x63\x65\x2c\x20\x53\x61\x74\x75\x72\x61\x74\x69\x6f\x6e\x0a\x48\x53\x56\x3a\x20\x48\x75\x65\x2c\x20\x53\x61\x74\x75\x72\x61\x74\x69\x6f\x6e\x2c\x20\x56\x61\x6c\x75\x65\x0a\x22\x22\x22\x0a\x0a\x23\x20\x52\x65\x66\x65\x72\x65\x6e\x63\x65\x73\x3a\x0a\x23\x20\x68\x74\x74\x70\x3a\x2f\x2f\x65\x6e\x2e\x77\x69\x6b\x69\x70\x65\x64\x69\x61\x2e\x6f\x72\x67\x2f\x77\x69\x6b\x69\x2f\x59\x49\x51\x0a\x23\x20\x68\x74\x74\x70\x3a\x2f\x2f\x65\x6e\x2e\x77\x69\x6b\x69\x70\x65\x64\x69\x61\x2e\x6f\x72\x67\x2f\x77\x69\x6b\x69\x2f\x48\x4c\x53\x5f\x63\x6f\x6c\x6f\x72\x5f\x73\x70\x61\x63\x65\x0a\x23\x20\x68\x74\x74\x70\x3a\x2f\x2f\x65\x6e\x2e\x77\x69\x6b\x69\x70\x65\x64\x69\x61\x2e\x6f\x72\x67\x2f\x77\x69\x6b\x69\x2f\x48\x53\x56\x5f\x63\x6f\x6c\x6f\x72\x5f\x73\x70\x61\x63\x65\x0a\x0a\x5f\x5f\x61\x6c\x6c\x5f\x5f\x20\x3d\x20\x5b\x22\x72\x67\x62\x5f\x74\x6f\x5f\x79\x69\x71\x22\x2c\x22\x79\x69\x71\x5f\x74\x6f\x5f\x72\x67\x62\x22\x2c\x22\x72\x67\x62\x5f\x74\x6f\x5f\x68\x6c\x73\x22\x2c\x22\x68\x6c\x73\x5f\x74\x6f\x5f\x72\x67\x62\x22\x2c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x22\x72\x67\x62\x5f\x74\x6f\x5f\x68\x73\x76\x22\x2c\x22\x68\x73\x76\x5f\x74\x6f\x5f\x72\x67\x62\x22\x5d\x0a\x0a\x23\x20\x53\x6f\x6d\x65\x20\x66\x6c\x6f\x61\x74\x69\x6e\x67\x20\x70\x6f\x69\x6e\x74\x20\x63\x6f\x6e\x73\x74\x61\x6e\x74\x73\x0a\x0a\x4f\x4e\x45\x5f\x54\x48\x49\x52\x44\x20\x3d\x20\x31\x2e\x30\x2f\x33\x2e\x30\x0a\x4f\x4e\x45\x5f\x53\x49\x58\x54\x48\x20\x3d\x20\x31\x2e\x30\x2f\x36\x2e\x30\x0a\x54\x57\x4f\x5f\x54\x48\x49\x52\x44\x20\x3d\x20\x32\x2e\x30\x2f\x33\x2e\x30\x0a\x0a\x23\x20\x59\x49\x51\x3a\x20\x75\x73\x65\x64\x20\x62\x79\x20\x63\x6f\x6d\x70\x6f\x73\x69\x74\x65\x20\x76\x69\x64\x65\x6f\x20\x73\x69\x67\x6e\x61\x6c\x73\x20\x28\x6c\x69\x6e\x65\x61\x72\x20\x63\x6f\x6d\x62\x69\x6e\x61\x74\x69\x6f\x6e\x73\x20\x6f\x66\x20\x52\x47\x42\x29\x0a\x23\x20\x59\x3a\x20\x70\x65\x72\x63\x65\x69\x76\x65\x64\x20\x67\x72\x65\x79\x20\x6c\x65\x76\x65\x6c\x20\x28\x30\x2e\x30\x20\x3d\x3d\x20\x62\x6c\x61\x63\x6b\x2c\x20\x31\x2e\x30\x20\x3d\x3d\x20\x77\x68\x69\x74\x65\x29\x0a\x23\x20\x49\x2c\x20\x51\x3a\x20\x63\x6f\x6c\x6f\x72\x20\x63\x6f\x6d\x70\x6f\x6e\x65\x6e\x74\x73\x0a\x23\x0a\x23\x20\x54\x68\x65\x72\x65\x20\x61\x72\x65\x20\x61\x20\x67\x72\x65\x61\x74\x20\x6d\x61\x6e\x79\x20\x76\x65\x72\x73\x69\x6f\x6e\x73\x20\x6f\x66\x20\x74\x68\x65\x20\x63\x6f\x6e\x73\x74\x61\x6e\x74\x73\x20\x75\x73\x65\x64\x20\x69\x6e\x20\x74\x68\x65\x73\x65\x20\x66\x6f\x72\x6d\x75\x6c\x61\x65\x2e\x0a\x23\x20\x54\x68\x65\x20\x6f\x6e\x65\x73\x20\x69\x6e\x20\x74\x68\x69\x73\x20\x6c\x69\x62\x72\x61\x72\x79\x20\x75\x73\x65\x73\x20\x63\x6f\x6e\x73\x74\x61\x6e\x74\x73\x20\x66\x72\x6f\x6d\x20\x74\x68\x65\x20\x46\x43\x43\x20\x76\x65\x72\x73\x69\x6f\x6e\x20\x6f\x66\x20\x4e\x54\x53\x43\x2e\x0a\x0a\x64\x65\x66\x20\x72\x67\x62\x5f\x74\x6f\x5f\x79\x69\x71\x28\x72\x2c" "\x20\x67\x2c\x20\x62\x29\x3a\x0a\x20\x20\x20\x20\x79\x20\x3d\x20\x30\x2e\x33\x30\x2a\x72\x20\x2b\x20\x30\x2e\x35\x39\x2a\x67\x20\x2b\x20\x30\x2e\x31\x31\x2a\x62\x0a\x20\x20\x20\x20\x69\x20\x3d\x20\x30\x2e\x37\x34\x2a\x28\x72\x2d\x79\x29\x20\x2d\x20\x30\x2e\x32\x37\x2a\x28\x62\x2d\x79\x29\x0a\x20\x20\x20\x20\x71\x20\x3d\x20\x30\x2e\x34\x38\x2a\x28\x72\x2d\x79\x29\x20\x2b\x20\x30\x2e\x34\x31\x2a\x28\x62\x2d\x79\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x79\x2c\x20\x69\x2c\x20\x71\x29\x0a\x0a\x64\x65\x66\x20\x79\x69\x71\x5f\x74\x6f\x5f\x72\x67\x62\x28\x79\x2c\x20\x69\x2c\x20\x71\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x72\x20\x3d\x20\x79\x20\x2b\x20\x28\x30\x2e\x32\x37\x2a\x71\x20\x2b\x20\x30\x2e\x34\x31\x2a\x69\x29\x20\x2f\x20\x28\x30\x2e\x37\x34\x2a\x30\x2e\x34\x31\x20\x2b\x20\x30\x2e\x32\x37\x2a\x30\x2e\x34\x38\x29\x0a\x20\x20\x20\x20\x23\x20\x62\x20\x3d\x20\x79\x20\x2b\x20\x28\x30\x2e\x37\x34\x2a\x71\x20\x2d\x20\x30\x2e\x34\x38\x2a\x69\x29\x20\x2f\x20\x28\x30\x2e\x37\x34\x2a\x30\x2e\x34\x31\x20\x2b\x20\x30\x2e\x32\x37\x2a\x30\x2e\x34\x38\x29\x0a\x20\x20\x20\x20\x23\x20\x67\x20\x3d\x20\x79\x20\x2d\x20\x28\x30\x2e\x33\x30\x2a\x28\x72\x2d\x79\x29\x20\x2b\x20\x30\x2e\x31\x31\x2a\x28\x62\x2d\x79\x29\x29\x20\x2f\x20\x30\x2e\x35\x39\x0a\x0a\x20\x20\x20\x20\x72\x20\x3d\x20\x79\x20\x2b\x20\x30\x2e\x39\x34\x36\x38\x38\x32\x32\x31\x37\x30\x39\x30\x30\x36\x39\x33\x2a\x69\x20\x2b\x20\x30\x2e\x36\x32\x33\x35\x35\x36\x35\x38\x31\x39\x38\x36\x31\x34\x33\x33\x2a\x71\x0a\x20\x20\x20\x20\x67\x20\x3d\x20\x79\x20\x2d\x20\x30\x2e\x32\x37\x34\x37\x38\x37\x36\x34\x36\x32\x39\x38\x39\x37\x38\x33\x34\x2a\x69\x20\x2d\x20\x30\x2e\x36\x33\x35\x36\x39\x31\x30\x37\x39\x31\x38\x37\x33\x38\x30\x31\x2a\x71\x0a\x20\x20\x20\x20\x62\x20\x3d\x20\x79\x20\x2d\x20\x31\x2e\x31\x30\x38\x35\x34\x35\x30\x33\x34\x36\x34\x32\x30\x33\x32\x32\x2a\x69\x20\x2b\x20\x31\x2e\x37\x30\x39\x30\x30\x36\x39\x32\x38\x34\x30\x36\x34\x36\x36\x36\x2a\x71\x0a\x0a\x20\x20\x20\x20\x69\x66\x20\x72\x20\x3c\x20\x30\x2e\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x20\x3d\x20\x30\x2e\x30\x0a\x20\x20\x20\x20\x69\x66\x20\x67\x20\x3c\x20\x30\x2e\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x67\x20\x3d\x20\x30\x2e\x30\x0a\x20\x20\x20\x20\x69\x66\x20\x62\x20\x3c\x20\x30\x2e\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x62\x20\x3d\x20\x30\x2e\x30\x0a\x20\x20\x20\x20\x69\x66\x20\x72\x20\x3e\x20\x31\x2e\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x20\x3d\x20\x31\x2e\x30\x0a\x20\x20\x20\x20\x69\x66\x20\x67\x20\x3e\x20\x31\x2e\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x67\x20\x3d\x20\x31\x2e\x30\x0a\x20\x20\x20\x20\x69\x66\x20\x62\x20\x3e\x20\x31\x2e\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x62\x20\x3d\x20\x31\x2e\x30\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x72\x2c\x20\x67\x2c\x20\x62\x29\x0a\x0a\x0a\x23\x20\x48\x4c\x53\x3a\x20\x48\x75\x65\x2c\x20\x4c\x75\x6d\x69\x6e\x61\x6e\x63\x65\x2c\x20\x53\x61\x74\x75\x72\x61\x74\x69\x6f\x6e\x0a\x23\x20\x48\x3a\x20\x70\x6f\x73\x69\x74\x69\x6f\x6e\x20\x69\x6e\x20\x74\x68\x65\x20\x73\x70\x65\x63\x74\x72\x75\x6d\x0a\x23\x20\x4c\x3a\x20\x63\x6f\x6c\x6f\x72\x20\x6c\x69\x67\x68\x74\x6e\x65\x73\x73\x0a\x23\x20\x53\x3a\x20\x63\x6f\x6c\x6f\x72\x20\x73\x61\x74\x75\x72\x61\x74\x69\x6f\x6e\x0a\x0a\x64\x65\x66\x20\x72\x67\x62\x5f\x74\x6f\x5f\x68\x6c\x73\x28\x72\x2c\x20\x67\x2c\x20\x62\x29\x3a\x0a\x20\x20\x20\x20\x6d\x61\x78\x63\x20\x3d\x20\x6d\x61\x78\x28\x72\x2c\x20\x67\x2c\x20\x62\x29\x0a\x20\x20\x20\x20\x6d\x69\x6e\x63\x20\x3d\x20\x6d\x69\x6e\x28\x72\x2c\x20\x67\x2c\x20\x62\x29\x0a\x20\x20\x20\x20\x73\x75\x6d\x63\x20\x3d\x20\x28\x6d\x61\x78\x63\x2b\x6d\x69\x6e\x63\x29\x0a\x20\x20\x20\x20\x72\x61\x6e\x67\x65\x63\x20\x3d\x20\x28\x6d\x61\x78\x63\x2d\x6d\x69\x6e\x63\x29\x0a\x20\x20\x20\x20\x6c\x20\x3d\x20\x73\x75\x6d\x63\x2f\x32\x2e\x30\x0a\x20\x20\x20\x20\x69\x66\x20\x6d\x69\x6e\x63\x20\x3d\x3d\x20\x6d\x61\x78\x63\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x30\x2e\x30\x2c\x20\x6c\x2c\x20\x30\x2e\x30\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x20\x3c\x3d\x20\x30\x2e\x35\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x20\x3d\x20\x72\x61\x6e\x67\x65\x63\x20\x2f\x20\x73\x75\x6d\x63\x0a\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x20\x3d\x20\x72\x61\x6e\x67\x65\x63\x20\x2f\x20\x28\x32\x2e\x30\x2d\x6d\x61\x78\x63\x2d\x6d\x69\x6e\x63\x29\x20\x20\x23\x20\x4e\x6f\x74\x20\x61\x6c\x77\x61\x79\x73\x20\x32\x2e\x30\x2d\x73\x75\x6d\x63\x3a\x20\x67\x68\x2d\x31\x30\x36\x34\x39\x38\x2e\x0a\x20\x20\x20\x20\x72\x63\x20\x3d\x20\x28\x6d\x61\x78\x63\x2d\x72\x29\x20\x2f\x20\x72\x61\x6e\x67\x65\x63\x0a\x20\x20\x20\x20\x67\x63\x20\x3d\x20\x28\x6d\x61\x78\x63\x2d\x67\x29\x20\x2f\x20\x72\x61\x6e\x67\x65\x63\x0a\x20\x20\x20\x20\x62\x63\x20\x3d\x20\x28\x6d\x61\x78\x63\x2d\x62\x29\x20\x2f\x20\x72\x61\x6e\x67\x65\x63\x0a\x20\x20\x20\x20\x69\x66\x20\x72\x20\x3d\x3d\x20\x6d\x61\x78\x63\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x20\x3d\x20\x62\x63\x2d\x67\x63\x0a\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x67\x20\x3d\x3d" "\x20\x6d\x61\x78\x63\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x20\x3d\x20\x32\x2e\x30\x2b\x72\x63\x2d\x62\x63\x0a\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x20\x3d\x20\x34\x2e\x30\x2b\x67\x63\x2d\x72\x63\x0a\x20\x20\x20\x20\x23\x20\x68\x20\x3d\x20\x28\x68\x2f\x36\x2e\x30\x29\x20\x25\x20\x31\x2e\x30\x0a\x20\x20\x20\x20\x68\x20\x3d\x20\x68\x20\x2f\x20\x36\x2e\x30\x0a\x20\x20\x20\x20\x68\x20\x3d\x20\x68\x20\x2d\x20\x69\x6e\x74\x28\x68\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x68\x2c\x20\x6c\x2c\x20\x73\x0a\x0a\x64\x65\x66\x20\x68\x6c\x73\x5f\x74\x6f\x5f\x72\x67\x62\x28\x68\x2c\x20\x6c\x2c\x20\x73\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x73\x20\x3d\x3d\x20\x30\x2e\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x2c\x20\x6c\x2c\x20\x6c\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x20\x3c\x3d\x20\x30\x2e\x35\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6d\x32\x20\x3d\x20\x6c\x20\x2a\x20\x28\x31\x2e\x30\x2b\x73\x29\x0a\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6d\x32\x20\x3d\x20\x6c\x2b\x73\x2d\x28\x6c\x2a\x73\x29\x0a\x20\x20\x20\x20\x6d\x31\x20\x3d\x20\x32\x2e\x30\x2a\x6c\x20\x2d\x20\x6d\x32\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x5f\x76\x28\x6d\x31\x2c\x20\x6d\x32\x2c\x20\x68\x2b\x4f\x4e\x45\x5f\x54\x48\x49\x52\x44\x29\x2c\x20\x5f\x76\x28\x6d\x31\x2c\x20\x6d\x32\x2c\x20\x68\x29\x2c\x20\x5f\x76\x28\x6d\x31\x2c\x20\x6d\x32\x2c\x20\x68\x2d\x4f\x4e\x45\x5f\x54\x48\x49\x52\x44\x29\x29\x0a\x0a\x64\x65\x66\x20\x5f\x76\x28\x6d\x31\x2c\x20\x6d\x32\x2c\x20\x68\x75\x65\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x68\x75\x65\x20\x3d\x20\x68\x75\x65\x20\x25\x20\x31\x2e\x30\x0a\x20\x20\x20\x20\x68\x75\x65\x20\x3d\x20\x68\x75\x65\x20\x2d\x20\x69\x6e\x74\x28\x68\x75\x65\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x75\x65\x20\x3c\x20\x4f\x4e\x45\x5f\x53\x49\x58\x54\x48\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6d\x31\x20\x2b\x20\x28\x6d\x32\x2d\x6d\x31\x29\x2a\x68\x75\x65\x2a\x36\x2e\x30\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x75\x65\x20\x3c\x20\x30\x2e\x35\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6d\x32\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x75\x65\x20\x3c\x20\x54\x57\x4f\x5f\x54\x48\x49\x52\x44\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6d\x31\x20\x2b\x20\x28\x6d\x32\x2d\x6d\x31\x29\x2a\x28\x54\x57\x4f\x5f\x54\x48\x49\x52\x44\x2d\x68\x75\x65\x29\x2a\x36\x2e\x30\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6d\x31\x0a\x0a\x0a\x23\x20\x48\x53\x56\x3a\x20\x48\x75\x65\x2c\x20\x53\x61\x74\x75\x72\x61\x74\x69\x6f\x6e\x2c\x20\x56\x61\x6c\x75\x65\x0a\x23\x20\x48\x3a\x20\x70\x6f\x73\x69\x74\x69\x6f\x6e\x20\x69\x6e\x20\x74\x68\x65\x20\x73\x70\x65\x63\x74\x72\x75\x6d\x0a\x23\x20\x53\x3a\x20\x63\x6f\x6c\x6f\x72\x20\x73\x61\x74\x75\x72\x61\x74\x69\x6f\x6e\x20\x28\x22\x70\x75\x72\x69\x74\x79\x22\x29\x0a\x23\x20\x56\x3a\x20\x63\x6f\x6c\x6f\x72\x20\x62\x72\x69\x67\x68\x74\x6e\x65\x73\x73\x0a\x0a\x64\x65\x66\x20\x72\x67\x62\x5f\x74\x6f\x5f\x68\x73\x76\x28\x72\x2c\x20\x67\x2c\x20\x62\x29\x3a\x0a\x20\x20\x20\x20\x6d\x61\x78\x63\x20\x3d\x20\x6d\x61\x78\x28\x72\x2c\x20\x67\x2c\x20\x62\x29\x0a\x20\x20\x20\x20\x6d\x69\x6e\x63\x20\x3d\x20\x6d\x69\x6e\x28\x72\x2c\x20\x67\x2c\x20\x62\x29\x0a\x20\x20\x20\x20\x72\x61\x6e\x67\x65\x63\x20\x3d\x20\x28\x6d\x61\x78\x63\x2d\x6d\x69\x6e\x63\x29\x0a\x20\x20\x20\x20\x76\x20\x3d\x20\x6d\x61\x78\x63\x0a\x20\x20\x20\x20\x69\x66\x20\x6d\x69\x6e\x63\x20\x3d\x3d\x20\x6d\x61\x78\x63\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x30\x2e\x30\x2c\x20\x30\x2e\x30\x2c\x20\x76\x0a\x20\x20\x20\x20\x73\x20\x3d\x20\x72\x61\x6e\x67\x65\x63\x20\x2f\x20\x6d\x61\x78\x63\x0a\x20\x20\x20\x20\x72\x63\x20\x3d\x20\x28\x6d\x61\x78\x63\x2d\x72\x29\x20\x2f\x20\x72\x61\x6e\x67\x65\x63\x0a\x20\x20\x20\x20\x67\x63\x20\x3d\x20\x28\x6d\x61\x78\x63\x2d\x67\x29\x20\x2f\x20\x72\x61\x6e\x67\x65\x63\x0a\x20\x20\x20\x20\x62\x63\x20\x3d\x20\x28\x6d\x61\x78\x63\x2d\x62\x29\x20\x2f\x20\x72\x61\x6e\x67\x65\x63\x0a\x20\x20\x20\x20\x69\x66\x20\x72\x20\x3d\x3d\x20\x6d\x61\x78\x63\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x20\x3d\x20\x62\x63\x2d\x67\x63\x0a\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x67\x20\x3d\x3d\x20\x6d\x61\x78\x63\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x20\x3d\x20\x32\x2e\x30\x2b\x72\x63\x2d\x62\x63\x0a\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x20\x3d\x20\x34\x2e\x30\x2b\x67\x63\x2d\x72\x63\x0a\x20\x20\x20\x20\x23\x20\x68\x20\x3d\x20\x28\x68\x2f\x36\x2e\x30\x29\x20\x25\x20\x31\x2e\x30\x0a\x20\x20\x20\x20\x68\x20\x3d\x20\x68\x20\x2f\x20\x36\x2e\x30\x0a\x20\x20\x20\x20\x68\x20\x3d\x20\x68\x20\x2d\x20\x69\x6e\x74\x28\x68\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x68\x2c\x20\x73\x2c\x20\x76\x0a\x0a\x64\x65\x66\x20\x68\x73\x76\x5f\x74\x6f\x5f\x72\x67\x62\x28\x68\x2c\x20\x73\x2c\x20\x76\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x73\x20\x3d\x3d\x20\x30\x2e\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74" "\x75\x72\x6e\x20\x76\x2c\x20\x76\x2c\x20\x76\x0a\x20\x20\x20\x20\x69\x20\x3d\x20\x69\x6e\x74\x28\x68\x2a\x36\x2e\x30\x29\x20\x23\x20\x58\x58\x58\x20\x61\x73\x73\x75\x6d\x65\x20\x69\x6e\x74\x28\x29\x20\x74\x72\x75\x6e\x63\x61\x74\x65\x73\x21\x0a\x20\x20\x20\x20\x66\x20\x3d\x20\x28\x68\x2a\x36\x2e\x30\x29\x20\x2d\x20\x69\x0a\x20\x20\x20\x20\x70\x20\x3d\x20\x76\x2a\x28\x31\x2e\x30\x20\x2d\x20\x73\x29\x0a\x20\x20\x20\x20\x71\x20\x3d\x20\x76\x2a\x28\x31\x2e\x30\x20\x2d\x20\x73\x2a\x66\x29\x0a\x20\x20\x20\x20\x74\x20\x3d\x20\x76\x2a\x28\x31\x2e\x30\x20\x2d\x20\x73\x2a\x28\x31\x2e\x30\x2d\x66\x29\x29\x0a\x20\x20\x20\x20\x69\x20\x3d\x20\x69\x25\x36\x0a\x20\x20\x20\x20\x69\x66\x20\x69\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x76\x2c\x20\x74\x2c\x20\x70\x0a\x20\x20\x20\x20\x69\x66\x20\x69\x20\x3d\x3d\x20\x31\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x71\x2c\x20\x76\x2c\x20\x70\x0a\x20\x20\x20\x20\x69\x66\x20\x69\x20\x3d\x3d\x20\x32\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x70\x2c\x20\x76\x2c\x20\x74\x0a\x20\x20\x20\x20\x69\x66\x20\x69\x20\x3d\x3d\x20\x33\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x70\x2c\x20\x71\x2c\x20\x76\x0a\x20\x20\x20\x20\x69\x66\x20\x69\x20\x3d\x3d\x20\x34\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x74\x2c\x20\x70\x2c\x20\x76\x0a\x20\x20\x20\x20\x69\x66\x20\x69\x20\x3d\x3d\x20\x35\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x76\x2c\x20\x70\x2c\x20\x71\x0a\x20\x20\x20\x20\x23\x20\x43\x61\x6e\x6e\x6f\x74\x20\x67\x65\x74\x20\x68\x65\x72\x65" },
        {"datetime", "\x66\x72\x6f\x6d\x20\x74\x69\x6d\x65\x20\x69\x6d\x70\x6f\x72\x74\x20\x6c\x6f\x63\x61\x6c\x74\x69\x6d\x65\x0a\x0a\x63\x6c\x61\x73\x73\x20\x74\x69\x6d\x65\x64\x65\x6c\x74\x61\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x64\x61\x79\x73\x3d\x30\x2c\x20\x73\x65\x63\x6f\x6e\x64\x73\x3d\x30\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x64\x61\x79\x73\x20\x3d\x20\x64\x61\x79\x73\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x65\x63\x6f\x6e\x64\x73\x20\x3d\x20\x73\x65\x63\x6f\x6e\x64\x73\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x22\x64\x61\x74\x65\x74\x69\x6d\x65\x2e\x74\x69\x6d\x65\x64\x65\x6c\x74\x61\x28\x64\x61\x79\x73\x3d\x7b\x73\x65\x6c\x66\x2e\x64\x61\x79\x73\x7d\x2c\x20\x73\x65\x63\x6f\x6e\x64\x73\x3d\x7b\x73\x65\x6c\x66\x2e\x73\x65\x63\x6f\x6e\x64\x73\x7d\x29\x22\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x27\x74\x69\x6d\x65\x64\x65\x6c\x74\x61\x27\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x74\x69\x6d\x65\x64\x65\x6c\x74\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x73\x65\x6c\x66\x2e\x64\x61\x79\x73\x2c\x20\x73\x65\x6c\x66\x2e\x73\x65\x63\x6f\x6e\x64\x73\x29\x20\x3d\x3d\x20\x28\x6f\x74\x68\x65\x72\x2e\x64\x61\x79\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x73\x65\x63\x6f\x6e\x64\x73\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x27\x74\x69\x6d\x65\x64\x65\x6c\x74\x61\x27\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x74\x69\x6d\x65\x64\x65\x6c\x74\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x73\x65\x6c\x66\x2e\x64\x61\x79\x73\x2c\x20\x73\x65\x6c\x66\x2e\x73\x65\x63\x6f\x6e\x64\x73\x29\x20\x3c\x20\x28\x6f\x74\x68\x65\x72\x2e\x64\x61\x79\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x73\x65\x63\x6f\x6e\x64\x73\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x27\x74\x69\x6d\x65\x64\x65\x6c\x74\x61\x27\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x74\x69\x6d\x65\x64\x65\x6c\x74\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x73\x65\x6c\x66\x2e\x64\x61\x79\x73\x2c\x20\x73\x65\x6c\x66\x2e\x73\x65\x63\x6f\x6e\x64\x73\x29\x20\x3c\x3d\x20\x28\x6f\x74\x68\x65\x72\x2e\x64\x61\x79\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x73\x65\x63\x6f\x6e\x64\x73\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x27\x74\x69\x6d\x65\x64\x65\x6c\x74\x61\x27\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x74\x69\x6d\x65\x64\x65\x6c\x74\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x73\x65\x6c\x66\x2e\x64\x61\x79\x73\x2c\x20\x73\x65\x6c\x66\x2e\x73\x65\x63\x6f\x6e\x64\x73\x29\x20\x3e\x20\x28\x6f\x74\x68\x65\x72\x2e\x64\x61\x79\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x73\x65\x63\x6f\x6e\x64\x73\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x65\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x27\x74\x69\x6d\x65\x64\x65\x6c\x74\x61\x27\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x74\x69\x6d\x65\x64\x65\x6c\x74\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x73\x65\x6c\x66\x2e\x64\x61\x79\x73\x2c\x20\x73\x65\x6c\x66\x2e\x73\x65\x63\x6f\x6e\x64\x73\x29\x20\x3e\x3d\x20\x28\x6f\x74\x68\x65\x72\x2e\x64\x61\x79\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x73\x65\x63\x6f\x6e\x64\x73\x29\x0a\x0a\x0a\x63\x6c\x61\x73\x73\x20\x64\x61\x74\x65\x3a\x0a" "\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x79\x65\x61\x72\x3a\x20\x69\x6e\x74\x2c\x20\x6d\x6f\x6e\x74\x68\x3a\x20\x69\x6e\x74\x2c\x20\x64\x61\x79\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x20\x3d\x20\x79\x65\x61\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x20\x3d\x20\x6d\x6f\x6e\x74\x68\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x64\x61\x79\x20\x3d\x20\x64\x61\x79\x0a\x0a\x20\x20\x20\x20\x40\x73\x74\x61\x74\x69\x63\x6d\x65\x74\x68\x6f\x64\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x74\x6f\x64\x61\x79\x28\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x74\x20\x3d\x20\x6c\x6f\x63\x61\x6c\x74\x69\x6d\x65\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x64\x61\x74\x65\x28\x74\x2e\x74\x6d\x5f\x79\x65\x61\x72\x2c\x20\x74\x2e\x74\x6d\x5f\x6d\x6f\x6e\x2c\x20\x74\x2e\x74\x6d\x5f\x6d\x64\x61\x79\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x27\x64\x61\x74\x65\x27\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x64\x61\x74\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x73\x65\x6c\x66\x2e\x64\x61\x79\x29\x20\x3d\x3d\x20\x28\x6f\x74\x68\x65\x72\x2e\x79\x65\x61\x72\x2c\x20\x6f\x74\x68\x65\x72\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x61\x79\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x27\x64\x61\x74\x65\x27\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x64\x61\x74\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x73\x65\x6c\x66\x2e\x64\x61\x79\x29\x20\x3c\x20\x28\x6f\x74\x68\x65\x72\x2e\x79\x65\x61\x72\x2c\x20\x6f\x74\x68\x65\x72\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x61\x79\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x27\x64\x61\x74\x65\x27\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x64\x61\x74\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x73\x65\x6c\x66\x2e\x64\x61\x79\x29\x20\x3c\x3d\x20\x28\x6f\x74\x68\x65\x72\x2e\x79\x65\x61\x72\x2c\x20\x6f\x74\x68\x65\x72\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x61\x79\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x27\x64\x61\x74\x65\x27\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x64\x61\x74\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x73\x65\x6c\x66\x2e\x64\x61\x79\x29\x20\x3e\x20\x28\x6f\x74\x68\x65\x72\x2e\x79\x65\x61\x72\x2c\x20\x6f\x74\x68\x65\x72\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x61\x79\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x65\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x27\x64\x61\x74\x65\x27\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x64\x61\x74\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x73\x65\x6c\x66\x2e\x64\x61\x79\x29\x20\x3e\x3d\x20\x28\x6f\x74\x68\x65\x72\x2e\x79\x65\x61\x72\x2c\x20\x6f\x74\x68\x65\x72\x2e\x6d" "\x6f\x6e\x74\x68\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x61\x79\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x73\x74\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x22\x7b\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x7d\x2d\x7b\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x3a\x30\x32\x7d\x2d\x7b\x73\x65\x6c\x66\x2e\x64\x61\x79\x3a\x30\x32\x7d\x22\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x22\x64\x61\x74\x65\x74\x69\x6d\x65\x2e\x64\x61\x74\x65\x28\x7b\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x7d\x2c\x20\x7b\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x7d\x2c\x20\x7b\x73\x65\x6c\x66\x2e\x64\x61\x79\x7d\x29\x22\x0a\x0a\x0a\x63\x6c\x61\x73\x73\x20\x64\x61\x74\x65\x74\x69\x6d\x65\x28\x64\x61\x74\x65\x29\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x79\x65\x61\x72\x3a\x20\x69\x6e\x74\x2c\x20\x6d\x6f\x6e\x74\x68\x3a\x20\x69\x6e\x74\x2c\x20\x64\x61\x79\x3a\x20\x69\x6e\x74\x2c\x20\x68\x6f\x75\x72\x3a\x20\x69\x6e\x74\x2c\x20\x6d\x69\x6e\x75\x74\x65\x3a\x20\x69\x6e\x74\x2c\x20\x73\x65\x63\x6f\x6e\x64\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x75\x70\x65\x72\x28\x29\x2e\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x79\x65\x61\x72\x2c\x20\x6d\x6f\x6e\x74\x68\x2c\x20\x64\x61\x79\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x56\x61\x6c\x69\x64\x61\x74\x65\x20\x61\x6e\x64\x20\x73\x65\x74\x20\x68\x6f\x75\x72\x2c\x20\x6d\x69\x6e\x75\x74\x65\x2c\x20\x61\x6e\x64\x20\x73\x65\x63\x6f\x6e\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x30\x20\x3c\x3d\x20\x68\x6f\x75\x72\x20\x3c\x3d\x20\x32\x33\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x22\x48\x6f\x75\x72\x20\x6d\x75\x73\x74\x20\x62\x65\x20\x62\x65\x74\x77\x65\x65\x6e\x20\x30\x20\x61\x6e\x64\x20\x32\x33\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x68\x6f\x75\x72\x20\x3d\x20\x68\x6f\x75\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x30\x20\x3c\x3d\x20\x6d\x69\x6e\x75\x74\x65\x20\x3c\x3d\x20\x35\x39\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x22\x4d\x69\x6e\x75\x74\x65\x20\x6d\x75\x73\x74\x20\x62\x65\x20\x62\x65\x74\x77\x65\x65\x6e\x20\x30\x20\x61\x6e\x64\x20\x35\x39\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x6d\x69\x6e\x75\x74\x65\x20\x3d\x20\x6d\x69\x6e\x75\x74\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x30\x20\x3c\x3d\x20\x73\x65\x63\x6f\x6e\x64\x20\x3c\x3d\x20\x35\x39\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x22\x53\x65\x63\x6f\x6e\x64\x20\x6d\x75\x73\x74\x20\x62\x65\x20\x62\x65\x74\x77\x65\x65\x6e\x20\x30\x20\x61\x6e\x64\x20\x35\x39\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x65\x63\x6f\x6e\x64\x20\x3d\x20\x73\x65\x63\x6f\x6e\x64\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x64\x61\x74\x65\x28\x73\x65\x6c\x66\x29\x20\x2d\x3e\x20\x64\x61\x74\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x64\x61\x74\x65\x28\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x73\x65\x6c\x66\x2e\x64\x61\x79\x29\x0a\x0a\x20\x20\x20\x20\x40\x73\x74\x61\x74\x69\x63\x6d\x65\x74\x68\x6f\x64\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x6e\x6f\x77\x28\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x74\x20\x3d\x20\x6c\x6f\x63\x61\x6c\x74\x69\x6d\x65\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6d\x5f\x73\x65\x63\x20\x3d\x20\x74\x2e\x74\x6d\x5f\x73\x65\x63\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x6d\x5f\x73\x65\x63\x20\x3d\x3d\x20\x36\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6d\x5f\x73\x65\x63\x20\x3d\x20\x35\x39\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x64\x61\x74\x65\x74\x69\x6d\x65\x28\x74\x2e\x74\x6d\x5f\x79\x65\x61\x72\x2c\x20\x74\x2e\x74\x6d\x5f\x6d\x6f\x6e\x2c\x20\x74\x2e\x74\x6d\x5f\x6d\x64\x61\x79\x2c\x20\x74\x2e\x74\x6d\x5f\x68\x6f\x75\x72\x2c\x20\x74\x2e\x74\x6d\x5f\x6d\x69\x6e\x2c\x20\x74\x6d\x5f\x73\x65\x63\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x73\x74\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x22\x7b\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x7d\x2d\x7b\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x3a\x30\x32\x7d\x2d\x7b\x73\x65\x6c\x66\x2e\x64\x61\x79\x3a\x30\x32\x7d\x20\x7b\x73\x65\x6c\x66\x2e\x68\x6f\x75\x72\x3a\x30\x32\x7d\x3a\x7b\x73\x65\x6c\x66\x2e\x6d\x69\x6e\x75\x74\x65\x3a\x30\x32\x7d\x3a\x7b\x73\x65\x6c\x66\x2e\x73\x65\x63\x6f\x6e\x64\x3a\x30\x32\x7d\x22\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72" "\x65\x74\x75\x72\x6e\x20\x66\x22\x64\x61\x74\x65\x74\x69\x6d\x65\x2e\x64\x61\x74\x65\x74\x69\x6d\x65\x28\x7b\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x7d\x2c\x20\x7b\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x7d\x2c\x20\x7b\x73\x65\x6c\x66\x2e\x64\x61\x79\x7d\x2c\x20\x7b\x73\x65\x6c\x66\x2e\x68\x6f\x75\x72\x7d\x2c\x20\x7b\x73\x65\x6c\x66\x2e\x6d\x69\x6e\x75\x74\x65\x7d\x2c\x20\x7b\x73\x65\x6c\x66\x2e\x73\x65\x63\x6f\x6e\x64\x7d\x29\x22\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x64\x61\x74\x65\x74\x69\x6d\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x73\x65\x6c\x66\x2e\x64\x61\x79\x2c\x20\x73\x65\x6c\x66\x2e\x68\x6f\x75\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x69\x6e\x75\x74\x65\x2c\x20\x73\x65\x6c\x66\x2e\x73\x65\x63\x6f\x6e\x64\x29\x20\x3d\x3d\x5c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x28\x6f\x74\x68\x65\x72\x2e\x79\x65\x61\x72\x2c\x20\x6f\x74\x68\x65\x72\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x61\x79\x2c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x74\x68\x65\x72\x2e\x68\x6f\x75\x72\x2c\x20\x6f\x74\x68\x65\x72\x2e\x6d\x69\x6e\x75\x74\x65\x2c\x20\x6f\x74\x68\x65\x72\x2e\x73\x65\x63\x6f\x6e\x64\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x64\x61\x74\x65\x74\x69\x6d\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x73\x65\x6c\x66\x2e\x64\x61\x79\x2c\x20\x73\x65\x6c\x66\x2e\x68\x6f\x75\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x69\x6e\x75\x74\x65\x2c\x20\x73\x65\x6c\x66\x2e\x73\x65\x63\x6f\x6e\x64\x29\x20\x3c\x5c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x28\x6f\x74\x68\x65\x72\x2e\x79\x65\x61\x72\x2c\x20\x6f\x74\x68\x65\x72\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x61\x79\x2c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x74\x68\x65\x72\x2e\x68\x6f\x75\x72\x2c\x20\x6f\x74\x68\x65\x72\x2e\x6d\x69\x6e\x75\x74\x65\x2c\x20\x6f\x74\x68\x65\x72\x2e\x73\x65\x63\x6f\x6e\x64\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x64\x61\x74\x65\x74\x69\x6d\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x73\x65\x6c\x66\x2e\x64\x61\x79\x2c\x20\x73\x65\x6c\x66\x2e\x68\x6f\x75\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x69\x6e\x75\x74\x65\x2c\x20\x73\x65\x6c\x66\x2e\x73\x65\x63\x6f\x6e\x64\x29\x20\x3c\x3d\x5c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x28\x6f\x74\x68\x65\x72\x2e\x79\x65\x61\x72\x2c\x20\x6f\x74\x68\x65\x72\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x61\x79\x2c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x74\x68\x65\x72\x2e\x68\x6f\x75\x72\x2c\x20\x6f\x74\x68\x65\x72\x2e\x6d\x69\x6e\x75\x74\x65\x2c\x20\x6f\x74\x68\x65\x72\x2e\x73\x65\x63\x6f\x6e\x64\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x64\x61\x74\x65\x74\x69\x6d\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x73\x65\x6c\x66\x2e\x64\x61\x79\x2c\x20\x73\x65\x6c\x66\x2e\x68\x6f\x75\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x69\x6e\x75\x74\x65\x2c\x20\x73\x65\x6c\x66\x2e\x73\x65\x63\x6f\x6e\x64\x29\x20\x3e\x5c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x28\x6f\x74\x68\x65\x72\x2e\x79\x65\x61\x72\x2c\x20\x6f\x74\x68\x65" "\x72\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x61\x79\x2c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x74\x68\x65\x72\x2e\x68\x6f\x75\x72\x2c\x20\x6f\x74\x68\x65\x72\x2e\x6d\x69\x6e\x75\x74\x65\x2c\x20\x6f\x74\x68\x65\x72\x2e\x73\x65\x63\x6f\x6e\x64\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x65\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x64\x61\x74\x65\x74\x69\x6d\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x28\x73\x65\x6c\x66\x2e\x79\x65\x61\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x73\x65\x6c\x66\x2e\x64\x61\x79\x2c\x20\x73\x65\x6c\x66\x2e\x68\x6f\x75\x72\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x69\x6e\x75\x74\x65\x2c\x20\x73\x65\x6c\x66\x2e\x73\x65\x63\x6f\x6e\x64\x29\x20\x3e\x3d\x5c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x28\x6f\x74\x68\x65\x72\x2e\x79\x65\x61\x72\x2c\x20\x6f\x74\x68\x65\x72\x2e\x6d\x6f\x6e\x74\x68\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x61\x79\x2c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x74\x68\x65\x72\x2e\x68\x6f\x75\x72\x2c\x20\x6f\x74\x68\x65\x72\x2e\x6d\x69\x6e\x75\x74\x65\x2c\x20\x6f\x74\x68\x65\x72\x2e\x73\x65\x63\x6f\x6e\x64\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x74\x69\x6d\x65\x73\x74\x61\x6d\x70\x28\x73\x65\x6c\x66\x29\x20\x2d\x3e\x20\x66\x6c\x6f\x61\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x45\x72\x72\x6f\x72\x0a\x0a" },
        {"functools", "\x23\x20\x64\x65\x66\x20\x63\x61\x63\x68\x65\x28\x66\x29\x3a\x0a\x23\x20\x20\x20\x20\x20\x64\x65\x66\x20\x77\x72\x61\x70\x70\x65\x72\x28\x2a\x61\x72\x67\x73\x29\x3a\x0a\x23\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x68\x61\x73\x61\x74\x74\x72\x28\x66\x2c\x20\x27\x5f\x5f\x63\x61\x63\x68\x65\x5f\x5f\x27\x29\x3a\x0a\x23\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x2e\x5f\x5f\x63\x61\x63\x68\x65\x5f\x5f\x20\x3d\x20\x7b\x7d\x0a\x23\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6b\x65\x79\x20\x3d\x20\x61\x72\x67\x73\x0a\x23\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6b\x65\x79\x20\x6e\x6f\x74\x20\x69\x6e\x20\x66\x2e\x5f\x5f\x63\x61\x63\x68\x65\x5f\x5f\x3a\x0a\x23\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x2e\x5f\x5f\x63\x61\x63\x68\x65\x5f\x5f\x5b\x6b\x65\x79\x5d\x20\x3d\x20\x66\x28\x2a\x61\x72\x67\x73\x29\x0a\x23\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x2e\x5f\x5f\x63\x61\x63\x68\x65\x5f\x5f\x5b\x6b\x65\x79\x5d\x0a\x23\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x77\x72\x61\x70\x70\x65\x72\x0a\x0a\x63\x6c\x61\x73\x73\x20\x63\x61\x63\x68\x65\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x66\x20\x3d\x20\x66\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x63\x61\x63\x68\x65\x20\x3d\x20\x7b\x7d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x63\x61\x6c\x6c\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x2a\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x61\x72\x67\x73\x20\x6e\x6f\x74\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x63\x61\x63\x68\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x63\x61\x63\x68\x65\x5b\x61\x72\x67\x73\x5d\x20\x3d\x20\x73\x65\x6c\x66\x2e\x66\x28\x2a\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x63\x61\x63\x68\x65\x5b\x61\x72\x67\x73\x5d" },
        {"heapq", "\x23\x20\x48\x65\x61\x70\x20\x71\x75\x65\x75\x65\x20\x61\x6c\x67\x6f\x72\x69\x74\x68\x6d\x20\x28\x61\x2e\x6b\x2e\x61\x2e\x20\x70\x72\x69\x6f\x72\x69\x74\x79\x20\x71\x75\x65\x75\x65\x29\x0a\x64\x65\x66\x20\x68\x65\x61\x70\x70\x75\x73\x68\x28\x68\x65\x61\x70\x2c\x20\x69\x74\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x50\x75\x73\x68\x20\x69\x74\x65\x6d\x20\x6f\x6e\x74\x6f\x20\x68\x65\x61\x70\x2c\x20\x6d\x61\x69\x6e\x74\x61\x69\x6e\x69\x6e\x67\x20\x74\x68\x65\x20\x68\x65\x61\x70\x20\x69\x6e\x76\x61\x72\x69\x61\x6e\x74\x2e\x22\x22\x22\x0a\x20\x20\x20\x20\x68\x65\x61\x70\x2e\x61\x70\x70\x65\x6e\x64\x28\x69\x74\x65\x6d\x29\x0a\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x64\x6f\x77\x6e\x28\x68\x65\x61\x70\x2c\x20\x30\x2c\x20\x6c\x65\x6e\x28\x68\x65\x61\x70\x29\x2d\x31\x29\x0a\x0a\x64\x65\x66\x20\x68\x65\x61\x70\x70\x6f\x70\x28\x68\x65\x61\x70\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x50\x6f\x70\x20\x74\x68\x65\x20\x73\x6d\x61\x6c\x6c\x65\x73\x74\x20\x69\x74\x65\x6d\x20\x6f\x66\x66\x20\x74\x68\x65\x20\x68\x65\x61\x70\x2c\x20\x6d\x61\x69\x6e\x74\x61\x69\x6e\x69\x6e\x67\x20\x74\x68\x65\x20\x68\x65\x61\x70\x20\x69\x6e\x76\x61\x72\x69\x61\x6e\x74\x2e\x22\x22\x22\x0a\x20\x20\x20\x20\x6c\x61\x73\x74\x65\x6c\x74\x20\x3d\x20\x68\x65\x61\x70\x2e\x70\x6f\x70\x28\x29\x20\x20\x20\x20\x23\x20\x72\x61\x69\x73\x65\x73\x20\x61\x70\x70\x72\x6f\x70\x72\x69\x61\x74\x65\x20\x49\x6e\x64\x65\x78\x45\x72\x72\x6f\x72\x20\x69\x66\x20\x68\x65\x61\x70\x20\x69\x73\x20\x65\x6d\x70\x74\x79\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x65\x61\x70\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x69\x74\x65\x6d\x20\x3d\x20\x68\x65\x61\x70\x5b\x30\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x30\x5d\x20\x3d\x20\x6c\x61\x73\x74\x65\x6c\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x75\x70\x28\x68\x65\x61\x70\x2c\x20\x30\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x75\x72\x6e\x69\x74\x65\x6d\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x61\x73\x74\x65\x6c\x74\x0a\x0a\x64\x65\x66\x20\x68\x65\x61\x70\x72\x65\x70\x6c\x61\x63\x65\x28\x68\x65\x61\x70\x2c\x20\x69\x74\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x50\x6f\x70\x20\x61\x6e\x64\x20\x72\x65\x74\x75\x72\x6e\x20\x74\x68\x65\x20\x63\x75\x72\x72\x65\x6e\x74\x20\x73\x6d\x61\x6c\x6c\x65\x73\x74\x20\x76\x61\x6c\x75\x65\x2c\x20\x61\x6e\x64\x20\x61\x64\x64\x20\x74\x68\x65\x20\x6e\x65\x77\x20\x69\x74\x65\x6d\x2e\x0a\x0a\x20\x20\x20\x20\x54\x68\x69\x73\x20\x69\x73\x20\x6d\x6f\x72\x65\x20\x65\x66\x66\x69\x63\x69\x65\x6e\x74\x20\x74\x68\x61\x6e\x20\x68\x65\x61\x70\x70\x6f\x70\x28\x29\x20\x66\x6f\x6c\x6c\x6f\x77\x65\x64\x20\x62\x79\x20\x68\x65\x61\x70\x70\x75\x73\x68\x28\x29\x2c\x20\x61\x6e\x64\x20\x63\x61\x6e\x20\x62\x65\x0a\x20\x20\x20\x20\x6d\x6f\x72\x65\x20\x61\x70\x70\x72\x6f\x70\x72\x69\x61\x74\x65\x20\x77\x68\x65\x6e\x20\x75\x73\x69\x6e\x67\x20\x61\x20\x66\x69\x78\x65\x64\x2d\x73\x69\x7a\x65\x20\x68\x65\x61\x70\x2e\x20\x20\x4e\x6f\x74\x65\x20\x74\x68\x61\x74\x20\x74\x68\x65\x20\x76\x61\x6c\x75\x65\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x65\x64\x20\x6d\x61\x79\x20\x62\x65\x20\x6c\x61\x72\x67\x65\x72\x20\x74\x68\x61\x6e\x20\x69\x74\x65\x6d\x21\x20\x20\x54\x68\x61\x74\x20\x63\x6f\x6e\x73\x74\x72\x61\x69\x6e\x73\x20\x72\x65\x61\x73\x6f\x6e\x61\x62\x6c\x65\x20\x75\x73\x65\x73\x20\x6f\x66\x0a\x20\x20\x20\x20\x74\x68\x69\x73\x20\x72\x6f\x75\x74\x69\x6e\x65\x20\x75\x6e\x6c\x65\x73\x73\x20\x77\x72\x69\x74\x74\x65\x6e\x20\x61\x73\x20\x70\x61\x72\x74\x20\x6f\x66\x20\x61\x20\x63\x6f\x6e\x64\x69\x74\x69\x6f\x6e\x61\x6c\x20\x72\x65\x70\x6c\x61\x63\x65\x6d\x65\x6e\x74\x3a\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x74\x65\x6d\x20\x3e\x20\x68\x65\x61\x70\x5b\x30\x5d\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x74\x65\x6d\x20\x3d\x20\x68\x65\x61\x70\x72\x65\x70\x6c\x61\x63\x65\x28\x68\x65\x61\x70\x2c\x20\x69\x74\x65\x6d\x29\x0a\x20\x20\x20\x20\x22\x22\x22\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x69\x74\x65\x6d\x20\x3d\x20\x68\x65\x61\x70\x5b\x30\x5d\x20\x20\x20\x20\x23\x20\x72\x61\x69\x73\x65\x73\x20\x61\x70\x70\x72\x6f\x70\x72\x69\x61\x74\x65\x20\x49\x6e\x64\x65\x78\x45\x72\x72\x6f\x72\x20\x69\x66\x20\x68\x65\x61\x70\x20\x69\x73\x20\x65\x6d\x70\x74\x79\x0a\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x30\x5d\x20\x3d\x20\x69\x74\x65\x6d\x0a\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x75\x70\x28\x68\x65\x61\x70\x2c\x20\x30\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x75\x72\x6e\x69\x74\x65\x6d\x0a\x0a\x64\x65\x66\x20\x68\x65\x61\x70\x70\x75\x73\x68\x70\x6f\x70\x28\x68\x65\x61\x70\x2c\x20\x69\x74\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x46\x61\x73\x74\x20\x76\x65\x72\x73\x69\x6f\x6e\x20\x6f\x66\x20\x61\x20\x68\x65\x61\x70\x70\x75\x73\x68\x20\x66\x6f\x6c\x6c\x6f\x77\x65\x64\x20\x62\x79\x20\x61\x20\x68\x65\x61\x70\x70\x6f\x70\x2e\x22\x22\x22\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x65\x61\x70\x20\x61\x6e\x64\x20\x68\x65\x61\x70\x5b\x30\x5d\x20\x3c\x20\x69\x74\x65\x6d\x3a\x0a\x20\x20\x20\x20\x20\x20\x20" "\x20\x69\x74\x65\x6d\x2c\x20\x68\x65\x61\x70\x5b\x30\x5d\x20\x3d\x20\x68\x65\x61\x70\x5b\x30\x5d\x2c\x20\x69\x74\x65\x6d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x75\x70\x28\x68\x65\x61\x70\x2c\x20\x30\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x74\x65\x6d\x0a\x0a\x64\x65\x66\x20\x68\x65\x61\x70\x69\x66\x79\x28\x78\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x54\x72\x61\x6e\x73\x66\x6f\x72\x6d\x20\x6c\x69\x73\x74\x20\x69\x6e\x74\x6f\x20\x61\x20\x68\x65\x61\x70\x2c\x20\x69\x6e\x2d\x70\x6c\x61\x63\x65\x2c\x20\x69\x6e\x20\x4f\x28\x6c\x65\x6e\x28\x78\x29\x29\x20\x74\x69\x6d\x65\x2e\x22\x22\x22\x0a\x20\x20\x20\x20\x6e\x20\x3d\x20\x6c\x65\x6e\x28\x78\x29\x0a\x20\x20\x20\x20\x23\x20\x54\x72\x61\x6e\x73\x66\x6f\x72\x6d\x20\x62\x6f\x74\x74\x6f\x6d\x2d\x75\x70\x2e\x20\x20\x54\x68\x65\x20\x6c\x61\x72\x67\x65\x73\x74\x20\x69\x6e\x64\x65\x78\x20\x74\x68\x65\x72\x65\x27\x73\x20\x61\x6e\x79\x20\x70\x6f\x69\x6e\x74\x20\x74\x6f\x20\x6c\x6f\x6f\x6b\x69\x6e\x67\x20\x61\x74\x0a\x20\x20\x20\x20\x23\x20\x69\x73\x20\x74\x68\x65\x20\x6c\x61\x72\x67\x65\x73\x74\x20\x77\x69\x74\x68\x20\x61\x20\x63\x68\x69\x6c\x64\x20\x69\x6e\x64\x65\x78\x20\x69\x6e\x2d\x72\x61\x6e\x67\x65\x2c\x20\x73\x6f\x20\x6d\x75\x73\x74\x20\x68\x61\x76\x65\x20\x32\x2a\x69\x20\x2b\x20\x31\x20\x3c\x20\x6e\x2c\x0a\x20\x20\x20\x20\x23\x20\x6f\x72\x20\x69\x20\x3c\x20\x28\x6e\x2d\x31\x29\x2f\x32\x2e\x20\x20\x49\x66\x20\x6e\x20\x69\x73\x20\x65\x76\x65\x6e\x20\x3d\x20\x32\x2a\x6a\x2c\x20\x74\x68\x69\x73\x20\x69\x73\x20\x28\x32\x2a\x6a\x2d\x31\x29\x2f\x32\x20\x3d\x20\x6a\x2d\x31\x2f\x32\x20\x73\x6f\x0a\x20\x20\x20\x20\x23\x20\x6a\x2d\x31\x20\x69\x73\x20\x74\x68\x65\x20\x6c\x61\x72\x67\x65\x73\x74\x2c\x20\x77\x68\x69\x63\x68\x20\x69\x73\x20\x6e\x2f\x2f\x32\x20\x2d\x20\x31\x2e\x20\x20\x49\x66\x20\x6e\x20\x69\x73\x20\x6f\x64\x64\x20\x3d\x20\x32\x2a\x6a\x2b\x31\x2c\x20\x74\x68\x69\x73\x20\x69\x73\x0a\x20\x20\x20\x20\x23\x20\x28\x32\x2a\x6a\x2b\x31\x2d\x31\x29\x2f\x32\x20\x3d\x20\x6a\x20\x73\x6f\x20\x6a\x2d\x31\x20\x69\x73\x20\x74\x68\x65\x20\x6c\x61\x72\x67\x65\x73\x74\x2c\x20\x61\x6e\x64\x20\x74\x68\x61\x74\x27\x73\x20\x61\x67\x61\x69\x6e\x20\x6e\x2f\x2f\x32\x2d\x31\x2e\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x65\x76\x65\x72\x73\x65\x64\x28\x72\x61\x6e\x67\x65\x28\x6e\x2f\x2f\x32\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x75\x70\x28\x78\x2c\x20\x69\x29\x0a\x0a\x23\x20\x27\x68\x65\x61\x70\x27\x20\x69\x73\x20\x61\x20\x68\x65\x61\x70\x20\x61\x74\x20\x61\x6c\x6c\x20\x69\x6e\x64\x69\x63\x65\x73\x20\x3e\x3d\x20\x73\x74\x61\x72\x74\x70\x6f\x73\x2c\x20\x65\x78\x63\x65\x70\x74\x20\x70\x6f\x73\x73\x69\x62\x6c\x79\x20\x66\x6f\x72\x20\x70\x6f\x73\x2e\x20\x20\x70\x6f\x73\x0a\x23\x20\x69\x73\x20\x74\x68\x65\x20\x69\x6e\x64\x65\x78\x20\x6f\x66\x20\x61\x20\x6c\x65\x61\x66\x20\x77\x69\x74\x68\x20\x61\x20\x70\x6f\x73\x73\x69\x62\x6c\x79\x20\x6f\x75\x74\x2d\x6f\x66\x2d\x6f\x72\x64\x65\x72\x20\x76\x61\x6c\x75\x65\x2e\x20\x20\x52\x65\x73\x74\x6f\x72\x65\x20\x74\x68\x65\x0a\x23\x20\x68\x65\x61\x70\x20\x69\x6e\x76\x61\x72\x69\x61\x6e\x74\x2e\x0a\x64\x65\x66\x20\x5f\x73\x69\x66\x74\x64\x6f\x77\x6e\x28\x68\x65\x61\x70\x2c\x20\x73\x74\x61\x72\x74\x70\x6f\x73\x2c\x20\x70\x6f\x73\x29\x3a\x0a\x20\x20\x20\x20\x6e\x65\x77\x69\x74\x65\x6d\x20\x3d\x20\x68\x65\x61\x70\x5b\x70\x6f\x73\x5d\x0a\x20\x20\x20\x20\x23\x20\x46\x6f\x6c\x6c\x6f\x77\x20\x74\x68\x65\x20\x70\x61\x74\x68\x20\x74\x6f\x20\x74\x68\x65\x20\x72\x6f\x6f\x74\x2c\x20\x6d\x6f\x76\x69\x6e\x67\x20\x70\x61\x72\x65\x6e\x74\x73\x20\x64\x6f\x77\x6e\x20\x75\x6e\x74\x69\x6c\x20\x66\x69\x6e\x64\x69\x6e\x67\x20\x61\x20\x70\x6c\x61\x63\x65\x0a\x20\x20\x20\x20\x23\x20\x6e\x65\x77\x69\x74\x65\x6d\x20\x66\x69\x74\x73\x2e\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x70\x6f\x73\x20\x3e\x20\x73\x74\x61\x72\x74\x70\x6f\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x70\x61\x72\x65\x6e\x74\x70\x6f\x73\x20\x3d\x20\x28\x70\x6f\x73\x20\x2d\x20\x31\x29\x20\x3e\x3e\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x70\x61\x72\x65\x6e\x74\x20\x3d\x20\x68\x65\x61\x70\x5b\x70\x61\x72\x65\x6e\x74\x70\x6f\x73\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x65\x77\x69\x74\x65\x6d\x20\x3c\x20\x70\x61\x72\x65\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x70\x6f\x73\x5d\x20\x3d\x20\x70\x61\x72\x65\x6e\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x70\x6f\x73\x20\x3d\x20\x70\x61\x72\x65\x6e\x74\x70\x6f\x73\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6f\x6e\x74\x69\x6e\x75\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x62\x72\x65\x61\x6b\x0a\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x70\x6f\x73\x5d\x20\x3d\x20\x6e\x65\x77\x69\x74\x65\x6d\x0a\x0a\x64\x65\x66\x20\x5f\x73\x69\x66\x74\x75\x70\x28\x68\x65\x61\x70\x2c\x20\x70\x6f\x73\x29\x3a\x0a\x20\x20\x20\x20\x65\x6e\x64\x70\x6f\x73\x20\x3d\x20\x6c\x65\x6e\x28\x68\x65\x61\x70\x29\x0a\x20\x20\x20\x20\x73\x74\x61\x72\x74\x70\x6f\x73\x20\x3d\x20\x70\x6f\x73\x0a\x20\x20\x20\x20\x6e\x65\x77\x69\x74\x65\x6d\x20\x3d\x20\x68" "\x65\x61\x70\x5b\x70\x6f\x73\x5d\x0a\x20\x20\x20\x20\x23\x20\x42\x75\x62\x62\x6c\x65\x20\x75\x70\x20\x74\x68\x65\x20\x73\x6d\x61\x6c\x6c\x65\x72\x20\x63\x68\x69\x6c\x64\x20\x75\x6e\x74\x69\x6c\x20\x68\x69\x74\x74\x69\x6e\x67\x20\x61\x20\x6c\x65\x61\x66\x2e\x0a\x20\x20\x20\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x3d\x20\x32\x2a\x70\x6f\x73\x20\x2b\x20\x31\x20\x20\x20\x20\x23\x20\x6c\x65\x66\x74\x6d\x6f\x73\x74\x20\x63\x68\x69\x6c\x64\x20\x70\x6f\x73\x69\x74\x69\x6f\x6e\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x3c\x20\x65\x6e\x64\x70\x6f\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x53\x65\x74\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x74\x6f\x20\x69\x6e\x64\x65\x78\x20\x6f\x66\x20\x73\x6d\x61\x6c\x6c\x65\x72\x20\x63\x68\x69\x6c\x64\x2e\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x69\x67\x68\x74\x70\x6f\x73\x20\x3d\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x2b\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x72\x69\x67\x68\x74\x70\x6f\x73\x20\x3c\x20\x65\x6e\x64\x70\x6f\x73\x20\x61\x6e\x64\x20\x6e\x6f\x74\x20\x68\x65\x61\x70\x5b\x63\x68\x69\x6c\x64\x70\x6f\x73\x5d\x20\x3c\x20\x68\x65\x61\x70\x5b\x72\x69\x67\x68\x74\x70\x6f\x73\x5d\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x3d\x20\x72\x69\x67\x68\x74\x70\x6f\x73\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x4d\x6f\x76\x65\x20\x74\x68\x65\x20\x73\x6d\x61\x6c\x6c\x65\x72\x20\x63\x68\x69\x6c\x64\x20\x75\x70\x2e\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x70\x6f\x73\x5d\x20\x3d\x20\x68\x65\x61\x70\x5b\x63\x68\x69\x6c\x64\x70\x6f\x73\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x70\x6f\x73\x20\x3d\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x3d\x20\x32\x2a\x70\x6f\x73\x20\x2b\x20\x31\x0a\x20\x20\x20\x20\x23\x20\x54\x68\x65\x20\x6c\x65\x61\x66\x20\x61\x74\x20\x70\x6f\x73\x20\x69\x73\x20\x65\x6d\x70\x74\x79\x20\x6e\x6f\x77\x2e\x20\x20\x50\x75\x74\x20\x6e\x65\x77\x69\x74\x65\x6d\x20\x74\x68\x65\x72\x65\x2c\x20\x61\x6e\x64\x20\x62\x75\x62\x62\x6c\x65\x20\x69\x74\x20\x75\x70\x0a\x20\x20\x20\x20\x23\x20\x74\x6f\x20\x69\x74\x73\x20\x66\x69\x6e\x61\x6c\x20\x72\x65\x73\x74\x69\x6e\x67\x20\x70\x6c\x61\x63\x65\x20\x28\x62\x79\x20\x73\x69\x66\x74\x69\x6e\x67\x20\x69\x74\x73\x20\x70\x61\x72\x65\x6e\x74\x73\x20\x64\x6f\x77\x6e\x29\x2e\x0a\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x70\x6f\x73\x5d\x20\x3d\x20\x6e\x65\x77\x69\x74\x65\x6d\x0a\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x64\x6f\x77\x6e\x28\x68\x65\x61\x70\x2c\x20\x73\x74\x61\x72\x74\x70\x6f\x73\x2c\x20\x70\x6f\x73\x29" },
        {"pickle", "\x69\x6d\x70\x6f\x72\x74\x20\x6a\x73\x6f\x6e\x0a\x66\x72\x6f\x6d\x20\x63\x20\x69\x6d\x70\x6f\x72\x74\x20\x73\x74\x72\x75\x63\x74\x0a\x69\x6d\x70\x6f\x72\x74\x20\x62\x75\x69\x6c\x74\x69\x6e\x73\x0a\x0a\x5f\x42\x41\x53\x49\x43\x5f\x54\x59\x50\x45\x53\x20\x3d\x20\x5b\x69\x6e\x74\x2c\x20\x66\x6c\x6f\x61\x74\x2c\x20\x73\x74\x72\x2c\x20\x62\x6f\x6f\x6c\x2c\x20\x74\x79\x70\x65\x28\x4e\x6f\x6e\x65\x29\x5d\x0a\x5f\x4d\x4f\x44\x5f\x54\x5f\x53\x45\x50\x20\x3d\x20\x22\x40\x22\x0a\x0a\x64\x65\x66\x20\x5f\x66\x69\x6e\x64\x5f\x63\x6c\x61\x73\x73\x28\x70\x61\x74\x68\x3a\x20\x73\x74\x72\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x5f\x4d\x4f\x44\x5f\x54\x5f\x53\x45\x50\x20\x6e\x6f\x74\x20\x69\x6e\x20\x70\x61\x74\x68\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x62\x75\x69\x6c\x74\x69\x6e\x73\x2e\x5f\x5f\x64\x69\x63\x74\x5f\x5f\x5b\x70\x61\x74\x68\x5d\x0a\x20\x20\x20\x20\x6d\x6f\x64\x70\x61\x74\x68\x2c\x20\x6e\x61\x6d\x65\x20\x3d\x20\x70\x61\x74\x68\x2e\x73\x70\x6c\x69\x74\x28\x5f\x4d\x4f\x44\x5f\x54\x5f\x53\x45\x50\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x5f\x69\x6d\x70\x6f\x72\x74\x5f\x5f\x28\x6d\x6f\x64\x70\x61\x74\x68\x29\x2e\x5f\x5f\x64\x69\x63\x74\x5f\x5f\x5b\x6e\x61\x6d\x65\x5d\x0a\x0a\x63\x6c\x61\x73\x73\x20\x5f\x50\x69\x63\x6b\x6c\x65\x72\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x62\x6a\x29\x20\x2d\x3e\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x6f\x62\x6a\x20\x3d\x20\x6f\x62\x6a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x72\x61\x77\x5f\x6d\x65\x6d\x6f\x20\x3d\x20\x7b\x7d\x20\x20\x23\x20\x69\x64\x20\x2d\x3e\x20\x69\x6e\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x6d\x65\x6d\x6f\x20\x3d\x20\x5b\x5d\x20\x20\x20\x20\x20\x20\x23\x20\x69\x6e\x74\x20\x2d\x3e\x20\x6f\x62\x6a\x65\x63\x74\x0a\x0a\x20\x20\x20\x20\x40\x73\x74\x61\x74\x69\x63\x6d\x65\x74\x68\x6f\x64\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x74\x79\x70\x65\x5f\x69\x64\x28\x74\x3a\x20\x74\x79\x70\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x74\x29\x20\x69\x73\x20\x74\x79\x70\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x61\x6d\x65\x20\x3d\x20\x74\x2e\x5f\x5f\x6e\x61\x6d\x65\x5f\x5f\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6d\x6f\x64\x20\x3d\x20\x74\x2e\x5f\x5f\x6d\x6f\x64\x75\x6c\x65\x5f\x5f\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6d\x6f\x64\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x61\x6d\x65\x20\x3d\x20\x6d\x6f\x64\x2e\x5f\x5f\x70\x61\x74\x68\x5f\x5f\x20\x2b\x20\x5f\x4d\x4f\x44\x5f\x54\x5f\x53\x45\x50\x20\x2b\x20\x6e\x61\x6d\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6e\x61\x6d\x65\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x77\x72\x61\x70\x28\x73\x65\x6c\x66\x2c\x20\x6f\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x5f\x74\x20\x3d\x20\x74\x79\x70\x65\x28\x6f\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x5f\x74\x20\x69\x6e\x20\x5f\x42\x41\x53\x49\x43\x5f\x54\x59\x50\x45\x53\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6f\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x5f\x74\x20\x69\x73\x20\x74\x79\x70\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x22\x74\x79\x70\x65\x22\x2c\x20\x73\x65\x6c\x66\x2e\x5f\x74\x79\x70\x65\x5f\x69\x64\x28\x6f\x29\x5d\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x6e\x64\x65\x78\x20\x3d\x20\x73\x65\x6c\x66\x2e\x72\x61\x77\x5f\x6d\x65\x6d\x6f\x2e\x67\x65\x74\x28\x69\x64\x28\x6f\x29\x2c\x20\x4e\x6f\x6e\x65\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x6e\x64\x65\x78\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x6e\x64\x65\x78\x20\x3d\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x2e\x6d\x65\x6d\x6f\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x6d\x65\x6d\x6f\x2e\x61\x70\x70\x65\x6e\x64\x28\x72\x65\x74\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x72\x61\x77\x5f\x6d\x65\x6d\x6f\x5b\x69\x64\x28\x6f\x29\x5d\x20\x3d\x20\x69\x6e\x64\x65\x78\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x5f\x74\x20\x69\x73\x20\x74\x75\x70\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x22\x74\x75\x70\x6c\x65\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x5b\x73\x65\x6c\x66\x2e\x77\x72\x61\x70\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x6f\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x5f\x74\x20\x69\x73\x20\x62\x79" "\x74\x65\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x22\x62\x79\x74\x65\x73\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x5b\x6f\x5b\x6a\x5d\x20\x66\x6f\x72\x20\x6a\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x6f\x29\x29\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x5f\x74\x20\x69\x73\x20\x6c\x69\x73\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x22\x6c\x69\x73\x74\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x5b\x73\x65\x6c\x66\x2e\x77\x72\x61\x70\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x6f\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x5f\x74\x20\x69\x73\x20\x64\x69\x63\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x22\x64\x69\x63\x74\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x5b\x5b\x73\x65\x6c\x66\x2e\x77\x72\x61\x70\x28\x6b\x29\x2c\x20\x73\x65\x6c\x66\x2e\x77\x72\x61\x70\x28\x76\x29\x5d\x20\x66\x6f\x72\x20\x6b\x2c\x76\x20\x69\x6e\x20\x6f\x2e\x69\x74\x65\x6d\x73\x28\x29\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x30\x20\x3d\x20\x73\x65\x6c\x66\x2e\x5f\x74\x79\x70\x65\x5f\x69\x64\x28\x6f\x5f\x74\x29\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x67\x65\x74\x61\x74\x74\x72\x28\x6f\x5f\x74\x2c\x20\x27\x5f\x5f\x73\x74\x72\x75\x63\x74\x5f\x5f\x27\x2c\x20\x46\x61\x6c\x73\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x5f\x30\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x6f\x2e\x74\x6f\x5f\x73\x74\x72\x75\x63\x74\x28\x29\x2e\x68\x65\x78\x28\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x68\x61\x73\x61\x74\x74\x72\x28\x6f\x2c\x20\x22\x5f\x5f\x67\x65\x74\x6e\x65\x77\x61\x72\x67\x73\x5f\x5f\x22\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x31\x20\x3d\x20\x6f\x2e\x5f\x5f\x67\x65\x74\x6e\x65\x77\x61\x72\x67\x73\x5f\x5f\x28\x29\x20\x20\x20\x20\x20\x23\x20\x61\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x31\x20\x3d\x20\x5b\x73\x65\x6c\x66\x2e\x77\x72\x61\x70\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x5f\x31\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x31\x20\x3d\x20\x4e\x6f\x6e\x65\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x2e\x5f\x5f\x64\x69\x63\x74\x5f\x5f\x20\x69\x73\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x32\x20\x3d\x20\x4e\x6f\x6e\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x32\x20\x3d\x20\x7b\x6b\x3a\x20\x73\x65\x6c\x66\x2e\x77\x72\x61\x70\x28\x76\x29\x20\x66\x6f\x72\x20\x6b\x2c\x76\x20\x69\x6e\x20\x6f\x2e\x5f\x5f\x64\x69\x63\x74\x5f\x5f\x2e\x69\x74\x65\x6d\x73\x28\x29\x7d\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x5f\x30\x29\x20\x20\x23\x20\x74\x79\x70\x65\x20\x69\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x5f\x31\x29\x20\x20\x23\x20\x6e\x65\x77\x61\x72\x67\x73\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x5f\x32\x29\x20\x20\x23\x20\x73\x74\x61\x74\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x72\x75\x6e\x5f\x70\x69\x70\x65\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x20\x3d\x20\x73\x65\x6c\x66\x2e\x77\x72\x61\x70\x28\x73\x65\x6c\x66\x2e\x6f\x62\x6a\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x6f\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x65\x6d\x6f\x5d\x0a\x0a\x0a\x0a\x63\x6c\x61\x73\x73\x20\x5f\x55\x6e\x70\x69\x63\x6b\x6c\x65\x72\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x62\x6a\x2c\x20\x6d\x65\x6d\x6f\x3a\x20\x6c\x69\x73\x74\x29\x20\x2d\x3e\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x6f\x62\x6a\x20\x3d\x20\x6f\x62\x6a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x6d\x65\x6d\x6f\x20\x3d\x20\x6d\x65\x6d\x6f\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f" "\x75\x6e\x77\x72\x61\x70\x70\x65\x64\x20\x3d\x20\x5b\x4e\x6f\x6e\x65\x5d\x20\x2a\x20\x6c\x65\x6e\x28\x6d\x65\x6d\x6f\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x74\x61\x67\x28\x73\x65\x6c\x66\x2c\x20\x69\x6e\x64\x65\x78\x2c\x20\x6f\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x73\x65\x6c\x66\x2e\x5f\x75\x6e\x77\x72\x61\x70\x70\x65\x64\x5b\x69\x6e\x64\x65\x78\x5d\x20\x69\x73\x20\x4e\x6f\x6e\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x75\x6e\x77\x72\x61\x70\x70\x65\x64\x5b\x69\x6e\x64\x65\x78\x5d\x20\x3d\x20\x6f\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x75\x6e\x77\x72\x61\x70\x28\x73\x65\x6c\x66\x2c\x20\x6f\x2c\x20\x69\x6e\x64\x65\x78\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x29\x20\x69\x6e\x20\x5f\x42\x41\x53\x49\x43\x5f\x54\x59\x50\x45\x53\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6f\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x6f\x29\x20\x69\x73\x20\x6c\x69\x73\x74\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x5b\x30\x5d\x20\x3d\x3d\x20\x22\x74\x79\x70\x65\x22\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x66\x69\x6e\x64\x5f\x63\x6c\x61\x73\x73\x28\x6f\x5b\x31\x5d\x29\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x72\x65\x66\x65\x72\x65\x6e\x63\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x5b\x30\x5d\x29\x20\x69\x73\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x69\x6e\x64\x65\x78\x20\x69\x73\x20\x4e\x6f\x6e\x65\x20\x20\x20\x20\x23\x20\x69\x6e\x64\x65\x78\x20\x73\x68\x6f\x75\x6c\x64\x20\x62\x65\x20\x4e\x6f\x6e\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x6e\x64\x65\x78\x20\x3d\x20\x6f\x5b\x30\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x5f\x75\x6e\x77\x72\x61\x70\x70\x65\x64\x5b\x69\x6e\x64\x65\x78\x5d\x20\x69\x73\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x20\x3d\x20\x73\x65\x6c\x66\x2e\x6d\x65\x6d\x6f\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x6f\x29\x20\x69\x73\x20\x6c\x69\x73\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x6f\x5b\x30\x5d\x29\x20\x69\x73\x20\x73\x74\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x6f\x2c\x20\x69\x6e\x64\x65\x78\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x73\x65\x6c\x66\x2e\x5f\x75\x6e\x77\x72\x61\x70\x70\x65\x64\x5b\x69\x6e\x64\x65\x78\x5d\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x75\x6e\x77\x72\x61\x70\x70\x65\x64\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x63\x6f\x6e\x63\x72\x65\x74\x65\x20\x72\x65\x66\x65\x72\x65\x6e\x63\x65\x20\x74\x79\x70\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x5b\x30\x5d\x20\x3d\x3d\x20\x22\x74\x75\x70\x6c\x65\x22\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x74\x75\x70\x6c\x65\x28\x5b\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x6f\x5b\x31\x5d\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x67\x28\x69\x6e\x64\x65\x78\x2c\x20\x72\x65\x74\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x5b\x30\x5d\x20\x3d\x3d\x20\x22\x62\x79\x74\x65\x73\x22\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x62\x79\x74\x65\x73\x28\x6f\x5b\x31\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x67\x28\x69\x6e\x64\x65\x78\x2c\x20\x72\x65\x74\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x5b\x30\x5d\x20\x3d\x3d\x20\x22\x6c\x69\x73\x74\x22\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x67\x28\x69\x6e\x64\x65\x78\x2c\x20\x72\x65\x74\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x6f\x5b\x31\x5d\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x69\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69" "\x66\x20\x6f\x5b\x30\x5d\x20\x3d\x3d\x20\x22\x64\x69\x63\x74\x22\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x7b\x7d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x67\x28\x69\x6e\x64\x65\x78\x2c\x20\x72\x65\x74\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x6b\x2c\x76\x20\x69\x6e\x20\x6f\x5b\x31\x5d\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x5b\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x6b\x29\x5d\x20\x3d\x20\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x76\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x67\x65\x6e\x65\x72\x69\x63\x20\x6f\x62\x6a\x65\x63\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6c\x73\x20\x3d\x20\x5f\x66\x69\x6e\x64\x5f\x63\x6c\x61\x73\x73\x28\x6f\x5b\x30\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x67\x65\x74\x61\x74\x74\x72\x28\x63\x6c\x73\x2c\x20\x27\x5f\x5f\x73\x74\x72\x75\x63\x74\x5f\x5f\x27\x2c\x20\x46\x61\x6c\x73\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x6e\x73\x74\x20\x3d\x20\x63\x6c\x73\x2e\x66\x72\x6f\x6d\x5f\x73\x74\x72\x75\x63\x74\x28\x73\x74\x72\x75\x63\x74\x2e\x66\x72\x6f\x6d\x68\x65\x78\x28\x6f\x5b\x31\x5d\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x67\x28\x69\x6e\x64\x65\x78\x2c\x20\x69\x6e\x73\x74\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x6e\x73\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x2c\x20\x6e\x65\x77\x61\x72\x67\x73\x2c\x20\x73\x74\x61\x74\x65\x20\x3d\x20\x6f\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x63\x72\x65\x61\x74\x65\x20\x75\x6e\x69\x6e\x69\x74\x69\x61\x6c\x69\x7a\x65\x64\x20\x69\x6e\x73\x74\x61\x6e\x63\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x65\x77\x5f\x66\x20\x3d\x20\x67\x65\x74\x61\x74\x74\x72\x28\x63\x6c\x73\x2c\x20\x22\x5f\x5f\x6e\x65\x77\x5f\x5f\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x65\x77\x61\x72\x67\x73\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x65\x77\x61\x72\x67\x73\x20\x3d\x20\x5b\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x6e\x65\x77\x61\x72\x67\x73\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x6e\x73\x74\x20\x3d\x20\x6e\x65\x77\x5f\x66\x28\x63\x6c\x73\x2c\x20\x2a\x6e\x65\x77\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x6e\x73\x74\x20\x3d\x20\x6e\x65\x77\x5f\x66\x28\x63\x6c\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x67\x28\x69\x6e\x64\x65\x78\x2c\x20\x69\x6e\x73\x74\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x72\x65\x73\x74\x6f\x72\x65\x20\x73\x74\x61\x74\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x74\x61\x74\x65\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x6b\x2c\x76\x20\x69\x6e\x20\x73\x74\x61\x74\x65\x2e\x69\x74\x65\x6d\x73\x28\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x74\x61\x74\x74\x72\x28\x69\x6e\x73\x74\x2c\x20\x6b\x2c\x20\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x76\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x6e\x73\x74\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x72\x75\x6e\x5f\x70\x69\x70\x65\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x73\x65\x6c\x66\x2e\x6f\x62\x6a\x29\x0a\x0a\x0a\x64\x65\x66\x20\x5f\x77\x72\x61\x70\x28\x6f\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x50\x69\x63\x6b\x6c\x65\x72\x28\x6f\x29\x2e\x72\x75\x6e\x5f\x70\x69\x70\x65\x28\x29\x0a\x0a\x64\x65\x66\x20\x5f\x75\x6e\x77\x72\x61\x70\x28\x70\x61\x63\x6b\x65\x64\x3a\x20\x6c\x69\x73\x74\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x55\x6e\x70\x69\x63\x6b\x6c\x65\x72\x28\x2a\x70\x61\x63\x6b\x65\x64\x29\x2e\x72\x75\x6e\x5f\x70\x69\x70\x65\x28\x29\x0a\x0a\x64\x65\x66\x20\x64\x75\x6d\x70\x73\x28\x6f\x29\x20\x2d\x3e\x20\x62\x79\x74\x65\x73\x3a\x0a\x20\x20\x20\x20\x6f\x20\x3d\x20\x5f\x77\x72\x61\x70\x28\x6f\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6a\x73\x6f\x6e\x2e\x64\x75\x6d\x70\x73\x28\x6f\x29\x2e\x65\x6e\x63\x6f\x64\x65\x28\x29\x0a\x0a\x64\x65\x66\x20\x6c\x6f\x61\x64\x73\x28\x62\x29\x20\x2d\x3e\x20\x6f\x62\x6a\x65\x63\x74\x3a\x0a\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x62\x29\x20\x69\x73" "\x20\x62\x79\x74\x65\x73\x0a\x20\x20\x20\x20\x6f\x20\x3d\x20\x6a\x73\x6f\x6e\x2e\x6c\x6f\x61\x64\x73\x28\x62\x2e\x64\x65\x63\x6f\x64\x65\x28\x29\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x75\x6e\x77\x72\x61\x70\x28\x6f\x29" },
        {"this", "\x70\x72\x69\x6e\x74\x28\x22\x22\x22\x54\x68\x65\x20\x5a\x65\x6e\x20\x6f\x66\x20\x50\x79\x74\x68\x6f\x6e\x2c\x20\x62\x79\x20\x54\x69\x6d\x20\x50\x65\x74\x65\x72\x73\x0a\x0a\x42\x65\x61\x75\x74\x69\x66\x75\x6c\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x75\x67\x6c\x79\x2e\x0a\x45\x78\x70\x6c\x69\x63\x69\x74\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x69\x6d\x70\x6c\x69\x63\x69\x74\x2e\x0a\x53\x69\x6d\x70\x6c\x65\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x63\x6f\x6d\x70\x6c\x65\x78\x2e\x0a\x43\x6f\x6d\x70\x6c\x65\x78\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x63\x6f\x6d\x70\x6c\x69\x63\x61\x74\x65\x64\x2e\x0a\x46\x6c\x61\x74\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x6e\x65\x73\x74\x65\x64\x2e\x0a\x53\x70\x61\x72\x73\x65\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x64\x65\x6e\x73\x65\x2e\x0a\x52\x65\x61\x64\x61\x62\x69\x6c\x69\x74\x79\x20\x63\x6f\x75\x6e\x74\x73\x2e\x0a\x53\x70\x65\x63\x69\x61\x6c\x20\x63\x61\x73\x65\x73\x20\x61\x72\x65\x6e\x27\x74\x20\x73\x70\x65\x63\x69\x61\x6c\x20\x65\x6e\x6f\x75\x67\x68\x20\x74\x6f\x20\x62\x72\x65\x61\x6b\x20\x74\x68\x65\x20\x72\x75\x6c\x65\x73\x2e\x0a\x41\x6c\x74\x68\x6f\x75\x67\x68\x20\x70\x72\x61\x63\x74\x69\x63\x61\x6c\x69\x74\x79\x20\x62\x65\x61\x74\x73\x20\x70\x75\x72\x69\x74\x79\x2e\x0a\x45\x72\x72\x6f\x72\x73\x20\x73\x68\x6f\x75\x6c\x64\x20\x6e\x65\x76\x65\x72\x20\x70\x61\x73\x73\x20\x73\x69\x6c\x65\x6e\x74\x6c\x79\x2e\x0a\x55\x6e\x6c\x65\x73\x73\x20\x65\x78\x70\x6c\x69\x63\x69\x74\x6c\x79\x20\x73\x69\x6c\x65\x6e\x63\x65\x64\x2e\x0a\x49\x6e\x20\x74\x68\x65\x20\x66\x61\x63\x65\x20\x6f\x66\x20\x61\x6d\x62\x69\x67\x75\x69\x74\x79\x2c\x20\x72\x65\x66\x75\x73\x65\x20\x74\x68\x65\x20\x74\x65\x6d\x70\x74\x61\x74\x69\x6f\x6e\x20\x74\x6f\x20\x67\x75\x65\x73\x73\x2e\x0a\x54\x68\x65\x72\x65\x20\x73\x68\x6f\x75\x6c\x64\x20\x62\x65\x20\x6f\x6e\x65\x2d\x2d\x20\x61\x6e\x64\x20\x70\x72\x65\x66\x65\x72\x61\x62\x6c\x79\x20\x6f\x6e\x6c\x79\x20\x6f\x6e\x65\x20\x2d\x2d\x6f\x62\x76\x69\x6f\x75\x73\x20\x77\x61\x79\x20\x74\x6f\x20\x64\x6f\x20\x69\x74\x2e\x0a\x41\x6c\x74\x68\x6f\x75\x67\x68\x20\x74\x68\x61\x74\x20\x77\x61\x79\x20\x6d\x61\x79\x20\x6e\x6f\x74\x20\x62\x65\x20\x6f\x62\x76\x69\x6f\x75\x73\x20\x61\x74\x20\x66\x69\x72\x73\x74\x20\x75\x6e\x6c\x65\x73\x73\x20\x79\x6f\x75\x27\x72\x65\x20\x44\x75\x74\x63\x68\x2e\x0a\x4e\x6f\x77\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x6e\x65\x76\x65\x72\x2e\x0a\x41\x6c\x74\x68\x6f\x75\x67\x68\x20\x6e\x65\x76\x65\x72\x20\x69\x73\x20\x6f\x66\x74\x65\x6e\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x2a\x72\x69\x67\x68\x74\x2a\x20\x6e\x6f\x77\x2e\x0a\x49\x66\x20\x74\x68\x65\x20\x69\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x61\x74\x69\x6f\x6e\x20\x69\x73\x20\x68\x61\x72\x64\x20\x74\x6f\x20\x65\x78\x70\x6c\x61\x69\x6e\x2c\x20\x69\x74\x27\x73\x20\x61\x20\x62\x61\x64\x20\x69\x64\x65\x61\x2e\x0a\x49\x66\x20\x74\x68\x65\x20\x69\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x61\x74\x69\x6f\x6e\x20\x69\x73\x20\x65\x61\x73\x79\x20\x74\x6f\x20\x65\x78\x70\x6c\x61\x69\x6e\x2c\x20\x69\x74\x20\x6d\x61\x79\x20\x62\x65\x20\x61\x20\x67\x6f\x6f\x64\x20\x69\x64\x65\x61\x2e\x0a\x4e\x61\x6d\x65\x73\x70\x61\x63\x65\x73\x20\x61\x72\x65\x20\x6f\x6e\x65\x20\x68\x6f\x6e\x6b\x69\x6e\x67\x20\x67\x72\x65\x61\x74\x20\x69\x64\x65\x61\x20\x2d\x2d\x20\x6c\x65\x74\x27\x73\x20\x64\x6f\x20\x6d\x6f\x72\x65\x20\x6f\x66\x20\x74\x68\x6f\x73\x65\x21\x22\x22\x22\x29" },
        {"typing", "\x63\x6c\x61\x73\x73\x20\x5f\x50\x6c\x61\x63\x65\x68\x6f\x6c\x64\x65\x72\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x2a\x61\x72\x67\x73\x2c\x20\x2a\x2a\x6b\x77\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x70\x61\x73\x73\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x65\x74\x69\x74\x65\x6d\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x2a\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x63\x61\x6c\x6c\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x2a\x61\x72\x67\x73\x2c\x20\x2a\x2a\x6b\x77\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x0a\x20\x20\x20\x20\x0a\x0a\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x20\x3d\x20\x5f\x50\x6c\x61\x63\x65\x68\x6f\x6c\x64\x65\x72\x28\x29\x0a\x0a\x4c\x69\x73\x74\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x44\x69\x63\x74\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x54\x75\x70\x6c\x65\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x53\x65\x74\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x41\x6e\x79\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x55\x6e\x69\x6f\x6e\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x4f\x70\x74\x69\x6f\x6e\x61\x6c\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x43\x61\x6c\x6c\x61\x62\x6c\x65\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x54\x79\x70\x65\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x50\x72\x6f\x74\x6f\x63\x6f\x6c\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x0a\x4c\x69\x74\x65\x72\x61\x6c\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x4c\x69\x74\x65\x72\x61\x6c\x53\x74\x72\x69\x6e\x67\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x0a\x49\x74\x65\x72\x61\x62\x6c\x65\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x47\x65\x6e\x65\x72\x61\x74\x6f\x72\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x0a\x54\x79\x70\x65\x56\x61\x72\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x53\x65\x6c\x66\x20\x3d\x20\x5f\x50\x4c\x41\x43\x45\x48\x4f\x4c\x44\x45\x52\x0a\x0a\x63\x6c\x61\x73\x73\x20\x47\x65\x6e\x65\x72\x69\x63\x3a\x0a\x20\x20\x20\x20\x70\x61\x73\x73\x0a\x0a\x54\x59\x50\x45\x5f\x43\x48\x45\x43\x4b\x49\x4e\x47\x20\x3d\x20\x46\x61\x6c\x73\x65\x0a\x0a\x23\x20\x64\x65\x63\x6f\x72\x61\x74\x6f\x72\x73\x0a\x6f\x76\x65\x72\x6c\x6f\x61\x64\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x78\x3a\x20\x78\x0a\x66\x69\x6e\x61\x6c\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x78\x3a\x20\x78\x0a" },
        {"_long", "\x23\x20\x61\x66\x74\x65\x72\x20\x76\x31\x2e\x32\x2e\x32\x2c\x20\x69\x6e\x74\x20\x69\x73\x20\x61\x6c\x77\x61\x79\x73\x20\x36\x34\x2d\x62\x69\x74\x0a\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x20\x3d\x20\x36\x30\x2f\x2f\x32\x20\x2d\x20\x31\x0a\x0a\x50\x79\x4c\x6f\x6e\x67\x5f\x42\x41\x53\x45\x20\x3d\x20\x32\x20\x2a\x2a\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x0a\x50\x79\x4c\x6f\x6e\x67\x5f\x4d\x41\x53\x4b\x20\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x42\x41\x53\x45\x20\x2d\x20\x31\x0a\x50\x79\x4c\x6f\x6e\x67\x5f\x44\x45\x43\x49\x4d\x41\x4c\x5f\x53\x48\x49\x46\x54\x20\x3d\x20\x34\x0a\x50\x79\x4c\x6f\x6e\x67\x5f\x44\x45\x43\x49\x4d\x41\x4c\x5f\x42\x41\x53\x45\x20\x3d\x20\x31\x30\x20\x2a\x2a\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x44\x45\x43\x49\x4d\x41\x4c\x5f\x53\x48\x49\x46\x54\x0a\x0a\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x66\x72\x6f\x6d\x69\x6e\x74\x28\x78\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x20\x6c\x69\x73\x74\x20\x6f\x66\x20\x64\x69\x67\x69\x74\x73\x20\x61\x6e\x64\x20\x73\x69\x67\x6e\x0a\x20\x20\x20\x20\x69\x66\x20\x78\x20\x3d\x3d\x20\x30\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x30\x5d\x2c\x20\x31\x0a\x20\x20\x20\x20\x73\x69\x67\x6e\x20\x3d\x20\x31\x20\x69\x66\x20\x78\x20\x3e\x20\x30\x20\x65\x6c\x73\x65\x20\x2d\x31\x0a\x20\x20\x20\x20\x69\x66\x20\x73\x69\x67\x6e\x20\x3c\x20\x30\x3a\x20\x78\x20\x3d\x20\x2d\x78\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x78\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x78\x20\x26\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x4d\x41\x53\x4b\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x78\x20\x3e\x3e\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x2c\x20\x73\x69\x67\x6e\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x63\x6d\x70\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x6c\x69\x73\x74\x29\x20\x2d\x3e\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x23\x20\x72\x65\x74\x75\x72\x6e\x20\x31\x20\x69\x66\x20\x61\x3e\x62\x2c\x20\x2d\x31\x20\x69\x66\x20\x61\x3c\x62\x2c\x20\x30\x20\x69\x66\x20\x61\x3d\x3d\x62\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x61\x29\x20\x3e\x20\x6c\x65\x6e\x28\x62\x29\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x31\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x61\x29\x20\x3c\x20\x6c\x65\x6e\x28\x62\x29\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x2d\x31\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x61\x29\x2d\x31\x2c\x20\x2d\x31\x2c\x20\x2d\x31\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x61\x5b\x69\x5d\x20\x3e\x20\x62\x5b\x69\x5d\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x61\x5b\x69\x5d\x20\x3c\x20\x62\x5b\x69\x5d\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x2d\x31\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x30\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x70\x61\x64\x5f\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x73\x69\x7a\x65\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x70\x61\x64\x20\x6c\x65\x61\x64\x69\x6e\x67\x20\x7a\x65\x72\x6f\x73\x20\x74\x6f\x20\x68\x61\x76\x65\x20\x60\x73\x69\x7a\x65\x60\x20\x64\x69\x67\x69\x74\x73\x0a\x20\x20\x20\x20\x64\x65\x6c\x74\x61\x20\x3d\x20\x73\x69\x7a\x65\x20\x2d\x20\x6c\x65\x6e\x28\x61\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x64\x65\x6c\x74\x61\x20\x3e\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x2e\x65\x78\x74\x65\x6e\x64\x28\x5b\x30\x5d\x20\x2a\x20\x64\x65\x6c\x74\x61\x29\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x75\x6e\x70\x61\x64\x5f\x28\x61\x3a\x20\x6c\x69\x73\x74\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x72\x65\x6d\x6f\x76\x65\x20\x6c\x65\x61\x64\x69\x6e\x67\x20\x7a\x65\x72\x6f\x73\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6c\x65\x6e\x28\x61\x29\x3e\x31\x20\x61\x6e\x64\x20\x61\x5b\x2d\x31\x5d\x3d\x3d\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x2e\x70\x6f\x70\x28\x29\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x61\x64\x64\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x6c\x69\x73\x74\x29\x20\x2d\x3e\x20\x6c\x69\x73\x74\x3a\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x30\x5d\x20\x2a\x20\x6d\x61\x78\x28\x6c\x65\x6e\x28\x61\x29\x2c\x20\x6c\x65\x6e\x28\x62\x29\x29\x0a\x20\x20\x20\x20\x75\x6c\x6f\x6e\x67\x5f\x70\x61\x64\x5f\x28\x61\x2c\x20\x6c\x65\x6e\x28\x72\x65\x73\x29\x29\x0a\x20\x20\x20\x20\x75\x6c\x6f\x6e\x67\x5f\x70\x61\x64\x5f\x28\x62\x2c\x20\x6c\x65\x6e\x28\x72\x65\x73\x29\x29\x0a\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x72\x65\x73\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x2b\x3d\x20\x61" "\x5b\x69\x5d\x20\x2b\x20\x62\x5b\x69\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x5b\x69\x5d\x20\x3d\x20\x63\x61\x72\x72\x79\x20\x26\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x4d\x41\x53\x4b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3e\x3e\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x0a\x20\x20\x20\x20\x69\x66\x20\x63\x61\x72\x72\x79\x20\x3e\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x63\x61\x72\x72\x79\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x69\x6e\x63\x5f\x28\x61\x3a\x20\x6c\x69\x73\x74\x29\x3a\x0a\x20\x20\x20\x20\x61\x5b\x30\x5d\x20\x2b\x3d\x20\x31\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x61\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x61\x5b\x69\x5d\x20\x3c\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x42\x41\x53\x45\x3a\x20\x62\x72\x65\x61\x6b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x5b\x69\x5d\x20\x2d\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x42\x41\x53\x45\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x2b\x31\x20\x3d\x3d\x20\x6c\x65\x6e\x28\x61\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x2e\x61\x70\x70\x65\x6e\x64\x28\x31\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x5b\x69\x2b\x31\x5d\x20\x2b\x3d\x20\x31\x0a\x20\x20\x20\x20\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x73\x75\x62\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x6c\x69\x73\x74\x29\x20\x2d\x3e\x20\x6c\x69\x73\x74\x3a\x0a\x20\x20\x20\x20\x23\x20\x61\x20\x3e\x3d\x20\x62\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x62\x6f\x72\x72\x6f\x77\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x62\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6d\x70\x20\x3d\x20\x61\x5b\x69\x5d\x20\x2d\x20\x62\x5b\x69\x5d\x20\x2d\x20\x62\x6f\x72\x72\x6f\x77\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x6d\x70\x20\x3c\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6d\x70\x20\x2b\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x42\x41\x53\x45\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x6f\x72\x72\x6f\x77\x20\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x6f\x72\x72\x6f\x77\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x74\x6d\x70\x29\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x62\x29\x2c\x20\x6c\x65\x6e\x28\x61\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6d\x70\x20\x3d\x20\x61\x5b\x69\x5d\x20\x2d\x20\x62\x6f\x72\x72\x6f\x77\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x6d\x70\x20\x3c\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6d\x70\x20\x2b\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x42\x41\x53\x45\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x6f\x72\x72\x6f\x77\x20\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x6f\x72\x72\x6f\x77\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x74\x6d\x70\x29\x0a\x20\x20\x20\x20\x75\x6c\x6f\x6e\x67\x5f\x75\x6e\x70\x61\x64\x5f\x28\x72\x65\x73\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x64\x69\x76\x6d\x6f\x64\x69\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x62\x20\x3e\x20\x30\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x61\x29\x2d\x31\x2c\x20\x2d\x31\x2c\x20\x2d\x31\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3c\x3c\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x2b\x3d\x20\x61\x5b\x69\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x63\x61\x72\x72\x79\x20\x2f\x2f\x20\x62\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x25\x3d\x20\x62\x0a\x20\x20\x20\x20\x72\x65\x73\x2e\x72\x65\x76\x65\x72\x73\x65\x28\x29\x0a\x20\x20\x20\x20\x75\x6c\x6f\x6e\x67\x5f\x75\x6e\x70\x61\x64\x5f\x28\x72\x65\x73\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x2c\x20\x63\x61\x72\x72\x79\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x64\x69\x76\x6d\x6f\x64\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x6c\x69\x73\x74\x29\x3a\x0a\x20\x20\x20\x20\x71\x20\x3d\x20\x5b\x30\x5d\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x75\x6c\x6f\x6e\x67\x5f\x63\x6d\x70\x28\x61\x2c\x20\x62\x29\x20\x3e\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x75" "\x6c\x6f\x6e\x67\x5f\x69\x6e\x63\x5f\x28\x71\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x73\x75\x62\x28\x61\x2c\x20\x62\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x71\x2c\x20\x61\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x66\x6c\x6f\x6f\x72\x64\x69\x76\x69\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x62\x20\x3e\x20\x30\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x75\x6c\x6f\x6e\x67\x5f\x64\x69\x76\x6d\x6f\x64\x69\x28\x61\x2c\x20\x62\x29\x5b\x30\x5d\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x69\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x62\x20\x3e\x3d\x20\x30\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x30\x5d\x20\x2a\x20\x6c\x65\x6e\x28\x61\x29\x0a\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x61\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x2b\x3d\x20\x61\x5b\x69\x5d\x20\x2a\x20\x62\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x5b\x69\x5d\x20\x3d\x20\x63\x61\x72\x72\x79\x20\x26\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x4d\x41\x53\x4b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3e\x3e\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x0a\x20\x20\x20\x20\x69\x66\x20\x63\x61\x72\x72\x79\x20\x3e\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x63\x61\x72\x72\x79\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x6c\x69\x73\x74\x29\x3a\x0a\x20\x20\x20\x20\x4e\x20\x3d\x20\x6c\x65\x6e\x28\x61\x29\x20\x2b\x20\x6c\x65\x6e\x28\x62\x29\x0a\x20\x20\x20\x20\x23\x20\x75\x73\x65\x20\x67\x72\x61\x64\x65\x2d\x73\x63\x68\x6f\x6f\x6c\x20\x6d\x75\x6c\x74\x69\x70\x6c\x69\x63\x61\x74\x69\x6f\x6e\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x30\x5d\x20\x2a\x20\x4e\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x61\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x6a\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x62\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x2b\x3d\x20\x72\x65\x73\x5b\x69\x2b\x6a\x5d\x20\x2b\x20\x61\x5b\x69\x5d\x20\x2a\x20\x62\x5b\x6a\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x5b\x69\x2b\x6a\x5d\x20\x3d\x20\x63\x61\x72\x72\x79\x20\x26\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x4d\x41\x53\x4b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3e\x3e\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x5b\x69\x2b\x6c\x65\x6e\x28\x62\x29\x5d\x20\x3d\x20\x63\x61\x72\x72\x79\x0a\x20\x20\x20\x20\x75\x6c\x6f\x6e\x67\x5f\x75\x6e\x70\x61\x64\x5f\x28\x72\x65\x73\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x70\x6f\x77\x69\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x62\x20\x3e\x3d\x20\x30\x0a\x20\x20\x20\x20\x69\x66\x20\x62\x20\x3d\x3d\x20\x30\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x31\x5d\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x31\x5d\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x62\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x62\x20\x26\x20\x31\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x28\x72\x65\x73\x2c\x20\x61\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x28\x61\x2c\x20\x61\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x62\x20\x3e\x3e\x3d\x20\x31\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x72\x65\x70\x72\x28\x78\x3a\x20\x6c\x69\x73\x74\x29\x20\x2d\x3e\x20\x73\x74\x72\x3a\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6c\x65\x6e\x28\x78\x29\x3e\x31\x20\x6f\x72\x20\x78\x5b\x30\x5d\x3e\x30\x3a\x20\x20\x20\x23\x20\x6e\x6f\x6e\x2d\x7a\x65\x72\x6f\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x78\x2c\x20\x72\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x64\x69\x76\x6d\x6f\x64\x69\x28\x78\x2c\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x44\x45\x43\x49\x4d\x41\x4c\x5f\x42\x41\x53\x45\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x74\x72\x28\x72\x29\x2e\x7a\x66\x69\x6c\x6c\x28\x50\x79\x4c\x6f\x6e\x67\x5f\x44\x45\x43\x49\x4d\x41\x4c\x5f\x53\x48\x49\x46\x54\x29\x29\x0a\x20\x20\x20\x20\x72\x65\x73\x2e\x72\x65\x76\x65\x72\x73\x65\x28\x29\x0a\x20\x20\x20\x20\x73\x20\x3d\x20\x27\x27\x2e\x6a\x6f\x69\x6e\x28\x72\x65\x73\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x73\x29" "\x20\x3d\x3d\x20\x30\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x30\x27\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x73\x29\x20\x3e\x20\x31\x3a\x20\x73\x20\x3d\x20\x73\x2e\x6c\x73\x74\x72\x69\x70\x28\x27\x30\x27\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x66\x72\x6f\x6d\x73\x74\x72\x28\x73\x3a\x20\x73\x74\x72\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x73\x5b\x2d\x31\x5d\x20\x3d\x3d\x20\x27\x4c\x27\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x20\x3d\x20\x73\x5b\x3a\x2d\x31\x5d\x0a\x20\x20\x20\x20\x72\x65\x73\x2c\x20\x62\x61\x73\x65\x20\x3d\x20\x5b\x30\x5d\x2c\x20\x5b\x31\x5d\x0a\x20\x20\x20\x20\x69\x66\x20\x73\x5b\x30\x5d\x20\x3d\x3d\x20\x27\x2d\x27\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x69\x67\x6e\x20\x3d\x20\x2d\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x20\x3d\x20\x73\x5b\x31\x3a\x5d\x0a\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x69\x67\x6e\x20\x3d\x20\x31\x0a\x20\x20\x20\x20\x73\x20\x3d\x20\x73\x5b\x3a\x3a\x2d\x31\x5d\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x63\x20\x69\x6e\x20\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x20\x3d\x20\x6f\x72\x64\x28\x63\x29\x20\x2d\x20\x34\x38\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x30\x20\x3c\x3d\x20\x63\x20\x3c\x3d\x20\x39\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x61\x64\x64\x28\x72\x65\x73\x2c\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x69\x28\x62\x61\x73\x65\x2c\x20\x63\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x62\x61\x73\x65\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x69\x28\x62\x61\x73\x65\x2c\x20\x31\x30\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x2c\x20\x73\x69\x67\x6e\x0a\x0a\x63\x6c\x61\x73\x73\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x78\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x78\x29\x20\x69\x73\x20\x74\x75\x70\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x20\x78\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x78\x29\x20\x69\x73\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x66\x72\x6f\x6d\x69\x6e\x74\x28\x78\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x78\x29\x20\x69\x73\x20\x66\x6c\x6f\x61\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x66\x72\x6f\x6d\x69\x6e\x74\x28\x69\x6e\x74\x28\x78\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x78\x29\x20\x69\x73\x20\x73\x74\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x66\x72\x6f\x6d\x73\x74\x72\x28\x78\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x78\x29\x20\x69\x73\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x20\x78\x2e\x64\x69\x67\x69\x74\x73\x2e\x63\x6f\x70\x79\x28\x29\x2c\x20\x78\x2e\x73\x69\x67\x6e\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x54\x79\x70\x65\x45\x72\x72\x6f\x72\x28\x27\x65\x78\x70\x65\x63\x74\x65\x64\x20\x69\x6e\x74\x20\x6f\x72\x20\x73\x74\x72\x27\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x61\x64\x64\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x74\x68\x65\x72\x20\x3d\x20\x6c\x6f\x6e\x67\x28\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x3d\x20\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x75\x6c\x6f\x6e\x67" "\x5f\x61\x64\x64\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6d\x70\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x63\x6d\x70\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x63\x6d\x70\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x30\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x63\x6d\x70\x20\x3e\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x75\x6c\x6f\x6e\x67\x5f\x73\x75\x62\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x75\x6c\x6f\x6e\x67\x5f\x73\x75\x62\x28\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x29\x2c\x20\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x61\x64\x64\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x61\x64\x64\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x74\x68\x65\x72\x20\x3d\x20\x6c\x6f\x6e\x67\x28\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x21\x3d\x20\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x75\x6c\x6f\x6e\x67\x5f\x61\x64\x64\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6d\x70\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x63\x6d\x70\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x63\x6d\x70\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x30\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x63\x6d\x70\x20\x3e\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x75\x6c\x6f\x6e\x67\x5f\x73\x75\x62\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x75\x6c\x6f\x6e\x67\x5f\x73\x75\x62\x28\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x29\x2c\x20\x2d\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x74\x68\x65\x72\x20\x3d\x20\x6c\x6f\x6e\x67\x28\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6f\x74\x68\x65\x72\x2e\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6d\x75\x6c\x5f\x5f" "\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x69\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x61\x62\x73\x28\x6f\x74\x68\x65\x72\x29\x29\x2c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x2a\x20\x28\x31\x20\x69\x66\x20\x6f\x74\x68\x65\x72\x20\x3e\x3d\x20\x30\x20\x65\x6c\x73\x65\x20\x2d\x31\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x2c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x2a\x20\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x6d\x75\x6c\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x6d\x75\x6c\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x64\x69\x76\x6d\x6f\x64\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x3d\x20\x31\x20\x61\x6e\x64\x20\x6f\x74\x68\x65\x72\x20\x3e\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x71\x2c\x20\x72\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x64\x69\x76\x6d\x6f\x64\x69\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x71\x2c\x20\x31\x29\x29\x2c\x20\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x3d\x20\x31\x20\x61\x6e\x64\x20\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x20\x3d\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x71\x2c\x20\x72\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x64\x69\x76\x6d\x6f\x64\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x6c\x65\x6e\x28\x6f\x74\x68\x65\x72\x29\x3e\x31\x20\x6f\x72\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x5b\x30\x5d\x3e\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x71\x2c\x20\x31\x29\x29\x2c\x20\x6c\x6f\x6e\x67\x28\x28\x72\x2c\x20\x31\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x45\x72\x72\x6f\x72\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x66\x6c\x6f\x6f\x72\x64\x69\x76\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x64\x69\x76\x6d\x6f\x64\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x5b\x30\x5d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6d\x6f\x64\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x64\x69\x76\x6d\x6f\x64\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x5b\x31\x5d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x70\x6f\x77\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x20\x61\x6e\x64\x20\x6f\x74\x68\x65\x72\x20\x3e\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66" "\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x3d\x20\x2d\x31\x20\x61\x6e\x64\x20\x6f\x74\x68\x65\x72\x20\x26\x20\x31\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x69\x67\x6e\x20\x3d\x20\x2d\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x69\x67\x6e\x20\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x75\x6c\x6f\x6e\x67\x5f\x70\x6f\x77\x69\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x29\x2c\x20\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x73\x68\x69\x66\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x20\x61\x6e\x64\x20\x6f\x74\x68\x65\x72\x20\x3e\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x78\x20\x3d\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2e\x63\x6f\x70\x79\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x71\x2c\x20\x72\x20\x3d\x20\x64\x69\x76\x6d\x6f\x64\x28\x6f\x74\x68\x65\x72\x2c\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x78\x20\x3d\x20\x5b\x30\x5d\x2a\x71\x20\x2b\x20\x78\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x5f\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x72\x29\x3a\x20\x78\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x69\x28\x78\x2c\x20\x32\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x78\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x73\x68\x69\x66\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x20\x61\x6e\x64\x20\x6f\x74\x68\x65\x72\x20\x3e\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x78\x20\x3d\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2e\x63\x6f\x70\x79\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x71\x2c\x20\x72\x20\x3d\x20\x64\x69\x76\x6d\x6f\x64\x28\x6f\x74\x68\x65\x72\x2c\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x78\x20\x3d\x20\x78\x5b\x71\x3a\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x78\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x30\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x5f\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x72\x29\x3a\x20\x78\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x66\x6c\x6f\x6f\x72\x64\x69\x76\x69\x28\x78\x2c\x20\x32\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x78\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6e\x65\x67\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x2d\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x63\x6d\x70\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x74\x68\x65\x72\x20\x3d\x20\x6c\x6f\x6e\x67\x28\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3e\x20\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3c\x20\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x2d\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x75\x6c\x6f\x6e\x67\x5f\x63\x6d\x70\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x63\x6d\x70\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x20\x3d\x3d\x20\x30\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x74\x5f\x5f\x28" "\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x63\x6d\x70\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x20\x3c\x20\x30\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x63\x6d\x70\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x20\x3c\x3d\x20\x30\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x63\x6d\x70\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x20\x3e\x20\x30\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x65\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x63\x6d\x70\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x20\x3e\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x70\x72\x65\x66\x69\x78\x20\x3d\x20\x27\x2d\x27\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3c\x20\x30\x20\x65\x6c\x73\x65\x20\x27\x27\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x70\x72\x65\x66\x69\x78\x20\x2b\x20\x75\x6c\x6f\x6e\x67\x5f\x72\x65\x70\x72\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x29\x20\x2b\x20\x27\x4c\x27\x0a" },
        {"_set", "\x63\x6c\x61\x73\x73\x20\x73\x65\x74\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x20\x3d\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x20\x6f\x72\x20\x5b\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x20\x3d\x20\x7b\x7d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x74\x65\x6d\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x61\x64\x64\x28\x69\x74\x65\x6d\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x61\x64\x64\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x65\x6c\x65\x6d\x5d\x20\x3d\x20\x4e\x6f\x6e\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x64\x69\x73\x63\x61\x72\x64\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x64\x65\x6c\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x65\x6c\x65\x6d\x5d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x72\x65\x6d\x6f\x76\x65\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x64\x65\x6c\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x65\x6c\x65\x6d\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6c\x65\x61\x72\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x63\x6c\x65\x61\x72\x28\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x75\x70\x64\x61\x74\x65\x28\x73\x65\x6c\x66\x2c\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x2e\x5f\x61\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6f\x70\x79\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x74\x28\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x6b\x65\x79\x73\x28\x29\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x61\x6e\x64\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x74\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6f\x72\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x6c\x66\x2e\x63\x6f\x70\x79\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x74\x28\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x6e\x6f\x74\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x78\x6f\x72\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x74\x28\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x6e\x6f\x74\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20" "\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x6e\x6f\x74\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x75\x6e\x69\x6f\x6e\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x7c\x20\x6f\x74\x68\x65\x72\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x6e\x74\x65\x72\x73\x65\x63\x74\x69\x6f\x6e\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x26\x20\x6f\x74\x68\x65\x72\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x64\x69\x66\x66\x65\x72\x65\x6e\x63\x65\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x2d\x20\x6f\x74\x68\x65\x72\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x73\x79\x6d\x6d\x65\x74\x72\x69\x63\x5f\x64\x69\x66\x66\x65\x72\x65\x6e\x63\x65\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x5e\x20\x6f\x74\x68\x65\x72\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x78\x6f\x72\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x73\x64\x69\x73\x6a\x6f\x69\x6e\x74\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x61\x6e\x64\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x73\x73\x75\x62\x73\x65\x74\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x73\x73\x75\x70\x65\x72\x73\x65\x74\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6f\x74\x68\x65\x72\x2e\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x63\x6f\x6e\x74\x61\x69\x6e\x73\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x73\x65\x74\x28\x29\x27\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x7b\x27\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x72\x65\x70\x72\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x6b\x65\x79\x73\x28\x29\x5d\x29\x20\x2b\x20\x27\x7d\x27\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x74\x65\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x74\x65\x72\x28\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x6b\x65\x79\x73\x28\x29\x29" },

    };
}   // namespace pkpy



namespace pkpy {

#define PY_CLASS(T, mod, name)                  \
    static Type _type(VM* vm) {                 \
        PK_LOCAL_STATIC const std::pair<StrName, StrName> _path(#mod, #name); \
        return PK_OBJ_GET(Type, vm->_modules[_path.first]->attr(_path.second)); \
    }                                                                       \
    static void _check_type(VM* vm, PyObject* val){                         \
        if(!vm->isinstance(val, T::_type(vm))){                             \
            vm->TypeError("expected '" #mod "." #name "', got " + _type_name(vm, vm->_tp(val)).escape());  \
        }                                                                   \
    }                                                                       \
    static PyObject* register_class(VM* vm, PyObject* mod, Type base=0) {   \
        std::string_view mod_name = PK_OBJ_GET(Str, mod->attr("__name__")).sv();   \
        if(mod_name != #mod) {                                                     \
            Str msg = _S("register_class() failed: ", mod_name, " != ", #mod);    \
            throw std::runtime_error(msg.str());                            \
        }                                                                   \
        PyObject* type = vm->new_type_object(mod, #name, base);             \
        mod->attr().set(#name, type);                                       \
        T::_register(vm, mod, type);                                        \
        return type;                                                        \
    }                                                                       

#define VAR_T(T, ...) vm->heap.gcnew<T>(T::_type(vm), __VA_ARGS__)

struct VoidP{
    PY_CLASS(VoidP, c, void_p)

    void* ptr;
    VoidP(const void* ptr): ptr(const_cast<void*>(ptr)){}

    bool operator==(const VoidP& other) const {
        return ptr == other.ptr;
    }
    bool operator!=(const VoidP& other) const {
        return ptr != other.ptr;
    }
    bool operator<(const VoidP& other) const { return ptr < other.ptr; }
    bool operator<=(const VoidP& other) const { return ptr <= other.ptr; }
    bool operator>(const VoidP& other) const { return ptr > other.ptr; }
    bool operator>=(const VoidP& other) const { return ptr >= other.ptr; }

    Str hex() const{
        SStream ss;
        ss.write_hex(ptr);
        return ss.str();
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

inline PyObject* py_var(VM* vm, const void* p){
    return VAR_T(VoidP, p);
}

#define POINTER_VAR(Tp, NAME)    \
    inline PyObject* py_var(VM* vm, Tp val){    \
        const static std::pair<StrName, StrName> P("c", NAME);      \
        PyObject* type = vm->_modules[P.first]->attr(P.second);     \
        return vm->heap.gcnew<VoidP>(PK_OBJ_GET(Type, type), val);  \
    }

POINTER_VAR(char*, "char_p")
// const char* is special, need to rethink about it
POINTER_VAR(const unsigned char*, "uchar_p")
POINTER_VAR(const short*, "short_p")
POINTER_VAR(const unsigned short*, "ushort_p")
POINTER_VAR(const int*, "int_p")
POINTER_VAR(const unsigned int*, "uint_p")
POINTER_VAR(const long*, "long_p")
POINTER_VAR(const unsigned long*, "ulong_p")
POINTER_VAR(const long long*, "longlong_p")
POINTER_VAR(const unsigned long long*, "ulonglong_p")
POINTER_VAR(const float*, "float_p")
POINTER_VAR(const double*, "double_p")
POINTER_VAR(const bool*, "bool_p")

#undef POINTER_VAR


struct C99Struct{
    PY_CLASS(C99Struct, c, struct)

    static constexpr int INLINE_SIZE = 24;

    char _inlined[INLINE_SIZE];
    char* p;
    int size;

    C99Struct(int new_size, bool zero_init=true){
        this->size = new_size;
        if(size <= INLINE_SIZE){
            p = _inlined;
        }else{
            p = (char*)malloc(size);
        }
        if(zero_init) memset(p, 0, size);
    }

    C99Struct(void* p, int size): C99Struct(size, false){
        if(p != nullptr) memcpy(this->p, p, size);
    }

    C99Struct(const C99Struct& other): C99Struct(other.p, other.size){}
    ~C99Struct(){ if(p!=_inlined) free(p); }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

static_assert(sizeof(Py_<C99Struct>) <= 64);
static_assert(sizeof(Py_<Tuple>) <= 64);

/***********************************************/
template<typename Tp>
Tp to_void_p(VM* vm, PyObject* var){
    static_assert(std::is_pointer_v<Tp>);
    if(var == vm->None) return nullptr;     // None can be casted to any pointer implicitly
    VoidP& p = CAST(VoidP&, var);
    return reinterpret_cast<Tp>(p.ptr);
}
/*****************************************************************/
void add_module_c(VM* vm);

}   // namespace pkpy
namespace pkpy{

    void VoidP::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<2>(type, [](VM* vm, ArgsView args){
            Type cls = PK_OBJ_GET(Type, args[0]);
            i64 addr = CAST(i64, args[1]);
            return vm->heap.gcnew<VoidP>(cls, reinterpret_cast<void*>(addr));
        });

        vm->bind__hash__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            VoidP& self = PK_OBJ_GET(VoidP, obj);
            return reinterpret_cast<i64>(self.ptr);
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            VoidP& self = PK_OBJ_GET(VoidP, obj);
            return VAR(_S("<void* at ", self.hex(), ">"));
        });

#define BIND_CMP(name, op)  \
        vm->bind##name(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){        \
            if(!vm->isinstance(rhs, VoidP::_type(vm))) return vm->NotImplemented;               \
            void* _0 = PK_OBJ_GET(VoidP, lhs).ptr;                                              \
            void* _1 = PK_OBJ_GET(VoidP, rhs).ptr;                                              \
            return VAR(_0 op _1);                                                               \
        });

        BIND_CMP(__eq__, ==)
        BIND_CMP(__lt__, <)
        BIND_CMP(__le__, <=)
        BIND_CMP(__gt__, >)
        BIND_CMP(__ge__, >=)

#undef BIND_CMP
    }


    void C99Struct::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<2>(type, [](VM* vm, ArgsView args){
            Type cls = PK_OBJ_GET(Type, args[0]);
            int size = CAST(int, args[1]);
            return vm->heap.gcnew<C99Struct>(cls, size);
        });

        vm->bind_method<0>(type, "hex", [](VM* vm, ArgsView args){
            const C99Struct& self = _CAST(C99Struct&, args[0]);
            SStream ss;
            for(int i=0; i<self.size; i++) ss.write_hex((unsigned char)self.p[i]);
            return VAR(ss.str());
        });

        // @staticmethod
        vm->bind_func<1>(type, "fromhex", [](VM* vm, ArgsView args){
            const Str& s = CAST(Str&, args[0]);
            if(s.size<2 || s.size%2!=0) vm->ValueError("invalid hex string");
            C99Struct buffer(s.size/2, false);
            for(int i=0; i<s.size; i+=2){
                char c = 0;
                if(s[i]>='0' && s[i]<='9') c += s[i]-'0';
                else if(s[i]>='A' && s[i]<='F') c += s[i]-'A'+10;
                else if(s[i]>='a' && s[i]<='f') c += s[i]-'a'+10;
                else vm->ValueError(_S("invalid hex char: '", s[i], "'"));
                c <<= 4;
                if(s[i+1]>='0' && s[i+1]<='9') c += s[i+1]-'0';
                else if(s[i+1]>='A' && s[i+1]<='F') c += s[i+1]-'A'+10;
                else if(s[i+1]>='a' && s[i+1]<='f') c += s[i+1]-'a'+10;
                else vm->ValueError(_S("invalid hex char: '", s[i+1], "'"));
                buffer.p[i/2] = c;
            }
            return VAR_T(C99Struct, std::move(buffer));
        }, {}, BindType::STATICMETHOD);

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            C99Struct& self = _CAST(C99Struct&, obj);
            SStream ss;
            ss << "<struct object of " << self.size << " bytes>";
            return VAR(ss.str());
        });

        vm->bind_method<0>(type, "addr", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR_T(VoidP, self.p);
        });

        vm->bind_method<0>(type, "sizeof", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR(self.size);
        });

        vm->bind_method<0>(type, "copy", [](VM* vm, ArgsView args){
            const C99Struct& self = _CAST(C99Struct&, args[0]);
            return vm->heap.gcnew<C99Struct>(vm->_tp(args[0]), self);
        });

        vm->bind__eq__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            C99Struct& self = _CAST(C99Struct&, lhs);
            if(!is_non_tagged_type(rhs, C99Struct::_type(vm))) return vm->NotImplemented;
            C99Struct& other = _CAST(C99Struct&, rhs);
            bool ok = self.size == other.size && memcmp(self.p, other.p, self.size) == 0;
            return VAR(ok);
        });

#define BIND_SETGET(T, name) \
        vm->bind(type, "read_" name "(self, offset=0)", [](VM* vm, ArgsView args){          \
            C99Struct& self = _CAST(C99Struct&, args[0]);   \
            i64 offset = CAST(i64, args[1]);    \
            void* ptr = self.p + offset;    \
            return VAR(*(T*)ptr);   \
        }); \
        vm->bind(type, "write_" name "(self, value, offset=0)", [](VM* vm, ArgsView args){  \
            C99Struct& self = _CAST(C99Struct&, args[0]);   \
            i64 offset = CAST(i64, args[2]);    \
            void* ptr = self.p + offset;    \
            *(T*)ptr = CAST(T, args[1]);    \
            return vm->None;    \
        });
        BIND_SETGET(char, "char")
        BIND_SETGET(unsigned char, "uchar")
        BIND_SETGET(short, "short")
        BIND_SETGET(unsigned short, "ushort")
        BIND_SETGET(int, "int")
        BIND_SETGET(unsigned int, "uint")
        BIND_SETGET(long, "long")
        BIND_SETGET(unsigned long, "ulong")
        BIND_SETGET(long long, "longlong")
        BIND_SETGET(unsigned long long, "ulonglong")
        BIND_SETGET(float, "float")
        BIND_SETGET(double, "double")
        BIND_SETGET(bool, "bool")
        BIND_SETGET(void*, "void_p")
#undef BIND_SETGET
    }

void add_module_c(VM* vm){
    PyObject* mod = vm->new_module("c");
    
    vm->bind_func<1>(mod, "malloc", [](VM* vm, ArgsView args){
        i64 size = CAST(i64, args[0]);
        return VAR(malloc(size));
    });

    vm->bind_func<1>(mod, "free", [](VM* vm, ArgsView args){
        void* p = CAST(void*, args[0]);
        free(p);
        return vm->None;
    });

    vm->bind_func<3>(mod, "memset", [](VM* vm, ArgsView args){
        void* p = CAST(void*, args[0]);
        memset(p, CAST(int, args[1]), CAST(size_t, args[2]));
        return vm->None;
    });

    vm->bind_func<3>(mod, "memcpy", [](VM* vm, ArgsView args){
        void* dst = CAST(void*, args[0]);
        void* src = CAST(void*, args[1]);
        i64 size = CAST(i64, args[2]);
        memcpy(dst, src, size);
        return vm->None;
    });

    VoidP::register_class(vm, mod);
    C99Struct::register_class(vm, mod);
    mod->attr().set("NULL", VAR_T(VoidP, nullptr));

    vm->bind(mod, "p_cast(ptr: 'void_p', cls: type[T]) -> T", [](VM* vm, ArgsView args){
        VoidP& ptr = CAST(VoidP&, args[0]);
        vm->check_non_tagged_type(args[1], vm->tp_type);
        Type cls = PK_OBJ_GET(Type, args[1]);
        if(!vm->issubclass(cls, VoidP::_type(vm))){
            vm->ValueError("expected a subclass of void_p");
        }
        return vm->heap.gcnew<VoidP>(cls, ptr.ptr);
    });

    vm->bind(mod, "p_value(ptr: 'void_p') -> int", [](VM* vm, ArgsView args){
        VoidP& ptr = CAST(VoidP&, args[0]);
        return VAR(reinterpret_cast<i64>(ptr.ptr));
    });

    vm->bind(mod, "pp_deref(ptr: Tp) -> Tp", [](VM* vm, ArgsView args){
        VoidP& ptr = CAST(VoidP&, args[0]);
        void* value = *reinterpret_cast<void**>(ptr.ptr);
        return vm->heap.gcnew<VoidP>(args[0]->type, value);
    });

    PyObject* type;
    Type type_t = -1;

#define BIND_PRIMITIVE(T, CNAME) \
    vm->bind_func<1>(mod, CNAME "_", [](VM* vm, ArgsView args){         \
        T val = CAST(T, args[0]);                                       \
        return VAR_T(C99Struct, &val, sizeof(T));                       \
    });                                                                 \
    type = vm->new_type_object(mod, CNAME "_p", VoidP::_type(vm));      \
    mod->attr().set(CNAME "_p", type);                                  \
    type_t = PK_OBJ_GET(Type, type);                                    \
    vm->bind_method<0>(type, "read", [](VM* vm, ArgsView args){         \
        VoidP& voidp = PK_OBJ_GET(VoidP, args[0]);                      \
        T* target = (T*)voidp.ptr;                                      \
        return VAR(*target);                                            \
    });                                                                 \
    vm->bind_method<1>(type, "write", [](VM* vm, ArgsView args){        \
        VoidP& voidp = PK_OBJ_GET(VoidP, args[0]);                      \
        T val = CAST(T, args[1]);                                       \
        T* target = (T*)voidp.ptr;                                      \
        *target = val;                                                  \
        return vm->None;                                                \
    });                                                                 \
    vm->bind__getitem__(type_t, [](VM* vm, PyObject* obj, PyObject* index){  \
        VoidP& voidp = PK_OBJ_GET(VoidP, obj);                               \
        i64 offset = CAST(i64, index);                                  \
        T* target = (T*)voidp.ptr;                                      \
        return VAR(target[offset]);                                     \
    });                                                                 \
    vm->bind__setitem__(type_t, [](VM* vm, PyObject* obj, PyObject* index, PyObject* value){   \
        VoidP& voidp = PK_OBJ_GET(VoidP, obj);                          \
        i64 offset = CAST(i64, index);                                  \
        T* target = (T*)voidp.ptr;                                      \
        target[offset] = CAST(T, value);                                \
    });                                                                 \
    vm->bind__add__(type_t, [](VM* vm, PyObject* lhs, PyObject* rhs){   \
        VoidP& voidp = PK_OBJ_GET(VoidP, lhs);                          \
        i64 offset = CAST(i64, rhs);                                    \
        T* target = (T*)voidp.ptr;                                      \
        return vm->heap.gcnew<VoidP>(lhs->type, target + offset);       \
    });                                                                 \
    vm->bind__sub__(type_t, [](VM* vm, PyObject* lhs, PyObject* rhs){   \
        VoidP& voidp = PK_OBJ_GET(VoidP, lhs);                          \
        i64 offset = CAST(i64, rhs);                                    \
        T* target = (T*)voidp.ptr;                                      \
        return vm->heap.gcnew<VoidP>(lhs->type, target - offset);       \
    });                                                                 \
    vm->bind__repr__(type_t, [](VM* vm, PyObject* obj){                 \
        VoidP& self = _CAST(VoidP&, obj);                               \
        return VAR(_S("<", CNAME, "* at ", self.hex(), ">"));         \
    });                                                                 \

    BIND_PRIMITIVE(char, "char")
    BIND_PRIMITIVE(unsigned char, "uchar")
    BIND_PRIMITIVE(short, "short")
    BIND_PRIMITIVE(unsigned short, "ushort")
    BIND_PRIMITIVE(int, "int")
    BIND_PRIMITIVE(unsigned int, "uint")
    BIND_PRIMITIVE(long, "long")
    BIND_PRIMITIVE(unsigned long, "ulong")
    BIND_PRIMITIVE(long long, "longlong")
    BIND_PRIMITIVE(unsigned long long, "ulonglong")
    BIND_PRIMITIVE(float, "float")
    BIND_PRIMITIVE(double, "double")
    BIND_PRIMITIVE(bool, "bool")

#undef BIND_PRIMITIVE

    PyObject* char_p_t = mod->attr("char_p");
    vm->bind(char_p_t, "read_string(self) -> str", [](VM* vm, ArgsView args){
        VoidP& voidp = PK_OBJ_GET(VoidP, args[0]);
        const char* target = (const char*)voidp.ptr;
        return VAR(target);
    });

    vm->bind(char_p_t, "write_string(self, value: str)", [](VM* vm, ArgsView args){
        VoidP& voidp = PK_OBJ_GET(VoidP, args[0]);
        std::string_view sv = CAST(Str&, args[1]).sv();
        char* target = (char*)voidp.ptr;
        memcpy(target, sv.data(), sv.size());
        target[sv.size()] = '\0';
        return vm->None;
    });
}

}   // namespace pkpy


namespace pkpy{

struct NativeProxyFuncCBase {
    virtual PyObject* operator()(VM* vm, ArgsView args) = 0;
};

template<typename Ret, typename... Params>
struct NativeProxyFuncC final: NativeProxyFuncCBase {
    static constexpr int N = sizeof...(Params);
    using _Fp = Ret(*)(Params...);
    _Fp func;
    NativeProxyFuncC(_Fp func) : func(func) {}

    PyObject* operator()(VM* vm, ArgsView args) override {
        PK_ASSERT(args.size() == N);
        return call<Ret>(vm, args, std::make_index_sequence<N>());
    }

    template<typename __Ret, size_t... Is>
    PyObject* call(VM* vm, ArgsView args, std::index_sequence<Is...>){
        if constexpr(std::is_void_v<__Ret>){
            func(py_cast<Params>(vm, args[Is])...);
            return vm->None;
        }else{
            __Ret ret = func(py_cast<Params>(vm, args[Is])...);
            return VAR(std::move(ret));
        }
    }
};

template<typename Ret, typename T, typename... Params>
struct NativeProxyMethodC final: NativeProxyFuncCBase {
    static constexpr int N = sizeof...(Params);
    using _Fp = Ret(T::*)(Params...);
    _Fp func;
    NativeProxyMethodC(_Fp func) : func(func) {}

    PyObject* operator()(VM* vm, ArgsView args) override {
        PK_ASSERT(args.size() == N+1);
        return call<Ret>(vm, args, std::make_index_sequence<N>());
    }

    template<typename __Ret, size_t... Is>
    PyObject* call(VM* vm, ArgsView args, std::index_sequence<Is...>){
        T& self = py_cast<T&>(vm, args[0]);
        if constexpr(std::is_void_v<__Ret>){
            (self.*func)(py_cast<Params>(vm, args[Is+1])...);
            return vm->None;
        }else{
            __Ret ret = (self.*func)(py_cast<Params>(vm, args[Is+1])...);
            return VAR(std::move(ret));
        }
    }
};

inline PyObject* proxy_wrapper(VM* vm, ArgsView args){
    NativeProxyFuncCBase* pf = lambda_get_userdata<NativeProxyFuncCBase*>(args.begin());
    return (*pf)(vm, args);
}

template<typename Ret, typename... Params>
void _bind(VM* vm, PyObject* obj, const char* sig, Ret(*func)(Params...)){
    auto proxy = new NativeProxyFuncC<Ret, Params...>(func);
    vm->bind(obj, sig, proxy_wrapper, proxy);
}

template<typename Ret, typename T, typename... Params>
void _bind(VM* vm, PyObject* obj, const char* sig, Ret(T::*func)(Params...)){
    auto proxy = new NativeProxyMethodC<Ret, T, Params...>(func);
    vm->bind(obj, sig, proxy_wrapper, proxy);
}
/*****************************************************************/
#define PY_FIELD(T, NAME, REF, EXPR)       \
        vm->bind_property(type, NAME,               \
            [](VM* vm, ArgsView args){              \
                T& self = PK_OBJ_GET(T, args[0]);   \
                return VAR(self.REF()->EXPR);       \
            },                                      \
            [](VM* vm, ArgsView args){              \
                T& self = PK_OBJ_GET(T, args[0]);   \
                self.REF()->EXPR = CAST(decltype(self.REF()->EXPR), args[1]);       \
                return vm->None;                                                    \
            });

#define PY_FIELD_P(T, NAME, EXPR)                   \
        vm->bind_property(type, NAME,               \
            [](VM* vm, ArgsView args){              \
                VoidP& self = PK_OBJ_GET(VoidP, args[0]);   \
                T* tgt = reinterpret_cast<T*>(self.ptr);    \
                return VAR(tgt->EXPR);                      \
            },                                      \
            [](VM* vm, ArgsView args){              \
                VoidP& self = PK_OBJ_GET(VoidP, args[0]);   \
                T* tgt = reinterpret_cast<T*>(self.ptr);    \
                tgt->EXPR = CAST(decltype(tgt->EXPR), args[1]);       \
                return vm->None;                                      \
            });

#define PY_READONLY_FIELD(T, NAME, REF, EXPR)          \
        vm->bind_property(type, NAME,                  \
            [](VM* vm, ArgsView args){              \
                T& self = PK_OBJ_GET(T, args[0]);   \
                return VAR(self.REF()->EXPR);       \
            });

#define PY_READONLY_FIELD_P(T, NAME, EXPR)          \
        vm->bind_property(type, NAME,                  \
            [](VM* vm, ArgsView args){              \
                VoidP& self = PK_OBJ_GET(VoidP, args[0]);   \
                T* tgt = reinterpret_cast<T*>(self.ptr);    \
                return VAR(tgt->EXPR);                      \
            });

#define PY_PROPERTY(T, NAME, REF, FGET, FSET)  \
        vm->bind_property(type, NAME,                   \
            [](VM* vm, ArgsView args){                  \
                T& self = PK_OBJ_GET(T, args[0]);       \
                return VAR(self.REF()->FGET());         \
            },                                          \
            [](VM* vm, ArgsView args){                  \
                T& self = _CAST(T&, args[0]);           \
                using __NT = decltype(self.REF()->FGET());   \
                self.REF()->FSET(CAST(__NT, args[1]));       \
                return vm->None;                            \
            });

#define PY_READONLY_PROPERTY(T, NAME, REF, FGET)  \
        vm->bind_property(type, NAME,                   \
            [](VM* vm, ArgsView args){                  \
                T& self = PK_OBJ_GET(T, args[0]);       \
                return VAR(self.REF()->FGET());         \
            });

#define PY_STRUCT_LIKE(wT)   \
        using vT = std::remove_pointer_t<decltype(std::declval<wT>()._())>;         \
        static_assert(std::is_trivially_copyable<vT>::value);                       \
        type->attr().set("__struct__", vm->True);                                   \
        vm->bind_func<1>(type, "from_struct", [](VM* vm, ArgsView args){            \
            C99Struct& s = CAST(C99Struct&, args[0]);                               \
            if(s.size != sizeof(vT)) vm->ValueError("size mismatch");               \
            PyObject* obj = vm->heap.gcnew<wT>(wT::_type(vm));                      \
            memcpy(_CAST(wT&, obj)._(), s.p, sizeof(vT));                           \
            return obj;                                                             \
        }, {}, BindType::STATICMETHOD);                                             \
        vm->bind_method<0>(type, "to_struct", [](VM* vm, ArgsView args){            \
            wT& self = _CAST(wT&, args[0]);                                         \
            return VAR_T(C99Struct, self._(), sizeof(vT));                          \
        });                                                                         \
        vm->bind_method<0>(type, "addr", [](VM* vm, ArgsView args){                 \
            wT& self = _CAST(wT&, args[0]);                                         \
            return VAR_T(VoidP, self._());                                          \
        });                                                                         \
        vm->bind_method<0>(type, "copy", [](VM* vm, ArgsView args){                 \
            wT& self = _CAST(wT&, args[0]);                                         \
            return VAR_T(wT, *self._());                                            \
        });                                                                         \
        vm->bind_method<0>(type, "sizeof", [](VM* vm, ArgsView args){               \
            return VAR(sizeof(vT));                                                 \
        });                                                                         \
        vm->bind__eq__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){  \
            wT& self = _CAST(wT&, _0);                                              \
            if(!vm->isinstance(_1, wT::_type(vm))) return vm->NotImplemented;       \
            wT& other = _CAST(wT&, _1);                                             \
            return VAR(self == other);                                              \
        });                                                                         \

#define PY_POINTER_SETGETITEM(T) \
        vm->bind__getitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){  \
            VoidP& self = PK_OBJ_GET(VoidP&, _0);                                       \
            i64 i = CAST(i64, _1);                                                      \
            T* tgt = reinterpret_cast<T*>(self.ptr);                                    \
            return VAR(tgt[i]);                                                         \
        });                                                                             \
        vm->bind__setitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1, PyObject* _2){  \
            VoidP& self = PK_OBJ_GET(VoidP&, _0);                                       \
            i64 i = CAST(i64, _1);                                                      \
            T* tgt = reinterpret_cast<T*>(self.ptr);                                    \
            tgt[i] = CAST(T, _2);                                                       \
        });                                                                         \

}   // namespace pkpy


namespace pkpy{

struct RangeIter{
    PY_CLASS(RangeIter, builtins, _range_iterator)
    Range r;
    i64 current;
    RangeIter(Range r) : r(r), current(r.start) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct ArrayIter{
    PY_CLASS(ArrayIter, builtins, _array_iterator)
    PyObject* ref;
    PyObject** begin;
    PyObject** end;
    PyObject** current;

    ArrayIter(PyObject* ref, PyObject** begin, PyObject** end)
        : ref(ref), begin(begin), end(end), current(begin) {}

    void _gc_mark() const{ PK_OBJ_MARK(ref); }
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct StringIter{
    PY_CLASS(StringIter, builtins, _string_iterator)
    PyObject* ref;
    Str* str;
    int index;      // byte index

    StringIter(PyObject* ref) : ref(ref), str(&PK_OBJ_GET(Str, ref)), index(0) {}

    void _gc_mark() const{ PK_OBJ_MARK(ref); }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct Generator{
    PY_CLASS(Generator, builtins, generator)
    Frame frame;
    int state;      // 0,1,2
    List s_backup;

    Generator(Frame&& frame, ArgsView buffer): frame(std::move(frame)), state(0) {
        for(PyObject* obj: buffer) s_backup.push_back(obj);
    }

    void _gc_mark() const{
        frame._gc_mark();
        for(PyObject* obj: s_backup) PK_OBJ_MARK(obj);
    }

    PyObject* next(VM* vm);
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

} // namespace pkpy
namespace pkpy{

    void RangeIter::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->_all_types[PK_OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<RangeIter>(type);
        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){ return obj; });
        vm->bind__next__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            RangeIter& self = _CAST(RangeIter&, obj);
            bool has_next = self.r.step > 0 ? self.current < self.r.stop : self.current > self.r.stop;
            if(!has_next) return vm->StopIteration;
            self.current += self.r.step;
            return VAR(self.current - self.r.step);
        });
    }

    void ArrayIter::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->_all_types[PK_OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<ArrayIter>(type);
        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){ return obj; });
        vm->bind__next__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            ArrayIter& self = _CAST(ArrayIter&, obj);
            if(self.current == self.end) return vm->StopIteration;
            return *self.current++;
        });
    }

    void StringIter::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->_all_types[PK_OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<StringIter>(type);
        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){ return obj; });
        vm->bind__next__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            StringIter& self = _CAST(StringIter&, obj);
            if(self.index == self.str->size) return vm->StopIteration;
            int start = self.index;
            int len = utf8len(self.str->data[self.index]);
            self.index += len;
            return VAR(self.str->substr(start, len));
        });
    }

    PyObject* Generator::next(VM* vm){
        if(state == 2) return vm->StopIteration;
        // reset frame._sp_base
        frame._sp_base = frame._s->_sp;
        frame._locals.a = frame._s->_sp;
        // restore the context
        for(PyObject* obj: s_backup) frame._s->push(obj);
        s_backup.clear();
        vm->callstack.push(std::move(frame));

        PyObject* ret;
        try{
            ret = vm->_run_top_frame();
        }catch(...){
            state = 2;      // end this generator immediately when an exception is thrown
            throw;
        }
        
        if(ret == PY_OP_YIELD){
            // backup the context
            frame = std::move(vm->callstack.top());
            ret = frame._s->popx();
            for(PyObject* obj: frame.stack_view()) s_backup.push_back(obj);
            vm->_pop_frame();
            state = 1;
            if(ret == vm->StopIteration) state = 2;
            return ret;
        }else{
            state = 2;
            return vm->StopIteration;
        }
    }

    void Generator::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->_all_types[PK_OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<Generator>(type);
        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){ return obj; });
        vm->bind__next__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            Generator& self = _CAST(Generator&, obj);
            return self.next(vm);
        });
    }

PyObject* VM::_py_generator(Frame&& frame, ArgsView buffer){
    return VAR_T(Generator, std::move(frame), buffer);
}

}   // namespace pkpy


namespace pkpy {

void add_module_base64(VM* vm);

} // namespace pkpy
namespace pkpy{

// https://github.com/zhicheng/base64/blob/master/base64.c

const char BASE64_PAD = '=';
const char BASE64DE_FIRST = '+';
const char BASE64DE_LAST = 'z';

/* BASE 64 encode table */
const char base64en[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/',
};

/* ASCII order for BASE 64 decode, 255 in unused character */
const unsigned char base64de[] = {
	/* nul, soh, stx, etx, eot, enq, ack, bel, */
	   255, 255, 255, 255, 255, 255, 255, 255,

	/*  bs,  ht,  nl,  vt,  np,  cr,  so,  si, */
	   255, 255, 255, 255, 255, 255, 255, 255,

	/* dle, dc1, dc2, dc3, dc4, nak, syn, etb, */
	   255, 255, 255, 255, 255, 255, 255, 255,

	/* can,  em, sub, esc,  fs,  gs,  rs,  us, */
	   255, 255, 255, 255, 255, 255, 255, 255,

	/*  sp, '!', '"', '#', '$', '%', '&', ''', */
	   255, 255, 255, 255, 255, 255, 255, 255,

	/* '(', ')', '*', '+', ',', '-', '.', '/', */
	   255, 255, 255,  62, 255, 255, 255,  63,

	/* '0', '1', '2', '3', '4', '5', '6', '7', */
	    52,  53,  54,  55,  56,  57,  58,  59,

	/* '8', '9', ':', ';', '<', '=', '>', '?', */
	    60,  61, 255, 255, 255, 255, 255, 255,

	/* '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', */
	   255,   0,   1,  2,   3,   4,   5,    6,

	/* 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', */
	     7,   8,   9,  10,  11,  12,  13,  14,

	/* 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', */
	    15,  16,  17,  18,  19,  20,  21,  22,

	/* 'X', 'Y', 'Z', '[', '\', ']', '^', '_', */
	    23,  24,  25, 255, 255, 255, 255, 255,

	/* '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', */
	   255,  26,  27,  28,  29,  30,  31,  32,

	/* 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', */
	    33,  34,  35,  36,  37,  38,  39,  40,

	/* 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', */
	    41,  42,  43,  44,  45,  46,  47,  48,

	/* 'x', 'y', 'z', '{', '|', '}', '~', del, */
	    49,  50,  51, 255, 255, 255, 255, 255
};

static unsigned int
base64_encode(const unsigned char *in, unsigned int inlen, char *out)
{
	int s;
	unsigned int i;
	unsigned int j;
	unsigned char c;
	unsigned char l;

	s = 0;
	l = 0;
	for (i = j = 0; i < inlen; i++) {
		c = in[i];

		switch (s) {
		case 0:
			s = 1;
			out[j++] = base64en[(c >> 2) & 0x3F];
			break;
		case 1:
			s = 2;
			out[j++] = base64en[((l & 0x3) << 4) | ((c >> 4) & 0xF)];
			break;
		case 2:
			s = 0;
			out[j++] = base64en[((l & 0xF) << 2) | ((c >> 6) & 0x3)];
			out[j++] = base64en[c & 0x3F];
			break;
		}
		l = c;
	}

	switch (s) {
	case 1:
		out[j++] = base64en[(l & 0x3) << 4];
		out[j++] = BASE64_PAD;
		out[j++] = BASE64_PAD;
		break;
	case 2:
		out[j++] = base64en[(l & 0xF) << 2];
		out[j++] = BASE64_PAD;
		break;
	}

	out[j] = 0;

	return j;
}

static unsigned int
base64_decode(const char *in, unsigned int inlen, unsigned char *out)
{
	unsigned int i;
	unsigned int j;
	unsigned char c;

	if (inlen & 0x3) {
		return 0;
	}

	for (i = j = 0; i < inlen; i++) {
		if (in[i] == BASE64_PAD) {
			break;
		}
		if (in[i] < BASE64DE_FIRST || in[i] > BASE64DE_LAST) {
			return 0;
		}

		c = base64de[(unsigned char)in[i]];
		if (c == 255) {
			return 0;
		}

		switch (i & 0x3) {
		case 0:
			out[j] = (c << 2) & 0xFF;
			break;
		case 1:
			out[j++] |= (c >> 4) & 0x3;
			out[j] = (c & 0xF) << 4; 
			break;
		case 2:
			out[j++] |= (c >> 2) & 0xF;
			out[j] = (c & 0x3) << 6;
			break;
		case 3:
			out[j++] |= c;
			break;
		}
	}

	return j;
}

void add_module_base64(VM* vm){
    PyObject* mod = vm->new_module("base64");

    // b64encode
    vm->bind_func<1>(mod, "b64encode", [](VM* vm, ArgsView args){
        Bytes& b = CAST(Bytes&, args[0]);
		unsigned char* p = new unsigned char[b.size() * 2];
        int size = base64_encode((const unsigned char*)b.data(), b.size(), (char*)p);
        return VAR(Bytes(p, size));
    });

    // b64decode
    vm->bind_func<1>(mod, "b64decode", [](VM* vm, ArgsView args){
        Bytes& b = CAST(Bytes&, args[0]);
        unsigned char* p = new unsigned char[b.size()];
        int size = base64_decode((const char*)b.data(), b.size(), p);
        return VAR(Bytes(p, size));
    });
}

}	// namespace pkpy


namespace pkpy {

void add_module_csv(VM* vm);

} // namespace pkpy
namespace pkpy{

void add_module_csv(VM *vm){
    PyObject* mod = vm->new_module("csv");

    vm->bind(mod, "reader(csvfile: list[str]) -> list[list]", [](VM* vm, ArgsView args){
        const List& csvfile = CAST(List&, args[0]);
        List ret;
        for(int i=0; i<csvfile.size(); i++){
            std::string_view line = CAST(Str&, csvfile[i]).sv();
            if(i == 0){
                // Skip utf8 BOM if there is any.
                if (strncmp(line.data(), "\xEF\xBB\xBF", 3) == 0) line = line.substr(3);
            }
            List row;
            int j;
            bool in_quote = false;
            std::string buffer;
__NEXT_LINE:
            j = 0;
            while(j < line.size()){
                switch(line[j]){
                    case '"':
                        if(in_quote){
                            if(j+1 < line.size() && line[j+1] == '"'){
                                buffer += '"';
                                j++;
                            }else{
                                in_quote = false;
                            }
                        }else{
                            in_quote = true;
                        }
                        break;
                    case ',':
                        if(in_quote){
                            buffer += line[j];
                        }else{
                            row.push_back(VAR(buffer));
                            buffer.clear();
                        }
                        break;
                    case '\r':
                        break;  // ignore
                    default:
                        buffer += line[j];
                        break;
                }
                j++;
            }
            if(in_quote){
                if(i == csvfile.size()-1){
                    vm->ValueError("unterminated quote");
                }else{
                    buffer += '\n';
                    i++;
                    line = CAST(Str&, csvfile[i]).sv();
                    goto __NEXT_LINE;
                }
            }
            row.push_back(VAR(buffer));
            ret.push_back(VAR(std::move(row)));
        }
        return VAR(std::move(ret));
    });

    vm->bind(mod, "DictReader(csvfile: list[str]) -> list[dict]", [](VM* vm, ArgsView args){
        PyObject* csv_reader = vm->_modules["csv"]->attr("reader");
        PyObject* ret_obj = vm->call(csv_reader, args[0]);
        const List& ret = CAST(List&, ret_obj);
        if(ret.size() == 0){
            vm->ValueError("empty csvfile");
        }
        List header = CAST(List&, ret[0]);
        List new_ret;
        for(int i=1; i<ret.size(); i++){
            const List& row = CAST(List&, ret[i]);
            if(row.size() != header.size()){
                vm->ValueError("row.size() != header.size()");
            }
            Dict row_dict(vm);
            for(int j=0; j<header.size(); j++){
                row_dict.set(header[j], row[j]);
            }
            new_ret.push_back(VAR(std::move(row_dict)));
        }
        return VAR(std::move(new_ret));
    });
}

}   // namespace pkpy


namespace pkpy
{
    void add_module_collections(VM *vm);
} // namespace pkpy
namespace pkpy
{
    struct PyDequeIter // Iterator for the deque type
    {
        PY_CLASS(PyDequeIter, collections, _deque_iterator)
        PyObject *ref;
        bool is_reversed;
        std::deque<PyObject *>::iterator begin, end, current;
        std::deque<PyObject *>::reverse_iterator rbegin, rend, rcurrent;
        PyDequeIter(PyObject *ref, std::deque<PyObject *>::iterator begin, std::deque<PyObject *>::iterator end)
            : ref(ref), begin(begin), end(end), current(begin)
        {
            this->is_reversed = false;
        }
        PyDequeIter(PyObject *ref, std::deque<PyObject *>::reverse_iterator rbegin, std::deque<PyObject *>::reverse_iterator rend)
            : ref(ref), rbegin(rbegin), rend(rend), rcurrent(rbegin)
        {
            this->is_reversed = true;
        }
        void _gc_mark() const { PK_OBJ_MARK(ref); }
        static void _register(VM *vm, PyObject *mod, PyObject *type);
    };
    void PyDequeIter::_register(VM *vm, PyObject *mod, PyObject *type)
    {
        // Iterator for the deque type
        vm->_all_types[PK_OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<PyDequeIter>(type);

        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject *obj)
                         { return obj; });
        vm->bind__next__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject *obj)
                         {
            PyDequeIter& self = _CAST(PyDequeIter&, obj);
            if(self.is_reversed){
                if(self.rcurrent == self.rend) return vm->StopIteration;
                PyObject* ret = *self.rcurrent;
                ++self.rcurrent;
                return ret;
            }
            else{
                if(self.current == self.end) return vm->StopIteration;
                PyObject* ret = *self.current;
                ++self.current;
                return ret;
            } });
    }
    struct PyDeque
    {
        PY_CLASS(PyDeque, collections, deque);
        PyDeque(VM *vm, PyObject *iterable, PyObject *maxlen); // constructor
        // PyDeque members
        std::deque<PyObject *> dequeItems;
        int maxlen = -1;                                                  // -1 means unbounded
        bool bounded = false;                                             // if true, maxlen is not -1
        void insertObj(bool front, bool back, int index, PyObject *item); // insert at index, used purely for internal purposes: append, appendleft, insert methods
        PyObject *popObj(bool front, bool back, PyObject *item, VM *vm);  // pop at index, used purely for internal purposes: pop, popleft, remove methods
        int findIndex(VM *vm, PyObject *obj, int start, int stop);        // find the index of the given object in the deque
        // Special methods
        static void _register(VM *vm, PyObject *mod, PyObject *type); // register the type
        void _gc_mark() const;                                        // needed for container types, mark all objects in the deque for gc
    };
    void PyDeque::_register(VM *vm, PyObject *mod, PyObject *type)
    {
        vm->bind(type, "__new__(cls, iterable=None, maxlen=None)",
                 [](VM *vm, ArgsView args)
                 {
                     Type cls_t = PK_OBJ_GET(Type, args[0]);
                     PyObject *iterable = args[1];
                     PyObject *maxlen = args[2];
                     return vm->heap.gcnew<PyDeque>(cls_t, vm, iterable, maxlen);
                 });
        // gets the item at the given index, if index is negative, it will be treated as index + len(deque)
        // if the index is out of range, IndexError will be thrown --> required for [] operator
        vm->bind__getitem__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject* _0, PyObject* _1)
        {
            PyDeque &self = _CAST(PyDeque &, _0);
            int index = CAST(int, _1);
            index = vm->normalized_index(index, self.dequeItems.size()); // error is handled by the vm->normalized_index
            return self.dequeItems.at(index);
        });
        // sets the item at the given index, if index is negative, it will be treated as index + len(deque)
        // if the index is out of range, IndexError will be thrown --> required for [] operator
        vm->bind__setitem__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject* _0, PyObject* _1, PyObject* _2)
        {
            PyDeque &self = _CAST(PyDeque&, _0);
            int index = CAST(int, _1);
            index = vm->normalized_index(index, self.dequeItems.size()); // error is handled by the vm->normalized_index
            self.dequeItems.at(index) = _2;
        });
        // erases the item at the given index, if index is negative, it will be treated as index + len(deque)
        // if the index is out of range, IndexError will be thrown --> required for [] operator
        vm->bind__delitem__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject* _0, PyObject* _1)
        {
            PyDeque &self = _CAST(PyDeque&, _0);
            int index = CAST(int, _1);
            index = vm->normalized_index(index, self.dequeItems.size()); // error is handled by the vm->normalized_index
            self.dequeItems.erase(self.dequeItems.begin() + index);
        });

        vm->bind__len__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject* _0)
        {
            PyDeque &self = _CAST(PyDeque&, _0);
            return (i64)self.dequeItems.size();
        });

        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject* _0)
        {
            PyDeque &self = _CAST(PyDeque &, _0);
            return vm->heap.gcnew<PyDequeIter>(
                PyDequeIter::_type(vm), _0,
                self.dequeItems.begin(), self.dequeItems.end());
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject* _0)
        {
            if(vm->_repr_recursion_set.count(_0)) return VAR("[...]");
            const PyDeque &self = _CAST(PyDeque&, _0);
            SStream ss;
            ss << "deque([";
            vm->_repr_recursion_set.insert(_0);
            for (auto it = self.dequeItems.begin(); it != self.dequeItems.end(); ++it)
            {
                ss << CAST(Str&, vm->py_repr(*it));
                if (it != self.dequeItems.end() - 1) ss << ", ";
            }
            vm->_repr_recursion_set.erase(_0);
            self.bounded ? ss << "], maxlen=" << self.maxlen << ")" : ss << "])";
            return VAR(ss.str());
        });

        // enables comparison between two deques, == and != are supported
        vm->bind__eq__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject* _0, PyObject* _1)
        {
            const PyDeque &self = _CAST(PyDeque&, _0);
            if(!is_non_tagged_type(_0, PyDeque::_type(vm))) return vm->NotImplemented;
            const PyDeque &other = _CAST(PyDeque&, _1);
            if (self.dequeItems.size() != other.dequeItems.size()) return vm->False;
            for (int i = 0; i < self.dequeItems.size(); i++){
                if (vm->py_ne(self.dequeItems[i], other.dequeItems[i])) return vm->False;
            }
            return vm->True;
        });

        // clear the deque
        vm->bind(type, "clear(self) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     self.dequeItems.clear();
                     return vm->None;
                 });
        // extend the deque with the given iterable
        vm->bind(type, "extend(self, iterable) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     auto _lock = vm->heap.gc_scope_lock(); // locking the heap
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *it = vm->py_iter(args[1]); // strong ref
                     PyObject *obj = vm->py_next(it);
                     while (obj != vm->StopIteration)
                     {
                         self.insertObj(false, true, -1, obj);
                         obj = vm->py_next(it);
                     }
                     return vm->None;
                 });
        // append at the end of the deque
        vm->bind(type, "append(self, item) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *item = args[1];
                     self.insertObj(false, true, -1, item);
                     return vm->None;
                 });
        // append at the beginning of the deque
        vm->bind(type, "appendleft(self, item) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *item = args[1];
                     self.insertObj(true, false, -1, item);
                     return vm->None;
                 });
        // pop from the end of the deque
        vm->bind(type, "pop(self) -> PyObject",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     if (self.dequeItems.empty())
                     {
                         vm->IndexError("pop from an empty deque");
                         return vm->None;
                     }
                     return self.popObj(false, true, nullptr, vm);
                 });
        // pop from the beginning of the deque
        vm->bind(type, "popleft(self) -> PyObject",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     if (self.dequeItems.empty())
                     {
                         vm->IndexError("pop from an empty deque");
                         return vm->None;
                     }
                     return self.popObj(true, false, nullptr, vm);
                 });
        // shallow copy of the deque
        vm->bind(type, "copy(self) -> deque",
                 [](VM *vm, ArgsView args)
                 {
                     auto _lock = vm->heap.gc_scope_lock(); // locking the heap
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *newDequeObj = vm->heap.gcnew<PyDeque>(PyDeque::_type(vm), vm, vm->None, vm->None); // create the empty deque
                     PyDeque &newDeque = _CAST(PyDeque &, newDequeObj);                                           // cast it to PyDeque so we can use its methods
                     for (auto it = self.dequeItems.begin(); it != self.dequeItems.end(); ++it)
                         newDeque.insertObj(false, true, -1, *it);
                     return newDequeObj;
                 });
        // NEW: counts the number of occurences of the given object in the deque
        vm->bind(type, "count(self, obj) -> int",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *obj = args[1];
                     int cnt = 0, sz = self.dequeItems.size();
                     for (auto it = self.dequeItems.begin(); it != self.dequeItems.end(); ++it)
                     {
                         if (vm->py_eq((*it), obj))
                             cnt++;
                         if (sz != self.dequeItems.size())// mutating the deque during iteration is not allowed
                             vm->RuntimeError("deque mutated during iteration"); 
                     }
                     return VAR(cnt);
                 });
        // NEW: extends the deque from the left
        vm->bind(type, "extendleft(self, iterable) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     auto _lock = vm->heap.gc_scope_lock();
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *it = vm->py_iter(args[1]); // strong ref
                     PyObject *obj = vm->py_next(it);
                     while (obj != vm->StopIteration)
                     {
                         self.insertObj(true, false, -1, obj);
                         obj = vm->py_next(it);
                     }
                     return vm->None;
                 });
        // NEW: returns the index of the given object in the deque
        vm->bind(type, "index(self, obj, start=None, stop=None) -> int",
                 [](VM *vm, ArgsView args)
                 {
                     // Return the position of x in the deque (at or after index start and before index stop). Returns the first match or raises ValueError if not found.
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *obj = args[1];
                     int start = 0, stop = self.dequeItems.size(); // default values
                     if (!vm->py_eq(args[2], vm->None))
                         start = CAST(int, args[2]);
                     if (!vm->py_eq(args[3], vm->None))
                         stop = CAST(int, args[3]);
                     int index = self.findIndex(vm, obj, start, stop);
                     if (index != -1)
                         return VAR(index);
                     else
                         vm->ValueError(_CAST(Str &, vm->py_repr(obj)) + " is not in deque");
                     return vm->None;
                 });
        // NEW: returns the index of the given object in the deque
        vm->bind(type, "__contains__(self, obj) -> bool",
                 [](VM *vm, ArgsView args)
                 {
                     // Return the position of x in the deque (at or after index start and before index stop). Returns the first match or raises ValueError if not found.
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *obj = args[1];
                     int start = 0, stop = self.dequeItems.size(); // default values
                     int index = self.findIndex(vm, obj, start, stop);
                     if (index != -1)
                         return VAR(true);
                     return VAR(false);
                 });
        // NEW: inserts the given object at the given index
        vm->bind(type, "insert(self, index, obj) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     int index = CAST(int, args[1]);
                     PyObject *obj = args[2];
                     if (self.bounded && self.dequeItems.size() == self.maxlen)
                         vm->IndexError("deque already at its maximum size");
                     else
                         self.insertObj(false, false, index, obj); // this index shouldn't be fixed using vm->normalized_index, pass as is
                     return vm->None;
                 });
        // NEW: removes the first occurence of the given object from the deque
        vm->bind(type, "remove(self, obj) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *obj = args[1];
                     PyObject *removed = self.popObj(false, false, obj, vm);
                     if (removed == nullptr)
                         vm->ValueError(_CAST(Str &, vm->py_repr(obj)) + " is not in list");
                     return vm->None;
                 });
        // NEW: reverses the deque
        vm->bind(type, "reverse(self) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     if (self.dequeItems.empty() || self.dequeItems.size() == 1)
                         return vm->None; // handle trivial cases
                     int sz = self.dequeItems.size();
                     for (int i = 0; i < sz / 2; i++)
                     {
                         PyObject *tmp = self.dequeItems[i];
                         self.dequeItems[i] = self.dequeItems[sz - i - 1]; // swapping
                         self.dequeItems[sz - i - 1] = tmp;
                     }
                     return vm->None;
                 });
        // NEW: rotates the deque by n steps
        vm->bind(type, "rotate(self, n=1) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     int n = CAST(int, args[1]);

                     if (n != 0 && !self.dequeItems.empty()) // trivial case
                     {
                         PyObject *tmp; // holds the object to be rotated
                         int direction = n > 0 ? 1 : -1;
                         n = abs(n);
                         n = n % self.dequeItems.size(); // make sure n is in range
                         while (n--)
                         {
                             if (direction == 1)
                             {
                                 tmp = self.dequeItems.back();
                                 self.dequeItems.pop_back();
                                 self.dequeItems.push_front(tmp);
                             }
                             else
                             {
                                 tmp = self.dequeItems.front();
                                 self.dequeItems.pop_front();
                                 self.dequeItems.push_back(tmp);
                             }
                         }
                     }
                     return vm->None;
                 });
        // NEW: getter and setter of property `maxlen`
        vm->bind_property(
            type, "maxlen: int",
            [](VM *vm, ArgsView args)
            {
                PyDeque &self = _CAST(PyDeque &, args[0]);
                if (self.bounded)
                    return VAR(self.maxlen);
                return vm->None;
            },
            [](VM *vm, ArgsView args)
            {
                vm->AttributeError("attribute 'maxlen' of 'collections.deque' objects is not writable");
                return vm->None;
            });
        // NEW: support pickle
        vm->bind(type, "__getnewargs__(self) -> tuple[list, int]",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     Tuple ret(2);
                     List list;
                     for (PyObject *obj : self.dequeItems)
                     {
                         list.push_back(obj);
                     }
                     ret[0] = VAR(std::move(list));
                     if (self.bounded)
                         ret[1] = VAR(self.maxlen);
                     else
                         ret[1] = vm->None;
                     return VAR(ret);
                 });
    }
    /// @brief initializes a new PyDeque object, actual initialization is done in __init__
    PyDeque::PyDeque(VM *vm, PyObject *iterable, PyObject *maxlen)
    {

        if (!vm->py_eq(maxlen, vm->None)) // fix the maxlen first
        {
            int tmp = CAST(int, maxlen);
            if (tmp < 0)
                vm->ValueError("maxlen must be non-negative");
            else
            {
                this->maxlen = tmp;
                this->bounded = true;
            }
        }
        else
        {
            this->bounded = false;
            this->maxlen = -1;
        }
        if (!vm->py_eq(iterable, vm->None))
        {
            this->dequeItems.clear();              // clear the deque
            auto _lock = vm->heap.gc_scope_lock(); // locking the heap
            PyObject *it = vm->py_iter(iterable);  // strong ref
            PyObject *obj = vm->py_next(it);
            while (obj != vm->StopIteration)
            {
                this->insertObj(false, true, -1, obj);
                obj = vm->py_next(it);
            }
        }
    }
    int PyDeque::findIndex(VM *vm, PyObject *obj, int start, int stop)
    {
        // the following code is special purpose normalization for this method, taken from CPython: _collectionsmodule.c file
        if (start < 0)
        {
            start = this->dequeItems.size() + start; // try to fix for negative indices
            if (start < 0)
                start = 0;
        }
        if (stop < 0)
        {
            stop = this->dequeItems.size() + stop; // try to fix for negative indices
            if (stop < 0)
                stop = 0;
        }
        if (stop > this->dequeItems.size())
            stop = this->dequeItems.size();
        if (start > stop)
            start = stop;                                                                                                           // end of normalization
        PK_ASSERT(start >= 0 && start <= this->dequeItems.size() && stop >= 0 && stop <= this->dequeItems.size() && start <= stop); // sanity check
        int loopSize = std::min((int)(this->dequeItems.size()), stop);
        int sz = this->dequeItems.size();
        for (int i = start; i < loopSize; i++)
        {
            if (vm->py_eq(this->dequeItems[i], obj))
                return i;
            if (sz != this->dequeItems.size())// mutating the deque during iteration is not allowed
                vm->RuntimeError("deque mutated during iteration");
        }
        return -1;
    }

    /// @brief pops or removes an item from the deque
    /// @param front  if true, pop from the front of the deque
    /// @param back if true, pop from the back of the deque
    /// @param item if front and back is not set, remove the first occurence of item from the deque
    /// @param vm is needed for the py_eq
    /// @return PyObject* if front or back is set, this is a pop operation and we return a PyObject*, if front and back are not set, this is a remove operation and we return the removed item or nullptr
    PyObject *PyDeque::popObj(bool front, bool back, PyObject *item, VM *vm)
    {
        // error handling
        if (front && back)
            throw std::runtime_error("both front and back are set"); // this should never happen
        if (front || back)
        {
            // front or back is set, we don't care about item, this is a pop operation and we return a PyObject*
            if (this->dequeItems.empty())
                throw std::runtime_error("pop from an empty deque"); // shouldn't happen
            PyObject *obj;
            if (front)
            {
                obj = this->dequeItems.front();
                this->dequeItems.pop_front();
            }
            else
            {
                obj = this->dequeItems.back();
                this->dequeItems.pop_back();
            }
            return obj;
        }
        else
        {
            // front and back are not set, we care about item, this is a remove operation and we return the removed item or nullptr
            int sz = this->dequeItems.size();
            for (auto it = this->dequeItems.begin(); it != this->dequeItems.end(); ++it)
            {
                bool found = vm->py_eq((*it), item);
                if (sz != this->dequeItems.size()) // mutating the deque during iteration is not allowed
                    vm->IndexError("deque mutated during iteration");
                if (found)
                {
                    PyObject *obj = *it; // keep a reference to the object for returning
                    this->dequeItems.erase(it);
                    return obj;
                }
            }
            return nullptr; // not found
        }
    }
    /// @brief inserts an item into the deque
    /// @param front if true, insert at the front of the deque
    /// @param back if true, insert at the back of the deque
    /// @param index if front and back are not set, insert at the given index
    /// @param item the item to insert
    /// @return true if the item was inserted successfully, false if the deque is bounded and is already at its maximum size
    void PyDeque::insertObj(bool front, bool back, int index, PyObject *item) // assume index is not fixed using the vm->normalized_index
    {
        // error handling
        if (front && back)
            throw std::runtime_error("both front and back are set"); // this should never happen
        if (front || back)
        {
            // front or back is set, we don't care about index
            if (this->bounded)
            {
                if (this->maxlen == 0)
                    return; // bounded and maxlen is 0, so we can't append
                else if (this->dequeItems.size() == this->maxlen)
                {
                    if (front)
                        this->dequeItems.pop_back(); // remove the last item
                    else if (back)
                        this->dequeItems.pop_front(); // remove the first item
                }
            }
            if (front)
                this->dequeItems.emplace_front(item);
            else if (back)
                this->dequeItems.emplace_back(item);
        }
        else
        {
            // front and back are not set, we care about index
            if (index < 0)
                index = this->dequeItems.size() + index; // try fixing for negative indices
            if (index < 0)                               // still negative means insert at the beginning
                this->dequeItems.push_front(item);
            else if (index >= this->dequeItems.size()) // still out of range means insert at the end
                this->dequeItems.push_back(item);
            else
                this->dequeItems.insert((this->dequeItems.begin() + index), item); // insert at the given index
        }
    }
    /// @brief marks the deque items for garbage collection
    void PyDeque::_gc_mark() const
    {
        for (PyObject *obj : this->dequeItems)
            PK_OBJ_MARK(obj);
    }
    /// @brief registers the PyDeque class
    /// @param vm is needed for the new_module and register_class
    void add_module_collections(VM *vm)
    {
        PyObject *mod = vm->new_module("collections");
        PyDeque::register_class(vm, mod);
        PyDequeIter::register_class(vm, mod);
        CodeObject_ code = vm->compile(kPythonLibs["collections"], "collections.py", EXEC_MODE);
        vm->_exec(code, mod);
    }
} // namespace pkpypkpy



namespace pkpy {

void add_module_array2d(VM* vm);

} // namespace pkpy
namespace pkpy{

struct Array2d{
    PK_ALWAYS_PASS_BY_POINTER(Array2d)
    PY_CLASS(Array2d, array2d, array2d)

    PyObject** data;
    int n_cols;
    int n_rows;
    int numel;

    Array2d(){
        data = nullptr;
        n_cols = 0;
        n_rows = 0;
        numel = 0;
    }

    Array2d* _() { return this; }

    void init(int n_cols, int n_rows){
        this->n_cols = n_cols;
        this->n_rows = n_rows;
        this->numel = n_cols * n_rows;
        this->data = new PyObject*[numel];
    }

    bool is_valid(int col, int row) const{
        return 0 <= col && col < n_cols && 0 <= row && row < n_rows;
    }

    PyObject* _get(int col, int row){
        return data[row * n_cols + col];
    }

    void _set(int col, int row, PyObject* value){
        data[row * n_cols + col] = value;
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind(type, "__new__(cls, *args, **kwargs)", [](VM* vm, ArgsView args){
            Type cls = PK_OBJ_GET(Type, args[0]);
            return vm->heap.gcnew<Array2d>(cls);
        });

        vm->bind(type, "__init__(self, n_cols: int, n_rows: int, default=None)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            int n_cols = CAST(int, args[1]);
            int n_rows = CAST(int, args[2]);
            if(n_cols <= 0 || n_rows <= 0){
                vm->ValueError("n_cols and n_rows must be positive integers");
            }
            self.init(n_cols, n_rows);
            if(vm->py_callable(args[3])){
                for(int i = 0; i < self.numel; i++) self.data[i] = vm->call(args[3]);
            }else{
                for(int i = 0; i < self.numel; i++) self.data[i] = args[3];
            }
            return vm->None;
        });

        PY_READONLY_FIELD(Array2d, "n_cols", _, n_cols);
        PY_READONLY_FIELD(Array2d, "n_rows", _, n_rows);
        PY_READONLY_FIELD(Array2d, "width", _, n_cols);
        PY_READONLY_FIELD(Array2d, "height", _, n_rows);
        PY_READONLY_FIELD(Array2d, "numel", _, numel);

        vm->bind(type, "is_valid(self, col: int, row: int)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            int col = CAST(int, args[1]);
            int row = CAST(int, args[2]);
            return VAR(self.is_valid(col, row));
        });

        vm->bind(type, "get(self, col: int, row: int, default=None)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            int col = CAST(int, args[1]);
            int row = CAST(int, args[2]);
            if(!self.is_valid(col, row)) return args[3];
            return self._get(col, row);
        });

        vm->bind__getitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            const Tuple& xy = CAST(Tuple&, _1);
            int col = CAST(int, xy[0]);
            int row = CAST(int, xy[1]);
            if(!self.is_valid(col, row)){
                vm->IndexError(_S('(', col, ", ", row, ')', " is not a valid index for array2d(", self.n_cols, ", ", self.n_rows, ')'));
            }
            return self._get(col, row);
        });

        vm->bind__setitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1, PyObject* _2){
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            const Tuple& xy = CAST(Tuple&, _1);
            int col = CAST(int, xy[0]);
            int row = CAST(int, xy[1]);
            if(!self.is_valid(col, row)){
                vm->IndexError(_S('(', col, ", ", row, ')', " is not a valid index for array2d(", self.n_cols, ", ", self.n_rows, ')'));
            }
            self._set(col, row, _2);
        });

        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0){
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            List t(self.n_rows);
            List row(self.n_cols);
            for(int j = 0; j < self.n_rows; j++){
                for(int i = 0; i < self.n_cols; i++) row[i] = self._get(i, j);
                t[j] = VAR(row);    // copy
            }
            return vm->py_iter(VAR(std::move(t)));
        });

        vm->bind__len__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0){
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            return (i64)self.n_rows;
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0){
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            return VAR(_S("array2d(", self.n_cols, ", ", self.n_rows, ')'));
        });

        vm->bind(type, "map(self, f)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            PyObject* f = args[1];
            PyObject* new_array_obj = vm->heap.gcnew<Array2d>(Array2d::_type(vm));
            Array2d& new_array = PK_OBJ_GET(Array2d, new_array_obj);
            new_array.init(self.n_cols, self.n_rows);
            for(int i = 0; i < new_array.numel; i++){
                new_array.data[i] = vm->call(f, self.data[i]);
            }
            return new_array_obj;
        });

        vm->bind(type, "copy(self)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            PyObject* new_array_obj = vm->heap.gcnew<Array2d>(Array2d::_type(vm));
            Array2d& new_array = PK_OBJ_GET(Array2d, new_array_obj);
            new_array.init(self.n_cols, self.n_rows);
            for(int i = 0; i < new_array.numel; i++){
                new_array.data[i] = self.data[i];
            }
            return new_array_obj;
        });

        vm->bind(type, "fill_(self, value)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]); 
            for(int i = 0; i < self.numel; i++){
                self.data[i] = args[1];
            }
            return vm->None;
        });

        vm->bind(type, "apply_(self, f)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            PyObject* f = args[1];
            for(int i = 0; i < self.numel; i++){
                self.data[i] = vm->call(f, self.data[i]);
            }
            return vm->None;
        });

        vm->bind(type, "copy_(self, other)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            if(is_non_tagged_type(args[1], VM::tp_list)){
                const List& list = PK_OBJ_GET(List, args[1]);
                if(list.size() != self.numel){
                    vm->ValueError("list size must be equal to the number of elements in the array2d");
                }
                for(int i = 0; i < self.numel; i++){
                    self.data[i] = list[i];
                }
                return vm->None;
            }
            Array2d& other = CAST(Array2d&, args[1]);
            // if self and other have different sizes, re-initialize self
            if(self.n_cols != other.n_cols || self.n_rows != other.n_rows){
                delete self.data;
                self.init(other.n_cols, other.n_rows);
            }
            for(int i = 0; i < self.numel; i++){
                self.data[i] = other.data[i];
            }
            return vm->None;
        });

        vm->bind__eq__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            if(!is_non_tagged_type(_1, Array2d::_type(vm))) return vm->NotImplemented;
            Array2d& other = PK_OBJ_GET(Array2d, _1);
            if(self.n_cols != other.n_cols || self.n_rows != other.n_rows) return vm->False;
            for(int i = 0; i < self.numel; i++){
                if(vm->py_ne(self.data[i], other.data[i])) return vm->False;
            }
            return vm->True;
        });

        // for cellular automata
        vm->bind(type, "count_neighbors(self, value) -> array2d[int]", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            PyObject* new_array_obj = vm->heap.gcnew<Array2d>(Array2d::_type(vm));
            Array2d& new_array = PK_OBJ_GET(Array2d, new_array_obj);
            new_array.init(self.n_cols, self.n_rows);
            PyObject* value = args[1];
            for(int j = 0; j < new_array.n_rows; j++){
                for(int i = 0; i < new_array.n_cols; i++){
                    int count = 0;
                    count += self.is_valid(i-1, j-1) && vm->py_eq(self._get(i-1, j-1), value);
                    count += self.is_valid(i, j-1) && vm->py_eq(self._get(i, j-1), value);
                    count += self.is_valid(i+1, j-1) && vm->py_eq(self._get(i+1, j-1), value);
                    count += self.is_valid(i-1, j) && vm->py_eq(self._get(i-1, j), value);
                    count += self.is_valid(i+1, j) && vm->py_eq(self._get(i+1, j), value);
                    count += self.is_valid(i-1, j+1) && vm->py_eq(self._get(i-1, j+1), value);
                    count += self.is_valid(i, j+1) && vm->py_eq(self._get(i, j+1), value);
                    count += self.is_valid(i+1, j+1) && vm->py_eq(self._get(i+1, j+1), value);
                    new_array._set(i, j, VAR(count));
                }
            }
            return new_array_obj; 
        });
    }

    void _gc_mark() const{
        for(int i = 0; i < numel; i++) PK_OBJ_MARK(data[i]);
    }

    ~Array2d(){
        delete[] data;
    }
};

void add_module_array2d(VM* vm){
    PyObject* mod = vm->new_module("array2d");

    Array2d::register_class(vm, mod);
}


}   // namespace pkpy


namespace pkpy{

void add_module_dataclasses(VM* vm);

} // namespace pkpy
namespace pkpy{

static void patch__init__(VM* vm, Type cls){
    vm->bind(vm->_t(cls), "__init__(self, *args, **kwargs)", [](VM* vm, ArgsView _view){
        PyObject* self = _view[0];
        const Tuple& args = CAST(Tuple&, _view[1]);
        const Dict& kwargs_ = CAST(Dict&, _view[2]);
        NameDict kwargs;
        kwargs_.apply([&](PyObject* k, PyObject* v){
            kwargs.set(CAST(Str&, k), v);
        });

        Type cls = vm->_tp(self);
        const PyTypeInfo* cls_info = &vm->_all_types[cls];
        NameDict& cls_d = cls_info->obj->attr();
        const auto& fields = cls_info->annotated_fields;

        int i = 0; // index into args
        for(StrName field: fields){
            if(kwargs.contains(field)){
                self->attr().set(field, kwargs[field]);
                kwargs.del(field);
            }else{
                if(i < args.size()){
                    self->attr().set(field, args[i]);
                    ++i;
                }else if(cls_d.contains(field)){    // has default value
                    self->attr().set(field, cls_d[field]);
                }else{
                    vm->TypeError(_S(cls_info->name, " missing required argument ", field.escape()));
                }
            }
        }
        if(args.size() > i){
            vm->TypeError(_S(cls_info->name, " takes ", fields.size(), " positional arguments but ", args.size(), " were given"));
        }
        if(kwargs.size() > 0){
            StrName unexpected_key = kwargs.items()[0].first;
            vm->TypeError(_S(cls_info->name, " got an unexpected keyword argument ", unexpected_key.escape()));
        }
        return vm->None;
    });
}

static void patch__repr__(VM* vm, Type cls){
    vm->bind__repr__(cls, [](VM* vm, PyObject* _0){
        auto _lock = vm->heap.gc_scope_lock();
        const PyTypeInfo* cls_info = &vm->_all_types[vm->_tp(_0)];
        const auto& fields = cls_info->annotated_fields;
        const NameDict& obj_d = _0->attr();
        SStream ss;
        ss << cls_info->name << "(";
        bool first = true;
        for(StrName field: fields){
            if(first) first = false;
            else ss << ", ";
            ss << field << "=" << CAST(Str&, vm->py_repr(obj_d[field]));
        }
        ss << ")";
        return VAR(ss.str());
    });
}

static void patch__eq__(VM* vm, Type cls){
    vm->bind__eq__(cls, [](VM* vm, PyObject* _0, PyObject* _1){
        if(vm->_tp(_0) != vm->_tp(_1)) return vm->NotImplemented;
        const PyTypeInfo* cls_info = &vm->_all_types[vm->_tp(_0)];
        const auto& fields = cls_info->annotated_fields;
        for(StrName field: fields){
            PyObject* lhs = _0->attr(field);
            PyObject* rhs = _1->attr(field);
            if(vm->py_ne(lhs, rhs)) return vm->False;
        }
        return vm->True;
    });
}

void add_module_dataclasses(VM* vm){
    PyObject* mod = vm->new_module("dataclasses");

    vm->bind_func<1>(mod, "dataclass", [](VM* vm, ArgsView args){
        vm->check_non_tagged_type(args[0], VM::tp_type);
        Type cls = PK_OBJ_GET(Type, args[0]);
        NameDict& cls_d = args[0]->attr();

        if(!cls_d.contains("__init__")) patch__init__(vm, cls);
        if(!cls_d.contains("__repr__")) patch__repr__(vm, cls);
        if(!cls_d.contains("__eq__")) patch__eq__(vm, cls);

        const auto& fields = vm->_all_types[cls].annotated_fields;
        bool has_default = false;
        for(StrName field: fields){
            if(cls_d.contains(field)){
                has_default = true;
            }else{
                if(has_default){
                    vm->TypeError(_S("non-default argument ", field.escape(), " follows default argument"));
                }
            }
        }
        return args[0];
    });

    vm->bind_func<1>(mod, "asdict", [](VM* vm, ArgsView args){
        const auto& fields = vm->_inst_type_info(args[0])->annotated_fields;
        const NameDict& obj_d = args[0]->attr();
        Dict d(vm);
        for(StrName field: fields){
            d.set(VAR(field.sv()), obj_d[field]);
        }
        return VAR(std::move(d));
    });
}

}   // namespace pkpy


namespace pkpy{

void add_module_random(VM* vm);

} // namespace pkpy
namespace pkpy{

struct Random{
    PY_CLASS(Random, random, Random)
    std::mt19937 gen;

    Random(){
        gen.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_default_constructor<Random>(type);

        vm->bind_method<1>(type, "seed", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            self.gen.seed(CAST(i64, args[1]));
            return vm->None;
        });

        vm->bind_method<2>(type, "randint", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            i64 a = CAST(i64, args[1]);
            i64 b = CAST(i64, args[2]);
            std::uniform_int_distribution<i64> dis(a, b);
            return VAR(dis(self.gen));
        });

        vm->bind_method<0>(type, "random", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            std::uniform_real_distribution<f64> dis(0.0, 1.0);
            return VAR(dis(self.gen));
        });

        vm->bind_method<2>(type, "uniform", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            f64 a = CAST(f64, args[1]);
            f64 b = CAST(f64, args[2]);
            std::uniform_real_distribution<f64> dis(a, b);
            return VAR(dis(self.gen));
        });

        vm->bind_method<1>(type, "shuffle", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            List& L = CAST(List&, args[1]);
            std::shuffle(L.begin(), L.end(), self.gen);
            return vm->None;
        });

        vm->bind_method<1>(type, "choice", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            const List& L = CAST(List&, args[1]);
            std::uniform_int_distribution<i64> dis(0, L.size() - 1);
            return L[dis(self.gen)];
        });
    }
};

void add_module_random(VM* vm){
    PyObject* mod = vm->new_module("random");
    Random::register_class(vm, mod);
    PyObject* instance = vm->heap.gcnew<Random>(Random::_type(vm));
    mod->attr().set("seed", vm->getattr(instance, "seed"));
    mod->attr().set("random", vm->getattr(instance, "random"));
    mod->attr().set("uniform", vm->getattr(instance, "uniform"));
    mod->attr().set("randint", vm->getattr(instance, "randint"));
    mod->attr().set("shuffle", vm->getattr(instance, "shuffle"));
    mod->attr().set("choice", vm->getattr(instance, "choice"));
}

}   // namespace pkpy


namespace pkpy{

inline bool isclose(float a, float b){ return std::fabs(a - b) <= NumberTraits<4>::kEpsilon; }

struct Vec2{
    float x, y;
    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2(const Vec2& v) = default;

    Vec2 operator+(const Vec2& v) const { return Vec2(x + v.x, y + v.y); }
    Vec2 operator-(const Vec2& v) const { return Vec2(x - v.x, y - v.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
    Vec2 operator*(const Vec2& v) const { return Vec2(x * v.x, y * v.y); }
    Vec2 operator/(float s) const { return Vec2(x / s, y / s); }
    Vec2 operator-() const { return Vec2(-x, -y); }
    bool operator==(const Vec2& v) const { return isclose(x, v.x) && isclose(y, v.y); }
    bool operator!=(const Vec2& v) const { return !isclose(x, v.x) || !isclose(y, v.y); }
    float dot(const Vec2& v) const { return x * v.x + y * v.y; }
    float cross(const Vec2& v) const { return x * v.y - y * v.x; }
    float length() const { return sqrtf(x * x + y * y); }
    float length_squared() const { return x * x + y * y; }
    Vec2 normalize() const { float l = length(); return Vec2(x / l, y / l); }
    Vec2 rotate(float radian) const { float cr = cosf(radian), sr = sinf(radian); return Vec2(x * cr - y * sr, x * sr + y * cr); }
    NoReturn normalize_() { float l = length(); x /= l; y /= l; return {}; }
    NoReturn copy_(const Vec2& v) { x = v.x; y = v.y; return {}; }
};

struct Vec3{
    float x, y, z;
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3(const Vec3& v) = default;

    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator*(const Vec3& v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
    Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    bool operator==(const Vec3& v) const { return isclose(x, v.x) && isclose(y, v.y) && isclose(z, v.z); }
    bool operator!=(const Vec3& v) const { return !isclose(x, v.x) || !isclose(y, v.y) || !isclose(z, v.z); }
    float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3 cross(const Vec3& v) const { return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
    float length() const { return sqrtf(x * x + y * y + z * z); }
    float length_squared() const { return x * x + y * y + z * z; }
    Vec3 normalize() const { float l = length(); return Vec3(x / l, y / l, z / l); }
    NoReturn normalize_() { float l = length(); x /= l; y /= l; z /= l; return {}; }
    NoReturn copy_(const Vec3& v) { x = v.x; y = v.y; z = v.z; return {}; }
};

struct Vec4{
    float x, y, z, w;
    Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vec4(const Vec4& v) = default;

    Vec4 operator+(const Vec4& v) const { return Vec4(x + v.x, y + v.y, z + v.z, w + v.w); }
    Vec4 operator-(const Vec4& v) const { return Vec4(x - v.x, y - v.y, z - v.z, w - v.w); }
    Vec4 operator*(float s) const { return Vec4(x * s, y * s, z * s, w * s); }
    Vec4 operator*(const Vec4& v) const { return Vec4(x * v.x, y * v.y, z * v.z, w * v.w); }
    Vec4 operator/(float s) const { return Vec4(x / s, y / s, z / s, w / s); }
    Vec4 operator-() const { return Vec4(-x, -y, -z, -w); }
    bool operator==(const Vec4& v) const { return isclose(x, v.x) && isclose(y, v.y) && isclose(z, v.z) && isclose(w, v.w); }
    bool operator!=(const Vec4& v) const { return !isclose(x, v.x) || !isclose(y, v.y) || !isclose(z, v.z) || !isclose(w, v.w); }
    float dot(const Vec4& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
    float length() const { return sqrtf(x * x + y * y + z * z + w * w); }
    float length_squared() const { return x * x + y * y + z * z + w * w; }
    Vec4 normalize() const { float l = length(); return Vec4(x / l, y / l, z / l, w / l); }
    NoReturn normalize_() { float l = length(); x /= l; y /= l; z /= l; w /= l; return {}; }
    NoReturn copy_(const Vec4& v) { x = v.x; y = v.y; z = v.z; w = v.w; return {}; }
};

struct Mat3x3{    
    union {
        struct {
            float        _11, _12, _13;
            float        _21, _22, _23;
            float        _31, _32, _33;
        };
        float m[3][3];
        float v[9];
    };

    Mat3x3();
    Mat3x3(float, float, float, float, float, float, float, float, float);
    Mat3x3(const Mat3x3& other) = default;

    static Mat3x3 zeros();
    static Mat3x3 ones();
    static Mat3x3 identity();

    Mat3x3 operator+(const Mat3x3& other) const;
    Mat3x3 operator-(const Mat3x3& other) const;
    Mat3x3 operator*(float scalar) const;
    Mat3x3 operator/(float scalar) const;

    bool operator==(const Mat3x3& other) const;
    bool operator!=(const Mat3x3& other) const;
    
    Mat3x3 matmul(const Mat3x3& other) const;
    Vec3 matmul(const Vec3& other) const;

    float determinant() const;
    Mat3x3 transpose() const;
    bool inverse(Mat3x3& out) const;

    /*************** affine transformations ***************/
    static Mat3x3 trs(Vec2 t, float radian, Vec2 s);
    bool is_affine() const;
    Vec2 _t() const;
    float _r() const;
    Vec2 _s() const;
};

struct PyVec2: Vec2 {
    PY_CLASS(PyVec2, linalg, vec2)

    PyVec2() : Vec2() {}
    PyVec2(const Vec2& v) : Vec2(v) {}
    PyVec2(const PyVec2& v) = default;
    Vec2* _() { return this; }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct PyVec3: Vec3 {
    PY_CLASS(PyVec3, linalg, vec3)

    PyVec3() : Vec3() {}
    PyVec3(const Vec3& v) : Vec3(v) {}
    PyVec3(const PyVec3& v) = default;
    Vec3* _() { return this; }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct PyVec4: Vec4{
    PY_CLASS(PyVec4, linalg, vec4)

    PyVec4(): Vec4(){}
    PyVec4(const Vec4& v): Vec4(v){}
    PyVec4(const PyVec4& v) = default;
    Vec4* _(){ return this; }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct PyMat3x3: Mat3x3{
    PY_CLASS(PyMat3x3, linalg, mat3x3)

    PyMat3x3(): Mat3x3(){}
    PyMat3x3(const Mat3x3& other): Mat3x3(other){}
    PyMat3x3(const PyMat3x3& other) = default;
    Mat3x3* _(){ return this; }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

inline PyObject* py_var(VM* vm, Vec2 obj){ return VAR_T(PyVec2, obj); }
inline PyObject* py_var(VM* vm, const PyVec2& obj){ return VAR_T(PyVec2, obj);}

inline PyObject* py_var(VM* vm, Vec3 obj){ return VAR_T(PyVec3, obj); }
inline PyObject* py_var(VM* vm, const PyVec3& obj){ return VAR_T(PyVec3, obj);}

inline PyObject* py_var(VM* vm, Vec4 obj){ return VAR_T(PyVec4, obj); }
inline PyObject* py_var(VM* vm, const PyVec4& obj){ return VAR_T(PyVec4, obj);}

inline PyObject* py_var(VM* vm, const Mat3x3& obj){ return VAR_T(PyMat3x3, obj); }
inline PyObject* py_var(VM* vm, const PyMat3x3& obj){ return VAR_T(PyMat3x3, obj); }

template<> inline Vec2 py_cast<Vec2>(VM* vm, PyObject* obj) { return CAST(PyVec2&, obj); }
template<> inline Vec3 py_cast<Vec3>(VM* vm, PyObject* obj) { return CAST(PyVec3&, obj); }
template<> inline Vec4 py_cast<Vec4>(VM* vm, PyObject* obj) { return CAST(PyVec4&, obj); }
template<> inline Mat3x3 py_cast<Mat3x3>(VM* vm, PyObject* obj) { return CAST(PyMat3x3&, obj); }

template<> inline Vec2 _py_cast<Vec2>(VM* vm, PyObject* obj) { return _CAST(PyVec2&, obj); }
template<> inline Vec3 _py_cast<Vec3>(VM* vm, PyObject* obj) { return _CAST(PyVec3&, obj); }
template<> inline Vec4 _py_cast<Vec4>(VM* vm, PyObject* obj) { return _CAST(PyVec4&, obj); }
template<> inline Mat3x3 _py_cast<Mat3x3>(VM* vm, PyObject* obj) { return _CAST(PyMat3x3&, obj); }

void add_module_linalg(VM* vm);

static_assert(sizeof(Py_<PyMat3x3>) <= 64);
static_assert(std::is_trivially_copyable<PyVec2>::value);
static_assert(std::is_trivially_copyable<PyVec3>::value);
static_assert(std::is_trivially_copyable<PyVec4>::value);
static_assert(std::is_trivially_copyable<PyMat3x3>::value);

}   // namespace pkpy
namespace pkpy{

#define BIND_VEC_VEC_OP(D, name, op)                                                    \
        vm->bind##name(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){  \
            PyVec##D& self = _CAST(PyVec##D&, _0);                                      \
            PyVec##D& other = CAST(PyVec##D&, _1);                                      \
            return VAR(self op other);                                                  \
        });

#define BIND_VEC_FLOAT_OP(D, name, op)  \
        vm->bind##name(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){  \
            PyVec##D& self = _CAST(PyVec##D&, _0);                                      \
            f64 other = CAST(f64, _1);                                                  \
            return VAR(self op other);                                                  \
        });

#define BIND_VEC_FUNCTION_0(D, name)        \
        vm->bind_method<0>(type, #name, [](VM* vm, ArgsView args){          \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                     \
            return VAR(self.name());                                        \
        });

#define BIND_VEC_FUNCTION_1(D, name)        \
        vm->bind_method<1>(type, #name, [](VM* vm, ArgsView args){          \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                     \
            PyVec##D& other = CAST(PyVec##D&, args[1]);                     \
            return VAR(self.name(other));                                   \
        });

#define BIND_VEC_MUL_OP(D)                                                                \
        vm->bind__mul__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){     \
            PyVec##D& self = _CAST(PyVec##D&, _0);                                          \
            if(is_non_tagged_type(_1, PyVec##D::_type(vm))){                                \
                PyVec##D& other = _CAST(PyVec##D&, _1);                                     \
                return VAR(self * other);                                                   \
            }                                                                               \
            f64 other = CAST(f64, _1);                                                      \
            return VAR(self * other);                                                       \
        });                                                                                 \
        vm->bind_method<1>(type, "__rmul__", [](VM* vm, ArgsView args){                     \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                                     \
            f64 other = CAST(f64, args[1]);                                                 \
            return VAR(self * other);                                                       \
        });                                                                                 \
        vm->bind__truediv__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){ \
            PyVec##D& self = _CAST(PyVec##D&, _0);                                          \
            f64 other = CAST(f64, _1);                                                      \
            return VAR(self / other);                                                       \
        });

// https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Math/Vector2.cs#L289
static Vec2 SmoothDamp(Vec2 current, Vec2 target, PyVec2& currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
{
    // Based on Game Programming Gems 4 Chapter 1.10
    smoothTime = std::max(0.0001F, smoothTime);
    float omega = 2.0F / smoothTime;

    float x = omega * deltaTime;
    float exp = 1.0F / (1.0F + x + 0.48F * x * x + 0.235F * x * x * x);

    float change_x = current.x - target.x;
    float change_y = current.y - target.y;
    Vec2 originalTo = target;

    // Clamp maximum speed
    float maxChange = maxSpeed * smoothTime;

    float maxChangeSq = maxChange * maxChange;
    float sqDist = change_x * change_x + change_y * change_y;
    if (sqDist > maxChangeSq)
    {
        float mag = std::sqrt(sqDist);
        change_x = change_x / mag * maxChange;
        change_y = change_y / mag * maxChange;
    }

    target.x = current.x - change_x;
    target.y = current.y - change_y;

    float temp_x = (currentVelocity.x + omega * change_x) * deltaTime;
    float temp_y = (currentVelocity.y + omega * change_y) * deltaTime;

    currentVelocity.x = (currentVelocity.x - omega * temp_x) * exp;
    currentVelocity.y = (currentVelocity.y - omega * temp_y) * exp;

    float output_x = target.x + (change_x + temp_x) * exp;
    float output_y = target.y + (change_y + temp_y) * exp;

    // Prevent overshooting
    float origMinusCurrent_x = originalTo.x - current.x;
    float origMinusCurrent_y = originalTo.y - current.y;
    float outMinusOrig_x = output_x - originalTo.x;
    float outMinusOrig_y = output_y - originalTo.y;

    if (origMinusCurrent_x * outMinusOrig_x + origMinusCurrent_y * outMinusOrig_y > 0)
    {
        output_x = originalTo.x;
        output_y = originalTo.y;

        currentVelocity.x = (output_x - originalTo.x) / deltaTime;
        currentVelocity.y = (output_y - originalTo.y) / deltaTime;
    }
    return Vec2(output_x, output_y);
}

    void PyVec2::_register(VM* vm, PyObject* mod, PyObject* type){
        PY_STRUCT_LIKE(PyVec2)

        vm->bind_constructor<3>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            return vm->heap.gcnew<PyVec2>(PK_OBJ_GET(Type, args[0]), Vec2(x, y));
        });

        // @staticmethod
        vm->bind(type, "smooth_damp(current: vec2, target: vec2, current_velocity_: vec2, smooth_time: float, max_speed: float, delta_time: float) -> vec2", [](VM* vm, ArgsView args){
            Vec2 current = CAST(Vec2, args[0]);
            Vec2 target = CAST(Vec2, args[1]);
            PyVec2& current_velocity_ = CAST(PyVec2&, args[2]);
            float smooth_time = CAST_F(args[3]);
            float max_speed = CAST_F(args[4]);
            float delta_time = CAST_F(args[5]);
            Vec2 ret = SmoothDamp(current, target, current_velocity_, smooth_time, max_speed, delta_time);
            return VAR(ret);
        }, {}, BindType::STATICMETHOD);

        // @staticmethod
        vm->bind(type, "angle(__from: vec2, __to: vec2) -> float", [](VM* vm, ArgsView args){
            PyVec2 __from = CAST(PyVec2, args[0]);
            PyVec2 __to = CAST(PyVec2, args[1]);
            float val = atan2f(__to.y, __to.x) - atan2f(__from.y, __from.x);
            const float PI = 3.1415926535897932384f;
            if(val > PI) val -= 2*PI;
            if(val < -PI) val += 2*PI;
            return VAR(val);
        }, {}, BindType::STATICMETHOD);

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            Vec2 self = _CAST(PyVec2&, obj);
            SStream ss;
            ss.setprecision(3);
            ss << "vec2(" << self.x << ", " << self.y << ")";
            return VAR(ss.str());
        });

        vm->bind_method<1>(type, "rotate", [](VM* vm, ArgsView args){
            Vec2 self = _CAST(PyVec2&, args[0]);
            float radian = CAST(f64, args[1]);
            return VAR_T(PyVec2, self.rotate(radian));
        });

        vm->bind_method<1>(type, "rotate_", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            float radian = CAST(f64, args[1]);
            self = self.rotate(radian);
            return vm->None;
        });

        PY_FIELD(PyVec2, "x", _, x)
        PY_FIELD(PyVec2, "y", _, y)

        BIND_VEC_VEC_OP(2, __add__, +)
        BIND_VEC_VEC_OP(2, __sub__, -)
        BIND_VEC_MUL_OP(2)
        BIND_VEC_FLOAT_OP(2, __truediv__, /)
        BIND_VEC_FUNCTION_1(2, dot)
        BIND_VEC_FUNCTION_1(2, cross)
        BIND_VEC_FUNCTION_1(2, copy_)
        BIND_VEC_FUNCTION_0(2, length)
        BIND_VEC_FUNCTION_0(2, length_squared)
        BIND_VEC_FUNCTION_0(2, normalize)
        BIND_VEC_FUNCTION_0(2, normalize_)
    }

    void PyVec3::_register(VM* vm, PyObject* mod, PyObject* type){
        PY_STRUCT_LIKE(PyVec3)

        vm->bind_constructor<4>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            float z = CAST_F(args[3]);
            return vm->heap.gcnew<PyVec3>(PK_OBJ_GET(Type, args[0]), Vec3(x, y, z));
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            Vec3 self = _CAST(PyVec3&, obj);
            SStream ss;
            ss.setprecision(3);
            ss << "vec3(" << self.x << ", " << self.y << ", " << self.z << ")";
            return VAR(ss.str());
        });

        PY_FIELD(PyVec3, "x", _, x)
        PY_FIELD(PyVec3, "y", _, y)
        PY_FIELD(PyVec3, "z", _, z)

        BIND_VEC_VEC_OP(3, __add__, +)
        BIND_VEC_VEC_OP(3, __sub__, -)
        BIND_VEC_MUL_OP(3)
        BIND_VEC_FUNCTION_1(3, dot)
        BIND_VEC_FUNCTION_1(3, cross)
        BIND_VEC_FUNCTION_1(3, copy_)
        BIND_VEC_FUNCTION_0(3, length)
        BIND_VEC_FUNCTION_0(3, length_squared)
        BIND_VEC_FUNCTION_0(3, normalize)
        BIND_VEC_FUNCTION_0(3, normalize_)
    }

    void PyVec4::_register(VM* vm, PyObject* mod, PyObject* type){
        PY_STRUCT_LIKE(PyVec4)

        vm->bind_constructor<1+4>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            float z = CAST_F(args[3]);
            float w = CAST_F(args[4]);
            return vm->heap.gcnew<PyVec4>(PK_OBJ_GET(Type, args[0]), Vec4(x, y, z, w));
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            Vec4 self = _CAST(PyVec4&, obj);
            SStream ss;
            ss.setprecision(3);
            ss << "vec4(" << self.x << ", " << self.y << ", " << self.z << ", " << self.w << ")";
            return VAR(ss.str());
        });

        PY_FIELD(PyVec4, "x", _, x)
        PY_FIELD(PyVec4, "y", _, y)
        PY_FIELD(PyVec4, "z", _, z)
        PY_FIELD(PyVec4, "w", _, w)

        BIND_VEC_VEC_OP(4, __add__, +)
        BIND_VEC_VEC_OP(4, __sub__, -)
        BIND_VEC_MUL_OP(4)
        BIND_VEC_FUNCTION_1(4, dot)
        BIND_VEC_FUNCTION_1(4, copy_)
        BIND_VEC_FUNCTION_0(4, length)
        BIND_VEC_FUNCTION_0(4, length_squared)
        BIND_VEC_FUNCTION_0(4, normalize)
        BIND_VEC_FUNCTION_0(4, normalize_)
    }

#undef BIND_VEC_VEC_OP
#undef BIND_VEC_MUL_OP
#undef BIND_VEC_FUNCTION_0
#undef BIND_VEC_FUNCTION_1

    void PyMat3x3::_register(VM* vm, PyObject* mod, PyObject* type){
        PY_STRUCT_LIKE(PyMat3x3)

        vm->bind_constructor<-1>(type, [](VM* vm, ArgsView args){
            if(args.size() == 1+0) return vm->heap.gcnew<PyMat3x3>(PK_OBJ_GET(Type, args[0]), Mat3x3::zeros());
            if(args.size() == 1+1){
                const List& list = CAST(List&, args[1]);
                if(list.size() != 9) vm->TypeError("Mat3x3.__new__ takes a list of 9 floats");
                Mat3x3 mat;
                for(int i=0; i<9; i++) mat.v[i] = CAST_F(list[i]);
                return vm->heap.gcnew<PyMat3x3>(PK_OBJ_GET(Type, args[0]), mat);
            }
            if(args.size() == 1+9){
                Mat3x3 mat;
                for(int i=0; i<9; i++) mat.v[i] = CAST_F(args[1+i]);
                return vm->heap.gcnew<PyMat3x3>(PK_OBJ_GET(Type, args[0]), mat);
            }
            vm->TypeError(_S("Mat3x3.__new__ takes 0 or 1 or 9 arguments, got ", args.size()-1));
            return vm->None;
        });

        vm->bind_method<1>(type, "copy_", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            const PyMat3x3& other = CAST(PyMat3x3&, args[1]);
            self = other;
            return vm->None;
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            const PyMat3x3& self = _CAST(PyMat3x3&, obj);
            SStream ss;
            ss.setprecision(3);
            ss << "mat3x3([" << self._11 << ", " << self._12 << ", " << self._13 << ",\n";
            ss << "        " << self._21 << ", " << self._22 << ", " << self._23 << ",\n";
            ss << "        " << self._31 << ", " << self._32 << ", " << self._33 << "])";
            return VAR(ss.str());
        });

        vm->bind__getitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj, PyObject* index){
            PyMat3x3& self = _CAST(PyMat3x3&, obj);
            Tuple& t = CAST(Tuple&, index);
            if(t.size() != 2){
                vm->TypeError("Mat3x3.__getitem__ takes a tuple of 2 integers");
            }
            i64 i = CAST(i64, t[0]);
            i64 j = CAST(i64, t[1]);
            if(i < 0 || i >= 3 || j < 0 || j >= 3){
                vm->IndexError("index out of range");
            }
            return VAR(self.m[i][j]);
        });

        vm->bind__setitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj, PyObject* index, PyObject* value){
            PyMat3x3& self = _CAST(PyMat3x3&, obj);
            const Tuple& t = CAST(Tuple&, index);
            if(t.size() != 2){
                vm->TypeError("Mat3x3.__setitem__ takes a tuple of 2 integers");
            }
            i64 i = CAST(i64, t[0]);
            i64 j = CAST(i64, t[1]);
            if(i < 0 || i >= 3 || j < 0 || j >= 3){
                vm->IndexError("index out of range");
            }
            self.m[i][j] = CAST_F(value);
        });

        PY_FIELD(PyMat3x3, "_11", _, _11)
        PY_FIELD(PyMat3x3, "_12", _, _12)
        PY_FIELD(PyMat3x3, "_13", _, _13)
        PY_FIELD(PyMat3x3, "_21", _, _21)
        PY_FIELD(PyMat3x3, "_22", _, _22)
        PY_FIELD(PyMat3x3, "_23", _, _23)
        PY_FIELD(PyMat3x3, "_31", _, _31)
        PY_FIELD(PyMat3x3, "_32", _, _32)
        PY_FIELD(PyMat3x3, "_33", _, _33)

        vm->bind__add__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            PyMat3x3& self = _CAST(PyMat3x3&, _0);
            PyMat3x3& other = CAST(PyMat3x3&, _1);
            return VAR_T(PyMat3x3, self + other);
        });

        vm->bind__sub__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            PyMat3x3& self = _CAST(PyMat3x3&, _0);
            PyMat3x3& other = CAST(PyMat3x3&, _1);
            return VAR_T(PyMat3x3, self - other);
        });

        vm->bind__mul__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            PyMat3x3& self = _CAST(PyMat3x3&, _0);
            f64 other = CAST_F(_1);
            return VAR_T(PyMat3x3, self * other);
        });

        vm->bind_method<1>(type, "__rmul__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            f64 other = CAST_F(args[1]);
            return VAR_T(PyMat3x3, self * other);
        });

        vm->bind__truediv__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            PyMat3x3& self = _CAST(PyMat3x3&, _0);
            f64 other = CAST_F(_1);
            return VAR_T(PyMat3x3, self / other);
        });

        vm->bind__matmul__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            PyMat3x3& self = _CAST(PyMat3x3&, _0);
            if(is_non_tagged_type(_1, PyMat3x3::_type(vm))){
                const PyMat3x3& other = _CAST(PyMat3x3&, _1);
                return VAR_T(PyMat3x3, self.matmul(other));
            }
            if(is_non_tagged_type(_1, PyVec3::_type(vm))){
                const PyVec3& other = _CAST(PyVec3&, _1);
                return VAR_T(PyVec3, self.matmul(other));
            }
            return vm->NotImplemented;
        });

        vm->bind(type, "matmul(self, other: mat3x3, out: mat3x3 = None)", [](VM* vm, ArgsView args){
            const PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            const PyMat3x3& other = CAST(PyMat3x3&, args[1]);
            if(args[2] == vm->None){
                return VAR_T(PyMat3x3, self.matmul(other));
            }else{
                PyMat3x3& out = CAST(PyMat3x3&, args[2]);
                out = self.matmul(other);
                return vm->None;
            }
        });

        vm->bind_method<0>(type, "determinant", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR(self.determinant());
        });

        vm->bind_method<0>(type, "transpose", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyMat3x3, self.transpose());
        });

        vm->bind__invert__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            PyMat3x3& self = _CAST(PyMat3x3&, obj);
            Mat3x3 ret;
            bool ok = self.inverse(ret);
            if(!ok) vm->ValueError("matrix is not invertible");
            return VAR_T(PyMat3x3, ret);
        });

        vm->bind_method<0>(type, "invert", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            Mat3x3 ret;
            bool ok = self.inverse(ret);
            if(!ok) vm->ValueError("matrix is not invertible");
            return VAR_T(PyMat3x3, ret);
        });

        vm->bind_method<0>(type, "invert_", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            Mat3x3 ret;
            bool ok = self.inverse(ret);
            if(!ok) vm->ValueError("matrix is not invertible");
            self = ret;
            return vm->None;
        });

        vm->bind_method<0>(type, "transpose_", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            self = self.transpose();
            return vm->None;
        });

        // @staticmethod
        vm->bind(type, "zeros()", [](VM* vm, ArgsView args){
            PK_UNUSED(args);
            return VAR_T(PyMat3x3, Mat3x3::zeros());
        }, {}, BindType::STATICMETHOD);

        // @staticmethod
        vm->bind(type, "ones()", [](VM* vm, ArgsView args){
            PK_UNUSED(args);
            return VAR_T(PyMat3x3, Mat3x3::ones());
        }, {}, BindType::STATICMETHOD);

        // @staticmethod
        vm->bind(type, "identity()", [](VM* vm, ArgsView args){
            PK_UNUSED(args);
            return VAR_T(PyMat3x3, Mat3x3::identity());
        }, {}, BindType::STATICMETHOD);

        /*************** affine transformations ***************/
        // @staticmethod
        vm->bind(type, "trs(t: vec2, r: float, s: vec2)", [](VM* vm, ArgsView args){
            Vec2 t = CAST(Vec2, args[0]);
            f64 r = CAST_F(args[1]);
            Vec2 s = CAST(Vec2, args[2]);
            return VAR_T(PyMat3x3, Mat3x3::trs(t, r, s));
        }, {}, BindType::STATICMETHOD);

        vm->bind(type, "copy_trs_(self, t: vec2, r: float, s: vec2)", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            Vec2 t = CAST(Vec2, args[1]);
            f64 r = CAST_F(args[2]);
            Vec2 s = CAST(Vec2, args[3]);
            self = Mat3x3::trs(t, r, s);
            return vm->None;
        });

        vm->bind(type, "copy_t_(self, t: vec2)", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            Vec2 t = CAST(Vec2, args[1]);
            self = Mat3x3::trs(t, self._r(), self._s());
            return vm->None;
        });

        vm->bind(type, "copy_r_(self, r: float)", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            f64 r = CAST_F(args[1]);
            self = Mat3x3::trs(self._t(), r, self._s());
            return vm->None;
        });

        vm->bind(type, "copy_s_(self, s: vec2)", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            Vec2 s = CAST(Vec2, args[1]);
            self = Mat3x3::trs(self._t(), self._r(), s);
            return vm->None;
        });

        vm->bind_method<0>(type, "is_affine", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR(self.is_affine());
        });

        vm->bind_method<0>(type, "_t", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyVec2, self._t());
        });

        vm->bind_method<0>(type, "_r", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR(self._r());
        });

        vm->bind_method<0>(type, "_s", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyVec2, self._s());
        });

        vm->bind_method<1>(type, "transform_point", [](VM* vm, ArgsView args){
            const PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            Vec2 v = CAST(Vec2, args[1]);
            Vec2 res = Vec2(self._11 * v.x + self._12 * v.y + self._13, self._21 * v.x + self._22 * v.y + self._23);
            return VAR_T(PyVec2, res);
        });

        vm->bind_method<1>(type, "transform_vector", [](VM* vm, ArgsView args){
            const PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            Vec2 v = CAST(Vec2, args[1]);
            Vec2 res = Vec2(self._11 * v.x + self._12 * v.y, self._21 * v.x + self._22 * v.y);
            return VAR_T(PyVec2, res);
        });
    }


void add_module_linalg(VM* vm){
    PyObject* linalg = vm->new_module("linalg");
    PyVec2::register_class(vm, linalg);
    PyVec3::register_class(vm, linalg);
    PyVec4::register_class(vm, linalg);
    PyMat3x3::register_class(vm, linalg);

    PyObject* float_p = vm->_modules["c"]->attr("float_p");
    linalg->attr().set("vec2_p", float_p);
    linalg->attr().set("vec3_p", float_p);
    linalg->attr().set("vec4_p", float_p);
    linalg->attr().set("mat3x3_p", float_p);
}


    /////////////// mat3x3 ///////////////
    Mat3x3::Mat3x3() {}
    Mat3x3::Mat3x3(float _11, float _12, float _13,
           float _21, float _22, float _23,
           float _31, float _32, float _33)
        : _11(_11), _12(_12), _13(_13)
        , _21(_21), _22(_22), _23(_23)
        , _31(_31), _32(_32), _33(_33) {}

    Mat3x3 Mat3x3::zeros(){
        return Mat3x3(0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    Mat3x3 Mat3x3::ones(){
        return Mat3x3(1, 1, 1, 1, 1, 1, 1, 1, 1);
    }

    Mat3x3 Mat3x3::identity(){
        return Mat3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    }

    Mat3x3 Mat3x3::operator+(const Mat3x3& other) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] + other.v[i];
        return ret;
    }

    Mat3x3 Mat3x3::operator-(const Mat3x3& other) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] - other.v[i];
        return ret;
    }

    Mat3x3 Mat3x3::operator*(float scalar) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] * scalar;
        return ret;
    }

    Mat3x3 Mat3x3::operator/(float scalar) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] / scalar;
        return ret;
    }

    bool Mat3x3::operator==(const Mat3x3& other) const{
        for (int i=0; i<9; ++i){
            if (!isclose(v[i], other.v[i])) return false;
        }
        return true;
    }

    bool Mat3x3::operator!=(const Mat3x3& other) const{
        for (int i=0; i<9; ++i){
            if (!isclose(v[i], other.v[i])) return true;
        }
        return false;
    }

    Mat3x3 Mat3x3::matmul(const Mat3x3& other) const{
        Mat3x3 out;
        out._11 = _11 * other._11 + _12 * other._21 + _13 * other._31;
        out._12 = _11 * other._12 + _12 * other._22 + _13 * other._32;
        out._13 = _11 * other._13 + _12 * other._23 + _13 * other._33;
        out._21 = _21 * other._11 + _22 * other._21 + _23 * other._31;
        out._22 = _21 * other._12 + _22 * other._22 + _23 * other._32;
        out._23 = _21 * other._13 + _22 * other._23 + _23 * other._33;
        out._31 = _31 * other._11 + _32 * other._21 + _33 * other._31;
        out._32 = _31 * other._12 + _32 * other._22 + _33 * other._32;
        out._33 = _31 * other._13 + _32 * other._23 + _33 * other._33;
        return out;
    }

    Vec3 Mat3x3::matmul(const Vec3& other) const{
        Vec3 out;
        out.x = _11 * other.x + _12 * other.y + _13 * other.z;
        out.y = _21 * other.x + _22 * other.y + _23 * other.z;
        out.z = _31 * other.x + _32 * other.y + _33 * other.z;
        return out;
    }

    float Mat3x3::determinant() const{
        return _11 * _22 * _33 + _12 * _23 * _31 + _13 * _21 * _32
             - _11 * _23 * _32 - _12 * _21 * _33 - _13 * _22 * _31;
    }

    Mat3x3 Mat3x3::transpose() const{
        Mat3x3 ret;
        ret._11 = _11;  ret._12 = _21;  ret._13 = _31;
        ret._21 = _12;  ret._22 = _22;  ret._23 = _32;
        ret._31 = _13;  ret._32 = _23;  ret._33 = _33;
        return ret;
    }

    bool Mat3x3::inverse(Mat3x3& out) const{
        float det = determinant();
        if (isclose(det, 0)) return false;
        float inv_det = 1.0f / det;
        out._11 = (_22 * _33 - _23 * _32) * inv_det;
        out._12 = (_13 * _32 - _12 * _33) * inv_det;
        out._13 = (_12 * _23 - _13 * _22) * inv_det;
        out._21 = (_23 * _31 - _21 * _33) * inv_det;
        out._22 = (_11 * _33 - _13 * _31) * inv_det;
        out._23 = (_13 * _21 - _11 * _23) * inv_det;
        out._31 = (_21 * _32 - _22 * _31) * inv_det;
        out._32 = (_12 * _31 - _11 * _32) * inv_det;
        out._33 = (_11 * _22 - _12 * _21) * inv_det;
        return true;
    }

    Mat3x3 Mat3x3::trs(Vec2 t, float radian, Vec2 s){
        float cr = cosf(radian);
        float sr = sinf(radian);
        return Mat3x3(s.x * cr,   -s.y * sr,  t.x,
                      s.x * sr,   s.y * cr,   t.y,
                      0.0f,       0.0f,       1.0f);
    }

    bool Mat3x3::is_affine() const{
        float det = _11 * _22 - _12 * _21;
        if(isclose(det, 0)) return false;
        return _31 == 0.0f && _32 == 0.0f && _33 == 1.0f;
    }

    Vec2 Mat3x3::_t() const { return Vec2(_13, _23); }
    float Mat3x3::_r() const { return atan2f(_21, _11); }
    Vec2 Mat3x3::_s() const {
        return Vec2(
            sqrtf(_11 * _11 + _21 * _21),
            sqrtf(_12 * _12 + _22 * _22)
        );
    }

}   // namespace pkpy


namespace pkpy{

void add_module_easing(VM* vm);

} // namespace pkpy
namespace pkpy{
// https://easings.net/

const double kPi = 3.1415926545;

static double easeLinear( double x ) {
    return x;
}

static double easeInSine( double x ) {
    return 1.0 - std::cos( x * kPi / 2 );
}

static double easeOutSine( double x ) {
	return std::sin( x * kPi / 2 );
}

static double easeInOutSine( double x ) {
	return -( std::cos( kPi * x ) - 1 ) / 2;
}

static double easeInQuad( double x ) {
    return x * x;
}

static double easeOutQuad( double x ) {
    return 1 - std::pow( 1 - x, 2 );
}

static double easeInOutQuad( double x ) {
    if( x < 0.5 ) {
        return 2 * x * x;
    } else {
        return 1 - std::pow( -2 * x + 2, 2 ) / 2;
    }
}

static double easeInCubic( double x ) {
    return x * x * x;
}

static double easeOutCubic( double x ) {
    return 1 - std::pow( 1 - x, 3 );
}

static double easeInOutCubic( double x ) {
    if( x < 0.5 ) {
        return 4 * x * x * x;
    } else {
        return 1 - std::pow( -2 * x + 2, 3 ) / 2;
    }
}

static double easeInQuart( double x ) {
    return std::pow( x, 4 );
}

static double easeOutQuart( double x ) {
    return 1 - std::pow( 1 - x, 4 );
}

static double easeInOutQuart( double x ) {
    if( x < 0.5 ) {
        return 8 * std::pow( x, 4 );
    } else {
        return 1 - std::pow( -2 * x + 2, 4 ) / 2;
    }
}

static double easeInQuint( double x ) {
    return std::pow( x, 5 );
}

static double easeOutQuint( double x ) {
    return 1 - std::pow( 1 - x, 5 );
}

static double easeInOutQuint( double x ) {
    if( x < 0.5 ) {
        return 16 * std::pow( x, 5 );
    } else {
        return 1 - std::pow( -2 * x + 2, 5 ) / 2;
    }
}

static double easeInExpo( double x ) {
    return x == 0 ? 0 : std::pow( 2, 10 * x - 10 );
}

static double easeOutExpo( double x ) {
    return x == 1 ? 1 : 1 - std::pow( 2, -10 * x );
}

static double easeInOutExpo( double x ) {
    if( x == 0 ) {
        return 0;
    } else if( x == 1 ) {
        return 1;
    } else if( x < 0.5 ) {
        return std::pow( 2, 20 * x - 10 ) / 2;
    } else {
        return (2 - std::pow( 2, -20 * x + 10 )) / 2;
    }
}

static double easeInCirc( double x ) {
    return 1 - std::sqrt( 1 - std::pow( x, 2 ) );
}

static double easeOutCirc( double x ) {
    return std::sqrt( 1 - std::pow( x - 1, 2 ) );
}

static double easeInOutCirc( double x ) {
    if( x < 0.5 ) {
        return (1 - std::sqrt( 1 - std::pow( 2 * x, 2 ) )) / 2;
    } else {
        return (std::sqrt( 1 - std::pow( -2 * x + 2, 2 ) ) + 1) / 2;
    }
}

static double easeInBack( double x ) {
    const double c1 = 1.70158;
    const double c3 = c1 + 1;
    return c3 * x * x * x - c1 * x * x;
}

static double easeOutBack( double x ) {
    const double c1 = 1.70158;
    const double c3 = c1 + 1;
    return 1 + c3 * std::pow( x - 1, 3 ) + c1 * std::pow( x - 1, 2 );
}

static double easeInOutBack( double x ) {
    const double c1 = 1.70158;
    const double c2 = c1 * 1.525;
    if( x < 0.5 ) {
        return (std::pow( 2 * x, 2 ) * ((c2 + 1) * 2 * x - c2)) / 2;
    } else {
        return (std::pow( 2 * x - 2, 2 ) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
    }
}

static double easeInElastic( double x ) {
    const double c4 = (2 * kPi) / 3;
    if( x == 0 ) {
        return 0;
    } else if( x == 1 ) {
        return 1;
    } else {
        return -std::pow( 2, 10 * x - 10 ) * std::sin( (x * 10 - 10.75) * c4 );
    }
}

static double easeOutElastic( double x ) {
    const double c4 = (2 * kPi) / 3;
    if( x == 0 ) {
        return 0;
    } else if( x == 1 ) {
        return 1;
    } else {
        return std::pow( 2, -10 * x ) * std::sin( (x * 10 - 0.75) * c4 ) + 1;
    }
}

static double easeInOutElastic( double x ) {
    const double c5 = (2 * kPi) / 4.5;
    if( x == 0 ) {
        return 0;
    } else if( x == 1 ) {
        return 1;
    } else if( x < 0.5 ) {
        return -(std::pow( 2, 20 * x - 10 ) * std::sin( (20 * x - 11.125) * c5 )) / 2;
    } else {
        return (std::pow( 2, -20 * x + 10 ) * std::sin( (20 * x - 11.125) * c5 )) / 2 + 1;
    }
}

static double easeOutBounce( double x ) {
    const double n1 = 7.5625;
    const double d1 = 2.75;
    if( x < 1 / d1 ) {
        return n1 * x * x;
    } else if( x < 2 / d1 ) {
        x -= 1.5 / d1;
        return n1 * x * x + 0.75;
    } else if( x < 2.5 / d1 ) {
        x -= 2.25 / d1;
        return n1 * x * x + 0.9375;
    } else {
        x -= 2.625 / d1;
        return n1 * x * x + 0.984375;
    }
}

static double easeInBounce( double x ) {
    return 1 - easeOutBounce(1 - x);
}

static double easeInOutBounce( double x ) {
    return x < 0.5
    ? (1 - easeOutBounce(1 - 2 * x)) / 2
    : (1 + easeOutBounce(2 * x - 1)) / 2;
}

void add_module_easing(VM* vm){
    PyObject* mod = vm->new_module("easing");

#define EASE(name)  \
    vm->bind_func<1>(mod, #name, [](VM* vm, ArgsView args){  \
        f64 t = CAST(f64, args[0]); \
        return VAR(ease##name(t));   \
    });

    EASE(Linear)
    EASE(InSine)
    EASE(OutSine)
    EASE(InOutSine)
    EASE(InQuad)
    EASE(OutQuad)
    EASE(InOutQuad)
    EASE(InCubic)
    EASE(OutCubic)
    EASE(InOutCubic)
    EASE(InQuart)
    EASE(OutQuart)
    EASE(InOutQuart)
    EASE(InQuint)
    EASE(OutQuint)
    EASE(InOutQuint)
    EASE(InExpo)
    EASE(OutExpo)
    EASE(InOutExpo)
    EASE(InCirc)
    EASE(OutCirc)
    EASE(InOutCirc)
    EASE(InBack)
    EASE(OutBack)
    EASE(InOutBack)
    EASE(InElastic)
    EASE(OutElastic)
    EASE(InOutElastic)
    EASE(InBounce)
    EASE(OutBounce)
    EASE(InOutBounce)

#undef EASE
}
}   // namespace pkpy


namespace pkpy{
    unsigned char* _default_import_handler(const char*, int, int*);
    void add_module_os(VM* vm);
    void add_module_io(VM* vm);
}

#if PK_ENABLE_OS

#include <filesystem>
#include <cstdio>

namespace pkpy{

struct FileIO {
    PY_CLASS(FileIO, io, FileIO)

    Str file;
    Str mode;
    FILE* fp;

    bool is_text() const { return mode != "rb" && mode != "wb" && mode != "ab"; }
    FileIO(VM* vm, std::string file, std::string mode);
    void close();
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

} // namespace pkpy
#endif
namespace pkpy{

#if PK_ENABLE_OS
static FILE* io_fopen(const char* name, const char* mode){
#if _MSC_VER
    FILE* fp;
    errno_t err = fopen_s(&fp, name, mode);
    if(err != 0) return nullptr;
    return fp;
#else
    return fopen(name, mode);
#endif
}

static size_t io_fread(void* buffer, size_t size, size_t count, FILE* fp){
#if _MSC_VER
    return fread_s(buffer, std::numeric_limits<size_t>::max(), size, count, fp);
#else
    return fread(buffer, size, count, fp);
#endif
}
#endif


unsigned char* _default_import_handler(const char* name_p, int name_size, int* out_size){
#if PK_ENABLE_OS
    std::string name(name_p, name_size);
    bool exists = std::filesystem::exists(std::filesystem::path(name));
    if(!exists) return nullptr;
    FILE* fp = io_fopen(name.c_str(), "rb");
    if(!fp) return nullptr;
    fseek(fp, 0, SEEK_END);
    int buffer_size = ftell(fp);
    unsigned char* buffer = new unsigned char[buffer_size];
    fseek(fp, 0, SEEK_SET);
    size_t sz = io_fread(buffer, 1, buffer_size, fp);
    PK_UNUSED(sz);
    fclose(fp);
    *out_size = buffer_size;
    return buffer;
#else
    return nullptr;
#endif
};


#if PK_ENABLE_OS
    void FileIO::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<3>(type, [](VM* vm, ArgsView args){
            Type cls = PK_OBJ_GET(Type, args[0]);
            return vm->heap.gcnew<FileIO>(cls, vm,
                       py_cast<Str&>(vm, args[1]).str(),
                       py_cast<Str&>(vm, args[2]).str());
        });

        vm->bind_method<0>(type, "read", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            fseek(io.fp, 0, SEEK_END);
            int buffer_size = ftell(io.fp);
            unsigned char* buffer = new unsigned char[buffer_size];
            fseek(io.fp, 0, SEEK_SET);
            size_t actual_size = io_fread(buffer, 1, buffer_size, io.fp);
            PK_ASSERT(actual_size <= buffer_size);
            // in text mode, CR may be dropped, which may cause `actual_size < buffer_size`
            Bytes b(buffer, actual_size);
            if(io.is_text()) return VAR(b.str());
            return VAR(std::move(b));
        });

        vm->bind_method<1>(type, "write", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            if(io.is_text()){
                Str& s = CAST(Str&, args[1]);
                fwrite(s.data, 1, s.length(), io.fp);
            }else{
                Bytes& buffer = CAST(Bytes&, args[1]);
                fwrite(buffer.data(), 1, buffer.size(), io.fp);
            }
            return vm->None;
        });

        vm->bind_method<0>(type, "close", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            io.close();
            return vm->None;
        });

        vm->bind_method<0>(type, "__exit__", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            io.close();
            return vm->None;
        });

        vm->bind_method<0>(type, "__enter__", PK_LAMBDA(args[0]));
    }

    FileIO::FileIO(VM* vm, std::string file, std::string mode): file(file), mode(mode) {
        fp = io_fopen(file.c_str(), mode.c_str());
        if(!fp) vm->IOError(strerror(errno));
    }

    void FileIO::close(){
        if(fp == nullptr) return;
        fclose(fp);
        fp = nullptr;
    }

#endif

void add_module_io(VM* vm){
#if PK_ENABLE_OS
    PyObject* mod = vm->new_module("io");
    FileIO::register_class(vm, mod);
    vm->bind(vm->builtins, "open(path, mode='r')", [](VM* vm, ArgsView args){
        PK_LOCAL_STATIC StrName m_io("io");
        PK_LOCAL_STATIC StrName m_FileIO("FileIO");
        return vm->call(vm->_modules[m_io]->attr(m_FileIO), args[0], args[1]);
    });
#endif
}

void add_module_os(VM* vm){
#if PK_ENABLE_OS
    PyObject* mod = vm->new_module("os");
    PyObject* path_obj = vm->heap.gcnew<DummyInstance>(vm->tp_object);
    mod->attr().set("path", path_obj);
    
    // Working directory is shared by all VMs!!
    vm->bind_func<0>(mod, "getcwd", [](VM* vm, ArgsView args){
        return VAR(std::filesystem::current_path().string());
    });

    vm->bind_func<1>(mod, "chdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        std::filesystem::current_path(path);
        return vm->None;
    });

    vm->bind_func<1>(mod, "listdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        std::filesystem::directory_iterator di;
        try{
            di = std::filesystem::directory_iterator(path);
        }catch(std::filesystem::filesystem_error& e){
            std::string msg = e.what();
            auto pos = msg.find_last_of(":");
            if(pos != std::string::npos) msg = msg.substr(pos + 1);
            vm->IOError(Str(msg).lstrip());
        }
        List ret;
        for(auto& p: di) ret.push_back(VAR(p.path().filename().string()));
        return VAR(ret);
    });

    vm->bind_func<1>(mod, "remove", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool ok = std::filesystem::remove(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func<1>(mod, "mkdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool ok = std::filesystem::create_directory(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func<1>(mod, "rmdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool ok = std::filesystem::remove(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func<-1>(path_obj, "join", [](VM* vm, ArgsView args){
        std::filesystem::path path;
        for(int i=0; i<args.size(); i++){
            path /= CAST(Str&, args[i]).sv();
        }
        return VAR(path.string());
    });

    vm->bind_func<1>(path_obj, "exists", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool exists = std::filesystem::exists(path);
        return VAR(exists);
    });

    vm->bind_func<1>(path_obj, "basename", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        return VAR(path.filename().string());
    });
#endif
}

}   // namespace pkpy
namespace pkpy{

void add_module_operator(VM* vm);
void add_module_time(VM* vm);
void add_module_sys(VM* vm);
void add_module_json(VM* vm);
void add_module_math(VM* vm);
void add_module_traceback(VM* vm);
void add_module_dis(VM* vm);
void add_module_gc(VM* vm);
void add_module_line_profiler(VM* vm);

}   // namespace pkpy
namespace pkpy{

void add_module_operator(VM* vm){
    PyObject* mod = vm->new_module("operator");
    vm->bind_func<2>(mod, "lt", [](VM* vm, ArgsView args) { return VAR(vm->py_lt(args[0], args[1]));});
    vm->bind_func<2>(mod, "le", [](VM* vm, ArgsView args) { return VAR(vm->py_le(args[0], args[1]));});
    vm->bind_func<2>(mod, "eq", [](VM* vm, ArgsView args) { return VAR(vm->py_eq(args[0], args[1]));});
    vm->bind_func<2>(mod, "ne", [](VM* vm, ArgsView args) { return VAR(vm->py_ne(args[0], args[1]));});
    vm->bind_func<2>(mod, "ge", [](VM* vm, ArgsView args) { return VAR(vm->py_ge(args[0], args[1]));});
    vm->bind_func<2>(mod, "gt", [](VM* vm, ArgsView args) { return VAR(vm->py_gt(args[0], args[1]));});
}

struct PyStructTime{
    PY_CLASS(PyStructTime, time, struct_time)

    int tm_year;
    int tm_mon;
    int tm_mday;
    int tm_hour;
    int tm_min;
    int tm_sec;
    int tm_wday;
    int tm_yday;
    int tm_isdst;

    PyStructTime(std::time_t t){
        std::tm* tm = std::localtime(&t);
        tm_year = tm->tm_year + 1900;
        tm_mon = tm->tm_mon + 1;
        tm_mday = tm->tm_mday;
        tm_hour = tm->tm_hour;
        tm_min = tm->tm_min;
        tm_sec = tm->tm_sec;
        tm_wday = (tm->tm_wday + 6) % 7;
        tm_yday = tm->tm_yday + 1;
        tm_isdst = tm->tm_isdst;
    }

    PyStructTime* _() { return this; }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_notimplemented_constructor<PyStructTime>(type);
        PY_READONLY_FIELD(PyStructTime, "tm_year", _, tm_year);
        PY_READONLY_FIELD(PyStructTime, "tm_mon", _, tm_mon);
        PY_READONLY_FIELD(PyStructTime, "tm_mday", _, tm_mday);
        PY_READONLY_FIELD(PyStructTime, "tm_hour", _, tm_hour);
        PY_READONLY_FIELD(PyStructTime, "tm_min", _, tm_min);
        PY_READONLY_FIELD(PyStructTime, "tm_sec", _, tm_sec);
        PY_READONLY_FIELD(PyStructTime, "tm_wday", _, tm_wday);
        PY_READONLY_FIELD(PyStructTime, "tm_yday", _, tm_yday);
        PY_READONLY_FIELD(PyStructTime, "tm_isdst", _, tm_isdst);
    }
};

void add_module_time(VM* vm){
    PyObject* mod = vm->new_module("time");
    PyStructTime::register_class(vm, mod);

    vm->bind_func<0>(mod, "time", [](VM* vm, ArgsView args) {
        auto now = std::chrono::system_clock::now();
        return VAR(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() / 1000.0);
    });

    vm->bind_func<1>(mod, "sleep", [](VM* vm, ArgsView args) {
        f64 seconds = CAST_F(args[0]);
        auto begin = std::chrono::system_clock::now();
        while(true){
            auto now = std::chrono::system_clock::now();
            f64 elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - begin).count() / 1000.0;
            if(elapsed >= seconds) break;
        }
        return vm->None;
    });

    vm->bind_func<0>(mod, "localtime", [](VM* vm, ArgsView args) {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        return VAR_T(PyStructTime, t);
    });
}

void add_module_sys(VM* vm){
    PyObject* mod = vm->new_module("sys");
    vm->setattr(mod, "version", VAR(PK_VERSION));
    vm->setattr(mod, "platform", VAR(kPlatformStrings[PK_SYS_PLATFORM]));

    PyObject* stdout_ = vm->heap.gcnew<DummyInstance>(vm->tp_object);
    PyObject* stderr_ = vm->heap.gcnew<DummyInstance>(vm->tp_object);
    vm->setattr(mod, "stdout", stdout_);
    vm->setattr(mod, "stderr", stderr_);

    vm->bind_func<1>(stdout_, "write", [](VM* vm, ArgsView args) {
        Str& s = CAST(Str&, args[0]);
        vm->stdout_write(s);
        return vm->None;
    });

    vm->bind_func<1>(stderr_, "write", [](VM* vm, ArgsView args) {
        Str& s = CAST(Str&, args[0]);
        vm->_stderr(s.data, s.size);
        return vm->None;
    });
}

void add_module_json(VM* vm){
    PyObject* mod = vm->new_module("json");
    vm->bind_func<1>(mod, "loads", [](VM* vm, ArgsView args) {
        std::string_view sv;
        if(is_non_tagged_type(args[0], vm->tp_bytes)){
            sv = PK_OBJ_GET(Bytes, args[0]).sv();
        }else{
            sv = CAST(Str&, args[0]).sv();
        }
        CodeObject_ code = vm->compile(sv, "<json>", JSON_MODE);
        return vm->_exec(code, vm->top_frame()->_module);
    });

    vm->bind_func<1>(mod, "dumps", [](VM* vm, ArgsView args) {
        return vm->py_json(args[0]);
    });
}

// https://docs.python.org/3.5/library/math.html
void add_module_math(VM* vm){
    PyObject* mod = vm->new_module("math");
    mod->attr().set("pi", VAR(3.1415926535897932384));
    mod->attr().set("e" , VAR(2.7182818284590452354));
    mod->attr().set("inf", VAR(std::numeric_limits<double>::infinity()));
    mod->attr().set("nan", VAR(std::numeric_limits<double>::quiet_NaN()));

    vm->bind_func<1>(mod, "ceil", PK_LAMBDA(VAR((i64)std::ceil(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "fabs", PK_LAMBDA(VAR(std::fabs(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "floor", PK_LAMBDA(VAR((i64)std::floor(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "fsum", [](VM* vm, ArgsView args) {
        List& list = CAST(List&, args[0]);
        double sum = 0;
        double c = 0;
        for(PyObject* arg : list){
            double x = CAST_F(arg);
            double y = x - c;
            double t = sum + y;
            c = (t - sum) - y;
            sum = t;
        }
        return VAR(sum);
    });
    vm->bind_func<2>(mod, "gcd", [](VM* vm, ArgsView args) {
        i64 a = CAST(i64, args[0]);
        i64 b = CAST(i64, args[1]);
        if(a < 0) a = -a;
        if(b < 0) b = -b;
        while(b != 0){
            i64 t = b;
            b = a % b;
            a = t;
        }
        return VAR(a);
    });

    vm->bind_func<1>(mod, "isfinite", PK_LAMBDA(VAR(std::isfinite(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "isinf", PK_LAMBDA(VAR(std::isinf(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "isnan", PK_LAMBDA(VAR(std::isnan(CAST_F(args[0])))));

    vm->bind_func<2>(mod, "isclose", [](VM* vm, ArgsView args) {
        f64 a = CAST_F(args[0]);
        f64 b = CAST_F(args[1]);
        return VAR(std::fabs(a - b) <= Number::kEpsilon);
    });

    vm->bind_func<1>(mod, "exp", PK_LAMBDA(VAR(std::exp(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "log", PK_LAMBDA(VAR(std::log(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "log2", PK_LAMBDA(VAR(std::log2(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "log10", PK_LAMBDA(VAR(std::log10(CAST_F(args[0])))));

    vm->bind_func<2>(mod, "pow", PK_LAMBDA(VAR(std::pow(CAST_F(args[0]), CAST_F(args[1])))));
    vm->bind_func<1>(mod, "sqrt", PK_LAMBDA(VAR(std::sqrt(CAST_F(args[0])))));

    vm->bind_func<1>(mod, "acos", PK_LAMBDA(VAR(std::acos(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "asin", PK_LAMBDA(VAR(std::asin(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "atan", PK_LAMBDA(VAR(std::atan(CAST_F(args[0])))));
    vm->bind_func<2>(mod, "atan2", PK_LAMBDA(VAR(std::atan2(CAST_F(args[0]), CAST_F(args[1])))));

    vm->bind_func<1>(mod, "cos", PK_LAMBDA(VAR(std::cos(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "sin", PK_LAMBDA(VAR(std::sin(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "tan", PK_LAMBDA(VAR(std::tan(CAST_F(args[0])))));
    
    vm->bind_func<1>(mod, "degrees", PK_LAMBDA(VAR(CAST_F(args[0]) * 180 / 3.1415926535897932384)));
    vm->bind_func<1>(mod, "radians", PK_LAMBDA(VAR(CAST_F(args[0]) * 3.1415926535897932384 / 180)));

    vm->bind_func<1>(mod, "modf", [](VM* vm, ArgsView args) {
        f64 i;
        f64 f = std::modf(CAST_F(args[0]), &i);
        return VAR(Tuple(VAR(f), VAR(i)));
    });

    vm->bind_func<1>(mod, "factorial", [](VM* vm, ArgsView args) {
        i64 n = CAST(i64, args[0]);
        if(n < 0) vm->ValueError("factorial() not defined for negative values");
        i64 r = 1;
        for(i64 i=2; i<=n; i++) r *= i;
        return VAR(r);
    });
}

void add_module_traceback(VM* vm){
    PyObject* mod = vm->new_module("traceback");
    vm->bind_func<0>(mod, "print_exc", [](VM* vm, ArgsView args) {
        if(vm->_last_exception==nullptr) vm->ValueError("no exception");
        Exception& e = _CAST(Exception&, vm->_last_exception);
        vm->stdout_write(e.summary());
        return vm->None;
    });

    vm->bind_func<0>(mod, "format_exc", [](VM* vm, ArgsView args) {
        if(vm->_last_exception==nullptr) vm->ValueError("no exception");
        Exception& e = _CAST(Exception&, vm->_last_exception);
        return VAR(e.summary());
    });
}

void add_module_dis(VM* vm){
    PyObject* mod = vm->new_module("dis");

    vm->bind_func<1>(mod, "dis", [](VM* vm, ArgsView args) {
        CodeObject_ code;
        PyObject* obj = args[0];
        if(is_type(obj, vm->tp_str)){
            const Str& source = CAST(Str, obj);
            code = vm->compile(source, "<dis>", EXEC_MODE);
        }
        PyObject* f = obj;
        if(is_type(f, vm->tp_bound_method)) f = CAST(BoundMethod, obj).func;
        code = CAST(Function&, f).decl->code;
        vm->stdout_write(vm->disassemble(code));
        return vm->None;
    });
}

void add_module_gc(VM* vm){
    PyObject* mod = vm->new_module("gc");
    vm->bind_func<0>(mod, "collect", PK_LAMBDA(VAR(vm->heap.collect())));
}

struct LineProfilerW;
struct _LpGuard{
    PK_ALWAYS_PASS_BY_POINTER(_LpGuard)
    LineProfilerW* lp;
    VM* vm;
    _LpGuard(LineProfilerW* lp, VM* vm);
    ~_LpGuard();
};

// line_profiler wrapper
struct LineProfilerW{
    PY_CLASS(LineProfilerW, line_profiler, LineProfiler)

    LineProfiler profiler;

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_default_constructor<LineProfilerW>(type);

        vm->bind(type, "add_function(self, func)", [](VM* vm, ArgsView args){
            LineProfilerW& self = PK_OBJ_GET(LineProfilerW, args[0]);
            vm->check_non_tagged_type(args[1], VM::tp_function);
            auto decl = PK_OBJ_GET(Function, args[1]).decl.get();
            self.profiler.functions.insert(decl);
            return vm->None;
        });

        vm->bind(type, "runcall(self, func, *args)", [](VM* vm, ArgsView view){
            LineProfilerW& self = PK_OBJ_GET(LineProfilerW, view[0]);
            PyObject* func = view[1];
            const Tuple& args = CAST(Tuple&, view[2]);
            vm->s_data.push(func);
            vm->s_data.push(PY_NULL);
            for(PyObject* arg : args) vm->s_data.push(arg);
            _LpGuard guard(&self, vm);
            PyObject* ret = vm->vectorcall(args.size());
            return ret;
        });

        vm->bind(type, "print_stats(self)", [](VM* vm, ArgsView args){
            LineProfilerW& self = PK_OBJ_GET(LineProfilerW, args[0]);
            vm->stdout_write(self.profiler.stats());
            return vm->None;
        });
    }
};


_LpGuard::_LpGuard(LineProfilerW* lp, VM* vm): lp(lp), vm(vm) {
    if(vm->_profiler){
        vm->ValueError("only one profiler can be enabled at a time");
    }
    vm->_profiler = &lp->profiler;
    lp->profiler.begin();
}

_LpGuard::~_LpGuard(){
    vm->_profiler = nullptr;
    lp->profiler.end();
}

void add_module_line_profiler(VM *vm){
    PyObject* mod = vm->new_module("line_profiler");
    LineProfilerW::register_class(vm, mod);
}

}   // namespace pkpy



namespace pkpy{

#ifdef PK_USE_CJSON
void add_module_cjson(VM* vm);
#endif

void init_builtins(VM* _vm) {
#define BIND_NUM_ARITH_OPT(name, op)                                                                    \
    _vm->bind##name(VM::tp_int, [](VM* vm, PyObject* lhs, PyObject* rhs) {                              \
        if(is_int(rhs)) return VAR(_CAST(i64, lhs) op _CAST(i64, rhs));                                 \
        if(is_float(rhs)) return VAR(_CAST(i64, lhs) op _CAST(f64, rhs));                               \
        return vm->NotImplemented;                                                                      \
    });                                                                                                 \
    _vm->bind##name(VM::tp_float, [](VM* vm, PyObject* lhs, PyObject* rhs) {                           \
        if(is_float(rhs)) return VAR(_CAST(f64, lhs) op _CAST(f64, rhs));                               \
        if(is_int(rhs)) return VAR(_CAST(f64, lhs) op _CAST(i64, rhs));                                 \
        return vm->NotImplemented;                                                                      \
    });

    BIND_NUM_ARITH_OPT(__add__, +)
    BIND_NUM_ARITH_OPT(__sub__, -)
    BIND_NUM_ARITH_OPT(__mul__, *)

#undef BIND_NUM_ARITH_OPT

#define BIND_NUM_LOGICAL_OPT(name, op)   \
    _vm->bind##name(VM::tp_int, [](VM* vm, PyObject* lhs, PyObject* rhs) {      \
        i64 val;                                                                \
        if(try_cast_int(rhs, &val)) return VAR(_CAST(i64, lhs) op val);         \
        if(is_float(rhs))   return VAR(_CAST(i64, lhs) op _CAST(f64, rhs));     \
        return vm->NotImplemented;                                              \
    });                                                                         \
    _vm->bind##name(VM::tp_float, [](VM* vm, PyObject* lhs, PyObject* rhs) {    \
        i64 val;                                                                \
        if(try_cast_int(rhs, &val)) return VAR(_CAST(f64, lhs) op val);         \
        if(is_float(rhs))   return VAR(_CAST(f64, lhs) op _CAST(f64, rhs));     \
        return vm->NotImplemented;                                              \
    });

    BIND_NUM_LOGICAL_OPT(__eq__, ==)
    BIND_NUM_LOGICAL_OPT(__lt__, <)
    BIND_NUM_LOGICAL_OPT(__le__, <=)
    BIND_NUM_LOGICAL_OPT(__gt__, >)
    BIND_NUM_LOGICAL_OPT(__ge__, >=)
    
#undef BIND_NUM_ARITH_OPT
#undef BIND_NUM_LOGICAL_OPT

    // builtin functions
    _vm->bind_func<-1>(_vm->builtins, "super", [](VM* vm, ArgsView args) {
        PyObject* class_arg = nullptr;
        PyObject* self_arg = nullptr;
        if(args.size() == 2){
            class_arg = args[0];
            self_arg = args[1];
        }else if(args.size() == 0){
            FrameId frame = vm->top_frame();
            if(frame->_callable != nullptr){
                class_arg = PK_OBJ_GET(Function, frame->_callable)._class;
                if(frame->_locals.size() > 0) self_arg = frame->_locals[0];
            }
            if(class_arg == nullptr || self_arg == nullptr){
                vm->TypeError("super(): unable to determine the class context, use super(class, self) instead");
            }
        }else{
            vm->TypeError("super() takes 0 or 2 arguments");
        }
        vm->check_non_tagged_type(class_arg, vm->tp_type);
        Type type = PK_OBJ_GET(Type, class_arg);
        if(!vm->isinstance(self_arg, type)){
            StrName _0 = _type_name(vm, vm->_tp(self_arg));
            StrName _1 = _type_name(vm, type);
            vm->TypeError("super(): " + _0.escape() + " is not an instance of " + _1.escape());
        }
        return vm->heap.gcnew<Super>(vm->tp_super, self_arg, vm->_all_types[type].base);
    });

    _vm->bind_func<1>(_vm->builtins, "staticmethod", [](VM* vm, ArgsView args) {
        PyObject* func = args[0];
        vm->check_non_tagged_type(func, vm->tp_function);
        return vm->heap.gcnew<StaticMethod>(vm->tp_staticmethod, args[0]);
    });

    _vm->bind_func<1>(_vm->builtins, "classmethod", [](VM* vm, ArgsView args) {
        PyObject* func = args[0];
        vm->check_non_tagged_type(func, vm->tp_function);
        return vm->heap.gcnew<ClassMethod>(vm->tp_classmethod, args[0]);
    });

    _vm->bind_func<2>(_vm->builtins, "isinstance", [](VM* vm, ArgsView args) {
        if(is_non_tagged_type(args[1], vm->tp_tuple)){
            Tuple& types = _CAST(Tuple&, args[1]);
            for(PyObject* type : types){
                vm->check_non_tagged_type(type, vm->tp_type);
                if(vm->isinstance(args[0], PK_OBJ_GET(Type, type))) return vm->True;
            }
            return vm->False;
        }
        vm->check_non_tagged_type(args[1], vm->tp_type);
        Type type = PK_OBJ_GET(Type, args[1]);
        return VAR(vm->isinstance(args[0], type));
    });

    _vm->bind_func<2>(_vm->builtins, "issubclass", [](VM* vm, ArgsView args) {
        vm->check_non_tagged_type(args[0], vm->tp_type);
        vm->check_non_tagged_type(args[1], vm->tp_type);
        return VAR(vm->issubclass(PK_OBJ_GET(Type, args[0]), PK_OBJ_GET(Type, args[1])));
    });

    _vm->bind_func<0>(_vm->builtins, "globals", [](VM* vm, ArgsView args) {
        PyObject* mod = vm->top_frame()->_module;
        return VAR(MappingProxy(mod));
    });

    _vm->bind(_vm->builtins, "round(x, ndigits=0)", [](VM* vm, ArgsView args) {
        f64 x = CAST(f64, args[0]);
        int ndigits = CAST(int, args[1]);
        if(ndigits == 0){
            return x >= 0 ? VAR((i64)(x + 0.5)) : VAR((i64)(x - 0.5));
        }
        if(ndigits < 0) vm->ValueError("ndigits should be non-negative");
        if(x >= 0){
            return VAR((i64)(x * std::pow(10, ndigits) + 0.5) / std::pow(10, ndigits));
        }else{
            return VAR((i64)(x * std::pow(10, ndigits) - 0.5) / std::pow(10, ndigits));
        }
    });

    _vm->bind_func<1>(_vm->builtins, "abs", [](VM* vm, ArgsView args) {
        if(is_int(args[0])) return VAR(std::abs(_CAST(i64, args[0])));
        if(is_float(args[0])) return VAR(std::abs(_CAST(f64, args[0])));
        vm->TypeError("bad operand type for abs()");
        return vm->None;
    });

    _vm->bind_func<1>(_vm->builtins, "id", [](VM* vm, ArgsView args) {
        PyObject* obj = args[0];
        if(is_tagged(obj)) return vm->None;
        return VAR(PK_BITS(obj));
    });

    _vm->bind_func<1>(_vm->builtins, "callable", [](VM* vm, ArgsView args) {
        return VAR(vm->py_callable(args[0]));
    });

    _vm->bind_func<1>(_vm->builtins, "__import__", [](VM* vm, ArgsView args) {
        const Str& name = CAST(Str&, args[0]);
        return vm->py_import(name);
    });

    _vm->bind_func<2>(_vm->builtins, "divmod", [](VM* vm, ArgsView args) {
        if(is_int(args[0])){
            i64 lhs = _CAST(i64, args[0]);
            i64 rhs = CAST(i64, args[1]);
            if(rhs == 0) vm->ZeroDivisionError();
            auto res = std::div(lhs, rhs);
            return VAR(Tuple(VAR(res.quot), VAR(res.rem)));
        }else{
            return vm->call_method(args[0], __divmod__, args[1]);
        }
    });

    _vm->bind(_vm->builtins, "eval(__source, __globals=None)", [](VM* vm, ArgsView args) {
        CodeObject_ code = vm->compile(CAST(Str&, args[0]), "<eval>", EVAL_MODE, true);
        PyObject* globals = args[1];
        if(globals == vm->None){
            FrameId frame = vm->top_frame();
            return vm->_exec(code.get(), frame->_module, frame->_callable, frame->_locals);
        }
        vm->check_non_tagged_type(globals, vm->tp_mappingproxy);
        PyObject* obj = PK_OBJ_GET(MappingProxy, globals).obj;
        return vm->_exec(code, obj);
    });

    _vm->bind(_vm->builtins, "exec(__source, __globals=None)", [](VM* vm, ArgsView args) {
        CodeObject_ code = vm->compile(CAST(Str&, args[0]), "<exec>", EXEC_MODE, true);
        PyObject* globals = args[1];
        if(globals == vm->None){
            FrameId frame = vm->top_frame();
            vm->_exec(code.get(), frame->_module, frame->_callable, frame->_locals);
            return vm->None;
        }
        vm->check_non_tagged_type(globals, vm->tp_mappingproxy);
        PyObject* obj = PK_OBJ_GET(MappingProxy, globals).obj;
        vm->_exec(code, obj);
        return vm->None;
    });

    _vm->bind(_vm->builtins, "exit(code=0)", [](VM* vm, ArgsView args) {
        std::exit(CAST(int, args[0]));
        return vm->None;
    });

    _vm->bind_func<1>(_vm->builtins, "repr", [](VM* vm, ArgsView args){
        return vm->py_repr(args[0]);
    });

    _vm->bind_func<1>(_vm->builtins, "len", [](VM* vm, ArgsView args){
        const PyTypeInfo* ti = vm->_inst_type_info(args[0]);
        if(ti->m__len__) return VAR(ti->m__len__(vm, args[0]));
        return vm->call_method(args[0], __len__);
    });

    _vm->bind_func<1>(_vm->builtins, "hash", [](VM* vm, ArgsView args){
        i64 value = vm->py_hash(args[0]);
        return VAR(value);
    });

    _vm->bind_func<1>(_vm->builtins, "chr", [](VM* vm, ArgsView args) {
        i64 i = CAST(i64, args[0]);
        if (i < 0 || i >= 128) vm->ValueError("chr() arg not in [0, 128)");
        return VAR(std::string(1, (char)i));
    });

    _vm->bind_func<1>(_vm->builtins, "ord", [](VM* vm, ArgsView args) {
        const Str& s = CAST(Str&, args[0]);
        if (s.length()!=1) vm->TypeError("ord() expected an ASCII character");
        return VAR((i64)(s[0]));
    });

    _vm->bind_func<2>(_vm->builtins, "hasattr", [](VM* vm, ArgsView args) {
        return VAR(vm->getattr(args[0], CAST(Str&, args[1]), false) != nullptr);
    });

    _vm->bind_func<3>(_vm->builtins, "setattr", [](VM* vm, ArgsView args) {
        vm->setattr(args[0], CAST(Str&, args[1]), args[2]);
        return vm->None;
    });

    _vm->bind_func<-1>(_vm->builtins, "getattr", [](VM* vm, ArgsView args) {
        if(args.size()!=2 && args.size()!=3) vm->TypeError("getattr() takes 2 or 3 arguments");
        StrName name = CAST(Str&, args[1]);
        PyObject* val = vm->getattr(args[0], name, false);
        if(val == nullptr){
            if(args.size()==2) vm->AttributeError(args[0], name);
            return args[2];
        }
        return val;
    });

    _vm->bind_func<2>(_vm->builtins, "delattr", [](VM* vm, ArgsView args) {
        vm->delattr(args[0], CAST(Str&, args[1]));
        return vm->None;
    });

    _vm->bind_func<1>(_vm->builtins, "hex", [](VM* vm, ArgsView args) {
        SStream ss;
        ss.write_hex(CAST(i64, args[0]));
        return VAR(ss.str());
    });

    _vm->bind_func<1>(_vm->builtins, "iter", [](VM* vm, ArgsView args) {
        return vm->py_iter(args[0]);
    });

    _vm->bind_func<1>(_vm->builtins, "next", [](VM* vm, ArgsView args) {
        return vm->py_next(args[0]);
    });

    _vm->bind_func<1>(_vm->builtins, "bin", [](VM* vm, ArgsView args) {
        SStream ss;
        i64 x = CAST(i64, args[0]);
        if(x < 0){ ss << "-"; x = -x; }
        ss << "0b";
        std::string bits;
        while(x){
            bits += (x & 1) ? '1' : '0';
            x >>= 1;
        }
        std::reverse(bits.begin(), bits.end());
        if(bits.empty()) bits = "0";
        ss << bits;
        return VAR(ss.str());
    });

    _vm->bind_func<1>(_vm->builtins, "dir", [](VM* vm, ArgsView args) {
        std::set<StrName> names;
        if(!is_tagged(args[0]) && args[0]->is_attr_valid()){
            std::vector<StrName> keys = args[0]->attr().keys();
            names.insert(keys.begin(), keys.end());
        }
        const NameDict& t_attr = vm->_t(args[0])->attr();
        std::vector<StrName> keys = t_attr.keys();
        names.insert(keys.begin(), keys.end());
        List ret;
        for (StrName name : names) ret.push_back(VAR(name.sv()));
        return VAR(std::move(ret));
    });

    // tp_object
    _vm->bind__repr__(VM::tp_object, [](VM* vm, PyObject* obj) {
        if(is_tagged(obj)) PK_FATAL_ERROR();
        SStream ss;
        ss << "<" << _type_name(vm, vm->_tp(obj)) << " object at ";
        ss.write_hex(obj);
        ss << ">";
        return VAR(ss.str());
    });

    _vm->bind__eq__(VM::tp_object, [](VM* vm, PyObject* _0, PyObject* _1) {
        return VAR(_0 == _1); 
    });

    _vm->cached_object__new__ = _vm->bind_constructor<1>(_vm->_t(VM::tp_object), [](VM* vm, ArgsView args) {
        vm->check_non_tagged_type(args[0], vm->tp_type);
        Type t = PK_OBJ_GET(Type, args[0]);
        return vm->heap.gcnew<DummyInstance>(t);
    });

    _vm->bind_method<0>(VM::tp_object, "_enable_instance_dict", [](VM* vm, ArgsView args){
        PyObject* self = args[0];
        if(is_tagged(self)){
            vm->TypeError("object: tagged object cannot enable instance dict");
        }
        if(self->is_attr_valid()){
            vm->TypeError("object: instance dict is already enabled");
        }
        self->_enable_instance_dict();
        return vm->None;
    });

    // tp_type
    _vm->bind_constructor<2>(_vm->_t(VM::tp_type), PK_LAMBDA(vm->_t(args[1])));

    // tp_range
    _vm->bind_constructor<-1>(_vm->_t(VM::tp_range), [](VM* vm, ArgsView args) {
        args._begin += 1;   // skip cls
        Range r;
        switch (args.size()) {
            case 1: r.stop = CAST(i64, args[0]); break;
            case 2: r.start = CAST(i64, args[0]); r.stop = CAST(i64, args[1]); break;
            case 3: r.start = CAST(i64, args[0]); r.stop = CAST(i64, args[1]); r.step = CAST(i64, args[2]); break;
            default: vm->TypeError("expected 1-3 arguments, got " + std::to_string(args.size()));
        }
        return VAR(r);
    });

    _vm->bind__iter__(VM::tp_range, [](VM* vm, PyObject* obj) { return VAR_T(RangeIter, PK_OBJ_GET(Range, obj)); });
    
    // tp_nonetype
    _vm->bind__repr__(_vm->_tp(_vm->None), [](VM* vm, PyObject* _0) {
        return VAR("None"); 
    });

    // tp_float / tp_float
    _vm->bind__truediv__(VM::tp_float, [](VM* vm, PyObject* _0, PyObject* _1) {
        f64 value = CAST_F(_1);
        return VAR(_CAST(f64, _0) / value);
    });
    _vm->bind__truediv__(VM::tp_int, [](VM* vm, PyObject* _0, PyObject* _1) {
        f64 value = CAST_F(_1);
        return VAR(_CAST(i64, _0) / value);
    });

    auto py_number_pow = [](VM* vm, PyObject* _0, PyObject* _1) {
        i64 lhs, rhs;
        if(try_cast_int(_0, &lhs) && try_cast_int(_1, &rhs)){
            if(rhs < 0) {
                if(lhs == 0) vm->ZeroDivisionError("0.0 cannot be raised to a negative power");
                return VAR((f64)std::pow(lhs, rhs));
            }
            i64 ret = 1;
            while(rhs){
                if(rhs & 1) ret *= lhs;
                lhs *= lhs;
                rhs >>= 1;
            }
            return VAR(ret);
        }else{
            return VAR((f64)std::pow(CAST_F(_0), CAST_F(_1)));
        }
    };

    _vm->bind__pow__(VM::tp_int, py_number_pow);
    _vm->bind__pow__(VM::tp_float, py_number_pow);

    _vm->bind_constructor<-1>(_vm->_t(VM::tp_int), [](VM* vm, ArgsView args) {
        if(args.size() == 1+0) return VAR(0);
        // 1 arg
        if(args.size() == 1+1){
            if (is_type(args[1], vm->tp_float)) return VAR((i64)CAST(f64, args[1]));
            if (is_type(args[1], vm->tp_int)) return args[1];
            if (is_type(args[1], vm->tp_bool)) return VAR(_CAST(bool, args[1]) ? 1 : 0);
        }
        if(args.size() > 1+2) vm->TypeError("int() takes at most 2 arguments");
        // 2 args
        if (is_type(args[1], vm->tp_str)) {
            int base = 10;
            if(args.size() == 1+2) base = CAST(i64, args[2]);
            const Str& s = CAST(Str&, args[1]);
            i64 val;
            if(!parse_int(s.sv(), &val, base)){
                vm->ValueError("invalid literal for int(): " + s.escape());
            }
            return VAR(val);
        }
        vm->TypeError("invalid arguments for int()");
        return vm->None;
    });

    _vm->bind__floordiv__(VM::tp_int, [](VM* vm, PyObject* _0, PyObject* _1) {
        i64 rhs = CAST(i64, _1);
        if(rhs == 0) vm->ZeroDivisionError();
        return VAR(_CAST(i64, _0) / rhs);
    });

    _vm->bind__mod__(VM::tp_int, [](VM* vm, PyObject* _0, PyObject* _1) {
        i64 rhs = CAST(i64, _1);
        if(rhs == 0) vm->ZeroDivisionError();
        return VAR(_CAST(i64, _0) % rhs);
    });

    _vm->bind__repr__(VM::tp_int, [](VM* vm, PyObject* obj) { return VAR(std::to_string(_CAST(i64, obj))); });
    _vm->bind__neg__(VM::tp_int, [](VM* vm, PyObject* obj) { return VAR(-_CAST(i64, obj)); });
    _vm->bind__hash__(VM::tp_int, [](VM* vm, PyObject* obj) { return _CAST(i64, obj); });
    _vm->bind__invert__(VM::tp_int, [](VM* vm, PyObject* obj) { return VAR(~_CAST(i64, obj)); });

#define INT_BITWISE_OP(name, op) \
    _vm->bind##name(VM::tp_int, [](VM* vm, PyObject* lhs, PyObject* rhs) { \
        return VAR(_CAST(i64, lhs) op CAST(i64, rhs)); \
    });

    INT_BITWISE_OP(__lshift__, <<)
    INT_BITWISE_OP(__rshift__, >>)
    INT_BITWISE_OP(__and__, &)
    INT_BITWISE_OP(__or__, |)
    INT_BITWISE_OP(__xor__, ^)

#undef INT_BITWISE_OP

    _vm->bind_constructor<-1>(_vm->_t(VM::tp_float), [](VM* vm, ArgsView args) {
        if(args.size() == 1+0) return VAR(0.0);
        if(args.size() > 1+1) vm->TypeError("float() takes at most 1 argument");
        // 1 arg
        if (is_type(args[1], vm->tp_int)) return VAR((f64)CAST(i64, args[1]));
        if (is_type(args[1], vm->tp_float)) return args[1];
        if (is_type(args[1], vm->tp_bool)) return VAR(_CAST(bool, args[1]) ? 1.0 : 0.0);
        if (is_type(args[1], vm->tp_str)) {
            const Str& s = CAST(Str&, args[1]);
            if(s == "inf") return VAR(INFINITY);
            if(s == "-inf") return VAR(-INFINITY);

            double float_out;
            char* p_end;
            try{
                float_out = std::strtod(s.data, &p_end);
                PK_ASSERT(p_end == s.end());
            }catch(...){
                vm->ValueError("invalid literal for float(): " + s.escape());
            }
            return VAR(float_out);
        }
        vm->TypeError("invalid arguments for float()");
        return vm->None;
    });

    _vm->bind__hash__(VM::tp_float, [](VM* vm, PyObject* _0) {
        f64 val = _CAST(f64, _0);
        return (i64)std::hash<f64>()(val);
    });

    _vm->bind__neg__(VM::tp_float, [](VM* vm, PyObject* _0) { return VAR(-_CAST(f64, _0)); });

    _vm->bind__repr__(VM::tp_float, [](VM* vm, PyObject* _0) {
        f64 val = _CAST(f64, _0);
        SStream ss;
        ss << val;
        return VAR(ss.str());
    });

    // tp_str
    _vm->bind_constructor<2>(_vm->_t(VM::tp_str), PK_LAMBDA(vm->py_str(args[1])));

    _vm->bind__hash__(VM::tp_str, [](VM* vm, PyObject* _0) {
        return (i64)_CAST(Str&, _0).hash();
    });

    _vm->bind__add__(VM::tp_str, [](VM* vm, PyObject* _0, PyObject* _1) {
        return VAR(_CAST(Str&, _0) + CAST(Str&, _1));
    });
    _vm->bind__len__(VM::tp_str, [](VM* vm, PyObject* _0) {
        return (i64)_CAST(Str&, _0).u8_length();
    });
    _vm->bind__mul__(VM::tp_str, [](VM* vm, PyObject* _0, PyObject* _1) {
        const Str& self = _CAST(Str&, _0);
        i64 n = CAST(i64, _1);
        SStream ss;
        for(i64 i = 0; i < n; i++) ss << self.sv();
        return VAR(ss.str());
    });
    _vm->bind_method<1>(VM::tp_str, "__rmul__", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        i64 n = CAST(i64, args[1]);
        SStream ss;
        for(i64 i = 0; i < n; i++) ss << self.sv();
        return VAR(ss.str());
    });
    _vm->bind__contains__(VM::tp_str, [](VM* vm, PyObject* _0, PyObject* _1) {
        const Str& self = _CAST(Str&, _0);
        return VAR(self.index(CAST(Str&, _1)) != -1);
    });
    _vm->bind__str__(VM::tp_str, [](VM* vm, PyObject* _0) { return _0; });
    _vm->bind__iter__(VM::tp_str, [](VM* vm, PyObject* _0) { return VAR_T(StringIter, _0); });
    _vm->bind__repr__(VM::tp_str, [](VM* vm, PyObject* _0) {
        const Str& self = _CAST(Str&, _0);
        return VAR(self.escape());
    });

#define BIND_CMP_STR(name, op) \
    _vm->bind##name(VM::tp_str, [](VM* vm, PyObject* lhs, PyObject* rhs) { \
        if(!is_non_tagged_type(rhs, vm->tp_str)) return vm->NotImplemented; \
        return VAR(_CAST(Str&, lhs) op _CAST(Str&, rhs));                   \
    });

    BIND_CMP_STR(__eq__, ==)
    BIND_CMP_STR(__lt__, <)
    BIND_CMP_STR(__le__, <=)
    BIND_CMP_STR(__gt__, >)
    BIND_CMP_STR(__ge__, >=)
#undef BIND_CMP_STR

    _vm->bind__getitem__(VM::tp_str, [](VM* vm, PyObject* _0, PyObject* _1) {
        const Str& self = _CAST(Str&, _0);
        if(is_non_tagged_type(_1, vm->tp_slice)){
            const Slice& s = _CAST(Slice&, _1);
            int start, stop, step;
            vm->parse_int_slice(s, self.u8_length(), start, stop, step);
            return VAR(self.u8_slice(start, stop, step));
        }
        int i = CAST(int, _1);
        i = vm->normalized_index(i, self.u8_length());
        return VAR(self.u8_getitem(i));
    });

    _vm->bind(_vm->_t(VM::tp_str), "replace(self, old, new, count=-1)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& old = CAST(Str&, args[1]);
        if(old.empty()) vm->ValueError("empty substring");
        const Str& new_ = CAST(Str&, args[2]);
        int count = CAST(int, args[3]);
        return VAR(self.replace(old, new_, count));
    });

    _vm->bind(_vm->_t(VM::tp_str), "split(self, sep=' ')", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& sep = CAST(Str&, args[1]);
        if(sep.empty()) vm->ValueError("empty separator");
        std::vector<std::string_view> parts;
        if(sep.size == 1){
            parts = self.split(sep[0]);
        }else{
            parts = self.split(sep);
        }
        List ret(parts.size());
        for(int i=0; i<parts.size(); i++) ret[i] = VAR(Str(parts[i]));
        return VAR(std::move(ret));
    });

    _vm->bind(_vm->_t(VM::tp_str), "splitlines(self)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        std::vector<std::string_view> parts;
        parts = self.split('\n');
        List ret(parts.size());
        for(int i=0; i<parts.size(); i++) ret[i] = VAR(Str(parts[i]));
        return VAR(std::move(ret));
    });

    _vm->bind(_vm->_t(VM::tp_str), "count(self, s: str)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& s = CAST(Str&, args[1]);
        return VAR(self.count(s));
    });

    _vm->bind_method<1>(VM::tp_str, "index", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& sub = CAST(Str&, args[1]);
        int index = self.index(sub);
        if(index == -1) vm->ValueError("substring not found");
        return VAR(index);
    });

    _vm->bind_method<1>(VM::tp_str, "find", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& sub = CAST(Str&, args[1]);
        return VAR(self.index(sub));
    });

    _vm->bind_method<1>(VM::tp_str, "startswith", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& prefix = CAST(Str&, args[1]);
        return VAR(self.index(prefix) == 0);
    });

    _vm->bind_method<1>(VM::tp_str, "endswith", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& suffix = CAST(Str&, args[1]);
        int offset = self.length() - suffix.length();
        if(offset < 0) return vm->False;
        bool ok = memcmp(self.data+offset, suffix.data, suffix.length()) == 0;
        return VAR(ok);
    });

    _vm->bind_method<0>(VM::tp_str, "encode", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        unsigned char* buffer = new unsigned char[self.length()];
        memcpy(buffer, self.data, self.length());
        return VAR(Bytes(buffer, self.length()));
    });

    _vm->bind_method<1>(VM::tp_str, "join", [](VM* vm, ArgsView args) {
        auto _lock = vm->heap.gc_scope_lock();
        const Str& self = _CAST(Str&, args[0]);
        SStream ss;
        PyObject* it = vm->py_iter(args[1]);     // strong ref
        PyObject* obj = vm->py_next(it);
        while(obj != vm->StopIteration){
            if(!ss.empty()) ss << self;
            ss << CAST(Str&, obj);
            obj = vm->py_next(it);
        }
        return VAR(ss.str());
    });

    _vm->bind_method<0>(VM::tp_str, "lower", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        return VAR(self.lower());
    });

    _vm->bind_method<0>(VM::tp_str, "upper", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        return VAR(self.upper());
    });

    _vm->bind(_vm->_t(VM::tp_str), "strip(self, chars=None)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        if(args[1] == vm->None){
            return VAR(self.strip());
        }else{
            const Str& chars = CAST(Str&, args[1]);
            return VAR(self.strip(true, true, chars));
        }
    });

    _vm->bind(_vm->_t(VM::tp_str), "lstrip(self, chars=None)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        if(args[1] == vm->None){
            return VAR(self.lstrip());
        }else{
            const Str& chars = CAST(Str&, args[1]);
            return VAR(self.strip(true, false, chars));
        }
    });

    _vm->bind(_vm->_t(VM::tp_str), "rstrip(self, chars=None)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        if(args[1] == vm->None){
            return VAR(self.rstrip());
        }else{
            const Str& chars = CAST(Str&, args[1]);
            return VAR(self.strip(false, true, chars));
        }
    });

    // zfill
    _vm->bind(_vm->_t(VM::tp_str), "zfill(self, width)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        int width = CAST(int, args[1]);
        int delta = width - self.u8_length();
        if(delta <= 0) return args[0];
        SStream ss;
        for(int i=0; i<delta; i++) ss << '0';
        ss << self;
        return VAR(ss.str());
    });

    // ljust
    _vm->bind(_vm->_t(VM::tp_str), "ljust(self, width, fillchar=' ')", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        int width = CAST(int, args[1]);
        int delta = width - self.u8_length();
        if(delta <= 0) return args[0];
        const Str& fillchar = CAST(Str&, args[2]);
        SStream ss;
        ss << self;
        for(int i=0; i<delta; i++) ss << fillchar;
        return VAR(ss.str());
    });

    // rjust
    _vm->bind(_vm->_t(VM::tp_str), "rjust(self, width, fillchar=' ')", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        int width = CAST(int, args[1]);
        int delta = width - self.u8_length();
        if(delta <= 0) return args[0];
        const Str& fillchar = CAST(Str&, args[2]);
        SStream ss;
        for(int i=0; i<delta; i++) ss << fillchar;
        ss << self;
        return VAR(ss.str());
    });

    // tp_list / tp_tuple
    _vm->bind(_vm->_t(VM::tp_list), "sort(self, key=None, reverse=False)", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        PyObject* key = args[1];
        if(key == vm->None){
            std::stable_sort(self.begin(), self.end(), [vm](PyObject* a, PyObject* b){
                return vm->py_lt(a, b);
            });
        }else{
            std::stable_sort(self.begin(), self.end(), [vm, key](PyObject* a, PyObject* b){
                return vm->py_lt(vm->call(key, a), vm->call(key, b));
            });
        }
        bool reverse = CAST(bool, args[2]);
        if(reverse) self.reverse();
        return vm->None;
    });

    _vm->bind__repr__(VM::tp_list, [](VM* vm, PyObject* _0){
        if(vm->_repr_recursion_set.count(_0)) return VAR("[...]");
        List& iterable = _CAST(List&, _0);
        SStream ss;
        ss << '[';
        vm->_repr_recursion_set.insert(_0);
        for(int i=0; i<iterable.size(); i++){
            ss << CAST(Str&, vm->py_repr(iterable[i]));
            if(i != iterable.size()-1) ss << ", ";
        }
        vm->_repr_recursion_set.erase(_0);
        ss << ']';
        return VAR(ss.str());
    });

    _vm->bind__repr__(VM::tp_tuple, [](VM* vm, PyObject* _0){
        Tuple& iterable = _CAST(Tuple&, _0);
        SStream ss;
        ss << '(';
        if(iterable.size() == 1){
            ss << CAST(Str&, vm->py_repr(iterable[0]));
            ss << ',';
        }else{
            for(int i=0; i<iterable.size(); i++){
                ss << CAST(Str&, vm->py_repr(iterable[i]));
                if(i != iterable.size()-1) ss << ", ";
            }
        }
        ss << ')';
        return VAR(ss.str());
    });

    _vm->bind_constructor<-1>(_vm->_t(VM::tp_list), [](VM* vm, ArgsView args) {
        if(args.size() == 1+0) return VAR(List());
        if(args.size() == 1+1) return vm->py_list(args[1]);
        vm->TypeError("list() takes 0 or 1 arguments");
        return vm->None;
    });

    _vm->bind__contains__(VM::tp_list, [](VM* vm, PyObject* _0, PyObject* _1) {
        List& self = _CAST(List&, _0);
        for(PyObject* i: self) if(vm->py_eq(i, _1)) return vm->True;
        return vm->False;
    });

    _vm->bind_method<1>(VM::tp_list, "count", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        int count = 0;
        for(PyObject* i: self) if(vm->py_eq(i, args[1])) count++;
        return VAR(count);
    });

    _vm->bind__eq__(VM::tp_list, [](VM* vm, PyObject* _0, PyObject* _1) {
        List& a = _CAST(List&, _0);
        if(!is_non_tagged_type(_1, vm->tp_list)) return vm->NotImplemented;
        List& b = _CAST(List&, _1);
        if(a.size() != b.size()) return vm->False;
        for(int i=0; i<a.size(); i++){
            if(!vm->py_eq(a[i], b[i])) return vm->False;
        }
        return vm->True;
    });

    _vm->bind_method<1>(VM::tp_list, "index", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        PyObject* obj = args[1];
        for(int i=0; i<self.size(); i++){
            if(vm->py_eq(self[i], obj)) return VAR(i);
        }
        vm->ValueError(_CAST(Str&, vm->py_repr(obj)) + " is not in list");
        return vm->None;
    });

    _vm->bind_method<1>(VM::tp_list, "remove", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        PyObject* obj = args[1];
        for(int i=0; i<self.size(); i++){
            if(vm->py_eq(self[i], obj)){
                self.erase(i);
                return vm->None;
            }
        }
        vm->ValueError(_CAST(Str&, vm->py_repr(obj)) + " is not in list");
        return vm->None;
    });

    _vm->bind_method<-1>(VM::tp_list, "pop", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        if(args.size() == 1+0){
            if(self.empty()) vm->IndexError("pop from empty list");
            return self.popx_back();
        }
        if(args.size() == 1+1){
            int index = CAST(int, args[1]);
            index = vm->normalized_index(index, self.size());
            PyObject* ret = self[index];
            self.erase(index);
            return ret;
        }
        vm->TypeError("pop() takes at most 1 argument");
        return vm->None;
    });

    _vm->bind_method<1>(VM::tp_list, "append", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        self.push_back(args[1]);
        return vm->None;
    });

    _vm->bind_method<1>(VM::tp_list, "extend", [](VM* vm, ArgsView args) {
        auto _lock = vm->heap.gc_scope_lock();
        List& self = _CAST(List&, args[0]);
        PyObject* it = vm->py_iter(args[1]);     // strong ref
        PyObject* obj = vm->py_next(it);
        while(obj != vm->StopIteration){
            self.push_back(obj);
            obj = vm->py_next(it);
        }
        return vm->None;
    });

    _vm->bind_method<0>(VM::tp_list, "reverse", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        std::reverse(self.begin(), self.end());
        return vm->None;
    });

    _vm->bind__mul__(VM::tp_list, [](VM* vm, PyObject* _0, PyObject* _1) {
        const List& self = _CAST(List&, _0);
        if(!is_int(_1)) return vm->NotImplemented;
        int n = _CAST(int, _1);
        List result;
        result.reserve(self.size() * n);
        for(int i = 0; i < n; i++) result.extend(self);
        return VAR(std::move(result));
    });
    _vm->bind_method<1>(VM::tp_list, "__rmul__", [](VM* vm, ArgsView args) {
        const List& self = _CAST(List&, args[0]);
        if(!is_int(args[1])) return vm->NotImplemented;
        int n = _CAST(int, args[1]);
        List result;
        result.reserve(self.size() * n);
        for(int i = 0; i < n; i++) result.extend(self);
        return VAR(std::move(result));
    });

    _vm->bind_method<2>(VM::tp_list, "insert", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        int index = CAST(int, args[1]);
        if(index < 0) index += self.size();
        if(index < 0) index = 0;
        if(index > self.size()) index = self.size();
        self.insert(index, args[2]);
        return vm->None;
    });

    _vm->bind_method<0>(VM::tp_list, "clear", [](VM* vm, ArgsView args) {
        _CAST(List&, args[0]).clear();
        return vm->None;
    });

    _vm->bind_method<0>(VM::tp_list, "copy", PK_LAMBDA(VAR(_CAST(List, args[0]))));

#define BIND_RICH_CMP(name, op, _t, _T)    \
    _vm->bind__##name##__(_vm->_t, [](VM* vm, PyObject* lhs, PyObject* rhs){        \
        if(!is_non_tagged_type(rhs, vm->_t)) return vm->NotImplemented;             \
        auto& a = _CAST(_T&, lhs);                                                  \
        auto& b = _CAST(_T&, rhs);                                                  \
        for(int i=0; i<a.size() && i<b.size(); i++){                                \
            if(vm->py_eq(a[i], b[i])) continue;                                     \
            return VAR(vm->py_##name(a[i], b[i]));                                  \
        }                                                                           \
        return VAR(a.size() op b.size());                                           \
    });

    BIND_RICH_CMP(lt, <, tp_list, List)
    BIND_RICH_CMP(le, <=, tp_list, List)
    BIND_RICH_CMP(gt, >, tp_list, List)
    BIND_RICH_CMP(ge, >=, tp_list, List)

    BIND_RICH_CMP(lt, <, tp_tuple, Tuple)
    BIND_RICH_CMP(le, <=, tp_tuple, Tuple)
    BIND_RICH_CMP(gt, >, tp_tuple, Tuple)
    BIND_RICH_CMP(ge, >=, tp_tuple, Tuple)

#undef BIND_RICH_CMP

    _vm->bind__add__(VM::tp_list, [](VM* vm, PyObject* _0, PyObject* _1) {
        const List& self = _CAST(List&, _0);
        const List& other = CAST(List&, _1);
        List new_list(self);    // copy construct
        new_list.extend(other);
        return VAR(std::move(new_list));
    });

    _vm->bind__len__(VM::tp_list, [](VM* vm, PyObject* _0) {
        return (i64)_CAST(List&, _0).size();
    });
    _vm->bind__iter__(VM::tp_list, [](VM* vm, PyObject* _0) {
        List& self = _CAST(List&, _0);
        return VAR_T(ArrayIter, _0, self.begin(), self.end());
    });
    _vm->bind__getitem__(VM::tp_list, PyArrayGetItem<List>);
    _vm->bind__setitem__(VM::tp_list, [](VM* vm, PyObject* _0, PyObject* _1, PyObject* _2){
        List& self = _CAST(List&, _0);
        int i = CAST(int, _1);
        i = vm->normalized_index(i, self.size());
        self[i] = _2;
    });
    _vm->bind__delitem__(VM::tp_list, [](VM* vm, PyObject* _0, PyObject* _1){
        List& self = _CAST(List&, _0);
        int i = CAST(int, _1);
        i = vm->normalized_index(i, self.size());
        self.erase(i);
    });

    _vm->bind_constructor<-1>(_vm->_t(VM::tp_tuple), [](VM* vm, ArgsView args) {
        if(args.size() == 1+0) return VAR(Tuple(0));
        if(args.size() == 1+1){
            List list(CAST(List, vm->py_list(args[1])));
            return VAR(Tuple(std::move(list)));
        }
        vm->TypeError("tuple() takes at most 1 argument");
        return vm->None;
    });

    _vm->bind__contains__(VM::tp_tuple, [](VM* vm, PyObject* obj, PyObject* item) {
        Tuple& self = _CAST(Tuple&, obj);
        for(PyObject* i: self) if(vm->py_eq(i, item)) return vm->True;
        return vm->False;
    });

    _vm->bind_method<1>(VM::tp_tuple, "count", [](VM* vm, ArgsView args) {
        Tuple& self = _CAST(Tuple&, args[0]);
        int count = 0;
        for(PyObject* i: self) if(vm->py_eq(i, args[1])) count++;
        return VAR(count);
    });

    _vm->bind__eq__(VM::tp_tuple, [](VM* vm, PyObject* _0, PyObject* _1) {
        const Tuple& self = _CAST(Tuple&, _0);
        if(!is_non_tagged_type(_1, vm->tp_tuple)) return vm->NotImplemented;
        const Tuple& other = _CAST(Tuple&, _1);
        if(self.size() != other.size()) return vm->False;
        for(int i = 0; i < self.size(); i++) {
            if(!vm->py_eq(self[i], other[i])) return vm->False;
        }
        return vm->True;
    });

    _vm->bind__hash__(VM::tp_tuple, [](VM* vm, PyObject* _0) {
        i64 x = 1000003;
        for (PyObject* item: _CAST(Tuple&, _0)) {
            i64 y = vm->py_hash(item);
            // recommended by Github Copilot
            x = x ^ (y + 0x9e3779b9 + (x << 6) + (x >> 2));
        }
        return x;
    });

    _vm->bind__iter__(VM::tp_tuple, [](VM* vm, PyObject* _0) {
        Tuple& self = _CAST(Tuple&, _0);
        return VAR_T(ArrayIter, _0, self.begin(), self.end());
    });
    _vm->bind__getitem__(VM::tp_tuple, PyArrayGetItem<Tuple>);
    _vm->bind__len__(VM::tp_tuple, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(Tuple&, obj).size();
    });

    // tp_bool
    _vm->bind_constructor<2>(_vm->_t(VM::tp_bool), PK_LAMBDA(VAR(vm->py_bool(args[1]))));
    _vm->bind__hash__(VM::tp_bool, [](VM* vm, PyObject* _0) {
        return (i64)_CAST(bool, _0);
    });
    _vm->bind__repr__(VM::tp_bool, [](VM* vm, PyObject* _0) {
        bool val = _CAST(bool, _0);
        return VAR(val ? "True" : "False");
    });

    _vm->bind__and__(VM::tp_bool, [](VM* vm, PyObject* _0, PyObject* _1) {
        return VAR(_CAST(bool, _0) && CAST(bool, _1));
    });
    _vm->bind__or__(VM::tp_bool, [](VM* vm, PyObject* _0, PyObject* _1) {
        return VAR(_CAST(bool, _0) || CAST(bool, _1));
    });
    _vm->bind__xor__(VM::tp_bool, [](VM* vm, PyObject* _0, PyObject* _1) {
        return VAR(_CAST(bool, _0) != CAST(bool, _1));
    });
    _vm->bind__eq__(VM::tp_bool, [](VM* vm, PyObject* _0, PyObject* _1) {
        if(is_non_tagged_type(_1, vm->tp_bool)) return VAR(_0 == _1);
        if(is_int(_1)) return VAR(_CAST(bool, _0) == (bool)CAST(i64, _1));
        return vm->NotImplemented;
    });

    // tp_ellipsis / tp_NotImplementedType
    _vm->bind__repr__(_vm->_tp(_vm->Ellipsis), [](VM* vm, PyObject* _0) {
        return VAR("...");
    });
    _vm->bind__repr__(_vm->_tp(_vm->NotImplemented), [](VM* vm, PyObject* _0) {
        return VAR("NotImplemented");
    });

    // tp_bytes
    _vm->bind_constructor<2>(_vm->_t(VM::tp_bytes), [](VM* vm, ArgsView args){
        List& list = CAST(List&, args[1]);
        std::vector<unsigned char> buffer(list.size());
        for(int i=0; i<list.size(); i++){
            i64 b = CAST(i64, list[i]);
            if(b<0 || b>255) vm->ValueError("byte must be in range[0, 256)");
            buffer[i] = (char)b;
        }
        return VAR(Bytes(buffer));
    });

    _vm->bind__getitem__(VM::tp_bytes, [](VM* vm, PyObject* obj, PyObject* index) {
        const Bytes& self = _CAST(Bytes&, obj);
        int i = CAST(int, index);
        i = vm->normalized_index(i, self.size());
        return VAR(self[i]);
    });

    _vm->bind__hash__(VM::tp_bytes, [](VM* vm, PyObject* _0) {
        const Bytes& self = _CAST(Bytes&, _0);
        std::string_view view((char*)self.data(), self.size());
        return (i64)std::hash<std::string_view>()(view);
    });

    _vm->bind__repr__(VM::tp_bytes, [](VM* vm, PyObject* _0) {
        const Bytes& self = _CAST(Bytes&, _0);
        SStream ss;
        ss << "b'";
        for(int i=0; i<self.size(); i++){
            ss << "\\x";
            ss.write_hex((unsigned char)self[i]);
        }
        ss << "'";
        return VAR(ss.str());
    });
    _vm->bind__len__(VM::tp_bytes, [](VM* vm, PyObject* _0) {
        return (i64)_CAST(Bytes&, _0).size();
    });

    _vm->bind_method<0>(VM::tp_bytes, "decode", [](VM* vm, ArgsView args) {
        const Bytes& self = _CAST(Bytes&, args[0]);
        // TODO: check encoding is utf-8
        return VAR(Str(self.str()));
    });

    _vm->bind__eq__(VM::tp_bytes, [](VM* vm, PyObject* _0, PyObject* _1) {
        if(!is_non_tagged_type(_1, vm->tp_bytes)) return vm->NotImplemented;
        return VAR(_CAST(Bytes&, _0) == _CAST(Bytes&, _1));
    });
    
    // tp_slice
    _vm->bind_constructor<4>(_vm->_t(VM::tp_slice), [](VM* vm, ArgsView args) {
        return VAR(Slice(args[1], args[2], args[3]));
    });

    _vm->bind__eq__(VM::tp_slice, [](VM* vm, PyObject* _0, PyObject* _1){
        const Slice& self = _CAST(Slice&, _0);
        if(!is_non_tagged_type(_1, vm->tp_slice)) return vm->NotImplemented;
        const Slice& other = _CAST(Slice&, _1);
        if(vm->py_ne(self.start, other.start)) return vm->False;
        if(vm->py_ne(self.stop, other.stop)) return vm->False;
        if(vm->py_ne(self.step, other.step)) return vm->False;
        return vm->True;
    });

    _vm->bind__repr__(VM::tp_slice, [](VM* vm, PyObject* _0) {
        const Slice& self = _CAST(Slice&, _0);
        SStream ss;
        ss << "slice(";
        ss << CAST(Str, vm->py_repr(self.start)) << ", ";
        ss << CAST(Str, vm->py_repr(self.stop)) << ", ";
        ss << CAST(Str, vm->py_repr(self.step)) << ")";
        return VAR(ss.str());
    });

    // tp_mappingproxy
    _vm->bind_method<0>(VM::tp_mappingproxy, "keys", [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        List keys;
        for(StrName name : self.attr().keys()) keys.push_back(VAR(name.sv()));
        return VAR(std::move(keys));
    });

    _vm->bind_method<0>(VM::tp_mappingproxy, "values", [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        List values;
        for(auto& item : self.attr().items()) values.push_back(item.second);
        return VAR(std::move(values));
    });

    _vm->bind_method<0>(VM::tp_mappingproxy, "items", [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        List items;
        for(auto& item : self.attr().items()){
            PyObject* t = VAR(Tuple(VAR(item.first.sv()), item.second));
            items.push_back(std::move(t));
        }
        return VAR(std::move(items));
    });

    _vm->bind__len__(VM::tp_mappingproxy, [](VM* vm, PyObject* _0) {
        return (i64)_CAST(MappingProxy&, _0).attr().size();
    });

    _vm->bind__eq__(VM::tp_mappingproxy, [](VM* vm, PyObject* _0, PyObject* _1){
        const MappingProxy& a = _CAST(MappingProxy&, _0);
        if(!is_non_tagged_type(_1, vm->tp_mappingproxy)) return vm->NotImplemented;
        const MappingProxy& b = _CAST(MappingProxy&, _1);
        return VAR(a.obj == b.obj);
    });

    _vm->bind__getitem__(VM::tp_mappingproxy, [](VM* vm, PyObject* _0, PyObject* _1) {
        MappingProxy& self = _CAST(MappingProxy&, _0);
        StrName key = CAST(Str&, _1);
        PyObject* ret = self.attr().try_get_likely_found(key);
        if(ret == nullptr) vm->KeyError(_1);
        return ret;
    });

    _vm->bind(_vm->_t(VM::tp_mappingproxy), "get(self, key, default=None)", [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        StrName key = CAST(Str&, args[1]);
        PyObject* ret = self.attr().try_get(key);
        if(ret == nullptr) return args[2];
        return ret;
    });

    _vm->bind__repr__(VM::tp_mappingproxy, [](VM* vm, PyObject* _0) {
        if(vm->_repr_recursion_set.count(_0)) return VAR("{...}");
        MappingProxy& self = _CAST(MappingProxy&, _0);
        SStream ss;
        ss << "mappingproxy({";
        bool first = true;
        vm->_repr_recursion_set.insert(_0);
        for(auto& item : self.attr().items()){
            if(!first) ss << ", ";
            first = false;
            ss << item.first.escape() << ": ";
            ss << CAST(Str, vm->py_repr(item.second));
        }
        vm->_repr_recursion_set.erase(_0);
        ss << "})";
        return VAR(ss.str());
    });

    _vm->bind__contains__(VM::tp_mappingproxy, [](VM* vm, PyObject* _0, PyObject* _1) {
        MappingProxy& self = _CAST(MappingProxy&, _0);
        return VAR(self.attr().contains(CAST(Str&, _1)));
    });

    // tp_dict
    _vm->bind_constructor<-1>(_vm->_t(VM::tp_dict), [](VM* vm, ArgsView args){
        return VAR(Dict(vm));
    });

    _vm->bind_method<-1>(VM::tp_dict, "__init__", [](VM* vm, ArgsView args){
        if(args.size() == 1+0) return vm->None;
        if(args.size() == 1+1){
            auto _lock = vm->heap.gc_scope_lock();
            Dict& self = _CAST(Dict&, args[0]);
            List& list = CAST(List&, args[1]);
            for(PyObject* item : list){
                Tuple& t = CAST(Tuple&, item);
                if(t.size() != 2){
                    vm->ValueError("dict() takes an iterable of tuples (key, value)");
                    return vm->None;
                }
                self.set(t[0], t[1]);
            }
            return vm->None;
        }
        vm->TypeError("dict() takes at most 1 argument");
        return vm->None;
    });

    _vm->bind__len__(VM::tp_dict, [](VM* vm, PyObject* _0) {
        return (i64)_CAST(Dict&, _0).size();
    });

    _vm->bind__getitem__(VM::tp_dict, [](VM* vm, PyObject* _0, PyObject* _1) {
        Dict& self = _CAST(Dict&, _0);
        PyObject* ret = self.try_get(_1);
        if(ret == nullptr) vm->KeyError(_1);
        return ret;
    });

    _vm->bind__setitem__(VM::tp_dict, [](VM* vm, PyObject* _0, PyObject* _1, PyObject* _2) {
        Dict& self = _CAST(Dict&, _0);
        self.set(_1, _2);
    });

    _vm->bind__delitem__(VM::tp_dict, [](VM* vm, PyObject* _0, PyObject* _1) {
        Dict& self = _CAST(Dict&, _0);
        bool ok = self.erase(_1);
        if(!ok) vm->KeyError(_1);
    });

    _vm->bind_method<-1>(VM::tp_dict, "pop", [](VM* vm, ArgsView args) {
        if(args.size() != 2 && args.size() != 3){
            vm->TypeError("pop() expected 1 or 2 arguments");
            return vm->None;
        }
        Dict& self = _CAST(Dict&, args[0]);
        PyObject* value = self.try_get(args[1]);
        if(value == nullptr){
            if(args.size() == 2) vm->KeyError(args[1]);
            if(args.size() == 3){
                return args[2];
            }
        }
        self.erase(args[1]);
        return value;
    });

    _vm->bind__contains__(VM::tp_dict, [](VM* vm, PyObject* _0, PyObject* _1) {
        Dict& self = _CAST(Dict&, _0);
        return VAR(self.contains(_1));
    });

    _vm->bind__iter__(VM::tp_dict, [](VM* vm, PyObject* _0) {
        const Dict& self = _CAST(Dict&, _0);
        return vm->py_iter(VAR(self.keys()));
    });

    _vm->bind_method<-1>(VM::tp_dict, "get", [](VM* vm, ArgsView args) {
        Dict& self = _CAST(Dict&, args[0]);
        if(args.size() == 1+1){
            PyObject* ret = self.try_get(args[1]);
            if(ret != nullptr) return ret;
            return vm->None;
        }else if(args.size() == 1+2){
            PyObject* ret = self.try_get(args[1]);
            if(ret != nullptr) return ret;
            return args[2];
        }
        vm->TypeError("get() takes at most 2 arguments");
        return vm->None;
    });

    _vm->bind_method<0>(VM::tp_dict, "keys", [](VM* vm, ArgsView args) {
        const Dict& self = _CAST(Dict&, args[0]);
        return VAR(self.keys());
    });

    _vm->bind_method<0>(VM::tp_dict, "values", [](VM* vm, ArgsView args) {
        const Dict& self = _CAST(Dict&, args[0]);
        return VAR(self.values());
    });

    _vm->bind_method<0>(VM::tp_dict, "items", [](VM* vm, ArgsView args) {
        const Dict& self = _CAST(Dict&, args[0]);
        Tuple items(self.size());
        int j = 0;
        self.apply([&](PyObject* k, PyObject* v){
            items[j++] = VAR(Tuple(k, v));
        });
        return VAR(std::move(items));
    });

    _vm->bind_method<1>(VM::tp_dict, "update", [](VM* vm, ArgsView args) {
        Dict& self = _CAST(Dict&, args[0]);
        const Dict& other = CAST(Dict&, args[1]);
        self.update(other);
        return vm->None;
    });

    _vm->bind_method<0>(VM::tp_dict, "copy", [](VM* vm, ArgsView args) {
        const Dict& self = _CAST(Dict&, args[0]);
        return VAR(self);
    });

    _vm->bind_method<0>(VM::tp_dict, "clear", [](VM* vm, ArgsView args) {
        Dict& self = _CAST(Dict&, args[0]);
        self.clear();
        return vm->None;
    });

    _vm->bind__repr__(VM::tp_dict, [](VM* vm, PyObject* _0) {
        if(vm->_repr_recursion_set.count(_0)) return VAR("{...}");
        Dict& self = _CAST(Dict&, _0);
        SStream ss;
        ss << "{";
        bool first = true;
        vm->_repr_recursion_set.insert(_0);
        self.apply([&](PyObject* k, PyObject* v){
            if(!first) ss << ", ";
            first = false;
            ss << CAST(Str&, vm->py_repr(k)) << ": " << CAST(Str&, vm->py_repr(v));
        });
        vm->_repr_recursion_set.erase(_0);
        ss << "}";
        return VAR(ss.str());
    });

    _vm->bind__eq__(VM::tp_dict, [](VM* vm, PyObject* _0, PyObject* _1) {
        Dict& self = _CAST(Dict&, _0);
        if(!is_non_tagged_type(_1, vm->tp_dict)) return vm->NotImplemented;
        Dict& other = _CAST(Dict&, _1);
        if(self.size() != other.size()) return vm->False;
        for(int i=0; i<self._capacity; i++){
            auto item = self._items[i];
            if(item.first == nullptr) continue;
            PyObject* value = other.try_get(item.first);
            if(value == nullptr) return vm->False;
            if(!vm->py_eq(item.second, value)) return vm->False;
        }
        return vm->True;
    });

    _vm->bind__repr__(VM::tp_module, [](VM* vm, PyObject* _0) {
        const Str& path = CAST(Str&, _0->attr(__path__));
        return VAR(_S("<module ", path.escape(), ">"));
    });

    // tp_property
    _vm->bind_constructor<-1>(_vm->_t(VM::tp_property), [](VM* vm, ArgsView args) {
        if(args.size() == 1+1){
            return VAR(Property(args[1], vm->None, ""));
        }else if(args.size() == 1+2){
            return VAR(Property(args[1], args[2], ""));
        }else if(args.size() == 1+3){
            return VAR(Property(args[1], args[2], CAST(Str, args[3])));
        }
        vm->TypeError("property() takes at most 3 arguments");
        return vm->None;
    });

    // properties
    _vm->bind_property(_vm->_t(VM::tp_property), "__signature__", [](VM* vm, ArgsView args){
        Property& self = _CAST(Property&, args[0]);
        return VAR(self.signature);
    });
    
    _vm->bind_property(_vm->_t(VM::tp_function), "__doc__", [](VM* vm, ArgsView args) {
        Function& func = _CAST(Function&, args[0]);
        return VAR(func.decl->docstring);
    });

    _vm->bind_property(_vm->_t(VM::tp_native_func), "__doc__", [](VM* vm, ArgsView args) {
        NativeFunc& func = _CAST(NativeFunc&, args[0]);
        if(func.decl != nullptr) return VAR(func.decl->docstring);
        return VAR("");
    });

    _vm->bind_property(_vm->_t(VM::tp_function), "__signature__", [](VM* vm, ArgsView args) {
        Function& func = _CAST(Function&, args[0]);
        return VAR(func.decl->signature);
    });

    _vm->bind_property(_vm->_t(VM::tp_native_func), "__signature__", [](VM* vm, ArgsView args) {
        NativeFunc& func = _CAST(NativeFunc&, args[0]);
        if(func.decl != nullptr) return VAR(func.decl->signature);
        return VAR("");
    });

    // tp_exception
    _vm->bind_constructor<-1>(_vm->_t(VM::tp_exception), [](VM* vm, ArgsView args){
        Type cls = PK_OBJ_GET(Type, args[0]);
        StrName cls_name = _type_name(vm, cls);
        PyObject* e_obj = vm->heap.gcnew<Exception>(cls, cls_name);
        e_obj->_enable_instance_dict();
        PK_OBJ_GET(Exception, e_obj)._self = e_obj;
        return e_obj;
    });

    _vm->bind(_vm->_t(VM::tp_exception), "__init__(self, msg=...)", [](VM* vm, ArgsView args){
        Exception& self = _CAST(Exception&, args[0]);
        if(args[1] == vm->Ellipsis){
            self.msg = "";
        }else{
            self.msg = CAST(Str, args[1]);
        }
        return vm->None;
    });

    _vm->bind__repr__(VM::tp_exception, [](VM* vm, PyObject* _0) {
        Exception& self = _CAST(Exception&, _0);
        return VAR(_S(_type_name(vm, _0->type), '(', self.msg.escape(), ')'));
    });

    _vm->bind__str__(VM::tp_exception, [](VM* vm, PyObject* _0) {
        Exception& self = _CAST(Exception&, _0);
        return VAR(self.msg);
    });

    RangeIter::register_class(_vm, _vm->builtins);
    ArrayIter::register_class(_vm, _vm->builtins);
    StringIter::register_class(_vm, _vm->builtins);
    Generator::register_class(_vm, _vm->builtins);
}

void VM::post_init(){
    init_builtins(this);

    bind_method<-1>(tp_module, "__init__", [](VM* vm, ArgsView args) {
        vm->NotImplementedError();
        return vm->None;
    });

    _all_types[tp_module].m__getattr__ = [](VM* vm, PyObject* obj, StrName name) -> PyObject*{
        const Str& path = CAST(Str&, obj->attr(__path__));
        return vm->py_import(_S(path, ".", name.sv()), false);
    };

    bind_method<1>(tp_property, "setter", [](VM* vm, ArgsView args) {
        Property& self = _CAST(Property&, args[0]);
        // The setter's name is not necessary to be the same as the property's name
        // However, for cpython compatibility, we recommend to use the same name
        self.setter = args[1];
        return args[0];
    });

    // type
    bind__getitem__(tp_type, [](VM* vm, PyObject* self, PyObject* _){
        PK_UNUSED(_);
        return self;        // for generics
    });

    bind__repr__(tp_type, [](VM* vm, PyObject* self){
        SStream ss;
        const PyTypeInfo& info = vm->_all_types[PK_OBJ_GET(Type, self)];
        ss << "<class '" << info.name << "'>";
        return VAR(ss.str());
    });

    bind_property(_t(tp_object), "__class__", PK_LAMBDA(vm->_t(args[0])));
    bind_property(_t(tp_type), "__base__", [](VM* vm, ArgsView args){
        const PyTypeInfo& info = vm->_all_types[PK_OBJ_GET(Type, args[0])];
        return info.base.index == -1 ? vm->None : vm->_all_types[info.base].obj;
    });
    bind_property(_t(tp_type), "__name__", [](VM* vm, ArgsView args){
        const PyTypeInfo& info = vm->_all_types[PK_OBJ_GET(Type, args[0])];
        return VAR(info.name.sv());
    });
    bind_property(_t(tp_type), "__module__", [](VM* vm, ArgsView args){
        const PyTypeInfo& info = vm->_all_types[PK_OBJ_GET(Type, args[0])];
        if(info.mod == nullptr) return vm->None;
        return info.mod;
    });
    bind_property(_t(tp_bound_method), "__self__", [](VM* vm, ArgsView args){
        return CAST(BoundMethod&, args[0]).self;
    });
    bind_property(_t(tp_bound_method), "__func__", [](VM* vm, ArgsView args){
        return CAST(BoundMethod&, args[0]).func;
    });

    bind__eq__(tp_bound_method, [](VM* vm, PyObject* lhs, PyObject* rhs){
        if(!is_non_tagged_type(rhs, vm->tp_bound_method)) return vm->NotImplemented;
        const BoundMethod& _0 = PK_OBJ_GET(BoundMethod, lhs);
        const BoundMethod& _1 = PK_OBJ_GET(BoundMethod, rhs);
        return VAR(_0.self == _1.self && _0.func == _1.func);
    });

    bind_property(_t(tp_slice), "start", [](VM* vm, ArgsView args){
        return CAST(Slice&, args[0]).start;
    });
    bind_property(_t(tp_slice), "stop", [](VM* vm, ArgsView args){
        return CAST(Slice&, args[0]).stop;
    });
    bind_property(_t(tp_slice), "step", [](VM* vm, ArgsView args){
        return CAST(Slice&, args[0]).step;
    });

    bind_property(_t(tp_object), "__dict__", [](VM* vm, ArgsView args){
        if(is_tagged(args[0]) || !args[0]->is_attr_valid()) return vm->None;
        return VAR(MappingProxy(args[0]));
    });

    add_module_sys(this);
    add_module_traceback(this);
    add_module_time(this);
    add_module_json(this);
    add_module_math(this);
    add_module_dis(this);
    add_module_c(this);
    add_module_gc(this);
    add_module_random(this);
    add_module_base64(this);
    add_module_operator(this);

    for(const char* name: {"this", "functools", "heapq", "bisect", "pickle", "_long", "colorsys", "typing", "datetime", "cmath"}){
        _lazy_modules[name] = kPythonLibs[name];
    }

    try{
        CodeObject_ code = compile(kPythonLibs["builtins"], "<builtins>", EXEC_MODE);
        this->_exec(code, this->builtins);
        code = compile(kPythonLibs["_set"], "<set>", EXEC_MODE);
        this->_exec(code, this->builtins);
    }catch(const Exception& e){
        std::cerr << e.summary() << std::endl;
        std::cerr << "failed to load builtins module!!" << std::endl;
        exit(1);
    }

    if(enable_os){
        add_module_io(this);
        add_module_os(this);
        _import_handler = _default_import_handler;
    }

    add_module_csv(this);
    add_module_dataclasses(this);
    add_module_linalg(this);
    add_module_easing(this);
    add_module_collections(this);
    add_module_array2d(this);
    add_module_line_profiler(this);

#ifdef PK_USE_CJSON
    add_module_cjson(this);
#endif
}

CodeObject_ VM::compile(std::string_view source, const Str& filename, CompileMode mode, bool unknown_global_scope) {
    Compiler compiler(this, source, filename, mode, unknown_global_scope);
    try{
        return compiler.compile();
    }catch(const Exception& e){
#if PK_DEBUG_FULL_EXCEPTION
        std::cerr << e.summary() << std::endl;
#endif
        _error(e.self());
        return nullptr;
    }
}

}   // namespace pkpy
#ifndef POCKETPY_C_H 
#define POCKETPY_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef struct pkpy_vm_handle pkpy_vm;
typedef int (*pkpy_CFunction)(pkpy_vm*);
typedef void (*pkpy_COutputHandler)(const char*, int);
typedef unsigned char* (*pkpy_CImportHandler)(const char*, int, int*);
typedef int pkpy_CName;
typedef int pkpy_CType;
typedef const char* pkpy_CString;

/* Basic Functions */
PK_EXPORT pkpy_vm* pkpy_new_vm(bool enable_os);
PK_EXPORT void pkpy_delete_vm(pkpy_vm*);
PK_EXPORT bool pkpy_exec(pkpy_vm*, const char* source);
PK_EXPORT bool pkpy_exec_2(pkpy_vm*, const char* source, const char* filename, int mode, const char* module);

/* Stack Manipulation */
PK_EXPORT bool pkpy_dup(pkpy_vm*, int i);
PK_EXPORT bool pkpy_pop(pkpy_vm*, int n);
PK_EXPORT bool pkpy_pop_top(pkpy_vm*);
PK_EXPORT bool pkpy_dup_top(pkpy_vm*);
PK_EXPORT bool pkpy_rot_two(pkpy_vm*);
PK_EXPORT int pkpy_stack_size(pkpy_vm*);

// int
PK_EXPORT bool pkpy_push_int(pkpy_vm*, int val);
PK_EXPORT bool pkpy_is_int(pkpy_vm*, int i);
PK_EXPORT bool pkpy_to_int(pkpy_vm*, int i, int* out);

// float
PK_EXPORT bool pkpy_push_float(pkpy_vm*, double val);
PK_EXPORT bool pkpy_is_float(pkpy_vm*, int i);
PK_EXPORT bool pkpy_to_float(pkpy_vm*, int i, double* out);

// bool
PK_EXPORT bool pkpy_push_bool(pkpy_vm*, bool val);
PK_EXPORT bool pkpy_is_bool(pkpy_vm*, int i);
PK_EXPORT bool pkpy_to_bool(pkpy_vm*, int i, bool* out);

// string
PK_EXPORT bool pkpy_push_string(pkpy_vm*, pkpy_CString val);
PK_EXPORT bool pkpy_is_string(pkpy_vm*, int i);
PK_EXPORT bool pkpy_to_string(pkpy_vm*, int i, pkpy_CString* out);

// void_p
PK_EXPORT bool pkpy_push_voidp(pkpy_vm*, void* val);
PK_EXPORT bool pkpy_is_voidp(pkpy_vm*, int i);
PK_EXPORT bool pkpy_to_voidp(pkpy_vm*, int i, void** out);

// none
PK_EXPORT bool pkpy_push_none(pkpy_vm*);
PK_EXPORT bool pkpy_is_none(pkpy_vm*, int i);

// special push
PK_EXPORT bool pkpy_push_null(pkpy_vm*);
PK_EXPORT bool pkpy_push_function(pkpy_vm*, const char* sig, pkpy_CFunction val);
PK_EXPORT bool pkpy_push_module(pkpy_vm*, const char* name);

// some opt
PK_EXPORT bool pkpy_getattr(pkpy_vm*, pkpy_CName name);
PK_EXPORT bool pkpy_setattr(pkpy_vm*, pkpy_CName name);
PK_EXPORT bool pkpy_getglobal(pkpy_vm*, pkpy_CName name);
PK_EXPORT bool pkpy_setglobal(pkpy_vm*, pkpy_CName name);
PK_EXPORT bool pkpy_eval(pkpy_vm*, const char* source);
PK_EXPORT bool pkpy_unpack_sequence(pkpy_vm*, int size);
PK_EXPORT bool pkpy_get_unbound_method(pkpy_vm*, pkpy_CName name);
PK_EXPORT bool pkpy_py_repr(pkpy_vm*);
PK_EXPORT bool pkpy_py_str(pkpy_vm*);

/* Error Handling */
PK_EXPORT bool pkpy_error(pkpy_vm*, const char* name, pkpy_CString msg);
PK_EXPORT bool pkpy_check_error(pkpy_vm*);
PK_EXPORT bool pkpy_clear_error(pkpy_vm*, char** message);

/* Callables */
PK_EXPORT bool pkpy_vectorcall(pkpy_vm*, int argc);

/* Special APIs */
PK_EXPORT void pkpy_free(void* p);
#define pkpy_string(__s) (__s)
PK_EXPORT pkpy_CName pkpy_name(const char* s);
PK_EXPORT pkpy_CString pkpy_name_to_string(pkpy_CName name);
PK_EXPORT void pkpy_set_output_handler(pkpy_vm*, pkpy_COutputHandler handler);
PK_EXPORT void pkpy_set_import_handler(pkpy_vm*, pkpy_CImportHandler handler);

/* REPL */
PK_EXPORT void* pkpy_new_repl(pkpy_vm*);
PK_EXPORT bool pkpy_repl_input(void* r, const char* line);
PK_EXPORT void pkpy_delete_repl(void* repl);
#ifdef __cplusplus
}
#endif


#endif

#ifndef PK_NO_EXPORT_C_API

using namespace pkpy;

typedef int (*LuaStyleFuncC)(VM*);

#define PK_ASSERT_N_EXTRA_ELEMENTS(n) \
    int __ex_count = count_extra_elements(vm, n); \
    if(__ex_count < n){ \
        Str msg = _S("expected at least ", n, " elements, got ", __ex_count); \
        pkpy_error(vm_handle, "StackError", pkpy_string(msg.c_str())); \
        return false; \
    }

#define PK_ASSERT_NO_ERROR() \
    if(vm->_c.error != nullptr) \
        return false;

static int count_extra_elements(VM* vm, int n){
    if(vm->callstack.empty()){
        return vm->s_data.size();
    }
    PK_ASSERT(!vm->_c.s_view.empty());
    return vm->s_data._sp - vm->_c.s_view.top().end();
}

static PyObject* stack_item(VM* vm, int index){
    PyObject** begin;
    PyObject** end = vm->s_data.end();
    if(vm->callstack.empty()){
        begin = vm->s_data.begin();
    }else{
        PK_ASSERT(!vm->_c.s_view.empty());
        begin = vm->_c.s_view.top().begin();
    }
    int size = end - begin;
    if(index < 0) index += size;
    if(index < 0 || index >= size){
        throw std::runtime_error("stack_item() => index out of range");
    }
    return begin[index];
}

#define PK_PROTECTED(__B) \
    try{ __B }  \
    catch(const Exception& e ) { \
        vm->_c.error = e.self(); \
        return false; \
    } catch(const std::exception& re){ \
        PyObject* e_t = vm->_t(vm->tp_exception); \
        vm->_c.error = vm->call(e_t, VAR(re.what())); \
        return false; \
    }

pkpy_vm* pkpy_new_vm(bool enable_os){
    return (pkpy_vm*)new VM(enable_os);
}

void pkpy_delete_vm(pkpy_vm* vm){
    return delete (VM*)vm;
}

bool pkpy_exec(pkpy_vm* vm_handle, const char* source) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* res;
    PK_PROTECTED(
        CodeObject_ code = vm->compile(source, "main.py", EXEC_MODE);
        res = vm->_exec(code, vm->_main);
    )
    return res != nullptr;
}

bool pkpy_exec_2(pkpy_vm* vm_handle, const char* source, const char* filename, int mode, const char* module){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* res;
    PyObject* mod;
    PK_PROTECTED(
        if(module == nullptr){
            mod = vm->_main;
        }else{
            mod = vm->_modules[module];     // may raise
        }
        CodeObject_ code = vm->compile(source, filename, (CompileMode)mode);
        res = vm->_exec(code, mod);
    )
    return res != nullptr;
}

bool pkpy_dup(pkpy_vm* vm_handle, int n){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, n);
        vm->s_data.push(item);
    )
    return true;
}

bool pkpy_pop(pkpy_vm* vm_handle, int n){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(n)
    vm->s_data.shrink(n);
    return true;
}

bool pkpy_pop_top(pkpy_vm* vm_handle){
    VM* vm = (VM*)vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(1)
    vm->s_data.pop();
    return true;
}

bool pkpy_dup_top(pkpy_vm* vm_handle){
    VM* vm = (VM*)vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(1)
    vm->s_data.push(vm->s_data.top());
    return true;
}

bool pkpy_rot_two(pkpy_vm* vm_handle){
    VM* vm = (VM*)vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(2)
    std::swap(vm->s_data.top(), vm->s_data.second());
    return true;
}

int pkpy_stack_size(pkpy_vm* vm_handle){
    VM* vm = (VM*)vm_handle;
    PK_ASSERT_NO_ERROR()
    if(vm->callstack.empty()){
        return vm->s_data.size();
    }
    if(vm->_c.s_view.empty()) exit(127);
    return vm->s_data._sp - vm->_c.s_view.top().begin();
}

// int
bool pkpy_push_int(pkpy_vm* vm_handle, int value) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* res;
    PK_PROTECTED(
        // int may overflow so we should protect it
        res = py_var(vm, value);
    )
    vm->s_data.push(res);
    return true;
}

bool pkpy_is_int(pkpy_vm* vm_handle, int i){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        return is_int(stack_item(vm, i));
    )
}

bool pkpy_to_int(pkpy_vm* vm_handle, int i, int* out){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        *out = py_cast<int>(vm, item);
    )
    return true;
}

// float
bool pkpy_push_float(pkpy_vm* vm_handle, double value) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* res = py_var(vm, value);
    vm->s_data.push(res);
    return true;
}

bool pkpy_is_float(pkpy_vm* vm_handle, int i){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        return is_float(item);
    )
}

bool pkpy_to_float(pkpy_vm* vm_handle, int i, double* out){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        *out = py_cast<double>(vm, item);
    )
    return true;
}

// bool
bool pkpy_push_bool(pkpy_vm* vm_handle, bool value) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    vm->s_data.push(value ? vm->True : vm->False);
    return true;
}

bool pkpy_is_bool(pkpy_vm* vm_handle, int i){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        return is_non_tagged_type(item, vm->tp_bool);
    )
}

bool pkpy_to_bool(pkpy_vm* vm_handle, int i, bool* out){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        *out = py_cast<bool>(vm, item);
    )
    return true;
}

// string
bool pkpy_push_string(pkpy_vm* vm_handle, pkpy_CString value) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* res = py_var(vm, value);
    vm->s_data.push(res);
    return true;
}

bool pkpy_is_string(pkpy_vm* vm_handle, int i){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        return is_non_tagged_type(item, vm->tp_str);
    )
}

bool pkpy_to_string(pkpy_vm* vm_handle, int i, pkpy_CString* out){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        const Str& s = py_cast<Str&>(vm, item);
        *out = s.c_str();
    )
    return true;
}

// void_p
bool pkpy_push_voidp(pkpy_vm* vm_handle, void* value) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* res = py_var(vm, value);
    vm->s_data.push(res);
    return true;
}

bool pkpy_is_voidp(pkpy_vm* vm_handle, int i){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        return is_non_tagged_type(item, VoidP::_type(vm));
    )
}

bool pkpy_to_voidp(pkpy_vm* vm_handle, int i, void** out){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        VoidP& vp = py_cast<VoidP&>(vm, item);
        *out = vp.ptr;
    )
    return true;
}

// none
bool pkpy_push_none(pkpy_vm* vm_handle) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    vm->s_data.push(vm->None);
    return true;
}

bool pkpy_is_none(pkpy_vm* vm_handle, int i){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        return item == vm->None;
    )
}

// null
bool pkpy_push_null(pkpy_vm* vm_handle) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    vm->s_data.push(PY_NULL);
    return true;
}

struct TempViewPopper{
    VM* vm;
    bool used;

    TempViewPopper(VM* vm): vm(vm), used(false) {}

    void restore() noexcept{
        if(used) return;
        vm->_c.s_view.pop();
        used = true;
    }

    ~TempViewPopper(){ restore(); }
};

// function
static PyObject* c_function_wrapper(VM* vm, ArgsView args) {
    LuaStyleFuncC f = lambda_get_userdata<LuaStyleFuncC>(args.begin());
    PyObject** curr_sp = vm->s_data._sp;

    vm->_c.s_view.push(args);
    TempViewPopper _tvp(vm);
    int retc = f(vm);       // may raise, _tvp will handle this via RAII
    _tvp.restore();

    // propagate_if_errored
    if (vm->_c.error != nullptr){
        PyObject* e_obj = PK_OBJ_GET(Exception, vm->_c.error).self();
        vm->_c.error = nullptr;
        vm->_error(e_obj);
        return nullptr;
    }
    PK_ASSERT(retc == vm->s_data._sp-curr_sp);
    if(retc == 0) return vm->None;
    if (retc == 1) return vm->s_data.popx();
    ArgsView ret_view(curr_sp, vm->s_data._sp);
    return py_var(vm, ret_view.to_tuple());
}

bool pkpy_push_function(pkpy_vm* vm_handle, const char* sig, pkpy_CFunction f) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* f_obj;
    PK_PROTECTED(
        f_obj = vm->bind(
            nullptr,
            sig,
            nullptr,
            c_function_wrapper,
            f
        );
    )
    vm->s_data.push(f_obj);
    return true;
}

// special push
bool pkpy_push_module(pkpy_vm* vm_handle, const char* name) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* module = vm->new_module(name);
        vm->s_data.push(module);
    )
    return true;
}

// some opt
bool pkpy_getattr(pkpy_vm* vm_handle, pkpy_CName name) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(1)
    PyObject* o = vm->s_data.top();
    o = vm->getattr(o, StrName(name), false);
    if(o == nullptr) return false;
    vm->s_data.top() = o;
    return true;
}

bool pkpy_setattr(pkpy_vm* vm_handle, pkpy_CName name) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(2)
    PyObject* a = vm->s_data.top();
    PyObject* val = vm->s_data.second();
    PK_PROTECTED(
        vm->setattr(a, StrName(name), val);
    )
    vm->s_data.shrink(2);
    return true;
}

//get global will also get bulitins
bool pkpy_getglobal(pkpy_vm* vm_handle, pkpy_CName name) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* o = vm->_main->attr().try_get(StrName(name));
    if (o == nullptr) {
        o = vm->builtins->attr().try_get(StrName(name));
        if (o == nullptr) return false;
    }
    vm->s_data.push(o);
    return true;
}

bool pkpy_setglobal(pkpy_vm* vm_handle, pkpy_CName name) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(1)
    vm->_main->attr().set(StrName(name), vm->s_data.popx());
    return true;
}

bool pkpy_eval(pkpy_vm* vm_handle, const char* source) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        CodeObject_ co = vm->compile(source, "<eval>", EVAL_MODE);
        PyObject* ret = vm->_exec(co, vm->_main);
        vm->s_data.push(ret);
    )
    return true;
}

bool pkpy_unpack_sequence(pkpy_vm* vm_handle, int n) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(1)
    auto _lock = vm->heap.gc_scope_lock();
    PK_PROTECTED(
        PyObject* _0 = vm->py_iter(vm->s_data.popx());
        for(int i=0; i<n; i++){
            PyObject* _1 = vm->py_next(_0);
            if(_1 == vm->StopIteration) vm->ValueError("not enough values to unpack");
            vm->s_data.push(_1);
        }
        if(vm->py_next(_0) != vm->StopIteration) vm->ValueError("too many values to unpack");
    )
    return true;
}

bool pkpy_get_unbound_method(pkpy_vm* vm_handle, pkpy_CName name){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(1)
    PyObject* o = vm->s_data.top();
    PyObject* self;
    PK_PROTECTED(
        o = vm->get_unbound_method(o, StrName(name), &self);
    )
    vm->s_data.pop();
    vm->s_data.push(o);
    vm->s_data.push(self);
    return true;
}

bool pkpy_py_repr(pkpy_vm* vm_handle) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(1)
    PyObject* item = vm->s_data.top();
    PK_PROTECTED(
        item = vm->py_repr(item);
    )
    vm->s_data.top() = item;
    return true;
}

bool pkpy_py_str(pkpy_vm* vm_handle) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(1)
    PyObject* item = vm->s_data.top();
    PK_PROTECTED(
        item = vm->py_str(item);
    )
    vm->s_data.top() = item;
    return true;
}

/* Error Handling */
bool pkpy_error(pkpy_vm* vm_handle, const char* name, pkpy_CString message) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* e_t = vm->_main->attr().try_get_likely_found(name);
    if(e_t == nullptr){
        e_t = vm->builtins->attr().try_get_likely_found(name);
        if(e_t == nullptr){
            e_t = vm->_t(vm->tp_exception);
            std::cerr << "[warning] pkpy_error(): " << Str(name).escape() << " not found, fallback to 'Exception'" << std::endl;
        }
    }
    vm->_c.error = vm->call(e_t, VAR(message));
    return false;
}

bool pkpy_check_error(pkpy_vm* vm_handle) {
    VM* vm = (VM*) vm_handle;
    return vm->_c.error != nullptr;
}

bool pkpy_clear_error(pkpy_vm* vm_handle, char** message) {
    VM* vm = (VM*) vm_handle;
    // no error
    if (vm->_c.error == nullptr) return false;
    Exception& e = PK_OBJ_GET(Exception, vm->_c.error);
    if (message != nullptr)
        *message = strdup(e.summary().c_str());
    else
        std::cout << e.summary() << std::endl;
    vm->_c.error = nullptr;
    if(vm->callstack.empty()){
        vm->s_data.clear();
    }else{
        if(vm->_c.s_view.empty()) exit(127);
        vm->s_data.reset(vm->_c.s_view.top().end());
    }
    return true;
}

bool pkpy_vectorcall(pkpy_vm* vm_handle, int argc) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(argc + 2)
    PyObject* res;
    PK_PROTECTED(
        res = vm->vectorcall(argc);
    )
    vm->s_data.push(res);
    return true;
}
/*****************************************************************/
void pkpy_free(void* p){
    free(p);
}

pkpy_CName pkpy_name(const char* name){
    return StrName(name).index;
}

pkpy_CString pkpy_name_to_string(pkpy_CName name){
    return StrName(name).c_str();
}

void pkpy_set_output_handler(pkpy_vm* vm_handle, pkpy_COutputHandler handler){
    VM* vm = (VM*) vm_handle;
    vm->_stdout = handler;
}

void pkpy_set_import_handler(pkpy_vm* vm_handle, pkpy_CImportHandler handler){
    VM* vm = (VM*) vm_handle;
    vm->_import_handler = handler;
}

void* pkpy_new_repl(pkpy_vm* vm_handle){
    return new REPL((VM*)vm_handle);
}

bool pkpy_repl_input(void* r, const char* line){
    return ((REPL*)r)->input(line);
}

void pkpy_delete_repl(void* repl){
    delete (REPL*)repl;
}

#endif // PK_NO_EXPORT_C_API

#endif // POCKETPY_H