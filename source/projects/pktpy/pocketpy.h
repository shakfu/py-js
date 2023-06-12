/*
 *  Copyright (c) 2023 blueloveTH
 *  Distributed Under The MIT License
 *  https://github.com/blueloveTH/pocketpy
 */

#ifndef POCKETPY_H
#define POCKETPY_H


#ifdef PK_USER_CONFIG_H

#include "user_config.h"

#else

/*************** feature settings ***************/

// Whether to compile os-related modules or not
#define PK_ENABLE_OS                1
// Enable this if you are working with multi-threading (experimental)
// This triggers necessary locks to make the VM thread-safe
#define PK_ENABLE_THREAD            0

// Whether to use `std::function` to do bindings or not
// By default, functions to be binded must be a C function pointer without capture
// However, someone thinks it's not convenient.
// By setting this to 1, capturing lambdas can be binded,
// but it's slower and may cause severe "code bloat", also needs more time to compile.
#define PK_ENABLE_STD_FUNCTION      0

/*************** debug settings ***************/

// Enable this may help you find bugs
#define DEBUG_EXTRA_CHECK           0

// Do not edit the following settings unless you know what you are doing
#define DEBUG_NO_BUILTIN_MODULES    0
#define DEBUG_DIS_EXEC              0
#define DEBUG_CEVAL_STEP            0
#define DEBUG_FULL_EXCEPTION        0
#define DEBUG_MEMORY_POOL           0
#define DEBUG_NO_MEMORY_POOL        0
#define DEBUG_NO_AUTO_GC            0
#define DEBUG_GC_STATS              0

/*************** internal settings ***************/

// This is the maximum size of the value stack in void* units
// The actual size in bytes equals `sizeof(void*) * PK_VM_STACK_SIZE`
#define PK_VM_STACK_SIZE            32768

// This is the maximum number of arguments in a function declaration
// including positional arguments, keyword-only arguments, and varargs
// (not recommended to change this)
#define PK_MAX_CO_VARNAMES          255

// Hash table load factor (smaller ones mean less collision but more memory)
// For class instance
inline const float kInstAttrLoadFactor = 0.67f;
// For class itself
inline const float kTypeAttrLoadFactor = 0.5f;

#ifdef _WIN32
    inline const char kPlatformSep = '\\';
#else
    inline const char kPlatformSep = '/';
#endif

#ifdef _MSC_VER
#pragma warning (disable:4267)
#pragma warning (disable:4101)
#pragma warning (disable:4244)
#define _CRT_NONSTDC_NO_DEPRECATE
#define strdup _strdup
#endif

#ifdef _MSC_VER
#define PK_ENABLE_COMPUTED_GOTO		0
#define UNREACHABLE()				__assume(0)
#else
#define PK_ENABLE_COMPUTED_GOTO		1
#define UNREACHABLE()				__builtin_unreachable()
#endif


#if DEBUG_CEVAL_STEP && defined(PK_ENABLE_COMPUTED_GOTO)
#undef PK_ENABLE_COMPUTED_GOTO
#endif

/*************** module settings ***************/

#define PK_MODULE_RE                1
#define PK_MODULE_RANDOM            1
#define PK_MODULE_BASE64            1
#define PK_MODULE_LINALG            1
#define PK_MODULE_EASING            1
#define PK_MODULE_REQUESTS          0

#endif


#include <cmath>
#include <cstring>

#include <sstream>
#include <regex>
#include <stdexcept>
#include <vector>
#include <string>
#include <chrono>
#include <string_view>
#include <iomanip>
#include <memory>
#include <iostream>
#include <map>
#include <set>
#include <algorithm>
#include <initializer_list>
#include <variant>
#include <type_traits>

#define PK_VERSION				"1.0.3"

/*******************************************************************************/
#if PK_ENABLE_STD_FUNCTION
#include <functional>
#endif
/*******************************************************************************/

#if PK_ENABLE_THREAD
#define THREAD_LOCAL thread_local
#include <mutex>

struct GIL {
	inline static std::mutex _mutex;
    explicit GIL() { _mutex.lock(); }
    ~GIL() { _mutex.unlock(); }
};
#define GLOBAL_SCOPE_LOCK() auto _lock = GIL();

#else
#define THREAD_LOCAL
#define GLOBAL_SCOPE_LOCK()
#endif

/*******************************************************************************/

namespace pkpy{

namespace std = ::std;

template <size_t T>
struct NumberTraits;

template <>
struct NumberTraits<4> {
	using int_t = int32_t;
	using float_t = float;

	template<typename... Args>
	static int_t stoi(Args&&... args) { return std::stoi(std::forward<Args>(args)...); }
	template<typename... Args>
	static float_t stof(Args&&... args) { return std::stof(std::forward<Args>(args)...); }

	static constexpr int_t c0 = 0b00000000011111111111111111111100;
	static constexpr int_t c1 = 0b11111111111111111111111111111100;
	static constexpr int_t c2 = 0b00000000000000000000000000000011;
};

template <>
struct NumberTraits<8> {
	using int_t = int64_t;
	using float_t = double;

	template<typename... Args>
	static int_t stoi(Args&&... args) { return std::stoll(std::forward<Args>(args)...); }
	template<typename... Args>
	static float_t stof(Args&&... args) { return std::stod(std::forward<Args>(args)...); }

	static constexpr int_t c0 = 0b0000000000001111111111111111111111111111111111111111111111111100;
	static constexpr int_t c1 = 0b1111111111111111111111111111111111111111111111111111111111111100;
	static constexpr int_t c2 = 0b0000000000000000000000000000000000000000000000000000000000000011;
};

using Number = NumberTraits<sizeof(void*)>;
using i64 = Number::int_t;
using f64 = Number::float_t;

static_assert(sizeof(i64) == sizeof(void*));
static_assert(sizeof(f64) == sizeof(void*));
static_assert(std::numeric_limits<f64>::is_iec559);

struct Dummy { };
struct DummyInstance { };
struct DummyModule { };
struct NoReturn { };
struct Discarded { };

struct Type {
	int index;
	Type(): index(-1) {}
	Type(int index): index(index) {}
	bool operator==(Type other) const noexcept { return this->index == other.index; }
	bool operator!=(Type other) const noexcept { return this->index != other.index; }
	operator int() const noexcept { return this->index; }
};

#define CPP_LAMBDA(x) ([](VM* vm, ArgsView args) { return x; })

#ifdef POCKETPY_H
#define FATAL_ERROR() throw std::runtime_error( "L" + std::to_string(__LINE__) + " FATAL_ERROR()!");
#else
#define FATAL_ERROR() throw std::runtime_error( __FILE__ + std::string(":") + std::to_string(__LINE__) + " FATAL_ERROR()!");
#endif

#define PK_ASSERT(x) if(!(x)) FATAL_ERROR();

struct PyObject;
#define BITS(p) (reinterpret_cast<i64>(p))
inline bool is_tagged(PyObject* p) noexcept { return (BITS(p) & 0b11) != 0b00; }
inline bool is_int(PyObject* p) noexcept { return (BITS(p) & 0b11) == 0b01; }
inline bool is_float(PyObject* p) noexcept { return (BITS(p) & 0b11) == 0b10; }
inline bool is_special(PyObject* p) noexcept { return (BITS(p) & 0b11) == 0b11; }

inline bool is_both_int_or_float(PyObject* a, PyObject* b) noexcept {
    return is_tagged(a) && is_tagged(b);
}

inline bool is_both_int(PyObject* a, PyObject* b) noexcept {
    return is_int(a) && is_int(b);
}

// special singals, is_tagged() for them is true
inline PyObject* const PY_NULL = (PyObject*)0b000011;		// tagged null
inline PyObject* const PY_OP_CALL = (PyObject*)0b100011;
inline PyObject* const PY_OP_YIELD = (PyObject*)0b110011;

#define ADD_MODULE_PLACEHOLDER(name) namespace pkpy { inline void add_module_##name(void* vm) { (void)vm; } }

} // namespace pkpy


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
#if DEBUG_MEMORY_POOL
        if(empty()) throw std::runtime_error("DoubleLinkedList::pop_back() called on empty list");
#endif
        tail.prev->prev->next = &tail;
        tail.prev = tail.prev->prev;
        _size--;
    }

    void pop_front(){
#if DEBUG_MEMORY_POOL
        if(empty()) throw std::runtime_error("DoubleLinkedList::pop_front() called on empty list");
#endif
        head.next->next->prev = &head;
        head.next = head.next->next;
        _size--;
    }

    T* back() const {
#if DEBUG_MEMORY_POOL
        if(empty()) throw std::runtime_error("DoubleLinkedList::back() called on empty list");
#endif
        return static_cast<T*>(tail.prev);
    }

    T* front() const {
#if DEBUG_MEMORY_POOL
        if(empty()) throw std::runtime_error("DoubleLinkedList::front() called on empty list");
#endif
        return static_cast<T*>(head.next);
    }

    void erase(T* node){
#if DEBUG_MEMORY_POOL
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

    void move_all_back(DoubleLinkedList<T>& other){
        if(other.empty()) return;
        other.tail.prev->next = &tail;
        tail.prev->next = other.head.next;
        other.head.next->prev = tail.prev;
        tail.prev = other.tail.prev;
        _size += other._size;
        other.head.next = &other.tail;
        other.tail.prev = &other.head;
        other._size = 0;
    }

    bool empty() const {
#if DEBUG_MEMORY_POOL
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
    struct Block{
        void* arena;
        char data[__BlockSize];
    };

    struct Arena: LinkedListNode{
        Block _blocks[__MaxBlocks];
        Block* _free_list[__MaxBlocks];
        int _free_list_size;
        bool dirty;
        
        Arena(): _free_list_size(__MaxBlocks), dirty(false){
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
#if DEBUG_MEMORY_POOL
            if(empty()) throw std::runtime_error("Arena::alloc() called on empty arena");
#endif
            _free_list_size--;
            return _free_list[_free_list_size];
        }

        void dealloc(Block* block){
#if DEBUG_MEMORY_POOL
            if(full()) throw std::runtime_error("Arena::dealloc() called on full arena");
#endif
            _free_list[_free_list_size] = block;
            _free_list_size++;
        }
    };

    DoubleLinkedList<Arena> _arenas;
    DoubleLinkedList<Arena> _empty_arenas;

    template<typename __T>
    void* alloc() { return alloc(sizeof(__T)); }

    void* alloc(size_t size){
        GLOBAL_SCOPE_LOCK();
#if DEBUG_NO_MEMORY_POOL
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
            arena->dirty = true;
            _empty_arenas.push_back(arena);
        }
        return p;
    }

    void dealloc(void* p){
        GLOBAL_SCOPE_LOCK();
#if DEBUG_NO_MEMORY_POOL
        free(p);
        return;
#endif
#if DEBUG_MEMORY_POOL
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
                if(arena->full() && arena->dirty){
                    _arenas.erase(arena);
                    delete arena;
                }
            }
        }
    }

    size_t allocated_size() {
        size_t n = 0;
        _arenas.apply([&n](Arena* arena){ n += arena->allocated_size(); });
        _empty_arenas.apply([&n](Arena* arena){ n += arena->allocated_size(); });
        return n;
    }

    ~MemoryPool(){
        _arenas.apply([](Arena* arena){ delete arena; });
        _empty_arenas.apply([](Arena* arena){ delete arena; });
    }
};

inline MemoryPool<64> pool64;
inline MemoryPool<128> pool128;

// get the total memory usage of pkpy (across all VMs)
inline size_t memory_usage(){
    return pool64.allocated_size() + pool128.allocated_size();
}

template <typename T>
struct shared_ptr {
    int* counter;

    T* _t() const noexcept { return (T*)(counter + 1); }
    void _inc_counter() { if(counter) ++(*counter); }
    void _dec_counter() { if(counter && --(*counter) == 0) {((T*)(counter + 1))->~T(); pool128.dealloc(counter);} }

public:
    shared_ptr() : counter(nullptr) {}
    shared_ptr(int* counter) : counter(counter) {}
    shared_ptr(const shared_ptr& other) : counter(other.counter) {
        _inc_counter();
    }
    shared_ptr(shared_ptr&& other) noexcept : counter(other.counter) {
        other.counter = nullptr;
    }
    ~shared_ptr() { _dec_counter(); }

    bool operator==(const shared_ptr& other) const { return counter == other.counter; }
    bool operator!=(const shared_ptr& other) const { return counter != other.counter; }
    bool operator<(const shared_ptr& other) const { return counter < other.counter; }
    bool operator>(const shared_ptr& other) const { return counter > other.counter; }
    bool operator<=(const shared_ptr& other) const { return counter <= other.counter; }
    bool operator>=(const shared_ptr& other) const { return counter >= other.counter; }
    bool operator==(std::nullptr_t) const { return counter == nullptr; }
    bool operator!=(std::nullptr_t) const { return counter != nullptr; }

    shared_ptr& operator=(const shared_ptr& other) {
        _dec_counter();
        counter = other.counter;
        _inc_counter();
        return *this;
    }

    shared_ptr& operator=(shared_ptr&& other) noexcept {
        _dec_counter();
        counter = other.counter;
        other.counter = nullptr;
        return *this;
    }

    T& operator*() const { return *_t(); }
    T* operator->() const { return _t(); }
    T* get() const { return _t(); }

    int use_count() const { 
        return counter ? *counter : 0;
    }

    void reset(){
        _dec_counter();
        counter = nullptr;
    }
};

template <typename T, typename... Args>
shared_ptr<T> make_sp(Args&&... args) {
    int* p = (int*)pool128.alloc(sizeof(int) + sizeof(T));
    *p = 1;
    new(p+1) T(std::forward<Args>(args)...);
    return shared_ptr<T>(p);
}

};  // namespace pkpy



namespace pkpy{

template<typename T>
struct pod_vector{
    static_assert(64 % sizeof(T) == 0);
    static_assert(std::is_pod_v<T>);
    static constexpr int N = 64 / sizeof(T);
    static_assert(N >= 4);
    int _size;
    int _capacity;
    T* _data;

    pod_vector(): _size(0), _capacity(N) {
        _data = (T*)pool64.alloc(_capacity * sizeof(T));
    }

    pod_vector(int size): _size(size), _capacity(std::max(N, size)) {
        _data = (T*)pool64.alloc(_capacity * sizeof(T));
    }

    pod_vector(const pod_vector& other): _size(other._size), _capacity(other._capacity) {
        _data = (T*)pool64.alloc(_capacity * sizeof(T));
        memcpy(_data, other._data, sizeof(T) * _size);
    }

    pod_vector(pod_vector&& other) noexcept {
        _size = other._size;
        _capacity = other._capacity;
        _data = other._data;
        other._data = nullptr;
    }

    pod_vector& operator=(pod_vector&& other) noexcept {
        if(_data!=nullptr) pool64.dealloc(_data);
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
        _data = (T*)pool64.alloc(_capacity * sizeof(T));
        if(old_data!=nullptr){
            memcpy(_data, old_data, sizeof(T) * _size);
            pool64.dealloc(old_data);
        }
    }

    void pop_back() { _size--; }
    T popx_back() { T t = std::move(_data[_size-1]); _size--; return t; }
    void extend(const pod_vector& other){
        for(int i=0; i<other.size(); i++) push_back(other[i]);
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

    void resize(int size){
        if(size > _capacity) reserve(size);
        _size = size;
    }

    ~pod_vector() {
        if(_data!=nullptr) pool64.dealloc(_data);
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

// template <typename T>
// using pod_stack = stack<T, pod_vector<T>>;
} // namespace pkpy


namespace pkpy {

inline int utf8len(unsigned char c, bool suppress=false){
    if((c & 0b10000000) == 0) return 1;
    if((c & 0b11100000) == 0b11000000) return 2;
    if((c & 0b11110000) == 0b11100000) return 3;
    if((c & 0b11111000) == 0b11110000) return 4;
    if((c & 0b11111100) == 0b11111000) return 5;
    if((c & 0b11111110) == 0b11111100) return 6;
    if(!suppress) throw std::runtime_error("invalid utf8 char: " + std::to_string(c));
    return 0;
}

struct Str{
    int size;
    bool is_ascii;
    char* data;

    Str(): size(0), is_ascii(true), data(nullptr) {}

    Str(int size, bool is_ascii): size(size), is_ascii(is_ascii) {
        data = (char*)pool64.alloc(size);
    }

#define STR_INIT()                                  \
        data = (char*)pool64.alloc(size);           \
        for(int i=0; i<size; i++){                  \
            data[i] = s[i];                         \
            if(!isascii(s[i])) is_ascii = false;    \
        }

    Str(const std::string& s): size(s.size()), is_ascii(true) {
        STR_INIT()
    }

    Str(std::string_view s): size(s.size()), is_ascii(true) {
        STR_INIT()
    }

    Str(const char* s): size(strlen(s)), is_ascii(true) {
        STR_INIT()
    }

    Str(const char* s, int len): size(len), is_ascii(true) {
        STR_INIT()
    }

#undef STR_INIT

    Str(const Str& other): size(other.size), is_ascii(other.is_ascii) {
        data = (char*)pool64.alloc(size);
        memcpy(data, other.data, size);
    }

    Str(Str&& other): size(other.size), is_ascii(other.is_ascii), data(other.data) {
        other.data = nullptr;
        other.size = 0;
    }

    const char* begin() const { return data; }
    const char* end() const { return data + size; }
    char operator[](int idx) const { return data[idx]; }
    int length() const { return size; }
    bool empty() const { return size == 0; }
    size_t hash() const{ return std::hash<std::string_view>()(sv()); }

    Str& operator=(const Str& other){
        if(data!=nullptr) pool64.dealloc(data);
        size = other.size;
        is_ascii = other.is_ascii;
        data = (char*)pool64.alloc(size);
        memcpy(data, other.data, size);
        return *this;
    }

    Str& operator=(Str&& other) noexcept{
        if(data!=nullptr) pool64.dealloc(data);
        size = other.size;
        is_ascii = other.is_ascii;
        data = other.data;
        other.data = nullptr;
        return *this;
    }

    ~Str(){
        if(data!=nullptr) pool64.dealloc(data);
    }

    Str operator+(const Str& other) const {
        Str ret(size + other.size, is_ascii && other.is_ascii);
        memcpy(ret.data, data, size);
        memcpy(ret.data + size, other.data, other.size);
        return ret;
    }

    Str operator+(const char* p) const {
        Str other(p);
        return *this + other;
    }

    friend Str operator+(const char* p, const Str& str){
        Str other(p);
        return other + str;
    }

    friend std::ostream& operator<<(std::ostream& os, const Str& str){
        if(str.data!=nullptr) os.write(str.data, str.size);
        return os;
    }

    bool operator==(const Str& other) const {
        if(size != other.size) return false;
        return memcmp(data, other.data, size) == 0;
    }

    bool operator!=(const Str& other) const {
        if(size != other.size) return true;
        return memcmp(data, other.data, size) != 0;
    }

    bool operator==(const std::string_view other) const {
        if(size != (int)other.size()) return false;
        return memcmp(data, other.data(), size) == 0;
    }

    bool operator!=(const std::string_view other) const {
        if(size != (int)other.size()) return true;
        return memcmp(data, other.data(), size) != 0;
    }

    bool operator==(const char* p) const {
        return *this == std::string_view(p);
    }

    bool operator!=(const char* p) const {
        return *this != std::string_view(p);
    }

    bool operator<(const Str& other) const {
        int ret = strncmp(data, other.data, std::min(size, other.size));
        if(ret != 0) return ret < 0;
        return size < other.size;
    }

    bool operator<(const std::string_view other) const {
        int ret = strncmp(data, other.data(), std::min(size, (int)other.size()));
        if(ret != 0) return ret < 0;
        return size < (int)other.size();
    }

    friend bool operator<(const std::string_view other, const Str& str){
        return str > other;
    }

    bool operator>(const Str& other) const {
        int ret = strncmp(data, other.data, std::min(size, other.size));
        if(ret != 0) return ret > 0;
        return size > other.size;
    }

    bool operator<=(const Str& other) const {
        int ret = strncmp(data, other.data, std::min(size, other.size));
        if(ret != 0) return ret < 0;
        return size <= other.size;
    }

    bool operator>=(const Str& other) const {
        int ret = strncmp(data, other.data, std::min(size, other.size));
        if(ret != 0) return ret > 0;
        return size >= other.size;
    }

    Str substr(int start, int len) const {
        Str ret(len, is_ascii);
        memcpy(ret.data, data + start, len);
        return ret;
    }

    Str substr(int start) const {
        return substr(start, size - start);
    }

    char* c_str_dup() const {
        char* p = (char*)malloc(size + 1);
        memcpy(p, data, size);
        p[size] = 0;
        return p;
    }

    std::string_view sv() const {
        return std::string_view(data, size);
    }

    std::string str() const {
        return std::string(data, size);
    }

    Str lstrip() const {
        std::string copy(data, size);
        copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](char c) {
            // std::isspace(c) does not working on windows (Debug)
            return c != ' ' && c != '\t' && c != '\r' && c != '\n';
        }));
        return Str(copy);
    }

    Str lower() const{
        std::string copy(data, size);
        std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c){ return std::tolower(c); });
        return Str(copy);
    }

    Str upper() const{
        std::string copy(data, size);
        std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c){ return std::toupper(c); });
        return Str(copy);
    }

    Str escape(bool single_quote=true) const {
        std::stringstream ss;
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
                default:
                    if ('\x00' <= c && c <= '\x1f') {
                        ss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c;
                    } else {
                        ss << c;
                    }
            }
        }
        ss << (single_quote ? '\'' : '"');
        return ss.str();
    }

    int index(const Str& sub, int start=0) const {
        auto p = std::search(data + start, data + size, sub.data, sub.data + sub.size);
        if(p == data + size) return -1;
        return p - data;
    }

    Str replace(const Str& old, const Str& new_, int count=-1) const {
        std::stringstream ss;
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

    /*************unicode*************/

    int _unicode_index_to_byte(int i) const{
        if(is_ascii) return i;
        int j = 0;
        while(i > 0){
            j += utf8len(data[j]);
            i--;
        }
        return j;
    }

    int _byte_index_to_unicode(int n) const{
        if(is_ascii) return n;
        int cnt = 0;
        for(int i=0; i<n; i++){
            if((data[i] & 0xC0) != 0x80) cnt++;
        }
        return cnt;
    }

    Str u8_getitem(int i) const{
        i = _unicode_index_to_byte(i);
        return substr(i, utf8len(data[i]));
    }

    Str u8_slice(int start, int stop, int step) const{
        std::stringstream ss;
        if(is_ascii){
            for(int i=start; step>0?i<stop:i>stop; i+=step) ss << data[i];
        }else{
            for(int i=start; step>0?i<stop:i>stop; i+=step) ss << u8_getitem(i);
        }
        return ss.str();
    }

    int u8_length() const {
        return _byte_index_to_unicode(size);
    }
};

template<typename... Args>
inline std::string fmt(Args&&... args) {
    std::stringstream ss;
    (ss << ... << args);
    return ss.str();
}

const uint32_t kLoRangeA[] = {170,186,443,448,660,1488,1519,1568,1601,1646,1649,1749,1774,1786,1791,1808,1810,1869,1969,1994,2048,2112,2144,2208,2230,2308,2365,2384,2392,2418,2437,2447,2451,2474,2482,2486,2493,2510,2524,2527,2544,2556,2565,2575,2579,2602,2610,2613,2616,2649,2654,2674,2693,2703,2707,2730,2738,2741,2749,2768,2784,2809,2821,2831,2835,2858,2866,2869,2877,2908,2911,2929,2947,2949,2958,2962,2969,2972,2974,2979,2984,2990,3024,3077,3086,3090,3114,3133,3160,3168,3200,3205,3214,3218,3242,3253,3261,3294,3296,3313,3333,3342,3346,3389,3406,3412,3423,3450,3461,3482,3507,3517,3520,3585,3634,3648,3713,3716,3718,3724,3749,3751,3762,3773,3776,3804,3840,3904,3913,3976,4096,4159,4176,4186,4193,4197,4206,4213,4238,4352,4682,4688,4696,4698,4704,4746,4752,4786,4792,4800,4802,4808,4824,4882,4888,4992,5121,5743,5761,5792,5873,5888,5902,5920,5952,5984,5998,6016,6108,6176,6212,6272,6279,6314,6320,6400,6480,6512,6528,6576,6656,6688,6917,6981,7043,7086,7098,7168,7245,7258,7401,7406,7413,7418,8501,11568,11648,11680,11688,11696,11704,11712,11720,11728,11736,12294,12348,12353,12447,12449,12543,12549,12593,12704,12784,13312,19968,40960,40982,42192,42240,42512,42538,42606,42656,42895,42999,43003,43011,43015,43020,43072,43138,43250,43259,43261,43274,43312,43360,43396,43488,43495,43514,43520,43584,43588,43616,43633,43642,43646,43697,43701,43705,43712,43714,43739,43744,43762,43777,43785,43793,43808,43816,43968,44032,55216,55243,63744,64112,64285,64287,64298,64312,64318,64320,64323,64326,64467,64848,64914,65008,65136,65142,65382,65393,65440,65474,65482,65490,65498,65536,65549,65576,65596,65599,65616,65664,66176,66208,66304,66349,66370,66384,66432,66464,66504,66640,66816,66864,67072,67392,67424,67584,67592,67594,67639,67644,67647,67680,67712,67808,67828,67840,67872,67968,68030,68096,68112,68117,68121,68192,68224,68288,68297,68352,68416,68448,68480,68608,68864,69376,69415,69424,69600,69635,69763,69840,69891,69956,69968,70006,70019,70081,70106,70108,70144,70163,70272,70280,70282,70287,70303,70320,70405,70415,70419,70442,70450,70453,70461,70480,70493,70656,70727,70751,70784,70852,70855,71040,71128,71168,71236,71296,71352,71424,71680,71935,72096,72106,72161,72163,72192,72203,72250,72272,72284,72349,72384,72704,72714,72768,72818,72960,72968,72971,73030,73056,73063,73066,73112,73440,73728,74880,77824,82944,92160,92736,92880,92928,93027,93053,93952,94032,94208,100352,110592,110928,110948,110960,113664,113776,113792,113808,123136,123214,123584,124928,126464,126469,126497,126500,126503,126505,126516,126521,126523,126530,126535,126537,126539,126541,126545,126548,126551,126553,126555,126557,126559,126561,126564,126567,126572,126580,126585,126590,126592,126603,126625,126629,126635,131072,173824,177984,178208,183984,194560};
const uint32_t kLoRangeB[] = {170,186,443,451,660,1514,1522,1599,1610,1647,1747,1749,1775,1788,1791,1808,1839,1957,1969,2026,2069,2136,2154,2228,2237,2361,2365,2384,2401,2432,2444,2448,2472,2480,2482,2489,2493,2510,2525,2529,2545,2556,2570,2576,2600,2608,2611,2614,2617,2652,2654,2676,2701,2705,2728,2736,2739,2745,2749,2768,2785,2809,2828,2832,2856,2864,2867,2873,2877,2909,2913,2929,2947,2954,2960,2965,2970,2972,2975,2980,2986,3001,3024,3084,3088,3112,3129,3133,3162,3169,3200,3212,3216,3240,3251,3257,3261,3294,3297,3314,3340,3344,3386,3389,3406,3414,3425,3455,3478,3505,3515,3517,3526,3632,3635,3653,3714,3716,3722,3747,3749,3760,3763,3773,3780,3807,3840,3911,3948,3980,4138,4159,4181,4189,4193,4198,4208,4225,4238,4680,4685,4694,4696,4701,4744,4749,4784,4789,4798,4800,4805,4822,4880,4885,4954,5007,5740,5759,5786,5866,5880,5900,5905,5937,5969,5996,6000,6067,6108,6210,6264,6276,6312,6314,6389,6430,6509,6516,6571,6601,6678,6740,6963,6987,7072,7087,7141,7203,7247,7287,7404,7411,7414,7418,8504,11623,11670,11686,11694,11702,11710,11718,11726,11734,11742,12294,12348,12438,12447,12538,12543,12591,12686,12730,12799,19893,40943,40980,42124,42231,42507,42527,42539,42606,42725,42895,42999,43009,43013,43018,43042,43123,43187,43255,43259,43262,43301,43334,43388,43442,43492,43503,43518,43560,43586,43595,43631,43638,43642,43695,43697,43702,43709,43712,43714,43740,43754,43762,43782,43790,43798,43814,43822,44002,55203,55238,55291,64109,64217,64285,64296,64310,64316,64318,64321,64324,64433,64829,64911,64967,65019,65140,65276,65391,65437,65470,65479,65487,65495,65500,65547,65574,65594,65597,65613,65629,65786,66204,66256,66335,66368,66377,66421,66461,66499,66511,66717,66855,66915,67382,67413,67431,67589,67592,67637,67640,67644,67669,67702,67742,67826,67829,67861,67897,68023,68031,68096,68115,68119,68149,68220,68252,68295,68324,68405,68437,68466,68497,68680,68899,69404,69415,69445,69622,69687,69807,69864,69926,69956,70002,70006,70066,70084,70106,70108,70161,70187,70278,70280,70285,70301,70312,70366,70412,70416,70440,70448,70451,70457,70461,70480,70497,70708,70730,70751,70831,70853,70855,71086,71131,71215,71236,71338,71352,71450,71723,71935,72103,72144,72161,72163,72192,72242,72250,72272,72329,72349,72440,72712,72750,72768,72847,72966,72969,73008,73030,73061,73064,73097,73112,73458,74649,75075,78894,83526,92728,92766,92909,92975,93047,93071,94026,94032,100343,101106,110878,110930,110951,111355,113770,113788,113800,113817,123180,123214,123627,125124,126467,126495,126498,126500,126503,126514,126519,126521,126523,126530,126535,126537,126539,126543,126546,126548,126551,126553,126555,126557,126559,126562,126564,126570,126578,126583,126588,126590,126601,126619,126627,126633,126651,173782,177972,178205,183969,191456,195101};

inline bool is_unicode_Lo_char(uint32_t c) {
    auto index = std::lower_bound(kLoRangeA, kLoRangeA + 476, c) - kLoRangeA;
    if(c == kLoRangeA[index]) return true;
    index -= 1;
    if(index < 0) return false;
    return c >= kLoRangeA[index] && c <= kLoRangeB[index];
}

struct StrName {
    uint16_t index;
    StrName(): index(0) {}
    explicit StrName(uint16_t index): index(index) {}
    StrName(const char* s): index(get(s).index) {}
    StrName(const Str& s){
        index = get(s.sv()).index;
    }
    std::string_view sv() const { return _r_interned[index-1].sv(); }
    bool empty() const { return index == 0; }

    friend std::ostream& operator<<(std::ostream& os, const StrName& sn){
        return os << sn.sv();
    }

    Str escape() const {
        return _r_interned[index-1].escape();
    }

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

    inline static std::map<Str, uint16_t, std::less<>> _interned;
    inline static std::vector<Str> _r_interned;

    static StrName get(std::string_view s){
        auto it = _interned.find(s);
        if(it != _interned.end()) return StrName(it->second);
        uint16_t index = (uint16_t)(_r_interned.size() + 1);
        _interned[s] = index;
        _r_interned.push_back(s);
        return StrName(index);
    }
};

struct FastStrStream{
    pod_vector<const Str*> parts;

    FastStrStream& operator<<(const Str& s){
        parts.push_back(&s);
        return *this;
    }

    bool empty() const { return parts.empty(); }

    Str str() const{
        int len = 0;
        bool is_ascii = true;
        for(auto& s: parts){
            len += s->length();
            is_ascii &= s->is_ascii;
        }
        Str result(len, is_ascii);
        char* p = result.data;
        for(auto& s: parts){
            memcpy(p, s->data, s->length());
            p += s->length();
        }
        return result;
    }
};

// unary operators
const StrName __repr__ = StrName::get("__repr__");
const StrName __str__ = StrName::get("__str__");
const StrName __hash__ = StrName::get("__hash__");      // unused
const StrName __len__ = StrName::get("__len__");
const StrName __iter__ = StrName::get("__iter__");
const StrName __next__ = StrName::get("__next__");      // unused
const StrName __json__ = StrName::get("__json__");
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
const StrName __sub__ = StrName::get("__sub__");
const StrName __mul__ = StrName::get("__mul__");
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
// indexer
const StrName __getitem__ = StrName::get("__getitem__");
const StrName __setitem__ = StrName::get("__setitem__");
const StrName __delitem__ = StrName::get("__delitem__");

#define DEF_SNAME(name) const static StrName name(#name)

} // namespace pkpy


namespace pkpy {

using List = pod_vector<PyObject*>;

class Tuple {
    PyObject** _args;
    PyObject* _inlined[2];
    int _size;

    bool is_inlined() const { return _args == _inlined; }

    void _alloc(int n){
        if(n <= 2){
            this->_args = _inlined;
        }else{
            this->_args = (PyObject**)pool64.alloc(n * sizeof(void*));
        }
        this->_size = n;
    }

public:
    Tuple(int n){ _alloc(n); }

    Tuple(const Tuple& other){
        _alloc(other._size);
        for(int i=0; i<_size; i++) _args[i] = other._args[i];
    }

    Tuple(Tuple&& other) noexcept {
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

    Tuple(List&& other) noexcept {
        _size = other.size();
        _args = other._data;
        other._data = nullptr;
    }

    Tuple(std::initializer_list<PyObject*> list) {
        _alloc(list.size());
        int i = 0;
        for(PyObject* obj: list) _args[i++] = obj;
    }

    PyObject*& operator[](int i){ return _args[i]; }
    PyObject* operator[](int i) const { return _args[i]; }

    int size() const { return _size; }

    PyObject** begin() const { return _args; }
    PyObject** end() const { return _args + _size; }

    ~Tuple(){ if(!is_inlined()) pool64.dealloc(_args); }
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

    List to_list() const{
        List ret(size());
        for(int i=0; i<size(); i++) ret[i] = _begin[i];
        return ret;
    }

    Tuple to_tuple() const{
        Tuple ret(size());
        for(int i=0; i<size(); i++) ret[i] = _begin[i];
        return ret;
    }
};
}   // namespace pkpy


namespace pkpy{

const uint16_t kHashSeeds[] = {9629, 43049, 13267, 59509, 39251, 1249, 27689, 9719, 19913};

#define _hash(key, mask, hash_seed) ( ( (key).index * (hash_seed) >> 8 ) & (mask) )

inline uint16_t find_perfect_hash_seed(uint16_t capacity, const std::vector<StrName>& keys){
    if(keys.empty()) return kHashSeeds[0];
    static std::set<uint16_t> indices;
    indices.clear();
    std::pair<uint16_t, float> best_score = {kHashSeeds[0], 0.0f};
    const int kHashSeedsSize = sizeof(kHashSeeds) / sizeof(kHashSeeds[0]);
    for(int i=0; i<kHashSeedsSize; i++){
        indices.clear();
        for(auto key: keys){
            uint16_t index = _hash(key, capacity-1, kHashSeeds[i]);
            indices.insert(index);
        }
        float score = indices.size() / (float)keys.size();
        if(score > best_score.second) best_score = {kHashSeeds[i], score};
    }
    return best_score.first;
}

template<typename T>
struct NameDictImpl {
    using Item = std::pair<StrName, T>;
    static constexpr uint16_t __Capacity = 8;
    // ensure the initial capacity is ok for memory pool
    static_assert(std::is_pod_v<T>);
    static_assert(sizeof(Item) * __Capacity <= 128);

    float _load_factor;
    uint16_t _capacity;
    uint16_t _size;
    uint16_t _hash_seed;
    uint16_t _mask;
    Item* _items;

#define HASH_PROBE(key, ok, i)          \
ok = false;                             \
i = _hash(key, _mask, _hash_seed);      \
while(!_items[i].first.empty()) {       \
    if(_items[i].first == (key)) { ok = true; break; }  \
    i = (i + 1) & _mask;                                \
}

#define NAMEDICT_ALLOC()                \
    _items = (Item*)pool128.alloc(_capacity * sizeof(Item));    \
    memset(_items, 0, _capacity * sizeof(Item));                \

    NameDictImpl(float load_factor=0.67f):
        _load_factor(load_factor), _capacity(__Capacity), _size(0), 
        _hash_seed(kHashSeeds[0]), _mask(__Capacity-1) {
        NAMEDICT_ALLOC()
    }

    NameDictImpl(const NameDictImpl& other) {
        memcpy(this, &other, sizeof(NameDictImpl));
        NAMEDICT_ALLOC()
        for(int i=0; i<_capacity; i++) _items[i] = other._items[i];
    }

    NameDictImpl& operator=(const NameDictImpl& other) {
        pool128.dealloc(_items);
        memcpy(this, &other, sizeof(NameDictImpl));
        NAMEDICT_ALLOC()
        for(int i=0; i<_capacity; i++) _items[i] = other._items[i];
        return *this;
    }
    
    ~NameDictImpl(){ pool128.dealloc(_items); }

    NameDictImpl(NameDictImpl&&) = delete;
    NameDictImpl& operator=(NameDictImpl&&) = delete;
    uint16_t size() const { return _size; }

    T operator[](StrName key) const {
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) throw std::out_of_range(fmt("NameDict key not found: ", key));
        return _items[i].second;
    }

    void set(StrName key, T val){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) {
            _size++;
            if(_size > _capacity*_load_factor){
                _rehash(true);
                HASH_PROBE(key, ok, i);
            }
            _items[i].first = key;
        }
        _items[i].second = val;
    }

    void _rehash(bool resize){
        Item* old_items = _items;
        uint16_t old_capacity = _capacity;
        if(resize){
            _capacity *= 2;
            _mask = _capacity - 1;
        }
        NAMEDICT_ALLOC()
        for(uint16_t i=0; i<old_capacity; i++){
            if(old_items[i].first.empty()) continue;
            bool ok; uint16_t j;
            HASH_PROBE(old_items[i].first, ok, j);
            if(ok) FATAL_ERROR();
            _items[j] = old_items[i];
        }
        pool128.dealloc(old_items);
    }

    void _try_perfect_rehash(){
        _hash_seed = find_perfect_hash_seed(_capacity, keys());
        _rehash(false); // do not resize
    }

    T try_get(StrName key) const{
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok){
            if constexpr(std::is_pointer_v<T>) return nullptr;
            else if constexpr(std::is_same_v<int, T>) return -1;
            else return Discarded();
        }
        return _items[i].second;
    }

    T* try_get_2(StrName key) {
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) return nullptr;
        return &_items[i].second;
    }

    bool try_set(StrName key, T val){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) return false;
        _items[i].second = val;
        return true;
    }

    bool contains(StrName key) const {
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        return ok;
    }

    void update(const NameDictImpl& other){
        for(uint16_t i=0; i<other._capacity; i++){
            auto& item = other._items[i];
            if(!item.first.empty()) set(item.first, item.second);
        }
    }

    void erase(StrName key){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) throw std::out_of_range(fmt("NameDict key not found: ", key));
        _items[i].first = StrName();
        _items[i].second = nullptr;
        _size--;
    }

    std::vector<Item> items() const {
        std::vector<Item> v;
        for(uint16_t i=0; i<_capacity; i++){
            if(_items[i].first.empty()) continue;
            v.push_back(_items[i]);
        }
        return v;
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
#undef HASH_PROBE
#undef NAMEDICT_ALLOC
#undef _hash
};

using NameDict = NameDictImpl<PyObject*>;
using NameDict_ = shared_ptr<NameDict>;
using NameDictInt = NameDictImpl<int>;

} // namespace pkpy


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
    std::string source;
    Str filename;
    std::vector<const char*> line_starts;
    CompileMode mode;

    std::pair<const char*,const char*> get_line(int lineno) const {
        if(lineno == -1) return {nullptr, nullptr};
        lineno -= 1;
        if(lineno < 0) lineno = 0;
        const char* _start = line_starts.at(lineno);
        const char* i = _start;
        while(*i != '\n' && *i != '\0') i++;
        return {_start, i};
    }

    SourceData(const Str& source, const Str& filename, CompileMode mode) {
        int index = 0;
        // Skip utf8 BOM if there is any.
        if (strncmp(source.begin(), "\xEF\xBB\xBF", 3) == 0) index += 3;
        // Remove all '\r'
        std::stringstream ss;
        while(index < source.length()){
            if(source[index] != '\r') ss << source[index];
            index++;
        }

        this->filename = filename;
        this->source = ss.str();
        line_starts.push_back(this->source.c_str());
        this->mode = mode;
    }

    Str snapshot(int lineno, const char* cursor=nullptr){
        std::stringstream ss;
        ss << "  " << "File \"" << filename << "\", line " << lineno << '\n';
        std::pair<const char*,const char*> pair = get_line(lineno);
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
        return ss.str();
    }
};

class Exception {
    using StackTrace = stack<Str>;
    StrName type;
    Str msg;
    StackTrace stacktrace;
public:
    Exception(StrName type, Str msg): type(type), msg(msg) {}
    bool match_type(StrName type) const { return this->type == type;}
    bool is_re = true;

    void st_push(Str snapshot){
        if(stacktrace.size() >= 8) return;
        stacktrace.push(snapshot);
    }

    Str summary() const {
        StackTrace st(stacktrace);
        std::stringstream ss;
        if(is_re) ss << "Traceback (most recent call last):\n";
        while(!st.empty()) { ss << st.top() << '\n'; st.pop(); }
        if (!msg.empty()) ss << type.sv() << ": " << msg;
        else ss << type.sv();
        return ss.str();
    }
};

}   // namespace pkpy


namespace pkpy{

typedef uint8_t TokenIndex;

constexpr const char* kTokens[] = {
    "is not", "not in", "yield from",
    "@eof", "@eol", "@sof",
    "@id", "@num", "@str", "@fstr",
    "@indent", "@dedent",
    /*****************************************/
    "+", "+=", "-", "-=",   // (INPLACE_OP - 1) can get '=' removed
    "*", "*=", "/", "/=", "//", "//=", "%", "%=",
    "&", "&=", "|", "|=", "^", "^=", 
    "<<", "<<=", ">>", ">>=",
    /*****************************************/
    ".", ",", ":", ";", "#", "(", ")", "[", "]", "{", "}",
    "**", "=", ">", "<", "...", "->", "?", "@", "==", "!=", ">=", "<=",
    "++", "--",
    /** SPEC_BEGIN **/
    "$goto", "$label",
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
  TokenValue value;

  Str str() const { return Str(start, length);}
  std::string_view sv() const { return std::string_view(start, length);}

  std::string info() const {
    std::stringstream ss;
    ss << line << ": " << TK_STR(type) << " '" << (
        sv()=="\n" ? "\\n" : sv()
    ) << "'";
    return ss.str();
  }
};

// https://docs.python.org/3/reference/expressions.html#operator-precedence
enum Precedence {
  PREC_NONE,
  PREC_TUPLE,         // ,
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
  PREC_UNARY,         // - not
  PREC_EXPONENT,      // **
  PREC_CALL,          // ()
  PREC_SUBSCRIPT,     // []
  PREC_ATTRIB,        // .index
  PREC_PRIMARY,
};

enum StringType { NORMAL_STRING, RAW_STRING, F_STRING };

struct Lexer {
    shared_ptr<SourceData> src;
    const char* token_start;
    const char* curr_char;
    int current_line = 1;
    std::vector<Token> nexts;
    stack<int> indents;
    int brackets_level = 0;
    bool used = false;

    char peekchar() const{ return *curr_char; }

    bool match_n_chars(int n, char c0){
        const char* c = curr_char;
        for(int i=0; i<n; i++){
            if(*c == '\0') return false;
            if(*c != c0) return false;
            c++;
        }
        for(int i=0; i<n; i++) eatchar_include_newline();
        return true;
    }

    bool match_string(const char* s){
        int s_len = strlen(s);
        bool ok = strncmp(curr_char, s, s_len) == 0;
        if(ok) for(int i=0; i<s_len; i++) eatchar_include_newline();
        return ok;
    }

    int eat_spaces(){
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

    bool eat_indentation(){
        if(brackets_level > 0) return true;
        int spaces = eat_spaces();
        if(peekchar() == '#') skip_line_comment();
        if(peekchar() == '\0' || peekchar() == '\n') return true;
        // https://docs.python.org/3/reference/lexical_analysis.html#indentation
        if(spaces > indents.top()){
            indents.push(spaces);
            nexts.push_back(Token{TK("@indent"), token_start, 0, current_line});
        } else if(spaces < indents.top()){
            while(spaces < indents.top()){
                indents.pop();
                nexts.push_back(Token{TK("@dedent"), token_start, 0, current_line});
            }
            if(spaces != indents.top()){
                return false;
            }
        }
        return true;
    }

    char eatchar() {
        char c = peekchar();
        if(c == '\n') throw std::runtime_error("eatchar() cannot consume a newline");
        curr_char++;
        return c;
    }

    char eatchar_include_newline() {
        char c = peekchar();
        curr_char++;
        if (c == '\n'){
            current_line++;
            src->line_starts.push_back(curr_char);
        }
        return c;
    }

    int eat_name() {
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

    void skip_line_comment() {
        char c;
        while ((c = peekchar()) != '\0') {
            if (c == '\n') return;
            eatchar();
        }
    }
    
    bool matchchar(char c) {
        if (peekchar() != c) return false;
        eatchar_include_newline();
        return true;
    }

    void add_token(TokenIndex type, TokenValue value={}) {
        switch(type){
            case TK("{"): case TK("["): case TK("("): brackets_level++; break;
            case TK(")"): case TK("]"): case TK("}"): brackets_level--; break;
        }
        auto token = Token{
            type,
            token_start,
            (int)(curr_char - token_start),
            current_line - ((type == TK("@eol")) ? 1 : 0),
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

    void add_token_2(char c, TokenIndex one, TokenIndex two) {
        if (matchchar(c)) add_token(two);
        else add_token(one);
    }

    Str eat_string_until(char quote, bool raw) {
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
                    default: SyntaxError("invalid escape char");
                }
            } else {
                buff.push_back(c);
            }
        }
        return Str(buff.data(), buff.size());
    }

    void eat_string(char quote, StringType type) {
        Str s = eat_string_until(quote, type == RAW_STRING);
        if(type == F_STRING){
            add_token(TK("@fstr"), s);
        }else{
            add_token(TK("@str"), s);
        }
    }

    void eat_number() {
        static const std::regex pattern("^(0x)?[0-9a-fA-F]+(\\.[0-9]+)?");
        std::smatch m;

        const char* i = token_start;
        while(*i != '\n' && *i != '\0') i++;
        std::string s = std::string(token_start, i);

        try{
            if (std::regex_search(s, m, pattern)) {
                // here is m.length()-1, since the first char was eaten by lex_token()
                for(int j=0; j<m.length()-1; j++) eatchar();

                int base = 10;
                size_t size;
                if (m[1].matched) base = 16;
                if (m[2].matched) {
                    if(base == 16) SyntaxError("hex literal should not contain a dot");
                    add_token(TK("@num"), Number::stof(m[0], &size));
                } else {
                    add_token(TK("@num"), Number::stoi(m[0], &size, base));
                }
                if (size != m.length()) FATAL_ERROR();
            }
        }catch(std::exception& _){
            SyntaxError("invalid number literal");
        } 
    }

    bool lex_one_token() {
        while (peekchar() != '\0') {
            token_start = curr_char;
            char c = eatchar_include_newline();
            switch (c) {
                case '\'': case '"': eat_string(c, NORMAL_STRING); return true;
                case '#': skip_line_comment(); break;
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
                case '$': {
                    for(int i=TK("$goto"); i<=TK("$label"); i++){
                        // +1 to skip the '$'
                        if(match_string(TK_STR(i) + 1)){
                            add_token((TokenIndex)i);
                            return true;
                        }
                    }
                    SyntaxError("invalid special token");
                }
                case '%': add_token_2('=', TK("%"), TK("%=")); return true;
                case '&': add_token_2('=', TK("&"), TK("&=")); return true;
                case '|': add_token_2('=', TK("|"), TK("|=")); return true;
                case '^': add_token_2('=', TK("^"), TK("^=")); return true;
                case '?': add_token(TK("?")); return true;
                case '.': {
                    if(matchchar('.')) {
                        if(matchchar('.')) {
                            add_token(TK("..."));
                        } else {
                            SyntaxError("invalid token '..'");
                        }
                    } else {
                        add_token(TK("."));
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
                    }
                    if (c >= '0' && c <= '9') {
                        eat_number();
                        return true;
                    }
                    switch (eat_name())
                    {
                        case 0: break;
                        case 1: SyntaxError("invalid char: " + std::string(1, c));
                        case 2: SyntaxError("invalid utf8 sequence: " + std::string(1, c));
                        case 3: SyntaxError("@id contains invalid char"); break;
                        case 4: SyntaxError("invalid JSON token"); break;
                        default: FATAL_ERROR();
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

    /***** Error Reporter *****/
    void throw_err(Str type, Str msg){
        int lineno = current_line;
        const char* cursor = curr_char;
        if(peekchar() == '\n'){
            lineno--;
            cursor--;
        }
        throw_err(type, msg, lineno, cursor);
    }

    void throw_err(Str type, Str msg, int lineno, const char* cursor){
        auto e = Exception(type, msg);
        e.st_push(src->snapshot(lineno, cursor));
        throw e;
    }
    void SyntaxError(Str msg){ throw_err("SyntaxError", msg); }
    void SyntaxError(){ throw_err("SyntaxError", "invalid syntax"); }
    void IndentationError(Str msg){ throw_err("IndentationError", msg); }

    Lexer(shared_ptr<SourceData> src) {
        this->src = src;
        this->token_start = src->source.c_str();
        this->curr_char = src->source.c_str();
        this->nexts.push_back(Token{TK("@sof"), token_start, 0, current_line});
        this->indents.push(0);
    }

    std::vector<Token> run() {
        if(used) FATAL_ERROR();
        used = true;
        while (lex_one_token());
        return std::move(nexts);
    }
};

} // namespace pkpy



namespace pkpy {
    
struct CodeObject;
struct Frame;
struct Function;
class VM;

#if PK_ENABLE_STD_FUNCTION
using NativeFuncC = std::function<PyObject*(VM*, ArgsView)>;
#else
typedef PyObject* (*NativeFuncC)(VM*, ArgsView);
#endif

typedef int (*LuaStyleFuncC)(VM*);

struct NativeFunc {
    NativeFuncC f;
    int argc;       // DONOT include self
    bool method;

    // this is designed for lua style C bindings
    // access it via `_CAST(NativeFunc&, args[-2])._lua_f`
    // (-2) or (-1) depends on the calling convention
    LuaStyleFuncC _lua_f;

    using UserData = char[32];
    UserData _userdata;
    bool _has_userdata;

    template <typename T>
    void set_userdata(T data) {
        static_assert(std::is_trivially_copyable_v<T>);
        static_assert(sizeof(T) <= sizeof(UserData));
        if(_has_userdata) throw std::runtime_error("userdata already set");
        _has_userdata = true;
        memcpy(_userdata, &data, sizeof(T));
    }

    template <typename T>
    T get_userdata() const {
        static_assert(std::is_trivially_copyable_v<T>);
        static_assert(sizeof(T) <= sizeof(UserData));
#if DEBUG_EXTRA_CHECK
        if(!_has_userdata) throw std::runtime_error("userdata not set");
#endif
        return reinterpret_cast<const T&>(_userdata);
    }
    
    NativeFunc(NativeFuncC f, int argc, bool method) : f(f), argc(argc), method(method), _has_userdata(false) {}
    PyObject* operator()(VM* vm, ArgsView args) const;
};

typedef shared_ptr<CodeObject> CodeObject_;

struct FuncDecl {
    struct KwArg {
        int key;                // index in co->varnames
        PyObject* value;        // default value
    };
    CodeObject_ code;           // code object of this function
    pod_vector<int> args;       // indices in co->varnames
    pod_vector<KwArg> kwargs;   // indices in co->varnames
    int starred_arg = -1;       // index in co->varnames, -1 if no *arg
    int starred_kwarg = -1;     // index in co->varnames, -1 if no **kwarg
    bool nested = false;        // whether this function is nested
    void _gc_mark() const;
};

using FuncDecl_ = shared_ptr<FuncDecl>;

struct Function{
    FuncDecl_ decl;
    bool is_simple;
    int argc;   // cached argc
    PyObject* _module;
    NameDict_ _closure;
};

struct BoundMethod {
    PyObject* self;
    PyObject* func;
    BoundMethod(PyObject* self, PyObject* func) : self(self), func(func) {}
    
    bool operator==(const BoundMethod& rhs) const noexcept {
        return self == rhs.self && func == rhs.func;
    }
    bool operator!=(const BoundMethod& rhs) const noexcept {
        return self != rhs.self || func != rhs.func;
    }
};

struct Property{
    PyObject* getter;
    PyObject* setter;
    Property(PyObject* getter, PyObject* setter) : getter(getter), setter(setter) {}
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
    std::vector<char> _data;
    bool _ok;

    int size() const noexcept { return _data.size(); }
    int operator[](int i) const noexcept { return (int)(uint8_t)_data[i]; }
    const char* data() const noexcept { return _data.data(); }

    bool operator==(const Bytes& rhs) const noexcept {
        return _data == rhs._data;
    }
    bool operator!=(const Bytes& rhs) const noexcept {
        return _data != rhs._data;
    }

    std::string str() const noexcept { return std::string(_data.begin(), _data.end()); }

    Bytes() : _data(), _ok(false) {}
    Bytes(std::vector<char>&& data) : _data(std::move(data)), _ok(true) {}
    Bytes(const std::string& data) : _data(data.begin(), data.end()), _ok(true) {}
    operator bool() const noexcept { return _ok; }
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
    virtual void _obj_gc_mark() = 0;

    PyObject(Type type) : type(type), _attr(nullptr) {}

    virtual ~PyObject() {
        if(_attr == nullptr) return;
        _attr->~NameDict();
        pool64.dealloc(_attr);
    }

    void enable_instance_dict(float lf=kInstAttrLoadFactor) noexcept {
        _attr = new(pool64.alloc<NameDict>()) NameDict(lf);
    }
};

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
    Py_(Type type, const T& value) : PyObject(type), _value(value) {}
    Py_(Type type, T&& value) : PyObject(type), _value(std::move(value)) {}
};

struct MappingProxy{
    PyObject* obj;
    MappingProxy(PyObject* obj) : obj(obj) {}
    NameDict& attr() noexcept { return obj->attr(); }
};

#define OBJ_GET(T, obj) (((Py_<T>*)(obj))->_value)

#define OBJ_MARK(obj) \
    if(!is_tagged(obj) && !(obj)->gc.marked) {                      \
        (obj)->gc.marked = true;                                    \
        (obj)->_obj_gc_mark();                                      \
        if((obj)->is_attr_valid()) gc_mark_namedict((obj)->attr()); \
    }

inline void gc_mark_namedict(NameDict& t){
    if(t.size() == 0) return;
    for(uint16_t i=0; i<t._capacity; i++){
        if(t._items[i].first.empty()) continue;
        OBJ_MARK(t._items[i].second);
    }
}

Str obj_type_name(VM* vm, Type type);

#if DEBUG_NO_BUILTIN_MODULES
#define OBJ_NAME(obj) Str("<?>")
#else
DEF_SNAME(__name__);
#define OBJ_NAME(obj) OBJ_GET(Str, vm->getattr(obj, __name__))
#endif

const int kTpIntIndex = 2;
const int kTpFloatIndex = 3;

inline bool is_type(PyObject* obj, Type type) {
#if DEBUG_EXTRA_CHECK
    if(obj == nullptr) throw std::runtime_error("is_type() called with nullptr");
    if(is_special(obj)) throw std::runtime_error("is_type() called with special object");
#endif
    switch(type.index){
        case kTpIntIndex: return is_int(obj);
        case kTpFloatIndex: return is_float(obj);
        default: return !is_tagged(obj) && obj->type == type;
    }
}

inline bool is_non_tagged_type(PyObject* obj, Type type) {
#if DEBUG_EXTRA_CHECK
    if(obj == nullptr) throw std::runtime_error("is_non_tagged_type() called with nullptr");
    if(is_special(obj)) throw std::runtime_error("is_non_tagged_type() called with special object");
#endif
    return !is_tagged(obj) && obj->type == type;
}

union BitsCvt {
    i64 _int;
    f64 _float;
    BitsCvt(i64 val) : _int(val) {}
    BitsCvt(f64 val) : _float(val) {}
};

template <typename, typename=void> struct is_py_class : std::false_type {};
template <typename T> struct is_py_class<T, std::void_t<decltype(T::_type)>> : std::true_type {};

template<typename T> T to_void_p(VM*, PyObject*);
template<typename T> T to_c99_struct(VM*, PyObject*);

template<typename __T>
__T py_cast(VM* vm, PyObject* obj) {
    using T = std::decay_t<__T>;
    if constexpr(std::is_enum_v<T>){
        return (__T)py_cast<i64>(vm, obj);
    }else if constexpr(std::is_pointer_v<T>){
        return to_void_p<T>(vm, obj);
    }else if constexpr(is_py_class<T>::value){
        T::_check_type(vm, obj);
        return OBJ_GET(T, obj);
    }else if constexpr(std::is_pod_v<T>){
        return to_c99_struct<T>(vm, obj);
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
        return OBJ_GET(T, obj);
    }else if constexpr(std::is_pod_v<T>){
        return to_c99_struct<T>(vm, obj);
    }else {
        return Discarded();
    }
}

#define VAR(x) py_var(vm, x)
#define CAST(T, x) py_cast<T>(vm, x)
#define _CAST(T, x) _py_cast<T>(vm, x)

#define CAST_F(x) vm->num_to_float(x)

/*****************************************************************/
template<>
struct Py_<List> final: PyObject {
    List _value;
    Py_(Type type, List&& val): PyObject(type), _value(std::move(val)) {}
    Py_(Type type, const List& val): PyObject(type), _value(val) {}

    void _obj_gc_mark() override {
        for(PyObject* obj: _value) OBJ_MARK(obj);
    }
};

template<>
struct Py_<Tuple> final: PyObject {
    Tuple _value;
    Py_(Type type, Tuple&& val): PyObject(type), _value(std::move(val)) {}
    Py_(Type type, const Tuple& val): PyObject(type), _value(val) {}

    void _obj_gc_mark() override {
        for(PyObject* obj: _value) OBJ_MARK(obj);
    }
};

template<>
struct Py_<MappingProxy> final: PyObject {
    MappingProxy _value;
    Py_(Type type, MappingProxy val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        OBJ_MARK(_value.obj);
    }
};

template<>
struct Py_<BoundMethod> final: PyObject {
    BoundMethod _value;
    Py_(Type type, BoundMethod val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        OBJ_MARK(_value.self);
        OBJ_MARK(_value.func);
    }
};

template<>
struct Py_<StarWrapper> final: PyObject {
    StarWrapper _value;
    Py_(Type type, StarWrapper val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        OBJ_MARK(_value.obj);
    }
};

template<>
struct Py_<Property> final: PyObject {
    Property _value;
    Py_(Type type, Property val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        OBJ_MARK(_value.getter);
        OBJ_MARK(_value.setter);
    }
};

template<>
struct Py_<Slice> final: PyObject {
    Slice _value;
    Py_(Type type, Slice val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        OBJ_MARK(_value.start);
        OBJ_MARK(_value.stop);
        OBJ_MARK(_value.step);
    }
};

template<>
struct Py_<Function> final: PyObject {
    Function _value;
    Py_(Type type, Function val): PyObject(type), _value(val) {
        enable_instance_dict();
    }
    void _obj_gc_mark() override {
        _value.decl->_gc_mark();
        if(_value._module != nullptr) OBJ_MARK(_value._module);
        if(_value._closure != nullptr) gc_mark_namedict(*_value._closure);
    }
};

template<>
struct Py_<NativeFunc> final: PyObject {
    NativeFunc _value;
    Py_(Type type, NativeFunc val): PyObject(type), _value(val) {
        enable_instance_dict();
    }
    void _obj_gc_mark() override {}
};

template<>
struct Py_<Super> final: PyObject {
    Super _value;
    Py_(Type type, Super val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        OBJ_MARK(_value.first);
    }
};

template<>
struct Py_<DummyInstance> final: PyObject {
    Py_(Type type, DummyInstance val): PyObject(type) {
        enable_instance_dict();
    }
    void _obj_gc_mark() override {}
};

template<>
struct Py_<Type> final: PyObject {
    Type _value;
    Py_(Type type, Type val): PyObject(type), _value(val) {
        enable_instance_dict(kTypeAttrLoadFactor);
    }
    void _obj_gc_mark() override {}
};

template<>
struct Py_<DummyModule> final: PyObject {
    Py_(Type type, DummyModule val): PyObject(type) {
        enable_instance_dict(kTypeAttrLoadFactor);
    }
    void _obj_gc_mark() override {}
};

template<typename T>
inline T lambda_get_userdata(PyObject** p){
    if(p[-1] != PY_NULL) return OBJ_GET(NativeFunc, p[-1]).get_userdata<T>();
    else return OBJ_GET(NativeFunc, p[-2]).get_userdata<T>();
}



}   // namespace pkpy


namespace pkpy{

struct Dict{
    using Item = std::pair<PyObject*, PyObject*>;
    static constexpr int __Capacity = 8;
    static constexpr float __LoadFactor = 0.67f;
    static_assert(sizeof(Item) * __Capacity <= 128);

    VM* vm;
    int _capacity;
    int _mask;
    int _size;
    int _critical_size;
    Item* _items;
    
    Dict(VM* vm): vm(vm), _capacity(__Capacity),
            _mask(__Capacity-1),
            _size(0), _critical_size(__Capacity*__LoadFactor+0.5f){
        _items = (Item*)pool128.alloc(_capacity * sizeof(Item));
        memset(_items, 0, _capacity * sizeof(Item));
    }

    int size() const { return _size; }

    Dict(Dict&& other){
        vm = other.vm;
        _capacity = other._capacity;
        _mask = other._mask;
        _size = other._size;
        _critical_size = other._critical_size;
        _items = other._items;
        other._items = nullptr;
    }

    Dict(const Dict& other){
        vm = other.vm;
        _capacity = other._capacity;
        _mask = other._mask;
        _size = other._size;
        _critical_size = other._critical_size;
        _items = (Item*)pool128.alloc(_capacity * sizeof(Item));
        memcpy(_items, other._items, _capacity * sizeof(Item));
    }

    Dict& operator=(const Dict&) = delete;
    Dict& operator=(Dict&&) = delete;

    void _probe(PyObject* key, bool& ok, int& i) const;

    void set(PyObject* key, PyObject* val){
        bool ok; int i;
        _probe(key, ok, i);
        if(!ok) {
            _size++;
            if(_size > _critical_size){
                _rehash();
                _probe(key, ok, i);
            }
            _items[i].first = key;
        }
        _items[i].second = val;
    }

    void _rehash(){
        Item* old_items = _items;
        int old_capacity = _capacity;
        _capacity *= 2;
        _mask = _capacity - 1;
        _critical_size = _capacity * __LoadFactor + 0.5f;
        _items = (Item*)pool128.alloc(_capacity * sizeof(Item));
        memset(_items, 0, _capacity * sizeof(Item));
        for(int i=0; i<old_capacity; i++){
            if(old_items[i].first == nullptr) continue;
            bool ok; int j;
            _probe(old_items[i].first, ok, j);
            if(ok) FATAL_ERROR();
            _items[j] = old_items[i];
        }
        pool128.dealloc(old_items);
    }

    PyObject* try_get(PyObject* key) const{
        bool ok; int i;
        _probe(key, ok, i);
        if(!ok) return nullptr;
        return _items[i].second;
    }

    bool contains(PyObject* key) const{
        bool ok; int i;
        _probe(key, ok, i);
        return ok;
    }

    void erase(PyObject* key){
        bool ok; int i;
        _probe(key, ok, i);
        if(!ok) return;
        _items[i].first = nullptr;
        _items[i].second = nullptr;
        _size--;
    }

    void update(const Dict& other){
        for(int i=0; i<other._capacity; i++){
            if(other._items[i].first == nullptr) continue;
            set(other._items[i].first, other._items[i].second);
        }
    }

    std::vector<Item> items() const {
        std::vector<Item> v;
        for(int i=0; i<_capacity; i++){
            if(_items[i].first == nullptr) continue;
            v.push_back(_items[i]);
        }
        return v;
    }

    template<typename __Func>
    void apply(__Func f) const {
        for(int i=0; i<_capacity; i++){
            if(_items[i].first == nullptr) continue;
            f(_items[i].first, _items[i].second);
        }
    }

    void clear(){
        memset(_items, 0, _capacity * sizeof(Item));
        _size = 0;
    }

    ~Dict(){ if(_items!=nullptr) pool128.dealloc(_items); }

    void _gc_mark() const{
        for(int i=0; i<_capacity; i++){
            if(_items[i].first == nullptr) continue;
            OBJ_MARK(_items[i].first);
            OBJ_MARK(_items[i].second);
        }
    }
};

} // namespace pkpy


namespace pkpy{

enum NameScope { NAME_LOCAL, NAME_GLOBAL, NAME_GLOBAL_UNKNOWN };

enum Opcode {
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
OPCODE(POP_JUMP_IF_FALSE)
OPCODE(JUMP_IF_TRUE_OR_POP)
OPCODE(JUMP_IF_FALSE_OR_POP)
OPCODE(SHORTCUT_IF_FALSE_OR_POP)
OPCODE(LOOP_CONTINUE)
OPCODE(LOOP_BREAK)
OPCODE(GOTO)
/**************************/
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
/**************************/
OPCODE(GET_ITER)
OPCODE(FOR_ITER)
/**************************/
OPCODE(IMPORT_NAME)
OPCODE(IMPORT_NAME_REL)
OPCODE(IMPORT_STAR)
/**************************/
OPCODE(UNPACK_SEQUENCE)
OPCODE(UNPACK_EX)
/**************************/
OPCODE(BEGIN_CLASS)
OPCODE(END_CLASS)
OPCODE(STORE_CLASS_ATTR)
/**************************/
OPCODE(WITH_ENTER)
OPCODE(WITH_EXIT)
/**************************/
OPCODE(ASSERT)
OPCODE(EXCEPTION_MATCH)
OPCODE(RAISE)
OPCODE(RE_RAISE)
OPCODE(POP_EXCEPTION)
/**************************/
OPCODE(SETUP_DOCSTRING)
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
OPCODE(POP_JUMP_IF_FALSE)
OPCODE(JUMP_IF_TRUE_OR_POP)
OPCODE(JUMP_IF_FALSE_OR_POP)
OPCODE(SHORTCUT_IF_FALSE_OR_POP)
OPCODE(LOOP_CONTINUE)
OPCODE(LOOP_BREAK)
OPCODE(GOTO)
/**************************/
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
/**************************/
OPCODE(GET_ITER)
OPCODE(FOR_ITER)
/**************************/
OPCODE(IMPORT_NAME)
OPCODE(IMPORT_NAME_REL)
OPCODE(IMPORT_STAR)
/**************************/
OPCODE(UNPACK_SEQUENCE)
OPCODE(UNPACK_EX)
/**************************/
OPCODE(BEGIN_CLASS)
OPCODE(END_CLASS)
OPCODE(STORE_CLASS_ATTR)
/**************************/
OPCODE(WITH_ENTER)
OPCODE(WITH_EXIT)
/**************************/
OPCODE(ASSERT)
OPCODE(EXCEPTION_MATCH)
OPCODE(RAISE)
OPCODE(RE_RAISE)
OPCODE(POP_EXCEPTION)
/**************************/
OPCODE(SETUP_DOCSTRING)
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
    uint16_t op;
    uint16_t block;
    int arg;
};

enum CodeBlockType {
    NO_BLOCK,
    FOR_LOOP,
    WHILE_LOOP,
    CONTEXT_MANAGER,
    TRY_EXCEPT,
};

#define BC_NOARG        -1
#define BC_KEEPLINE     -1

struct CodeBlock {
    CodeBlockType type;
    int parent;         // parent index in blocks
    int for_loop_depth; // this is used for exception handling
    int start;          // start index of this block in codes, inclusive
    int end;            // end index of this block in codes, exclusive

    CodeBlock(CodeBlockType type, int parent, int for_loop_depth, int start):
        type(type), parent(parent), for_loop_depth(for_loop_depth), start(start), end(-1) {}
};

struct CodeObject {
    shared_ptr<SourceData> src;
    Str name;
    bool is_generator = false;

    CodeObject(shared_ptr<SourceData> src, const Str& name):
        src(src), name(name) {}

    std::vector<Bytecode> codes;
    std::vector<int> lines; // line number for each bytecode
    List consts;
    std::vector<StrName> varnames;      // local variables
    NameDictInt varnames_inv;
    std::set<Str> global_names;
    std::vector<CodeBlock> blocks = { CodeBlock(NO_BLOCK, -1, 0, 0) };
    NameDictInt labels;
    std::vector<FuncDecl_> func_decls;

    void _gc_mark() const {
        for(PyObject* v : consts) OBJ_MARK(v);
        for(auto& decl: func_decls) decl->_gc_mark();
    }
};

} // namespace pkpy


namespace pkpy{

// weak reference fast locals
struct FastLocals{
    // this is a weak reference
    const NameDictInt* varnames_inv;
    PyObject** a;

    int size() const{
        return varnames_inv->size();
    }

    PyObject*& operator[](int i){ return a[i]; }
    PyObject* operator[](int i) const { return a[i]; }

    FastLocals(const CodeObject* co, PyObject** a): varnames_inv(&co->varnames_inv), a(a) {}
    FastLocals(const FastLocals& other): varnames_inv(other.varnames_inv), a(other.a) {}

    PyObject* try_get(StrName name){
        int index = varnames_inv->try_get(name);
        if(index == -1) return nullptr;
        return a[index];
    }

    bool contains(StrName name){
        return varnames_inv->contains(name);
    }

    void erase(StrName name){
        int index = varnames_inv->try_get(name);
        if(index == -1) FATAL_ERROR();
        a[index] = nullptr;
    }

    bool try_set(StrName name, PyObject* value){
        int index = varnames_inv->try_get(name);
        if(index == -1) return false;
        a[index] = value;
        return true;
    }

    NameDict_ to_namedict(){
        NameDict_ dict = make_sp<NameDict>();
        varnames_inv->apply([&](StrName name, int index){
            PyObject* value = a[index];
            if(value != PY_NULL) dict->set(name, value);
        });
        return dict;
    }
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
#if DEBUG_EXTRA_CHECK
        if(sp < _begin || sp > _begin + MAX_SIZE) FATAL_ERROR();
#endif
        _sp = sp;
    }
    void clear() { _sp = _begin; }
    bool is_overflow() const { return _sp >= _max_end; }
    
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
    
    PyObject* f_closure_try_get(StrName name){
        if(_callable == nullptr) return nullptr;
        Function& fn = OBJ_GET(Function, _callable);
        if(fn._closure == nullptr) return nullptr;
        return fn._closure->try_get(name);
    }

    Frame(ValueStack* _s, PyObject** p0, const CodeObject* co, PyObject* _module, PyObject* _callable)
            : _s(_s), _sp_base(p0), co(co), _module(_module), _callable(_callable), _locals(co, p0) { }

    Frame(ValueStack* _s, PyObject** p0, const CodeObject* co, PyObject* _module, PyObject* _callable, FastLocals _locals)
            : _s(_s), _sp_base(p0), co(co), _module(_module), _callable(_callable), _locals(_locals) { }

    Frame(ValueStack* _s, PyObject** p0, const CodeObject_& co, PyObject* _module)
            : _s(_s), _sp_base(p0), co(co.get()), _module(_module), _callable(nullptr), _locals(co.get(), p0) {}

    Bytecode next_bytecode() {
        _ip = _next_ip++;
#if DEBUG_EXTRA_CHECK
        if(_ip >= co->codes.size()) FATAL_ERROR();
#endif
        return co->codes[_ip];
    }

    Str snapshot(){
        int line = co->lines[_ip];
        return co->src->snapshot(line);
    }

    PyObject** actual_sp_base() const { return _locals.a; }
    int stack_size() const { return _s->_sp - actual_sp_base(); }
    ArgsView stack_view() const { return ArgsView(actual_sp_base(), _s->_sp); }

    void jump_abs(int i){ _next_ip = i; }
    // void jump_rel(int i){ _next_ip += i; }

    bool jump_to_exception_handler(){
        // try to find a parent try block
        int block = co->codes[_ip].block;
        while(block >= 0){
            if(co->blocks[block].type == TRY_EXCEPT) break;
            block = co->blocks[block].parent;
        }
        if(block < 0) return false;
        PyObject* obj = _s->popx();         // pop exception object
        // get the stack size of the try block (depth of for loops)
        int _stack_size = co->blocks[block].for_loop_depth;
        if(stack_size() < _stack_size) throw std::runtime_error("invalid stack size");
        _s->reset(actual_sp_base() + _locals.size() + _stack_size);          // rollback the stack   
        _s->push(obj);                                      // push exception object
        _next_ip = co->blocks[block].end;
        return true;
    }

    int _exit_block(int i){
        if(co->blocks[i].type == FOR_LOOP) _s->pop();
        return co->blocks[i].parent;
    }

    void jump_abs_break(int target){
        const Bytecode& prev = co->codes[_ip];
        int i = prev.block;
        _next_ip = target;
        if(_next_ip >= co->codes.size()){
            while(i>=0) i = _exit_block(i);
        }else{
            const Bytecode& next = co->codes[target];
            while(i>=0 && i!=next.block) i = _exit_block(i);
            if(i!=next.block) throw std::runtime_error("invalid jump");
        }
    }

    void _gc_mark() const {
        OBJ_MARK(_module);
        co->_gc_mark();
    }
};

}; // namespace pkpy


namespace pkpy {
struct ManagedHeap{
    std::vector<PyObject*> _no_gc;
    std::vector<PyObject*> gen;
    VM* vm;
    void (*_gc_on_delete)(VM*, PyObject*) = nullptr;
    void (*_gc_marker_ex)(VM*) = nullptr;

    ManagedHeap(VM* vm): vm(vm) {}
    
    static const int kMinGCThreshold = 3072;
    int gc_threshold = kMinGCThreshold;
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

    template<typename T>
    PyObject* gcnew(Type type, T&& val){
        using __T = Py_<std::decay_t<T>>;
        PyObject* obj = new(pool64.alloc<__T>()) __T(type, std::forward<T>(val));
        gen.push_back(obj);
        gc_counter++;
        return obj;
    }

    template<typename T>
    PyObject* _new(Type type, T&& val){
        using __T = Py_<std::decay_t<T>>;
        PyObject* obj = new(pool64.alloc<__T>()) __T(type, std::forward<T>(val));
        obj->gc.enabled = false;
        _no_gc.push_back(obj);
        return obj;
    }

#if DEBUG_GC_STATS
    inline static std::map<Type, int> deleted;
#endif

    int sweep(){
        std::vector<PyObject*> alive;
        for(PyObject* obj: gen){
            if(obj->gc.marked){
                obj->gc.marked = false;
                alive.push_back(obj);
            }else{
#if DEBUG_GC_STATS
                deleted[obj->type] += 1;
#endif
                if(_gc_on_delete) _gc_on_delete(vm, obj);
                obj->~PyObject();
                pool64.dealloc(obj);
            }
        }

        // clear _no_gc marked flag
        for(PyObject* obj: _no_gc) obj->gc.marked = false;

        int freed = gen.size() - alive.size();
        // std::cout << "GC: " << alive.size() << "/" << gen.size() << " (" << freed << " freed)" << std::endl;
        gen.clear();
        gen.swap(alive);
        return freed;
    }

    void _auto_collect(){
#if !DEBUG_NO_AUTO_GC
        if(_gc_lock_counter > 0) return;
        if(gc_counter < gc_threshold) return;
        gc_counter = 0;
        collect();
        gc_threshold = gen.size() * 2;
        if(gc_threshold < kMinGCThreshold) gc_threshold = kMinGCThreshold;
#endif
    }

    int collect(){
        if(_gc_lock_counter > 0) FATAL_ERROR();
        mark();
        int freed = sweep();
        return freed;
    }

    void mark();

    ~ManagedHeap(){
        for(PyObject* obj: _no_gc) { obj->~PyObject(); pool64.dealloc(obj); }
        for(PyObject* obj: gen) { obj->~PyObject(); pool64.dealloc(obj); }
#if DEBUG_GC_STATS
        for(auto& [type, count]: deleted){
            std::cout << "GC: " << obj_type_name(vm, type) << "=" << count << std::endl;
        }
#endif
    }
};

inline void FuncDecl::_gc_mark() const{
    code->_gc_mark();
    for(int i=0; i<kwargs.size(); i++) OBJ_MARK(kwargs[i].value);
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
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> inline ctype _py_cast<ctype>(VM* vm, PyObject* obj) {    \
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> inline ctype& py_cast<ctype&>(VM* vm, PyObject* obj) {   \
        vm->check_non_tagged_type(obj, vm->ptype);                      \
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    template<> inline ctype& _py_cast<ctype&>(VM* vm, PyObject* obj) {  \
        return OBJ_GET(ctype, obj);                                     \
    }                                                                   \
    inline PyObject* py_var(VM* vm, const ctype& value) { return vm->heap.gcnew(vm->ptype, value);}     \
    inline PyObject* py_var(VM* vm, ctype&& value) { return vm->heap.gcnew(vm->ptype, std::move(value));}


struct PyTypeInfo{
    PyObject* obj;
    Type base;
    Str name;
    bool subclass_enabled;

    // cached special methods
    // unary operators
    PyObject* (*m__repr__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__str__)(VM* vm, PyObject*) = nullptr;
    i64 (*m__hash__)(VM* vm, PyObject*) = nullptr;
    i64 (*m__len__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__iter__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__next__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__json__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__neg__)(VM* vm, PyObject*) = nullptr;
    PyObject* (*m__bool__)(VM* vm, PyObject*) = nullptr;

    bool (*m__eq__)(VM* vm, PyObject*, PyObject*) = nullptr;
    bool (*m__lt__)(VM* vm, PyObject*, PyObject*) = nullptr;
    bool (*m__le__)(VM* vm, PyObject*, PyObject*) = nullptr;
    bool (*m__gt__)(VM* vm, PyObject*, PyObject*) = nullptr;
    bool (*m__ge__)(VM* vm, PyObject*, PyObject*) = nullptr;
    bool (*m__contains__)(VM* vm, PyObject*, PyObject*) = nullptr;

    // binary operators
    PyObject* (*m__add__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__sub__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__mul__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__truediv__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__floordiv__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__mod__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__pow__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__matmul__)(VM* vm, PyObject*, PyObject*) = nullptr;

    PyObject* (*m__lshift__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__rshift__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__and__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__or__)(VM* vm, PyObject*, PyObject*) = nullptr;
    PyObject* (*m__xor__)(VM* vm, PyObject*, PyObject*) = nullptr;

    // indexer
    PyObject* (*m__getitem__)(VM* vm, PyObject*, PyObject*) = nullptr;
    void (*m__setitem__)(VM* vm, PyObject*, PyObject*, PyObject*) = nullptr;
    void (*m__delitem__)(VM* vm, PyObject*, PyObject*) = nullptr;
};

struct FrameId{
    std::vector<pkpy::Frame>* data;
    int index;
    FrameId(std::vector<pkpy::Frame>* data, int index) : data(data), index(index) {}
    Frame* operator->() const { return &data->operator[](index); }
};

typedef void(*PrintFunc)(VM*, const Str&);

class VM {
    VM* vm;     // self reference for simplify code
public:
    ManagedHeap heap;
    ValueStack s_data;
    stack< Frame > callstack;
    std::vector<PyTypeInfo> _all_types;
    
    NameDict _modules;                                 // loaded modules
    std::map<StrName, Str> _lazy_modules;              // lazy loaded modules

    PyObject* None;
    PyObject* True;
    PyObject* False;
    PyObject* Ellipsis;
    PyObject* builtins;         // builtins module
    PyObject* StopIteration;
    PyObject* _main;            // __main__ module

    PyObject* _last_exception;

    PrintFunc _stdout;
    PrintFunc _stderr;
    Bytes (*_import_handler)(const Str& name);

    // for quick access
    Type tp_object, tp_type, tp_int, tp_float, tp_bool, tp_str;
    Type tp_list, tp_tuple;
    Type tp_function, tp_native_func, tp_bound_method;
    Type tp_slice, tp_range, tp_module;
    Type tp_super, tp_exception, tp_bytes, tp_mappingproxy;
    Type tp_dict, tp_property, tp_star_wrapper;

    const bool enable_os;

    VM(bool enable_os=true) : heap(this), enable_os(enable_os) {
        this->vm = this;
        _stdout = [](VM* vm, const Str& s) { std::cout << s; };
        _stderr = [](VM* vm, const Str& s) { std::cerr << s; };
        callstack.reserve(8);
        _main = nullptr;
        _last_exception = nullptr;
        _import_handler = [](const Str& name) { return Bytes(); };
        init_builtin_types();
    }

    FrameId top_frame() {
#if DEBUG_EXTRA_CHECK
        if(callstack.empty()) FATAL_ERROR();
#endif
        return FrameId(&callstack.data(), callstack.size()-1);
    }

    PyObject* py_str(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__str__) return ti->m__str__(this, obj);
        PyObject* self;
        PyObject* f = get_unbound_method(obj, __str__, &self, false);
        if(self != PY_NULL) return call_method(self, f);
        return py_repr(obj);
    }

    PyObject* py_repr(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__repr__) return ti->m__repr__(this, obj);
        return call_method(obj, __repr__);
    }

    PyObject* py_json(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__json__) return ti->m__json__(this, obj);
        return call_method(obj, __json__);
    }

    PyObject* py_iter(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__iter__) return ti->m__iter__(this, obj);
        PyObject* self;
        PyObject* iter_f = get_unbound_method(obj, __iter__, &self, false);
        if(self != PY_NULL) return call_method(self, iter_f);
        TypeError(OBJ_NAME(_t(obj)).escape() + " object is not iterable");
        return nullptr;
    }

    PyObject* find_name_in_mro(PyObject* cls, StrName name){
        PyObject* val;
        do{
            val = cls->attr().try_get(name);
            if(val != nullptr) return val;
            Type base = _all_types[OBJ_GET(Type, cls)].base;
            if(base.index == -1) break;
            cls = _all_types[base].obj;
        }while(true);
        return nullptr;
    }

    bool isinstance(PyObject* obj, Type cls_t){
        Type obj_t = OBJ_GET(Type, _t(obj));
        do{
            if(obj_t == cls_t) return true;
            Type base = _all_types[obj_t].base;
            if(base.index == -1) break;
            obj_t = base;
        }while(true);
        return false;
    }

    PyObject* exec(Str source, Str filename, CompileMode mode, PyObject* _module=nullptr){
        if(_module == nullptr) _module = _main;
        try {
            CodeObject_ code = compile(source, filename, mode);
#if DEBUG_DIS_EXEC
            if(_module == _main) std::cout << disassemble(code) << '\n';
#endif
            return _exec(code, _module);
        }catch (const Exception& e){
            _stderr(this, e.summary() + "\n");
        }
#if !DEBUG_FULL_EXCEPTION
        catch (const std::exception& e) {
            Str msg = "An std::exception occurred! It could be a bug.\n";
            msg = msg + e.what();
            _stderr(this, msg + "\n");
        }
#endif
        callstack.clear();
        s_data.clear();
        return nullptr;
    }

    template<typename ...Args>
    PyObject* _exec(Args&&... args){
        callstack.emplace(&s_data, s_data._sp, std::forward<Args>(args)...);
        return _run_top_frame();
    }

    void _pop_frame(){
        Frame* frame = &callstack.top();
        s_data.reset(frame->_sp_base);
        callstack.pop();
    }

    void _push_varargs(){ }
    void _push_varargs(PyObject* _0){ PUSH(_0); }
    void _push_varargs(PyObject* _0, PyObject* _1){ PUSH(_0); PUSH(_1); }
    void _push_varargs(PyObject* _0, PyObject* _1, PyObject* _2){ PUSH(_0); PUSH(_1); PUSH(_2); }
    void _push_varargs(PyObject* _0, PyObject* _1, PyObject* _2, PyObject* _3){ PUSH(_0); PUSH(_1); PUSH(_2); PUSH(_3); }

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

    PyObject* property(NativeFuncC fget, NativeFuncC fset=nullptr){
        PyObject* _0 = heap.gcnew(tp_native_func, NativeFunc(fget, 1, false));
        PyObject* _1 = vm->None;
        if(fset != nullptr) _1 = heap.gcnew(tp_native_func, NativeFunc(fset, 2, false));
        return call(_t(tp_property), _0, _1);
    }

    PyObject* new_type_object(PyObject* mod, StrName name, Type base, bool subclass_enabled=true){
        PyObject* obj = heap._new<Type>(tp_type, _all_types.size());
        const PyTypeInfo& base_info = _all_types[base];
        if(!base_info.subclass_enabled){
            TypeError(fmt("type ", base_info.name.escape(), " is not `subclass_enabled`"));
        }
        PyTypeInfo info{
            obj,
            base,
            (mod!=nullptr && mod!=builtins) ? Str(OBJ_NAME(mod)+"."+name.sv()): name.sv(),
            subclass_enabled,
        };
        if(mod != nullptr) mod->attr().set(name, obj);
        _all_types.push_back(info);
        return obj;
    }

    Type _new_type_object(StrName name, Type base=0) {
        PyObject* obj = new_type_object(nullptr, name, base, false);
        return OBJ_GET(Type, obj);
    }

    PyObject* _find_type_object(const Str& type){
        PyObject* obj = builtins->attr().try_get(type);
        if(obj == nullptr){
            for(auto& t: _all_types) if(t.name == type) return t.obj;
            throw std::runtime_error(fmt("type not found: ", type));
        }
        check_non_tagged_type(obj, tp_type);
        return obj;
    }

    Type _type(const Str& type){
        PyObject* obj = _find_type_object(type);
        return OBJ_GET(Type, obj);
    }

    PyTypeInfo* _type_info(const Str& type){
        PyObject* obj = builtins->attr().try_get(type);
        if(obj == nullptr){
            for(auto& t: _all_types) if(t.name == type) return &t;
            FATAL_ERROR();
        }
        return &_all_types[OBJ_GET(Type, obj)];
    }

    PyTypeInfo* _type_info(Type type){
        return &_all_types[type];
    }

    const PyTypeInfo* _inst_type_info(PyObject* obj){
        if(is_int(obj)) return &_all_types[tp_int];
        if(is_float(obj)) return &_all_types[tp_float];
        return &_all_types[obj->type];
    }

#define BIND_UNARY_SPECIAL(name)                                                        \
    void bind##name(Type type, PyObject* (*f)(VM*, PyObject*)){                         \
        _all_types[type].m##name = f;                                                   \
        PyObject* nf = bind_method<0>(_t(type), #name, [](VM* vm, ArgsView args){       \
            return lambda_get_userdata<PyObject*(*)(VM*, PyObject*)>(args.begin())(vm, args[0]);\
        });                                                                             \
        OBJ_GET(NativeFunc, nf).set_userdata(f);                                        \
    }

    BIND_UNARY_SPECIAL(__repr__)
    BIND_UNARY_SPECIAL(__str__)
    BIND_UNARY_SPECIAL(__iter__)
    BIND_UNARY_SPECIAL(__next__)
    BIND_UNARY_SPECIAL(__json__)
    BIND_UNARY_SPECIAL(__neg__)
    BIND_UNARY_SPECIAL(__bool__)

    void bind__hash__(Type type, i64 (*f)(VM* vm, PyObject*));
    void bind__len__(Type type, i64 (*f)(VM* vm, PyObject*));
#undef BIND_UNARY_SPECIAL


#define BIND_LOGICAL_SPECIAL(name)                                                      \
    void bind##name(Type type, bool (*f)(VM*, PyObject*, PyObject*)){                   \
        PyObject* obj = _t(type);                                                       \
        _all_types[type].m##name = f;                                                   \
        PyObject* nf = bind_method<1>(obj, #name, [](VM* vm, ArgsView args){            \
            bool ok = lambda_get_userdata<bool(*)(VM*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1]); \
            return ok ? vm->True : vm->False;                                           \
        });                                                                             \
        OBJ_GET(NativeFunc, nf).set_userdata(f);                                        \
    }

    BIND_LOGICAL_SPECIAL(__eq__)
    BIND_LOGICAL_SPECIAL(__lt__)
    BIND_LOGICAL_SPECIAL(__le__)
    BIND_LOGICAL_SPECIAL(__gt__)
    BIND_LOGICAL_SPECIAL(__ge__)
    BIND_LOGICAL_SPECIAL(__contains__)

#undef BIND_LOGICAL_SPECIAL


#define BIND_BINARY_SPECIAL(name)                                                       \
    void bind##name(Type type, PyObject* (*f)(VM*, PyObject*, PyObject*)){              \
        PyObject* obj = _t(type);                                                       \
        _all_types[type].m##name = f;                                                   \
        PyObject* nf = bind_method<1>(obj, #name, [](VM* vm, ArgsView args){            \
            return lambda_get_userdata<PyObject*(*)(VM*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1]); \
        });                                                                             \
        OBJ_GET(NativeFunc, nf).set_userdata(f);                                        \
    }

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

    void bind__getitem__(Type type, PyObject* (*f)(VM*, PyObject*, PyObject*)){
        PyObject* obj = _t(type);
        _all_types[type].m__getitem__ = f;
        PyObject* nf = bind_method<1>(obj, "__getitem__", [](VM* vm, ArgsView args){
            return lambda_get_userdata<PyObject*(*)(VM*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1]);
        });
        OBJ_GET(NativeFunc, nf).set_userdata(f);
    }

    void bind__setitem__(Type type, void (*f)(VM*, PyObject*, PyObject*, PyObject*)){
        PyObject* obj = _t(type);
        _all_types[type].m__setitem__ = f;
        PyObject* nf = bind_method<2>(obj, "__setitem__", [](VM* vm, ArgsView args){
            lambda_get_userdata<void(*)(VM* vm, PyObject*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1], args[2]);
            return vm->None;
        });
        OBJ_GET(NativeFunc, nf).set_userdata(f);
    }

    void bind__delitem__(Type type, void (*f)(VM*, PyObject*, PyObject*)){
        PyObject* obj = _t(type);
        _all_types[type].m__delitem__ = f;
        PyObject* nf = bind_method<1>(obj, "__delitem__", [](VM* vm, ArgsView args){
            lambda_get_userdata<void(*)(VM*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1]);
            return vm->None;
        });
        OBJ_GET(NativeFunc, nf).set_userdata(f);
    }

    bool py_equals(PyObject* lhs, PyObject* rhs){
        if(lhs == rhs) return true;
        const PyTypeInfo* ti = _inst_type_info(lhs);
        if(ti->m__eq__) return ti->m__eq__(this, lhs, rhs);
        return call_method(lhs, __eq__, rhs) == True;
    }

    template<int ARGC>
    PyObject* bind_func(Str type, Str name, NativeFuncC fn) {
        return bind_func<ARGC>(_find_type_object(type), name, fn);
    }

    template<int ARGC>
    PyObject* bind_method(Str type, Str name, NativeFuncC fn) {
        return bind_method<ARGC>(_find_type_object(type), name, fn);
    }

    template<int ARGC, typename __T>
    PyObject* bind_constructor(__T&& type, NativeFuncC fn) {
        static_assert(ARGC==-1 || ARGC>=1);
        return bind_func<ARGC>(std::forward<__T>(type), "__new__", fn);
    }

    template<typename T, typename __T>
    PyObject* bind_default_constructor(__T&& type) {
        return bind_constructor<1>(std::forward<__T>(type), [](VM* vm, ArgsView args){
            Type t = OBJ_GET(Type, args[0]);
            return vm->heap.gcnew<T>(t, T());
        });
    }

    template<typename T, typename __T>
    PyObject* bind_notimplemented_constructor(__T&& type) {
        return bind_constructor<-1>(std::forward<__T>(type), [](VM* vm, ArgsView args){
            vm->NotImplementedError();
            return vm->None;
        });
    }

    template<int ARGC>
    PyObject* bind_builtin_func(Str name, NativeFuncC fn) {
        return bind_func<ARGC>(builtins, name, fn);
    }

    int normalized_index(int index, int size){
        if(index < 0) index += size;
        if(index < 0 || index >= size){
            IndexError(std::to_string(index) + " not in [0, " + std::to_string(size) + ")");
        }
        return index;
    }

    PyObject* py_next(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__next__) return ti->m__next__(this, obj);
        return call_method(obj, __next__);
    }
    
    /***** Error Reporter *****/
    void _error(StrName name, const Str& msg){
        _error(Exception(name, msg));
    }

    void _raise(){
        bool ok = top_frame()->jump_to_exception_handler();
        if(ok) throw HandledException();
        else throw UnhandledException();
    }

    void StackOverflowError() { _error("StackOverflowError", ""); }
    void IOError(const Str& msg) { _error("IOError", msg); }
    void NotImplementedError(){ _error("NotImplementedError", ""); }
    void TypeError(const Str& msg){ _error("TypeError", msg); }
    void IndexError(const Str& msg){ _error("IndexError", msg); }
    void ValueError(const Str& msg){ _error("ValueError", msg); }
    void NameError(StrName name){ _error("NameError", fmt("name ", name.escape() + " is not defined")); }
    void KeyError(PyObject* obj){ _error("KeyError", OBJ_GET(Str, py_repr(obj))); }

    void AttributeError(PyObject* obj, StrName name){
        // OBJ_NAME calls getattr, which may lead to a infinite recursion
        _error("AttributeError", fmt("type ", OBJ_NAME(_t(obj)).escape(), " has no attribute ", name.escape()));
    }

    void AttributeError(Str msg){ _error("AttributeError", msg); }

    void check_type(PyObject* obj, Type type){
        if(is_type(obj, type)) return;
        TypeError("expected " + OBJ_NAME(_t(type)).escape() + ", but got " + OBJ_NAME(_t(obj)).escape());
    }

    void check_non_tagged_type(PyObject* obj, Type type){
        if(is_non_tagged_type(obj, type)) return;
        TypeError("expected " + OBJ_NAME(_t(type)).escape() + ", but got " + OBJ_NAME(_t(obj)).escape());
    }

    void check_int(PyObject* obj){
        if(is_int(obj)) return;
        check_type(obj, tp_int);    // if failed, redirect to check_type to raise TypeError
    }

    void check_float(PyObject* obj){
        if(is_float(obj)) return;
        check_type(obj, tp_float);  // if failed, redirect to check_type to raise TypeError
    }

    PyObject* _t(Type t){
        return _all_types[t.index].obj;
    }

    PyObject* _t(PyObject* obj){
        if(is_int(obj)) return _t(tp_int);
        if(is_float(obj)) return _t(tp_float);
        return _all_types[obj->type].obj;
    }

    struct ImportContext{
        // 0: normal; 1: __init__.py; 2: relative
        std::vector<std::pair<StrName, int>> pending;

        struct Temp{
            VM* vm;
            StrName name;

            Temp(VM* vm, StrName name, int type): vm(vm), name(name){
                ImportContext* ctx = &vm->_import_context;
                ctx->pending.emplace_back(name, type);
            }

            ~Temp(){
                ImportContext* ctx = &vm->_import_context;
                ctx->pending.pop_back();
            }
        };

        Temp temp(VM* vm, StrName name, int type){
            return Temp(vm, name, type);
        }
    };

    ImportContext _import_context;

    PyObject* py_import(StrName name, bool relative=false){
        Str filename;
        int type;
        if(relative){
            ImportContext* ctx = &_import_context;
            type = 2;
            for(auto it=ctx->pending.rbegin(); it!=ctx->pending.rend(); ++it){
                if(it->second == 2) continue;
                if(it->second == 1){
                    filename = fmt(it->first, kPlatformSep, name, ".py");
                    name = fmt(it->first, '.', name).c_str();
                    break;
                }
            }
            if(filename.length() == 0) _error("ImportError", "relative import outside of package");
        }else{
            type = 0;
            filename = fmt(name, ".py");
        }
        for(auto& [k, v]: _import_context.pending){
            if(k == name){
                vm->_error("ImportError", fmt("circular import ", name.escape()));
            }
        }
        PyObject* ext_mod = _modules.try_get(name);
        if(ext_mod == nullptr){
            Str source;
            auto it = _lazy_modules.find(name);
            if(it == _lazy_modules.end()){
                Bytes b = _import_handler(filename);
                if(!relative && !b){
                    filename = fmt(name, kPlatformSep, "__init__.py");
                    b = _import_handler(filename);
                    if(b) type = 1;
                }
                if(!b) _error("ImportError", fmt("module ", name.escape(), " not found"));
                source = Str(b.str());
            }else{
                source = it->second;
                _lazy_modules.erase(it);
            }
            auto _ = _import_context.temp(this, name, type);
            CodeObject_ code = compile(source, filename, EXEC_MODE);
            PyObject* new_mod = new_module(name);
            _exec(code, new_mod);
            new_mod->attr()._try_perfect_rehash();
            return new_mod;
        }else{
            return ext_mod;
        }
    }

    ~VM() {
        callstack.clear();
        s_data.clear();
        _all_types.clear();
        _modules.clear();
        _lazy_modules.clear();
    }
#if DEBUG_CEVAL_STEP
    void _log_s_data(const char* title = nullptr);
#endif
    void _unpack_as_list(ArgsView args, List& list);
    void _unpack_as_dict(ArgsView args, Dict& dict);
    PyObject* vectorcall(int ARGC, int KWARGC=0, bool op_call=false);
    CodeObject_ compile(Str source, Str filename, CompileMode mode, bool unknown_global_scope=false);
    PyObject* py_negate(PyObject* obj);
    f64 num_to_float(PyObject* obj);
    bool py_bool(PyObject* obj);
    i64 py_hash(PyObject* obj);
    PyObject* py_list(PyObject*);
    PyObject* new_module(StrName name);
    Str disassemble(CodeObject_ co);
    void init_builtin_types();
    PyObject* getattr(PyObject* obj, StrName name, bool throw_err=true);
    PyObject* get_unbound_method(PyObject* obj, StrName name, PyObject** self, bool throw_err=true, bool fallback=false);
    void parse_int_slice(const Slice& s, int length, int& start, int& stop, int& step);
    PyObject* format(Str, PyObject*);
    void setattr(PyObject* obj, StrName name, PyObject* value);
    template<int ARGC>
    PyObject* bind_method(PyObject*, Str, NativeFuncC);
    template<int ARGC>
    PyObject* bind_func(PyObject*, Str, NativeFuncC);
    void _error(Exception);
    PyObject* _run_top_frame();
    void post_init();
    PyObject* _py_generator(Frame&& frame, ArgsView buffer);
};

inline PyObject* NativeFunc::operator()(VM* vm, ArgsView args) const{
    int args_size = args.size() - (int)method;  // remove self
    if(args_size != argc && argc != -1) {
        vm->TypeError(fmt("expected ", argc, " arguments, but got ", args_size));
    }
#if DEBUG_EXTRA_CHECK
    if(f == nullptr) FATAL_ERROR();
#endif
    return f(vm, args);
}

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

#undef DEF_NATIVE_2

#define PY_CAST_INT(T)                                  \
template<> inline T py_cast<T>(VM* vm, PyObject* obj){  \
    vm->check_int(obj);                                 \
    return (T)(BITS(obj) >> 2);                         \
}                                                       \
template<> inline T _py_cast<T>(VM* vm, PyObject* obj){ \
    return (T)(BITS(obj) >> 2);                         \
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
    vm->check_float(obj);
    i64 bits = BITS(obj) & Number::c1;
    return BitsCvt(bits)._float;
}
template<> inline float _py_cast<float>(VM* vm, PyObject* obj){
    i64 bits = BITS(obj) & Number::c1;
    return BitsCvt(bits)._float;
}
template<> inline double py_cast<double>(VM* vm, PyObject* obj){
    vm->check_float(obj);
    i64 bits = BITS(obj) & Number::c1;
    return BitsCvt(bits)._float;
}
template<> inline double _py_cast<double>(VM* vm, PyObject* obj){
    i64 bits = BITS(obj) & Number::c1;
    return BitsCvt(bits)._float;
}


#define PY_VAR_INT(T)                                       \
    inline PyObject* py_var(VM* vm, T _val){                \
        i64 val = static_cast<i64>(_val);                   \
        if(((val << 2) >> 2) != val){                       \
            vm->_error("OverflowError", std::to_string(val) + " is out of range");  \
        }                                                                           \
        val = (val << 2) | 0b01;                                                    \
        return reinterpret_cast<PyObject*>(val);                                    \
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


#define PY_VAR_FLOAT(T)                             \
    inline PyObject* py_var(VM* vm, T _val){        \
        BitsCvt val(static_cast<f64>(_val));        \
        i64 bits = val._int & Number::c1;           \
        i64 tail = val._int & Number::c2;           \
        if(tail == 0b10){                           \
            if(bits&0b100) bits += 0b100;           \
        }else if(tail == 0b11){                     \
            bits += 0b100;                          \
        }                                           \
        bits |= 0b10;                               \
        return reinterpret_cast<PyObject*>(bits);   \
    }

PY_VAR_FLOAT(float)
PY_VAR_FLOAT(double)

#undef PY_VAR_INT
#undef PY_VAR_FLOAT

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

inline PyObject* py_var(VM* vm, const char val[]){
    return VAR(Str(val));
}

inline PyObject* py_var(VM* vm, std::string val){
    return VAR(Str(std::move(val)));
}

inline PyObject* py_var(VM* vm, std::string_view val){
    return VAR(Str(val));
}

inline PyObject* py_var(VM* vm, NoReturn val){
    return vm->None;
}

inline PyObject* py_var(VM* vm, PyObject* val){
    return val;
}

inline PyObject* VM::py_negate(PyObject* obj){
    const PyTypeInfo* ti = _inst_type_info(obj);
    if(ti->m__neg__) return ti->m__neg__(this, obj);
    return call_method(obj, __neg__);
}

inline f64 VM::num_to_float(PyObject* obj){
    if(is_float(obj)){
        return _CAST(f64, obj);
    } else if (is_int(obj)){
        return (f64)_CAST(i64, obj);
    }
    TypeError("expected 'int' or 'float', got " + OBJ_NAME(_t(obj)).escape());
    return 0;
}

inline bool VM::py_bool(PyObject* obj){
    if(is_non_tagged_type(obj, tp_bool)) return obj == True;
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

inline PyObject* VM::py_list(PyObject* it){
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

inline void VM::parse_int_slice(const Slice& s, int length, int& start, int& stop, int& step){
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

inline i64 VM::py_hash(PyObject* obj){
    const PyTypeInfo* ti = _inst_type_info(obj);
    if(ti->m__hash__) return ti->m__hash__(this, obj);
    PyObject* ret = call_method(obj, __hash__);
    return CAST(i64, ret);
}

inline PyObject* VM::format(Str spec, PyObject* obj){
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
    if(spec[0] == '0'){
        pad_c = '0';
        spec = spec.substr(1);
    }
    char align;
    if(spec[0] == '>'){
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
            width = Number::stoi(spec.substr(0, dot).str());
            precision = Number::stoi(spec.substr(dot+1).str());
        }else{
            width = Number::stoi(spec.str());
            precision = -1;
        }
    }catch(...){
        ValueError("invalid format specifer");
    }

    if(type != 'f' && dot >= 0) ValueError("precision not allowed in the format specifier");
    Str ret;
    if(type == 'f'){
        f64 val = num_to_float(obj);
        if(precision < 0) precision = 6;
        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision) << val;
        ret = ss.str();
    }else if(type == 'd'){
        ret = std::to_string(CAST(i64, obj));
    }else if(type == 's'){
        ret = CAST(Str&, obj);
    }else{
        ret = CAST(Str&, py_str(obj));
    }
    if(width > ret.length()){
        int pad = width - ret.length();
        std::string padding(pad, pad_c);
        if(align == '>') ret = padding.c_str() + ret;
        else ret = ret + padding.c_str();
    }
    return VAR(ret);
}

inline PyObject* VM::new_module(StrName name) {
    PyObject* obj = heap._new<DummyModule>(tp_module, DummyModule());
    obj->attr().set("__name__", VAR(name.sv()));
    // we do not allow override in order to avoid memory leak
    // it is because Module objects are not garbage collected
    if(_modules.contains(name)) throw std::runtime_error("module already exists");
    _modules.set(name, obj);
    return obj;
}

inline std::string _opcode_argstr(VM* vm, Bytecode byte, const CodeObject* co){
    std::string argStr = byte.arg == -1 ? "" : std::to_string(byte.arg);
    switch(byte.op){
        case OP_LOAD_CONST:
            if(vm != nullptr){
                argStr += fmt(" (", CAST(Str, vm->py_repr(co->consts[byte.arg])), ")");
            }
            break;
        case OP_LOAD_NAME: case OP_LOAD_GLOBAL: case OP_LOAD_NONLOCAL: case OP_STORE_GLOBAL:
        case OP_LOAD_ATTR: case OP_LOAD_METHOD: case OP_STORE_ATTR: case OP_DELETE_ATTR:
        case OP_IMPORT_NAME: case OP_BEGIN_CLASS: case OP_RAISE:
        case OP_DELETE_GLOBAL: case OP_INC_GLOBAL: case OP_DEC_GLOBAL:
            argStr += fmt(" (", StrName(byte.arg).sv(), ")");
            break;
        case OP_LOAD_FAST: case OP_STORE_FAST: case OP_DELETE_FAST: case OP_INC_FAST: case OP_DEC_FAST:
            argStr += fmt(" (", co->varnames[byte.arg].sv(), ")");
            break;
        case OP_LOAD_FUNCTION:
            argStr += fmt(" (", co->func_decls[byte.arg]->code->name, ")");
            break;
    }
    return argStr;
}

inline Str VM::disassemble(CodeObject_ co){
    auto pad = [](const Str& s, const int n){
        if(s.length() >= n) return s.substr(0, n);
        return s + std::string(n - s.length(), ' ');
    };

    std::vector<int> jumpTargets;
    for(auto byte : co->codes){
        if(byte.op == OP_JUMP_ABSOLUTE || byte.op == OP_POP_JUMP_IF_FALSE || byte.op == OP_POP_JUMP_IF_FALSE || byte.op == OP_SHORTCUT_IF_FALSE_OR_POP){
            jumpTargets.push_back(byte.arg);
        }
    }
    std::stringstream ss;
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

#if DEBUG_CEVAL_STEP
inline void VM::_log_s_data(const char* title) {
    if(_main == nullptr) return;
    if(callstack.empty()) return;
    std::stringstream ss;
    if(title) ss << title << " | ";
    std::map<PyObject**, int> sp_bases;
    for(Frame& f: callstack.data()){
        if(f._sp_base == nullptr) FATAL_ERROR();
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
            Type t = OBJ_GET(Type, obj);
            ss << "<class " + _all_types[t].name.escape() + ">";
        } else if(is_type(obj, tp_list)){
            auto& t = CAST(List&, obj);
            ss << "list(size=" << t.size() << ")";
        } else if(is_type(obj, tp_tuple)){
            auto& t = CAST(Tuple&, obj);
            ss << "tuple(size=" << t.size() << ")";
        } else ss << "(" << obj_type_name(this, obj->type) << ")";
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

inline void VM::init_builtin_types(){
    _all_types.push_back({heap._new<Type>(Type(1), Type(0)), -1, "object", true});
    _all_types.push_back({heap._new<Type>(Type(1), Type(1)), 0, "type", false});
    tp_object = 0; tp_type = 1;

    tp_int = _new_type_object("int");
    tp_float = _new_type_object("float");
    if(tp_int.index != kTpIntIndex || tp_float.index != kTpFloatIndex) FATAL_ERROR();

    tp_bool = _new_type_object("bool");
    tp_str = _new_type_object("str");
    tp_list = _new_type_object("list");
    tp_tuple = _new_type_object("tuple");
    tp_slice = _new_type_object("slice");
    tp_range = _new_type_object("range");
    tp_module = _new_type_object("module");
    tp_function = _new_type_object("function");
    tp_native_func = _new_type_object("native_func");
    tp_bound_method = _new_type_object("bound_method");
    tp_super = _new_type_object("super");
    tp_exception = _new_type_object("Exception");
    tp_bytes = _new_type_object("bytes");
    tp_mappingproxy = _new_type_object("mappingproxy");
    tp_dict = _new_type_object("dict");
    tp_property = _new_type_object("property");
    tp_star_wrapper = _new_type_object("_star_wrapper");

    this->None = heap._new<Dummy>(_new_type_object("NoneType"), {});
    this->Ellipsis = heap._new<Dummy>(_new_type_object("ellipsis"), {});
    this->True = heap._new<Dummy>(tp_bool, {});
    this->False = heap._new<Dummy>(tp_bool, {});
    this->StopIteration = heap._new<Dummy>(_new_type_object("StopIterationType"), {});

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
    builtins->attr().set("slice", _t(tp_slice));

    post_init();
    for(int i=0; i<_all_types.size(); i++){
        _all_types[i].obj->attr()._try_perfect_rehash();
    }
    for(auto [k, v]: _modules.items()) v->attr()._try_perfect_rehash();
    this->_main = new_module("__main__");
}

// `heap.gc_scope_lock();` needed before calling this function
inline void VM::_unpack_as_list(ArgsView args, List& list){
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
inline void VM::_unpack_as_dict(ArgsView args, Dict& dict){
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

inline PyObject* VM::vectorcall(int ARGC, int KWARGC, bool op_call){
    PyObject** p1 = s_data._sp - KWARGC*2;
    PyObject** p0 = p1 - ARGC - 2;
    // [callable, <self>, args..., kwargs...]
    //      ^p0                    ^p1      ^_sp
    PyObject* callable = p1[-(ARGC + 2)];
    bool method_call = p1[-(ARGC + 1)] != PY_NULL;

    // handle boundmethod, do a patch
    if(is_non_tagged_type(callable, tp_bound_method)){
        if(method_call) FATAL_ERROR();
        auto& bm = CAST(BoundMethod&, callable);
        callable = bm.func;      // get unbound method
        p1[-(ARGC + 2)] = bm.func;
        p1[-(ARGC + 1)] = bm.self;
        method_call = true;
        // [unbound, self, args..., kwargs...]
    }

    ArgsView args(p1 - ARGC - int(method_call), p1);

    if(is_non_tagged_type(callable, tp_native_func)){
        const auto& f = OBJ_GET(NativeFunc, callable);
        if(KWARGC != 0) TypeError("native_func does not accept keyword arguments");
        PyObject* ret = f(this, args);
        s_data.reset(p0);
        return ret;
    }

    ArgsView kwargs(p1, s_data._sp);

    if(is_non_tagged_type(callable, tp_function)){
        /*****************_py_call*****************/
        // callable must be a `function` object
        if(s_data.is_overflow()) StackOverflowError();

        const Function& fn = CAST(Function&, callable);
        const CodeObject* co = fn.decl->code.get();
        int co_nlocals = co->varnames.size();

        if(args.size() < fn.argc){
            vm->TypeError(fmt(
                "expected ",
                fn.argc - (int)method_call,
                " positional arguments, but got ",
                args.size() - (int)method_call,
                " (", fn.decl->code->name, ')'
            ));
        }

        // if this function is simple, a.k.a, no kwargs and no *args and not a generator
        // we can use a fast path to avoid using buffer copy
        if(fn.is_simple){
            if(args.size() > fn.argc) TypeError("too many positional arguments");
            int spaces = co_nlocals - fn.argc;
            for(int j=0; j<spaces; j++) PUSH(PY_NULL);
            callstack.emplace(&s_data, p0, co, fn._module, callable, FastLocals(co, args.begin()));
            if(op_call) return PY_OP_CALL;
            return _run_top_frame();
        }

        int i = 0;
        static THREAD_LOCAL PyObject* buffer[PK_MAX_CO_VARNAMES];

        // prepare args
        for(int index: fn.decl->args) buffer[index] = args[i++];
        // set extra varnames to nullptr
        for(int j=i; j<co_nlocals; j++) buffer[j] = PY_NULL;
        // prepare kwdefaults
        for(auto& kv: fn.decl->kwargs) buffer[kv.key] = kv.value;
        
        // handle *args
        if(fn.decl->starred_arg != -1){
            ArgsView vargs(args.begin() + i, args.end());
            buffer[fn.decl->starred_arg] = VAR(vargs.to_tuple());
            i += vargs.size();
        }else{
            // kwdefaults override
            for(auto& kv: fn.decl->kwargs){
                if(i >= args.size()) break;
                buffer[kv.key] = args[i++];
            }
            if(i < args.size()) TypeError(fmt("too many arguments", " (", fn.decl->code->name, ')'));
        }
        
        PyObject* vkwargs;
        if(fn.decl->starred_kwarg != -1){
            vkwargs = VAR(Dict(this));
            buffer[fn.decl->starred_kwarg] = vkwargs;
        }else{
            vkwargs = nullptr;
        }

        for(int i=0; i<kwargs.size(); i+=2){
            StrName key(CAST(int, kwargs[i]));
            int index = co->varnames_inv.try_get(key);
            if(index < 0){
                if(vkwargs == nullptr){
                    TypeError(fmt(key.escape(), " is an invalid keyword argument for ", co->name, "()"));
                }else{
                    Dict& dict = _CAST(Dict&, vkwargs);
                    dict.set(VAR(key.sv()), kwargs[i+1]);
                }
            }else{
                buffer[index] = kwargs[i+1];
            }
        }
        
        if(co->is_generator){
            s_data.reset(p0);
            return _py_generator(
                Frame(&s_data, nullptr, co, fn._module, callable),
                ArgsView(buffer, buffer + co_nlocals)
            );
        }

        // copy buffer back to stack
        s_data.reset(args.begin());
        for(int i=0; i<co_nlocals; i++) PUSH(buffer[i]);
        callstack.emplace(&s_data, p0, co, fn._module, callable, FastLocals(co, args.begin()));
        if(op_call) return PY_OP_CALL;
        return _run_top_frame();
        /*****************_py_call*****************/
    }

    if(is_non_tagged_type(callable, tp_type)){
        if(method_call) FATAL_ERROR();
        // [type, NULL, args..., kwargs...]

        DEF_SNAME(__new__);
        PyObject* new_f = find_name_in_mro(callable, __new__);
        PyObject* obj;
        if(new_f != nullptr){
            PUSH(new_f);
            PUSH(PY_NULL);
            PUSH(callable);    // cls
            for(PyObject* obj: args) PUSH(obj);
            for(PyObject* obj: kwargs) PUSH(obj);
            // if obj is not an instance of callable, the behavior is undefined
            obj = vectorcall(ARGC+1, KWARGC);
        }else{
            // fast path for object.__new__
            Type t = OBJ_GET(Type, callable);
            obj= vm->heap.gcnew<DummyInstance>(t, {});
        }

        // __init__
        PyObject* self;
        DEF_SNAME(__init__);
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
    DEF_SNAME(__call__);
    PyObject* call_f = get_unbound_method(callable, __call__, &self, false);
    if(self != PY_NULL){
        p1[-(ARGC + 2)] = call_f;
        p1[-(ARGC + 1)] = self;
        // [call_f, self, args..., kwargs...]
        return vectorcall(ARGC, KWARGC, false);
    }
    TypeError(OBJ_NAME(_t(callable)).escape() + " object is not callable");
    return nullptr;
}

// https://docs.python.org/3/howto/descriptor.html#invocation-from-an-instance
inline PyObject* VM::getattr(PyObject* obj, StrName name, bool throw_err){
    PyObject* objtype = _t(obj);
    // handle super() proxy
    if(is_non_tagged_type(obj, tp_super)){
        const Super& super = OBJ_GET(Super, obj);
        obj = super.first;
        objtype = _t(super.second);
    }
    PyObject* cls_var = find_name_in_mro(objtype, name);
    if(cls_var != nullptr){
        // handle descriptor
        if(is_non_tagged_type(cls_var, tp_property)){
            const Property& prop = _CAST(Property&, cls_var);
            return call(prop.getter, obj);
        }
    }
    // handle instance __dict__
    if(!is_tagged(obj) && obj->is_attr_valid()){
        PyObject* val = obj->attr().try_get(name);
        if(val != nullptr) return val;
    }
    if(cls_var != nullptr){
        // bound method is non-data descriptor
        if(is_non_tagged_type(cls_var, tp_function) || is_non_tagged_type(cls_var, tp_native_func)){
            return VAR(BoundMethod(obj, cls_var));
        }
        return cls_var;
    }
    if(throw_err) AttributeError(obj, name);
    return nullptr;
}

// used by OP_LOAD_METHOD
// try to load a unbound method (fallback to `getattr` if not found)
inline PyObject* VM::get_unbound_method(PyObject* obj, StrName name, PyObject** self, bool throw_err, bool fallback){
    *self = PY_NULL;
    PyObject* objtype = _t(obj);
    // handle super() proxy
    if(is_non_tagged_type(obj, tp_super)){
        const Super& super = OBJ_GET(Super, obj);
        obj = super.first;
        objtype = _t(super.second);
    }
    PyObject* cls_var = find_name_in_mro(objtype, name);

    if(fallback){
        if(cls_var != nullptr){
            // handle descriptor
            if(is_non_tagged_type(cls_var, tp_property)){
                const Property& prop = _CAST(Property&, cls_var);
                return call(prop.getter, obj);
            }
        }
        // handle instance __dict__
        if(!is_tagged(obj) && obj->is_attr_valid()){
            PyObject* val = obj->attr().try_get(name);
            if(val != nullptr) return val;
        }
    }

    if(cls_var != nullptr){
        if(is_non_tagged_type(cls_var, tp_function) || is_non_tagged_type(cls_var, tp_native_func)){
            *self = obj;
        }
        return cls_var;
    }
    if(throw_err) AttributeError(obj, name);
    return nullptr;
}

inline void VM::setattr(PyObject* obj, StrName name, PyObject* value){
    PyObject* objtype = _t(obj);
    // handle super() proxy
    if(is_non_tagged_type(obj, tp_super)){
        Super& super = OBJ_GET(Super, obj);
        obj = super.first;
        objtype = _t(super.second);
    }
    PyObject* cls_var = find_name_in_mro(objtype, name);
    if(cls_var != nullptr){
        // handle descriptor
        if(is_non_tagged_type(cls_var, tp_property)){
            const Property& prop = _CAST(Property&, cls_var);
            if(prop.setter != vm->None){
                call(prop.setter, obj, value);
            }else{
                TypeError(fmt("readonly attribute: ", name.escape()));
            }
            return;
        }
    }
    // handle instance __dict__
    if(is_tagged(obj) || !obj->is_attr_valid()) TypeError("cannot set attribute");
    obj->attr().set(name, value);
}

template<int ARGC>
PyObject* VM::bind_method(PyObject* obj, Str name, NativeFuncC fn) {
    check_non_tagged_type(obj, tp_type);
    PyObject* nf = VAR(NativeFunc(fn, ARGC, true));
    obj->attr().set(name, nf);
    return nf;
}

template<int ARGC>
PyObject* VM::bind_func(PyObject* obj, Str name, NativeFuncC fn) {
    PyObject* nf = VAR(NativeFunc(fn, ARGC, false));
    obj->attr().set(name, nf);
    return nf;
}

inline void VM::_error(Exception e){
    if(callstack.empty()){
        e.is_re = false;
        throw e;
    }
    PUSH(VAR(e));
    _raise();
}

inline void ManagedHeap::mark() {
    for(PyObject* obj: _no_gc) OBJ_MARK(obj);
    for(auto& frame : vm->callstack.data()) frame._gc_mark();
    for(PyObject* obj: vm->s_data) OBJ_MARK(obj);
    if(_gc_marker_ex) _gc_marker_ex(vm);
    if(vm->_last_exception) OBJ_MARK(vm->_last_exception);
}

inline Str obj_type_name(VM *vm, Type type){
    return vm->_all_types[type].name;
}

#undef PY_VAR_INT
#undef PY_VAR_FLOAT

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

inline void VM::bind__hash__(Type type, i64 (*f)(VM*, PyObject*)){
    PyObject* obj = _t(type);
    _all_types[type].m__hash__ = f;
    PyObject* nf = bind_method<0>(obj, "__hash__", [](VM* vm, ArgsView args){
        i64 ret = lambda_get_userdata<i64(*)(VM*, PyObject*)>(args.begin())(vm, args[0]);
        return VAR(ret);
    });
    OBJ_GET(NativeFunc, nf).set_userdata(f);
}

inline void VM::bind__len__(Type type, i64 (*f)(VM*, PyObject*)){
    PyObject* obj = _t(type);
    _all_types[type].m__len__ = f;
    PyObject* nf = bind_method<0>(obj, "__len__", [](VM* vm, ArgsView args){
        i64 ret = lambda_get_userdata<i64(*)(VM*, PyObject*)>(args.begin())(vm, args[0]);
        return VAR(ret);
    });
    OBJ_GET(NativeFunc, nf).set_userdata(f);
}


inline void Dict::_probe(PyObject *key, bool &ok, int &i) const{
    ok = false;
    i = vm->py_hash(key) & _mask;
    while(_items[i].first != nullptr) {
        if(vm->py_equals(_items[i].first, key)) { ok = true; break; }
        i = (i + 1) & _mask;
    }
}

}   // namespace pkpy


namespace pkpy{

inline PyObject* VM::_run_top_frame(){
    DEF_SNAME(add);
    DEF_SNAME(set);
    DEF_SNAME(__enter__);
    DEF_SNAME(__exit__);
    DEF_SNAME(__doc__);

    FrameId frame = top_frame();
    const int base_id = frame.index;
    bool need_raise = false;

    // shared registers
    PyObject *_0, *_1, *_2;
    const PyTypeInfo* _ti;
    StrName _name;

    while(true){
#if DEBUG_EXTRA_CHECK
        if(frame.index < base_id) FATAL_ERROR();
#endif
        try{
            if(need_raise){ need_raise = false; _raise(); }
/**********************************************************************/
/* NOTE: 
 * Be aware of accidental gc!
 * DO NOT leave any strong reference of PyObject* in the C stack
 */
{
#define DISPATCH_OP_CALL() { frame = top_frame(); goto __NEXT_FRAME; }
__NEXT_FRAME:
    Bytecode byte = frame->next_bytecode();
    // cache
    const CodeObject* co = frame->co;
    const auto& co_consts = co->consts;
    const auto& co_blocks = co->blocks;

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
OPCODE(POP_JUMP_IF_FALSE)
OPCODE(JUMP_IF_TRUE_OR_POP)
OPCODE(JUMP_IF_FALSE_OR_POP)
OPCODE(SHORTCUT_IF_FALSE_OR_POP)
OPCODE(LOOP_CONTINUE)
OPCODE(LOOP_BREAK)
OPCODE(GOTO)
/**************************/
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
/**************************/
OPCODE(GET_ITER)
OPCODE(FOR_ITER)
/**************************/
OPCODE(IMPORT_NAME)
OPCODE(IMPORT_NAME_REL)
OPCODE(IMPORT_STAR)
/**************************/
OPCODE(UNPACK_SEQUENCE)
OPCODE(UNPACK_EX)
/**************************/
OPCODE(BEGIN_CLASS)
OPCODE(END_CLASS)
OPCODE(STORE_CLASS_ATTR)
/**************************/
OPCODE(WITH_ENTER)
OPCODE(WITH_EXIT)
/**************************/
OPCODE(ASSERT)
OPCODE(EXCEPTION_MATCH)
OPCODE(RAISE)
OPCODE(RE_RAISE)
OPCODE(POP_EXCEPTION)
/**************************/
OPCODE(SETUP_DOCSTRING)
OPCODE(FORMAT_STRING)
/**************************/
OPCODE(INC_FAST)
OPCODE(DEC_FAST)
OPCODE(INC_GLOBAL)
OPCODE(DEC_GLOBAL)
#endif
    #undef OPCODE
};

#define DISPATCH() { byte = frame->next_bytecode(); goto *OP_LABELS[byte.op];}
#define TARGET(op) CASE_OP_##op:
goto *OP_LABELS[byte.op];

#else
#define TARGET(op) case OP_##op:
#define DISPATCH() { byte = frame->next_bytecode(); goto __NEXT_STEP;}

__NEXT_STEP:;
#if DEBUG_CEVAL_STEP
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
    TARGET(ROT_THREE)
        _0 = TOP();
        TOP() = SECOND();
        SECOND() = THIRD();
        THIRD() = _0;
        DISPATCH();
    TARGET(PRINT_EXPR)
        if(TOP() != None) _stdout(this, CAST(Str&, py_repr(TOP())) + "\n");
        POP();
        DISPATCH();
    /*****************************************/
    TARGET(LOAD_CONST)
        heap._auto_collect();
        PUSH(co_consts[byte.arg]);
        DISPATCH();
    TARGET(LOAD_NONE) PUSH(None); DISPATCH();
    TARGET(LOAD_TRUE) PUSH(True); DISPATCH();
    TARGET(LOAD_FALSE) PUSH(False); DISPATCH();
    TARGET(LOAD_INTEGER) PUSH(VAR(byte.arg)); DISPATCH();
    TARGET(LOAD_ELLIPSIS) PUSH(Ellipsis); DISPATCH();
    TARGET(LOAD_FUNCTION) {
        FuncDecl_ decl = co->func_decls[byte.arg];
        bool is_simple = decl->starred_kwarg==-1 && decl->starred_arg==-1 && decl->kwargs.size()==0 && !decl->code->is_generator;
        int argc = decl->args.size();
        PyObject* obj;
        if(decl->nested){
            NameDict_ captured = frame->_locals.to_namedict();
            obj = VAR(Function({decl, is_simple, argc, frame->_module, captured}));
            captured->set(decl->code->name, obj);
        }else{
            obj = VAR(Function({decl, is_simple, argc, frame->_module}));
        }
        PUSH(obj);
    } DISPATCH();
    TARGET(LOAD_NULL) PUSH(PY_NULL); DISPATCH();
    /*****************************************/
    TARGET(LOAD_FAST) {
        heap._auto_collect();
        _0 = frame->_locals[byte.arg];
        if(_0 == PY_NULL) vm->NameError(co->varnames[byte.arg]);
        PUSH(_0);
    } DISPATCH();
    TARGET(LOAD_NAME)
        heap._auto_collect();
        _name = StrName(byte.arg);
        _0 = frame->_locals.try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = frame->f_closure_try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = frame->f_globals().try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = vm->builtins->attr().try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        vm->NameError(_name);
        DISPATCH();
    TARGET(LOAD_NONLOCAL) {
        heap._auto_collect();
        _name = StrName(byte.arg);
        _0 = frame->f_closure_try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = frame->f_globals().try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = vm->builtins->attr().try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        vm->NameError(_name);
        DISPATCH();
    } DISPATCH();
    TARGET(LOAD_GLOBAL)
        heap._auto_collect();
        _name = StrName(byte.arg);
        _0 = frame->f_globals().try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = vm->builtins->attr().try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        vm->NameError(_name);
        DISPATCH();
    TARGET(LOAD_ATTR)
        TOP() = getattr(TOP(), StrName(byte.arg));
        DISPATCH();
    TARGET(LOAD_METHOD)
        TOP() = get_unbound_method(TOP(), StrName(byte.arg), &_0, true, true);
        PUSH(_0);
        DISPATCH();
    TARGET(LOAD_SUBSCR)
        _1 = POPX();    // b
        _0 = TOP();     // a
        _ti = _inst_type_info(_0);
        if(_ti->m__getitem__){
            TOP() = _ti->m__getitem__(this, _0, _1);
        }else{
            TOP() = call_method(_0, __getitem__, _1);
        }
        DISPATCH();
    TARGET(STORE_FAST)
        frame->_locals[byte.arg] = POPX();
        DISPATCH();
    TARGET(STORE_NAME)
        _name = StrName(byte.arg);
        _0 = POPX();
        if(frame->_callable != nullptr){
            bool ok = frame->_locals.try_set(_name, _0);
            if(!ok) vm->NameError(_name);
        }else{
            frame->f_globals().set(_name, _0);
        }
        DISPATCH();
    TARGET(STORE_GLOBAL)
        frame->f_globals().set(StrName(byte.arg), POPX());
        DISPATCH();
    TARGET(STORE_ATTR) {
        _0 = TOP();         // a
        _1 = SECOND();      // val
        setattr(_0, StrName(byte.arg), _1);
        STACK_SHRINK(2);
    } DISPATCH();
    TARGET(STORE_SUBSCR)
        _2 = POPX();        // b
        _1 = POPX();        // a
        _0 = POPX();        // val
        _ti = _inst_type_info(_1);
        if(_ti->m__setitem__){
            _ti->m__setitem__(this, _1, _2, _0);
        }else{
            call_method(_1, __setitem__, _2, _0);
        }
        DISPATCH();
    TARGET(DELETE_FAST)
        _0 = frame->_locals[byte.arg];
        if(_0 == PY_NULL) vm->NameError(co->varnames[byte.arg]);
        frame->_locals[byte.arg] = PY_NULL;
        DISPATCH();
    TARGET(DELETE_NAME)
        _name = StrName(byte.arg);
        if(frame->_callable != nullptr){
            if(!frame->_locals.contains(_name)) vm->NameError(_name);
            frame->_locals.erase(_name);
        }else{
            if(!frame->f_globals().contains(_name)) vm->NameError(_name);
            frame->f_globals().erase(_name);
        }
        DISPATCH();
    TARGET(DELETE_GLOBAL)
        _name = StrName(byte.arg);
        if(frame->f_globals().contains(_name)){
            frame->f_globals().erase(_name);
        }else{
            NameError(_name);
        }
        DISPATCH();
    TARGET(DELETE_ATTR)
        _0 = POPX();
        _name = StrName(byte.arg);
        if(is_tagged(_0) || !_0->is_attr_valid()) TypeError("cannot delete attribute");
        if(!_0->attr().contains(_name)) AttributeError(_0, _name);
        _0->attr().erase(_name);
        DISPATCH();
    TARGET(DELETE_SUBSCR)
        _1 = POPX();
        _0 = POPX();
        _ti = _inst_type_info(_0);
        if(_ti->m__delitem__){
            _ti->m__delitem__(this, _0, _1);
        }else{
            call_method(_0, __delitem__, _1);
        }
        DISPATCH();
    /*****************************************/
    TARGET(BUILD_TUPLE)
        _0 = VAR(STACK_VIEW(byte.arg).to_tuple());
        STACK_SHRINK(byte.arg);
        PUSH(_0);
        DISPATCH();
    TARGET(BUILD_LIST)
        _0 = VAR(STACK_VIEW(byte.arg).to_list());
        STACK_SHRINK(byte.arg);
        PUSH(_0);
        DISPATCH();
    TARGET(BUILD_DICT)
        if(byte.arg == 0){
            PUSH(VAR(Dict(this)));
            DISPATCH();
        }
        _0 = VAR(STACK_VIEW(byte.arg).to_list());
        _0 = call(_t(tp_dict), _0);
        STACK_SHRINK(byte.arg);
        PUSH(_0);
        DISPATCH();
    TARGET(BUILD_SET)
        _0 = VAR(STACK_VIEW(byte.arg).to_list());
        _0 = call(builtins->attr(set), _0);
        STACK_SHRINK(byte.arg);
        PUSH(_0);
        DISPATCH();
    TARGET(BUILD_SLICE)
        _2 = POPX();    // step
        _1 = POPX();    // stop
        _0 = POPX();    // start
        PUSH(VAR(Slice(_0, _1, _2)));
        DISPATCH();
    TARGET(BUILD_STRING) {
        std::stringstream ss;
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
        _0 = VAR(Tuple(std::move(list)));
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_LIST_UNPACK) {
        auto _lock = heap.gc_scope_lock();
        List list;
        _unpack_as_list(STACK_VIEW(byte.arg), list);
        STACK_SHRINK(byte.arg);
        _0 = VAR(std::move(list));
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_DICT_UNPACK) {
        auto _lock = heap.gc_scope_lock();
        Dict dict(this);
        _unpack_as_dict(STACK_VIEW(byte.arg), dict);
        STACK_SHRINK(byte.arg);
        _0 = VAR(std::move(dict));
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_SET_UNPACK) {
        auto _lock = heap.gc_scope_lock();
        List list;
        _unpack_as_list(STACK_VIEW(byte.arg), list);
        STACK_SHRINK(byte.arg);
        _0 = VAR(std::move(list));
        _0 = call(builtins->attr(set), _0);
        PUSH(_0);
    } DISPATCH();
    /*****************************************/
#define PREDICT_INT_OP(op)                              \
    if(is_both_int(TOP(), SECOND())){                   \
        _1 = POPX();                                    \
        _0 = TOP();                                     \
        TOP() = VAR(_CAST(i64, _0) op _CAST(i64, _1));  \
        DISPATCH();                                     \
    }

#define BINARY_OP_SPECIAL(func)                         \
        _1 = POPX();                                    \
        _0 = TOP();                                     \
        _ti = _inst_type_info(_0);                      \
        if(_ti->m##func){                               \
            TOP() = VAR(_ti->m##func(this, _0, _1));    \
        }else{                                          \
            TOP() = call_method(_0, func, _1);          \
        }

    TARGET(BINARY_TRUEDIV)
        if(is_tagged(SECOND())){
            f64 lhs = num_to_float(SECOND());
            f64 rhs = num_to_float(TOP());
            POP();
            TOP() = VAR(lhs / rhs);
            DISPATCH();
        }
        BINARY_OP_SPECIAL(__truediv__);
        DISPATCH();
    TARGET(BINARY_POW)
        BINARY_OP_SPECIAL(__pow__);
        DISPATCH();
    TARGET(BINARY_ADD)
        PREDICT_INT_OP(+);
        BINARY_OP_SPECIAL(__add__);
        DISPATCH()
    TARGET(BINARY_SUB)
        PREDICT_INT_OP(-);
        BINARY_OP_SPECIAL(__sub__);
        DISPATCH()
    TARGET(BINARY_MUL)
        BINARY_OP_SPECIAL(__mul__);
        DISPATCH()
    TARGET(BINARY_FLOORDIV)
        PREDICT_INT_OP(/);
        BINARY_OP_SPECIAL(__floordiv__);
        DISPATCH()
    TARGET(BINARY_MOD)
        PREDICT_INT_OP(%);
        BINARY_OP_SPECIAL(__mod__);
        DISPATCH()
    TARGET(COMPARE_LT)
        BINARY_OP_SPECIAL(__lt__);
        DISPATCH()
    TARGET(COMPARE_LE)
        BINARY_OP_SPECIAL(__le__);
        DISPATCH()
    TARGET(COMPARE_EQ)
        _1 = POPX();
        _0 = TOP();
        TOP() = VAR(py_equals(_0, _1));
        DISPATCH()
    TARGET(COMPARE_NE)
        _1 = POPX();
        _0 = TOP();
        TOP() = VAR(!py_equals(_0, _1));
        DISPATCH()
    TARGET(COMPARE_GT)
        BINARY_OP_SPECIAL(__gt__);
        DISPATCH()
    TARGET(COMPARE_GE)
        BINARY_OP_SPECIAL(__ge__);
        DISPATCH()
    TARGET(BITWISE_LSHIFT)
        PREDICT_INT_OP(<<);
        BINARY_OP_SPECIAL(__lshift__);
        DISPATCH()
    TARGET(BITWISE_RSHIFT)
        PREDICT_INT_OP(>>);
        BINARY_OP_SPECIAL(__rshift__);
        DISPATCH()
    TARGET(BITWISE_AND)
        PREDICT_INT_OP(&);
        BINARY_OP_SPECIAL(__and__);
        DISPATCH()
    TARGET(BITWISE_OR)
        PREDICT_INT_OP(|);
        BINARY_OP_SPECIAL(__or__);
        DISPATCH()
    TARGET(BITWISE_XOR)
        PREDICT_INT_OP(^);
        BINARY_OP_SPECIAL(__xor__);
        DISPATCH()
    TARGET(BINARY_MATMUL)
        BINARY_OP_SPECIAL(__matmul__);
        DISPATCH();

#undef BINARY_OP_SPECIAL
#undef PREDICT_INT_OP

    TARGET(IS_OP)
        _1 = POPX();    // rhs
        _0 = TOP();     // lhs
        TOP() = VAR(static_cast<bool>((_0==_1) ^ byte.arg));
        DISPATCH();
    TARGET(CONTAINS_OP)
        // a in b -> b __contains__ a
        _ti = _inst_type_info(TOP());
        if(_ti->m__contains__){
            _0 = VAR(_ti->m__contains__(this, TOP(), SECOND()));
        }else{
            _0 = call_method(TOP(), __contains__, SECOND());
        }
        POP();
        TOP() = VAR(static_cast<bool>(CAST(bool, _0) ^ byte.arg));
        DISPATCH();
    /*****************************************/
    TARGET(JUMP_ABSOLUTE)
        frame->jump_abs(byte.arg);
        DISPATCH();
    TARGET(POP_JUMP_IF_FALSE)
        if(!py_bool(POPX())) frame->jump_abs(byte.arg);
        DISPATCH();
    TARGET(JUMP_IF_TRUE_OR_POP)
        if(py_bool(TOP()) == true) frame->jump_abs(byte.arg);
        else POP();
        DISPATCH();
    TARGET(JUMP_IF_FALSE_OR_POP)
        if(py_bool(TOP()) == false) frame->jump_abs(byte.arg);
        else POP();
        DISPATCH();
    TARGET(SHORTCUT_IF_FALSE_OR_POP)
        if(py_bool(TOP()) == false){        // [b, False]
            STACK_SHRINK(2);                // []
            PUSH(vm->False);                // [False]
            frame->jump_abs(byte.arg);
        } else POP();                       // [b]
        DISPATCH();
    TARGET(LOOP_CONTINUE)
        frame->jump_abs(co_blocks[byte.block].start);
        DISPATCH();
    TARGET(LOOP_BREAK)
        frame->jump_abs_break(co_blocks[byte.block].end);
        DISPATCH();
    TARGET(GOTO) {
        _name = StrName(byte.arg);
        int index = co->labels.try_get(_name);
        if(index < 0) _error("KeyError", fmt("label ", _name.escape(), " not found"));
        frame->jump_abs_break(index);
    } DISPATCH();
    /*****************************************/
    TARGET(CALL)
        _0 = vectorcall(
            byte.arg & 0xFFFF,          // ARGC
            (byte.arg>>16) & 0xFFFF,    // KWARGC
            true
        );
        if(_0 == PY_OP_CALL) DISPATCH_OP_CALL();
        PUSH(_0);
        DISPATCH();
    TARGET(CALL_TP)
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
        DISPATCH();
    TARGET(RETURN_VALUE)
        _0 = POPX();
        _pop_frame();
        if(frame.index == base_id){       // [ frameBase<- ]
            return _0;
        }else{
            frame = top_frame();
            PUSH(_0);
            goto __NEXT_FRAME;
        }
    TARGET(YIELD_VALUE)
        return PY_OP_YIELD;
    /*****************************************/
    TARGET(LIST_APPEND)
        _0 = POPX();
        CAST(List&, SECOND()).push_back(_0);
        DISPATCH();
    TARGET(DICT_ADD) {
        _0 = POPX();
        Tuple& t = CAST(Tuple&, _0);
        call_method(SECOND(), __setitem__, t[0], t[1]);
    } DISPATCH();
    TARGET(SET_ADD)
        _0 = POPX();
        call_method(SECOND(), add, _0);
        DISPATCH();
    /*****************************************/
    TARGET(UNARY_NEGATIVE)
        TOP() = py_negate(TOP());
        DISPATCH();
    TARGET(UNARY_NOT)
        TOP() = VAR(!py_bool(TOP()));
        DISPATCH();
    TARGET(UNARY_STAR)
        TOP() = VAR(StarWrapper(byte.arg, TOP()));
        DISPATCH();
    /*****************************************/
    TARGET(GET_ITER)
        TOP() = py_iter(TOP());
        DISPATCH();
    TARGET(FOR_ITER)
        _0 = py_next(TOP());
        if(_0 != StopIteration){
            PUSH(_0);
        }else{
            frame->jump_abs_break(co_blocks[byte.block].end);
        }
        DISPATCH();
    /*****************************************/
    TARGET(IMPORT_NAME)
        _name = StrName(byte.arg);
        PUSH(py_import(_name));
        DISPATCH();
    TARGET(IMPORT_NAME_REL)
        _name = StrName(byte.arg);
        PUSH(py_import(_name, true));
        DISPATCH();
    TARGET(IMPORT_STAR)
        _0 = POPX();
        for(auto& [name, value]: _0->attr().items()){
            std::string_view s = name.sv();
            if(s.empty() || s[0] == '_') continue;
            frame->f_globals().set(name, value);
        }
        frame->f_globals()._try_perfect_rehash();
        DISPATCH();
    /*****************************************/
    TARGET(UNPACK_SEQUENCE){
        auto _lock = heap.gc_scope_lock();  // lock the gc via RAII!!
        _0 = py_iter(POPX());
        for(int i=0; i<byte.arg; i++){
            _1 = py_next(_0);
            if(_1 == StopIteration) ValueError("not enough values to unpack");
            PUSH(_1);
        }
        if(py_next(_0) != StopIteration) ValueError("too many values to unpack");
    } DISPATCH();
    TARGET(UNPACK_EX) {
        auto _lock = heap.gc_scope_lock();  // lock the gc via RAII!!
        _0 = py_iter(POPX());
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
    TARGET(BEGIN_CLASS)
        _name = StrName(byte.arg);
        _0 = POPX();   // super
        if(_0 == None) _0 = _t(tp_object);
        check_non_tagged_type(_0, tp_type);
        _1 = new_type_object(frame->_module, _name, OBJ_GET(Type, _0));
        PUSH(_1);
        DISPATCH();
    TARGET(END_CLASS)
        _0 = POPX();
        _0->attr()._try_perfect_rehash();
        DISPATCH();
    TARGET(STORE_CLASS_ATTR)
        _name = StrName(byte.arg);
        _0 = POPX();
        TOP()->attr().set(_name, _0);
        DISPATCH();
    /*****************************************/
    TARGET(WITH_ENTER)
        call_method(POPX(), __enter__);
        DISPATCH();
    TARGET(WITH_EXIT)
        call_method(POPX(), __exit__);
        DISPATCH();
    /*****************************************/
    TARGET(ASSERT) {
        _0 = TOP();
        Str msg;
        if(is_type(_0, tp_tuple)){
            auto& t = CAST(Tuple&, _0);
            if(t.size() != 2) ValueError("assert tuple must have 2 elements");
            _0 = t[0];
            msg = CAST(Str&, py_str(t[1]));
        }
        bool ok = py_bool(_0);
        POP();
        if(!ok) _error("AssertionError", msg);
    } DISPATCH();
    TARGET(EXCEPTION_MATCH) {
        const auto& e = CAST(Exception&, TOP());
        _name = StrName(byte.arg);
        PUSH(VAR(e.match_type(_name)));
    } DISPATCH();
    TARGET(RAISE) {
        _0 = POPX();
        Str msg = _0 == None ? "" : CAST(Str, py_str(_0));
        _error(StrName(byte.arg), msg);
    } DISPATCH();
    TARGET(RE_RAISE) _raise(); DISPATCH();
    TARGET(POP_EXCEPTION) _last_exception = POPX(); DISPATCH();
    /*****************************************/
    TARGET(SETUP_DOCSTRING)
        TOP()->attr().set(__doc__, co_consts[byte.arg]);
        DISPATCH();
    TARGET(FORMAT_STRING) {
        _0 = POPX();
        const Str& spec = CAST(Str&, co_consts[byte.arg]);
        PUSH(format(spec, _0));
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
        _name = StrName(byte.arg);
        PyObject** p = frame->f_globals().try_get_2(_name);
        if(p == nullptr) vm->NameError(_name);
        *p = VAR(CAST(i64, *p) + 1);
    } DISPATCH();
    TARGET(DEC_GLOBAL){
        _name = StrName(byte.arg);
        PyObject** p = frame->f_globals().try_get_2(_name);
        if(p == nullptr) vm->NameError(_name);
        *p = VAR(CAST(i64, *p) - 1);
    } DISPATCH();

#if !PK_ENABLE_COMPUTED_GOTO
#if DEBUG_EXTRA_CHECK
    default: throw std::runtime_error(fmt(OP_NAMES[byte.op], " is not implemented"));
#else
    default: UNREACHABLE();
#endif
    }
#endif
}

#undef DISPATCH
#undef TARGET
#undef DISPATCH_OP_CALL
/**********************************************************************/
            UNREACHABLE();
        }catch(HandledException& e){
            continue;
        }catch(UnhandledException& e){
            PyObject* obj = POPX();
            Exception& _e = CAST(Exception&, obj);
            _e.st_push(frame->snapshot());
            _pop_frame();
            if(callstack.empty()){
#if DEBUG_FULL_EXCEPTION
                std::cerr << _e.summary() << std::endl;
#endif
                throw _e;
            }
            frame = top_frame();
            PUSH(obj);
            if(frame.index < base_id) throw ToBeRaisedException();
            need_raise = true;
        }catch(ToBeRaisedException& e){
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

struct CodeEmitContext;
struct Expr;
typedef std::unique_ptr<Expr> Expr_;

struct Expr{
    int line = 0;
    virtual ~Expr() = default;
    virtual void emit(CodeEmitContext* ctx) = 0;
    virtual std::string str() const = 0;

    virtual bool is_literal() const { return false; }
    virtual bool is_json_object() const { return false; }
    virtual bool is_attrib() const { return false; }
    virtual bool is_compare() const { return false; }
    virtual int star_level() const { return 0; }
    bool is_starred() const { return star_level() > 0; }

    // for OP_DELETE_XXX
    [[nodiscard]] virtual bool emit_del(CodeEmitContext* ctx) { return false; }

    // for OP_STORE_XXX
    [[nodiscard]] virtual bool emit_store(CodeEmitContext* ctx) { return false; }
};

struct CodeEmitContext{
    VM* vm;
    CodeObject_ co;
    stack<Expr_> s_expr;
    int level;
    CodeEmitContext(VM* vm, CodeObject_ co, int level): vm(vm), co(co), level(level) {}

    int curr_block_i = 0;
    bool is_compiling_class = false;
    int for_loop_depth = 0;

    bool is_curr_block_loop() const {
        return co->blocks[curr_block_i].type == FOR_LOOP || co->blocks[curr_block_i].type == WHILE_LOOP;
    }

    void enter_block(CodeBlockType type){
        if(type == FOR_LOOP) for_loop_depth++;
        co->blocks.push_back(CodeBlock(
            type, curr_block_i, for_loop_depth, (int)co->codes.size()
        ));
        curr_block_i = co->blocks.size()-1;
    }

    void exit_block(){
        if(co->blocks[curr_block_i].type == FOR_LOOP) for_loop_depth--;
        co->blocks[curr_block_i].end = co->codes.size();
        curr_block_i = co->blocks[curr_block_i].parent;
        if(curr_block_i < 0) FATAL_ERROR();
    }

    // clear the expression stack and generate bytecode
    void emit_expr(){
        if(s_expr.size() != 1){
            throw std::runtime_error("s_expr.size() != 1\n" + _log_s_expr());
        }
        Expr_ expr = s_expr.popx();
        expr->emit(this);
    }

    std::string _log_s_expr(){
        std::stringstream ss;
        for(auto& e: s_expr.data()) ss << e->str() << " ";
        return ss.str();
    }

    int emit(Opcode opcode, int arg, int line) {
        co->codes.push_back(
            Bytecode{(uint16_t)opcode, (uint16_t)curr_block_i, arg}
        );
        co->lines.push_back(line);
        int i = co->codes.size() - 1;
        if(line==BC_KEEPLINE){
            if(i>=1) co->lines[i] = co->lines[i-1];
            else co->lines[i] = 1;
        }
        return i;
    }

    void patch_jump(int index) {
        int target = co->codes.size();
        co->codes[index].arg = target;
    }

    bool add_label(StrName name){
        if(co->labels.contains(name)) return false;
        co->labels.set(name, co->codes.size());
        return true;
    }

    int add_varname(StrName name){
        int index = co->varnames_inv.try_get(name);
        if(index >= 0) return index;
        co->varnames.push_back(name);
        index = co->varnames.size() - 1;
        co->varnames_inv.set(name, index);
        return index;
    }

    int add_const(PyObject* v){
        // simple deduplication, only works for int/float
        for(int i=0; i<co->consts.size(); i++){
            if(co->consts[i] == v) return i;
        }
        co->consts.push_back(v);
        return co->consts.size() - 1;
    }

    int add_func_decl(FuncDecl_ decl){
        co->func_decls.push_back(decl);
        return co->func_decls.size() - 1;
    }
};

struct NameExpr: Expr{
    StrName name;
    NameScope scope;
    NameExpr(StrName name, NameScope scope): name(name), scope(scope) {}

    std::string str() const override { return fmt("Name(", name.escape(), ")"); }

    void emit(CodeEmitContext* ctx) override {
        int index = ctx->co->varnames_inv.try_get(name);
        if(scope == NAME_LOCAL && index >= 0){
            ctx->emit(OP_LOAD_FAST, index, line);
        }else{
            Opcode op = ctx->level <= 1 ? OP_LOAD_GLOBAL : OP_LOAD_NONLOCAL;
            // we cannot determine the scope when calling exec()/eval()
            if(scope == NAME_GLOBAL_UNKNOWN) op = OP_LOAD_NAME;
            ctx->emit(op, StrName(name).index, line);
        }
    }

    bool emit_del(CodeEmitContext* ctx) override {
        switch(scope){
            case NAME_LOCAL:
                ctx->emit(OP_DELETE_FAST, ctx->add_varname(name), line);
                break;
            case NAME_GLOBAL:
                ctx->emit(OP_DELETE_GLOBAL, StrName(name).index, line);
                break;
            case NAME_GLOBAL_UNKNOWN:
                ctx->emit(OP_DELETE_NAME, StrName(name).index, line);
                break;
            default: FATAL_ERROR(); break;
        }
        return true;
    }

    bool emit_store(CodeEmitContext* ctx) override {
        if(ctx->is_compiling_class){
            int index = StrName(name).index;
            ctx->emit(OP_STORE_CLASS_ATTR, index, line);
            return true;
        }
        switch(scope){
            case NAME_LOCAL:
                ctx->emit(OP_STORE_FAST, ctx->add_varname(name), line);
                break;
            case NAME_GLOBAL:
                ctx->emit(OP_STORE_GLOBAL, StrName(name).index, line);
                break;
            case NAME_GLOBAL_UNKNOWN:
                ctx->emit(OP_STORE_NAME, StrName(name).index, line);
                break;
            default: FATAL_ERROR(); break;
        }
        return true;
    }
};

struct StarredExpr: Expr{
    int level;
    Expr_ child;
    StarredExpr(int level, Expr_&& child): level(level), child(std::move(child)) {}
    std::string str() const override { return fmt("Starred(level=", level, ")"); }

    int star_level() const override { return level; }

    void emit(CodeEmitContext* ctx) override {
        child->emit(ctx);
        ctx->emit(OP_UNARY_STAR, level, line);
    }

    bool emit_store(CodeEmitContext* ctx) override {
        if(level != 1) return false;
        // simply proxy to child
        return child->emit_store(ctx);
    }
};

struct NotExpr: Expr{
    Expr_ child;
    NotExpr(Expr_&& child): child(std::move(child)) {}
    std::string str() const override { return "Not()"; }

    void emit(CodeEmitContext* ctx) override {
        child->emit(ctx);
        ctx->emit(OP_UNARY_NOT, BC_NOARG, line);
    }
};

struct AndExpr: Expr{
    Expr_ lhs;
    Expr_ rhs;
    std::string str() const override { return "And()"; }

    void emit(CodeEmitContext* ctx) override {
        lhs->emit(ctx);
        int patch = ctx->emit(OP_JUMP_IF_FALSE_OR_POP, BC_NOARG, line);
        rhs->emit(ctx);
        ctx->patch_jump(patch);
    }
};

struct OrExpr: Expr{
    Expr_ lhs;
    Expr_ rhs;
    std::string str() const override { return "Or()"; }

    void emit(CodeEmitContext* ctx) override {
        lhs->emit(ctx);
        int patch = ctx->emit(OP_JUMP_IF_TRUE_OR_POP, BC_NOARG, line);
        rhs->emit(ctx);
        ctx->patch_jump(patch);
    }
};

// [None, True, False, ...]
struct Literal0Expr: Expr{
    TokenIndex token;
    Literal0Expr(TokenIndex token): token(token) {}
    std::string str() const override { return TK_STR(token); }

    void emit(CodeEmitContext* ctx) override {
        switch (token) {
            case TK("None"):    ctx->emit(OP_LOAD_NONE, BC_NOARG, line); break;
            case TK("True"):    ctx->emit(OP_LOAD_TRUE, BC_NOARG, line); break;
            case TK("False"):   ctx->emit(OP_LOAD_FALSE, BC_NOARG, line); break;
            case TK("..."):     ctx->emit(OP_LOAD_ELLIPSIS, BC_NOARG, line); break;
            default: FATAL_ERROR();
        }
    }

    bool is_json_object() const override { return true; }
};

// @num, @str which needs to invoke OP_LOAD_CONST
struct LiteralExpr: Expr{
    TokenValue value;
    LiteralExpr(TokenValue value): value(value) {}
    std::string str() const override {
        if(std::holds_alternative<i64>(value)){
            return std::to_string(std::get<i64>(value));
        }
        if(std::holds_alternative<f64>(value)){
            return std::to_string(std::get<f64>(value));
        }
        if(std::holds_alternative<Str>(value)){
            Str s = std::get<Str>(value).escape();
            return s.str();
        }
        FATAL_ERROR();
    }

    void emit(CodeEmitContext* ctx) override {
        VM* vm = ctx->vm;
        PyObject* obj = nullptr;
        if(std::holds_alternative<i64>(value)){
            i64 _val = std::get<i64>(value);
            if(_val >= INT16_MIN && _val <= INT16_MAX){
                ctx->emit(OP_LOAD_INTEGER, (int)_val, line);
                return;
            }
            obj = VAR(_val);
        }
        if(std::holds_alternative<f64>(value)){
            obj = VAR(std::get<f64>(value));
        }
        if(std::holds_alternative<Str>(value)){
            obj = VAR(std::get<Str>(value));
        }
        if(obj == nullptr) FATAL_ERROR();
        ctx->emit(OP_LOAD_CONST, ctx->add_const(obj), line);
    }

    bool is_literal() const override { return true; }
    bool is_json_object() const override { return true; }
};

struct NegatedExpr: Expr{
    Expr_ child;
    NegatedExpr(Expr_&& child): child(std::move(child)) {}
    std::string str() const override { return "Negated()"; }

    void emit(CodeEmitContext* ctx) override {
        VM* vm = ctx->vm;
        // if child is a int of float, do constant folding
        if(child->is_literal()){
            LiteralExpr* lit = static_cast<LiteralExpr*>(child.get());
            if(std::holds_alternative<i64>(lit->value)){
                i64 _val = -std::get<i64>(lit->value);
                if(_val >= INT16_MIN && _val <= INT16_MAX){
                    ctx->emit(OP_LOAD_INTEGER, (int)_val, line);
                }else{
                    ctx->emit(OP_LOAD_CONST, ctx->add_const(VAR(_val)), line);
                }
                return;
            }
            if(std::holds_alternative<f64>(lit->value)){
                PyObject* obj = VAR(-std::get<f64>(lit->value));
                ctx->emit(OP_LOAD_CONST, ctx->add_const(obj), line);
                return;
            }
        }
        child->emit(ctx);
        ctx->emit(OP_UNARY_NEGATIVE, BC_NOARG, line);
    }

    bool is_json_object() const override {
        return child->is_literal();
    }
};

struct SliceExpr: Expr{
    Expr_ start;
    Expr_ stop;
    Expr_ step;
    std::string str() const override { return "Slice()"; }

    void emit(CodeEmitContext* ctx) override {
        if(start){
            start->emit(ctx);
        }else{
            ctx->emit(OP_LOAD_NONE, BC_NOARG, line);
        }

        if(stop){
            stop->emit(ctx);
        }else{
            ctx->emit(OP_LOAD_NONE, BC_NOARG, line);
        }

        if(step){
            step->emit(ctx);
        }else{
            ctx->emit(OP_LOAD_NONE, BC_NOARG, line);
        }

        ctx->emit(OP_BUILD_SLICE, BC_NOARG, line);
    }
};

struct DictItemExpr: Expr{
    Expr_ key;      // maybe nullptr if it is **kwargs
    Expr_ value;
    std::string str() const override { return "DictItem()"; }

    int star_level() const override { return value->star_level(); }

    void emit(CodeEmitContext* ctx) override {
        if(is_starred()){
            PK_ASSERT(key == nullptr);
            value->emit(ctx);
        }else{
            value->emit(ctx);
            key->emit(ctx);     // reverse order
            ctx->emit(OP_BUILD_TUPLE, 2, line);
        }
    }
};

struct SequenceExpr: Expr{
    std::vector<Expr_> items;
    SequenceExpr(std::vector<Expr_>&& items): items(std::move(items)) {}
    virtual Opcode opcode() const = 0;

    void emit(CodeEmitContext* ctx) override {
        for(auto& item: items) item->emit(ctx);
        ctx->emit(opcode(), items.size(), line);
    }
};

struct ListExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    std::string str() const override { return "List()"; }

    Opcode opcode() const override {
        for(auto& e: items) if(e->is_starred()) return OP_BUILD_LIST_UNPACK;
        return OP_BUILD_LIST;
    }

    bool is_json_object() const override { return true; }
};

struct DictExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    std::string str() const override { return "Dict()"; }
    Opcode opcode() const override {
        for(auto& e: items) if(e->is_starred()) return OP_BUILD_DICT_UNPACK;
        return OP_BUILD_DICT;
    }

    bool is_json_object() const override { return true; }
};

struct SetExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    std::string str() const override { return "Set()"; }
    Opcode opcode() const override {
        for(auto& e: items) if(e->is_starred()) return OP_BUILD_SET_UNPACK;
        return OP_BUILD_SET;
    }
};

struct TupleExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    std::string str() const override { return "Tuple()"; }
    Opcode opcode() const override {
        for(auto& e: items) if(e->is_starred()) return OP_BUILD_TUPLE_UNPACK;
        return OP_BUILD_TUPLE;
    }

    bool emit_store(CodeEmitContext* ctx) override {
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
                ctx->emit(OP_UNPACK_SEQUENCE, items.size(), line);
            }
        }else{
            // starred assignment target must be in a tuple
            if(items.size() == 1) return false;
            // starred assignment target must be the last one (differ from cpython)
            if(starred_i != items.size()-1) return false;
            // a,*b = [1,2,3]
            // stack is [1,2,3] -> [1,[2,3]]
            ctx->emit(OP_UNPACK_EX, items.size()-1, line);
        }
        // do reverse emit
        for(int i=items.size()-1; i>=0; i--){
            bool ok = items[i]->emit_store(ctx);
            if(!ok) return false;
        }
        return true;
    }

    bool emit_del(CodeEmitContext* ctx) override{
        for(auto& e: items){
            bool ok = e->emit_del(ctx);
            if(!ok) return false;
        }
        return true;
    }
};

struct CompExpr: Expr{
    Expr_ expr;       // loop expr
    Expr_ vars;       // loop vars
    Expr_ iter;       // loop iter
    Expr_ cond;       // optional if condition

    virtual Opcode op0() = 0;
    virtual Opcode op1() = 0;

    void emit(CodeEmitContext* ctx){
        ctx->emit(op0(), 0, line);
        iter->emit(ctx);
        ctx->emit(OP_GET_ITER, BC_NOARG, BC_KEEPLINE);
        ctx->enter_block(FOR_LOOP);
        ctx->emit(OP_FOR_ITER, BC_NOARG, BC_KEEPLINE);
        bool ok = vars->emit_store(ctx);
        // this error occurs in `vars` instead of this line, but...nevermind
        PK_ASSERT(ok);  // TODO: raise a SyntaxError instead
        if(cond){
            cond->emit(ctx);
            int patch = ctx->emit(OP_POP_JUMP_IF_FALSE, BC_NOARG, BC_KEEPLINE);
            expr->emit(ctx);
            ctx->emit(op1(), BC_NOARG, BC_KEEPLINE);
            ctx->patch_jump(patch);
        }else{
            expr->emit(ctx);
            ctx->emit(op1(), BC_NOARG, BC_KEEPLINE);
        }
        ctx->emit(OP_LOOP_CONTINUE, BC_NOARG, BC_KEEPLINE);
        ctx->exit_block();
    }
};

struct ListCompExpr: CompExpr{
    Opcode op0() override { return OP_BUILD_LIST; }
    Opcode op1() override { return OP_LIST_APPEND; }
    std::string str() const override { return "ListComp()"; }
};

struct DictCompExpr: CompExpr{
    Opcode op0() override { return OP_BUILD_DICT; }
    Opcode op1() override { return OP_DICT_ADD; }
    std::string str() const override { return "DictComp()"; }
};

struct SetCompExpr: CompExpr{
    Opcode op0() override { return OP_BUILD_SET; }
    Opcode op1() override { return OP_SET_ADD; }
    std::string str() const override { return "SetComp()"; }
};

struct LambdaExpr: Expr{
    FuncDecl_ decl;
    std::string str() const override { return "Lambda()"; }

    LambdaExpr(FuncDecl_ decl): decl(decl) {}

    void emit(CodeEmitContext* ctx) override {
        int index = ctx->add_func_decl(decl);
        ctx->emit(OP_LOAD_FUNCTION, index, line);
    }
};

struct FStringExpr: Expr{
    Str src;
    FStringExpr(const Str& src): src(src) {}
    std::string str() const override {
        return fmt("f", src.escape());
    }

    void _load_simple_expr(CodeEmitContext* ctx, Str expr){
        // TODO: pre compile this into a function
        int dot = expr.index(".");
        if(dot < 0){
            ctx->emit(OP_LOAD_NAME, StrName(expr.sv()).index, line);
        }else{
            StrName name(expr.substr(0, dot).sv());
            StrName attr(expr.substr(dot+1).sv());
            ctx->emit(OP_LOAD_NAME, name.index, line);
            ctx->emit(OP_LOAD_ATTR, attr.index, line);
        }
    }

    void emit(CodeEmitContext* ctx) override {
        VM* vm = ctx->vm;
        static const std::regex pattern(R"(\{(.*?)\})");
        std::cregex_iterator begin(src.begin(), src.end(), pattern);
        std::cregex_iterator end;
        int size = 0;
        int i = 0;
        for(auto it = begin; it != end; it++) {
            std::cmatch m = *it;
            if (i < m.position()) {
                Str literal = src.substr(i, m.position() - i);
                ctx->emit(OP_LOAD_CONST, ctx->add_const(VAR(literal)), line);
                size++;
            }
            Str expr = m[1].str();
            int conon = expr.index(":");
            if(conon >= 0){
                _load_simple_expr(ctx, expr.substr(0, conon));
                Str spec = expr.substr(conon+1);
                ctx->emit(OP_FORMAT_STRING, ctx->add_const(VAR(spec)), line);
            }else{
                _load_simple_expr(ctx, expr);
            }
            size++;
            i = (int)(m.position() + m.length());
        }
        if (i < src.length()) {
            Str literal = src.substr(i, src.length() - i);
            ctx->emit(OP_LOAD_CONST, ctx->add_const(VAR(literal)), line);
            size++;
        }
        ctx->emit(OP_BUILD_STRING, size, line);
    }
};

struct SubscrExpr: Expr{
    Expr_ a;
    Expr_ b;
    std::string str() const override { return "Subscr()"; }

    void emit(CodeEmitContext* ctx) override{
        a->emit(ctx);
        b->emit(ctx);
        ctx->emit(OP_LOAD_SUBSCR, BC_NOARG, line);
    }

    bool emit_del(CodeEmitContext* ctx) override {
        a->emit(ctx);
        b->emit(ctx);
        ctx->emit(OP_DELETE_SUBSCR, BC_NOARG, line);
        return true;
    }

    bool emit_store(CodeEmitContext* ctx) override {
        a->emit(ctx);
        b->emit(ctx);
        ctx->emit(OP_STORE_SUBSCR, BC_NOARG, line);
        return true;
    }
};

struct AttribExpr: Expr{
    Expr_ a;
    Str b;
    AttribExpr(Expr_ a, const Str& b): a(std::move(a)), b(b) {}
    AttribExpr(Expr_ a, Str&& b): a(std::move(a)), b(std::move(b)) {}
    std::string str() const override { return "Attrib()"; }

    void emit(CodeEmitContext* ctx) override{
        a->emit(ctx);
        int index = StrName(b).index;
        ctx->emit(OP_LOAD_ATTR, index, line);
    }

    bool emit_del(CodeEmitContext* ctx) override {
        a->emit(ctx);
        int index = StrName(b).index;
        ctx->emit(OP_DELETE_ATTR, index, line);
        return true;
    }

    bool emit_store(CodeEmitContext* ctx) override {
        a->emit(ctx);
        int index = StrName(b).index;
        ctx->emit(OP_STORE_ATTR, index, line);
        return true;
    }

    void emit_method(CodeEmitContext* ctx) {
        a->emit(ctx);
        int index = StrName(b).index;
        ctx->emit(OP_LOAD_METHOD, index, line);
    }

    bool is_attrib() const override { return true; }
};

struct CallExpr: Expr{
    Expr_ callable;
    std::vector<Expr_> args;
    // **a will be interpreted as a special keyword argument: {"**": a}
    std::vector<std::pair<Str, Expr_>> kwargs;
    std::string str() const override { return "Call()"; }

    void emit(CodeEmitContext* ctx) override {
        bool vargs = false;
        bool vkwargs = false;
        for(auto& arg: args) if(arg->is_starred()) vargs = true;
        for(auto& item: kwargs) if(item.second->is_starred()) vkwargs = true;

        // if callable is a AttrExpr, we should try to use `fast_call` instead of use `boundmethod` proxy
        if(callable->is_attrib()){
            auto p = static_cast<AttribExpr*>(callable.get());
            p->emit_method(ctx);    // OP_LOAD_METHOD
        }else{
            callable->emit(ctx);
            ctx->emit(OP_LOAD_NULL, BC_NOARG, BC_KEEPLINE);
        }

        if(vargs || vkwargs){
            for(auto& item: args) item->emit(ctx);
            ctx->emit(OP_BUILD_TUPLE_UNPACK, (int)args.size(), line);

            if(!kwargs.empty()){
                for(auto& item: kwargs){
                    if(item.second->is_starred()){
                        if(item.second->star_level() != 2) FATAL_ERROR();
                        item.second->emit(ctx);
                    }else{
                        // k=v
                        int index = ctx->add_const(py_var(ctx->vm, item.first));
                        ctx->emit(OP_LOAD_CONST, index, line);
                        item.second->emit(ctx);
                        ctx->emit(OP_BUILD_TUPLE, 2, line);
                    }
                }
                ctx->emit(OP_BUILD_DICT_UNPACK, (int)kwargs.size(), line);
                ctx->emit(OP_CALL_TP, 1, line);
            }else{
                ctx->emit(OP_CALL_TP, 0, line);
            }
        }else{
            // vectorcall protocal
            for(auto& item: args) item->emit(ctx);
            for(auto& item: kwargs){
                int index = StrName(item.first.sv()).index;
                ctx->emit(OP_LOAD_INTEGER, index, line);
                item.second->emit(ctx);
            }
            int KWARGC = (int)kwargs.size();
            int ARGC = (int)args.size();
            ctx->emit(OP_CALL, (KWARGC<<16)|ARGC, line);
        }
    }
};

struct BinaryExpr: Expr{
    TokenIndex op;
    Expr_ lhs;
    Expr_ rhs;
    std::string str() const override { return TK_STR(op); }

    bool is_compare() const override {
        switch(op){
            case TK("<"): case TK("<="): case TK("=="):
            case TK("!="): case TK(">"): case TK(">="): return true;
            default: return false;
        }
    }

    void _emit_compare(CodeEmitContext* ctx, std::vector<int>& jmps){
        if(lhs->is_compare()){
            static_cast<BinaryExpr*>(lhs.get())->_emit_compare(ctx, jmps);
        }else{
            lhs->emit(ctx); // [a]
        }
        rhs->emit(ctx); // [a, b]
        ctx->emit(OP_DUP_TOP, BC_NOARG, line);      // [a, b, b]
        ctx->emit(OP_ROT_THREE, BC_NOARG, line);    // [b, a, b]
        switch(op){
            case TK("<"):   ctx->emit(OP_COMPARE_LT, BC_NOARG, line);  break;
            case TK("<="):  ctx->emit(OP_COMPARE_LE, BC_NOARG, line);  break;
            case TK("=="):  ctx->emit(OP_COMPARE_EQ, BC_NOARG, line);  break;
            case TK("!="):  ctx->emit(OP_COMPARE_NE, BC_NOARG, line);  break;
            case TK(">"):   ctx->emit(OP_COMPARE_GT, BC_NOARG, line);  break;
            case TK(">="):  ctx->emit(OP_COMPARE_GE, BC_NOARG, line);  break;
            default: UNREACHABLE();
        }
        // [b, RES]
        int index = ctx->emit(OP_SHORTCUT_IF_FALSE_OR_POP, BC_NOARG, line);
        jmps.push_back(index);
    }

    void emit(CodeEmitContext* ctx) override {
        std::vector<int> jmps;
        if(is_compare() && lhs->is_compare()){
            // (a < b) < c
            static_cast<BinaryExpr*>(lhs.get())->_emit_compare(ctx, jmps);
            // [b, RES]
        }else{
            // (1 + 2) < c
            lhs->emit(ctx);
        }

        rhs->emit(ctx);
        switch (op) {
            case TK("+"):   ctx->emit(OP_BINARY_ADD, BC_NOARG, line);  break;
            case TK("-"):   ctx->emit(OP_BINARY_SUB, BC_NOARG, line);  break;
            case TK("*"):   ctx->emit(OP_BINARY_MUL, BC_NOARG, line);  break;
            case TK("/"):   ctx->emit(OP_BINARY_TRUEDIV, BC_NOARG, line);  break;
            case TK("//"):  ctx->emit(OP_BINARY_FLOORDIV, BC_NOARG, line);  break;
            case TK("%"):   ctx->emit(OP_BINARY_MOD, BC_NOARG, line);  break;
            case TK("**"):  ctx->emit(OP_BINARY_POW, BC_NOARG, line);  break;

            case TK("<"):   ctx->emit(OP_COMPARE_LT, BC_NOARG, line);  break;
            case TK("<="):  ctx->emit(OP_COMPARE_LE, BC_NOARG, line);  break;
            case TK("=="):  ctx->emit(OP_COMPARE_EQ, BC_NOARG, line);  break;
            case TK("!="):  ctx->emit(OP_COMPARE_NE, BC_NOARG, line);  break;
            case TK(">"):   ctx->emit(OP_COMPARE_GT, BC_NOARG, line);  break;
            case TK(">="):  ctx->emit(OP_COMPARE_GE, BC_NOARG, line);  break;

            case TK("in"):      ctx->emit(OP_CONTAINS_OP, 0, line);   break;
            case TK("not in"):  ctx->emit(OP_CONTAINS_OP, 1, line);   break;
            case TK("is"):      ctx->emit(OP_IS_OP, 0, line);         break;
            case TK("is not"):  ctx->emit(OP_IS_OP, 1, line);         break;

            case TK("<<"):  ctx->emit(OP_BITWISE_LSHIFT, BC_NOARG, line);  break;
            case TK(">>"):  ctx->emit(OP_BITWISE_RSHIFT, BC_NOARG, line);  break;
            case TK("&"):   ctx->emit(OP_BITWISE_AND, BC_NOARG, line);  break;
            case TK("|"):   ctx->emit(OP_BITWISE_OR, BC_NOARG, line);  break;
            case TK("^"):   ctx->emit(OP_BITWISE_XOR, BC_NOARG, line);  break;

            case TK("@"):   ctx->emit(OP_BINARY_MATMUL, BC_NOARG, line);  break;
            default: FATAL_ERROR();
        }

        for(int i: jmps) ctx->patch_jump(i);
    }
};


struct TernaryExpr: Expr{
    Expr_ cond;
    Expr_ true_expr;
    Expr_ false_expr;
    std::string str() const override { return "Ternary()"; }

    void emit(CodeEmitContext* ctx) override {
        cond->emit(ctx);
        int patch = ctx->emit(OP_POP_JUMP_IF_FALSE, BC_NOARG, cond->line);
        true_expr->emit(ctx);
        int patch_2 = ctx->emit(OP_JUMP_ABSOLUTE, BC_NOARG, true_expr->line);
        ctx->patch_jump(patch);
        false_expr->emit(ctx);
        ctx->patch_jump(patch_2);
    }
};


} // namespace pkpy


namespace pkpy{

class Compiler;
typedef void (Compiler::*PrattCallback)();

struct PrattRule{
    PrattCallback prefix;
    PrattCallback infix;
    Precedence precedence;
};

class Compiler {
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
    NameScope name_scope() const {
        auto s = contexts.size()>1 ? NAME_LOCAL : NAME_GLOBAL;
        if(unknown_global_scope && s == NAME_GLOBAL) s = NAME_GLOBAL_UNKNOWN;
        return s;
    }

    CodeObject_ push_global_context(){
        CodeObject_ co = make_sp<CodeObject>(lexer->src, lexer->src->filename);
        contexts.push(CodeEmitContext(vm, co, contexts.size()));
        return co;
    }

    FuncDecl_ push_f_context(Str name){
        FuncDecl_ decl = make_sp<FuncDecl>();
        decl->code = make_sp<CodeObject>(lexer->src, name);
        decl->nested = name_scope() == NAME_LOCAL;
        contexts.push(CodeEmitContext(vm, decl->code, contexts.size()));
        return decl;
    }

    void pop_context(){
        if(!ctx()->s_expr.empty()){
            throw std::runtime_error("!ctx()->s_expr.empty()\n" + ctx()->_log_s_expr());
        }
        // add a `return None` in the end as a guard
        // previously, we only do this if the last opcode is not a return
        // however, this is buggy...since there may be a jump to the end (out of bound) even if the last opcode is a return
        ctx()->emit(OP_LOAD_NONE, BC_NOARG, BC_KEEPLINE);
        ctx()->emit(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
        // ctx()->co->optimize(vm);
        if(ctx()->co->varnames.size() > PK_MAX_CO_VARNAMES){
            SyntaxError("maximum number of local variables exceeded");
        }
        contexts.pop();
    }

    static void init_pratt_rules(){
        if(rules[TK(".")].precedence != PREC_NONE) return;
// http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
#define METHOD(name) &Compiler::name
#define NO_INFIX nullptr, PREC_NONE
        for(TokenIndex i=0; i<kTokenCount; i++) rules[i] = { nullptr, NO_INFIX };
        rules[TK(".")] =        { nullptr,               METHOD(exprAttrib),         PREC_ATTRIB };
        rules[TK("(")] =        { METHOD(exprGroup),     METHOD(exprCall),           PREC_CALL };
        rules[TK("[")] =        { METHOD(exprList),      METHOD(exprSubscr),         PREC_SUBSCRIPT };
        rules[TK("{")] =        { METHOD(exprMap),       NO_INFIX };
        rules[TK("%")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("+")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_TERM };
        rules[TK("-")] =        { METHOD(exprUnaryOp),   METHOD(exprBinaryOp),       PREC_TERM };
        rules[TK("*")] =        { METHOD(exprUnaryOp),   METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("/")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("//")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("**")] =       { METHOD(exprUnaryOp),   METHOD(exprBinaryOp),       PREC_EXPONENT };
        rules[TK(">")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("<")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("==")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("!=")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK(">=")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("<=")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("in")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("is")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("<<")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK(">>")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK("&")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_AND };
        rules[TK("|")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_OR };
        rules[TK("^")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_XOR };
        rules[TK("@")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("if")] =       { nullptr,               METHOD(exprTernary),        PREC_TERNARY };
        rules[TK(",")] =        { nullptr,               METHOD(exprTuple),          PREC_TUPLE };
        rules[TK("not in")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("is not")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("and") ] =     { nullptr,               METHOD(exprAnd),            PREC_LOGICAL_AND };
        rules[TK("or")] =       { nullptr,               METHOD(exprOr),             PREC_LOGICAL_OR };
        rules[TK("not")] =      { METHOD(exprNot),       nullptr,                    PREC_LOGICAL_NOT };
        rules[TK("True")] =     { METHOD(exprLiteral0),  NO_INFIX };
        rules[TK("False")] =    { METHOD(exprLiteral0),  NO_INFIX };
        rules[TK("None")] =     { METHOD(exprLiteral0),  NO_INFIX };
        rules[TK("...")] =      { METHOD(exprLiteral0),  NO_INFIX };
        rules[TK("lambda")] =   { METHOD(exprLambda),    NO_INFIX };
        rules[TK("@id")] =      { METHOD(exprName),      NO_INFIX };
        rules[TK("@num")] =     { METHOD(exprLiteral),   NO_INFIX };
        rules[TK("@str")] =     { METHOD(exprLiteral),   NO_INFIX };
        rules[TK("@fstr")] =    { METHOD(exprFString),   NO_INFIX };
#undef METHOD
#undef NO_INFIX
    }

    bool match(TokenIndex expected) {
        if (curr().type != expected) return false;
        advance();
        return true;
    }

    void consume(TokenIndex expected) {
        if (!match(expected)){
            SyntaxError(
                fmt("expected '", TK_STR(expected), "', but got '", TK_STR(curr().type), "'")
            );
        }
    }

    bool match_newlines_repl(){
        return match_newlines(mode()==REPL_MODE);
    }

    bool match_newlines(bool repl_throw=false) {
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

    bool match_end_stmt() {
        if (match(TK(";"))) { match_newlines(); return true; }
        if (match_newlines() || curr().type == TK("@eof")) return true;
        if (curr().type == TK("@dedent")) return true;
        return false;
    }

    void consume_end_stmt() {
        if (!match_end_stmt()) SyntaxError("expected statement end");
    }

    /*************************************************/

    void EXPR(bool push_stack=true) {
        parse_expression(PREC_TUPLE+1, push_stack);
    }

    void EXPR_TUPLE(bool push_stack=true) {
        parse_expression(PREC_TUPLE, push_stack);
    }

    // special case for `for loop` and `comp`
    Expr_ EXPR_VARS(){
        std::vector<Expr_> items;
        do {
            consume(TK("@id"));
            items.push_back(make_expr<NameExpr>(prev().str(), name_scope()));
        } while(match(TK(",")));
        if(items.size()==1) return std::move(items[0]);
        return make_expr<TupleExpr>(std::move(items));
    }

    template <typename T, typename... Args>
    std::unique_ptr<T> make_expr(Args&&... args) {
        std::unique_ptr<T> expr = std::make_unique<T>(std::forward<Args>(args)...);
        expr->line = prev().line;
        return expr;
    }

    
    void exprLiteral(){
        ctx()->s_expr.push(make_expr<LiteralExpr>(prev().value));
    }

    void exprFString(){
        ctx()->s_expr.push(make_expr<FStringExpr>(std::get<Str>(prev().value)));
    }

    void exprLambda(){
        FuncDecl_ decl = push_f_context("<lambda>");
        auto e = make_expr<LambdaExpr>(decl);
        if(!match(TK(":"))){
            _compile_f_args(e->decl, false);
            consume(TK(":"));
        }
        // https://github.com/blueloveTH/pocketpy/issues/37
        parse_expression(PREC_LAMBDA + 1, false);
        ctx()->emit(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
        pop_context();
        ctx()->s_expr.push(std::move(e));
    }

    void exprTuple(){
        std::vector<Expr_> items;
        items.push_back(ctx()->s_expr.popx());
        do {
            EXPR();         // NOTE: "1," will fail, "1,2" will be ok
            items.push_back(ctx()->s_expr.popx());
        } while(match(TK(",")));
        ctx()->s_expr.push(make_expr<TupleExpr>(
            std::move(items)
        ));
    }

    void exprOr(){
        auto e = make_expr<OrExpr>();
        e->lhs = ctx()->s_expr.popx();
        parse_expression(PREC_LOGICAL_OR + 1);
        e->rhs = ctx()->s_expr.popx();
        ctx()->s_expr.push(std::move(e));
    }
    
    void exprAnd(){
        auto e = make_expr<AndExpr>();
        e->lhs = ctx()->s_expr.popx();
        parse_expression(PREC_LOGICAL_AND + 1);
        e->rhs = ctx()->s_expr.popx();
        ctx()->s_expr.push(std::move(e));
    }
    
    void exprTernary(){
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
    
    void exprBinaryOp(){
        auto e = make_expr<BinaryExpr>();
        e->op = prev().type;
        e->lhs = ctx()->s_expr.popx();
        parse_expression(rules[e->op].precedence + 1);
        e->rhs = ctx()->s_expr.popx();
        ctx()->s_expr.push(std::move(e));
    }

    void exprNot() {
        parse_expression(PREC_LOGICAL_NOT + 1);
        ctx()->s_expr.push(make_expr<NotExpr>(ctx()->s_expr.popx()));
    }
    
    void exprUnaryOp(){
        TokenIndex op = prev().type;
        parse_expression(PREC_UNARY + 1);
        switch(op){
            case TK("-"):
                ctx()->s_expr.push(make_expr<NegatedExpr>(ctx()->s_expr.popx()));
                break;
            case TK("*"):
                ctx()->s_expr.push(make_expr<StarredExpr>(1, ctx()->s_expr.popx()));
                break;
            case TK("**"):
                ctx()->s_expr.push(make_expr<StarredExpr>(2, ctx()->s_expr.popx()));
                break;
            default: FATAL_ERROR();
        }
    }

    void exprGroup(){
        match_newlines_repl();
        EXPR_TUPLE();   // () is just for change precedence
        match_newlines_repl();
        consume(TK(")"));
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

    void exprList() {
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

    void exprMap() {
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

    void exprCall() {
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

    void exprName(){
        Str name = prev().str();
        NameScope scope = name_scope();
        if(ctx()->co->global_names.count(name)){
            scope = NAME_GLOBAL;
        }
        ctx()->s_expr.push(make_expr<NameExpr>(name, scope));
    }

    void exprAttrib() {
        consume(TK("@id"));
        ctx()->s_expr.push(
            make_expr<AttribExpr>(ctx()->s_expr.popx(), prev().str())
        );
    }
    
    void exprSubscr() {
        auto e = make_expr<SubscrExpr>();
        e->a = ctx()->s_expr.popx();
        auto slice = make_expr<SliceExpr>();
        bool is_slice = false;
        // a[<0> <state:1> : state<3> : state<5>]
        int state = 0;
        do{
            switch(state){
                case 0:
                    if(match(TK(":"))){
                        is_slice=true;
                        state=2;
                        break;
                    }
                    if(match(TK("]"))) SyntaxError();
                    EXPR_TUPLE();
                    slice->start = ctx()->s_expr.popx();
                    state=1;
                    break;
                case 1:
                    if(match(TK(":"))){
                        is_slice=true;
                        state=2;
                        break;
                    }
                    if(match(TK("]"))) goto __SUBSCR_END;
                    SyntaxError("expected ':' or ']'");
                    break;
                case 2:
                    if(match(TK(":"))){
                        state=4;
                        break;
                    }
                    if(match(TK("]"))) goto __SUBSCR_END;
                    EXPR_TUPLE();
                    slice->stop = ctx()->s_expr.popx();
                    state=3;
                    break;
                case 3:
                    if(match(TK(":"))){
                        state=4;
                        break;
                    }
                    if(match(TK("]"))) goto __SUBSCR_END;
                    SyntaxError("expected ':' or ']'");
                    break;
                case 4:
                    if(match(TK("]"))) goto __SUBSCR_END;
                    EXPR_TUPLE();
                    slice->step = ctx()->s_expr.popx();
                    state=5;
                    break;
                case 5: consume(TK("]")); goto __SUBSCR_END;
            }
        }while(true);
__SUBSCR_END:
        if(is_slice){
            e->b = std::move(slice);
        }else{
            if(state != 1) FATAL_ERROR();
            e->b = std::move(slice->start);
        }
        ctx()->s_expr.push(std::move(e));
    }

    void exprLiteral0() {
        ctx()->s_expr.push(make_expr<Literal0Expr>(prev().type));
    }

    void compile_block_body() {
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
            compile_stmt();
            match_newlines();
        }
        consume(TK("@dedent"));
    }

    Str _compile_import() {
        if(name_scope() != NAME_GLOBAL) SyntaxError("import statement should be used in global scope");
        Opcode op = OP_IMPORT_NAME;
        if(match(TK("."))) op = OP_IMPORT_NAME_REL;
        consume(TK("@id"));
        Str name = prev().str();
        ctx()->emit(op, StrName(name).index, prev().line);
        return name;
    }

    // import a as b
    void compile_normal_import() {
        do {
            Str name = _compile_import();
            if (match(TK("as"))) {
                consume(TK("@id"));
                name = prev().str();
            }
            ctx()->emit(OP_STORE_GLOBAL, StrName(name).index, prev().line);
        } while (match(TK(",")));
        consume_end_stmt();
    }

    // from a import b as c, d as e
    void compile_from_import() {
        _compile_import();
        consume(TK("import"));
        if (match(TK("*"))) {
            ctx()->emit(OP_IMPORT_STAR, BC_NOARG, prev().line);
            consume_end_stmt();
            return;
        }
        do {
            ctx()->emit(OP_DUP_TOP, BC_NOARG, BC_KEEPLINE);
            consume(TK("@id"));
            Str name = prev().str();
            ctx()->emit(OP_LOAD_ATTR, StrName(name).index, prev().line);
            if (match(TK("as"))) {
                consume(TK("@id"));
                name = prev().str();
            }
            ctx()->emit(OP_STORE_GLOBAL, StrName(name).index, prev().line);
        } while (match(TK(",")));
        ctx()->emit(OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
        consume_end_stmt();
    }

    void parse_expression(int precedence, bool push_stack=true) {
        advance();
        PrattCallback prefix = rules[prev().type].prefix;
        if (prefix == nullptr) SyntaxError(Str("expected an expression, but got ") + TK_STR(prev().type));
        (this->*prefix)();
        while (rules[curr().type].precedence >= precedence) {
            TokenIndex op = curr().type;
            advance();
            PrattCallback infix = rules[op].infix;
            if(infix == nullptr) throw std::runtime_error("(infix == nullptr) is true");
            (this->*infix)();
        }
        if(!push_stack) ctx()->emit_expr();
    }

    void compile_if_stmt() {
        EXPR(false);   // condition
        int patch = ctx()->emit(OP_POP_JUMP_IF_FALSE, BC_NOARG, prev().line);
        compile_block_body();
        if (match(TK("elif"))) {
            int exit_patch = ctx()->emit(OP_JUMP_ABSOLUTE, BC_NOARG, prev().line);
            ctx()->patch_jump(patch);
            compile_if_stmt();
            ctx()->patch_jump(exit_patch);
        } else if (match(TK("else"))) {
            int exit_patch = ctx()->emit(OP_JUMP_ABSOLUTE, BC_NOARG, prev().line);
            ctx()->patch_jump(patch);
            compile_block_body();
            ctx()->patch_jump(exit_patch);
        } else {
            ctx()->patch_jump(patch);
        }
    }

    void compile_while_loop() {
        ctx()->enter_block(WHILE_LOOP);
        EXPR(false);   // condition
        int patch = ctx()->emit(OP_POP_JUMP_IF_FALSE, BC_NOARG, prev().line);
        compile_block_body();
        ctx()->emit(OP_LOOP_CONTINUE, BC_NOARG, BC_KEEPLINE);
        ctx()->patch_jump(patch);
        ctx()->exit_block();
    }

    void compile_for_loop() {
        Expr_ vars = EXPR_VARS();
        consume(TK("in"));
        EXPR_TUPLE(false);
        ctx()->emit(OP_GET_ITER, BC_NOARG, BC_KEEPLINE);
        ctx()->enter_block(FOR_LOOP);
        ctx()->emit(OP_FOR_ITER, BC_NOARG, BC_KEEPLINE);
        bool ok = vars->emit_store(ctx());
        if(!ok) SyntaxError();  // this error occurs in `vars` instead of this line, but...nevermind
        compile_block_body();
        ctx()->emit(OP_LOOP_CONTINUE, BC_NOARG, BC_KEEPLINE);
        ctx()->exit_block();
    }

    void compile_try_except() {
        ctx()->enter_block(TRY_EXCEPT);
        compile_block_body();
        std::vector<int> patches = {
            ctx()->emit(OP_JUMP_ABSOLUTE, BC_NOARG, BC_KEEPLINE)
        };
        ctx()->exit_block();
        do {
            consume(TK("except"));
            if(match(TK("@id"))){
                ctx()->emit(OP_EXCEPTION_MATCH, StrName(prev().str()).index, prev().line);
            }else{
                ctx()->emit(OP_LOAD_TRUE, BC_NOARG, BC_KEEPLINE);
            }
            int patch = ctx()->emit(OP_POP_JUMP_IF_FALSE, BC_NOARG, BC_KEEPLINE);
            // pop the exception on match
            ctx()->emit(OP_POP_EXCEPTION, BC_NOARG, BC_KEEPLINE);
            compile_block_body();
            patches.push_back(ctx()->emit(OP_JUMP_ABSOLUTE, BC_NOARG, BC_KEEPLINE));
            ctx()->patch_jump(patch);
        }while(curr().type == TK("except"));
        // no match, re-raise
        ctx()->emit(OP_RE_RAISE, BC_NOARG, BC_KEEPLINE);
        for (int patch : patches) ctx()->patch_jump(patch);
    }

    void compile_decorated(){
        std::vector<Expr_> decorators;
        do{
            EXPR();
            decorators.push_back(ctx()->s_expr.popx());
            if(!match_newlines_repl()) SyntaxError();
        }while(match(TK("@")));
        consume(TK("def"));
        compile_function(decorators);
    }

    bool try_compile_assignment(){
        switch (curr().type) {
            case TK("+="): case TK("-="): case TK("*="): case TK("/="): case TK("//="): case TK("%="):
            case TK("<<="): case TK(">>="): case TK("&="): case TK("|="): case TK("^="): {
                Expr* lhs_p = ctx()->s_expr.top().get();
                if(lhs_p->is_starred()) SyntaxError();
                if(ctx()->is_compiling_class) SyntaxError();
                advance();
                auto e = make_expr<BinaryExpr>();
                e->op = prev().type - 1; // -1 to remove =
                e->lhs = ctx()->s_expr.popx();
                EXPR_TUPLE();
                e->rhs = ctx()->s_expr.popx();
                if(e->is_starred()) SyntaxError();
                e->emit(ctx());
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
                val->emit(ctx());
                for(int i=1; i<n; i++) ctx()->emit(OP_DUP_TOP, BC_NOARG, BC_KEEPLINE);
                for(int i=0; i<n; i++){
                    auto e = ctx()->s_expr.popx();
                    if(e->is_starred()) SyntaxError();
                    bool ok = e->emit_store(ctx());
                    if(!ok) SyntaxError();
                }
            } return true;
            default: return false;
        }
    }

    void compile_stmt() {
        advance();
        int kw_line = prev().line;  // backup line number
        switch(prev().type){
            case TK("break"):
                if (!ctx()->is_curr_block_loop()) SyntaxError("'break' outside loop");
                ctx()->emit(OP_LOOP_BREAK, BC_NOARG, kw_line);
                consume_end_stmt();
                break;
            case TK("continue"):
                if (!ctx()->is_curr_block_loop()) SyntaxError("'continue' not properly in loop");
                ctx()->emit(OP_LOOP_CONTINUE, BC_NOARG, kw_line);
                consume_end_stmt();
                break;
            case TK("yield"): 
                if (contexts.size() <= 1) SyntaxError("'yield' outside function");
                EXPR_TUPLE(false);
                // if yield present, mark the function as generator
                ctx()->co->is_generator = true;
                ctx()->emit(OP_YIELD_VALUE, BC_NOARG, kw_line);
                consume_end_stmt();
                break;
            case TK("yield from"):
                if (contexts.size() <= 1) SyntaxError("'yield from' outside function");
                EXPR_TUPLE(false);
                // if yield from present, mark the function as generator
                ctx()->co->is_generator = true;
                ctx()->emit(OP_GET_ITER, BC_NOARG, kw_line);
                ctx()->enter_block(FOR_LOOP);
                ctx()->emit(OP_FOR_ITER, BC_NOARG, BC_KEEPLINE);
                ctx()->emit(OP_YIELD_VALUE, BC_NOARG, BC_KEEPLINE);
                ctx()->emit(OP_LOOP_CONTINUE, BC_NOARG, BC_KEEPLINE);
                ctx()->exit_block();
                consume_end_stmt();
                break;
            case TK("return"):
                if (contexts.size() <= 1) SyntaxError("'return' outside function");
                if(match_end_stmt()){
                    ctx()->emit(OP_LOAD_NONE, BC_NOARG, kw_line);
                }else{
                    EXPR_TUPLE(false);
                    consume_end_stmt();
                }
                ctx()->emit(OP_RETURN_VALUE, BC_NOARG, kw_line);
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
                switch(name_scope()){
                    case NAME_LOCAL:
                        ctx()->emit(OP_INC_FAST, ctx()->add_varname(name), prev().line);
                        break;
                    case NAME_GLOBAL:
                        ctx()->emit(OP_INC_GLOBAL, name.index, prev().line);
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
                        ctx()->emit(OP_DEC_FAST, ctx()->add_varname(name), prev().line);
                        break;
                    case NAME_GLOBAL:
                        ctx()->emit(OP_DEC_GLOBAL, name.index, prev().line);
                        break;
                    default: SyntaxError(); break;
                }
                consume_end_stmt();
                break;
            }
            case TK("assert"):
                EXPR_TUPLE(false);
                ctx()->emit(OP_ASSERT, BC_NOARG, kw_line);
                consume_end_stmt();
                break;
            case TK("global"):
                do {
                    consume(TK("@id"));
                    ctx()->co->global_names.insert(prev().str());
                } while (match(TK(",")));
                consume_end_stmt();
                break;
            case TK("raise"): {
                consume(TK("@id"));
                int dummy_t = StrName(prev().str()).index;
                if(match(TK("(")) && !match(TK(")"))){
                    EXPR(false); consume(TK(")"));
                }else{
                    ctx()->emit(OP_LOAD_NONE, BC_NOARG, kw_line);
                }
                ctx()->emit(OP_RAISE, dummy_t, kw_line);
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
                EXPR(false);
                consume(TK("as"));
                consume(TK("@id"));
                StrName name(prev().str());
                ctx()->emit(OP_STORE_NAME, name.index, prev().line);
                ctx()->emit(OP_LOAD_NAME, name.index, prev().line);
                ctx()->emit(OP_WITH_ENTER, BC_NOARG, prev().line);
                compile_block_body();
                ctx()->emit(OP_LOAD_NAME, name.index, prev().line);
                ctx()->emit(OP_WITH_EXIT, BC_NOARG, prev().line);
            } break;
            /*************************************************/
            case TK("$label"): {
                if(mode()!=EXEC_MODE) SyntaxError("'label' is only available in EXEC_MODE");
                consume(TK("@id"));
                bool ok = ctx()->add_label(prev().str());
                if(!ok) SyntaxError("label " + prev().str().escape() + " already exists");
                consume_end_stmt();
            } break;
            case TK("$goto"):
                if(mode()!=EXEC_MODE) SyntaxError("'goto' is only available in EXEC_MODE");
                consume(TK("@id"));
                ctx()->emit(OP_GOTO, StrName(prev().str()).index, prev().line);
                consume_end_stmt();
                break;
            /*************************************************/
            // handle dangling expression or assignment
            default: {
                advance(-1);    // do revert since we have pre-called advance() at the beginning
                EXPR_TUPLE();
                // eat variable's type hint
                if(match(TK(":"))) consume_type_hints();
                if(!try_compile_assignment()){
                    if(!ctx()->s_expr.empty() && ctx()->s_expr.top()->is_starred()){
                        SyntaxError();
                    }
                    ctx()->emit_expr();
                    if((mode()==CELL_MODE || mode()==REPL_MODE) && name_scope()==NAME_GLOBAL){
                        ctx()->emit(OP_PRINT_EXPR, BC_NOARG, BC_KEEPLINE);
                    }else{
                        ctx()->emit(OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
                    }
                }
                consume_end_stmt();
            }
        }
    }

    void consume_type_hints(){
        EXPR();
        ctx()->s_expr.pop();
    }

    void compile_class(){
        consume(TK("@id"));
        int namei = StrName(prev().str()).index;
        int super_namei = -1;
        if(match(TK("("))){
            if(match(TK("@id"))){
                super_namei = StrName(prev().str()).index;
            }
            consume(TK(")"));
        }
        if(super_namei == -1) ctx()->emit(OP_LOAD_NONE, BC_NOARG, prev().line);
        else ctx()->emit(OP_LOAD_GLOBAL, super_namei, prev().line);
        ctx()->emit(OP_BEGIN_CLASS, namei, BC_KEEPLINE);
        ctx()->is_compiling_class = true;
        compile_block_body();
        ctx()->is_compiling_class = false;
        ctx()->emit(OP_END_CLASS, BC_NOARG, BC_KEEPLINE);
    }

    void _compile_f_args(FuncDecl_ decl, bool enable_type_hints){
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
            for(int i: decl->args){
                if(decl->code->varnames[i] == name) {
                    SyntaxError("duplicate argument name");
                }
            }
            for(auto& kv: decl->kwargs){
                if(decl->code->varnames[kv.key] == name){
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
                        SyntaxError(Str("expect a literal, not ") + TK_STR(curr().type));
                    }
                    decl->kwargs.push_back(FuncDecl::KwArg{index, value});
                } break;
                case 3:
                    decl->starred_kwarg = index;
                    state+=1;
                    break;
            }
        } while (match(TK(",")));
    }

    void compile_function(const std::vector<Expr_>& decorators={}){
        Str decl_name;
        consume(TK("@id"));
        decl_name = prev().str();
        FuncDecl_ decl = push_f_context(decl_name);
        consume(TK("("));
        if (!match(TK(")"))) {
            _compile_f_args(decl, true);
            consume(TK(")"));
        }
        if(match(TK("->"))) consume_type_hints();
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
        ctx()->emit(OP_LOAD_FUNCTION, ctx()->add_func_decl(decl), prev().line);
        if(docstring != nullptr){
            ctx()->emit(OP_SETUP_DOCSTRING, ctx()->add_const(docstring), prev().line);
        }
        // add decorators
        for(auto it=decorators.rbegin(); it!=decorators.rend(); ++it){
            (*it)->emit(ctx());
            ctx()->emit(OP_ROT_TWO, BC_NOARG, (*it)->line);
            ctx()->emit(OP_LOAD_NULL, BC_NOARG, BC_KEEPLINE);
            ctx()->emit(OP_ROT_TWO, BC_NOARG, BC_KEEPLINE);
            ctx()->emit(OP_CALL, 1, (*it)->line);
        }
        if(!ctx()->is_compiling_class){
            auto e = make_expr<NameExpr>(decl_name, name_scope());
            e->emit_store(ctx());
        }else{
            int index = StrName(decl_name).index;
            ctx()->emit(OP_STORE_CLASS_ATTR, index, prev().line);
        }
    }

    PyObject* to_object(const TokenValue& value){
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
        if(obj == nullptr) FATAL_ERROR();
        return obj;
    }

    PyObject* read_literal(){
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
            default: break;
        }
        return nullptr;
    }

    void SyntaxError(Str msg){ lexer->throw_err("SyntaxError", msg, err().line, err().start); }
    void SyntaxError(){ lexer->throw_err("SyntaxError", "invalid syntax", err().line, err().start); }
    void IndentationError(Str msg){ lexer->throw_err("IndentationError", msg, err().line, err().start); }

public:
    Compiler(VM* vm, const Str& source, const Str& filename, CompileMode mode, bool unknown_global_scope=false){
        this->vm = vm;
        this->used = false;
        this->unknown_global_scope = unknown_global_scope;
        this->lexer = std::make_unique<Lexer>(
            make_sp<SourceData>(source, filename, mode)
        );
        init_pratt_rules();
    }

    CodeObject_ compile(){
        if(used) FATAL_ERROR();
        used = true;

        tokens = lexer->run();
        // if(lexer->src->filename == "<stdin>"){
        //     for(auto& t: tokens) std::cout << t.info() << std::endl;
        // }

        CodeObject_ code = push_global_context();

        advance();          // skip @sof, so prev() is always valid
        match_newlines();   // skip possible leading '\n'

        if(mode()==EVAL_MODE) {
            EXPR_TUPLE(false);
            consume(TK("@eof"));
            ctx()->emit(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
            pop_context();
            return code;
        }else if(mode()==JSON_MODE){
            EXPR();
            Expr_ e = ctx()->s_expr.popx();
            if(!e->is_json_object()) SyntaxError("expect a JSON object, literal or array");
            consume(TK("@eof"));
            e->emit(ctx());
            ctx()->emit(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
            pop_context();
            return code;
        }

        while (!match(TK("@eof"))) {
            if (match(TK("class"))) {
                compile_class();
            } else {
                compile_stmt();
            }
            match_newlines();
        }
        pop_context();
        return code;
    }
};

} // namespace pkpy


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace pkpy{

#ifdef _WIN32

inline std::string getline(bool* eof=nullptr) {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    std::wstringstream wss;
    WCHAR buf;
    DWORD read;
    while (ReadConsoleW(hStdin, &buf, 1, &read, NULL) && buf != L'\n') {
        if(eof && buf == L'\x1A') *eof = true;  // Ctrl+Z
        wss << buf;
    }
    std::wstring wideInput = wss.str();
    int length = WideCharToMultiByte(CP_UTF8, 0, wideInput.c_str(), (int)wideInput.length(), NULL, 0, NULL, NULL);
    std::string output;
    output.resize(length);
    WideCharToMultiByte(CP_UTF8, 0, wideInput.c_str(), (int)wideInput.length(), &output[0], length, NULL, NULL);
    if(!output.empty() && output.back() == '\r') output.pop_back();
    return output;
}

#else

inline std::string getline(bool* eof=nullptr){
    std::string line;
    if(!std::getline(std::cin, line)){
        if(eof) *eof = true;
    }
    return line;
}

#endif

class REPL {
protected:
    int need_more_lines = 0;
    std::string buffer;
    VM* vm;
public:
    REPL(VM* vm) : vm(vm){
        vm->_stdout(vm, "pocketpy " PK_VERSION " (" __DATE__ ", " __TIME__ ") ");
        vm->_stdout(vm, fmt("[", sizeof(void*)*8, " bit]" "\n"));
        vm->_stdout(vm, "https://github.com/blueloveTH/pocketpy" "\n");
        vm->_stdout(vm, "Type \"exit()\" to exit." "\n");
    }

    bool input(std::string line){
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
        }catch(NeedMoreLines& ne){
            buffer += line;
            buffer += '\n';
            need_more_lines = ne.is_compiling_class ? 3 : 2;
            if (need_more_lines) return true;
        }
        return false;
    }
};

} // namespace pkpy

// generated on 2023-06-11 09:55:58
#include <map>
#include <string>

namespace pkpy{
    inline static std::map<std::string, const char*> kPythonLibs = {
        {"_set", "\x63\x6c\x61\x73\x73\x20\x73\x65\x74\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x20\x3d\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x20\x6f\x72\x20\x5b\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x20\x3d\x20\x7b\x7d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x74\x65\x6d\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x61\x64\x64\x28\x69\x74\x65\x6d\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x61\x64\x64\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x65\x6c\x65\x6d\x5d\x20\x3d\x20\x4e\x6f\x6e\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x64\x69\x73\x63\x61\x72\x64\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x64\x65\x6c\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x65\x6c\x65\x6d\x5d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x72\x65\x6d\x6f\x76\x65\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x64\x65\x6c\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x65\x6c\x65\x6d\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6c\x65\x61\x72\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x63\x6c\x65\x61\x72\x28\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x75\x70\x64\x61\x74\x65\x28\x73\x65\x6c\x66\x2c\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x2e\x5f\x61\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6f\x70\x79\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x74\x28\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x6b\x65\x79\x73\x28\x29\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x61\x6e\x64\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x74\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6f\x72\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x6c\x66\x2e\x63\x6f\x70\x79\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x74\x28\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x6e\x6f\x74\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x78\x6f\x72\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x74\x28\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x6e\x6f\x74\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20" "\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x6e\x6f\x74\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x75\x6e\x69\x6f\x6e\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x7c\x20\x6f\x74\x68\x65\x72\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x6e\x74\x65\x72\x73\x65\x63\x74\x69\x6f\x6e\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x26\x20\x6f\x74\x68\x65\x72\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x64\x69\x66\x66\x65\x72\x65\x6e\x63\x65\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x2d\x20\x6f\x74\x68\x65\x72\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x73\x79\x6d\x6d\x65\x74\x72\x69\x63\x5f\x64\x69\x66\x66\x65\x72\x65\x6e\x63\x65\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x5e\x20\x6f\x74\x68\x65\x72\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x78\x6f\x72\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x73\x64\x69\x73\x6a\x6f\x69\x6e\x74\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x61\x6e\x64\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x73\x73\x75\x62\x73\x65\x74\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x73\x73\x75\x70\x65\x72\x73\x65\x74\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6f\x74\x68\x65\x72\x2e\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x63\x6f\x6e\x74\x61\x69\x6e\x73\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x73\x65\x74\x28\x29\x27\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x7b\x27\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x72\x65\x70\x72\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x6b\x65\x79\x73\x28\x29\x5d\x29\x20\x2b\x20\x27\x7d\x27\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x74\x65\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x74\x65\x72\x28\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x6b\x65\x79\x73\x28\x29\x29" },
        {"bisect", "\x22\x22\x22\x42\x69\x73\x65\x63\x74\x69\x6f\x6e\x20\x61\x6c\x67\x6f\x72\x69\x74\x68\x6d\x73\x2e\x22\x22\x22\x0a\x0a\x64\x65\x66\x20\x69\x6e\x73\x6f\x72\x74\x5f\x72\x69\x67\x68\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x3d\x30\x2c\x20\x68\x69\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x49\x6e\x73\x65\x72\x74\x20\x69\x74\x65\x6d\x20\x78\x20\x69\x6e\x20\x6c\x69\x73\x74\x20\x61\x2c\x20\x61\x6e\x64\x20\x6b\x65\x65\x70\x20\x69\x74\x20\x73\x6f\x72\x74\x65\x64\x20\x61\x73\x73\x75\x6d\x69\x6e\x67\x20\x61\x20\x69\x73\x20\x73\x6f\x72\x74\x65\x64\x2e\x0a\x0a\x20\x20\x20\x20\x49\x66\x20\x78\x20\x69\x73\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x69\x6e\x20\x61\x2c\x20\x69\x6e\x73\x65\x72\x74\x20\x69\x74\x20\x74\x6f\x20\x74\x68\x65\x20\x72\x69\x67\x68\x74\x20\x6f\x66\x20\x74\x68\x65\x20\x72\x69\x67\x68\x74\x6d\x6f\x73\x74\x20\x78\x2e\x0a\x0a\x20\x20\x20\x20\x4f\x70\x74\x69\x6f\x6e\x61\x6c\x20\x61\x72\x67\x73\x20\x6c\x6f\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x30\x29\x20\x61\x6e\x64\x20\x68\x69\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x6c\x65\x6e\x28\x61\x29\x29\x20\x62\x6f\x75\x6e\x64\x20\x74\x68\x65\x0a\x20\x20\x20\x20\x73\x6c\x69\x63\x65\x20\x6f\x66\x20\x61\x20\x74\x6f\x20\x62\x65\x20\x73\x65\x61\x72\x63\x68\x65\x64\x2e\x0a\x20\x20\x20\x20\x22\x22\x22\x0a\x0a\x20\x20\x20\x20\x6c\x6f\x20\x3d\x20\x62\x69\x73\x65\x63\x74\x5f\x72\x69\x67\x68\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x2c\x20\x68\x69\x29\x0a\x20\x20\x20\x20\x61\x2e\x69\x6e\x73\x65\x72\x74\x28\x6c\x6f\x2c\x20\x78\x29\x0a\x0a\x64\x65\x66\x20\x62\x69\x73\x65\x63\x74\x5f\x72\x69\x67\x68\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x3d\x30\x2c\x20\x68\x69\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x52\x65\x74\x75\x72\x6e\x20\x74\x68\x65\x20\x69\x6e\x64\x65\x78\x20\x77\x68\x65\x72\x65\x20\x74\x6f\x20\x69\x6e\x73\x65\x72\x74\x20\x69\x74\x65\x6d\x20\x78\x20\x69\x6e\x20\x6c\x69\x73\x74\x20\x61\x2c\x20\x61\x73\x73\x75\x6d\x69\x6e\x67\x20\x61\x20\x69\x73\x20\x73\x6f\x72\x74\x65\x64\x2e\x0a\x0a\x20\x20\x20\x20\x54\x68\x65\x20\x72\x65\x74\x75\x72\x6e\x20\x76\x61\x6c\x75\x65\x20\x69\x20\x69\x73\x20\x73\x75\x63\x68\x20\x74\x68\x61\x74\x20\x61\x6c\x6c\x20\x65\x20\x69\x6e\x20\x61\x5b\x3a\x69\x5d\x20\x68\x61\x76\x65\x20\x65\x20\x3c\x3d\x20\x78\x2c\x20\x61\x6e\x64\x20\x61\x6c\x6c\x20\x65\x20\x69\x6e\x0a\x20\x20\x20\x20\x61\x5b\x69\x3a\x5d\x20\x68\x61\x76\x65\x20\x65\x20\x3e\x20\x78\x2e\x20\x20\x53\x6f\x20\x69\x66\x20\x78\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x61\x70\x70\x65\x61\x72\x73\x20\x69\x6e\x20\x74\x68\x65\x20\x6c\x69\x73\x74\x2c\x20\x61\x2e\x69\x6e\x73\x65\x72\x74\x28\x78\x29\x20\x77\x69\x6c\x6c\x0a\x20\x20\x20\x20\x69\x6e\x73\x65\x72\x74\x20\x6a\x75\x73\x74\x20\x61\x66\x74\x65\x72\x20\x74\x68\x65\x20\x72\x69\x67\x68\x74\x6d\x6f\x73\x74\x20\x78\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x74\x68\x65\x72\x65\x2e\x0a\x0a\x20\x20\x20\x20\x4f\x70\x74\x69\x6f\x6e\x61\x6c\x20\x61\x72\x67\x73\x20\x6c\x6f\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x30\x29\x20\x61\x6e\x64\x20\x68\x69\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x6c\x65\x6e\x28\x61\x29\x29\x20\x62\x6f\x75\x6e\x64\x20\x74\x68\x65\x0a\x20\x20\x20\x20\x73\x6c\x69\x63\x65\x20\x6f\x66\x20\x61\x20\x74\x6f\x20\x62\x65\x20\x73\x65\x61\x72\x63\x68\x65\x64\x2e\x0a\x20\x20\x20\x20\x22\x22\x22\x0a\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x6f\x20\x3c\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x27\x6c\x6f\x20\x6d\x75\x73\x74\x20\x62\x65\x20\x6e\x6f\x6e\x2d\x6e\x65\x67\x61\x74\x69\x76\x65\x27\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x69\x20\x69\x73\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x69\x20\x3d\x20\x6c\x65\x6e\x28\x61\x29\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6c\x6f\x20\x3c\x20\x68\x69\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6d\x69\x64\x20\x3d\x20\x28\x6c\x6f\x2b\x68\x69\x29\x2f\x2f\x32\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x78\x20\x3c\x20\x61\x5b\x6d\x69\x64\x5d\x3a\x20\x68\x69\x20\x3d\x20\x6d\x69\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x20\x6c\x6f\x20\x3d\x20\x6d\x69\x64\x2b\x31\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x0a\x0a\x64\x65\x66\x20\x69\x6e\x73\x6f\x72\x74\x5f\x6c\x65\x66\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x3d\x30\x2c\x20\x68\x69\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x49\x6e\x73\x65\x72\x74\x20\x69\x74\x65\x6d\x20\x78\x20\x69\x6e\x20\x6c\x69\x73\x74\x20\x61\x2c\x20\x61\x6e\x64\x20\x6b\x65\x65\x70\x20\x69\x74\x20\x73\x6f\x72\x74\x65\x64\x20\x61\x73\x73\x75\x6d\x69\x6e\x67\x20\x61\x20\x69\x73\x20\x73\x6f\x72\x74\x65\x64\x2e\x0a\x0a\x20\x20\x20\x20\x49\x66\x20\x78\x20\x69\x73\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x69\x6e\x20\x61\x2c\x20\x69\x6e\x73\x65\x72\x74\x20\x69\x74\x20\x74\x6f\x20\x74\x68\x65\x20\x6c\x65\x66\x74\x20\x6f\x66\x20\x74\x68\x65\x20\x6c\x65\x66\x74\x6d\x6f\x73\x74\x20\x78\x2e\x0a\x0a\x20\x20\x20\x20\x4f\x70\x74\x69\x6f\x6e\x61\x6c\x20\x61\x72\x67\x73\x20\x6c\x6f\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x30\x29\x20\x61\x6e\x64\x20" "\x68\x69\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x6c\x65\x6e\x28\x61\x29\x29\x20\x62\x6f\x75\x6e\x64\x20\x74\x68\x65\x0a\x20\x20\x20\x20\x73\x6c\x69\x63\x65\x20\x6f\x66\x20\x61\x20\x74\x6f\x20\x62\x65\x20\x73\x65\x61\x72\x63\x68\x65\x64\x2e\x0a\x20\x20\x20\x20\x22\x22\x22\x0a\x0a\x20\x20\x20\x20\x6c\x6f\x20\x3d\x20\x62\x69\x73\x65\x63\x74\x5f\x6c\x65\x66\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x2c\x20\x68\x69\x29\x0a\x20\x20\x20\x20\x61\x2e\x69\x6e\x73\x65\x72\x74\x28\x6c\x6f\x2c\x20\x78\x29\x0a\x0a\x0a\x64\x65\x66\x20\x62\x69\x73\x65\x63\x74\x5f\x6c\x65\x66\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x3d\x30\x2c\x20\x68\x69\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x52\x65\x74\x75\x72\x6e\x20\x74\x68\x65\x20\x69\x6e\x64\x65\x78\x20\x77\x68\x65\x72\x65\x20\x74\x6f\x20\x69\x6e\x73\x65\x72\x74\x20\x69\x74\x65\x6d\x20\x78\x20\x69\x6e\x20\x6c\x69\x73\x74\x20\x61\x2c\x20\x61\x73\x73\x75\x6d\x69\x6e\x67\x20\x61\x20\x69\x73\x20\x73\x6f\x72\x74\x65\x64\x2e\x0a\x0a\x20\x20\x20\x20\x54\x68\x65\x20\x72\x65\x74\x75\x72\x6e\x20\x76\x61\x6c\x75\x65\x20\x69\x20\x69\x73\x20\x73\x75\x63\x68\x20\x74\x68\x61\x74\x20\x61\x6c\x6c\x20\x65\x20\x69\x6e\x20\x61\x5b\x3a\x69\x5d\x20\x68\x61\x76\x65\x20\x65\x20\x3c\x20\x78\x2c\x20\x61\x6e\x64\x20\x61\x6c\x6c\x20\x65\x20\x69\x6e\x0a\x20\x20\x20\x20\x61\x5b\x69\x3a\x5d\x20\x68\x61\x76\x65\x20\x65\x20\x3e\x3d\x20\x78\x2e\x20\x20\x53\x6f\x20\x69\x66\x20\x78\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x61\x70\x70\x65\x61\x72\x73\x20\x69\x6e\x20\x74\x68\x65\x20\x6c\x69\x73\x74\x2c\x20\x61\x2e\x69\x6e\x73\x65\x72\x74\x28\x78\x29\x20\x77\x69\x6c\x6c\x0a\x20\x20\x20\x20\x69\x6e\x73\x65\x72\x74\x20\x6a\x75\x73\x74\x20\x62\x65\x66\x6f\x72\x65\x20\x74\x68\x65\x20\x6c\x65\x66\x74\x6d\x6f\x73\x74\x20\x78\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x74\x68\x65\x72\x65\x2e\x0a\x0a\x20\x20\x20\x20\x4f\x70\x74\x69\x6f\x6e\x61\x6c\x20\x61\x72\x67\x73\x20\x6c\x6f\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x30\x29\x20\x61\x6e\x64\x20\x68\x69\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x6c\x65\x6e\x28\x61\x29\x29\x20\x62\x6f\x75\x6e\x64\x20\x74\x68\x65\x0a\x20\x20\x20\x20\x73\x6c\x69\x63\x65\x20\x6f\x66\x20\x61\x20\x74\x6f\x20\x62\x65\x20\x73\x65\x61\x72\x63\x68\x65\x64\x2e\x0a\x20\x20\x20\x20\x22\x22\x22\x0a\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x6f\x20\x3c\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x27\x6c\x6f\x20\x6d\x75\x73\x74\x20\x62\x65\x20\x6e\x6f\x6e\x2d\x6e\x65\x67\x61\x74\x69\x76\x65\x27\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x69\x20\x69\x73\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x69\x20\x3d\x20\x6c\x65\x6e\x28\x61\x29\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6c\x6f\x20\x3c\x20\x68\x69\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6d\x69\x64\x20\x3d\x20\x28\x6c\x6f\x2b\x68\x69\x29\x2f\x2f\x32\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x61\x5b\x6d\x69\x64\x5d\x20\x3c\x20\x78\x3a\x20\x6c\x6f\x20\x3d\x20\x6d\x69\x64\x2b\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x20\x68\x69\x20\x3d\x20\x6d\x69\x64\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x0a\x0a\x23\x20\x43\x72\x65\x61\x74\x65\x20\x61\x6c\x69\x61\x73\x65\x73\x0a\x62\x69\x73\x65\x63\x74\x20\x3d\x20\x62\x69\x73\x65\x63\x74\x5f\x72\x69\x67\x68\x74\x0a\x69\x6e\x73\x6f\x72\x74\x20\x3d\x20\x69\x6e\x73\x6f\x72\x74\x5f\x72\x69\x67\x68\x74\x0a" },
        {"heapq", "\x23\x20\x48\x65\x61\x70\x20\x71\x75\x65\x75\x65\x20\x61\x6c\x67\x6f\x72\x69\x74\x68\x6d\x20\x28\x61\x2e\x6b\x2e\x61\x2e\x20\x70\x72\x69\x6f\x72\x69\x74\x79\x20\x71\x75\x65\x75\x65\x29\x0a\x64\x65\x66\x20\x68\x65\x61\x70\x70\x75\x73\x68\x28\x68\x65\x61\x70\x2c\x20\x69\x74\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x50\x75\x73\x68\x20\x69\x74\x65\x6d\x20\x6f\x6e\x74\x6f\x20\x68\x65\x61\x70\x2c\x20\x6d\x61\x69\x6e\x74\x61\x69\x6e\x69\x6e\x67\x20\x74\x68\x65\x20\x68\x65\x61\x70\x20\x69\x6e\x76\x61\x72\x69\x61\x6e\x74\x2e\x22\x22\x22\x0a\x20\x20\x20\x20\x68\x65\x61\x70\x2e\x61\x70\x70\x65\x6e\x64\x28\x69\x74\x65\x6d\x29\x0a\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x64\x6f\x77\x6e\x28\x68\x65\x61\x70\x2c\x20\x30\x2c\x20\x6c\x65\x6e\x28\x68\x65\x61\x70\x29\x2d\x31\x29\x0a\x0a\x64\x65\x66\x20\x68\x65\x61\x70\x70\x6f\x70\x28\x68\x65\x61\x70\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x50\x6f\x70\x20\x74\x68\x65\x20\x73\x6d\x61\x6c\x6c\x65\x73\x74\x20\x69\x74\x65\x6d\x20\x6f\x66\x66\x20\x74\x68\x65\x20\x68\x65\x61\x70\x2c\x20\x6d\x61\x69\x6e\x74\x61\x69\x6e\x69\x6e\x67\x20\x74\x68\x65\x20\x68\x65\x61\x70\x20\x69\x6e\x76\x61\x72\x69\x61\x6e\x74\x2e\x22\x22\x22\x0a\x20\x20\x20\x20\x6c\x61\x73\x74\x65\x6c\x74\x20\x3d\x20\x68\x65\x61\x70\x2e\x70\x6f\x70\x28\x29\x20\x20\x20\x20\x23\x20\x72\x61\x69\x73\x65\x73\x20\x61\x70\x70\x72\x6f\x70\x72\x69\x61\x74\x65\x20\x49\x6e\x64\x65\x78\x45\x72\x72\x6f\x72\x20\x69\x66\x20\x68\x65\x61\x70\x20\x69\x73\x20\x65\x6d\x70\x74\x79\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x65\x61\x70\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x69\x74\x65\x6d\x20\x3d\x20\x68\x65\x61\x70\x5b\x30\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x30\x5d\x20\x3d\x20\x6c\x61\x73\x74\x65\x6c\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x75\x70\x28\x68\x65\x61\x70\x2c\x20\x30\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x75\x72\x6e\x69\x74\x65\x6d\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x61\x73\x74\x65\x6c\x74\x0a\x0a\x64\x65\x66\x20\x68\x65\x61\x70\x72\x65\x70\x6c\x61\x63\x65\x28\x68\x65\x61\x70\x2c\x20\x69\x74\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x50\x6f\x70\x20\x61\x6e\x64\x20\x72\x65\x74\x75\x72\x6e\x20\x74\x68\x65\x20\x63\x75\x72\x72\x65\x6e\x74\x20\x73\x6d\x61\x6c\x6c\x65\x73\x74\x20\x76\x61\x6c\x75\x65\x2c\x20\x61\x6e\x64\x20\x61\x64\x64\x20\x74\x68\x65\x20\x6e\x65\x77\x20\x69\x74\x65\x6d\x2e\x0a\x0a\x20\x20\x20\x20\x54\x68\x69\x73\x20\x69\x73\x20\x6d\x6f\x72\x65\x20\x65\x66\x66\x69\x63\x69\x65\x6e\x74\x20\x74\x68\x61\x6e\x20\x68\x65\x61\x70\x70\x6f\x70\x28\x29\x20\x66\x6f\x6c\x6c\x6f\x77\x65\x64\x20\x62\x79\x20\x68\x65\x61\x70\x70\x75\x73\x68\x28\x29\x2c\x20\x61\x6e\x64\x20\x63\x61\x6e\x20\x62\x65\x0a\x20\x20\x20\x20\x6d\x6f\x72\x65\x20\x61\x70\x70\x72\x6f\x70\x72\x69\x61\x74\x65\x20\x77\x68\x65\x6e\x20\x75\x73\x69\x6e\x67\x20\x61\x20\x66\x69\x78\x65\x64\x2d\x73\x69\x7a\x65\x20\x68\x65\x61\x70\x2e\x20\x20\x4e\x6f\x74\x65\x20\x74\x68\x61\x74\x20\x74\x68\x65\x20\x76\x61\x6c\x75\x65\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x65\x64\x20\x6d\x61\x79\x20\x62\x65\x20\x6c\x61\x72\x67\x65\x72\x20\x74\x68\x61\x6e\x20\x69\x74\x65\x6d\x21\x20\x20\x54\x68\x61\x74\x20\x63\x6f\x6e\x73\x74\x72\x61\x69\x6e\x73\x20\x72\x65\x61\x73\x6f\x6e\x61\x62\x6c\x65\x20\x75\x73\x65\x73\x20\x6f\x66\x0a\x20\x20\x20\x20\x74\x68\x69\x73\x20\x72\x6f\x75\x74\x69\x6e\x65\x20\x75\x6e\x6c\x65\x73\x73\x20\x77\x72\x69\x74\x74\x65\x6e\x20\x61\x73\x20\x70\x61\x72\x74\x20\x6f\x66\x20\x61\x20\x63\x6f\x6e\x64\x69\x74\x69\x6f\x6e\x61\x6c\x20\x72\x65\x70\x6c\x61\x63\x65\x6d\x65\x6e\x74\x3a\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x74\x65\x6d\x20\x3e\x20\x68\x65\x61\x70\x5b\x30\x5d\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x74\x65\x6d\x20\x3d\x20\x68\x65\x61\x70\x72\x65\x70\x6c\x61\x63\x65\x28\x68\x65\x61\x70\x2c\x20\x69\x74\x65\x6d\x29\x0a\x20\x20\x20\x20\x22\x22\x22\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x69\x74\x65\x6d\x20\x3d\x20\x68\x65\x61\x70\x5b\x30\x5d\x20\x20\x20\x20\x23\x20\x72\x61\x69\x73\x65\x73\x20\x61\x70\x70\x72\x6f\x70\x72\x69\x61\x74\x65\x20\x49\x6e\x64\x65\x78\x45\x72\x72\x6f\x72\x20\x69\x66\x20\x68\x65\x61\x70\x20\x69\x73\x20\x65\x6d\x70\x74\x79\x0a\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x30\x5d\x20\x3d\x20\x69\x74\x65\x6d\x0a\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x75\x70\x28\x68\x65\x61\x70\x2c\x20\x30\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x75\x72\x6e\x69\x74\x65\x6d\x0a\x0a\x64\x65\x66\x20\x68\x65\x61\x70\x70\x75\x73\x68\x70\x6f\x70\x28\x68\x65\x61\x70\x2c\x20\x69\x74\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x46\x61\x73\x74\x20\x76\x65\x72\x73\x69\x6f\x6e\x20\x6f\x66\x20\x61\x20\x68\x65\x61\x70\x70\x75\x73\x68\x20\x66\x6f\x6c\x6c\x6f\x77\x65\x64\x20\x62\x79\x20\x61\x20\x68\x65\x61\x70\x70\x6f\x70\x2e\x22\x22\x22\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x65\x61\x70\x20\x61\x6e\x64\x20\x68\x65\x61\x70\x5b\x30\x5d\x20\x3c\x20\x69\x74\x65\x6d\x3a\x0a\x20\x20\x20\x20\x20\x20\x20" "\x20\x69\x74\x65\x6d\x2c\x20\x68\x65\x61\x70\x5b\x30\x5d\x20\x3d\x20\x68\x65\x61\x70\x5b\x30\x5d\x2c\x20\x69\x74\x65\x6d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x75\x70\x28\x68\x65\x61\x70\x2c\x20\x30\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x74\x65\x6d\x0a\x0a\x64\x65\x66\x20\x68\x65\x61\x70\x69\x66\x79\x28\x78\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x54\x72\x61\x6e\x73\x66\x6f\x72\x6d\x20\x6c\x69\x73\x74\x20\x69\x6e\x74\x6f\x20\x61\x20\x68\x65\x61\x70\x2c\x20\x69\x6e\x2d\x70\x6c\x61\x63\x65\x2c\x20\x69\x6e\x20\x4f\x28\x6c\x65\x6e\x28\x78\x29\x29\x20\x74\x69\x6d\x65\x2e\x22\x22\x22\x0a\x20\x20\x20\x20\x6e\x20\x3d\x20\x6c\x65\x6e\x28\x78\x29\x0a\x20\x20\x20\x20\x23\x20\x54\x72\x61\x6e\x73\x66\x6f\x72\x6d\x20\x62\x6f\x74\x74\x6f\x6d\x2d\x75\x70\x2e\x20\x20\x54\x68\x65\x20\x6c\x61\x72\x67\x65\x73\x74\x20\x69\x6e\x64\x65\x78\x20\x74\x68\x65\x72\x65\x27\x73\x20\x61\x6e\x79\x20\x70\x6f\x69\x6e\x74\x20\x74\x6f\x20\x6c\x6f\x6f\x6b\x69\x6e\x67\x20\x61\x74\x0a\x20\x20\x20\x20\x23\x20\x69\x73\x20\x74\x68\x65\x20\x6c\x61\x72\x67\x65\x73\x74\x20\x77\x69\x74\x68\x20\x61\x20\x63\x68\x69\x6c\x64\x20\x69\x6e\x64\x65\x78\x20\x69\x6e\x2d\x72\x61\x6e\x67\x65\x2c\x20\x73\x6f\x20\x6d\x75\x73\x74\x20\x68\x61\x76\x65\x20\x32\x2a\x69\x20\x2b\x20\x31\x20\x3c\x20\x6e\x2c\x0a\x20\x20\x20\x20\x23\x20\x6f\x72\x20\x69\x20\x3c\x20\x28\x6e\x2d\x31\x29\x2f\x32\x2e\x20\x20\x49\x66\x20\x6e\x20\x69\x73\x20\x65\x76\x65\x6e\x20\x3d\x20\x32\x2a\x6a\x2c\x20\x74\x68\x69\x73\x20\x69\x73\x20\x28\x32\x2a\x6a\x2d\x31\x29\x2f\x32\x20\x3d\x20\x6a\x2d\x31\x2f\x32\x20\x73\x6f\x0a\x20\x20\x20\x20\x23\x20\x6a\x2d\x31\x20\x69\x73\x20\x74\x68\x65\x20\x6c\x61\x72\x67\x65\x73\x74\x2c\x20\x77\x68\x69\x63\x68\x20\x69\x73\x20\x6e\x2f\x2f\x32\x20\x2d\x20\x31\x2e\x20\x20\x49\x66\x20\x6e\x20\x69\x73\x20\x6f\x64\x64\x20\x3d\x20\x32\x2a\x6a\x2b\x31\x2c\x20\x74\x68\x69\x73\x20\x69\x73\x0a\x20\x20\x20\x20\x23\x20\x28\x32\x2a\x6a\x2b\x31\x2d\x31\x29\x2f\x32\x20\x3d\x20\x6a\x20\x73\x6f\x20\x6a\x2d\x31\x20\x69\x73\x20\x74\x68\x65\x20\x6c\x61\x72\x67\x65\x73\x74\x2c\x20\x61\x6e\x64\x20\x74\x68\x61\x74\x27\x73\x20\x61\x67\x61\x69\x6e\x20\x6e\x2f\x2f\x32\x2d\x31\x2e\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x65\x76\x65\x72\x73\x65\x64\x28\x72\x61\x6e\x67\x65\x28\x6e\x2f\x2f\x32\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x75\x70\x28\x78\x2c\x20\x69\x29\x0a\x0a\x23\x20\x27\x68\x65\x61\x70\x27\x20\x69\x73\x20\x61\x20\x68\x65\x61\x70\x20\x61\x74\x20\x61\x6c\x6c\x20\x69\x6e\x64\x69\x63\x65\x73\x20\x3e\x3d\x20\x73\x74\x61\x72\x74\x70\x6f\x73\x2c\x20\x65\x78\x63\x65\x70\x74\x20\x70\x6f\x73\x73\x69\x62\x6c\x79\x20\x66\x6f\x72\x20\x70\x6f\x73\x2e\x20\x20\x70\x6f\x73\x0a\x23\x20\x69\x73\x20\x74\x68\x65\x20\x69\x6e\x64\x65\x78\x20\x6f\x66\x20\x61\x20\x6c\x65\x61\x66\x20\x77\x69\x74\x68\x20\x61\x20\x70\x6f\x73\x73\x69\x62\x6c\x79\x20\x6f\x75\x74\x2d\x6f\x66\x2d\x6f\x72\x64\x65\x72\x20\x76\x61\x6c\x75\x65\x2e\x20\x20\x52\x65\x73\x74\x6f\x72\x65\x20\x74\x68\x65\x0a\x23\x20\x68\x65\x61\x70\x20\x69\x6e\x76\x61\x72\x69\x61\x6e\x74\x2e\x0a\x64\x65\x66\x20\x5f\x73\x69\x66\x74\x64\x6f\x77\x6e\x28\x68\x65\x61\x70\x2c\x20\x73\x74\x61\x72\x74\x70\x6f\x73\x2c\x20\x70\x6f\x73\x29\x3a\x0a\x20\x20\x20\x20\x6e\x65\x77\x69\x74\x65\x6d\x20\x3d\x20\x68\x65\x61\x70\x5b\x70\x6f\x73\x5d\x0a\x20\x20\x20\x20\x23\x20\x46\x6f\x6c\x6c\x6f\x77\x20\x74\x68\x65\x20\x70\x61\x74\x68\x20\x74\x6f\x20\x74\x68\x65\x20\x72\x6f\x6f\x74\x2c\x20\x6d\x6f\x76\x69\x6e\x67\x20\x70\x61\x72\x65\x6e\x74\x73\x20\x64\x6f\x77\x6e\x20\x75\x6e\x74\x69\x6c\x20\x66\x69\x6e\x64\x69\x6e\x67\x20\x61\x20\x70\x6c\x61\x63\x65\x0a\x20\x20\x20\x20\x23\x20\x6e\x65\x77\x69\x74\x65\x6d\x20\x66\x69\x74\x73\x2e\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x70\x6f\x73\x20\x3e\x20\x73\x74\x61\x72\x74\x70\x6f\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x70\x61\x72\x65\x6e\x74\x70\x6f\x73\x20\x3d\x20\x28\x70\x6f\x73\x20\x2d\x20\x31\x29\x20\x3e\x3e\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x70\x61\x72\x65\x6e\x74\x20\x3d\x20\x68\x65\x61\x70\x5b\x70\x61\x72\x65\x6e\x74\x70\x6f\x73\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x65\x77\x69\x74\x65\x6d\x20\x3c\x20\x70\x61\x72\x65\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x70\x6f\x73\x5d\x20\x3d\x20\x70\x61\x72\x65\x6e\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x70\x6f\x73\x20\x3d\x20\x70\x61\x72\x65\x6e\x74\x70\x6f\x73\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6f\x6e\x74\x69\x6e\x75\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x62\x72\x65\x61\x6b\x0a\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x70\x6f\x73\x5d\x20\x3d\x20\x6e\x65\x77\x69\x74\x65\x6d\x0a\x0a\x64\x65\x66\x20\x5f\x73\x69\x66\x74\x75\x70\x28\x68\x65\x61\x70\x2c\x20\x70\x6f\x73\x29\x3a\x0a\x20\x20\x20\x20\x65\x6e\x64\x70\x6f\x73\x20\x3d\x20\x6c\x65\x6e\x28\x68\x65\x61\x70\x29\x0a\x20\x20\x20\x20\x73\x74\x61\x72\x74\x70\x6f\x73\x20\x3d\x20\x70\x6f\x73\x0a\x20\x20\x20\x20\x6e\x65\x77\x69\x74\x65\x6d\x20\x3d\x20\x68" "\x65\x61\x70\x5b\x70\x6f\x73\x5d\x0a\x20\x20\x20\x20\x23\x20\x42\x75\x62\x62\x6c\x65\x20\x75\x70\x20\x74\x68\x65\x20\x73\x6d\x61\x6c\x6c\x65\x72\x20\x63\x68\x69\x6c\x64\x20\x75\x6e\x74\x69\x6c\x20\x68\x69\x74\x74\x69\x6e\x67\x20\x61\x20\x6c\x65\x61\x66\x2e\x0a\x20\x20\x20\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x3d\x20\x32\x2a\x70\x6f\x73\x20\x2b\x20\x31\x20\x20\x20\x20\x23\x20\x6c\x65\x66\x74\x6d\x6f\x73\x74\x20\x63\x68\x69\x6c\x64\x20\x70\x6f\x73\x69\x74\x69\x6f\x6e\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x3c\x20\x65\x6e\x64\x70\x6f\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x53\x65\x74\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x74\x6f\x20\x69\x6e\x64\x65\x78\x20\x6f\x66\x20\x73\x6d\x61\x6c\x6c\x65\x72\x20\x63\x68\x69\x6c\x64\x2e\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x69\x67\x68\x74\x70\x6f\x73\x20\x3d\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x2b\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x72\x69\x67\x68\x74\x70\x6f\x73\x20\x3c\x20\x65\x6e\x64\x70\x6f\x73\x20\x61\x6e\x64\x20\x6e\x6f\x74\x20\x68\x65\x61\x70\x5b\x63\x68\x69\x6c\x64\x70\x6f\x73\x5d\x20\x3c\x20\x68\x65\x61\x70\x5b\x72\x69\x67\x68\x74\x70\x6f\x73\x5d\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x3d\x20\x72\x69\x67\x68\x74\x70\x6f\x73\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x4d\x6f\x76\x65\x20\x74\x68\x65\x20\x73\x6d\x61\x6c\x6c\x65\x72\x20\x63\x68\x69\x6c\x64\x20\x75\x70\x2e\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x70\x6f\x73\x5d\x20\x3d\x20\x68\x65\x61\x70\x5b\x63\x68\x69\x6c\x64\x70\x6f\x73\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x70\x6f\x73\x20\x3d\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x3d\x20\x32\x2a\x70\x6f\x73\x20\x2b\x20\x31\x0a\x20\x20\x20\x20\x23\x20\x54\x68\x65\x20\x6c\x65\x61\x66\x20\x61\x74\x20\x70\x6f\x73\x20\x69\x73\x20\x65\x6d\x70\x74\x79\x20\x6e\x6f\x77\x2e\x20\x20\x50\x75\x74\x20\x6e\x65\x77\x69\x74\x65\x6d\x20\x74\x68\x65\x72\x65\x2c\x20\x61\x6e\x64\x20\x62\x75\x62\x62\x6c\x65\x20\x69\x74\x20\x75\x70\x0a\x20\x20\x20\x20\x23\x20\x74\x6f\x20\x69\x74\x73\x20\x66\x69\x6e\x61\x6c\x20\x72\x65\x73\x74\x69\x6e\x67\x20\x70\x6c\x61\x63\x65\x20\x28\x62\x79\x20\x73\x69\x66\x74\x69\x6e\x67\x20\x69\x74\x73\x20\x70\x61\x72\x65\x6e\x74\x73\x20\x64\x6f\x77\x6e\x29\x2e\x0a\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x70\x6f\x73\x5d\x20\x3d\x20\x6e\x65\x77\x69\x74\x65\x6d\x0a\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x64\x6f\x77\x6e\x28\x68\x65\x61\x70\x2c\x20\x73\x74\x61\x72\x74\x70\x6f\x73\x2c\x20\x70\x6f\x73\x29" },
        {"functools", "\x64\x65\x66\x20\x63\x61\x63\x68\x65\x28\x66\x29\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x77\x72\x61\x70\x70\x65\x72\x28\x2a\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x68\x61\x73\x61\x74\x74\x72\x28\x66\x2c\x20\x27\x5f\x5f\x63\x61\x63\x68\x65\x5f\x5f\x27\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x2e\x5f\x5f\x63\x61\x63\x68\x65\x5f\x5f\x20\x3d\x20\x7b\x7d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6b\x65\x79\x20\x3d\x20\x61\x72\x67\x73\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6b\x65\x79\x20\x6e\x6f\x74\x20\x69\x6e\x20\x66\x2e\x5f\x5f\x63\x61\x63\x68\x65\x5f\x5f\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x2e\x5f\x5f\x63\x61\x63\x68\x65\x5f\x5f\x5b\x6b\x65\x79\x5d\x20\x3d\x20\x66\x28\x2a\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x2e\x5f\x5f\x63\x61\x63\x68\x65\x5f\x5f\x5b\x6b\x65\x79\x5d\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x77\x72\x61\x70\x70\x65\x72" },
        {"builtins", "\x69\x6d\x70\x6f\x72\x74\x20\x73\x79\x73\x20\x61\x73\x20\x5f\x73\x79\x73\x0a\x0a\x64\x65\x66\x20\x70\x72\x69\x6e\x74\x28\x2a\x61\x72\x67\x73\x2c\x20\x73\x65\x70\x3d\x27\x20\x27\x2c\x20\x65\x6e\x64\x3d\x27\x5c\x6e\x27\x29\x3a\x0a\x20\x20\x20\x20\x73\x20\x3d\x20\x73\x65\x70\x2e\x6a\x6f\x69\x6e\x28\x5b\x73\x74\x72\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x61\x72\x67\x73\x5d\x29\x0a\x20\x20\x20\x20\x5f\x73\x79\x73\x2e\x73\x74\x64\x6f\x75\x74\x2e\x77\x72\x69\x74\x65\x28\x73\x20\x2b\x20\x65\x6e\x64\x29\x0a\x0a\x64\x65\x66\x20\x72\x6f\x75\x6e\x64\x28\x78\x2c\x20\x6e\x64\x69\x67\x69\x74\x73\x3d\x30\x29\x3a\x0a\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x6e\x64\x69\x67\x69\x74\x73\x20\x3e\x3d\x20\x30\x0a\x20\x20\x20\x20\x69\x66\x20\x6e\x64\x69\x67\x69\x74\x73\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x6e\x74\x28\x78\x20\x2b\x20\x30\x2e\x35\x29\x20\x69\x66\x20\x78\x20\x3e\x3d\x20\x30\x20\x65\x6c\x73\x65\x20\x69\x6e\x74\x28\x78\x20\x2d\x20\x30\x2e\x35\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x78\x20\x3e\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x6e\x74\x28\x78\x20\x2a\x20\x31\x30\x2a\x2a\x6e\x64\x69\x67\x69\x74\x73\x20\x2b\x20\x30\x2e\x35\x29\x20\x2f\x20\x31\x30\x2a\x2a\x6e\x64\x69\x67\x69\x74\x73\x0a\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x6e\x74\x28\x78\x20\x2a\x20\x31\x30\x2a\x2a\x6e\x64\x69\x67\x69\x74\x73\x20\x2d\x20\x30\x2e\x35\x29\x20\x2f\x20\x31\x30\x2a\x2a\x6e\x64\x69\x67\x69\x74\x73\x0a\x0a\x64\x65\x66\x20\x61\x62\x73\x28\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x2d\x78\x20\x69\x66\x20\x78\x20\x3c\x20\x30\x20\x65\x6c\x73\x65\x20\x78\x0a\x0a\x64\x65\x66\x20\x6d\x61\x78\x28\x2a\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x61\x72\x67\x73\x29\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x54\x79\x70\x65\x45\x72\x72\x6f\x72\x28\x27\x6d\x61\x78\x20\x65\x78\x70\x65\x63\x74\x65\x64\x20\x31\x20\x61\x72\x67\x75\x6d\x65\x6e\x74\x73\x2c\x20\x67\x6f\x74\x20\x30\x27\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x61\x72\x67\x73\x29\x20\x3d\x3d\x20\x31\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x72\x67\x73\x20\x3d\x20\x61\x72\x67\x73\x5b\x30\x5d\x0a\x20\x20\x20\x20\x61\x72\x67\x73\x20\x3d\x20\x69\x74\x65\x72\x28\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x6e\x65\x78\x74\x28\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x72\x65\x73\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x27\x6d\x61\x78\x28\x29\x20\x61\x72\x67\x20\x69\x73\x20\x61\x6e\x20\x65\x6d\x70\x74\x79\x20\x73\x65\x71\x75\x65\x6e\x63\x65\x27\x29\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x54\x72\x75\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x20\x3d\x20\x6e\x65\x78\x74\x28\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x72\x65\x61\x6b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x3e\x20\x72\x65\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x69\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x6d\x69\x6e\x28\x2a\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x61\x72\x67\x73\x29\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x54\x79\x70\x65\x45\x72\x72\x6f\x72\x28\x27\x6d\x69\x6e\x20\x65\x78\x70\x65\x63\x74\x65\x64\x20\x31\x20\x61\x72\x67\x75\x6d\x65\x6e\x74\x73\x2c\x20\x67\x6f\x74\x20\x30\x27\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x61\x72\x67\x73\x29\x20\x3d\x3d\x20\x31\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x72\x67\x73\x20\x3d\x20\x61\x72\x67\x73\x5b\x30\x5d\x0a\x20\x20\x20\x20\x61\x72\x67\x73\x20\x3d\x20\x69\x74\x65\x72\x28\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x6e\x65\x78\x74\x28\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x72\x65\x73\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x27\x6d\x69\x6e\x28\x29\x20\x61\x72\x67\x20\x69\x73\x20\x61\x6e\x20\x65\x6d\x70\x74\x79\x20\x73\x65\x71\x75\x65\x6e\x63\x65\x27\x29\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x54\x72\x75\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x20\x3d\x20\x6e\x65\x78\x74\x28\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x72\x65\x61\x6b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x3c\x20\x72\x65\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20" "\x20\x72\x65\x73\x20\x3d\x20\x69\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x61\x6c\x6c\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x69\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x54\x72\x75\x65\x0a\x0a\x64\x65\x66\x20\x61\x6e\x79\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x54\x72\x75\x65\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x0a\x64\x65\x66\x20\x65\x6e\x75\x6d\x65\x72\x61\x74\x65\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x2c\x20\x73\x74\x61\x72\x74\x3d\x30\x29\x3a\x0a\x20\x20\x20\x20\x6e\x20\x3d\x20\x73\x74\x61\x72\x74\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x6e\x2c\x20\x65\x6c\x65\x6d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x2b\x2b\x6e\x0a\x0a\x64\x65\x66\x20\x73\x75\x6d\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x20\x2b\x3d\x20\x69\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x6d\x61\x70\x28\x66\x2c\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x66\x28\x69\x29\x0a\x0a\x64\x65\x66\x20\x66\x69\x6c\x74\x65\x72\x28\x66\x2c\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x66\x28\x69\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x69\x0a\x0a\x64\x65\x66\x20\x7a\x69\x70\x28\x61\x2c\x20\x62\x29\x3a\x0a\x20\x20\x20\x20\x61\x20\x3d\x20\x69\x74\x65\x72\x28\x61\x29\x0a\x20\x20\x20\x20\x62\x20\x3d\x20\x69\x74\x65\x72\x28\x62\x29\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x54\x72\x75\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x69\x20\x3d\x20\x6e\x65\x78\x74\x28\x61\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x62\x69\x20\x3d\x20\x6e\x65\x78\x74\x28\x62\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x61\x69\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x20\x6f\x72\x20\x62\x69\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x72\x65\x61\x6b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x61\x69\x2c\x20\x62\x69\x0a\x0a\x64\x65\x66\x20\x72\x65\x76\x65\x72\x73\x65\x64\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x61\x20\x3d\x20\x6c\x69\x73\x74\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x0a\x20\x20\x20\x20\x61\x2e\x72\x65\x76\x65\x72\x73\x65\x28\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x0a\x0a\x64\x65\x66\x20\x73\x6f\x72\x74\x65\x64\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x2c\x20\x72\x65\x76\x65\x72\x73\x65\x3d\x46\x61\x6c\x73\x65\x2c\x20\x6b\x65\x79\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x61\x20\x3d\x20\x6c\x69\x73\x74\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x0a\x20\x20\x20\x20\x61\x2e\x73\x6f\x72\x74\x28\x72\x65\x76\x65\x72\x73\x65\x3d\x72\x65\x76\x65\x72\x73\x65\x2c\x20\x6b\x65\x79\x3d\x6b\x65\x79\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x0a\x0a\x23\x23\x23\x23\x23\x20\x73\x74\x72\x20\x23\x23\x23\x23\x23\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x73\x65\x70\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x73\x65\x70\x20\x3d\x20\x73\x65\x70\x20\x6f\x72\x20\x27\x20\x27\x0a\x20\x20\x20\x20\x69\x66\x20\x73\x65\x70\x20\x3d\x3d\x20\x22\x22\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x69\x73\x74\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x69\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x69\x20\x3c\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x5b\x69\x3a\x69\x2b\x6c\x65\x6e\x28\x73\x65\x70\x29\x5d\x20\x3d\x3d\x20\x73\x65\x70\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x65\x6c\x66\x5b\x3a\x69\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x20\x3d\x20\x73\x65\x6c\x66\x5b\x69\x2b\x6c\x65\x6e\x28\x73\x65\x70\x29\x3a\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x20\x3d\x20\x30\x0a" "\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x2b\x2b\x69\x0a\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x73\x74\x72\x2e\x73\x70\x6c\x69\x74\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x2a\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x27\x7b\x7d\x27\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x61\x72\x67\x73\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x20\x3d\x20\x73\x65\x6c\x66\x2e\x72\x65\x70\x6c\x61\x63\x65\x28\x27\x7b\x7d\x27\x2c\x20\x73\x74\x72\x28\x61\x72\x67\x73\x5b\x69\x5d\x29\x2c\x20\x31\x29\x0a\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x61\x72\x67\x73\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x20\x3d\x20\x73\x65\x6c\x66\x2e\x72\x65\x70\x6c\x61\x63\x65\x28\x27\x7b\x27\x2b\x73\x74\x72\x28\x69\x29\x2b\x27\x7d\x27\x2c\x20\x73\x74\x72\x28\x61\x72\x67\x73\x5b\x69\x5d\x29\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x0a\x73\x74\x72\x2e\x66\x6f\x72\x6d\x61\x74\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x63\x68\x61\x72\x73\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x63\x68\x61\x72\x73\x20\x3d\x20\x63\x68\x61\x72\x73\x20\x6f\x72\x20\x27\x20\x5c\x74\x5c\x6e\x5c\x72\x27\x0a\x20\x20\x20\x20\x69\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x69\x20\x3c\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x61\x6e\x64\x20\x73\x65\x6c\x66\x5b\x69\x5d\x20\x69\x6e\x20\x63\x68\x61\x72\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x2b\x2b\x69\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x5b\x69\x3a\x5d\x0a\x73\x74\x72\x2e\x6c\x73\x74\x72\x69\x70\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x63\x68\x61\x72\x73\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x63\x68\x61\x72\x73\x20\x3d\x20\x63\x68\x61\x72\x73\x20\x6f\x72\x20\x27\x20\x5c\x74\x5c\x6e\x5c\x72\x27\x0a\x20\x20\x20\x20\x6a\x20\x3d\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x2d\x20\x31\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6a\x20\x3e\x3d\x20\x30\x20\x61\x6e\x64\x20\x73\x65\x6c\x66\x5b\x6a\x5d\x20\x69\x6e\x20\x63\x68\x61\x72\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x2d\x2d\x6a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x5b\x3a\x6a\x2b\x31\x5d\x0a\x73\x74\x72\x2e\x72\x73\x74\x72\x69\x70\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x63\x68\x61\x72\x73\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x63\x68\x61\x72\x73\x20\x3d\x20\x63\x68\x61\x72\x73\x20\x6f\x72\x20\x27\x20\x5c\x74\x5c\x6e\x5c\x72\x27\x0a\x20\x20\x20\x20\x69\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x69\x20\x3c\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x61\x6e\x64\x20\x73\x65\x6c\x66\x5b\x69\x5d\x20\x69\x6e\x20\x63\x68\x61\x72\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x2b\x2b\x69\x0a\x20\x20\x20\x20\x6a\x20\x3d\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x2d\x20\x31\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6a\x20\x3e\x3d\x20\x30\x20\x61\x6e\x64\x20\x73\x65\x6c\x66\x5b\x6a\x5d\x20\x69\x6e\x20\x63\x68\x61\x72\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x2d\x2d\x6a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x5b\x69\x3a\x6a\x2b\x31\x5d\x0a\x73\x74\x72\x2e\x73\x74\x72\x69\x70\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x23\x23\x23\x23\x23\x20\x6c\x69\x73\x74\x20\x23\x23\x23\x23\x23\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66\x3a\x20\x27\x5b\x27\x20\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x72\x65\x70\x72\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x5d\x29\x20\x2b\x20\x27\x5d\x27\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66\x3a\x20\x27\x28\x27\x20\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x72\x65\x70\x72\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x5d\x29\x20\x2b\x20\x27\x29\x27\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x6a\x73\x6f\x6e\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66\x3a\x20\x27\x5b\x27\x20\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x69\x2e\x5f\x5f\x6a\x73\x6f\x6e\x5f\x5f\x28\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x5d\x29\x20\x2b\x20\x27\x5d\x27\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x6a\x73\x6f\x6e\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66\x3a\x20\x27\x5b\x27\x20\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x69\x2e\x5f\x5f\x6a\x73\x6f\x6e\x5f\x5f\x28\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e" "\x20\x73\x65\x6c\x66\x5d\x29\x20\x2b\x20\x27\x5d\x27\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x71\x73\x6f\x72\x74\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x4c\x3a\x20\x69\x6e\x74\x2c\x20\x52\x3a\x20\x69\x6e\x74\x2c\x20\x6b\x65\x79\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x4c\x20\x3e\x3d\x20\x52\x3a\x20\x72\x65\x74\x75\x72\x6e\x3b\x0a\x20\x20\x20\x20\x6d\x69\x64\x20\x3d\x20\x61\x5b\x28\x52\x2b\x4c\x29\x2f\x2f\x32\x5d\x3b\x0a\x20\x20\x20\x20\x6d\x69\x64\x20\x3d\x20\x6b\x65\x79\x28\x6d\x69\x64\x29\x0a\x20\x20\x20\x20\x69\x2c\x20\x6a\x20\x3d\x20\x4c\x2c\x20\x52\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x69\x3c\x3d\x6a\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6b\x65\x79\x28\x61\x5b\x69\x5d\x29\x3c\x6d\x69\x64\x3a\x20\x2b\x2b\x69\x3b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6b\x65\x79\x28\x61\x5b\x6a\x5d\x29\x3e\x6d\x69\x64\x3a\x20\x2d\x2d\x6a\x3b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x3c\x3d\x6a\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x5b\x69\x5d\x2c\x20\x61\x5b\x6a\x5d\x20\x3d\x20\x61\x5b\x6a\x5d\x2c\x20\x61\x5b\x69\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x2b\x2b\x69\x3b\x20\x2d\x2d\x6a\x3b\x0a\x20\x20\x20\x20\x5f\x5f\x71\x73\x6f\x72\x74\x28\x61\x2c\x20\x4c\x2c\x20\x6a\x2c\x20\x6b\x65\x79\x29\x0a\x20\x20\x20\x20\x5f\x5f\x71\x73\x6f\x72\x74\x28\x61\x2c\x20\x69\x2c\x20\x52\x2c\x20\x6b\x65\x79\x29\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x72\x65\x76\x65\x72\x73\x65\x3d\x46\x61\x6c\x73\x65\x2c\x20\x6b\x65\x79\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x6b\x65\x79\x20\x69\x73\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6b\x65\x79\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x78\x3a\x78\x0a\x20\x20\x20\x20\x5f\x5f\x71\x73\x6f\x72\x74\x28\x73\x65\x6c\x66\x2c\x20\x30\x2c\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x2d\x31\x2c\x20\x6b\x65\x79\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x72\x65\x76\x65\x72\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x72\x65\x76\x65\x72\x73\x65\x28\x29\x0a\x6c\x69\x73\x74\x2e\x73\x6f\x72\x74\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x2c\x20\x6a\x20\x69\x6e\x20\x7a\x69\x70\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x21\x3d\x20\x6a\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x20\x3c\x20\x6a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x3c\x20\x6c\x65\x6e\x28\x6f\x74\x68\x65\x72\x29\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x6c\x74\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x6c\x74\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x2c\x20\x6a\x20\x69\x6e\x20\x7a\x69\x70\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x21\x3d\x20\x6a\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x20\x3e\x20\x6a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x3e\x20\x6c\x65\x6e\x28\x6f\x74\x68\x65\x72\x29\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x67\x74\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x67\x74\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x2c\x20\x6a\x20\x69\x6e\x20\x7a\x69\x70\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x21\x3d\x20\x6a\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x20\x3c\x3d\x20\x6a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x3c\x3d\x20\x6c\x65\x6e\x28\x6f\x74\x68\x65\x72\x29\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x6c\x65\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x6c\x65\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x2c\x20\x6a\x20\x69\x6e\x20\x7a\x69\x70\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x21\x3d\x20\x6a\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x20\x3e\x3d\x20\x6a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x3e\x3d\x20\x6c\x65\x6e\x28\x6f\x74\x68\x65\x72\x29\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x67\x65\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x67\x65\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x74\x79\x70\x65\x2e\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66" "\x3a\x20\x22\x3c\x63\x6c\x61\x73\x73\x20\x27\x22\x20\x2b\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x6e\x61\x6d\x65\x5f\x5f\x20\x2b\x20\x22\x27\x3e\x22\x0a\x0a\x64\x65\x66\x20\x68\x65\x6c\x70\x28\x6f\x62\x6a\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x61\x73\x61\x74\x74\x72\x28\x6f\x62\x6a\x2c\x20\x27\x5f\x5f\x66\x75\x6e\x63\x5f\x5f\x27\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x62\x6a\x20\x3d\x20\x6f\x62\x6a\x2e\x5f\x5f\x66\x75\x6e\x63\x5f\x5f\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x61\x73\x61\x74\x74\x72\x28\x6f\x62\x6a\x2c\x20\x27\x5f\x5f\x64\x6f\x63\x5f\x5f\x27\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x70\x72\x69\x6e\x74\x28\x6f\x62\x6a\x2e\x5f\x5f\x64\x6f\x63\x5f\x5f\x29\x0a\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x70\x72\x69\x6e\x74\x28\x22\x4e\x6f\x20\x64\x6f\x63\x73\x74\x72\x69\x6e\x67\x20\x66\x6f\x75\x6e\x64\x22\x29\x0a\x0a\x0a\x64\x65\x6c\x20\x5f\x5f\x66" },
        {"this", "\x70\x72\x69\x6e\x74\x28\x22\x22\x22\x54\x68\x65\x20\x5a\x65\x6e\x20\x6f\x66\x20\x50\x79\x74\x68\x6f\x6e\x2c\x20\x62\x79\x20\x54\x69\x6d\x20\x50\x65\x74\x65\x72\x73\x0a\x0a\x42\x65\x61\x75\x74\x69\x66\x75\x6c\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x75\x67\x6c\x79\x2e\x0a\x45\x78\x70\x6c\x69\x63\x69\x74\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x69\x6d\x70\x6c\x69\x63\x69\x74\x2e\x0a\x53\x69\x6d\x70\x6c\x65\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x63\x6f\x6d\x70\x6c\x65\x78\x2e\x0a\x43\x6f\x6d\x70\x6c\x65\x78\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x63\x6f\x6d\x70\x6c\x69\x63\x61\x74\x65\x64\x2e\x0a\x46\x6c\x61\x74\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x6e\x65\x73\x74\x65\x64\x2e\x0a\x53\x70\x61\x72\x73\x65\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x64\x65\x6e\x73\x65\x2e\x0a\x52\x65\x61\x64\x61\x62\x69\x6c\x69\x74\x79\x20\x63\x6f\x75\x6e\x74\x73\x2e\x0a\x53\x70\x65\x63\x69\x61\x6c\x20\x63\x61\x73\x65\x73\x20\x61\x72\x65\x6e\x27\x74\x20\x73\x70\x65\x63\x69\x61\x6c\x20\x65\x6e\x6f\x75\x67\x68\x20\x74\x6f\x20\x62\x72\x65\x61\x6b\x20\x74\x68\x65\x20\x72\x75\x6c\x65\x73\x2e\x0a\x41\x6c\x74\x68\x6f\x75\x67\x68\x20\x70\x72\x61\x63\x74\x69\x63\x61\x6c\x69\x74\x79\x20\x62\x65\x61\x74\x73\x20\x70\x75\x72\x69\x74\x79\x2e\x0a\x45\x72\x72\x6f\x72\x73\x20\x73\x68\x6f\x75\x6c\x64\x20\x6e\x65\x76\x65\x72\x20\x70\x61\x73\x73\x20\x73\x69\x6c\x65\x6e\x74\x6c\x79\x2e\x0a\x55\x6e\x6c\x65\x73\x73\x20\x65\x78\x70\x6c\x69\x63\x69\x74\x6c\x79\x20\x73\x69\x6c\x65\x6e\x63\x65\x64\x2e\x0a\x49\x6e\x20\x74\x68\x65\x20\x66\x61\x63\x65\x20\x6f\x66\x20\x61\x6d\x62\x69\x67\x75\x69\x74\x79\x2c\x20\x72\x65\x66\x75\x73\x65\x20\x74\x68\x65\x20\x74\x65\x6d\x70\x74\x61\x74\x69\x6f\x6e\x20\x74\x6f\x20\x67\x75\x65\x73\x73\x2e\x0a\x54\x68\x65\x72\x65\x20\x73\x68\x6f\x75\x6c\x64\x20\x62\x65\x20\x6f\x6e\x65\x2d\x2d\x20\x61\x6e\x64\x20\x70\x72\x65\x66\x65\x72\x61\x62\x6c\x79\x20\x6f\x6e\x6c\x79\x20\x6f\x6e\x65\x20\x2d\x2d\x6f\x62\x76\x69\x6f\x75\x73\x20\x77\x61\x79\x20\x74\x6f\x20\x64\x6f\x20\x69\x74\x2e\x0a\x41\x6c\x74\x68\x6f\x75\x67\x68\x20\x74\x68\x61\x74\x20\x77\x61\x79\x20\x6d\x61\x79\x20\x6e\x6f\x74\x20\x62\x65\x20\x6f\x62\x76\x69\x6f\x75\x73\x20\x61\x74\x20\x66\x69\x72\x73\x74\x20\x75\x6e\x6c\x65\x73\x73\x20\x79\x6f\x75\x27\x72\x65\x20\x44\x75\x74\x63\x68\x2e\x0a\x4e\x6f\x77\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x6e\x65\x76\x65\x72\x2e\x0a\x41\x6c\x74\x68\x6f\x75\x67\x68\x20\x6e\x65\x76\x65\x72\x20\x69\x73\x20\x6f\x66\x74\x65\x6e\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x2a\x72\x69\x67\x68\x74\x2a\x20\x6e\x6f\x77\x2e\x0a\x49\x66\x20\x74\x68\x65\x20\x69\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x61\x74\x69\x6f\x6e\x20\x69\x73\x20\x68\x61\x72\x64\x20\x74\x6f\x20\x65\x78\x70\x6c\x61\x69\x6e\x2c\x20\x69\x74\x27\x73\x20\x61\x20\x62\x61\x64\x20\x69\x64\x65\x61\x2e\x0a\x49\x66\x20\x74\x68\x65\x20\x69\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x61\x74\x69\x6f\x6e\x20\x69\x73\x20\x65\x61\x73\x79\x20\x74\x6f\x20\x65\x78\x70\x6c\x61\x69\x6e\x2c\x20\x69\x74\x20\x6d\x61\x79\x20\x62\x65\x20\x61\x20\x67\x6f\x6f\x64\x20\x69\x64\x65\x61\x2e\x0a\x4e\x61\x6d\x65\x73\x70\x61\x63\x65\x73\x20\x61\x72\x65\x20\x6f\x6e\x65\x20\x68\x6f\x6e\x6b\x69\x6e\x67\x20\x67\x72\x65\x61\x74\x20\x69\x64\x65\x61\x20\x2d\x2d\x20\x6c\x65\x74\x27\x73\x20\x64\x6f\x20\x6d\x6f\x72\x65\x20\x6f\x66\x20\x74\x68\x6f\x73\x65\x21\x22\x22\x22\x29" },
        {"random", "\x5f\x69\x6e\x73\x74\x20\x3d\x20\x52\x61\x6e\x64\x6f\x6d\x28\x29\x0a\x0a\x73\x65\x65\x64\x20\x3d\x20\x5f\x69\x6e\x73\x74\x2e\x73\x65\x65\x64\x0a\x72\x61\x6e\x64\x6f\x6d\x20\x3d\x20\x5f\x69\x6e\x73\x74\x2e\x72\x61\x6e\x64\x6f\x6d\x0a\x75\x6e\x69\x66\x6f\x72\x6d\x20\x3d\x20\x5f\x69\x6e\x73\x74\x2e\x75\x6e\x69\x66\x6f\x72\x6d\x0a\x72\x61\x6e\x64\x69\x6e\x74\x20\x3d\x20\x5f\x69\x6e\x73\x74\x2e\x72\x61\x6e\x64\x69\x6e\x74\x0a\x0a\x64\x65\x66\x20\x73\x68\x75\x66\x66\x6c\x65\x28\x4c\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x4c\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6a\x20\x3d\x20\x72\x61\x6e\x64\x69\x6e\x74\x28\x69\x2c\x20\x6c\x65\x6e\x28\x4c\x29\x20\x2d\x20\x31\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x4c\x5b\x69\x5d\x2c\x20\x4c\x5b\x6a\x5d\x20\x3d\x20\x4c\x5b\x6a\x5d\x2c\x20\x4c\x5b\x69\x5d\x0a\x0a\x64\x65\x66\x20\x63\x68\x6f\x69\x63\x65\x28\x4c\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4c\x5b\x72\x61\x6e\x64\x69\x6e\x74\x28\x30\x2c\x20\x6c\x65\x6e\x28\x4c\x29\x20\x2d\x20\x31\x29\x5d" },
        {"collections", "\x63\x6c\x61\x73\x73\x20\x5f\x4c\x69\x6e\x6b\x65\x64\x4c\x69\x73\x74\x4e\x6f\x64\x65\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x70\x72\x65\x76\x2c\x20\x6e\x65\x78\x74\x2c\x20\x76\x61\x6c\x75\x65\x29\x20\x2d\x3e\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x70\x72\x65\x76\x20\x3d\x20\x70\x72\x65\x76\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x6e\x65\x78\x74\x20\x3d\x20\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x76\x61\x6c\x75\x65\x20\x3d\x20\x76\x61\x6c\x75\x65\x0a\x0a\x63\x6c\x61\x73\x73\x20\x64\x65\x71\x75\x65\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3d\x4e\x6f\x6e\x65\x29\x20\x2d\x3e\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x20\x3d\x20\x5f\x4c\x69\x6e\x6b\x65\x64\x4c\x69\x73\x74\x4e\x6f\x64\x65\x28\x4e\x6f\x6e\x65\x2c\x20\x4e\x6f\x6e\x65\x2c\x20\x4e\x6f\x6e\x65\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x20\x3d\x20\x5f\x4c\x69\x6e\x6b\x65\x64\x4c\x69\x73\x74\x4e\x6f\x64\x65\x28\x4e\x6f\x6e\x65\x2c\x20\x4e\x6f\x6e\x65\x2c\x20\x4e\x6f\x6e\x65\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x20\x3d\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x2e\x70\x72\x65\x76\x20\x3d\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x76\x61\x6c\x75\x65\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x61\x70\x70\x65\x6e\x64\x28\x76\x61\x6c\x75\x65\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x61\x70\x70\x65\x6e\x64\x28\x73\x65\x6c\x66\x2c\x20\x76\x61\x6c\x75\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x5f\x4c\x69\x6e\x6b\x65\x64\x4c\x69\x73\x74\x4e\x6f\x64\x65\x28\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x2e\x70\x72\x65\x76\x2c\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x2c\x20\x76\x61\x6c\x75\x65\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x2e\x70\x72\x65\x76\x2e\x6e\x65\x78\x74\x20\x3d\x20\x6e\x6f\x64\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x2e\x70\x72\x65\x76\x20\x3d\x20\x6e\x6f\x64\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x2b\x3d\x20\x31\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x61\x70\x70\x65\x6e\x64\x6c\x65\x66\x74\x28\x73\x65\x6c\x66\x2c\x20\x76\x61\x6c\x75\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x5f\x4c\x69\x6e\x6b\x65\x64\x4c\x69\x73\x74\x4e\x6f\x64\x65\x28\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2c\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x2c\x20\x76\x61\x6c\x75\x65\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x2e\x70\x72\x65\x76\x20\x3d\x20\x6e\x6f\x64\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x20\x3d\x20\x6e\x6f\x64\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x2b\x3d\x20\x31\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x70\x6f\x70\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x3e\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x2e\x70\x72\x65\x76\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x2e\x70\x72\x65\x76\x2e\x6e\x65\x78\x74\x20\x3d\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x2e\x70\x72\x65\x76\x20\x3d\x20\x6e\x6f\x64\x65\x2e\x70\x72\x65\x76\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x2d\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6e\x6f\x64\x65\x2e\x76\x61\x6c\x75\x65\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x70\x6f\x70\x6c\x65\x66\x74\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x3e\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x2e\x6e\x65\x78\x74\x2e\x70\x72\x65\x76\x20\x3d\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x20\x3d\x20\x6e\x6f\x64\x65\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20" "\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x2d\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6e\x6f\x64\x65\x2e\x76\x61\x6c\x75\x65\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6f\x70\x79\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x65\x77\x5f\x6c\x69\x73\x74\x20\x3d\x20\x64\x65\x71\x75\x65\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x76\x61\x6c\x75\x65\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x65\x77\x5f\x6c\x69\x73\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x76\x61\x6c\x75\x65\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6e\x65\x77\x5f\x6c\x69\x73\x74\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x74\x65\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6e\x6f\x64\x65\x20\x69\x73\x20\x6e\x6f\x74\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x6e\x6f\x64\x65\x2e\x76\x61\x6c\x75\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x6e\x6f\x64\x65\x2e\x6e\x65\x78\x74\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x20\x2d\x3e\x20\x73\x74\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x22\x64\x65\x71\x75\x65\x28\x7b\x6c\x69\x73\x74\x28\x73\x65\x6c\x66\x29\x7d\x29\x22\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x5f\x5f\x6f\x3a\x20\x6f\x62\x6a\x65\x63\x74\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x69\x73\x69\x6e\x73\x74\x61\x6e\x63\x65\x28\x5f\x5f\x6f\x2c\x20\x64\x65\x71\x75\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x21\x3d\x20\x6c\x65\x6e\x28\x5f\x5f\x6f\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x74\x31\x2c\x20\x74\x32\x20\x3d\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x2c\x20\x5f\x5f\x6f\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x74\x31\x20\x69\x73\x20\x6e\x6f\x74\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x31\x2e\x76\x61\x6c\x75\x65\x20\x21\x3d\x20\x74\x32\x2e\x76\x61\x6c\x75\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x74\x31\x2c\x20\x74\x32\x20\x3d\x20\x74\x31\x2e\x6e\x65\x78\x74\x2c\x20\x74\x32\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x54\x72\x75\x65\x0a\x0a\x64\x65\x66\x20\x43\x6f\x75\x6e\x74\x65\x72\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x61\x20\x3d\x20\x7b\x7d\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x78\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x78\x20\x69\x6e\x20\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x5b\x78\x5d\x20\x2b\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x5b\x78\x5d\x20\x3d\x20\x31\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x0a\x0a\x63\x6c\x61\x73\x73\x20\x64\x65\x66\x61\x75\x6c\x74\x64\x69\x63\x74\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x29\x20\x2d\x3e\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x20\x3d\x20\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x20\x3d\x20\x7b\x7d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x65\x74\x69\x74\x65\x6d\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6b\x65\x79\x20\x6e\x6f\x74\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x6b\x65\x79\x5d\x20\x3d\x20\x73\x65\x6c\x66\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65" "\x6c\x66\x2e\x5f\x61\x5b\x6b\x65\x79\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x73\x65\x74\x69\x74\x65\x6d\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x2c\x20\x76\x61\x6c\x75\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x6b\x65\x79\x5d\x20\x3d\x20\x76\x61\x6c\x75\x65\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x20\x2d\x3e\x20\x73\x74\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x22\x64\x65\x66\x61\x75\x6c\x74\x64\x69\x63\x74\x28\x7b\x73\x65\x6c\x66\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x7d\x2c\x20\x7b\x73\x65\x6c\x66\x2e\x5f\x61\x7d\x29\x22\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x5f\x5f\x6f\x3a\x20\x6f\x62\x6a\x65\x63\x74\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x69\x73\x69\x6e\x73\x74\x61\x6e\x63\x65\x28\x5f\x5f\x6f\x2c\x20\x64\x65\x66\x61\x75\x6c\x74\x64\x69\x63\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x20\x21\x3d\x20\x5f\x5f\x6f\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x20\x3d\x3d\x20\x5f\x5f\x6f\x2e\x5f\x61\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x2e\x5f\x61\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x6b\x65\x79\x73\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x6b\x65\x79\x73\x28\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x76\x61\x6c\x75\x65\x73\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x76\x61\x6c\x75\x65\x73\x28\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x74\x65\x6d\x73\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x69\x74\x65\x6d\x73\x28\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x70\x6f\x70\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x70\x6f\x70\x28\x6b\x65\x79\x29\x0a" },
        {"requests", "\x63\x6c\x61\x73\x73\x20\x52\x65\x73\x70\x6f\x6e\x73\x65\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x73\x74\x61\x74\x75\x73\x5f\x63\x6f\x64\x65\x2c\x20\x72\x65\x61\x73\x6f\x6e\x2c\x20\x63\x6f\x6e\x74\x65\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x74\x61\x74\x75\x73\x5f\x63\x6f\x64\x65\x20\x3d\x20\x73\x74\x61\x74\x75\x73\x5f\x63\x6f\x64\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x72\x65\x61\x73\x6f\x6e\x20\x3d\x20\x72\x65\x61\x73\x6f\x6e\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x63\x6f\x6e\x74\x65\x6e\x74\x20\x3d\x20\x63\x6f\x6e\x74\x65\x6e\x74\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x73\x65\x6c\x66\x2e\x73\x74\x61\x74\x75\x73\x5f\x63\x6f\x64\x65\x29\x20\x69\x73\x20\x69\x6e\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x73\x65\x6c\x66\x2e\x72\x65\x61\x73\x6f\x6e\x29\x20\x69\x73\x20\x73\x74\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x73\x65\x6c\x66\x2e\x63\x6f\x6e\x74\x65\x6e\x74\x29\x20\x69\x73\x20\x62\x79\x74\x65\x73\x0a\x0a\x20\x20\x20\x20\x40\x70\x72\x6f\x70\x65\x72\x74\x79\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x74\x65\x78\x74\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x63\x6f\x6e\x74\x65\x6e\x74\x2e\x64\x65\x63\x6f\x64\x65\x28\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6f\x64\x65\x20\x3d\x20\x73\x65\x6c\x66\x2e\x73\x74\x61\x74\x75\x73\x5f\x63\x6f\x64\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x27\x3c\x52\x65\x73\x70\x6f\x6e\x73\x65\x20\x5b\x7b\x63\x6f\x64\x65\x7d\x5d\x3e\x27\x0a\x0a\x64\x65\x66\x20\x5f\x70\x61\x72\x73\x65\x5f\x68\x28\x68\x65\x61\x64\x65\x72\x73\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x65\x61\x64\x65\x72\x73\x20\x69\x73\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x5d\x0a\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x68\x65\x61\x64\x65\x72\x73\x29\x20\x69\x73\x20\x64\x69\x63\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x69\x73\x74\x28\x68\x65\x61\x64\x65\x72\x73\x2e\x69\x74\x65\x6d\x73\x28\x29\x29\x0a\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x27\x68\x65\x61\x64\x65\x72\x73\x20\x6d\x75\x73\x74\x20\x62\x65\x20\x64\x69\x63\x74\x20\x6f\x72\x20\x4e\x6f\x6e\x65\x27\x29\x0a\x0a\x64\x65\x66\x20\x67\x65\x74\x28\x75\x72\x6c\x2c\x20\x68\x65\x61\x64\x65\x72\x73\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x68\x65\x61\x64\x65\x72\x73\x20\x3d\x20\x5f\x70\x61\x72\x73\x65\x5f\x68\x28\x68\x65\x61\x64\x65\x72\x73\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x72\x65\x71\x75\x65\x73\x74\x28\x27\x47\x45\x54\x27\x2c\x20\x75\x72\x6c\x2c\x20\x68\x65\x61\x64\x65\x72\x73\x2c\x20\x4e\x6f\x6e\x65\x29\x0a\x0a\x64\x65\x66\x20\x70\x6f\x73\x74\x28\x75\x72\x6c\x2c\x20\x64\x61\x74\x61\x3a\x20\x62\x79\x74\x65\x73\x2c\x20\x68\x65\x61\x64\x65\x72\x73\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x68\x65\x61\x64\x65\x72\x73\x20\x3d\x20\x5f\x70\x61\x72\x73\x65\x5f\x68\x28\x68\x65\x61\x64\x65\x72\x73\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x72\x65\x71\x75\x65\x73\x74\x28\x27\x50\x4f\x53\x54\x27\x2c\x20\x75\x72\x6c\x2c\x20\x68\x65\x61\x64\x65\x72\x73\x2c\x20\x64\x61\x74\x61\x29\x0a\x0a\x64\x65\x66\x20\x70\x75\x74\x28\x75\x72\x6c\x2c\x20\x64\x61\x74\x61\x3a\x20\x62\x79\x74\x65\x73\x2c\x20\x68\x65\x61\x64\x65\x72\x73\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x68\x65\x61\x64\x65\x72\x73\x20\x3d\x20\x5f\x70\x61\x72\x73\x65\x5f\x68\x28\x68\x65\x61\x64\x65\x72\x73\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x72\x65\x71\x75\x65\x73\x74\x28\x27\x50\x55\x54\x27\x2c\x20\x75\x72\x6c\x2c\x20\x68\x65\x61\x64\x65\x72\x73\x2c\x20\x64\x61\x74\x61\x29\x0a\x0a\x64\x65\x66\x20\x64\x65\x6c\x65\x74\x65\x28\x75\x72\x6c\x2c\x20\x68\x65\x61\x64\x65\x72\x73\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x68\x65\x61\x64\x65\x72\x73\x20\x3d\x20\x5f\x70\x61\x72\x73\x65\x5f\x68\x28\x68\x65\x61\x64\x65\x72\x73\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x72\x65\x71\x75\x65\x73\x74\x28\x27\x44\x45\x4c\x45\x54\x45\x27\x2c\x20\x75\x72\x6c\x2c\x20\x68\x65\x61\x64\x65\x72\x73\x2c\x20\x4e\x6f\x6e\x65\x29" },

    };
}   // namespace pkpy



namespace pkpy {

#define PY_CLASS(T, mod, name)                  \
    static Type _type(VM* vm) {                 \
        static const StrName __x0(#mod);        \
        static const StrName __x1(#name);       \
        return OBJ_GET(Type, vm->_modules[__x0]->attr(__x1));               \
    }                                                                       \
    static void _check_type(VM* vm, PyObject* val){                         \
        if(!vm->isinstance(val, T::_type(vm))){                             \
            vm->TypeError("expected '" #mod "." #name "', got " + OBJ_NAME(vm->_t(val)).escape());  \
        }                                                                   \
    }                                                                       \
    static PyObject* register_class(VM* vm, PyObject* mod) {                \
        if(OBJ_NAME(mod) != #mod) {                                         \
            auto msg = fmt("register_class() failed: ", OBJ_NAME(mod), " != ", #mod); \
            throw std::runtime_error(msg);                                  \
        }                                                                   \
        PyObject* type = vm->new_type_object(mod, #name, vm->tp_object);    \
        T::_register(vm, mod, type);                                        \
        type->attr()._try_perfect_rehash();                                 \
        return type;                                                        \
    }                                                                       

#define VAR_T(T, ...) vm->heap.gcnew<T>(T::_type(vm), T(__VA_ARGS__))

static int c99_sizeof(VM*, const Str&);

struct VoidP{
    PY_CLASS(VoidP, c, void_p)

    void* ptr;
    int base_offset;
    VoidP(void* ptr): ptr(ptr), base_offset(1){}
    VoidP(): ptr(nullptr), base_offset(1){}

    bool operator==(const VoidP& other) const {
        return ptr == other.ptr && base_offset == other.base_offset;
    }
    bool operator!=(const VoidP& other) const {
        return ptr != other.ptr || base_offset != other.base_offset;
    }

    Str hex() const{
        std::stringstream ss;
        ss << std::hex << reinterpret_cast<intptr_t>(ptr);
        return "0x" + ss.str();
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_default_constructor<VoidP>(type);

        vm->bind_func<1>(type, "from_hex", [](VM* vm, ArgsView args){
            std::string s = CAST(Str&, args[0]).str();
            size_t size;
            intptr_t ptr = std::stoll(s, &size, 16);
            if(size != s.size()) vm->ValueError("invalid literal for void_p(): " + s);
            return VAR_T(VoidP, (void*)ptr);
        });
        vm->bind_method<0>(type, "hex", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            return VAR(self.hex());
        });

        vm->bind__repr__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            VoidP& self = _CAST(VoidP&, obj);
            std::stringstream ss;
            ss << "<void* at " << self.hex();
            if(self.base_offset != 1) ss << ", base_offset=" << self.base_offset;
            ss << ">";
            return VAR(ss.str());
        });

        vm->bind__eq__(OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            if(!is_non_tagged_type(rhs, VoidP::_type(vm))) return false;
            return _CAST(VoidP&, lhs) == _CAST(VoidP&, rhs);
        });
        vm->bind__gt__(OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            return _CAST(VoidP&, lhs).ptr > CAST(VoidP&, rhs).ptr;
        });
        vm->bind__lt__(OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            return _CAST(VoidP&, lhs).ptr < CAST(VoidP&, rhs).ptr;
        });
        vm->bind__ge__(OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            return _CAST(VoidP&, lhs).ptr >= CAST(VoidP&, rhs).ptr;
        });
        vm->bind__le__(OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            return _CAST(VoidP&, lhs).ptr <= CAST(VoidP&, rhs).ptr;
        });

        vm->bind__hash__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            VoidP& self = _CAST(VoidP&, obj);
            return reinterpret_cast<i64>(self.ptr);
        });

        vm->bind_method<1>(type, "set_base_offset", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            if(is_non_tagged_type(args[1], vm->tp_str)){
                const Str& type = _CAST(Str&, args[1]);
                self.base_offset = c99_sizeof(vm, type);
            }else{
                self.base_offset = CAST(int, args[1]);
            }
            return vm->None;
        });

        vm->bind_method<0>(type, "get_base_offset", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            return VAR(self.base_offset);
        });

        vm->bind_method<1>(type, "offset", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            i64 offset = CAST(i64, args[1]);
            return VAR_T(VoidP, (char*)self.ptr + offset * self.base_offset);
        });

        vm->bind__add__(OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            VoidP& self = _CAST(VoidP&, lhs);
            i64 offset = CAST(i64, rhs);
            return VAR_T(VoidP, (char*)self.ptr + offset);
        });

        vm->bind__sub__(OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            VoidP& self = _CAST(VoidP&, lhs);
            i64 offset = CAST(i64, rhs);
            return VAR_T(VoidP, (char*)self.ptr - offset);
        });

#define BIND_SETGET(T, name) \
        vm->bind_method<0>(type, "read_" name, [](VM* vm, ArgsView args){   \
            VoidP& self = _CAST(VoidP&, args[0]);                   \
            return VAR(*(T*)self.ptr);                              \
        });                                                         \
        vm->bind_method<1>(type, "write_" name, [](VM* vm, ArgsView args){   \
            VoidP& self = _CAST(VoidP&, args[0]);                   \
            *(T*)self.ptr = CAST(T, args[1]);                       \
            return vm->None;                                        \
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

        vm->bind_method<0>(type, "read_void_p", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            return VAR_T(VoidP, *(void**)self.ptr);
        });
        vm->bind_method<1>(type, "write_void_p", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            VoidP& other = CAST(VoidP&, args[0]);
            self.ptr = other.ptr;
            return vm->None;
        });

        vm->bind_method<1>(type, "read_bytes", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            i64 size = CAST(i64, args[1]);
            std::vector<char> buffer(size);
            memcpy(buffer.data(), self.ptr, size);
            return VAR(Bytes(std::move(buffer)));
        });

        vm->bind_method<1>(type, "write_bytes", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            Bytes& bytes = CAST(Bytes&, args[1]);
            memcpy(self.ptr, bytes.data(), bytes.size());
            return vm->None;
        });
    }
};

struct C99Struct{
    PY_CLASS(C99Struct, c, struct)

    static constexpr int INLINE_SIZE = 24;

    char _inlined[INLINE_SIZE];
    char* p;
    int size;

    void _init(int size){
        this->size = size;
        if(size <= INLINE_SIZE){
            p = _inlined;
        }else{
            p = (char*)malloc(size);
        }
    }

    template<typename T>
    C99Struct(const T& data){
        static_assert(std::is_pod_v<T>);
        static_assert(!std::is_pointer_v<T>);
        _init(sizeof(T));
        memcpy(p, &data, this->size);
    }

    C99Struct() { p = _inlined; }
    C99Struct(void* p, int size){
        _init(size);
        if(p!=nullptr) memcpy(this->p, p, size);
    }
    ~C99Struct(){ if(p!=_inlined) free(p); }

    C99Struct(const C99Struct& other){
        _init(other.size);
        memcpy(p, other.p, size);
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_default_constructor<C99Struct>(type);

        vm->bind_method<0>(type, "addr", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR_T(VoidP, self.p);
        });

        vm->bind_method<0>(type, "size", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR(self.size);
        });

        vm->bind_method<0>(type, "copy", [](VM* vm, ArgsView args){
            const C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR_T(C99Struct, self);
        });

        vm->bind__eq__(OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            C99Struct& self = _CAST(C99Struct&, lhs);
            if(!is_non_tagged_type(rhs, C99Struct::_type(vm))) return false;
            C99Struct& other = _CAST(C99Struct&, rhs);
            return self.size == other.size && memcmp(self.p, other.p, self.size) == 0;
        });

        // patch VoidP
        type = vm->_t(VoidP::_type(vm));

        vm->bind_method<1>(type, "read_struct", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            const Str& type = CAST(Str&, args[1]);
            int size = c99_sizeof(vm, type);
            return VAR_T(C99Struct, self.ptr, size);
        });

        vm->bind_method<1>(type, "write_struct", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            C99Struct& other = CAST(C99Struct&, args[1]);
            memcpy(self.ptr, other.p, other.size);
            return vm->None;
        });
    }
};

struct ReflField{
    std::string_view name;
    int offset;
    bool operator<(const ReflField& other) const{ return name < other.name; }
    bool operator==(const ReflField& other) const{ return name == other.name; }
    bool operator!=(const ReflField& other) const{ return name != other.name; }
    bool operator<(std::string_view other) const{ return name < other; }
    bool operator==(std::string_view other) const{ return name == other; }
    bool operator!=(std::string_view other) const{ return name != other; }
};

struct ReflType{
    std::string_view name;
    size_t size;
    std::vector<ReflField> fields;
};
inline static std::map<std::string_view, ReflType> _refl_types;

inline void add_refl_type(std::string_view name, size_t size, std::vector<ReflField> fields){
    ReflType type{name, size, std::move(fields)};
    std::sort(type.fields.begin(), type.fields.end());
    _refl_types[name] = std::move(type);
}

inline static int c99_sizeof(VM* vm, const Str& type){
    auto it = _refl_types.find(type.sv());
    if(it != _refl_types.end()) return it->second.size;
    vm->ValueError("not a valid c99 type");
    return 0;
}

struct C99ReflType final: ReflType{
    PY_CLASS(C99ReflType, c, _refl)

    C99ReflType(const ReflType& type){
        this->name = type.name;
        this->size = type.size;
        this->fields = type.fields;
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_notimplemented_constructor<C99ReflType>(type);

        vm->bind_method<0>(type, "__call__", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR_T(C99Struct, nullptr, self.size);
        });

        vm->bind_method<0>(type, "name", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR(self.name);
        });

        vm->bind_method<0>(type, "size", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR(self.size);
        });

        vm->bind__getitem__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj, PyObject* key){
            C99ReflType& self = _CAST(C99ReflType&, obj);
            const Str& name = CAST(Str&, key);
            auto it = std::lower_bound(self.fields.begin(), self.fields.end(), name.sv());
            if(it == self.fields.end() || it->name != name.sv()){
                vm->KeyError(key);
                return vm->None;
            }
            return VAR(it->offset);
        });
    }
};

static_assert(sizeof(Py_<C99Struct>) <= 64);
static_assert(sizeof(Py_<Tuple>) <= 64);

inline PyObject* py_var(VM* vm, void* p){
    return VAR_T(VoidP, p);
}

inline PyObject* py_var(VM* vm, char* p){
    return VAR_T(VoidP, p);
}
/***********************************************/
template<typename T>
T to_void_p(VM* vm, PyObject* var){
    static_assert(std::is_pointer_v<T>);
    VoidP& p = CAST(VoidP&, var);
    return reinterpret_cast<T>(p.ptr);
}

template<typename T>
T to_c99_struct(VM* vm, PyObject* var){
    static_assert(std::is_pod_v<T>);
    C99Struct& pod = CAST(C99Struct&, var);
    return *reinterpret_cast<T*>(pod.p);
}

template<typename T>
std::enable_if_t<std::is_pod_v<T> && !std::is_pointer_v<T>, PyObject*> py_var(VM* vm, const T& data){
    return VAR_T(C99Struct, data);
}
/*****************************************************************/
struct NativeProxyFuncCBase {
    virtual PyObject* operator()(VM* vm, ArgsView args) = 0;

    static void check_args_size(VM* vm, ArgsView args, int n){
        if (args.size() != n){
            vm->TypeError("expected " + std::to_string(n) + " arguments, but got " + std::to_string(args.size()));
        }
    }
};

template<typename Ret, typename... Params>
struct NativeProxyFuncC final: NativeProxyFuncCBase {
    static constexpr int N = sizeof...(Params);
    using _Fp = Ret(*)(Params...);
    _Fp func;
    NativeProxyFuncC(_Fp func) : func(func) {}

    PyObject* operator()(VM* vm, ArgsView args) override {
        check_args_size(vm, args, N);
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

inline PyObject* _any_c_wrapper(VM* vm, ArgsView args){
    NativeProxyFuncCBase* pf = lambda_get_userdata<NativeProxyFuncCBase*>(args.begin());
    return (*pf)(vm, args);
}

template<typename T>
inline void bind_any_c_fp(VM* vm, PyObject* obj, Str name, T fp){
    static_assert(std::is_pod_v<T>);
    static_assert(std::is_pointer_v<T>);
    auto proxy = new NativeProxyFuncC(fp);
    PyObject* func = VAR(NativeFunc(_any_c_wrapper, proxy->N, false));
    _CAST(NativeFunc&, func).set_userdata(proxy);
    obj->attr().set(name, func);
}

inline void add_module_c(VM* vm){
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

    vm->bind_func<1>(mod, "sizeof", [](VM* vm, ArgsView args){
        const Str& type = CAST(Str&, args[0]);
        i64 size = c99_sizeof(vm, type);
        return VAR(size);
    });

    vm->bind_func<1>(mod, "refl", [](VM* vm, ArgsView args){
        const Str& key = CAST(Str&, args[0]);
        auto it = _refl_types.find(key.sv());
        if(it == _refl_types.end()) vm->ValueError("reflection type not found");
        const ReflType& rt = it->second;
        return VAR_T(C99ReflType, rt);
    });

    vm->bind_func<3>(mod, "memset", [](VM* vm, ArgsView args){
        void* p = CAST(void*, args[0]);
        i64 c = CAST(i64, args[1]);
        i64 size = CAST(i64, args[2]);
        memset(p, c, size);
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
    C99ReflType::register_class(vm, mod);
    mod->attr().set("NULL", VAR_T(VoidP, nullptr));

    add_refl_type("char", sizeof(char), {});
    add_refl_type("uchar", sizeof(unsigned char), {});
    add_refl_type("short", sizeof(short), {});
    add_refl_type("ushort", sizeof(unsigned short), {});
    add_refl_type("int", sizeof(int), {});
    add_refl_type("uint", sizeof(unsigned int), {});
    add_refl_type("long", sizeof(long), {});
    add_refl_type("ulong", sizeof(unsigned long), {});
    add_refl_type("longlong", sizeof(long long), {});
    add_refl_type("ulonglong", sizeof(unsigned long long), {});
    add_refl_type("float", sizeof(float), {});
    add_refl_type("double", sizeof(double), {});
    add_refl_type("bool", sizeof(bool), {});
    add_refl_type("void_p", sizeof(void*), {});
}

}   // namespace pkpy


namespace pkpy{

struct RangeIter{
    PY_CLASS(RangeIter, builtins, "_range_iterator")
    Range r;
    i64 current;
    RangeIter(Range r) : r(r), current(r.start) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->_all_types[OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<RangeIter>(type);
        vm->bind__iter__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj){ return obj; });
        vm->bind__next__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            RangeIter& self = _CAST(RangeIter&, obj);
            bool has_next = self.r.step > 0 ? self.current < self.r.stop : self.current > self.r.stop;
            if(!has_next) return vm->StopIteration;
            self.current += self.r.step;
            return VAR(self.current - self.r.step);
        });
    }
};

struct ArrayIter{
    PY_CLASS(ArrayIter, builtins, "_array_iterator")
    PyObject* ref;
    PyObject** begin;
    PyObject** end;
    PyObject** current;

    ArrayIter(PyObject* ref, PyObject** begin, PyObject** end)
        : ref(ref), begin(begin), end(end), current(begin) {}

    void _gc_mark() const{ OBJ_MARK(ref); }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->_all_types[OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<ArrayIter>(type);
        vm->bind__iter__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj){ return obj; });
        vm->bind__next__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            ArrayIter& self = _CAST(ArrayIter&, obj);
            if(self.current == self.end) return vm->StopIteration;
            return *self.current++;
        });
    }
};

struct StringIter{
    PY_CLASS(StringIter, builtins, "_string_iterator")
    PyObject* ref;
    Str* str;
    int index;

    StringIter(PyObject* ref) : ref(ref), str(&OBJ_GET(Str, ref)), index(0) {}

    void _gc_mark() const{ OBJ_MARK(ref); }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->_all_types[OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<StringIter>(type);
        vm->bind__iter__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj){ return obj; });
        vm->bind__next__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            StringIter& self = _CAST(StringIter&, obj);
            // TODO: optimize this... operator[] is of O(n) complexity
            if(self.index == self.str->u8_length()) return vm->StopIteration;
            return VAR(self.str->u8_getitem(self.index++));
        });
    }
};

struct Generator{
    PY_CLASS(Generator, builtins, "_generator")
    Frame frame;
    int state;      // 0,1,2
    List s_backup;

    Generator(Frame&& frame, ArgsView buffer): frame(std::move(frame)), state(0) {
        for(PyObject* obj: buffer) s_backup.push_back(obj);
    }

    void _gc_mark() const{
        frame._gc_mark();
        for(PyObject* obj: s_backup) OBJ_MARK(obj);
    }

    PyObject* next(VM* vm){
        if(state == 2) return vm->StopIteration;
        // reset frame._sp_base
        frame._sp_base = frame._s->_sp;
        frame._locals.a = frame._s->_sp;
        // restore the context
        for(PyObject* obj: s_backup) frame._s->push(obj);
        s_backup.clear();
        vm->callstack.push(std::move(frame));
        PyObject* ret = vm->_run_top_frame();
        if(ret == PY_OP_YIELD){
            // backup the context
            frame = std::move(vm->callstack.top());
            PyObject* ret = frame._s->popx();
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

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->_all_types[OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<Generator>(type);
        vm->bind__iter__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj){ return obj; });
        vm->bind__next__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            Generator& self = _CAST(Generator&, obj);
            return self.next(vm);
        });
    }
};

inline PyObject* VM::_py_generator(Frame&& frame, ArgsView buffer){
    return VAR_T(Generator, std::move(frame), buffer);
}

} // namespace pkpy


#if PK_MODULE_BASE64

namespace pkpy {

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

inline static unsigned int
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

inline static unsigned int
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

inline void add_module_base64(VM* vm){
    PyObject* mod = vm->new_module("base64");

    // b64encode
    vm->bind_func<1>(mod, "b64encode", [](VM* vm, ArgsView args){
        Bytes& b = CAST(Bytes&, args[0]);
        std::vector<char> out(b.size() * 2);
        int size = base64_encode((const unsigned char*)b.data(), b.size(), out.data());
        out.resize(size);
        return VAR(Bytes(std::move(out)));
    });

    // b64decode
    vm->bind_func<1>(mod, "b64decode", [](VM* vm, ArgsView args){
        Bytes& b = CAST(Bytes&, args[0]);
        std::vector<char> out(b.size());
        int size = base64_decode(b.data(), b.size(), (unsigned char*)out.data());
        out.resize(size);
        return VAR(Bytes(std::move(out)));
    });
}

} // namespace pkpy


#else

ADD_MODULE_PLACEHOLDER(base64)

#endif


#if PK_MODULE_RANDOM

#include <random>

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
    }
};

inline void add_module_random(VM* vm){
    PyObject* mod = vm->new_module("random");
    Random::register_class(vm, mod);
    CodeObject_ code = vm->compile(kPythonLibs["random"], "random.py", EXEC_MODE);
    vm->_exec(code, mod);
}

}   // namespace pkpy

#else

ADD_MODULE_PLACEHOLDER(random)

#endif


#if PK_MODULE_RE

namespace pkpy{

struct ReMatch {
    PY_CLASS(ReMatch, re, Match)

    i64 start;
    i64 end;
    std::cmatch m;
    ReMatch(i64 start, i64 end, std::cmatch m) : start(start), end(end), m(m) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_notimplemented_constructor<ReMatch>(type);
        vm->bind_method<0>(type, "start", CPP_LAMBDA(VAR(_CAST(ReMatch&, args[0]).start)));
        vm->bind_method<0>(type, "end", CPP_LAMBDA(VAR(_CAST(ReMatch&, args[0]).end)));

        vm->bind_method<0>(type, "span", [](VM* vm, ArgsView args) {
            auto& self = _CAST(ReMatch&, args[0]);
            return VAR(Tuple({VAR(self.start), VAR(self.end)}));
        });

        vm->bind_method<1>(type, "group", [](VM* vm, ArgsView args) {
            auto& self = _CAST(ReMatch&, args[0]);
            int index = CAST(int, args[1]);
            index = vm->normalized_index(index, self.m.size());
            return VAR(self.m[index].str());
        });
    }
};

inline PyObject* _regex_search(const Str& pattern, const Str& string, bool from_start, VM* vm){
    std::regex re(pattern.begin(), pattern.end());
    std::cmatch m;
    if(std::regex_search(string.begin(), string.end(), m, re)){
        if(from_start && m.position() != 0) return vm->None;
        i64 start = string._byte_index_to_unicode(m.position());
        i64 end = string._byte_index_to_unicode(m.position() + m.length());
        return VAR_T(ReMatch, start, end, m);
    }
    return vm->None;
};

inline void add_module_re(VM* vm){
    PyObject* mod = vm->new_module("re");
    ReMatch::register_class(vm, mod);

    vm->bind_func<2>(mod, "match", [](VM* vm, ArgsView args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& string = CAST(Str&, args[1]);
        return _regex_search(pattern, string, true, vm);
    });

    vm->bind_func<2>(mod, "search", [](VM* vm, ArgsView args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& string = CAST(Str&, args[1]);
        return _regex_search(pattern, string, false, vm);
    });

    vm->bind_func<3>(mod, "sub", [](VM* vm, ArgsView args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& repl = CAST(Str&, args[1]);
        const Str& string = CAST(Str&, args[2]);
        std::regex re(pattern.begin(), pattern.end());
        return VAR(std::regex_replace(string.str(), re, repl.str()));
    });

    vm->bind_func<2>(mod, "split", [](VM* vm, ArgsView args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& string = CAST(Str&, args[1]);
        std::regex re(pattern.begin(), pattern.end());
        std::cregex_token_iterator it(string.begin(), string.end(), re, -1);
        std::cregex_token_iterator end;
        List vec;
        for(; it != end; ++it){
            vec.push_back(VAR(it->str()));
        }
        return VAR(vec);
    });
}

}   // namespace pkpy

#else

ADD_MODULE_PLACEHOLDER(re)

#endif


#if PK_MODULE_LINALG

namespace pkpy{

static constexpr float kEpsilon = 1e-4f;
inline static bool isclose(float a, float b){ return fabsf(a - b) < kEpsilon; }

struct Vec2{
    float x, y;
    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2(const Vec2& v) : x(v.x), y(v.y) {}

    Vec2 operator+(const Vec2& v) const { return Vec2(x + v.x, y + v.y); }
    Vec2& operator+=(const Vec2& v) { x += v.x; y += v.y; return *this; }
    Vec2 operator-(const Vec2& v) const { return Vec2(x - v.x, y - v.y); }
    Vec2& operator-=(const Vec2& v) { x -= v.x; y -= v.y; return *this; }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }
    Vec2 operator/(float s) const { return Vec2(x / s, y / s); }
    Vec2& operator/=(float s) { x /= s; y /= s; return *this; }
    Vec2 operator-() const { return Vec2(-x, -y); }
    bool operator==(const Vec2& v) const { return isclose(x, v.x) && isclose(y, v.y); }
    bool operator!=(const Vec2& v) const { return !isclose(x, v.x) || !isclose(y, v.y); }
    float dot(const Vec2& v) const { return x * v.x + y * v.y; }
    float cross(const Vec2& v) const { return x * v.y - y * v.x; }
    float length() const { return sqrtf(x * x + y * y); }
    float length_squared() const { return x * x + y * y; }
    Vec2 normalize() const { float l = length(); return Vec2(x / l, y / l); }
};

struct Vec3{
    float x, y, z;
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3(const Vec3& v) : x(v.x), y(v.y), z(v.z) {}

    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }
    Vec3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    bool operator==(const Vec3& v) const { return isclose(x, v.x) && isclose(y, v.y) && isclose(z, v.z); }
    bool operator!=(const Vec3& v) const { return !isclose(x, v.x) || !isclose(y, v.y) || !isclose(z, v.z); }
    float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3 cross(const Vec3& v) const { return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
    float length() const { return sqrtf(x * x + y * y + z * z); }
    float length_squared() const { return x * x + y * y + z * z; }
    Vec3 normalize() const { float l = length(); return Vec3(x / l, y / l, z / l); }
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

    Mat3x3() {}
    Mat3x3(float _11, float _12, float _13,
           float _21, float _22, float _23,
           float _31, float _32, float _33)
        : _11(_11), _12(_12), _13(_13)
        , _21(_21), _22(_22), _23(_23)
        , _31(_31), _32(_32), _33(_33) {}

    void set_zeros(){ for (int i=0; i<9; ++i) v[i] = 0.0f; }
    void set_ones(){ for (int i=0; i<9; ++i) v[i] = 1.0f; }
    void set_identity(){ set_zeros(); _11 = _22 = _33 = 1.0f; }

    static Mat3x3 zeros(){
        static Mat3x3 ret(0, 0, 0, 0, 0, 0, 0, 0, 0);
        return ret;
    }

    static Mat3x3 ones(){
        static Mat3x3 ret(1, 1, 1, 1, 1, 1, 1, 1, 1);
        return ret;
    }

    static Mat3x3 identity(){
        static Mat3x3 ret(1, 0, 0, 0, 1, 0, 0, 0, 1);
        return ret;
    }

    Mat3x3 operator+(const Mat3x3& other) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] + other.v[i];
        return ret;
    }

    Mat3x3 operator-(const Mat3x3& other) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] - other.v[i];
        return ret;
    }

    Mat3x3 operator*(float scalar) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] * scalar;
        return ret;
    }

    Mat3x3 operator/(float scalar) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] / scalar;
        return ret;
    }

    Mat3x3& operator+=(const Mat3x3& other){ 
        for (int i=0; i<9; ++i) v[i] += other.v[i];
        return *this;
    }

    Mat3x3& operator-=(const Mat3x3& other){ 
        for (int i=0; i<9; ++i) v[i] -= other.v[i];
        return *this;
    }

    Mat3x3& operator*=(float scalar){ 
        for (int i=0; i<9; ++i) v[i] *= scalar;
        return *this;
    }

    Mat3x3& operator/=(float scalar){ 
        for (int i=0; i<9; ++i) v[i] /= scalar;
        return *this;
    }

    Mat3x3 matmul(const Mat3x3& other) const{
        Mat3x3 ret;
        ret._11 = _11 * other._11 + _12 * other._21 + _13 * other._31;
        ret._12 = _11 * other._12 + _12 * other._22 + _13 * other._32;
        ret._13 = _11 * other._13 + _12 * other._23 + _13 * other._33;
        ret._21 = _21 * other._11 + _22 * other._21 + _23 * other._31;
        ret._22 = _21 * other._12 + _22 * other._22 + _23 * other._32;
        ret._23 = _21 * other._13 + _22 * other._23 + _23 * other._33;
        ret._31 = _31 * other._11 + _32 * other._21 + _33 * other._31;
        ret._32 = _31 * other._12 + _32 * other._22 + _33 * other._32;
        ret._33 = _31 * other._13 + _32 * other._23 + _33 * other._33;
        return ret;
    }

    Vec3 matmul(const Vec3& other) const{
        Vec3 ret;
        ret.x = _11 * other.x + _12 * other.y + _13 * other.z;
        ret.y = _21 * other.x + _22 * other.y + _23 * other.z;
        ret.z = _31 * other.x + _32 * other.y + _33 * other.z;
        return ret;
    }

    bool operator==(const Mat3x3& other) const{
        for (int i=0; i<9; ++i){
            if (!isclose(v[i], other.v[i])) return false;
        }
        return true;
    }

    bool operator!=(const Mat3x3& other) const{
        for (int i=0; i<9; ++i){
            if (!isclose(v[i], other.v[i])) return true;
        }
        return false;
    }

    float determinant() const{
        return _11 * _22 * _33 + _12 * _23 * _31 + _13 * _21 * _32
             - _11 * _23 * _32 - _12 * _21 * _33 - _13 * _22 * _31;
    }

    Mat3x3 transpose() const{
        Mat3x3 ret;
        ret._11 = _11;  ret._12 = _21;  ret._13 = _31;
        ret._21 = _12;  ret._22 = _22;  ret._23 = _32;
        ret._31 = _13;  ret._32 = _23;  ret._33 = _33;
        return ret;
    }

    bool inverse(Mat3x3& ret) const{
        float det = determinant();
        if (fabsf(det) < kEpsilon) return false;
        float inv_det = 1.0f / det;
        ret._11 = (_22 * _33 - _23 * _32) * inv_det;
        ret._12 = (_13 * _32 - _12 * _33) * inv_det;
        ret._13 = (_12 * _23 - _13 * _22) * inv_det;
        ret._21 = (_23 * _31 - _21 * _33) * inv_det;
        ret._22 = (_11 * _33 - _13 * _31) * inv_det;
        ret._23 = (_13 * _21 - _11 * _23) * inv_det;
        ret._31 = (_21 * _32 - _22 * _31) * inv_det;
        ret._32 = (_12 * _31 - _11 * _32) * inv_det;
        ret._33 = (_11 * _22 - _12 * _21) * inv_det;
        return true;
    }

    /*************** affine transformations ***************/
    static Mat3x3 trs(Vec2 t, float radian, Vec2 s){
        float cr = cosf(radian);
        float sr = sinf(radian);
        return Mat3x3(s.x * cr,   -s.y * sr,  t.x,
                      s.x * sr,   s.y * cr,   t.y,
                      0.0f,       0.0f,       1.0f);
    }

    bool is_affine() const{
        float det = _11 * _22 - _12 * _21;
        if(fabsf(det) < kEpsilon) return false;
        return _31 == 0.0f && _32 == 0.0f && _33 == 1.0f;
    }

    Mat3x3 inverse_affine() const{
        Mat3x3 ret;
        float det = _11 * _22 - _12 * _21;
        float inv_det = 1.0f / det;
        ret._11 = _22 * inv_det;
        ret._12 = -_12 * inv_det;
        ret._13 = (_12 * _23 - _13 * _22) * inv_det;
        ret._21 = -_21 * inv_det;
        ret._22 = _11 * inv_det;
        ret._23 = (_13 * _21 - _11 * _23) * inv_det;
        ret._31 = 0.0f;
        ret._32 = 0.0f;
        ret._33 = 1.0f;
        return ret;
    }

    Mat3x3 matmul_affine(const Mat3x3& other) const{
        Mat3x3 ret;
        ret._11 = _11 * other._11 + _12 * other._21;
        ret._12 = _11 * other._12 + _12 * other._22;
        ret._13 = _11 * other._13 + _12 * other._23 + _13;
        ret._21 = _21 * other._11 + _22 * other._21;
        ret._22 = _21 * other._12 + _22 * other._22;
        ret._23 = _21 * other._13 + _22 * other._23 + _23;
        ret._31 = 0.0f;
        ret._32 = 0.0f;
        ret._33 = 1.0f;
        return ret;
    }

    Vec2 translation() const { return Vec2(_13, _23); }
    float rotation() const { return atan2f(_21, _11); }
    Vec2 scale() const {
        return Vec2(
            sqrtf(_11 * _11 + _21 * _21),
            sqrtf(_12 * _12 + _22 * _22)
        );
    }

    Vec2 transform_point(Vec2 v) const {
        return Vec2(_11 * v.x + _12 * v.y + _13, _21 * v.x + _22 * v.y + _23);
    }

    Vec2 transform_vector(Vec2 v) const {
        return Vec2(_11 * v.x + _12 * v.y, _21 * v.x + _22 * v.y);
    }
};

struct PyVec2;
struct PyVec3;
struct PyMat3x3;
PyObject* py_var(VM*, Vec2);
PyObject* py_var(VM*, const PyVec2&);
PyObject* py_var(VM*, Vec3);
PyObject* py_var(VM*, const PyVec3&);
PyObject* py_var(VM*, const Mat3x3&);
PyObject* py_var(VM*, const PyMat3x3&);

#define BIND_VEC_VEC_OP(D, name, op)                                        \
        vm->bind_method<1>(type, #name, [](VM* vm, ArgsView args){          \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                     \
            PyVec##D& other = CAST(PyVec##D&, args[1]);                     \
            return VAR(self op other);                                      \
        });

#define BIND_VEC_FLOAT_OP(D, name, op)  \
        vm->bind_method<1>(type, #name, [](VM* vm, ArgsView args){          \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                     \
            f64 other = vm->num_to_float(args[1]);                          \
            return VAR(self op other);                                      \
        });

#define BIND_VEC_FUNCTION_0(D, name)        \
        vm->bind_method<0>(type, #name, [](VM* vm, ArgsView args){          \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                     \
            return VAR(self.name());                                        \
        });

#define BIND_VEC_FUNCTION_1(D, name)        \
        vm->bind_method<0>(type, #name, [](VM* vm, ArgsView args){          \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                     \
            PyVec##D& other = CAST(PyVec##D&, args[1]);                     \
            return VAR(self.name(other));                                   \
        });

#define BIND_VEC_FIELD(D, name)  \
        type->attr().set(#name, vm->property([](VM* vm, ArgsView args){     \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                     \
            return VAR(self.name);                                          \
        }, [](VM* vm, ArgsView args){                                       \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                     \
            self.name = vm->num_to_float(args[1]);                          \
            return vm->None;                                                \
        }));

struct PyVec2: Vec2 {
    PY_CLASS(PyVec2, linalg, vec2)

    PyVec2() : Vec2() {}
    PyVec2(const Vec2& v) : Vec2(v) {}
    PyVec2(const PyVec2& v) : Vec2(v) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<3>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            return VAR(Vec2(x, y));
        });

        vm->bind__repr__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            PyVec2& self = _CAST(PyVec2&, obj);
            std::stringstream ss;
            ss << "vec2(" << self.x << ", " << self.y << ")";
            return VAR(ss.str());
        });

        vm->bind_method<0>(type, "copy", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            return VAR_T(PyVec2, self);
        });

        vm->bind_method<1>(type, "rotate", [](VM* vm, ArgsView args){
            Vec2 self = _CAST(PyVec2&, args[0]);
            float radian = vm->num_to_float(args[1]);
            float cr = cosf(radian);
            float sr = sinf(radian);
            Mat3x3 rotate(cr,   -sr,  0.0f,
                          sr,   cr,   0.0f,
                          0.0f, 0.0f, 1.0f);
            self = rotate.transform_vector(self);
            return VAR(self);
        });

        BIND_VEC_VEC_OP(2, __add__, +)
        BIND_VEC_VEC_OP(2, __sub__, -)
        BIND_VEC_FLOAT_OP(2, __mul__, *)
        BIND_VEC_FLOAT_OP(2, __truediv__, /)
        BIND_VEC_VEC_OP(2, __eq__, ==)
        BIND_VEC_FIELD(2, x)
        BIND_VEC_FIELD(2, y)
        BIND_VEC_FUNCTION_1(2, dot)
        BIND_VEC_FUNCTION_1(2, cross)
        BIND_VEC_FUNCTION_0(2, length)
        BIND_VEC_FUNCTION_0(2, length_squared)
        BIND_VEC_FUNCTION_0(2, normalize)
    }
};

struct PyVec3: Vec3 {
    PY_CLASS(PyVec3, linalg, vec3)

    PyVec3() : Vec3() {}
    PyVec3(const Vec3& v) : Vec3(v) {}
    PyVec3(const PyVec3& v) : Vec3(v) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<4>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            float z = CAST_F(args[3]);
            return VAR(Vec3(x, y, z));
        });

        vm->bind__repr__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            PyVec3& self = _CAST(PyVec3&, obj);
            std::stringstream ss;
            ss << "vec3(" << self.x << ", " << self.y << ", " << self.z << ")";
            return VAR(ss.str());
        });

        vm->bind_method<0>(type, "copy", [](VM* vm, ArgsView args){
            PyVec3& self = _CAST(PyVec3&, args[0]);
            return VAR_T(PyVec3, self);
        });

        BIND_VEC_VEC_OP(3, __add__, +)
        BIND_VEC_VEC_OP(3, __sub__, -)
        BIND_VEC_FLOAT_OP(3, __mul__, *)
        BIND_VEC_FLOAT_OP(3, __truediv__, /)
        BIND_VEC_VEC_OP(3, __eq__, ==)
        BIND_VEC_FIELD(3, x)
        BIND_VEC_FIELD(3, y)
        BIND_VEC_FIELD(3, z)
        BIND_VEC_FUNCTION_1(3, dot)
        BIND_VEC_FUNCTION_1(3, cross)
        BIND_VEC_FUNCTION_0(3, length)
        BIND_VEC_FUNCTION_0(3, length_squared)
        BIND_VEC_FUNCTION_0(3, normalize)
    }
};

struct PyMat3x3: Mat3x3{
    PY_CLASS(PyMat3x3, linalg, mat3x3)

    PyMat3x3(): Mat3x3(){}
    PyMat3x3(const Mat3x3& other): Mat3x3(other){}
    PyMat3x3(const PyMat3x3& other): Mat3x3(other){}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<-1>(type, [](VM* vm, ArgsView args){
            if(args.size() == 1+0) return VAR_T(PyMat3x3, Mat3x3::zeros());
            if(args.size() == 1+9){
                Mat3x3 mat;
                for(int i=0; i<9; i++) mat.v[i] = CAST_F(args[1+i]);
                return VAR_T(PyMat3x3, mat);
            }
            if(args.size() == 1+1){
                List& a = CAST(List&, args[1]);
                if(a.size() != 3) vm->ValueError("Mat3x3.__new__ takes 3x3 list");
                Mat3x3 mat;
                for(int i=0; i<3; i++){
                    List& b = CAST(List&, a[i]);
                    if(b.size() != 3) vm->ValueError("Mat3x3.__new__ takes 3x3 list");
                    for(int j=0; j<3; j++){
                        mat.m[i][j] = CAST_F(b[j]);
                    }
                }
                return VAR_T(PyMat3x3, mat);
            }
            vm->TypeError("Mat3x3.__new__ takes 0 or 1 arguments");
            return vm->None;
        });

#define METHOD_PROXY_NONE(name)  \
        vm->bind_method<0>(type, #name, [](VM* vm, ArgsView args){    \
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);               \
            self.name();                                              \
            return vm->None;                                          \
        });

        METHOD_PROXY_NONE(set_zeros)
        METHOD_PROXY_NONE(set_ones)
        METHOD_PROXY_NONE(set_identity)

#undef METHOD_PROXY_NONE

        vm->bind__repr__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            PyMat3x3& self = _CAST(PyMat3x3&, obj);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(4);
            ss << "mat3x3([[" << self._11 << ", " << self._12 << ", " << self._13 << "],\n";
            ss << "        [" << self._21 << ", " << self._22 << ", " << self._23 << "],\n";
            ss << "        [" << self._31 << ", " << self._32 << ", " << self._33 << "]])";
            return VAR(ss.str());
        });

        vm->bind_method<0>(type, "copy", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyMat3x3, self);
        });

        vm->bind__getitem__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj, PyObject* index){
            PyMat3x3& self = _CAST(PyMat3x3&, obj);
            Tuple& t = CAST(Tuple&, index);
            if(t.size() != 2){
                vm->TypeError("Mat3x3.__getitem__ takes a tuple of 2 integers");
                return vm->None;
            }
            i64 i = CAST(i64, t[0]);
            i64 j = CAST(i64, t[1]);
            if(i < 0 || i >= 3 || j < 0 || j >= 3){
                vm->IndexError("index out of range");
                return vm->None;
            }
            return VAR(self.m[i][j]);
        });

        vm->bind_method<2>(type, "__setitem__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            Tuple& t = CAST(Tuple&, args[1]);
            if(t.size() != 2){
                vm->TypeError("Mat3x3.__setitem__ takes a tuple of 2 integers");
                return vm->None;
            }
            i64 i = CAST(i64, t[0]);
            i64 j = CAST(i64, t[1]);
            if(i < 0 || i >= 3 || j < 0 || j >= 3){
                vm->IndexError("index out of range");
                return vm->None;
            }
            self.m[i][j] = CAST_F(args[2]);
            return vm->None;
        });

#define PROPERTY_FIELD(field) \
        type->attr().set(#field, vm->property([](VM* vm, ArgsView args){    \
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);                     \
            return VAR(self.field);                                         \
        }, [](VM* vm, ArgsView args){                                       \
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);                     \
            self.field = vm->num_to_float(args[1]);                         \
            return vm->None;                                                \
        }));

        PROPERTY_FIELD(_11)
        PROPERTY_FIELD(_12)
        PROPERTY_FIELD(_13)
        PROPERTY_FIELD(_21)
        PROPERTY_FIELD(_22)
        PROPERTY_FIELD(_23)
        PROPERTY_FIELD(_31)
        PROPERTY_FIELD(_32)
        PROPERTY_FIELD(_33)

#undef PROPERTY_FIELD

        vm->bind_method<1>(type, "__add__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyMat3x3& other = CAST(PyMat3x3&, args[1]);
            return VAR_T(PyMat3x3, self + other);
        });

        vm->bind_method<1>(type, "__sub__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyMat3x3& other = CAST(PyMat3x3&, args[1]);
            return VAR_T(PyMat3x3, self - other);
        });

        vm->bind_method<1>(type, "__mul__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            f64 other = CAST_F(args[1]);
            return VAR_T(PyMat3x3, self * other);
        });

        vm->bind_method<1>(type, "__truediv__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            f64 other = CAST_F(args[1]);
            return VAR_T(PyMat3x3, self / other);
        });

        auto f_mm = [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            if(is_non_tagged_type(args[1], PyMat3x3::_type(vm))){
                PyMat3x3& other = _CAST(PyMat3x3&, args[1]);
                return VAR_T(PyMat3x3, self.matmul(other));
            }
            if(is_non_tagged_type(args[1], PyVec3::_type(vm))){
                PyVec3& other = _CAST(PyVec3&, args[1]);
                return VAR_T(PyVec3, self.matmul(other));
            }
            vm->TypeError("unsupported operand type(s) for @");
            return vm->None;
        };

        vm->bind_method<1>(type, "__matmul__", f_mm);
        vm->bind_method<1>(type, "matmul", f_mm);

        vm->bind_method<1>(type, "__eq__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyMat3x3& other = CAST(PyMat3x3&, args[1]);
            return VAR(self == other);
        });

        vm->bind_method<0>(type, "determinant", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR(self.determinant());
        });

        vm->bind_method<0>(type, "transpose", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyMat3x3, self.transpose());
        });

        vm->bind_method<0>(type, "inverse", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            Mat3x3 ret;
            bool ok = self.inverse(ret);
            if(!ok) vm->ValueError("matrix is not invertible");
            return VAR_T(PyMat3x3, ret);
        });

        vm->bind_func<0>(type, "zeros", [](VM* vm, ArgsView args){
            return VAR_T(PyMat3x3, Mat3x3::zeros());
        });

        vm->bind_func<0>(type, "ones", [](VM* vm, ArgsView args){
            return VAR_T(PyMat3x3, Mat3x3::ones());
        });

        vm->bind_func<0>(type, "identity", [](VM* vm, ArgsView args){
            return VAR_T(PyMat3x3, Mat3x3::identity());
        });

        /*************** affine transformations ***************/
        vm->bind_func<3>(type, "trs", [](VM* vm, ArgsView args){
            PyVec2& t = CAST(PyVec2&, args[0]);
            f64 r = CAST_F(args[1]);
            PyVec2& s = CAST(PyVec2&, args[2]);
            return VAR_T(PyMat3x3, Mat3x3::trs(t, r, s));
        });

        vm->bind_method<0>(type, "is_affine", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR(self.is_affine());
        });

        vm->bind_method<0>(type, "inverse_affine", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyMat3x3, self.inverse_affine());
        });

        vm->bind_method<1>(type, "matmul_affine", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyMat3x3& other = CAST(PyMat3x3&, args[1]);
            return VAR_T(PyMat3x3, self.matmul_affine(other));
        });


        vm->bind_method<0>(type, "translation", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyVec2, self.translation());
        });

        vm->bind_method<0>(type, "rotation", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR(self.rotation());
        });

        vm->bind_method<0>(type, "scale", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyVec2, self.scale());
        });

        vm->bind_method<1>(type, "transform_point", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyVec2& v = CAST(PyVec2&, args[1]);
            return VAR_T(PyVec2, self.transform_point(v));
        });

        vm->bind_method<1>(type, "transform_vector", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyVec2& v = CAST(PyVec2&, args[1]);
            return VAR_T(PyVec2, self.transform_vector(v));
        });
    }
};

inline PyObject* py_var(VM* vm, Vec2 obj){ return VAR_T(PyVec2, obj); }
inline PyObject* py_var(VM* vm, const PyVec2& obj){ return VAR_T(PyVec2, obj);}

inline PyObject* py_var(VM* vm, Vec3 obj){ return VAR_T(PyVec3, obj); }
inline PyObject* py_var(VM* vm, const PyVec3& obj){ return VAR_T(PyVec3, obj);}

inline PyObject* py_var(VM* vm, const Mat3x3& obj){ return VAR_T(PyMat3x3, obj); }
inline PyObject* py_var(VM* vm, const PyMat3x3& obj){ return VAR_T(PyMat3x3, obj); }

inline void add_module_linalg(VM* vm){
    PyObject* linalg = vm->new_module("linalg");
    PyVec2::register_class(vm, linalg);
    PyVec3::register_class(vm, linalg);
    PyMat3x3::register_class(vm, linalg);
}

static_assert(sizeof(Py_<PyMat3x3>) <= 64);

}   // namespace pkpy

#else

ADD_MODULE_PLACEHOLDER(linalg)

#endif


#if PK_MODULE_EASING

namespace pkpy{

// https://easings.net/

static const double PI = 3.1415926545;

inline static double easeLinear( double x ) {
    return x;
}

inline static double easeInSine( double x ) {
    return 1.0 - std::cos( x * PI / 2 );
}

inline static double easeOutSine( double x ) {
	return std::sin( x * PI / 2 );
}

inline static double easeInOutSine( double x ) {
	return -( std::cos( PI * x ) - 1 ) / 2;
}

inline static double easeInQuad( double x ) {
    return x * x;
}

inline static double easeOutQuad( double x ) {
    return 1 - std::pow( 1 - x, 2 );
}

inline static double easeInOutQuad( double x ) {
    if( x < 0.5 ) {
        return 2 * x * x;
    } else {
        return 1 - std::pow( -2 * x + 2, 2 ) / 2;
    }
}

inline static double easeInCubic( double x ) {
    return x * x * x;
}

inline static double easeOutCubic( double x ) {
    return 1 - std::pow( 1 - x, 3 );
}

inline static double easeInOutCubic( double x ) {
    if( x < 0.5 ) {
        return 4 * x * x * x;
    } else {
        return 1 - std::pow( -2 * x + 2, 3 ) / 2;
    }
}

inline static double easeInQuart( double x ) {
    return std::pow( x, 4 );
}

inline static double easeOutQuart( double x ) {
    return 1 - std::pow( 1 - x, 4 );
}

inline static double easeInOutQuart( double x ) {
    if( x < 0.5 ) {
        return 8 * std::pow( x, 4 );
    } else {
        return 1 - std::pow( -2 * x + 2, 4 ) / 2;
    }
}

inline static double easeInQuint( double x ) {
    return std::pow( x, 5 );
}

inline static double easeOutQuint( double x ) {
    return 1 - std::pow( 1 - x, 5 );
}

inline static double easeInOutQuint( double x ) {
    if( x < 0.5 ) {
        return 16 * std::pow( x, 5 );
    } else {
        return 1 - std::pow( -2 * x + 2, 5 ) / 2;
    }
}

inline static double easeInExpo( double x ) {
    return x == 0 ? 0 : std::pow( 2, 10 * x - 10 );
}

inline static double easeOutExpo( double x ) {
    return x == 1 ? 1 : 1 - std::pow( 2, -10 * x );
}

inline double easeInOutExpo( double x ) {
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

inline static double easeInCirc( double x ) {
    return 1 - std::sqrt( 1 - std::pow( x, 2 ) );
}

inline static double easeOutCirc( double x ) {
    return std::sqrt( 1 - std::pow( x - 1, 2 ) );
}

inline static double easeInOutCirc( double x ) {
    if( x < 0.5 ) {
        return (1 - std::sqrt( 1 - std::pow( 2 * x, 2 ) )) / 2;
    } else {
        return (std::sqrt( 1 - std::pow( -2 * x + 2, 2 ) ) + 1) / 2;
    }
}

inline static double easeInBack( double x ) {
    const double c1 = 1.70158;
    const double c3 = c1 + 1;
    return c3 * x * x * x - c1 * x * x;
}

inline static double easeOutBack( double x ) {
    const double c1 = 1.70158;
    const double c3 = c1 + 1;
    return 1 + c3 * std::pow( x - 1, 3 ) + c1 * std::pow( x - 1, 2 );
}

inline static double easeInOutBack( double x ) {
    const double c1 = 1.70158;
    const double c2 = c1 * 1.525;
    if( x < 0.5 ) {
        return (std::pow( 2 * x, 2 ) * ((c2 + 1) * 2 * x - c2)) / 2;
    } else {
        return (std::pow( 2 * x - 2, 2 ) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
    }
}

inline static double easeInElastic( double x ) {
    const double c4 = (2 * PI) / 3;
    if( x == 0 ) {
        return 0;
    } else if( x == 1 ) {
        return 1;
    } else {
        return -std::pow( 2, 10 * x - 10 ) * std::sin( (x * 10 - 10.75) * c4 );
    }
}

inline static double easeOutElastic( double x ) {
    const double c4 = (2 * PI) / 3;
    if( x == 0 ) {
        return 0;
    } else if( x == 1 ) {
        return 1;
    } else {
        return std::pow( 2, -10 * x ) * std::sin( (x * 10 - 0.75) * c4 ) + 1;
    }
}

inline double easeInOutElastic( double x ) {
    const double c5 = (2 * PI) / 4.5;
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

inline static double easeOutBounce( double x ) {
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

inline double easeInBounce( double x ) {
    return 1 - easeOutBounce(1 - x);
}

inline static double easeInOutBounce( double x ) {
    return x < 0.5
    ? (1 - easeOutBounce(1 - 2 * x)) / 2
    : (1 + easeOutBounce(2 * x - 1)) / 2;
}

inline void add_module_easing(VM* vm){
    PyObject* mod = vm->new_module("easing");

#define EASE(name)  \
    vm->bind_func<1>(mod, "Ease"#name, [](VM* vm, ArgsView args){  \
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

} // namespace pkpy

#else

ADD_MODULE_PLACEHOLDER(easing)

#endif


#if PK_MODULE_REQUESTS
#include "httplib.h"
namespace pkpy {

inline void add_module_requests(VM* vm){
    static StrName m_requests("requests");
    static StrName m_Response("Response");
    PyObject* mod = vm->new_module(m_requests);
    CodeObject_ code = vm->compile(kPythonLibs["requests"], "requests.py", EXEC_MODE);
    vm->_exec(code, mod);

    vm->bind_func<4>(mod, "_request", [](VM* vm, ArgsView args){
        Str method = CAST(Str&, args[0]);
        Str url = CAST(Str&, args[1]);
        PyObject* headers = args[2];            // a dict object
        PyObject* body = args[3];               // a bytes object

        if(url.index("http://") != 0){
            vm->ValueError("url must start with http://");
        }

        for(char c: url){
            switch(c){
                case '.':
                case '-':
                case '_':
                case '~':
                case ':':
                case '/':
                break;
                default:
                    if(!isalnum(c)){
                        vm->ValueError(fmt("invalid character in url: '", c, "'"));
                    }
            }
        }

        int slash = url.index("/", 7);
        Str path = "/";
        if(slash != -1){
            path = url.substr(slash);
            url = url.substr(0, slash);
            if(path.empty()) path = "/";
        }

        httplib::Client client(url.str());

        httplib::Headers h;
        if(headers != vm->None){
            List list = CAST(List&, headers);
            for(auto& item : list){
                Tuple t = CAST(Tuple&, item);
                Str key = CAST(Str&, t[0]);
                Str value = CAST(Str&, t[1]);
                h.emplace(key.str(), value.str());
            }
        }

        auto _to_resp = [=](const httplib::Result& res){
            return vm->call(
                vm->_modules[m_requests]->attr(m_Response),
                VAR(res->status),
                VAR(res->reason),
                VAR(Bytes(res->body))
            );
        };

        if(method == "GET"){
            httplib::Result res = client.Get(path.str(), h);
            return _to_resp(res);
        }else if(method == "POST"){
            Bytes b = CAST(Bytes&, body);
            httplib::Result res = client.Post(path.str(), h, b.data(), b.size(), "application/octet-stream");
            return _to_resp(res);
        }else if(method == "PUT"){
            Bytes b = CAST(Bytes&, body);
            httplib::Result res = client.Put(path.str(), h, b.data(), b.size(), "application/octet-stream");
            return _to_resp(res);
        }else if(method == "DELETE"){
            httplib::Result res = client.Delete(path.str(), h);
            return _to_resp(res);
        }else{
            vm->ValueError("invalid method");
        }
        UNREACHABLE();
    });
}

}   // namespace pkpy

#else

ADD_MODULE_PLACEHOLDER(requests)

#endif


#if PK_ENABLE_OS

#include <filesystem>
#include <cstdio>

namespace pkpy{

inline Bytes _default_import_handler(const Str& name){
    std::filesystem::path path(name.sv());
    bool exists = std::filesystem::exists(path);
    if(!exists) return Bytes();
    std::string cname = name.str();
    FILE* fp = fopen(cname.c_str(), "rb");
    if(!fp) return Bytes();
    fseek(fp, 0, SEEK_END);
    std::vector<char> buffer(ftell(fp));
    fseek(fp, 0, SEEK_SET);
    fread(buffer.data(), 1, buffer.size(), fp);
    fclose(fp);
    return Bytes(std::move(buffer));
};

struct FileIO {
    PY_CLASS(FileIO, io, FileIO)

    Str file;
    Str mode;
    FILE* fp;

    bool is_text() const { return mode != "rb" && mode != "wb" && mode != "ab"; }

    FileIO(VM* vm, std::string file, std::string mode): file(file), mode(mode) {
        fp = fopen(file.c_str(), mode.c_str());
        if(!fp) vm->IOError(strerror(errno));
    }

    void close(){
        if(fp == nullptr) return;
        fclose(fp);
        fp = nullptr;
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<3>(type, [](VM* vm, ArgsView args){
            return VAR_T(FileIO, 
                vm, CAST(Str&, args[1]).str(), CAST(Str&, args[2]).str()
            );
        });

        vm->bind_method<0>(type, "read", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            fseek(io.fp, 0, SEEK_END);
            std::vector<char> buffer(ftell(io.fp));
            fseek(io.fp, 0, SEEK_SET);
            fread(buffer.data(), 1, buffer.size(), io.fp);
            Bytes b(std::move(buffer));
            if(io.is_text()) return VAR(Str(b.str()));
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

        vm->bind_method<0>(type, "__enter__", CPP_LAMBDA(vm->None));
    }
};

inline void add_module_io(VM* vm){
    PyObject* mod = vm->new_module("io");
    FileIO::register_class(vm, mod);
    vm->bind_builtin_func<2>("open", [](VM* vm, ArgsView args){
        static StrName m_io("io");
        static StrName m_FileIO("FileIO");
        return vm->call(vm->_modules[m_io]->attr(m_FileIO), args[0], args[1]);
    });
}

inline void add_module_os(VM* vm){
    PyObject* mod = vm->new_module("os");
    PyObject* path_obj = vm->heap.gcnew<DummyInstance>(vm->tp_object, {});
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
}

} // namespace pkpy


#else

namespace pkpy{
inline void add_module_io(void* vm){}
inline void add_module_os(void* vm){}
inline Bytes _default_import_handler(const Str& name) { return Bytes(); }
} // namespace pkpy

#endif
#ifndef PK_EXPORT

#ifdef _WIN32
#define PK_EXPORT __declspec(dllexport)
#elif __APPLE__
#define PK_EXPORT __attribute__((visibility("default"))) __attribute__((used))
#elif __EMSCRIPTEN__
#include <emscripten.h>
#define PK_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define PK_EXPORT
#endif

#define PK_LEGACY_EXPORT PK_EXPORT inline

#endif


namespace pkpy {

inline CodeObject_ VM::compile(Str source, Str filename, CompileMode mode, bool unknown_global_scope) {
    Compiler compiler(this, source, filename, mode, unknown_global_scope);
    try{
        return compiler.compile();
    }catch(Exception& e){
#if DEBUG_FULL_EXCEPTION
        std::cerr << e.summary() << std::endl;
#endif
        _error(e);
        return nullptr;
    }
}


inline void init_builtins(VM* _vm) {
#define BIND_NUM_ARITH_OPT(name, op)                                                                    \
    _vm->bind##name(_vm->tp_int, [](VM* vm, PyObject* lhs, PyObject* rhs) {                             \
        if(is_int(rhs)){                                                                                \
            return VAR(_CAST(i64, lhs) op _CAST(i64, rhs));                                             \
        }else{                                                                                          \
            return VAR(_CAST(i64, lhs) op vm->num_to_float(rhs));                                       \
        }                                                                                               \
    });                                                                                                 \
    _vm->bind##name(_vm->tp_float, [](VM* vm, PyObject* lhs, PyObject* rhs) {                           \
        return VAR(_CAST(f64, lhs) op vm->num_to_float(rhs));                                           \
    });

    BIND_NUM_ARITH_OPT(__add__, +)
    BIND_NUM_ARITH_OPT(__sub__, -)
    BIND_NUM_ARITH_OPT(__mul__, *)

#undef BIND_NUM_ARITH_OPT

#define BIND_NUM_LOGICAL_OPT(name, op, is_eq)   \
    _vm->bind##name(_vm->tp_int, [](VM* vm, PyObject* lhs, PyObject* rhs) { \
        if(is_int(rhs))     return _CAST(i64, lhs) op _CAST(i64, rhs);      \
        if(is_float(rhs))   return _CAST(i64, lhs) op _CAST(f64, rhs);      \
        if constexpr(is_eq) return lhs op rhs;                              \
        vm->TypeError("unsupported operand type(s) for " #op );             \
        return false;                                                       \
    });                                                                     \
    _vm->bind##name(_vm->tp_float, [](VM* vm, PyObject* lhs, PyObject* rhs) {   \
        if(is_int(rhs))     return _CAST(f64, lhs) op _CAST(i64, rhs);          \
        if(is_float(rhs))   return _CAST(f64, lhs) op _CAST(f64, rhs);          \
        if constexpr(is_eq) return lhs op rhs;                                  \
        vm->TypeError("unsupported operand type(s) for " #op );                 \
        return false;                                                           \
    });

    BIND_NUM_LOGICAL_OPT(__lt__, <, false)
    BIND_NUM_LOGICAL_OPT(__le__, <=, false)
    BIND_NUM_LOGICAL_OPT(__gt__, >, false)
    BIND_NUM_LOGICAL_OPT(__ge__, >=, false)
    BIND_NUM_LOGICAL_OPT(__eq__, ==, true)

#undef BIND_NUM_ARITH_OPT
#undef BIND_NUM_LOGICAL_OPT

    _vm->bind_builtin_func<2>("super", [](VM* vm, ArgsView args) {
        vm->check_non_tagged_type(args[0], vm->tp_type);
        Type type = OBJ_GET(Type, args[0]);
        if(!vm->isinstance(args[1], type)){
            Str _0 = obj_type_name(vm, OBJ_GET(Type, vm->_t(args[1])));
            Str _1 = obj_type_name(vm, type);
            vm->TypeError("super(): " + _0.escape() + " is not an instance of " + _1.escape());
        }
        Type base = vm->_all_types[type].base;
        return vm->heap.gcnew(vm->tp_super, Super(args[1], base));
    });

    _vm->bind_builtin_func<2>("isinstance", [](VM* vm, ArgsView args) {
        vm->check_non_tagged_type(args[1], vm->tp_type);
        Type type = OBJ_GET(Type, args[1]);
        return VAR(vm->isinstance(args[0], type));
    });

    _vm->bind_builtin_func<0>("globals", [](VM* vm, ArgsView args) {
        PyObject* mod = vm->top_frame()->_module;
        return VAR(MappingProxy(mod));
    });

    _vm->bind_builtin_func<1>("id", [](VM* vm, ArgsView args) {
        PyObject* obj = args[0];
        if(is_tagged(obj)) return vm->None;
        return VAR_T(VoidP, obj);
    });

    _vm->bind_builtin_func<1>("staticmethod", [](VM* vm, ArgsView args) {
        return args[0];
    });

    _vm->bind_builtin_func<1>("__import__", [](VM* vm, ArgsView args) {
        return vm->py_import(CAST(Str&, args[0]));
    });

    _vm->bind_builtin_func<2>("divmod", [](VM* vm, ArgsView args) {
        i64 lhs = CAST(i64, args[0]);
        i64 rhs = CAST(i64, args[1]);
        return VAR(Tuple({VAR(lhs/rhs), VAR(lhs%rhs)}));
    });

    _vm->bind_builtin_func<1>("eval", [](VM* vm, ArgsView args) {
        CodeObject_ code = vm->compile(CAST(Str&, args[0]), "<eval>", EVAL_MODE, true);
        FrameId frame = vm->top_frame();
        return vm->_exec(code.get(), frame->_module, frame->_callable, frame->_locals);
    });

    _vm->bind_builtin_func<1>("exec", [](VM* vm, ArgsView args) {
        CodeObject_ code = vm->compile(CAST(Str&, args[0]), "<exec>", EXEC_MODE, true);
        FrameId frame = vm->top_frame();
        vm->_exec(code.get(), frame->_module, frame->_callable, frame->_locals);
        return vm->None;
    });

    _vm->bind_builtin_func<-1>("exit", [](VM* vm, ArgsView args) {
        if(args.size() == 0) std::exit(0);
        else if(args.size() == 1) std::exit(CAST(int, args[0]));
        else vm->TypeError("exit() takes at most 1 argument");
        return vm->None;
    });

    _vm->bind_builtin_func<1>("repr", CPP_LAMBDA(vm->py_repr(args[0])));

    _vm->bind_builtin_func<1>("len", [](VM* vm, ArgsView args){
        const PyTypeInfo* ti = vm->_inst_type_info(args[0]);
        if(ti->m__len__) return VAR(ti->m__len__(vm, args[0]));
        return vm->call_method(args[0], __len__);
    });

    _vm->bind_builtin_func<1>("hash", [](VM* vm, ArgsView args){
        i64 value = vm->py_hash(args[0]);
        if(((value << 2) >> 2) != value) value >>= 2;
        return VAR(value);
    });

    _vm->bind_builtin_func<1>("chr", [](VM* vm, ArgsView args) {
        i64 i = CAST(i64, args[0]);
        if (i < 0 || i > 128) vm->ValueError("chr() arg not in range(128)");
        return VAR(std::string(1, (char)i));
    });

    _vm->bind_builtin_func<1>("ord", [](VM* vm, ArgsView args) {
        const Str& s = CAST(Str&, args[0]);
        if (s.length()!=1) vm->TypeError("ord() expected an ASCII character");
        return VAR((i64)(s[0]));
    });

    _vm->bind_builtin_func<2>("hasattr", [](VM* vm, ArgsView args) {
        return VAR(vm->getattr(args[0], CAST(Str&, args[1]), false) != nullptr);
    });

    _vm->bind_builtin_func<3>("setattr", [](VM* vm, ArgsView args) {
        vm->setattr(args[0], CAST(Str&, args[1]), args[2]);
        return vm->None;
    });

    _vm->bind_builtin_func<2>("getattr", [](VM* vm, ArgsView args) {
        const Str& name = CAST(Str&, args[1]);
        return vm->getattr(args[0], name);
    });

    _vm->bind_builtin_func<1>("hex", [](VM* vm, ArgsView args) {
        std::stringstream ss;
        ss << std::hex << CAST(i64, args[0]);
        return VAR("0x" + ss.str());
    });

    _vm->bind_builtin_func<1>("iter", [](VM* vm, ArgsView args) {
        return vm->py_iter(args[0]);
    });

    _vm->bind_builtin_func<1>("next", [](VM* vm, ArgsView args) {
        return vm->py_next(args[0]);
    });

    _vm->bind_builtin_func<1>("dir", [](VM* vm, ArgsView args) {
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

    _vm->bind__repr__(_vm->tp_object, [](VM* vm, PyObject* obj) {
        if(is_tagged(obj)) FATAL_ERROR();
        std::stringstream ss;
        ss << "<" << OBJ_NAME(vm->_t(obj)) << " object at 0x";
        ss << std::hex << reinterpret_cast<intptr_t>(obj) << ">";
        return VAR(ss.str());
    });

    _vm->bind__eq__(_vm->tp_object, [](VM* vm, PyObject* lhs, PyObject* rhs) { return lhs == rhs; });
    _vm->bind__hash__(_vm->tp_object, [](VM* vm, PyObject* obj) { return BITS(obj); });

    _vm->bind_constructor<2>("type", CPP_LAMBDA(vm->_t(args[1])));

    _vm->bind_constructor<-1>("range", [](VM* vm, ArgsView args) {
        args._begin += 1;   // skip cls
        Range r;
        switch (args.size()) {
            case 1: r.stop = CAST(i64, args[0]); break;
            case 2: r.start = CAST(i64, args[0]); r.stop = CAST(i64, args[1]); break;
            case 3: r.start = CAST(i64, args[0]); r.stop = CAST(i64, args[1]); r.step = CAST(i64, args[2]); break;
            default: vm->TypeError("expected 1-3 arguments, but got " + std::to_string(args.size()));
        }
        return VAR(r);
    });

    _vm->bind__iter__(_vm->tp_range, [](VM* vm, PyObject* obj) { return VAR_T(RangeIter, OBJ_GET(Range, obj)); });
    _vm->bind__repr__(_vm->_type("NoneType"), [](VM* vm, PyObject* obj) { return VAR("None"); });
    _vm->bind__json__(_vm->_type("NoneType"), [](VM* vm, PyObject* obj) { return VAR("null"); });

    _vm->bind__truediv__(_vm->tp_float, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        f64 value = CAST_F(rhs);
        return VAR(_CAST(f64, lhs) / value);
    });

    _vm->bind__truediv__(_vm->tp_int, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        f64 value = CAST_F(rhs);
        return VAR(_CAST(i64, lhs) / value);
    });

    auto py_number_pow = [](VM* vm, PyObject* lhs_, PyObject* rhs_) {
        if(is_both_int(lhs_, rhs_)){
            i64 lhs = _CAST(i64, lhs_);
            i64 rhs = _CAST(i64, rhs_);
            bool flag = false;
            if(rhs < 0) {flag = true; rhs = -rhs;}
            i64 ret = 1;
            while(rhs){
                if(rhs & 1) ret *= lhs;
                lhs *= lhs;
                rhs >>= 1;
            }
            if(flag) return VAR((f64)(1.0 / ret));
            return VAR(ret);
        }else{
            return VAR((f64)std::pow(CAST_F(lhs_), CAST_F(rhs_)));
        }
    };

    _vm->bind__pow__(_vm->tp_int, py_number_pow);
    _vm->bind__pow__(_vm->tp_float, py_number_pow);

    /************ int ************/
    _vm->bind_constructor<2>("int", [](VM* vm, ArgsView args) {
        if (is_type(args[1], vm->tp_float)) return VAR((i64)CAST(f64, args[1]));
        if (is_type(args[1], vm->tp_int)) return args[1];
        if (is_type(args[1], vm->tp_bool)) return VAR(_CAST(bool, args[1]) ? 1 : 0);
        if (is_type(args[1], vm->tp_str)) {
            const Str& s = CAST(Str&, args[1]);
            try{
                size_t parsed = 0;
                i64 val = Number::stoi(s.str(), &parsed, 10);
                if(parsed != s.length()) throw std::invalid_argument("<?>");
                return VAR(val);
            }catch(std::invalid_argument&){
                vm->ValueError("invalid literal for int(): " + s.escape());
            }
        }
        vm->TypeError("int() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bind__floordiv__(_vm->tp_int, [](VM* vm, PyObject* lhs_, PyObject* rhs_) {
        i64 rhs = CAST(i64, rhs_);
        return VAR(_CAST(i64, lhs_) / rhs);
    });

    _vm->bind__mod__(_vm->tp_int, [](VM* vm, PyObject* lhs_, PyObject* rhs_) {
        i64 rhs = CAST(i64, rhs_);
        return VAR(_CAST(i64, lhs_) % rhs);
    });

    _vm->bind__repr__(_vm->tp_int, [](VM* vm, PyObject* obj) { return VAR(std::to_string(_CAST(i64, obj))); });
    _vm->bind__json__(_vm->tp_int, [](VM* vm, PyObject* obj) { return VAR(std::to_string(_CAST(i64, obj))); });

    _vm->bind__neg__(_vm->tp_int, [](VM* vm, PyObject* obj) { return VAR(-_CAST(i64, obj)); });

    _vm->bind__hash__(_vm->tp_int, [](VM* vm, PyObject* obj) { return _CAST(i64, obj); });

#define INT_BITWISE_OP(name, op) \
    _vm->bind##name(_vm->tp_int, [](VM* vm, PyObject* lhs, PyObject* rhs) { \
        return VAR(_CAST(i64, lhs) op CAST(i64, rhs)); \
    });

    INT_BITWISE_OP(__lshift__, <<)
    INT_BITWISE_OP(__rshift__, >>)
    INT_BITWISE_OP(__and__, &)
    INT_BITWISE_OP(__or__, |)
    INT_BITWISE_OP(__xor__, ^)

#undef INT_BITWISE_OP

    /************ float ************/
    _vm->bind_constructor<2>("float", [](VM* vm, ArgsView args) {
        if (is_type(args[1], vm->tp_int)) return VAR((f64)CAST(i64, args[1]));
        if (is_type(args[1], vm->tp_float)) return args[1];
        if (is_type(args[1], vm->tp_bool)) return VAR(_CAST(bool, args[1]) ? 1.0 : 0.0);
        if (is_type(args[1], vm->tp_str)) {
            const Str& s = CAST(Str&, args[1]);
            if(s == "inf") return VAR(INFINITY);
            if(s == "-inf") return VAR(-INFINITY);
            try{
                f64 val = Number::stof(s.str());
                return VAR(val);
            }catch(std::invalid_argument&){
                vm->ValueError("invalid literal for float(): " + s.escape());
            }
        }
        vm->TypeError("float() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bind__hash__(_vm->tp_float, [](VM* vm, PyObject* obj) {
        f64 val = _CAST(f64, obj);
        return (i64)std::hash<f64>()(val);
    });

    _vm->bind__neg__(_vm->tp_float, [](VM* vm, PyObject* obj) { return VAR(-_CAST(f64, obj)); });

    _vm->bind__repr__(_vm->tp_float, [](VM* vm, PyObject* obj) {
        f64 val = _CAST(f64, obj);
        if(std::isinf(val) || std::isnan(val)) return VAR(std::to_string(val));
        std::stringstream ss;
        ss << std::setprecision(std::numeric_limits<f64>::max_digits10-2) << val;
        std::string s = ss.str();
        if(std::all_of(s.begin()+1, s.end(), isdigit)) s += ".0";
        return VAR(s);
    });
    _vm->bind__json__(_vm->tp_float, [](VM* vm, PyObject* obj) {
        f64 val = _CAST(f64, obj);
        if(std::isinf(val) || std::isnan(val)) vm->ValueError("cannot jsonify 'nan' or 'inf'");
        return VAR(std::to_string(val));
    });

    /************ str ************/
    _vm->bind_constructor<2>("str", CPP_LAMBDA(vm->py_str(args[1])));

    _vm->bind__hash__(_vm->tp_str, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(Str&, obj).hash();
    });

    _vm->bind__add__(_vm->tp_str, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        return VAR(_CAST(Str&, lhs) + CAST(Str&, rhs));
    });
    _vm->bind__len__(_vm->tp_str, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(Str&, obj).u8_length();
    });
    _vm->bind__mul__(_vm->tp_str, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        const Str& self = _CAST(Str&, lhs);
        i64 n = CAST(i64, rhs);
        std::stringstream ss;
        for(i64 i = 0; i < n; i++) ss << self.sv();
        return VAR(ss.str());
    });
    _vm->bind__contains__(_vm->tp_str, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        const Str& self = _CAST(Str&, lhs);
        const Str& other = CAST(Str&, rhs);
        return self.index(other) != -1;
    });
    _vm->bind__str__(_vm->tp_str, [](VM* vm, PyObject* obj) { return obj; });
    _vm->bind__iter__(_vm->tp_str, [](VM* vm, PyObject* obj) { return VAR_T(StringIter, obj); });
    _vm->bind__repr__(_vm->tp_str, [](VM* vm, PyObject* obj) {
        const Str& self = _CAST(Str&, obj);
        return VAR(self.escape(true));
    });
    _vm->bind__json__(_vm->tp_str, [](VM* vm, PyObject* obj) {
        const Str& self = _CAST(Str&, obj);
        return VAR(self.escape(false));
    });
    _vm->bind__eq__(_vm->tp_str, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        if(!is_non_tagged_type(rhs, vm->tp_str)) return false;
        return _CAST(Str&, lhs) == _CAST(Str&, rhs);
    });
    _vm->bind__gt__(_vm->tp_str, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        return _CAST(Str&, lhs) > CAST(Str&, rhs);
    });
    _vm->bind__lt__(_vm->tp_str, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        return _CAST(Str&, lhs) < CAST(Str&, rhs);
    });
    _vm->bind__ge__(_vm->tp_str, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        return _CAST(Str&, lhs) >= CAST(Str&, rhs);
    });
    _vm->bind__le__(_vm->tp_str, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        return _CAST(Str&, lhs) <= CAST(Str&, rhs);
    });

    _vm->bind__getitem__(_vm->tp_str, [](VM* vm, PyObject* obj, PyObject* index) {
        const Str& self = _CAST(Str&, obj);
        if(is_non_tagged_type(index, vm->tp_slice)){
            const Slice& s = _CAST(Slice&, index);
            int start, stop, step;
            vm->parse_int_slice(s, self.u8_length(), start, stop, step);
            return VAR(self.u8_slice(start, stop, step));
        }
        int i = CAST(int, index);
        i = vm->normalized_index(i, self.u8_length());
        return VAR(self.u8_getitem(i));
    });

    _vm->bind_method<-1>("str", "replace", [](VM* vm, ArgsView args) {
        if(args.size() != 1+2 && args.size() != 1+3) vm->TypeError("replace() takes 2 or 3 arguments");
        const Str& self = _CAST(Str&, args[0]);
        const Str& old = CAST(Str&, args[1]);
        const Str& new_ = CAST(Str&, args[2]);
        int count = args.size()==1+3 ? CAST(int, args[3]) : -1;
        return VAR(self.replace(old, new_, count));
    });

    _vm->bind_method<1>("str", "index", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& sub = CAST(Str&, args[1]);
        int index = self.index(sub);
        if(index == -1) vm->ValueError("substring not found");
        return VAR(index);
    });

    _vm->bind_method<1>("str", "startswith", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& prefix = CAST(Str&, args[1]);
        return VAR(self.index(prefix) == 0);
    });

    _vm->bind_method<1>("str", "endswith", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& suffix = CAST(Str&, args[1]);
        int offset = self.length() - suffix.length();
        if(offset < 0) return vm->False;
        bool ok = memcmp(self.data+offset, suffix.data, suffix.length()) == 0;
        return VAR(ok);
    });

    _vm->bind_method<0>("str", "encode", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        std::vector<char> buffer(self.length());
        memcpy(buffer.data(), self.data, self.length());
        return VAR(Bytes(std::move(buffer)));
    });

    _vm->bind_method<1>("str", "join", [](VM* vm, ArgsView args) {
        auto _lock = vm->heap.gc_scope_lock();
        const Str& self = _CAST(Str&, args[0]);
        FastStrStream ss;
        PyObject* it = vm->py_iter(args[1]);     // strong ref
        PyObject* obj = vm->py_next(it);
        while(obj != vm->StopIteration){
            if(!ss.empty()) ss << self;
            ss << CAST(Str&, obj);
            obj = vm->py_next(it);
        }
        return VAR(ss.str());
    });

    _vm->bind_method<0>("str", "to_c_str", [](VM* vm, ArgsView args){
        const Str& self = _CAST(Str&, args[0]);
        return VAR(self.c_str_dup());
    });

    _vm->bind_func<1>("str", "from_c_str", [](VM* vm, ArgsView args){
        char* p = CAST(char*, args[0]);
        return VAR(Str(p));
    });

    _vm->bind_method<0>("str", "lower", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        return VAR(self.lower());
    });

    _vm->bind_method<0>("str", "upper", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        return VAR(self.upper());
    });

    /************ list ************/
    _vm->bind_constructor<2>("list", [](VM* vm, ArgsView args) {
        return vm->py_list(args[1]);
    });

    _vm->bind__contains__(_vm->tp_list, [](VM* vm, PyObject* obj, PyObject* item) {
        List& self = _CAST(List&, obj);
        for(PyObject* i: self) if(vm->py_equals(i, item)) return true;
        return false;
    });

    _vm->bind_method<1>("list", "count", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        int count = 0;
        for(PyObject* i: self) if(vm->py_equals(i, args[1])) count++;
        return VAR(count);
    });

    _vm->bind__eq__(_vm->tp_list, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        List& a = _CAST(List&, lhs);
        List& b = _CAST(List&, rhs);
        if(a.size() != b.size()) return false;
        for(int i=0; i<a.size(); i++){
            if(!vm->py_equals(a[i], b[i])) return false;
        }
        return true;
    });

    _vm->bind_method<1>("list", "index", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        PyObject* obj = args[1];
        for(int i=0; i<self.size(); i++){
            if(vm->py_equals(self[i], obj)) return VAR(i);
        }
        vm->ValueError(_CAST(Str&, vm->py_repr(obj)) + " is not in list");
        return vm->None;
    });

    _vm->bind_method<1>("list", "remove", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        PyObject* obj = args[1];
        for(int i=0; i<self.size(); i++){
            if(vm->py_equals(self[i], obj)){
                self.erase(i);
                return vm->None;
            }
        }
        vm->ValueError(_CAST(Str&, vm->py_repr(obj)) + " is not in list");
        return vm->None;
    });

    _vm->bind_method<-1>("list", "pop", [](VM* vm, ArgsView args) {
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

    _vm->bind_method<1>("list", "append", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        self.push_back(args[1]);
        return vm->None;
    });

    _vm->bind_method<1>("list", "extend", [](VM* vm, ArgsView args) {
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

    _vm->bind_method<0>("list", "reverse", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        std::reverse(self.begin(), self.end());
        return vm->None;
    });

    _vm->bind__mul__(_vm->tp_list, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        const List& self = _CAST(List&, lhs);
        int n = CAST(int, rhs);
        List result;
        result.reserve(self.size() * n);
        for(int i = 0; i < n; i++) result.extend(self);
        return VAR(std::move(result));
    });

    _vm->bind_method<2>("list", "insert", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        int index = CAST(int, args[1]);
        if(index < 0) index += self.size();
        if(index < 0) index = 0;
        if(index > self.size()) index = self.size();
        self.insert(index, args[2]);
        return vm->None;
    });

    _vm->bind_method<0>("list", "clear", [](VM* vm, ArgsView args) {
        _CAST(List&, args[0]).clear();
        return vm->None;
    });

    _vm->bind_method<0>("list", "copy", CPP_LAMBDA(VAR(_CAST(List, args[0]))));

    _vm->bind__hash__(_vm->tp_list, [](VM* vm, PyObject* obj) {
        vm->TypeError("unhashable type: 'list'");
        return (i64)0;
    });

    _vm->bind__add__(_vm->tp_list, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        const List& self = _CAST(List&, lhs);
        const List& other = CAST(List&, rhs);
        List new_list(self);    // copy construct
        new_list.extend(other);
        return VAR(std::move(new_list));
    });

    _vm->bind__len__(_vm->tp_list, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(List&, obj).size();
    });
    _vm->bind__iter__(_vm->tp_list, [](VM* vm, PyObject* obj) {
        List& self = _CAST(List&, obj);
        return VAR_T(ArrayIter, obj, self.begin(), self.end());
    });
    _vm->bind__getitem__(_vm->tp_list, PyArrayGetItem<List>);
    _vm->bind__setitem__(_vm->tp_list, [](VM* vm, PyObject* obj, PyObject* index, PyObject* value){
        List& self = _CAST(List&, obj);
        int i = CAST(int, index);
        i = vm->normalized_index(i, self.size());
        self[i] = value;
    });
    _vm->bind__delitem__(_vm->tp_list, [](VM* vm, PyObject* obj, PyObject* index){
        List& self = _CAST(List&, obj);
        int i = CAST(int, index);
        i = vm->normalized_index(i, self.size());
        self.erase(i);
    });

    /************ tuple ************/
    _vm->bind_constructor<2>("tuple", [](VM* vm, ArgsView args) {
        List list = CAST(List, vm->py_list(args[1]));
        return VAR(Tuple(std::move(list)));
    });

    _vm->bind__contains__(_vm->tp_tuple, [](VM* vm, PyObject* obj, PyObject* item) {
        Tuple& self = _CAST(Tuple&, obj);
        for(PyObject* i: self) if(vm->py_equals(i, item)) return true;
        return false;
    });

    _vm->bind_method<1>("tuple", "count", [](VM* vm, ArgsView args) {
        Tuple& self = _CAST(Tuple&, args[0]);
        int count = 0;
        for(PyObject* i: self) if(vm->py_equals(i, args[1])) count++;
        return VAR(count);
    });

    _vm->bind__eq__(_vm->tp_tuple, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        const Tuple& self = _CAST(Tuple&, lhs);
        const Tuple& other = CAST(Tuple&, rhs);
        if(self.size() != other.size()) return false;
        for(int i = 0; i < self.size(); i++) {
            if(!vm->py_equals(self[i], other[i])) return false;
        }
        return true;
    });

    _vm->bind__hash__(_vm->tp_tuple, [](VM* vm, PyObject* obj) {
        i64 x = 1000003;
        const Tuple& items = CAST(Tuple&, obj);
        for (int i=0; i<items.size(); i++) {
            i64 y = vm->py_hash(items[i]);
            // recommended by Github Copilot
            x = x ^ (y + 0x9e3779b9 + (x << 6) + (x >> 2));
        }
        return x;
    });

    _vm->bind__iter__(_vm->tp_tuple, [](VM* vm, PyObject* obj) {
        Tuple& self = _CAST(Tuple&, obj);
        return VAR_T(ArrayIter, obj, self.begin(), self.end());
    });
    _vm->bind__getitem__(_vm->tp_tuple, PyArrayGetItem<Tuple>);
    _vm->bind__len__(_vm->tp_tuple, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(Tuple&, obj).size();
    });

    /************ bool ************/
    _vm->bind_constructor<2>("bool", CPP_LAMBDA(VAR(vm->py_bool(args[1]))));
    _vm->bind__hash__(_vm->tp_bool, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(bool, obj);
    });
    _vm->bind__repr__(_vm->tp_bool, [](VM* vm, PyObject* self) {
        bool val = _CAST(bool, self);
        return VAR(val ? "True" : "False");
    });
    _vm->bind__json__(_vm->tp_bool, [](VM* vm, PyObject* self) {
        bool val = _CAST(bool, self);
        return VAR(val ? "true" : "false");
    });

    _vm->bind__and__(_vm->tp_bool, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        return VAR(_CAST(bool, lhs) && CAST(bool, rhs));
    });
    _vm->bind__or__(_vm->tp_bool, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        return VAR(_CAST(bool, lhs) || CAST(bool, rhs));
    });
    _vm->bind__xor__(_vm->tp_bool, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        return VAR(_CAST(bool, lhs) != CAST(bool, rhs));
    });
    _vm->bind__eq__(_vm->tp_bool, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        return _CAST(bool, lhs) == CAST(bool, rhs);
    });
    _vm->bind__repr__(_vm->_type("ellipsis"), [](VM* vm, PyObject* self) {
        return VAR("Ellipsis");
    });

    /************ bytes ************/
    _vm->bind_constructor<2>("bytes", [](VM* vm, ArgsView args){
        List& list = CAST(List&, args[1]);
        std::vector<char> buffer(list.size());
        for(int i=0; i<list.size(); i++){
            i64 b = CAST(i64, list[i]);
            if(b<0 || b>255) vm->ValueError("byte must be in range[0, 256)");
            buffer[i] = (char)b;
        }
        return VAR(Bytes(std::move(buffer)));
    });

    _vm->bind__getitem__(_vm->tp_bytes, [](VM* vm, PyObject* obj, PyObject* index) {
        const Bytes& self = _CAST(Bytes&, obj);
        int i = CAST(int, index);
        i = vm->normalized_index(i, self.size());
        return VAR(self[i]);
    });

    _vm->bind__hash__(_vm->tp_bytes, [](VM* vm, PyObject* obj) {
        const Bytes& self = _CAST(Bytes&, obj);
        return (i64)std::hash<std::string>()(self.str());
    });

    _vm->bind__repr__(_vm->tp_bytes, [](VM* vm, PyObject* obj) {
        const Bytes& self = _CAST(Bytes&, obj);
        std::stringstream ss;
        ss << "b'";
        for(int i=0; i<self.size(); i++){
            ss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << self[i];
        }
        ss << "'";
        return VAR(ss.str());
    });
    _vm->bind__len__(_vm->tp_bytes, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(Bytes&, obj).size();
    });

    _vm->bind_method<0>("bytes", "decode", [](VM* vm, ArgsView args) {
        const Bytes& self = _CAST(Bytes&, args[0]);
        // TODO: check encoding is utf-8
        return VAR(Str(self.str()));
    });

    _vm->bind_method<0>("bytes", "to_char_array", [](VM* vm, ArgsView args) {
        const Bytes& self = _CAST(Bytes&, args[0]);
        void* buffer = malloc(self.size());
        memcpy(buffer, self.data(), self.size());
        return VAR_T(VoidP, buffer);
    });

    _vm->bind_func<2>("bytes", "from_char_array", [](VM* vm, ArgsView args) {
        const VoidP& data = _CAST(VoidP&, args[0]);
        int size = CAST(int, args[1]);
        std::vector<char> buffer(size);
        memcpy(buffer.data(), data.ptr, size);
        return VAR(Bytes(std::move(buffer)));
    });

    _vm->bind__eq__(_vm->tp_bytes, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        return _CAST(Bytes&, lhs) == _CAST(Bytes&, rhs);
    });
    /************ slice ************/
    _vm->bind_constructor<4>("slice", [](VM* vm, ArgsView args) {
        return VAR(Slice(args[1], args[2], args[3]));
    });

    _vm->bind__repr__(_vm->tp_slice, [](VM* vm, PyObject* obj) {
        const Slice& self = _CAST(Slice&, obj);
        std::stringstream ss;
        ss << "slice(";
        ss << CAST(Str, vm->py_repr(self.start)) << ", ";
        ss << CAST(Str, vm->py_repr(self.stop)) << ", ";
        ss << CAST(Str, vm->py_repr(self.step)) << ")";
        return VAR(ss.str());
    });

    /************ mappingproxy ************/
    _vm->bind_method<0>("mappingproxy", "keys", [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        List keys;
        for(StrName name : self.attr().keys()) keys.push_back(VAR(name.sv()));
        return VAR(std::move(keys));
    });

    _vm->bind_method<0>("mappingproxy", "values", [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        List values;
        for(auto& item : self.attr().items()) values.push_back(item.second);
        return VAR(std::move(values));
    });

    _vm->bind_method<0>("mappingproxy", "items", [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        List items;
        for(auto& item : self.attr().items()){
            PyObject* t = VAR(Tuple({VAR(item.first.sv()), item.second}));
            items.push_back(std::move(t));
        }
        return VAR(std::move(items));
    });

    _vm->bind__len__(_vm->tp_mappingproxy, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(MappingProxy&, obj).attr().size();
    });

    _vm->bind__getitem__(_vm->tp_mappingproxy, [](VM* vm, PyObject* obj, PyObject* index) {
        MappingProxy& self = _CAST(MappingProxy&, obj);
        StrName key = CAST(Str&, index);
        PyObject* ret = self.attr().try_get(key);
        if(ret == nullptr) vm->AttributeError(key.sv());
        return ret;
    });

    _vm->bind__repr__(_vm->tp_mappingproxy, [](VM* vm, PyObject* obj) {
        MappingProxy& self = _CAST(MappingProxy&, obj);
        std::stringstream ss;
        ss << "mappingproxy({";
        bool first = true;
        for(auto& item : self.attr().items()){
            if(!first) ss << ", ";
            first = false;
            ss << item.first.escape() << ": " << CAST(Str, vm->py_repr(item.second));
        }
        ss << "})";
        return VAR(ss.str());
    });

    _vm->bind__contains__(_vm->tp_mappingproxy, [](VM* vm, PyObject* obj, PyObject* key) {
        MappingProxy& self = _CAST(MappingProxy&, obj);
        return self.attr().contains(CAST(Str&, key));
    });

    /************ dict ************/
    _vm->bind_constructor<-1>("dict", [](VM* vm, ArgsView args){
        return VAR(Dict(vm));
    });

    _vm->bind_method<-1>("dict", "__init__", [](VM* vm, ArgsView args){
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

    _vm->bind__len__(_vm->tp_dict, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(Dict&, obj).size();
    });

    _vm->bind__getitem__(_vm->tp_dict, [](VM* vm, PyObject* obj, PyObject* index) {
        Dict& self = _CAST(Dict&, obj);
        PyObject* ret = self.try_get(index);
        if(ret == nullptr) vm->KeyError(index);
        return ret;
    });

    _vm->bind__setitem__(_vm->tp_dict, [](VM* vm, PyObject* obj, PyObject* key, PyObject* value) {
        Dict& self = _CAST(Dict&, obj);
        self.set(key, value);
    });

    _vm->bind__delitem__(_vm->tp_dict, [](VM* vm, PyObject* obj, PyObject* key) {
        Dict& self = _CAST(Dict&, obj);
        if(!self.contains(key)) vm->KeyError(key);
        self.erase(key);
    });

    _vm->bind_method<1>("dict", "pop", [](VM* vm, ArgsView args) {
        Dict& self = _CAST(Dict&, args[0]);
        PyObject* value = self.try_get(args[1]);
        if(value == nullptr) vm->KeyError(args[1]);
        self.erase(args[1]);
        return value;
    });

    _vm->bind__contains__(_vm->tp_dict, [](VM* vm, PyObject* obj, PyObject* key) {
        Dict& self = _CAST(Dict&, obj);
        return self.contains(key);
    });

    _vm->bind_method<-1>("dict", "get", [](VM* vm, ArgsView args) {
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

    _vm->bind_method<0>("dict", "keys", [](VM* vm, ArgsView args) {
        Dict& self = _CAST(Dict&, args[0]);
        List keys;
        for(auto& item : self.items()) keys.push_back(item.first);
        return VAR(std::move(keys));
    });

    _vm->bind_method<0>("dict", "values", [](VM* vm, ArgsView args) {
        Dict& self = _CAST(Dict&, args[0]);
        List values;
        for(auto& item : self.items()) values.push_back(item.second);
        return VAR(std::move(values));
    });

    _vm->bind_method<0>("dict", "items", [](VM* vm, ArgsView args) {
        Dict& self = _CAST(Dict&, args[0]);
        List items;
        for(auto& item : self.items()){
            PyObject* t = VAR(Tuple({item.first, item.second}));
            items.push_back(std::move(t));
        }
        return VAR(std::move(items));
    });

    _vm->bind_method<1>("dict", "update", [](VM* vm, ArgsView args) {
        Dict& self = _CAST(Dict&, args[0]);
        const Dict& other = CAST(Dict&, args[1]);
        self.update(other);
        return vm->None;
    });

    _vm->bind_method<0>("dict", "copy", [](VM* vm, ArgsView args) {
        const Dict& self = _CAST(Dict&, args[0]);
        return VAR(self);
    });

    _vm->bind_method<0>("dict", "clear", [](VM* vm, ArgsView args) {
        Dict& self = _CAST(Dict&, args[0]);
        self.clear();
        return vm->None;
    });

    _vm->bind__repr__(_vm->tp_dict, [](VM* vm, PyObject* obj) {
        Dict& self = _CAST(Dict&, obj);
        std::stringstream ss;
        ss << "{";
        bool first = true;
        for(auto& item : self.items()){
            if(!first) ss << ", ";
            first = false;
            Str key = CAST(Str&, vm->py_repr(item.first));
            Str value = CAST(Str&, vm->py_repr(item.second));
            ss << key << ": " << value;
        }
        ss << "}";
        return VAR(ss.str());
    });

    _vm->bind__json__(_vm->tp_dict, [](VM* vm, PyObject* obj) {
        Dict& self = _CAST(Dict&, obj);
        std::stringstream ss;
        ss << "{";
        bool first = true;
        for(auto& item : self.items()){
            if(!first) ss << ", ";
            first = false;
            Str key = CAST(Str&, item.first).escape(false);
            Str value = CAST(Str&, vm->py_json(item.second));
            ss << key << ": " << value;
        }
        ss << "}";
        return VAR(ss.str());
    });

    _vm->bind__eq__(_vm->tp_dict, [](VM* vm, PyObject* a, PyObject* b) {
        Dict& self = _CAST(Dict&, a);
        if(!is_non_tagged_type(b, vm->tp_dict)) return false;
        Dict& other = _CAST(Dict&, b);
        if(self.size() != other.size()) return false;
        for(auto& item : self.items()){
            PyObject* value = other.try_get(item.first);
            if(value == nullptr) return false;
            if(!vm->py_equals(item.second, value)) return false;
        }
        return true;
    });
    /************ property ************/
    _vm->bind_constructor<-1>("property", [](VM* vm, ArgsView args) {
        if(args.size() == 1+1){
            return VAR(Property(args[1], vm->None));
        }else if(args.size() == 1+2){
            return VAR(Property(args[1], args[2]));
        }
        vm->TypeError("property() takes at most 2 arguments");
        return vm->None;
    });

    RangeIter::register_class(_vm, _vm->builtins);
    ArrayIter::register_class(_vm, _vm->builtins);
    StringIter::register_class(_vm, _vm->builtins);
    Generator::register_class(_vm, _vm->builtins);
}

inline void add_module_time(VM* vm){
    PyObject* mod = vm->new_module("time");
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
        std::tm* tm = std::localtime(&t);
        Dict d(vm);
        d.set(VAR("tm_year"), VAR(tm->tm_year + 1900));
        d.set(VAR("tm_mon"), VAR(tm->tm_mon + 1));
        d.set(VAR("tm_mday"), VAR(tm->tm_mday));
        d.set(VAR("tm_hour"), VAR(tm->tm_hour));
        d.set(VAR("tm_min"), VAR(tm->tm_min));
        d.set(VAR("tm_sec"), VAR(tm->tm_sec + 1));
        d.set(VAR("tm_wday"), VAR((tm->tm_wday + 6) % 7));
        d.set(VAR("tm_yday"), VAR(tm->tm_yday + 1));
        d.set(VAR("tm_isdst"), VAR(tm->tm_isdst));
        return VAR(std::move(d));
    });
}

struct PyREPL{
    PY_CLASS(PyREPL, sys, _repl)

    REPL* repl;

    PyREPL(VM* vm){ repl = new REPL(vm); }
    ~PyREPL(){ delete repl; }

    PyREPL(const PyREPL&) = delete;
    PyREPL& operator=(const PyREPL&) = delete;

    PyREPL(PyREPL&& other) noexcept{
        repl = other.repl;
        other.repl = nullptr;
    }

    struct TempOut{
        PrintFunc backup;
        VM* vm;
        TempOut(VM* vm, PrintFunc f){
            this->vm = vm;
            this->backup = vm->_stdout;
            vm->_stdout = f;
        }
        ~TempOut(){
            vm->_stdout = backup;
        }
        TempOut(const TempOut&) = delete;
        TempOut& operator=(const TempOut&) = delete;
        TempOut(TempOut&&) = delete;
        TempOut& operator=(TempOut&&) = delete;
    };

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<1>(type, [](VM* vm, ArgsView args){
            return VAR_T(PyREPL, vm);
        });

        vm->bind_method<1>(type, "input", [](VM* vm, ArgsView args){
            PyREPL& self = _CAST(PyREPL&, args[0]);
            const Str& s = CAST(Str&, args[1]);
            static std::stringstream ss_out;
            ss_out.str("");
            TempOut _(vm, [](VM* vm, const Str& s){ ss_out << s; });
            bool ok = self.repl->input(s.str());
            return VAR(Tuple({VAR(ok), VAR(ss_out.str())}));
        });
    }
};

inline void add_module_sys(VM* vm){
    PyObject* mod = vm->new_module("sys");
    PyREPL::register_class(vm, mod);
    vm->setattr(mod, "version", VAR(PK_VERSION));

    PyObject* stdout_ = vm->heap.gcnew<DummyInstance>(vm->tp_object, {});
    PyObject* stderr_ = vm->heap.gcnew<DummyInstance>(vm->tp_object, {});
    PyObject* stdin_ = vm->heap.gcnew<DummyInstance>(vm->tp_object, {});
    vm->setattr(mod, "stdout", stdout_);
    vm->setattr(mod, "stderr", stderr_);
    vm->setattr(mod, "stdin", stdin_);

    vm->bind_func<1>(stdout_, "write", [](VM* vm, ArgsView args) {
        vm->_stdout(vm, CAST(Str&, args[0]));
        return vm->None;
    });

    vm->bind_func<1>(stderr_, "write", [](VM* vm, ArgsView args) {
        vm->_stderr(vm, CAST(Str&, args[0]));
        return vm->None;
    });
}

inline void add_module_json(VM* vm){
    PyObject* mod = vm->new_module("json");
    vm->bind_func<1>(mod, "loads", [](VM* vm, ArgsView args) {
        const Str& expr = CAST(Str&, args[0]);
        CodeObject_ code = vm->compile(expr, "<json>", JSON_MODE);
        return vm->_exec(code, vm->top_frame()->_module);
    });

    vm->bind_func<1>(mod, "dumps", [](VM* vm, ArgsView args) {
        return vm->py_json(args[0]);
    });
}


// https://docs.python.org/3.5/library/math.html
inline void add_module_math(VM* vm){
    PyObject* mod = vm->new_module("math");
    mod->attr().set("pi", VAR(3.1415926535897932384));
    mod->attr().set("e" , VAR(2.7182818284590452354));
    mod->attr().set("inf", VAR(std::numeric_limits<double>::infinity()));
    mod->attr().set("nan", VAR(std::numeric_limits<double>::quiet_NaN()));

    vm->bind_func<1>(mod, "ceil", CPP_LAMBDA(VAR((i64)std::ceil(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "fabs", CPP_LAMBDA(VAR(std::fabs(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "floor", CPP_LAMBDA(VAR((i64)std::floor(CAST_F(args[0])))));
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

    vm->bind_func<1>(mod, "isfinite", CPP_LAMBDA(VAR(std::isfinite(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "isinf", CPP_LAMBDA(VAR(std::isinf(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "isnan", CPP_LAMBDA(VAR(std::isnan(CAST_F(args[0])))));

    vm->bind_func<1>(mod, "exp", CPP_LAMBDA(VAR(std::exp(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "log", CPP_LAMBDA(VAR(std::log(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "log2", CPP_LAMBDA(VAR(std::log2(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "log10", CPP_LAMBDA(VAR(std::log10(CAST_F(args[0])))));

    vm->bind_func<2>(mod, "pow", CPP_LAMBDA(VAR(std::pow(CAST_F(args[0]), CAST_F(args[1])))));
    vm->bind_func<1>(mod, "sqrt", CPP_LAMBDA(VAR(std::sqrt(CAST_F(args[0])))));

    vm->bind_func<1>(mod, "acos", CPP_LAMBDA(VAR(std::acos(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "asin", CPP_LAMBDA(VAR(std::asin(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "atan", CPP_LAMBDA(VAR(std::atan(CAST_F(args[0])))));
    vm->bind_func<2>(mod, "atan2", CPP_LAMBDA(VAR(std::atan2(CAST_F(args[0]), CAST_F(args[1])))));

    vm->bind_func<1>(mod, "cos", CPP_LAMBDA(VAR(std::cos(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "sin", CPP_LAMBDA(VAR(std::sin(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "tan", CPP_LAMBDA(VAR(std::tan(CAST_F(args[0])))));
    
    vm->bind_func<1>(mod, "degrees", CPP_LAMBDA(VAR(CAST_F(args[0]) * 180 / 3.1415926535897932384)));
    vm->bind_func<1>(mod, "radians", CPP_LAMBDA(VAR(CAST_F(args[0]) * 3.1415926535897932384 / 180)));

    vm->bind_func<1>(mod, "modf", [](VM* vm, ArgsView args) {
        f64 i;
        f64 f = std::modf(CAST_F(args[0]), &i);
        return VAR(Tuple({VAR(f), VAR(i)}));
    });
}

inline void add_module_traceback(VM* vm){
    PyObject* mod = vm->new_module("traceback");
    vm->bind_func<0>(mod, "print_exc", [](VM* vm, ArgsView args) {
        if(vm->_last_exception==nullptr) vm->ValueError("no exception");
        Exception& e = CAST(Exception&, vm->_last_exception);
        vm->_stdout(vm, e.summary());
        return vm->None;
    });

    vm->bind_func<0>(mod, "format_exc", [](VM* vm, ArgsView args) {
        if(vm->_last_exception==nullptr) vm->ValueError("no exception");
        Exception& e = CAST(Exception&, vm->_last_exception);
        return VAR(e.summary());
    });
}

inline void add_module_dis(VM* vm){
    PyObject* mod = vm->new_module("dis");
    vm->bind_func<1>(mod, "dis", [](VM* vm, ArgsView args) {
        if(is_type(args[0], vm->tp_str)){
            const Str& source = CAST(Str, args[0]);
            CodeObject_ code = vm->compile(source, "<dis>", EXEC_MODE);
            vm->_stdout(vm, vm->disassemble(code));
            return vm->None;
        }
        PyObject* f = args[0];
        if(is_type(f, vm->tp_bound_method)) f = CAST(BoundMethod, args[0]).func;
        CodeObject_ code = CAST(Function&, f).decl->code;
        vm->_stdout(vm, vm->disassemble(code));
        return vm->None;
    });
}

inline void add_module_gc(VM* vm){
    PyObject* mod = vm->new_module("gc");
    vm->bind_func<0>(mod, "collect", CPP_LAMBDA(VAR(vm->heap.collect())));
}

inline void VM::post_init(){
    init_builtins(this);
#if !DEBUG_NO_BUILTIN_MODULES
    add_module_sys(this);
    add_module_traceback(this);
    add_module_time(this);
    add_module_json(this);
    add_module_math(this);
    add_module_re(this);
    add_module_dis(this);
    add_module_c(this);
    add_module_gc(this);
    add_module_random(this);
    add_module_base64(this);

    for(const char* name: {"this", "functools", "collections", "heapq", "bisect"}){
        _lazy_modules[name] = kPythonLibs[name];
    }

    CodeObject_ code = compile(kPythonLibs["builtins"], "<builtins>", EXEC_MODE);
    this->_exec(code, this->builtins);
    code = compile(kPythonLibs["_set"], "<set>", EXEC_MODE);
    this->_exec(code, this->builtins);

    // property is defined in builtins.py so we need to add it after builtins is loaded
    _t(tp_object)->attr().set("__class__", property(CPP_LAMBDA(vm->_t(args[0]))));
    _t(tp_type)->attr().set("__base__", property([](VM* vm, ArgsView args){
        const PyTypeInfo& info = vm->_all_types[OBJ_GET(Type, args[0])];
        return info.base.index == -1 ? vm->None : vm->_all_types[info.base].obj;
    }));
    _t(tp_type)->attr().set("__name__", property([](VM* vm, ArgsView args){
        const PyTypeInfo& info = vm->_all_types[OBJ_GET(Type, args[0])];
        return VAR(info.name);
    }));

    _t(tp_bound_method)->attr().set("__self__", property([](VM* vm, ArgsView args){
        return CAST(BoundMethod&, args[0]).self;
    }));
    _t(tp_bound_method)->attr().set("__func__", property([](VM* vm, ArgsView args){
        return CAST(BoundMethod&, args[0]).func;
    }));

    bind__eq__(tp_bound_method, [](VM* vm, PyObject* lhs, PyObject* rhs){
        if(!is_non_tagged_type(rhs, vm->tp_bound_method)) return false;
        return _CAST(BoundMethod&, lhs) == _CAST(BoundMethod&, rhs);
    });
    _t(tp_slice)->attr().set("start", property([](VM* vm, ArgsView args){
        return CAST(Slice&, args[0]).start;
    }));
    _t(tp_slice)->attr().set("stop", property([](VM* vm, ArgsView args){
        return CAST(Slice&, args[0]).stop;
    }));
    _t(tp_slice)->attr().set("step", property([](VM* vm, ArgsView args){
        return CAST(Slice&, args[0]).step;
    }));

    _t(tp_object)->attr().set("__dict__", property([](VM* vm, ArgsView args){
        if(is_tagged(args[0]) || !args[0]->is_attr_valid()) vm->AttributeError("'__dict__'");
        return VAR(MappingProxy(args[0]));
    }));

    if(enable_os){
        add_module_io(this);
        add_module_os(this);
        add_module_requests(this);
        _import_handler = _default_import_handler;
    }

    add_module_linalg(this);
    add_module_easing(this);
#endif
}

}   // namespace pkpy

/*************************GLOBAL NAMESPACE*************************/
extern "C" {
    PK_LEGACY_EXPORT
    void pkpy_free(void* p){
        free(p);
    }

    PK_LEGACY_EXPORT
    void pkpy_vm_exec(pkpy::VM* vm, const char* source){
        vm->exec(source, "main.py", pkpy::EXEC_MODE);
    }

    PK_LEGACY_EXPORT
    void pkpy_vm_exec_2(pkpy::VM* vm, const char* source, const char* filename, int mode, const char* module){
        pkpy::PyObject* mod;
        if(module == nullptr) mod = vm->_main;
        else{
            mod = vm->_modules.try_get(module);
            if(mod == nullptr) return;
        }
        vm->exec(source, filename, (pkpy::CompileMode)mode, mod);
    }

    PK_LEGACY_EXPORT
    pkpy::REPL* pkpy_new_repl(pkpy::VM* vm){
        pkpy::REPL* p = new pkpy::REPL(vm);
        return p;
    }

    PK_LEGACY_EXPORT
    bool pkpy_repl_input(pkpy::REPL* r, const char* line){
        return r->input(line);
    }

    PK_LEGACY_EXPORT
    void pkpy_vm_add_module(pkpy::VM* vm, const char* name, const char* source){
        vm->_lazy_modules[name] = source;
    }

    PK_LEGACY_EXPORT
    pkpy::VM* pkpy_new_vm(bool enable_os=true){
        pkpy::VM* p = new pkpy::VM(enable_os);
        return p;
    }

    PK_LEGACY_EXPORT
    void pkpy_delete_vm(pkpy::VM* vm){
        delete vm;
    }

    PK_LEGACY_EXPORT
    void pkpy_delete_repl(pkpy::REPL* repl){
        delete repl;
    }

    PK_LEGACY_EXPORT
    void pkpy_vm_gc_on_delete(pkpy::VM* vm, void (*f)(pkpy::VM *, pkpy::PyObject *)){
        vm->heap._gc_on_delete = f;
    }
}


#endif // POCKETPY_H