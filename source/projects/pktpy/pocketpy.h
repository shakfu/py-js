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

// Enable this for `vm->_ceval_on_step`
#define PK_ENABLE_CEVAL_CALLBACK    0

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

// This is the maximum number of arguments in a function declaration
// including positional arguments, keyword-only arguments, and varargs
// (not recommended to change this)
#define PK_MAX_CO_VARNAMES          255

namespace pkpy{
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
}

#ifdef _MSC_VER
#pragma warning (disable:4267)
#pragma warning (disable:4100)
#pragma warning (disable:4244)
#pragma warning (disable:4996)
#endif

#ifdef _MSC_VER
#define PK_ENABLE_COMPUTED_GOTO		0
#define UNREACHABLE()				__assume(0)
#else
#define PK_ENABLE_COMPUTED_GOTO		1
#define UNREACHABLE()				__builtin_unreachable()
#endif


#if PK_DEBUG_CEVAL_STEP && defined(PK_ENABLE_COMPUTED_GOTO)
#undef PK_ENABLE_COMPUTED_GOTO
#endif

/*************** module settings ***************/

#define PK_MODULE_RE                1
#define PK_MODULE_RANDOM            1
#define PK_MODULE_EASING            1

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

#define PK_VERSION				"1.0.9"

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
#define PK_GLOBAL_SCOPE_LOCK() auto _lock = GIL();

#else
#define THREAD_LOCAL
#define PK_GLOBAL_SCOPE_LOCK()
#endif

/*******************************************************************************/

#define PK_UNUSED(x) (void)(x)

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

#define PK_LAMBDA(x) ([](VM* vm, ArgsView args) { return x; })
#define PK_VAR_LAMBDA(x) ([](VM* vm, ArgsView args) { return VAR(x); })
#define PK_ACTION(x) ([](VM* vm, ArgsView args) { x; return vm->None; })

#ifdef POCKETPY_H
#define FATAL_ERROR() throw std::runtime_error( "L" + std::to_string(__LINE__) + " FATAL_ERROR()!");
#else
#define FATAL_ERROR() throw std::runtime_error( __FILE__ + std::string(":") + std::to_string(__LINE__) + " FATAL_ERROR()!");
#endif

#define PK_ASSERT(x) if(!(x)) FATAL_ERROR();

struct PyObject;
#define PK_BITS(p) (reinterpret_cast<i64>(p))
inline bool is_tagged(PyObject* p) noexcept { return (PK_BITS(p) & 0b11) != 0b00; }
inline bool is_int(PyObject* p) noexcept { return (PK_BITS(p) & 0b11) == 0b01; }
inline bool is_float(PyObject* p) noexcept { return (PK_BITS(p) & 0b11) == 0b10; }
inline bool is_special(PyObject* p) noexcept { return (PK_BITS(p) & 0b11) == 0b11; }

inline bool is_both_int_or_float(PyObject* a, PyObject* b) noexcept {
    return is_tagged(a) && is_tagged(b);
}

inline bool is_both_int(PyObject* a, PyObject* b) noexcept {
    return is_int(a) && is_int(b);
}

inline bool is_both_float(PyObject* a, PyObject* b) noexcept {
	return is_float(a) && is_float(b);
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

    template<typename __T>
    void* alloc() { return alloc(sizeof(__T)); }

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
            arena->dirty = true;
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

    void reverse(){
        std::reverse(_data, _data+_size);
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
    char _inlined[16];

    mutable const char* _cached_c_str = nullptr;

    bool is_inlined() const { return data == _inlined; }

    Str(): size(0), is_ascii(true), data(_inlined) {}

    void _alloc(){
        if(size <= 16){
            this->data = _inlined;
        }else{
            this->data = (char*)pool64.alloc(size);
        }
    }

    Str(int size, bool is_ascii): size(size), is_ascii(is_ascii) {
        _alloc();
    }

#define STR_INIT()                                  \
        _alloc();                                   \
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
        _alloc();
        memcpy(data, other.data, size);
    }

    Str(Str&& other): size(other.size), is_ascii(other.is_ascii) {
        if(other.is_inlined()){
            data = _inlined;
            for(int i=0; i<size; i++) _inlined[i] = other._inlined[i];
        }else{
            data = other.data;
            other.data = other._inlined;
            other.size = 0;
        }
    }

    const char* begin() const { return data; }
    const char* end() const { return data + size; }
    char operator[](int idx) const { return data[idx]; }
    int length() const { return size; }
    bool empty() const { return size == 0; }
    size_t hash() const{ return std::hash<std::string_view>()(sv()); }

    Str& operator=(const Str& other);
    Str operator+(const Str& other) const;
    Str operator+(const char* p) const;

    bool operator==(const Str& other) const;
    bool operator!=(const Str& other) const;
    bool operator==(const std::string_view other) const;
    bool operator!=(const std::string_view other) const;
    bool operator==(const char* p) const;
    bool operator!=(const char* p) const;
    bool operator<(const Str& other) const;
    bool operator>(const Str& other) const;
    bool operator<=(const Str& other) const;
    bool operator>=(const Str& other) const;
    bool operator<(const std::string_view other) const;

    ~Str();

    friend Str operator+(const char* p, const Str& str){
        Str other(p);
        return other + str;
    }

    friend std::ostream& operator<<(std::ostream& os, const Str& str){
        os.write(str.data, str.size);
        return os;
    }

    friend bool operator<(const std::string_view other, const Str& str){
        return str > other;
    }

    Str substr(int start, int len) const;
    Str substr(int start) const;
    char* c_str_dup() const;
    const char* c_str() const;
    std::string_view sv() const;
    std::string str() const;
    Str lstrip() const;
    Str strip() const;
    Str lower() const;
    Str upper() const;
    Str escape(bool single_quote=true) const;
    int index(const Str& sub, int start=0) const;
    Str replace(const Str& old, const Str& new_, int count=-1) const;

    /*************unicode*************/
    int _unicode_index_to_byte(int i) const;
    int _byte_index_to_unicode(int n) const;
    Str u8_getitem(int i) const;
    Str u8_slice(int start, int stop, int step) const;
    int u8_length() const;
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

    static bool is_valid(int index) {
        // check _r_interned[index-1] is valid
        return index > 0 && index <= _r_interned.size();
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
// indexer
const StrName __getitem__ = StrName::get("__getitem__");
const StrName __setitem__ = StrName::get("__setitem__");
const StrName __delitem__ = StrName::get("__delitem__");

#define DEF_SNAME(name) const static StrName name(#name)

} // namespace pkpy
namespace pkpy {

    Str& Str::operator=(const Str& other){
        if(!is_inlined()) pool64.dealloc(data);
        size = other.size;
        is_ascii = other.is_ascii;
        _alloc();
        memcpy(data, other.data, size);
        return *this;
    }

    Str Str::operator+(const Str& other) const {
        Str ret(size + other.size, is_ascii && other.is_ascii);
        memcpy(ret.data, data, size);
        memcpy(ret.data + size, other.data, other.size);
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
        int ret = strncmp(data, other.data, std::min(size, other.size));
        if(ret != 0) return ret < 0;
        return size < other.size;
    }

    bool Str::operator<(const std::string_view other) const {
        int ret = strncmp(data, other.data(), std::min(size, (int)other.size()));
        if(ret != 0) return ret < 0;
        return size < (int)other.size();
    }

    bool Str::operator>(const Str& other) const {
        int ret = strncmp(data, other.data, std::min(size, other.size));
        if(ret != 0) return ret > 0;
        return size > other.size;
    }

    bool Str::operator<=(const Str& other) const {
        int ret = strncmp(data, other.data, std::min(size, other.size));
        if(ret != 0) return ret < 0;
        return size <= other.size;
    }

    bool Str::operator>=(const Str& other) const {
        int ret = strncmp(data, other.data, std::min(size, other.size));
        if(ret != 0) return ret > 0;
        return size >= other.size;
    }

    Str::~Str(){
        if(!is_inlined()) pool64.dealloc(data);
        if(_cached_c_str != nullptr) free((void*)_cached_c_str);
    }

    Str Str::substr(int start, int len) const {
        Str ret(len, is_ascii);
        memcpy(ret.data, data + start, len);
        return ret;
    }

    Str Str::substr(int start) const {
        return substr(start, size - start);
    }

    char* Str::c_str_dup() const {
        char* p = (char*)malloc(size + 1);
        memcpy(p, data, size);
        p[size] = 0;
        return p;
    }

    const char* Str::c_str() const{
        if(_cached_c_str == nullptr){
            _cached_c_str = c_str_dup();
        }
        return _cached_c_str;
    }

    std::string_view Str::sv() const {
        return std::string_view(data, size);
    }

    std::string Str::str() const {
        return std::string(data, size);
    }

    Str Str::lstrip() const {
        std::string copy(data, size);
        copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](char c) {
            // std::isspace(c) does not working on windows (Debug)
            return c != ' ' && c != '\t' && c != '\r' && c != '\n';
        }));
        return Str(copy);
    }

    Str Str::strip() const {
        std::string copy(data, size);
        copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](char c) {
            return c != ' ' && c != '\t' && c != '\r' && c != '\n';
        }));
        copy.erase(std::find_if(copy.rbegin(), copy.rend(), [](char c) {
            return c != ' ' && c != '\t' && c != '\r' && c != '\n';
        }).base(), copy.end());
        return Str(copy);
    }

    Str Str::lower() const{
        std::string copy(data, size);
        std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c){ return std::tolower(c); });
        return Str(copy);
    }

    Str Str::upper() const{
        std::string copy(data, size);
        std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c){ return std::toupper(c); });
        return Str(copy);
    }

    Str Str::escape(bool single_quote) const {
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

    int Str::index(const Str& sub, int start) const {
        auto p = std::search(data + start, data + size, sub.data, sub.data + sub.size);
        if(p == data + size) return -1;
        return p - data;
    }

    Str Str::replace(const Str& old, const Str& new_, int count) const {
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
        std::stringstream ss;
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
} // namespace pkpy


namespace pkpy {

using List = pod_vector<PyObject*>;

struct Tuple {
    PyObject** _args;
    PyObject* _inlined[3];
    int _size;

    Tuple(int n);
    Tuple(std::initializer_list<PyObject*> list);
    Tuple(const Tuple& other);
    Tuple(Tuple&& other) noexcept;
    Tuple(List&& other) noexcept;
    ~Tuple();

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
        this->_args = (PyObject**)pool64.alloc(n * sizeof(void*));
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

Tuple::Tuple(std::initializer_list<PyObject*> list): Tuple(list.size()){
    int i = 0;
    for(PyObject* obj: list) _args[i++] = obj;
}

Tuple::~Tuple(){ if(!is_inlined()) pool64.dealloc(_args); }

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

inline const uint16_t kHashSeeds[] = {9629, 43049, 13267, 59509, 39251, 1249, 27689, 9719, 19913};

inline uint16_t _hash(StrName key, uint16_t mask, uint16_t hash_seed){
    return ( (key).index * (hash_seed) >> 8 ) & (mask);
}

uint16_t _find_perfect_hash_seed(uint16_t capacity, const std::vector<StrName>& keys);

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
        _hash_seed = _find_perfect_hash_seed(_capacity, keys());
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

uint16_t _find_perfect_hash_seed(uint16_t capacity, const std::vector<StrName>& keys){
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
    std::string source;
    Str filename;
    std::vector<const char*> line_starts;
    CompileMode mode;

    SourceData(const SourceData&) = delete;
    SourceData& operator=(const SourceData&) = delete;

    SourceData(const Str& source, const Str& filename, CompileMode mode);
    std::pair<const char*,const char*> get_line(int lineno) const;
    Str snapshot(int lineno, const char* cursor=nullptr);
};

struct Exception {
    StrName type;
    Str msg;
    bool is_re;
    stack<Str> stacktrace;

    Exception(StrName type, Str msg): type(type), msg(msg), is_re(true) {}
    bool match_type(StrName t) const { return this->type == t;}
    void st_push(Str snapshot);
    Str summary() const;
};

}   // namespace pkpy
namespace pkpy{

    SourceData::SourceData(const Str& source, const Str& filename, CompileMode mode) {
        int index = 0;
        // Skip utf8 BOM if there is any.
        if (strncmp(source.begin(), "\xEF\xBB\xBF", 3) == 0) index += 3;
        // Replace all '\r' with ' '
        std::stringstream ss;
        while(index < source.length()){
            if(source[index] == '\r') ss << ' ';
            else ss << source[index];
            index++;
        }

        this->filename = filename;
        this->source = ss.str();
        line_starts.push_back(this->source.c_str());
        this->mode = mode;
    }

    std::pair<const char*,const char*> SourceData::get_line(int lineno) const {
        if(lineno == -1) return {nullptr, nullptr};
        lineno -= 1;
        if(lineno < 0) lineno = 0;
        const char* _start = line_starts.at(lineno);
        const char* i = _start;
        while(*i != '\n' && *i != '\0') i++;
        return {_start, i};
    }

    Str SourceData::snapshot(int lineno, const char* cursor){
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

    void Exception::st_push(Str snapshot){
        if(stacktrace.size() >= 8) return;
        stacktrace.push(snapshot);
    }

    Str Exception::summary() const {
        stack<Str> st(stacktrace);
        std::stringstream ss;
        if(is_re) ss << "Traceback (most recent call last):\n";
        while(!st.empty()) { ss << st.top() << '\n'; st.pop(); }
        if (!msg.empty()) ss << type.sv() << ": " << msg;
        else ss << type.sv();
        return ss.str();
    }

}   // namespace pkpy


namespace pkpy{

typedef uint8_t TokenIndex;

constexpr const char* kTokens[] = {
    "is not", "not in", "yield from",
    "@eof", "@eol", "@sof",
    "@id", "@num", "@str", "@fstr", "@long",
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
  int brackets_level;
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
    void throw_err(Str type, Str msg);
    void throw_err(Str type, Str msg, int lineno, const char* cursor);
    void SyntaxError(Str msg){ throw_err("SyntaxError", msg); }
    void SyntaxError(){ throw_err("SyntaxError", "invalid syntax"); }
    void IndentationError(Str msg){ throw_err("IndentationError", msg); }
    Lexer(shared_ptr<SourceData> src);
    std::vector<Token> run();
};

} // namespace pkpy

namespace pkpy{

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
                    case 'x': {
                        char hex[3] = {eatchar(), eatchar(), '\0'};
                        size_t parsed;
                        char code;
                        try{
                            code = (char)Number::stoi(hex, &parsed, 16);
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
        }else{
            add_token(TK("@str"), s);
        }
    }

    void Lexer::eat_number() {
        static const std::regex pattern("^(0x)?[0-9a-fA-F]+(\\.[0-9]+)?(L)?");
        std::smatch m;

        const char* i = token_start;
        while(*i != '\n' && *i != '\0') i++;
        std::string s = std::string(token_start, i);

        bool ok = std::regex_search(s, m, pattern);
        PK_ASSERT(ok);
        // here is m.length()-1, since the first char was eaten by lex_token()
        for(int j=0; j<m.length()-1; j++) eatchar();

        if(m[3].matched){
            add_token(TK("@long"));
            return;
        }

        if(m[1].matched && m[2].matched){
            SyntaxError("hex literal should not contain a dot");
        }

        try{
            int base = 10;
            size_t size;
            if (m[1].matched) base = 16;
            if (m[2].matched) {
                PK_ASSERT(base == 10);
                add_token(TK("@num"), Number::stof(m[0], &size));
            } else {
                add_token(TK("@num"), Number::stoi(m[0], &size, base));
            }
            PK_ASSERT((int)size == (int)m.length());
        }catch(...){
            SyntaxError("invalid number literal");
        }
    }

    bool Lexer::lex_one_token() {
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
                } return false;
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
                        case 1: SyntaxError("invalid char: " + std::string(1, c)); break;
                        case 2: SyntaxError("invalid utf8 sequence: " + std::string(1, c)); break;
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

    void Lexer::throw_err(Str type, Str msg){
        int lineno = current_line;
        const char* cursor = curr_char;
        if(peekchar() == '\n'){
            lineno--;
            cursor--;
        }
        throw_err(type, msg, lineno, cursor);
    }

    void Lexer::throw_err(Str type, Str msg, int lineno, const char* cursor){
        auto e = Exception(type, msg);
        e.st_push(src->snapshot(lineno, cursor));
        throw e;
    }

    Lexer::Lexer(shared_ptr<SourceData> src) {
        this->src = src;
        this->token_start = src->source.c_str();
        this->curr_char = src->source.c_str();
        this->nexts.push_back(Token{TK("@sof"), token_start, 0, current_line, brackets_level});
        this->indents.push(0);
    }

    std::vector<Token> Lexer::run() {
        if(used) FATAL_ERROR();
        used = true;
        while (lex_one_token());
        return std::move(nexts);
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

    virtual ~PyObject();

    void enable_instance_dict(float lf=kInstAttrLoadFactor) {
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

#define PK_OBJ_GET(T, obj) (((Py_<T>*)(obj))->_value)

#define PK_OBJ_MARK(obj) \
    if(!is_tagged(obj) && !(obj)->gc.marked) {                      \
        (obj)->gc.marked = true;                                    \
        (obj)->_obj_gc_mark();                                      \
        if((obj)->is_attr_valid()) gc_mark_namedict((obj)->attr()); \
    }

inline void gc_mark_namedict(NameDict& t){
    if(t.size() == 0) return;
    for(uint16_t i=0; i<t._capacity; i++){
        if(t._items[i].first.empty()) continue;
        PK_OBJ_MARK(t._items[i].second);
    }
}

Str obj_type_name(VM* vm, Type type);

#if PK_DEBUG_NO_BUILTINS
#define OBJ_NAME(obj) Str("<?>")
#else
DEF_SNAME(__name__);
#define OBJ_NAME(obj) PK_OBJ_GET(Str, vm->getattr(obj, __name__))
#endif

const int kTpIntIndex = 2;
const int kTpFloatIndex = 3;

inline bool is_type(PyObject* obj, Type type) {
#if PK_DEBUG_EXTRA_CHECK
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
#if PK_DEBUG_EXTRA_CHECK
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
        return PK_OBJ_GET(T, obj);
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
        return PK_OBJ_GET(T, obj);
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
#define CAST_DEFAULT(T, x, default_value) (x != vm->None) ? py_cast<T>(vm, x) : (default_value)

/*****************************************************************/
template<>
struct Py_<List> final: PyObject {
    List _value;
    Py_(Type type, List&& val): PyObject(type), _value(std::move(val)) {}
    Py_(Type type, const List& val): PyObject(type), _value(val) {}

    void _obj_gc_mark() override {
        for(PyObject* obj: _value) PK_OBJ_MARK(obj);
    }
};

template<>
struct Py_<Tuple> final: PyObject {
    Tuple _value;
    Py_(Type type, Tuple&& val): PyObject(type), _value(std::move(val)) {}
    Py_(Type type, const Tuple& val): PyObject(type), _value(val) {}

    void _obj_gc_mark() override {
        for(PyObject* obj: _value) PK_OBJ_MARK(obj);
    }
};

template<>
struct Py_<MappingProxy> final: PyObject {
    MappingProxy _value;
    Py_(Type type, MappingProxy val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.obj);
    }
};

template<>
struct Py_<BoundMethod> final: PyObject {
    BoundMethod _value;
    Py_(Type type, BoundMethod val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.self);
        PK_OBJ_MARK(_value.func);
    }
};

template<>
struct Py_<StarWrapper> final: PyObject {
    StarWrapper _value;
    Py_(Type type, StarWrapper val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.obj);
    }
};

template<>
struct Py_<Property> final: PyObject {
    Property _value;
    Py_(Type type, Property val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.getter);
        PK_OBJ_MARK(_value.setter);
    }
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
};

template<>
struct Py_<Super> final: PyObject {
    Super _value;
    Py_(Type type, Super val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.first);
    }
};

template<>
struct Py_<DummyInstance> final: PyObject {
    Py_(Type type, DummyInstance val): PyObject(type) {
        PK_UNUSED(val);
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
        PK_UNUSED(val);
        enable_instance_dict(kTypeAttrLoadFactor);
    }
    void _obj_gc_mark() override {}
};

}   // namespace pkpy
namespace pkpy{
    PyObject::~PyObject() {
        if(_attr == nullptr) return;
        _attr->~NameDict();
        pool64.dealloc(_attr);
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

    void _probe(PyObject* key, bool& ok, int& i) const;

    void set(PyObject* key, PyObject* val);
    void _rehash();

    PyObject* try_get(PyObject* key) const;

    bool contains(PyObject* key) const;
    void erase(PyObject* key);
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
        _items = (Item*)pool128.alloc(_capacity * sizeof(Item));
        memset(_items, 0, _capacity * sizeof(Item));
        _nodes = (ItemNode*)pool64.alloc(_capacity * sizeof(ItemNode));
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
        _items = (Item*)pool128.alloc(_capacity * sizeof(Item));
        memcpy(_items, other._items, _capacity * sizeof(Item));
        _nodes = (ItemNode*)pool64.alloc(_capacity * sizeof(ItemNode));
        memcpy(_nodes, other._nodes, _capacity * sizeof(ItemNode));
    }

    void Dict::set(PyObject* key, PyObject* val){
        // do possible rehash
        if(_size+1 > _critical_size) _rehash();
        bool ok; int i;
        _probe(key, ok, i);
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
        int old_capacity = _capacity;
        _capacity *= 2;
        _mask = _capacity - 1;
        _size = 0;
        _critical_size = _capacity*__LoadFactor+0.5f;
        _head_idx = -1;
        _tail_idx = -1;
        pool64.dealloc(_nodes);
        _items = (Item*)pool128.alloc(_capacity * sizeof(Item));
        memset(_items, 0, _capacity * sizeof(Item));
        _nodes = (ItemNode*)pool64.alloc(_capacity * sizeof(ItemNode));
        memset(_nodes, -1, _capacity * sizeof(ItemNode));

        for(int i=0; i<old_capacity; i++){
            if(old_items[i].first == nullptr) continue;
            set(old_items[i].first, old_items[i].second);
        }
        pool128.dealloc(old_items);
    }


    PyObject* Dict::try_get(PyObject* key) const{
        bool ok; int i;
        _probe(key, ok, i);
        if(!ok) return nullptr;
        return _items[i].second;
    }

    bool Dict::contains(PyObject* key) const{
        bool ok; int i;
        _probe(key, ok, i);
        return ok;
    }

    void Dict::erase(PyObject* key){
        bool ok; int i;
        _probe(key, ok, i);
        if(!ok) return;
        _items[i].first = nullptr;
        _items[i].second = nullptr;
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
        pool128.dealloc(_items);
        pool64.dealloc(_nodes);
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
OPCODE(BUILD_LONG)
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
OPCODE(BUILD_LONG)
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

inline const int BC_NOARG = -1;
inline const int BC_KEEPLINE = -1;

struct CodeBlock {
    CodeBlockType type;
    int parent;         // parent index in blocks
    int for_loop_depth; // this is used for exception handling
    int start;          // start index of this block in codes, inclusive
    int end;            // end index of this block in codes, exclusive

    CodeBlock(CodeBlockType type, int parent, int for_loop_depth, int start):
        type(type), parent(parent), for_loop_depth(for_loop_depth), start(start), end(-1) {}
};

struct CodeObject;
typedef shared_ptr<CodeObject> CodeObject_;
struct FuncDecl;
using FuncDecl_ = shared_ptr<FuncDecl>;

struct CodeObjectSerializer{
    std::string buffer;
    int depth = 0;

    std::set<StrName> names;

    static const char END = '\n';

    CodeObjectSerializer();

    void write_int(i64 v);
    void write_float(f64 v);
    void write_str(const Str& v);
    void write_none();
    void write_ellipsis();
    void write_bool(bool v);
    void write_begin_mark();
    void write_name(StrName name);
    void write_end_mark();

    template<typename T>
    void write_bytes(T v){
        static_assert(std::is_trivially_copyable<T>::value);
        buffer += 'x';
        char* p = (char*)&v;
        for(int i=0; i<sizeof(T); i++){
            char c = p[i];
            buffer += "0123456789abcdef"[(c >> 4) & 0xf];
            buffer += "0123456789abcdef"[c & 0xf];
        }
        buffer += END;
    }

    void write_object(VM* vm, PyObject* obj);
    void write_code(VM* vm, const CodeObject* co);
    std::string str();
};


struct CodeObject {
    shared_ptr<SourceData> src;
    Str name;
    bool is_generator = false;

    std::vector<Bytecode> codes;
    std::vector<int> lines; // line number for each bytecode
    List consts;
    std::vector<StrName> varnames;      // local variables
    NameDictInt varnames_inv;
    std::vector<CodeBlock> blocks = { CodeBlock(NO_BLOCK, -1, 0, 0) };
    NameDictInt labels;
    std::vector<FuncDecl_> func_decls;

    CodeObject(shared_ptr<SourceData> src, const Str& name);
    void _gc_mark() const;
    void write(VM* vm, CodeObjectSerializer& ss) const;
    Str serialize(VM* vm) const;
};

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

    Str signature;              // signature of this function
    Str docstring;              // docstring of this function
    void _gc_mark() const;
};

struct NativeFunc {
    NativeFuncC f;

    // old style argc-based call
    int argc;

    // new style decl-based call
    FuncDecl_ decl;

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
#if PK_DEBUG_EXTRA_CHECK
        if(!_has_userdata) throw std::runtime_error("userdata not set");
#endif
        return reinterpret_cast<const T&>(_userdata);
    }
    
    NativeFunc(NativeFuncC f, int argc, bool method);
    NativeFunc(NativeFuncC f, FuncDecl_ decl);

    void check_size(VM* vm, ArgsView args) const;
    PyObject* call(VM* vm, ArgsView args) const;
};

struct Function{
    FuncDecl_ decl;
    PyObject* _module;
    NameDict_ _closure;
};

template<>
struct Py_<Function> final: PyObject {
    Function _value;
    Py_(Type type, Function val): PyObject(type), _value(val) {
        enable_instance_dict();
    }
    void _obj_gc_mark() override {
        _value.decl->_gc_mark();
        if(_value._module != nullptr) PK_OBJ_MARK(_value._module);
        if(_value._closure != nullptr) gc_mark_namedict(*_value._closure);
    }
};

template<>
struct Py_<NativeFunc> final: PyObject {
    NativeFunc _value;
    Py_(Type type, NativeFunc val): PyObject(type), _value(val) {
        enable_instance_dict();
    }
    void _obj_gc_mark() override {
        if(_value.decl != nullptr){
            _value.decl->_gc_mark();
        }
    }
};

template<typename T>
T lambda_get_userdata(PyObject** p){
    if(p[-1] != PY_NULL) return PK_OBJ_GET(NativeFunc, p[-1]).get_userdata<T>();
    else return PK_OBJ_GET(NativeFunc, p[-2]).get_userdata<T>();
}

} // namespace pkpy
namespace pkpy{

    CodeObject::CodeObject(shared_ptr<SourceData> src, const Str& name):
        src(src), name(name) {}

    void CodeObject::_gc_mark() const {
        for(PyObject* v : consts) PK_OBJ_MARK(v);
        for(auto& decl: func_decls) decl->_gc_mark();
    }

    void CodeObject::write(VM* vm, CodeObjectSerializer& ss) const{
        ss.write_begin_mark();          // [
        ss.write_str(src->filename);    // src->filename
        ss.write_int(src->mode);        // src->mode
        ss.write_end_mark();            // ]
        ss.write_str(name);             // name
        ss.write_bool(is_generator);    // is_generator
        ss.write_begin_mark();          // [
            for(Bytecode bc: codes){
                if(StrName::is_valid(bc.arg)) ss.names.insert(StrName(bc.arg));
                ss.write_bytes(bc);
            }
        ss.write_end_mark();            // ]
        ss.write_begin_mark();          // [
            for(int line: lines){
                ss.write_int(line);         // line
            }
        ss.write_end_mark();            // ]
        ss.write_begin_mark();          // [
            for(PyObject* o: consts){
                ss.write_object(vm, o);
            }
        ss.write_end_mark();            // ]
        ss.write_begin_mark();          // [
            for(StrName vn: varnames){
                ss.write_name(vn);        // name
            }
        ss.write_end_mark();            // ]
        ss.write_begin_mark();          // [
            for(CodeBlock block: blocks){
                ss.write_bytes(block);      // block
            }
        ss.write_end_mark();            // ]
        ss.write_begin_mark();          // [
            for(auto& label: labels.items()){
                ss.write_name(label.first);     // label.first
                ss.write_int(label.second);     // label.second
            }
        ss.write_end_mark();            // ]
        ss.write_begin_mark();          // [
            for(auto& decl: func_decls){
                ss.write_code(vm, decl->code.get()); // decl->code
                ss.write_begin_mark();      // [
                    for(int arg: decl->args) ss.write_int(arg);
                ss.write_end_mark();        // ]
                
                ss.write_begin_mark();      // [
                    for(auto kw: decl->kwargs){
                        ss.write_int(kw.key);           // kw.key
                        ss.write_object(vm, kw.value);  // kw.value
                    }
                ss.write_end_mark();        // ]

                ss.write_int(decl->starred_arg);
                ss.write_int(decl->starred_kwarg);
                ss.write_bool(decl->nested);
            }
        ss.write_end_mark();            // ]
    }

    Str CodeObject::serialize(VM* vm) const{
        CodeObjectSerializer ss;
        ss.write_code(vm, this);
        return ss.str();
    }


    void CodeObjectSerializer::write_int(i64 v){
        buffer += 'i';
        buffer += std::to_string(v);
        buffer += END;
    }

    void CodeObjectSerializer::write_float(f64 v){
        buffer += 'f';
        buffer += std::to_string(v);
        buffer += END;
    }

    void CodeObjectSerializer::write_str(const Str& v){
        buffer += 's';
        buffer += v.escape(false).str();
        buffer += END;
    }

    void CodeObjectSerializer::write_none(){
        buffer += 'N';
        buffer += END;
    }

    void CodeObjectSerializer::write_ellipsis(){
        buffer += 'E';
        buffer += END;
    }

    void CodeObjectSerializer::write_bool(bool v){
        buffer += 'b';
        buffer += v ? '1' : '0';
        buffer += END;
    }

    void CodeObjectSerializer::write_begin_mark(){
        buffer += '[';
        buffer += END;
        depth++;
    }

    void CodeObjectSerializer::write_name(StrName name){
        PK_ASSERT(StrName::is_valid(name.index));
        buffer += 'n';
        buffer += std::to_string(name.index);
        buffer += END;
        names.insert(name);
    }

    void CodeObjectSerializer::write_end_mark(){
        buffer += ']';
        buffer += END;
        depth--;
        PK_ASSERT(depth >= 0);
    }

    std::string CodeObjectSerializer::str(){
        PK_ASSERT(depth == 0);
        for(auto name: names){
            PK_ASSERT(StrName::is_valid(name.index));
            write_name(name);
            write_str(name.sv());
        }
        return std::move(buffer);
    }

    CodeObjectSerializer::CodeObjectSerializer(){
        write_str(PK_VERSION);
    }

void CodeObjectSerializer::write_code(VM* vm, const CodeObject* co){
    buffer += '(';
    buffer += END;
    co->write(vm, *this);
    buffer += ')';
    buffer += END;
}

    NativeFunc::NativeFunc(NativeFuncC f, int argc, bool method){
        this->f = f;
        this->argc = argc;
        if(argc != -1) this->argc += (int)method;
        _has_userdata = false;
    }

    NativeFunc::NativeFunc(NativeFuncC f, FuncDecl_ decl){
        this->f = f;
        this->argc = -1;
        this->decl = decl;
        _has_userdata = false;
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
        if(_ip >= co->codes.size()) FATAL_ERROR();
#endif
        return co->codes[_ip];
    }

    Str snapshot();

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
        NameDict_ dict = make_sp<NameDict>();
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

    Str Frame::snapshot(){
        int line = co->lines[_ip];
        return co->src->snapshot(line);
    }

    bool Frame::jump_to_exception_handler(){
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

    int Frame::_exit_block(int i){
        if(co->blocks[i].type == FOR_LOOP) _s->pop();
        return co->blocks[i].parent;
    }

    void Frame::jump_abs_break(int target){
        const Bytecode& prev = co->codes[_ip];
        int i = prev.block;
        _next_ip = target;
        if(_next_ip >= co->codes.size()){
            while(i>=0) i = _exit_block(i);
        }else{
            // BUG!!!
            // for i in range(4):
            //     _ = 0
            // # if there is no op here, the block check will fail
            // while i: --i
            const Bytecode& next = co->codes[target];
            while(i>=0 && i!=next.block) i = _exit_block(i);
            if(i!=next.block) throw std::runtime_error("invalid jump");
        }
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
#if _WIN32
        // https://github.com/blueloveTH/pocketpy/issues/94#issuecomment-1594784476
        PyObject* obj = new(pool64.alloc<__T>()) Py_<std::decay_t<T>>(type, std::forward<T>(val));
#else
        PyObject* obj = new(pool64.alloc<__T>()) __T(type, std::forward<T>(val));
#endif
        gen.push_back(obj);
        gc_counter++;
        return obj;
    }

    template<typename T>
    PyObject* _new(Type type, T&& val){
        using __T = Py_<std::decay_t<T>>;
#if _WIN32
        // https://github.com/blueloveTH/pocketpy/issues/94#issuecomment-1594784476
        PyObject* obj = new(pool64.alloc<__T>()) Py_<std::decay_t<T>>(type, std::forward<T>(val));
#else
        PyObject* obj = new(pool64.alloc<__T>()) __T(type, std::forward<T>(val));
#endif
        obj->gc.enabled = false;
        _no_gc.push_back(obj);
        return obj;
    }

#if PK_DEBUG_GC_STATS
    inline static std::map<Type, int> deleted;
#endif

    int sweep();
    void _auto_collect();
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

    void ManagedHeap::_auto_collect(){
#if !PK_DEBUG_NO_AUTO_GC
        if(_gc_lock_counter > 0) return;
        if(gc_counter < gc_threshold) return;
        gc_counter = 0;
        collect();
        gc_threshold = gen.size() * 2;
        if(gc_threshold < kMinGCThreshold) gc_threshold = kMinGCThreshold;
#endif
    }

    int ManagedHeap::collect(){
        if(_gc_lock_counter > 0) FATAL_ERROR();
        mark();
        int freed = sweep();
        return freed;
    }

    ManagedHeap::~ManagedHeap(){
        for(PyObject* obj: _no_gc) { obj->~PyObject(); pool64.dealloc(obj); }
        for(PyObject* obj: gen) { obj->~PyObject(); pool64.dealloc(obj); }
#if PK_DEBUG_GC_STATS
        for(auto& [type, count]: deleted){
            std::cout << "GC: " << obj_type_name(vm, type) << "=" << count << std::endl;
        }
#endif
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
    inline PyObject* py_var(VM* vm, const ctype& value) { return vm->heap.gcnew(vm->ptype, value);}     \
    inline PyObject* py_var(VM* vm, ctype&& value) { return vm->heap.gcnew(vm->ptype, std::move(value));}


typedef PyObject* (*BinaryFuncC)(VM*, PyObject*, PyObject*);

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
};

struct FrameId{
    std::vector<pkpy::Frame>* data;
    int index;
    FrameId(std::vector<pkpy::Frame>* data, int index) : data(data), index(index) {}
    Frame* operator->() const { return &data->operator[](index); }
    Frame* get() const { return &data->operator[](index); }
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
    PyObject* NotImplemented;   // unused
    PyObject* Ellipsis;
    PyObject* builtins;         // builtins module
    PyObject* StopIteration;
    PyObject* _main;            // __main__ module

    PyObject* _last_exception;

#if PK_ENABLE_CEVAL_CALLBACK
    void (*_ceval_on_step)(VM*, Frame*, Bytecode bc) = nullptr;
#endif

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

    PyObject* cached_object__new__;

    const bool enable_os;

    VM(bool enable_os=true);

    FrameId top_frame();
    void _pop_frame();

    PyObject* py_str(PyObject* obj);
    PyObject* py_repr(PyObject* obj);
    PyObject* py_json(PyObject* obj);
    PyObject* py_iter(PyObject* obj);

    PyObject* find_name_in_mro(PyObject* cls, StrName name);
    bool isinstance(PyObject* obj, Type cls_t);
    PyObject* exec(Str source, Str filename, CompileMode mode, PyObject* _module=nullptr);

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

    PyObject* property(NativeFuncC fget, NativeFuncC fset=nullptr);
    PyObject* new_type_object(PyObject* mod, StrName name, Type base, bool subclass_enabled=true);
    Type _new_type_object(StrName name, Type base=0);
    PyObject* _find_type_object(const Str& type);

    Type _type(const Str& type);
    PyTypeInfo* _type_info(const Str& type);
    PyTypeInfo* _type_info(Type type);
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
    BIND_UNARY_SPECIAL(__json__)
    BIND_UNARY_SPECIAL(__neg__)
    BIND_UNARY_SPECIAL(__bool__)

    void bind__hash__(Type type, i64 (*f)(VM* vm, PyObject*));
    void bind__len__(Type type, i64 (*f)(VM* vm, PyObject*));
#undef BIND_UNARY_SPECIAL


#define BIND_BINARY_SPECIAL(name)                                                       \
    void bind##name(Type type, BinaryFuncC f){                                          \
        PyObject* obj = _t(type);                                                       \
        _all_types[type].m##name = f;                                                   \
        PyObject* nf = bind_method<1>(obj, #name, [](VM* vm, ArgsView args){            \
            return lambda_get_userdata<BinaryFuncC>(args.begin())(vm, args[0], args[1]); \
        });                                                                             \
        PK_OBJ_GET(NativeFunc, nf).set_userdata(f);                                        \
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

    void bind__getitem__(Type type, PyObject* (*f)(VM*, PyObject*, PyObject*)){
        PyObject* obj = _t(type);
        _all_types[type].m__getitem__ = f;
        PyObject* nf = bind_method<1>(obj, "__getitem__", [](VM* vm, ArgsView args){
            return lambda_get_userdata<PyObject*(*)(VM*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1]);
        });
        PK_OBJ_GET(NativeFunc, nf).set_userdata(f);
    }

    void bind__setitem__(Type type, void (*f)(VM*, PyObject*, PyObject*, PyObject*)){
        PyObject* obj = _t(type);
        _all_types[type].m__setitem__ = f;
        PyObject* nf = bind_method<2>(obj, "__setitem__", [](VM* vm, ArgsView args){
            lambda_get_userdata<void(*)(VM* vm, PyObject*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1], args[2]);
            return vm->None;
        });
        PK_OBJ_GET(NativeFunc, nf).set_userdata(f);
    }

    void bind__delitem__(Type type, void (*f)(VM*, PyObject*, PyObject*)){
        PyObject* obj = _t(type);
        _all_types[type].m__delitem__ = f;
        PyObject* nf = bind_method<1>(obj, "__delitem__", [](VM* vm, ArgsView args){
            lambda_get_userdata<void(*)(VM*, PyObject*, PyObject*)>(args.begin())(vm, args[0], args[1]);
            return vm->None;
        });
        PK_OBJ_GET(NativeFunc, nf).set_userdata(f);
    }

    bool py_equals(PyObject* lhs, PyObject* rhs);

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
            Type t = PK_OBJ_GET(Type, args[0]);
            return vm->heap.gcnew<T>(t, T());
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

    template<int ARGC>
    PyObject* bind_builtin_func(Str name, NativeFuncC fn) {
        return bind_func<ARGC>(builtins, name, fn);
    }

    int normalized_index(int index, int size);
    PyObject* py_next(PyObject* obj);
    
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
    void UnboundLocalError(StrName name){ _error("UnboundLocalError", fmt("local variable ", name.escape() + " referenced before assignment")); }
    void KeyError(PyObject* obj){ _error("KeyError", PK_OBJ_GET(Str, py_repr(obj))); }
    void BinaryOptError(const char* op) { TypeError(fmt("unsupported operand type(s) for ", op)); }

    void AttributeError(PyObject* obj, StrName name){
        // OBJ_NAME calls getattr, which may lead to a infinite recursion
        _error("AttributeError", fmt("type ", OBJ_NAME(_t(obj)).escape(), " has no attribute ", name.escape()));
    }

    void AttributeError(Str msg){ _error("AttributeError", msg); }

    void check_type(PyObject* obj, Type type){
        if(is_type(obj, type)) return;
        TypeError("expected " + OBJ_NAME(_t(type)).escape() + ", got " + OBJ_NAME(_t(obj)).escape());
    }

    void check_args_size(int size, int min_size, int max_size){
        if(size >= min_size && size <= max_size) return;
        TypeError(fmt("expected ", min_size, "-", max_size, " arguments, got ", size));
    }

    void check_non_tagged_type(PyObject* obj, Type type){
        if(is_non_tagged_type(obj, type)) return;
        TypeError("expected " + OBJ_NAME(_t(type)).escape() + ", got " + OBJ_NAME(_t(obj)).escape());
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
    PyObject* py_import(StrName name, bool relative=false);
    ~VM();

#if PK_DEBUG_CEVAL_STEP
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
    // new style binding api
    PyObject* bind(PyObject*, const char*, const char*, NativeFuncC);
    PyObject* bind(PyObject*, const char*, NativeFuncC);
    void _prepare_py_call(PyObject**, ArgsView, ArgsView, const FuncDecl_&);
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

#undef DEF_NATIVE_2

#define PY_CAST_INT(T)                                  \
template<> inline T py_cast<T>(VM* vm, PyObject* obj){  \
    vm->check_int(obj);                                 \
    return (T)(PK_BITS(obj) >> 2);                         \
}                                                       \
template<> inline T _py_cast<T>(VM* vm, PyObject* obj){ \
    PK_UNUSED(vm);                                      \
    return (T)(PK_BITS(obj) >> 2);                         \
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
    i64 bits = PK_BITS(obj) & Number::c1;
    return BitsCvt(bits)._float;
}
template<> inline float _py_cast<float>(VM* vm, PyObject* obj){
    PK_UNUSED(vm);
    i64 bits = PK_BITS(obj) & Number::c1;
    return BitsCvt(bits)._float;
}
template<> inline double py_cast<double>(VM* vm, PyObject* obj){
    vm->check_float(obj);
    i64 bits = PK_BITS(obj) & Number::c1;
    return BitsCvt(bits)._float;
}
template<> inline double _py_cast<double>(VM* vm, PyObject* obj){
    PK_UNUSED(vm);
    i64 bits = PK_BITS(obj) & Number::c1;
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
        PK_UNUSED(vm);                              \
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

template<> inline CString py_cast<CString>(VM* vm, PyObject* obj){
    vm->check_non_tagged_type(obj, vm->tp_str);
    return PK_OBJ_GET(Str, obj).c_str();
}

template<> inline CString _py_cast<CString>(VM* vm, PyObject* obj){
    return PK_OBJ_GET(Str, obj).c_str();
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
    PK_UNUSED(val);
    return vm->None;
}

inline PyObject* py_var(VM* vm, PyObject* val){
    PK_UNUSED(vm);
    return val;
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

    VM::VM(bool enable_os) : heap(this), enable_os(enable_os) {
        this->vm = this;
        _stdout = [](VM* vm, const Str& s) {
            PK_UNUSED(vm);
            std::cout << s;
        };
        _stderr = [](VM* vm, const Str& s) {
            PK_UNUSED(vm);
            std::cerr << s;
        };
        callstack.reserve(8);
        _main = nullptr;
        _last_exception = nullptr;
        _import_handler = [](const Str& name) {
            PK_UNUSED(name);
            return Bytes();
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
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__json__) return ti->m__json__(this, obj);
        return call_method(obj, __json__);
    }

    PyObject* VM::py_iter(PyObject* obj){
        const PyTypeInfo* ti = _inst_type_info(obj);
        if(ti->m__iter__) return ti->m__iter__(this, obj);
        PyObject* self;
        PyObject* iter_f = get_unbound_method(obj, __iter__, &self, false);
        if(self != PY_NULL) return call_method(self, iter_f);
        TypeError(OBJ_NAME(_t(obj)).escape() + " object is not iterable");
        return nullptr;
    }

    FrameId VM::top_frame(){
#if PK_DEBUG_EXTRA_CHECK
        if(callstack.empty()) FATAL_ERROR();
#endif
        return FrameId(&callstack.data(), callstack.size()-1);
    }

    void VM::_pop_frame(){
        Frame* frame = &callstack.top();
        s_data.reset(frame->_sp_base);
        callstack.pop();
    }

    PyObject* VM::find_name_in_mro(PyObject* cls, StrName name){
        PyObject* val;
        do{
            val = cls->attr().try_get(name);
            if(val != nullptr) return val;
            Type base = _all_types[PK_OBJ_GET(Type, cls)].base;
            if(base.index == -1) break;
            cls = _all_types[base].obj;
        }while(true);
        return nullptr;
    }

    bool VM::isinstance(PyObject* obj, Type cls_t){
        Type obj_t = PK_OBJ_GET(Type, _t(obj));
        do{
            if(obj_t == cls_t) return true;
            Type base = _all_types[obj_t].base;
            if(base.index == -1) break;
            obj_t = base;
        }while(true);
        return false;
    }

    PyObject* VM::exec(Str source, Str filename, CompileMode mode, PyObject* _module){
        if(_module == nullptr) _module = _main;
        try {
            CodeObject_ code = compile(source, filename, mode);
#if PK_DEBUG_DIS_EXEC
            if(_module == _main) std::cout << disassemble(code) << '\n';
#endif
            return _exec(code, _module);
        }catch (const Exception& e){
            _stderr(this, e.summary() + "\n");
        }
#if !PK_DEBUG_FULL_EXCEPTION
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

    PyObject* VM::property(NativeFuncC fget, NativeFuncC fset){
        PyObject* _0 = heap.gcnew(tp_native_func, NativeFunc(fget, 1, false));
        PyObject* _1 = vm->None;
        if(fset != nullptr) _1 = heap.gcnew(tp_native_func, NativeFunc(fset, 2, false));
        return call(_t(tp_property), _0, _1);
    }

    PyObject* VM::new_type_object(PyObject* mod, StrName name, Type base, bool subclass_enabled){
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

    Type VM::_new_type_object(StrName name, Type base) {
        PyObject* obj = new_type_object(nullptr, name, base, false);
        return PK_OBJ_GET(Type, obj);
    }

    PyObject* VM::_find_type_object(const Str& type){
        PyObject* obj = builtins->attr().try_get(type);
        if(obj == nullptr){
            for(auto& t: _all_types) if(t.name == type) return t.obj;
            throw std::runtime_error(fmt("type not found: ", type));
        }
        check_non_tagged_type(obj, tp_type);
        return obj;
    }


    Type VM::_type(const Str& type){
        PyObject* obj = _find_type_object(type);
        return PK_OBJ_GET(Type, obj);
    }

    PyTypeInfo* VM::_type_info(const Str& type){
        PyObject* obj = builtins->attr().try_get(type);
        if(obj == nullptr){
            for(auto& t: _all_types) if(t.name == type) return &t;
            FATAL_ERROR();
        }
        return &_all_types[PK_OBJ_GET(Type, obj)];
    }

    PyTypeInfo* VM::_type_info(Type type){
        return &_all_types[type];
    }

    const PyTypeInfo* VM::_inst_type_info(PyObject* obj){
        if(is_int(obj)) return &_all_types[tp_int];
        if(is_float(obj)) return &_all_types[tp_float];
        return &_all_types[obj->type];
    }

    bool VM::py_equals(PyObject* lhs, PyObject* rhs){
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

    PyObject* VM::py_import(StrName name, bool relative){
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

f64 VM::num_to_float(PyObject* obj){
    if(is_float(obj)){
        return _CAST(f64, obj);
    } else if (is_int(obj)){
        return (f64)_CAST(i64, obj);
    }
    TypeError("expected 'int' or 'float', got " + OBJ_NAME(_t(obj)).escape());
    return 0;
}


bool VM::py_bool(PyObject* obj){
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
    const PyTypeInfo* ti = _inst_type_info(obj);
    if(ti->m__hash__) return ti->m__hash__(this, obj);
    PyObject* ret = call_method(obj, __hash__);
    return CAST(i64, ret);
}

PyObject* VM::format(Str spec, PyObject* obj){
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
            if(dot == 0){
                width = -1;
            }else{
                width = Number::stoi(spec.substr(0, dot).str());
            }
            precision = Number::stoi(spec.substr(dot+1).str());
        }else{
            width = Number::stoi(spec.str());
            precision = -1;
        }
    }catch(...){
        ValueError("invalid format specifer");
        UNREACHABLE();
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
    if(width != -1 && width > ret.length()){
        int pad = width - ret.length();
        std::string padding(pad, pad_c);
        if(align == '>') ret = padding.c_str() + ret;
        else ret = ret + padding.c_str();
    }
    return VAR(ret);
}

PyObject* VM::new_module(StrName name) {
    PyObject* obj = heap._new<DummyModule>(tp_module, DummyModule());
    obj->attr().set("__name__", VAR(name.sv()));
    // we do not allow override in order to avoid memory leak
    // it is because Module objects are not garbage collected
    if(_modules.contains(name)) throw std::runtime_error("module already exists");
    _modules.set(name, obj);
    return obj;
}

static std::string _opcode_argstr(VM* vm, Bytecode byte, const CodeObject* co){
    std::string argStr = byte.arg == -1 ? "" : std::to_string(byte.arg);
    switch(byte.op){
        case OP_LOAD_CONST: case OP_FORMAT_STRING:
            if(vm != nullptr){
                argStr += fmt(" (", CAST(Str, vm->py_repr(co->consts[byte.arg])), ")");
            }
            break;
        case OP_LOAD_NAME: case OP_LOAD_GLOBAL: case OP_LOAD_NONLOCAL: case OP_STORE_GLOBAL:
        case OP_LOAD_ATTR: case OP_LOAD_METHOD: case OP_STORE_ATTR: case OP_DELETE_ATTR:
        case OP_IMPORT_NAME: case OP_BEGIN_CLASS: case OP_RAISE:
        case OP_DELETE_GLOBAL: case OP_INC_GLOBAL: case OP_DEC_GLOBAL: case OP_STORE_CLASS_ATTR:
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

Str VM::disassemble(CodeObject_ co){
    auto pad = [](const Str& s, const int n){
        if(s.length() >= n) return s.substr(0, n);
        return s + std::string(n - s.length(), ' ');
    };

    std::vector<int> jumpTargets;
    for(auto byte : co->codes){
        if(byte.op == OP_JUMP_ABSOLUTE || byte.op == OP_POP_JUMP_IF_FALSE || byte.op == OP_SHORTCUT_IF_FALSE_OR_POP){
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

#if PK_DEBUG_CEVAL_STEP
void VM::_log_s_data(const char* title) {
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
            Type t = PK_OBJ_GET(Type, obj);
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

void VM::init_builtin_types(){
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
    this->NotImplemented = heap._new<Dummy>(_new_type_object("NotImplementedType"), {});
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
    builtins->attr().set("NotImplemented", NotImplemented);
    builtins->attr().set("slice", _t(tp_slice));

    post_init();
    for(int i=0; i<_all_types.size(); i++){
        _all_types[i].obj->attr()._try_perfect_rehash();
    }
    for(auto [k, v]: _modules.items()) v->attr()._try_perfect_rehash();
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
        vm->TypeError(fmt(
            "expected ", decl_argc, " positional arguments, got ", args.size(),
            " (", co->name, ')'
        ));
    }

    int i = 0;
    // prepare args
    for(int index: decl->args) buffer[index] = args[i++];
    // set extra varnames to nullptr
    for(int j=i; j<co_nlocals; j++) buffer[j] = PY_NULL;
    // prepare kwdefaults
    for(auto& kv: decl->kwargs) buffer[kv.key] = kv.value;
    
    // handle *args
    if(decl->starred_arg != -1){
        ArgsView vargs(args.begin() + i, args.end());
        buffer[decl->starred_arg] = VAR(vargs.to_tuple());
        i += vargs.size();
    }else{
        // kwdefaults override
        for(auto& kv: decl->kwargs){
            if(i >= args.size()) break;
            buffer[kv.key] = args[i++];
        }
        if(i < args.size()) TypeError(fmt("too many arguments", " (", decl->code->name, ')'));
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
        int index = co->varnames_inv.try_get(key);
        if(index < 0){
            if(vkwargs == nullptr){
                TypeError(fmt(key.escape(), " is an invalid keyword argument for ", co->name, "()"));
            }else{
                Dict& dict = _CAST(Dict&, vkwargs);
                dict.set(VAR(key.sv()), kwargs[j+1]);
            }
        }else{
            buffer[index] = kwargs[j+1];
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
        if(method_call) FATAL_ERROR();
        auto& bm = CAST(BoundMethod&, callable);
        callable = bm.func;      // get unbound method
        p1[-(ARGC + 2)] = bm.func;
        p1[-(ARGC + 1)] = bm.self;
        method_call = true;
        // [unbound, self, args..., kwargs...]
    }

    ArgsView args(p1 - ARGC - int(method_call), p1);
    ArgsView kwargs(p1, s_data._sp);

    static THREAD_LOCAL PyObject* buffer[PK_MAX_CO_VARNAMES];

    if(is_non_tagged_type(callable, tp_native_func)){
        const auto& f = PK_OBJ_GET(NativeFunc, callable);
        PyObject* ret;
        if(f.decl != nullptr){
            int co_nlocals = f.decl->code->varnames.size();
            _prepare_py_call(buffer, args, kwargs, f.decl);
            // copy buffer back to stack
            s_data.reset(args.begin());
            for(int j=0; j<co_nlocals; j++) PUSH(buffer[j]);
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

        _prepare_py_call(buffer, args, kwargs, decl);
        
        if(co->is_generator){
            s_data.reset(p0);
            return _py_generator(
                Frame(&s_data, nullptr, co, fn._module, callable),
                ArgsView(buffer, buffer + co_nlocals)
            );
        }

        // copy buffer back to stack
        s_data.reset(args.begin());
        for(int j=0; j<co_nlocals; j++) PUSH(buffer[j]);
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
#if PK_DEBUG_EXTRA_CHECK
        PK_ASSERT(new_f != nullptr);
#endif
        if(new_f == cached_object__new__) {
            // fast path for object.__new__
            Type t = PK_OBJ_GET(Type, callable);
            obj= vm->heap.gcnew<DummyInstance>(t, {});
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
PyObject* VM::getattr(PyObject* obj, StrName name, bool throw_err){
    PyObject* objtype;
    // handle super() proxy
    if(is_non_tagged_type(obj, tp_super)){
        const Super& super = PK_OBJ_GET(Super, obj);
        obj = super.first;
        objtype = _t(super.second);
    }else{
        objtype = _t(obj);
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
PyObject* VM::get_unbound_method(PyObject* obj, StrName name, PyObject** self, bool throw_err, bool fallback){
    *self = PY_NULL;
    PyObject* objtype;
    // handle super() proxy
    if(is_non_tagged_type(obj, tp_super)){
        const Super& super = PK_OBJ_GET(Super, obj);
        obj = super.first;
        objtype = _t(super.second);
    }else{
        objtype = _t(obj);
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

void VM::setattr(PyObject* obj, StrName name, PyObject* value){
    PyObject* objtype;
    // handle super() proxy
    if(is_non_tagged_type(obj, tp_super)){
        Super& super = PK_OBJ_GET(Super, obj);
        obj = super.first;
        objtype = _t(super.second);
    }else{
        objtype = _t(obj);
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

PyObject* VM::bind(PyObject* obj, const char* sig, NativeFuncC fn){
    return bind(obj, sig, nullptr, fn);
}

PyObject* VM::bind(PyObject* obj, const char* sig, const char* docstring, NativeFuncC fn){
    CodeObject_ co;
    try{
        // fn(a, b, *c, d=1) -> None
        co = compile("def " + Str(sig) + " : pass", "<bind>", EXEC_MODE);
    }catch(Exception& e){
        PK_UNUSED(e);
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
    obj->attr().set(decl->code->name, f_obj);
    return f_obj;
}

void VM::_error(Exception e){
    if(callstack.empty()){
        e.is_re = false;
        throw e;
    }
    PUSH(VAR(e));
    _raise();
}

void ManagedHeap::mark() {
    for(PyObject* obj: _no_gc) PK_OBJ_MARK(obj);
    for(auto& frame : vm->callstack.data()) frame._gc_mark();
    for(PyObject* obj: vm->s_data) PK_OBJ_MARK(obj);
    if(_gc_marker_ex) _gc_marker_ex(vm);
    if(vm->_last_exception) PK_OBJ_MARK(vm->_last_exception);
}

Str obj_type_name(VM *vm, Type type){
    return vm->_all_types[type].name;
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

void Dict::_probe(PyObject *key, bool &ok, int &i) const{
    ok = false;
    i = vm->py_hash(key) & _mask;
    while(_items[i].first != nullptr) {
        if(vm->py_equals(_items[i].first, key)) { ok = true; break; }
        // https://github.com/python/cpython/blob/3.8/Objects/dictobject.c#L166
        i = ((5*i) + 1) & _mask;
    }
}

void CodeObjectSerializer::write_object(VM *vm, PyObject *obj){
    if(is_int(obj)) write_int(_CAST(i64, obj));
    else if(is_float(obj)) write_float(_CAST(f64, obj));
    else if(is_type(obj, vm->tp_str)) write_str(_CAST(Str&, obj));
    else if(is_type(obj, vm->tp_bool)) write_bool(_CAST(bool, obj));
    else if(obj == vm->None) write_none();
    else if(obj == vm->Ellipsis) write_ellipsis();
    else{
        throw std::runtime_error(fmt(OBJ_NAME(vm->_t(obj)).escape(), " is not serializable"));
    }
}

void NativeFunc::check_size(VM* vm, ArgsView args) const{
    if(args.size() != argc && argc != -1) {
        vm->TypeError(fmt("expected ", argc, " arguments, got ", args.size()));
    }
}

PyObject* NativeFunc::call(VM *vm, ArgsView args) const {
    return f(vm, args);
}

}   // namespace pkpy


// dummy header for ceval.cpp
namespace pkpy{

PyObject* VM::_run_top_frame(){
    DEF_SNAME(add);
    DEF_SNAME(set);
    DEF_SNAME(__enter__);
    DEF_SNAME(__exit__);

    FrameId frame = top_frame();
    const int base_id = frame.index;
    bool need_raise = false;

    // shared registers
    PyObject *_0, *_1, *_2;
    const PyTypeInfo* _ti;
    StrName _name;

    while(true){
#if PK_DEBUG_EXTRA_CHECK
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

#if PK_ENABLE_CEVAL_CALLBACK
#define CEVAL_STEP() byte = frame->next_bytecode(); if(_ceval_on_step) _ceval_on_step(this, frame.get(), byte)
#else
#define CEVAL_STEP() byte = frame->next_bytecode()
#endif

#define DISPATCH_OP_CALL() { frame = top_frame(); goto __NEXT_FRAME; }
__NEXT_FRAME:
    Bytecode CEVAL_STEP();
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
OPCODE(BUILD_LONG)
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
OPCODE(FORMAT_STRING)
/**************************/
OPCODE(INC_FAST)
OPCODE(DEC_FAST)
OPCODE(INC_GLOBAL)
OPCODE(DEC_GLOBAL)
#endif
#undef OPCODE
};

#define DISPATCH() { CEVAL_STEP(); goto *OP_LABELS[byte.op];}
#define TARGET(op) CASE_OP_##op:
goto *OP_LABELS[byte.op];

#else
#define TARGET(op) case OP_##op:
#define DISPATCH() { CEVAL_STEP(); goto __NEXT_STEP;}

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
        PyObject* obj;
        if(decl->nested){
            NameDict_ captured = frame->_locals.to_namedict();
            obj = VAR(Function({decl, frame->_module, captured}));
            captured->set(decl->code->name, obj);
        }else{
            obj = VAR(Function({decl, frame->_module}));
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
    TARGET(LOAD_NAME) {
        heap._auto_collect();
        _name = StrName(byte.arg);
        PyObject** slot = frame->_locals.try_get_name(_name);
        if(slot != nullptr) {
            if(*slot == PY_NULL) vm->UnboundLocalError(_name);
            PUSH(*slot);
            DISPATCH();
        }
        _0 = frame->f_closure_try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = frame->f_globals().try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = vm->builtins->attr().try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        vm->NameError(_name);
    } DISPATCH();
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
    TARGET(STORE_NAME){
        _name = StrName(byte.arg);
        _0 = POPX();
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
            PyObject** slot = frame->_locals.try_get_name(_name);
            if(slot == nullptr) vm->UnboundLocalError(_name);
            *slot = PY_NULL;
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
    TARGET(BUILD_LONG) {
        const static StrName m_long("long");
        _0 = builtins->attr().try_get(m_long);
        if(_0 == nullptr) AttributeError(builtins, m_long);
        TOP() = call(_0, TOP());
    } DISPATCH();
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
            PyObject* self;                                         \
            _2 = get_unbound_method(_0, func, &self, false);        \
            if(_2 != nullptr) TOP() = call_method(self, _2, _1);    \
            else TOP() = NotImplemented;                            \
        }

#define BINARY_OP_RSPECIAL(op, func)                                \
        if(TOP() == NotImplemented){                                \
            PyObject* self;                                         \
            _2 = get_unbound_method(_1, func, &self, false);        \
            if(_2 != nullptr) TOP() = call_method(self, _2, _0);    \
            else BinaryOptError(op);                                \
            if(TOP() == NotImplemented) BinaryOptError(op);         \
        }

    TARGET(BINARY_TRUEDIV)
        BINARY_OP_SPECIAL(__truediv__);
        if(TOP() == NotImplemented) BinaryOptError("/");
        DISPATCH();
    TARGET(BINARY_POW)
        BINARY_OP_SPECIAL(__pow__);
        if(TOP() == NotImplemented) BinaryOptError("**");
        DISPATCH();
    TARGET(BINARY_ADD)
        PREDICT_INT_OP(+);
        BINARY_OP_SPECIAL(__add__);
        BINARY_OP_RSPECIAL("+", __radd__);
        DISPATCH()
    TARGET(BINARY_SUB)
        PREDICT_INT_OP(-);
        BINARY_OP_SPECIAL(__sub__);
        BINARY_OP_RSPECIAL("-", __rsub__);
        DISPATCH()
    TARGET(BINARY_MUL)
        BINARY_OP_SPECIAL(__mul__);
        BINARY_OP_RSPECIAL("*", __rmul__);
        DISPATCH()
    TARGET(BINARY_FLOORDIV)
        PREDICT_INT_OP(/);
        BINARY_OP_SPECIAL(__floordiv__);
        if(TOP() == NotImplemented) BinaryOptError("//");
        DISPATCH()
    TARGET(BINARY_MOD)
        PREDICT_INT_OP(%);
        BINARY_OP_SPECIAL(__mod__);
        if(TOP() == NotImplemented) BinaryOptError("%");
        DISPATCH()
    TARGET(COMPARE_LT)
        BINARY_OP_SPECIAL(__lt__);
        BINARY_OP_RSPECIAL("<", __gt__);
        DISPATCH()
    TARGET(COMPARE_LE)
        BINARY_OP_SPECIAL(__le__);
        BINARY_OP_RSPECIAL("<=", __ge__);
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
        BINARY_OP_RSPECIAL(">", __lt__);
        DISPATCH()
    TARGET(COMPARE_GE)
        BINARY_OP_SPECIAL(__ge__);
        BINARY_OP_RSPECIAL(">=", __le__);
        DISPATCH()
    TARGET(BITWISE_LSHIFT)
        PREDICT_INT_OP(<<);
        BINARY_OP_SPECIAL(__lshift__);
        if(TOP() == NotImplemented) BinaryOptError("<<");
        DISPATCH()
    TARGET(BITWISE_RSHIFT)
        PREDICT_INT_OP(>>);
        BINARY_OP_SPECIAL(__rshift__);
        if(TOP() == NotImplemented) BinaryOptError(">>");
        DISPATCH()
    TARGET(BITWISE_AND)
        PREDICT_INT_OP(&);
        BINARY_OP_SPECIAL(__and__);
        if(TOP() == NotImplemented) BinaryOptError("&");
        DISPATCH()
    TARGET(BITWISE_OR)
        PREDICT_INT_OP(|);
        BINARY_OP_SPECIAL(__or__);
        if(TOP() == NotImplemented) BinaryOptError("|");
        DISPATCH()
    TARGET(BITWISE_XOR)
        PREDICT_INT_OP(^);
        BINARY_OP_SPECIAL(__xor__);
        if(TOP() == NotImplemented) BinaryOptError("^");
        DISPATCH()
    TARGET(BINARY_MATMUL)
        BINARY_OP_SPECIAL(__matmul__);
        if(TOP() == NotImplemented) BinaryOptError("@");
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
        TOP() = VAR(static_cast<bool>((int)CAST(bool, _0) ^ byte.arg));
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
        _1 = new_type_object(frame->_module, _name, PK_OBJ_GET(Type, _0));
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
#if PK_DEBUG_EXTRA_CHECK
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
#undef CEVAL_STEP
/**********************************************************************/
            UNREACHABLE();
        }catch(HandledException& e){
            PK_UNUSED(e);
            continue;
        }catch(UnhandledException& e){
            PK_UNUSED(e);
            PyObject* obj = POPX();
            Exception& _e = CAST(Exception&, obj);
            _e.st_push(frame->snapshot());
            _pop_frame();
            if(callstack.empty()){
#if PK_DEBUG_FULL_EXCEPTION
                std::cerr << _e.summary() << std::endl;
#endif
                throw _e;
            }
            frame = top_frame();
            PUSH(obj);
            if(frame.index < base_id) throw ToBeRaisedException();
            need_raise = true;
        }catch(ToBeRaisedException& e){
            PK_UNUSED(e);
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
    virtual bool is_tuple() const { return false; }
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
    CodeObject_ co;
    // some bugs on MSVC (error C2280) when using std::vector<Expr_>
    // so we use stack_no_copy instead
    stack_no_copy<Expr_> s_expr;
    int level;
    std::set<Str> global_names;
    CodeEmitContext(VM* vm, CodeObject_ co, int level): vm(vm), co(co), level(level) {}

    int curr_block_i = 0;
    bool is_compiling_class = false;
    int for_loop_depth = 0;

    bool is_curr_block_loop() const;
    void enter_block(CodeBlockType type);
    void exit_block();
    void emit_expr();   // clear the expression stack and generate bytecode
    std::string _log_s_expr();
    int emit(Opcode opcode, int arg, int line);
    void patch_jump(int index);
    bool add_label(StrName name);
    int add_varname(StrName name);
    int add_const(PyObject* v);
    int add_func_decl(FuncDecl_ decl);
};

struct NameExpr: Expr{
    StrName name;
    NameScope scope;
    NameExpr(StrName name, NameScope scope): name(name), scope(scope) {}

    std::string str() const override { return fmt("Name(", name.escape(), ")"); }

    void emit(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
};

struct StarredExpr: Expr{
    int level;
    Expr_ child;
    StarredExpr(int level, Expr_&& child): level(level), child(std::move(child)) {}
    std::string str() const override { return fmt("Starred(level=", level, ")"); }
    int star_level() const override { return level; }
    void emit(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
};

struct NotExpr: Expr{
    Expr_ child;
    NotExpr(Expr_&& child): child(std::move(child)) {}
    std::string str() const override { return "Not()"; }

    void emit(CodeEmitContext* ctx) override;
};

struct AndExpr: Expr{
    Expr_ lhs;
    Expr_ rhs;
    std::string str() const override { return "And()"; }
    void emit(CodeEmitContext* ctx) override;
};

struct OrExpr: Expr{
    Expr_ lhs;
    Expr_ rhs;
    std::string str() const override { return "Or()"; }
    void emit(CodeEmitContext* ctx) override;
};

// [None, True, False, ...]
struct Literal0Expr: Expr{
    TokenIndex token;
    Literal0Expr(TokenIndex token): token(token) {}
    std::string str() const override { return TK_STR(token); }
    bool is_json_object() const override { return true; }

    void emit(CodeEmitContext* ctx) override;
};

struct LongExpr: Expr{
    Str s;
    LongExpr(const Str& s): s(s) {}
    std::string str() const override { return s.str(); }
    void emit(CodeEmitContext* ctx) override;
};

// @num, @str which needs to invoke OP_LOAD_CONST
struct LiteralExpr: Expr{
    TokenValue value;
    LiteralExpr(TokenValue value): value(value) {}
    std::string str() const override;
    void emit(CodeEmitContext* ctx) override;
    bool is_literal() const override { return true; }
    bool is_json_object() const override { return true; }
};

struct NegatedExpr: Expr{
    Expr_ child;
    NegatedExpr(Expr_&& child): child(std::move(child)) {}
    std::string str() const override { return "Negated()"; }

    void emit(CodeEmitContext* ctx) override;
    bool is_json_object() const override { return child->is_literal(); }
};

struct SliceExpr: Expr{
    Expr_ start;
    Expr_ stop;
    Expr_ step;
    std::string str() const override { return "Slice()"; }
    void emit(CodeEmitContext* ctx) override;
};

struct DictItemExpr: Expr{
    Expr_ key;      // maybe nullptr if it is **kwargs
    Expr_ value;
    std::string str() const override { return "DictItem()"; }
    int star_level() const override { return value->star_level(); }
    void emit(CodeEmitContext* ctx) override;
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

    void emit(CodeEmitContext* ctx) override;
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

    void _load_simple_expr(CodeEmitContext* ctx, Str expr);
    void emit(CodeEmitContext* ctx) override;
};

struct SubscrExpr: Expr{
    Expr_ a;
    Expr_ b;
    std::string str() const override { return "Subscr()"; }

    void emit(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
};

struct AttribExpr: Expr{
    Expr_ a;
    Str b;
    AttribExpr(Expr_ a, const Str& b): a(std::move(a)), b(b) {}
    AttribExpr(Expr_ a, Str&& b): a(std::move(a)), b(std::move(b)) {}
    std::string str() const override { return "Attrib()"; }

    void emit(CodeEmitContext* ctx) override;
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
    std::string str() const override { return "Call()"; }
    void emit(CodeEmitContext* ctx) override;
};

struct GroupedExpr: Expr{
    Expr_ a;
    std::string str() const override { return "Grouped()"; }

    GroupedExpr(Expr_&& a): a(std::move(a)) {}

    void emit(CodeEmitContext* ctx) override{
        a->emit(ctx);
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
    std::string str() const override { return TK_STR(op); }

    bool is_compare() const override;
    void _emit_compare(CodeEmitContext* ctx, std::vector<int>& jmps);
    void emit(CodeEmitContext* ctx) override;
};


struct TernaryExpr: Expr{
    Expr_ cond;
    Expr_ true_expr;
    Expr_ false_expr;
    std::string str() const override { return "Ternary()"; }
    void emit(CodeEmitContext* ctx) override;
};


} // namespace pkpy
namespace pkpy{

    bool CodeEmitContext::is_curr_block_loop() const {
        return co->blocks[curr_block_i].type == FOR_LOOP || co->blocks[curr_block_i].type == WHILE_LOOP;
    }

    void CodeEmitContext::enter_block(CodeBlockType type){
        if(type == FOR_LOOP) for_loop_depth++;
        co->blocks.push_back(CodeBlock(
            type, curr_block_i, for_loop_depth, (int)co->codes.size()
        ));
        curr_block_i = co->blocks.size()-1;
    }

    void CodeEmitContext::exit_block(){
        auto curr_type = co->blocks[curr_block_i].type;
        if(curr_type == FOR_LOOP) for_loop_depth--;
        co->blocks[curr_block_i].end = co->codes.size();
        curr_block_i = co->blocks[curr_block_i].parent;
        if(curr_block_i < 0) FATAL_ERROR();

        if(curr_type == FOR_LOOP){
            // add a no op here to make block check work
            emit(OP_NO_OP, BC_NOARG, BC_KEEPLINE);
        }
    }

    // clear the expression stack and generate bytecode
    void CodeEmitContext::emit_expr(){
        if(s_expr.size() != 1){
            throw std::runtime_error("s_expr.size() != 1\n" + _log_s_expr());
        }
        Expr_ expr = s_expr.popx();
        expr->emit(this);
    }

    std::string CodeEmitContext::_log_s_expr(){
        std::stringstream ss;
        for(auto& e: s_expr.data()) ss << e->str() << " ";
        return ss.str();
    }

    int CodeEmitContext::emit(Opcode opcode, int arg, int line) {
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
        int index = co->varnames_inv.try_get(name);
        if(index >= 0) return index;
        co->varnames.push_back(name);
        index = co->varnames.size() - 1;
        co->varnames_inv.set(name, index);
        return index;
    }

    int CodeEmitContext::add_const(PyObject* v){
        // simple deduplication, only works for int/float
        for(int i=0; i<co->consts.size(); i++){
            if(co->consts[i] == v) return i;
        }
        co->consts.push_back(v);
        return co->consts.size() - 1;
    }

    int CodeEmitContext::add_func_decl(FuncDecl_ decl){
        co->func_decls.push_back(decl);
        return co->func_decls.size() - 1;
    }


    void NameExpr::emit(CodeEmitContext* ctx) {
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

    bool NameExpr::emit_del(CodeEmitContext* ctx) {
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

    bool NameExpr::emit_store(CodeEmitContext* ctx) {
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


    void StarredExpr::emit(CodeEmitContext* ctx) {
        child->emit(ctx);
        ctx->emit(OP_UNARY_STAR, level, line);
    }

    bool StarredExpr::emit_store(CodeEmitContext* ctx) {
        if(level != 1) return false;
        // simply proxy to child
        return child->emit_store(ctx);
    }

    void NotExpr::emit(CodeEmitContext* ctx) {
        child->emit(ctx);
        ctx->emit(OP_UNARY_NOT, BC_NOARG, line);
    }

    void AndExpr::emit(CodeEmitContext* ctx) {
        lhs->emit(ctx);
        int patch = ctx->emit(OP_JUMP_IF_FALSE_OR_POP, BC_NOARG, line);
        rhs->emit(ctx);
        ctx->patch_jump(patch);
    }

    void OrExpr::emit(CodeEmitContext* ctx) {
        lhs->emit(ctx);
        int patch = ctx->emit(OP_JUMP_IF_TRUE_OR_POP, BC_NOARG, line);
        rhs->emit(ctx);
        ctx->patch_jump(patch);
    }

    void Literal0Expr::emit(CodeEmitContext* ctx){
        switch (token) {
            case TK("None"):    ctx->emit(OP_LOAD_NONE, BC_NOARG, line); break;
            case TK("True"):    ctx->emit(OP_LOAD_TRUE, BC_NOARG, line); break;
            case TK("False"):   ctx->emit(OP_LOAD_FALSE, BC_NOARG, line); break;
            case TK("..."):     ctx->emit(OP_LOAD_ELLIPSIS, BC_NOARG, line); break;
            default: FATAL_ERROR();
        }
    }

    void LongExpr::emit(CodeEmitContext* ctx) {
        VM* vm = ctx->vm;
        ctx->emit(OP_LOAD_CONST, ctx->add_const(VAR(s)), line);
        ctx->emit(OP_BUILD_LONG, BC_NOARG, line);
    }

    std::string LiteralExpr::str() const{
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

    void LiteralExpr::emit(CodeEmitContext* ctx) {
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

    void NegatedExpr::emit(CodeEmitContext* ctx){
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


    void SliceExpr::emit(CodeEmitContext* ctx){
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

    void DictItemExpr::emit(CodeEmitContext* ctx) {
        if(is_starred()){
            PK_ASSERT(key == nullptr);
            value->emit(ctx);
        }else{
            value->emit(ctx);
            key->emit(ctx);     // reverse order
            ctx->emit(OP_BUILD_TUPLE, 2, line);
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

    bool TupleExpr::emit_del(CodeEmitContext* ctx){
        for(auto& e: items){
            bool ok = e->emit_del(ctx);
            if(!ok) return false;
        }
        return true;
    }

    void CompExpr::emit(CodeEmitContext* ctx){
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


    void FStringExpr::_load_simple_expr(CodeEmitContext* ctx, Str expr){
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

    void FStringExpr::emit(CodeEmitContext* ctx){
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


    void SubscrExpr::emit(CodeEmitContext* ctx){
        a->emit(ctx);
        b->emit(ctx);
        ctx->emit(OP_LOAD_SUBSCR, BC_NOARG, line);
    }

    bool SubscrExpr::emit_del(CodeEmitContext* ctx){
        a->emit(ctx);
        b->emit(ctx);
        ctx->emit(OP_DELETE_SUBSCR, BC_NOARG, line);
        return true;
    }

    bool SubscrExpr::emit_store(CodeEmitContext* ctx){
        a->emit(ctx);
        b->emit(ctx);
        ctx->emit(OP_STORE_SUBSCR, BC_NOARG, line);
        return true;
    }

    void AttribExpr::emit(CodeEmitContext* ctx){
        a->emit(ctx);
        int index = StrName(b).index;
        ctx->emit(OP_LOAD_ATTR, index, line);
    }

    bool AttribExpr::emit_del(CodeEmitContext* ctx) {
        a->emit(ctx);
        int index = StrName(b).index;
        ctx->emit(OP_DELETE_ATTR, index, line);
        return true;
    }

    bool AttribExpr::emit_store(CodeEmitContext* ctx){
        a->emit(ctx);
        int index = StrName(b).index;
        ctx->emit(OP_STORE_ATTR, index, line);
        return true;
    }

    void AttribExpr::emit_method(CodeEmitContext* ctx) {
        a->emit(ctx);
        int index = StrName(b).index;
        ctx->emit(OP_LOAD_METHOD, index, line);
    }

    void CallExpr::emit(CodeEmitContext* ctx) {
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

    void BinaryExpr::emit(CodeEmitContext* ctx) {
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

    void TernaryExpr::emit(CodeEmitContext* ctx){
        cond->emit(ctx);
        int patch = ctx->emit(OP_POP_JUMP_IF_FALSE, BC_NOARG, cond->line);
        true_expr->emit(ctx);
        int patch_2 = ctx->emit(OP_JUMP_ABSOLUTE, BC_NOARG, true_expr->line);
        ctx->patch_jump(patch);
        false_expr->emit(ctx);
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
    void EXPR(bool push_stack=true);
    void EXPR_TUPLE(bool push_stack=true);
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
    void exprFString();
    void exprLambda();
    void exprTuple();
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
    void exprSubscr();
    void exprLiteral0();

    void compile_block_body();
    Str _compile_import();
    void compile_normal_import();
    void compile_from_import();
    bool is_expression();
    void parse_expression(int precedence, bool push_stack=true);
    void compile_if_stmt();
    void compile_while_loop();
    void compile_for_loop();
    void compile_try_except();
    void compile_decorated();

    bool try_compile_assignment();
    void compile_stmt();
    void consume_type_hints();
    void compile_class();
    void _compile_f_args(FuncDecl_ decl, bool enable_type_hints);
    void compile_function(const std::vector<Expr_>& decorators={});

    PyObject* to_object(const TokenValue& value);
    PyObject* read_literal();

    void SyntaxError(Str msg){ lexer->throw_err("SyntaxError", msg, err().line, err().start); }
    void SyntaxError(){ lexer->throw_err("SyntaxError", "invalid syntax", err().line, err().start); }
    void IndentationError(Str msg){ lexer->throw_err("IndentationError", msg, err().line, err().start); }

public:
    Compiler(VM* vm, const Str& source, const Str& filename, CompileMode mode, bool unknown_global_scope=false);
    CodeObject_ compile();

    Compiler(const Compiler&) = delete;
    Compiler& operator=(const Compiler&) = delete;
};

#undef BC_NOARG
#undef BC_KEEPLINE

} // namespace pkpy
namespace pkpy{

    NameScope Compiler::name_scope() const {
        auto s = contexts.size()>1 ? NAME_LOCAL : NAME_GLOBAL;
        if(unknown_global_scope && s == NAME_GLOBAL) s = NAME_GLOBAL_UNKNOWN;
        return s;
    }

    CodeObject_ Compiler::push_global_context(){
        CodeObject_ co = make_sp<CodeObject>(lexer->src, lexer->src->filename);
        contexts.push(CodeEmitContext(vm, co, contexts.size()));
        return co;
    }

    FuncDecl_ Compiler::push_f_context(Str name){
        FuncDecl_ decl = make_sp<FuncDecl>();
        decl->code = make_sp<CodeObject>(lexer->src, name);
        decl->nested = name_scope() == NAME_LOCAL;
        contexts.push(CodeEmitContext(vm, decl->code, contexts.size()));
        return decl;
    }

    void Compiler::pop_context(){
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

    void Compiler::init_pratt_rules(){
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
        rules[TK("@long")] =    { METHOD(exprLong),      NO_INFIX };
#undef METHOD
#undef NO_INFIX
    }

    bool Compiler::match(TokenIndex expected) {
        if (curr().type != expected) return false;
        advance();
        return true;
    }

    void Compiler::consume(TokenIndex expected) {
        if (!match(expected)){
            SyntaxError(
                fmt("expected '", TK_STR(expected), "', got '", TK_STR(curr().type), "'")
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

    void Compiler::EXPR(bool push_stack) {
        parse_expression(PREC_TUPLE+1, push_stack);
    }

    void Compiler::EXPR_TUPLE(bool push_stack) {
        parse_expression(PREC_TUPLE, push_stack);
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
        // https://github.com/blueloveTH/pocketpy/issues/37
        parse_expression(PREC_LAMBDA + 1, false);
        ctx()->emit(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
        pop_context();
        ctx()->s_expr.push(std::move(e));
    }

    void Compiler::exprTuple(){
        std::vector<Expr_> items;
        items.push_back(ctx()->s_expr.popx());
        do {
            if(curr().brackets_level) match_newlines_repl();
            if(!is_expression()) break;
            EXPR();
            items.push_back(ctx()->s_expr.popx());
            if(curr().brackets_level) match_newlines_repl();
        } while(match(TK(",")));
        ctx()->s_expr.push(make_expr<TupleExpr>(
            std::move(items)
        ));
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
            case TK("*"):
                ctx()->s_expr.push(make_expr<StarredExpr>(1, ctx()->s_expr.popx()));
                break;
            case TK("**"):
                ctx()->s_expr.push(make_expr<StarredExpr>(2, ctx()->s_expr.popx()));
                break;
            default: FATAL_ERROR();
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
            make_expr<AttribExpr>(ctx()->s_expr.popx(), prev().str())
        );
    }
    
    void Compiler::exprSubscr() {
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

    void Compiler::exprLiteral0() {
        ctx()->s_expr.push(make_expr<Literal0Expr>(prev().type));
    }

    void Compiler::compile_block_body() {
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

    Str Compiler::_compile_import() {
        if(name_scope() != NAME_GLOBAL) SyntaxError("import statement should be used in global scope");
        Opcode op = OP_IMPORT_NAME;
        if(match(TK("."))) op = OP_IMPORT_NAME_REL;
        consume(TK("@id"));
        Str name = prev().str();
        ctx()->emit(op, StrName(name).index, prev().line);
        return name;
    }

    // import a as b
    void Compiler::compile_normal_import() {
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
    void Compiler::compile_from_import() {
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

    bool Compiler::is_expression(){
        PrattCallback prefix = rules[curr().type].prefix;
        return prefix != nullptr;
    }

    void Compiler::parse_expression(int precedence, bool push_stack) {
        PrattCallback prefix = rules[curr().type].prefix;
        if (prefix == nullptr) SyntaxError(Str("expected an expression, got ") + TK_STR(curr().type));
        advance();
        (this->*prefix)();
        while (rules[curr().type].precedence >= precedence) {
            TokenIndex op = curr().type;
            advance();
            PrattCallback infix = rules[op].infix;
            PK_ASSERT(infix != nullptr);
            (this->*infix)();
        }
        if(!push_stack) ctx()->emit_expr();
    }

    void Compiler::compile_if_stmt() {
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

    void Compiler::compile_while_loop() {
        ctx()->enter_block(WHILE_LOOP);
        EXPR(false);   // condition
        int patch = ctx()->emit(OP_POP_JUMP_IF_FALSE, BC_NOARG, prev().line);
        compile_block_body();
        ctx()->emit(OP_LOOP_CONTINUE, BC_NOARG, BC_KEEPLINE);
        ctx()->patch_jump(patch);
        ctx()->exit_block();
    }

    void Compiler::compile_for_loop() {
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

    void Compiler::compile_try_except() {
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

    void Compiler::compile_decorated(){
        std::vector<Expr_> decorators;
        do{
            EXPR();
            decorators.push_back(ctx()->s_expr.popx());
            if(!match_newlines_repl()) SyntaxError();
        }while(match(TK("@")));
        consume(TK("def"));
        compile_function(decorators);
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
                e->emit(ctx());
                bool ok = lhs_p->emit_store(ctx());
                if(!ok) SyntaxError();
            } return true;
            case TK("="): {
                int n = 0;
                while(match(TK("="))){
                    EXPR_TUPLE();
                    Expr* _tp = ctx()->s_expr.top().get();
                    if(ctx()->is_compiling_class && _tp->is_tuple()){
                        SyntaxError("can't use unpack tuple in class definition");
                    }
                    n += 1;
                }
                if(ctx()->is_compiling_class && n>1){
                    SyntaxError("can't assign to multiple targets in class definition");
                }
                // stack size is n+1
                Expr_ val = ctx()->s_expr.popx();
                val->emit(ctx());
                for(int j=1; j<n; j++) ctx()->emit(OP_DUP_TOP, BC_NOARG, BC_KEEPLINE);
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
                    ctx()->global_names.insert(prev().str());
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
                Expr_ e = make_expr<NameExpr>(prev().str(), name_scope());
                bool ok = e->emit_store(ctx());
                if(!ok) SyntaxError();
                e->emit(ctx());
                ctx()->emit(OP_WITH_ENTER, BC_NOARG, prev().line);
                compile_block_body();
                e->emit(ctx());
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

    void Compiler::consume_type_hints(){
        EXPR();
        ctx()->s_expr.pop();
    }

    void Compiler::compile_class(){
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
                        SyntaxError(Str("default argument must be a literal"));
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
        ctx()->emit(OP_LOAD_FUNCTION, ctx()->add_func_decl(decl), prev().line);

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
        if(obj == nullptr) FATAL_ERROR();
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
            default: break;
        }
        return nullptr;
    }

    Compiler::Compiler(VM* vm, const Str& source, const Str& filename, CompileMode mode, bool unknown_global_scope){
        this->vm = vm;
        this->used = false;
        this->unknown_global_scope = unknown_global_scope;
        this->lexer = std::make_unique<Lexer>(
            make_sp<SourceData>(source, filename, mode)
        );
        init_pratt_rules();
    }


    CodeObject_ Compiler::compile(){
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

}   // namespace pkpy


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

// generated on 2023-07-06 20:32:20
#include <map>
#include <string>

namespace pkpy{
    inline static std::map<std::string, const char*> kPythonLibs = {
        {"_set", "\x63\x6c\x61\x73\x73\x20\x73\x65\x74\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x20\x3d\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x20\x6f\x72\x20\x5b\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x20\x3d\x20\x7b\x7d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x74\x65\x6d\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x61\x64\x64\x28\x69\x74\x65\x6d\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x61\x64\x64\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x65\x6c\x65\x6d\x5d\x20\x3d\x20\x4e\x6f\x6e\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x64\x69\x73\x63\x61\x72\x64\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x64\x65\x6c\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x65\x6c\x65\x6d\x5d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x72\x65\x6d\x6f\x76\x65\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x64\x65\x6c\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x65\x6c\x65\x6d\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6c\x65\x61\x72\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x63\x6c\x65\x61\x72\x28\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x75\x70\x64\x61\x74\x65\x28\x73\x65\x6c\x66\x2c\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x2e\x5f\x61\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6f\x70\x79\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x74\x28\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x6b\x65\x79\x73\x28\x29\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x61\x6e\x64\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x74\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6f\x72\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x6c\x66\x2e\x63\x6f\x70\x79\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x74\x28\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x6e\x6f\x74\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x78\x6f\x72\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x73\x65\x74\x28\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x6e\x6f\x74\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20" "\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x6f\x74\x68\x65\x72\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x65\x6c\x65\x6d\x20\x6e\x6f\x74\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x64\x64\x28\x65\x6c\x65\x6d\x29\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x75\x6e\x69\x6f\x6e\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x7c\x20\x6f\x74\x68\x65\x72\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x6e\x74\x65\x72\x73\x65\x63\x74\x69\x6f\x6e\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x26\x20\x6f\x74\x68\x65\x72\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x64\x69\x66\x66\x65\x72\x65\x6e\x63\x65\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x2d\x20\x6f\x74\x68\x65\x72\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x73\x79\x6d\x6d\x65\x74\x72\x69\x63\x5f\x64\x69\x66\x66\x65\x72\x65\x6e\x63\x65\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x5e\x20\x6f\x74\x68\x65\x72\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x78\x6f\x72\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x73\x64\x69\x73\x6a\x6f\x69\x6e\x74\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x61\x6e\x64\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x73\x73\x75\x62\x73\x65\x74\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x73\x73\x75\x70\x65\x72\x73\x65\x74\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6f\x74\x68\x65\x72\x2e\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x29\x2e\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x29\x20\x3d\x3d\x20\x30\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x63\x6f\x6e\x74\x61\x69\x6e\x73\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x65\x6c\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x73\x65\x74\x28\x29\x27\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x7b\x27\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x72\x65\x70\x72\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x6b\x65\x79\x73\x28\x29\x5d\x29\x20\x2b\x20\x27\x7d\x27\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x74\x65\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x74\x65\x72\x28\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x6b\x65\x79\x73\x28\x29\x29" },
        {"_long", "\x66\x72\x6f\x6d\x20\x63\x20\x69\x6d\x70\x6f\x72\x74\x20\x73\x69\x7a\x65\x6f\x66\x0a\x0a\x23\x20\x68\x74\x74\x70\x73\x3a\x2f\x2f\x77\x77\x77\x2e\x63\x6e\x62\x6c\x6f\x67\x73\x2e\x63\x6f\x6d\x2f\x6c\x69\x75\x63\x68\x61\x6e\x67\x6c\x63\x2f\x70\x2f\x31\x34\x32\x30\x33\x37\x38\x33\x2e\x68\x74\x6d\x6c\x0a\x69\x66\x20\x73\x69\x7a\x65\x6f\x66\x28\x27\x76\x6f\x69\x64\x5f\x70\x27\x29\x20\x3d\x3d\x20\x34\x3a\x0a\x20\x20\x20\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x20\x3d\x20\x32\x38\x2f\x2f\x32\x20\x2d\x20\x31\x0a\x65\x6c\x69\x66\x20\x73\x69\x7a\x65\x6f\x66\x28\x27\x76\x6f\x69\x64\x5f\x70\x27\x29\x20\x3d\x3d\x20\x38\x3a\x0a\x20\x20\x20\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x20\x3d\x20\x36\x30\x2f\x2f\x32\x20\x2d\x20\x31\x0a\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x45\x72\x72\x6f\x72\x0a\x0a\x50\x79\x4c\x6f\x6e\x67\x5f\x42\x41\x53\x45\x20\x3d\x20\x32\x20\x2a\x2a\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x0a\x50\x79\x4c\x6f\x6e\x67\x5f\x4d\x41\x53\x4b\x20\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x42\x41\x53\x45\x20\x2d\x20\x31\x0a\x50\x79\x4c\x6f\x6e\x67\x5f\x44\x45\x43\x49\x4d\x41\x4c\x5f\x53\x48\x49\x46\x54\x20\x3d\x20\x34\x0a\x50\x79\x4c\x6f\x6e\x67\x5f\x44\x45\x43\x49\x4d\x41\x4c\x5f\x42\x41\x53\x45\x20\x3d\x20\x31\x30\x20\x2a\x2a\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x44\x45\x43\x49\x4d\x41\x4c\x5f\x53\x48\x49\x46\x54\x0a\x0a\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x66\x72\x6f\x6d\x69\x6e\x74\x28\x78\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x20\x6c\x69\x73\x74\x20\x6f\x66\x20\x64\x69\x67\x69\x74\x73\x20\x61\x6e\x64\x20\x73\x69\x67\x6e\x0a\x20\x20\x20\x20\x69\x66\x20\x78\x20\x3d\x3d\x20\x30\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x30\x5d\x2c\x20\x31\x0a\x20\x20\x20\x20\x73\x69\x67\x6e\x20\x3d\x20\x31\x20\x69\x66\x20\x78\x20\x3e\x20\x30\x20\x65\x6c\x73\x65\x20\x2d\x31\x0a\x20\x20\x20\x20\x69\x66\x20\x73\x69\x67\x6e\x20\x3c\x20\x30\x3a\x20\x78\x20\x3d\x20\x2d\x78\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x78\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x78\x20\x26\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x4d\x41\x53\x4b\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x78\x20\x3e\x3e\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x2c\x20\x73\x69\x67\x6e\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x63\x6d\x70\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x6c\x69\x73\x74\x29\x20\x2d\x3e\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x23\x20\x72\x65\x74\x75\x72\x6e\x20\x31\x20\x69\x66\x20\x61\x3e\x62\x2c\x20\x2d\x31\x20\x69\x66\x20\x61\x3c\x62\x2c\x20\x30\x20\x69\x66\x20\x61\x3d\x3d\x62\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x61\x29\x20\x3e\x20\x6c\x65\x6e\x28\x62\x29\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x31\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x61\x29\x20\x3c\x20\x6c\x65\x6e\x28\x62\x29\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x2d\x31\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x61\x29\x2d\x31\x2c\x20\x2d\x31\x2c\x20\x2d\x31\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x61\x5b\x69\x5d\x20\x3e\x20\x62\x5b\x69\x5d\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x61\x5b\x69\x5d\x20\x3c\x20\x62\x5b\x69\x5d\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x2d\x31\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x30\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x70\x61\x64\x5f\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x73\x69\x7a\x65\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x70\x61\x64\x20\x6c\x65\x61\x64\x69\x6e\x67\x20\x7a\x65\x72\x6f\x73\x20\x74\x6f\x20\x68\x61\x76\x65\x20\x60\x73\x69\x7a\x65\x60\x20\x64\x69\x67\x69\x74\x73\x0a\x20\x20\x20\x20\x64\x65\x6c\x74\x61\x20\x3d\x20\x73\x69\x7a\x65\x20\x2d\x20\x6c\x65\x6e\x28\x61\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x64\x65\x6c\x74\x61\x20\x3e\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x2e\x65\x78\x74\x65\x6e\x64\x28\x5b\x30\x5d\x20\x2a\x20\x64\x65\x6c\x74\x61\x29\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x75\x6e\x70\x61\x64\x5f\x28\x61\x3a\x20\x6c\x69\x73\x74\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x72\x65\x6d\x6f\x76\x65\x20\x6c\x65\x61\x64\x69\x6e\x67\x20\x7a\x65\x72\x6f\x73\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6c\x65\x6e\x28\x61\x29\x3e\x31\x20\x61\x6e\x64\x20\x61\x5b\x2d\x31\x5d\x3d\x3d\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x2e\x70\x6f\x70\x28\x29\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x61\x64\x64\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x6c\x69\x73\x74\x29\x20\x2d\x3e" "\x20\x6c\x69\x73\x74\x3a\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x30\x5d\x20\x2a\x20\x6d\x61\x78\x28\x6c\x65\x6e\x28\x61\x29\x2c\x20\x6c\x65\x6e\x28\x62\x29\x29\x0a\x20\x20\x20\x20\x75\x6c\x6f\x6e\x67\x5f\x70\x61\x64\x5f\x28\x61\x2c\x20\x6c\x65\x6e\x28\x72\x65\x73\x29\x29\x0a\x20\x20\x20\x20\x75\x6c\x6f\x6e\x67\x5f\x70\x61\x64\x5f\x28\x62\x2c\x20\x6c\x65\x6e\x28\x72\x65\x73\x29\x29\x0a\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x72\x65\x73\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x2b\x3d\x20\x61\x5b\x69\x5d\x20\x2b\x20\x62\x5b\x69\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x5b\x69\x5d\x20\x3d\x20\x63\x61\x72\x72\x79\x20\x26\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x4d\x41\x53\x4b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3e\x3e\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x0a\x20\x20\x20\x20\x69\x66\x20\x63\x61\x72\x72\x79\x20\x3e\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x63\x61\x72\x72\x79\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x69\x6e\x63\x5f\x28\x61\x3a\x20\x6c\x69\x73\x74\x29\x3a\x0a\x20\x20\x20\x20\x61\x5b\x30\x5d\x20\x2b\x3d\x20\x31\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x61\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x61\x5b\x69\x5d\x20\x3c\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x42\x41\x53\x45\x3a\x20\x62\x72\x65\x61\x6b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x5b\x69\x5d\x20\x2d\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x42\x41\x53\x45\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x2b\x31\x20\x3d\x3d\x20\x6c\x65\x6e\x28\x61\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x2e\x61\x70\x70\x65\x6e\x64\x28\x31\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x5b\x69\x2b\x31\x5d\x20\x2b\x3d\x20\x31\x0a\x20\x20\x20\x20\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x73\x75\x62\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x6c\x69\x73\x74\x29\x20\x2d\x3e\x20\x6c\x69\x73\x74\x3a\x0a\x20\x20\x20\x20\x23\x20\x61\x20\x3e\x3d\x20\x62\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x62\x6f\x72\x72\x6f\x77\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x62\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6d\x70\x20\x3d\x20\x61\x5b\x69\x5d\x20\x2d\x20\x62\x5b\x69\x5d\x20\x2d\x20\x62\x6f\x72\x72\x6f\x77\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x6d\x70\x20\x3c\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6d\x70\x20\x2b\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x42\x41\x53\x45\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x6f\x72\x72\x6f\x77\x20\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x6f\x72\x72\x6f\x77\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x74\x6d\x70\x29\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x62\x29\x2c\x20\x6c\x65\x6e\x28\x61\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6d\x70\x20\x3d\x20\x61\x5b\x69\x5d\x20\x2d\x20\x62\x6f\x72\x72\x6f\x77\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x6d\x70\x20\x3c\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x74\x6d\x70\x20\x2b\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x42\x41\x53\x45\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x6f\x72\x72\x6f\x77\x20\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x6f\x72\x72\x6f\x77\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x74\x6d\x70\x29\x0a\x20\x20\x20\x20\x75\x6c\x6f\x6e\x67\x5f\x75\x6e\x70\x61\x64\x5f\x28\x72\x65\x73\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x64\x69\x76\x6d\x6f\x64\x69\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x62\x20\x3e\x20\x30\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x61\x29\x2d\x31\x2c\x20\x2d\x31\x2c\x20\x2d\x31\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3c\x3c\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x2b\x3d\x20\x61\x5b\x69\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x63\x61\x72\x72\x79\x20\x2f\x2f\x20\x62\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61" "\x72\x72\x79\x20\x25\x3d\x20\x62\x0a\x20\x20\x20\x20\x72\x65\x73\x2e\x72\x65\x76\x65\x72\x73\x65\x28\x29\x0a\x20\x20\x20\x20\x75\x6c\x6f\x6e\x67\x5f\x75\x6e\x70\x61\x64\x5f\x28\x72\x65\x73\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x2c\x20\x63\x61\x72\x72\x79\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x64\x69\x76\x6d\x6f\x64\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x6c\x69\x73\x74\x29\x3a\x0a\x20\x20\x20\x20\x71\x20\x3d\x20\x5b\x30\x5d\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x75\x6c\x6f\x6e\x67\x5f\x63\x6d\x70\x28\x61\x2c\x20\x62\x29\x20\x3e\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x75\x6c\x6f\x6e\x67\x5f\x69\x6e\x63\x5f\x28\x71\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x73\x75\x62\x28\x61\x2c\x20\x62\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x71\x2c\x20\x61\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x66\x6c\x6f\x6f\x72\x64\x69\x76\x69\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x62\x20\x3e\x20\x30\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x75\x6c\x6f\x6e\x67\x5f\x64\x69\x76\x6d\x6f\x64\x69\x28\x61\x2c\x20\x62\x29\x5b\x30\x5d\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x69\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x62\x20\x3e\x3d\x20\x30\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x30\x5d\x20\x2a\x20\x6c\x65\x6e\x28\x61\x29\x0a\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x61\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x2b\x3d\x20\x61\x5b\x69\x5d\x20\x2a\x20\x62\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x5b\x69\x5d\x20\x3d\x20\x63\x61\x72\x72\x79\x20\x26\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x4d\x41\x53\x4b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3e\x3e\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x0a\x20\x20\x20\x20\x69\x66\x20\x63\x61\x72\x72\x79\x20\x3e\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x63\x61\x72\x72\x79\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x6c\x69\x73\x74\x29\x3a\x0a\x20\x20\x20\x20\x4e\x20\x3d\x20\x6c\x65\x6e\x28\x61\x29\x20\x2b\x20\x6c\x65\x6e\x28\x62\x29\x0a\x20\x20\x20\x20\x23\x20\x75\x73\x65\x20\x67\x72\x61\x64\x65\x2d\x73\x63\x68\x6f\x6f\x6c\x20\x6d\x75\x6c\x74\x69\x70\x6c\x69\x63\x61\x74\x69\x6f\x6e\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x30\x5d\x20\x2a\x20\x4e\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x61\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x6a\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x62\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x2b\x3d\x20\x72\x65\x73\x5b\x69\x2b\x6a\x5d\x20\x2b\x20\x61\x5b\x69\x5d\x20\x2a\x20\x62\x5b\x6a\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x5b\x69\x2b\x6a\x5d\x20\x3d\x20\x63\x61\x72\x72\x79\x20\x26\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x4d\x41\x53\x4b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x61\x72\x72\x79\x20\x3e\x3e\x3d\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x5b\x69\x2b\x6c\x65\x6e\x28\x62\x29\x5d\x20\x3d\x20\x63\x61\x72\x72\x79\x0a\x20\x20\x20\x20\x75\x6c\x6f\x6e\x67\x5f\x75\x6e\x70\x61\x64\x5f\x28\x72\x65\x73\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x70\x6f\x77\x69\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x62\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x23\x20\x62\x20\x3e\x3d\x20\x30\x0a\x20\x20\x20\x20\x69\x66\x20\x62\x20\x3d\x3d\x20\x30\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x31\x5d\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x31\x5d\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x62\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x62\x20\x26\x20\x31\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x28\x72\x65\x73\x2c\x20\x61\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x28\x61\x2c\x20\x61\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x62\x20\x3e\x3e\x3d\x20\x31\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x72\x65\x70\x72\x28\x78\x3a\x20\x6c\x69\x73\x74\x29\x20\x2d\x3e\x20\x73\x74\x72\x3a\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6c\x65\x6e\x28\x78\x29\x3e\x31\x20\x6f\x72\x20\x78\x5b\x30\x5d\x3e\x30\x3a\x20\x20\x20\x23\x20\x6e\x6f\x6e\x2d\x7a\x65\x72\x6f" "\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x78\x2c\x20\x72\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x64\x69\x76\x6d\x6f\x64\x69\x28\x78\x2c\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x44\x45\x43\x49\x4d\x41\x4c\x5f\x42\x41\x53\x45\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x74\x72\x28\x72\x29\x2e\x7a\x66\x69\x6c\x6c\x28\x50\x79\x4c\x6f\x6e\x67\x5f\x44\x45\x43\x49\x4d\x41\x4c\x5f\x53\x48\x49\x46\x54\x29\x29\x0a\x20\x20\x20\x20\x72\x65\x73\x2e\x72\x65\x76\x65\x72\x73\x65\x28\x29\x0a\x20\x20\x20\x20\x73\x20\x3d\x20\x27\x27\x2e\x6a\x6f\x69\x6e\x28\x72\x65\x73\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x73\x29\x20\x3d\x3d\x20\x30\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x30\x27\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x73\x29\x20\x3e\x20\x31\x3a\x20\x73\x20\x3d\x20\x73\x2e\x6c\x73\x74\x72\x69\x70\x28\x27\x30\x27\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x0a\x0a\x64\x65\x66\x20\x75\x6c\x6f\x6e\x67\x5f\x66\x72\x6f\x6d\x73\x74\x72\x28\x73\x3a\x20\x73\x74\x72\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x73\x5b\x2d\x31\x5d\x20\x3d\x3d\x20\x27\x4c\x27\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x20\x3d\x20\x73\x5b\x3a\x2d\x31\x5d\x0a\x20\x20\x20\x20\x72\x65\x73\x2c\x20\x62\x61\x73\x65\x20\x3d\x20\x5b\x30\x5d\x2c\x20\x5b\x31\x5d\x0a\x20\x20\x20\x20\x69\x66\x20\x73\x5b\x30\x5d\x20\x3d\x3d\x20\x27\x2d\x27\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x69\x67\x6e\x20\x3d\x20\x2d\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x20\x3d\x20\x73\x5b\x31\x3a\x5d\x0a\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x69\x67\x6e\x20\x3d\x20\x31\x0a\x20\x20\x20\x20\x73\x20\x3d\x20\x73\x5b\x3a\x3a\x2d\x31\x5d\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x63\x20\x69\x6e\x20\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x20\x3d\x20\x6f\x72\x64\x28\x63\x29\x20\x2d\x20\x34\x38\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x30\x20\x3c\x3d\x20\x63\x20\x3c\x3d\x20\x39\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x61\x64\x64\x28\x72\x65\x73\x2c\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x69\x28\x62\x61\x73\x65\x2c\x20\x63\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x62\x61\x73\x65\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x69\x28\x62\x61\x73\x65\x2c\x20\x31\x30\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x2c\x20\x73\x69\x67\x6e\x0a\x0a\x63\x6c\x61\x73\x73\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x78\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x78\x29\x20\x69\x73\x20\x74\x75\x70\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x20\x78\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x78\x29\x20\x69\x73\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x66\x72\x6f\x6d\x69\x6e\x74\x28\x78\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x78\x29\x20\x69\x73\x20\x66\x6c\x6f\x61\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x66\x72\x6f\x6d\x69\x6e\x74\x28\x69\x6e\x74\x28\x78\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x78\x29\x20\x69\x73\x20\x73\x74\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x66\x72\x6f\x6d\x73\x74\x72\x28\x78\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x78\x29\x20\x69\x73\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x20\x78\x2e\x64\x69\x67\x69\x74\x73\x2e\x63\x6f\x70\x79\x28\x29\x2c\x20\x78\x2e\x73\x69\x67\x6e\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x54\x79\x70\x65\x45\x72\x72\x6f\x72\x28\x27\x65\x78\x70\x65\x63\x74\x65\x64\x20\x69\x6e\x74\x20\x6f\x72\x20\x73\x74\x72\x27\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x61\x64\x64\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20" "\x20\x20\x20\x6f\x74\x68\x65\x72\x20\x3d\x20\x6c\x6f\x6e\x67\x28\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x3d\x20\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x75\x6c\x6f\x6e\x67\x5f\x61\x64\x64\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6d\x70\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x63\x6d\x70\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x63\x6d\x70\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x30\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x63\x6d\x70\x20\x3e\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x75\x6c\x6f\x6e\x67\x5f\x73\x75\x62\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x75\x6c\x6f\x6e\x67\x5f\x73\x75\x62\x28\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x29\x2c\x20\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x61\x64\x64\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x61\x64\x64\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x74\x68\x65\x72\x20\x3d\x20\x6c\x6f\x6e\x67\x28\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x21\x3d\x20\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x75\x6c\x6f\x6e\x67\x5f\x61\x64\x64\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6d\x70\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x63\x6d\x70\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x63\x6d\x70\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x30\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x63\x6d\x70\x20\x3e\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x75\x6c\x6f\x6e\x67\x5f\x73\x75\x62\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x75\x6c\x6f\x6e\x67\x5f\x73\x75\x62\x28\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x29\x2c\x20\x2d\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74" "\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x74\x68\x65\x72\x20\x3d\x20\x6c\x6f\x6e\x67\x28\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6f\x74\x68\x65\x72\x2e\x5f\x5f\x73\x75\x62\x5f\x5f\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6d\x75\x6c\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x69\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x61\x62\x73\x28\x6f\x74\x68\x65\x72\x29\x29\x2c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x2a\x20\x28\x31\x20\x69\x66\x20\x6f\x74\x68\x65\x72\x20\x3e\x3d\x20\x30\x20\x65\x6c\x73\x65\x20\x2d\x31\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x2c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x2a\x20\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x6d\x75\x6c\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x6d\x75\x6c\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x64\x69\x76\x6d\x6f\x64\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x3d\x20\x31\x20\x61\x6e\x64\x20\x6f\x74\x68\x65\x72\x20\x3e\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x71\x2c\x20\x72\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x64\x69\x76\x6d\x6f\x64\x69\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x71\x2c\x20\x31\x29\x29\x2c\x20\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x3d\x20\x31\x20\x61\x6e\x64\x20\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x20\x3d\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x71\x2c\x20\x72\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x64\x69\x76\x6d\x6f\x64\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x6c\x65\x6e\x28\x6f\x74\x68\x65\x72\x29\x3e\x31\x20\x6f\x72\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x5b\x30\x5d\x3e\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x71\x2c\x20\x31\x29\x29\x2c\x20\x6c\x6f\x6e\x67\x28\x28\x72\x2c\x20\x31\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x45\x72\x72\x6f\x72\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x66\x6c\x6f\x6f\x72\x64\x69\x76\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x64\x69\x76\x6d\x6f\x64\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x5b\x30\x5d\x0a\x0a\x20\x20\x20\x20\x64" "\x65\x66\x20\x5f\x5f\x6d\x6f\x64\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x64\x69\x76\x6d\x6f\x64\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x5b\x31\x5d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x70\x6f\x77\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x20\x61\x6e\x64\x20\x6f\x74\x68\x65\x72\x20\x3e\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3d\x3d\x20\x2d\x31\x20\x61\x6e\x64\x20\x6f\x74\x68\x65\x72\x20\x26\x20\x31\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x69\x67\x6e\x20\x3d\x20\x2d\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x69\x67\x6e\x20\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x75\x6c\x6f\x6e\x67\x5f\x70\x6f\x77\x69\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x29\x2c\x20\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x73\x68\x69\x66\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x20\x61\x6e\x64\x20\x6f\x74\x68\x65\x72\x20\x3e\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x78\x20\x3d\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2e\x63\x6f\x70\x79\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x71\x2c\x20\x72\x20\x3d\x20\x64\x69\x76\x6d\x6f\x64\x28\x6f\x74\x68\x65\x72\x2c\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x78\x20\x3d\x20\x5b\x30\x5d\x2a\x71\x20\x2b\x20\x78\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x5f\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x72\x29\x3a\x20\x78\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x6d\x75\x6c\x69\x28\x78\x2c\x20\x32\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x78\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x73\x68\x69\x66\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x20\x61\x6e\x64\x20\x6f\x74\x68\x65\x72\x20\x3e\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x78\x20\x3d\x20\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2e\x63\x6f\x70\x79\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x71\x2c\x20\x72\x20\x3d\x20\x64\x69\x76\x6d\x6f\x64\x28\x6f\x74\x68\x65\x72\x2c\x20\x50\x79\x4c\x6f\x6e\x67\x5f\x53\x48\x49\x46\x54\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x78\x20\x3d\x20\x78\x5b\x71\x3a\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x78\x3a\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x30\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x5f\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x72\x29\x3a\x20\x78\x20\x3d\x20\x75\x6c\x6f\x6e\x67\x5f\x66\x6c\x6f\x6f\x72\x64\x69\x76\x69\x28\x78\x2c\x20\x32\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x78\x2c\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6e\x65\x67\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x6e\x67\x28\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x2d\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x29\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x63\x6d\x70\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x74\x68\x65\x72\x20\x3d\x20\x6c\x6f\x6e\x67\x28\x6f\x74\x68\x65\x72\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x74\x68\x65\x72\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x6c\x6f\x6e\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4e\x6f\x74\x49\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x65\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3e\x20\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x69\x66\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3c\x20\x6f\x74\x68\x65\x72\x2e\x73\x69\x67\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x2d\x31\x0a\x20\x20" "\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x75\x6c\x6f\x6e\x67\x5f\x63\x6d\x70\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x2c\x20\x6f\x74\x68\x65\x72\x2e\x64\x69\x67\x69\x74\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x63\x6d\x70\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x20\x3d\x3d\x20\x30\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x63\x6d\x70\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x20\x3c\x20\x30\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x63\x6d\x70\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x20\x3c\x3d\x20\x30\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x63\x6d\x70\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x20\x3e\x20\x30\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x65\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x63\x6d\x70\x5f\x5f\x28\x6f\x74\x68\x65\x72\x29\x20\x3e\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x70\x72\x65\x66\x69\x78\x20\x3d\x20\x27\x2d\x27\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x73\x69\x67\x6e\x20\x3c\x20\x30\x20\x65\x6c\x73\x65\x20\x27\x27\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x70\x72\x65\x66\x69\x78\x20\x2b\x20\x75\x6c\x6f\x6e\x67\x5f\x72\x65\x70\x72\x28\x73\x65\x6c\x66\x2e\x64\x69\x67\x69\x74\x73\x29\x20\x2b\x20\x27\x4c\x27" },
        {"bisect", "\x22\x22\x22\x42\x69\x73\x65\x63\x74\x69\x6f\x6e\x20\x61\x6c\x67\x6f\x72\x69\x74\x68\x6d\x73\x2e\x22\x22\x22\x0a\x0a\x64\x65\x66\x20\x69\x6e\x73\x6f\x72\x74\x5f\x72\x69\x67\x68\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x3d\x30\x2c\x20\x68\x69\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x49\x6e\x73\x65\x72\x74\x20\x69\x74\x65\x6d\x20\x78\x20\x69\x6e\x20\x6c\x69\x73\x74\x20\x61\x2c\x20\x61\x6e\x64\x20\x6b\x65\x65\x70\x20\x69\x74\x20\x73\x6f\x72\x74\x65\x64\x20\x61\x73\x73\x75\x6d\x69\x6e\x67\x20\x61\x20\x69\x73\x20\x73\x6f\x72\x74\x65\x64\x2e\x0a\x0a\x20\x20\x20\x20\x49\x66\x20\x78\x20\x69\x73\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x69\x6e\x20\x61\x2c\x20\x69\x6e\x73\x65\x72\x74\x20\x69\x74\x20\x74\x6f\x20\x74\x68\x65\x20\x72\x69\x67\x68\x74\x20\x6f\x66\x20\x74\x68\x65\x20\x72\x69\x67\x68\x74\x6d\x6f\x73\x74\x20\x78\x2e\x0a\x0a\x20\x20\x20\x20\x4f\x70\x74\x69\x6f\x6e\x61\x6c\x20\x61\x72\x67\x73\x20\x6c\x6f\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x30\x29\x20\x61\x6e\x64\x20\x68\x69\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x6c\x65\x6e\x28\x61\x29\x29\x20\x62\x6f\x75\x6e\x64\x20\x74\x68\x65\x0a\x20\x20\x20\x20\x73\x6c\x69\x63\x65\x20\x6f\x66\x20\x61\x20\x74\x6f\x20\x62\x65\x20\x73\x65\x61\x72\x63\x68\x65\x64\x2e\x0a\x20\x20\x20\x20\x22\x22\x22\x0a\x0a\x20\x20\x20\x20\x6c\x6f\x20\x3d\x20\x62\x69\x73\x65\x63\x74\x5f\x72\x69\x67\x68\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x2c\x20\x68\x69\x29\x0a\x20\x20\x20\x20\x61\x2e\x69\x6e\x73\x65\x72\x74\x28\x6c\x6f\x2c\x20\x78\x29\x0a\x0a\x64\x65\x66\x20\x62\x69\x73\x65\x63\x74\x5f\x72\x69\x67\x68\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x3d\x30\x2c\x20\x68\x69\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x52\x65\x74\x75\x72\x6e\x20\x74\x68\x65\x20\x69\x6e\x64\x65\x78\x20\x77\x68\x65\x72\x65\x20\x74\x6f\x20\x69\x6e\x73\x65\x72\x74\x20\x69\x74\x65\x6d\x20\x78\x20\x69\x6e\x20\x6c\x69\x73\x74\x20\x61\x2c\x20\x61\x73\x73\x75\x6d\x69\x6e\x67\x20\x61\x20\x69\x73\x20\x73\x6f\x72\x74\x65\x64\x2e\x0a\x0a\x20\x20\x20\x20\x54\x68\x65\x20\x72\x65\x74\x75\x72\x6e\x20\x76\x61\x6c\x75\x65\x20\x69\x20\x69\x73\x20\x73\x75\x63\x68\x20\x74\x68\x61\x74\x20\x61\x6c\x6c\x20\x65\x20\x69\x6e\x20\x61\x5b\x3a\x69\x5d\x20\x68\x61\x76\x65\x20\x65\x20\x3c\x3d\x20\x78\x2c\x20\x61\x6e\x64\x20\x61\x6c\x6c\x20\x65\x20\x69\x6e\x0a\x20\x20\x20\x20\x61\x5b\x69\x3a\x5d\x20\x68\x61\x76\x65\x20\x65\x20\x3e\x20\x78\x2e\x20\x20\x53\x6f\x20\x69\x66\x20\x78\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x61\x70\x70\x65\x61\x72\x73\x20\x69\x6e\x20\x74\x68\x65\x20\x6c\x69\x73\x74\x2c\x20\x61\x2e\x69\x6e\x73\x65\x72\x74\x28\x78\x29\x20\x77\x69\x6c\x6c\x0a\x20\x20\x20\x20\x69\x6e\x73\x65\x72\x74\x20\x6a\x75\x73\x74\x20\x61\x66\x74\x65\x72\x20\x74\x68\x65\x20\x72\x69\x67\x68\x74\x6d\x6f\x73\x74\x20\x78\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x74\x68\x65\x72\x65\x2e\x0a\x0a\x20\x20\x20\x20\x4f\x70\x74\x69\x6f\x6e\x61\x6c\x20\x61\x72\x67\x73\x20\x6c\x6f\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x30\x29\x20\x61\x6e\x64\x20\x68\x69\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x6c\x65\x6e\x28\x61\x29\x29\x20\x62\x6f\x75\x6e\x64\x20\x74\x68\x65\x0a\x20\x20\x20\x20\x73\x6c\x69\x63\x65\x20\x6f\x66\x20\x61\x20\x74\x6f\x20\x62\x65\x20\x73\x65\x61\x72\x63\x68\x65\x64\x2e\x0a\x20\x20\x20\x20\x22\x22\x22\x0a\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x6f\x20\x3c\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x27\x6c\x6f\x20\x6d\x75\x73\x74\x20\x62\x65\x20\x6e\x6f\x6e\x2d\x6e\x65\x67\x61\x74\x69\x76\x65\x27\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x69\x20\x69\x73\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x69\x20\x3d\x20\x6c\x65\x6e\x28\x61\x29\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6c\x6f\x20\x3c\x20\x68\x69\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6d\x69\x64\x20\x3d\x20\x28\x6c\x6f\x2b\x68\x69\x29\x2f\x2f\x32\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x78\x20\x3c\x20\x61\x5b\x6d\x69\x64\x5d\x3a\x20\x68\x69\x20\x3d\x20\x6d\x69\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x20\x6c\x6f\x20\x3d\x20\x6d\x69\x64\x2b\x31\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x0a\x0a\x64\x65\x66\x20\x69\x6e\x73\x6f\x72\x74\x5f\x6c\x65\x66\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x3d\x30\x2c\x20\x68\x69\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x49\x6e\x73\x65\x72\x74\x20\x69\x74\x65\x6d\x20\x78\x20\x69\x6e\x20\x6c\x69\x73\x74\x20\x61\x2c\x20\x61\x6e\x64\x20\x6b\x65\x65\x70\x20\x69\x74\x20\x73\x6f\x72\x74\x65\x64\x20\x61\x73\x73\x75\x6d\x69\x6e\x67\x20\x61\x20\x69\x73\x20\x73\x6f\x72\x74\x65\x64\x2e\x0a\x0a\x20\x20\x20\x20\x49\x66\x20\x78\x20\x69\x73\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x69\x6e\x20\x61\x2c\x20\x69\x6e\x73\x65\x72\x74\x20\x69\x74\x20\x74\x6f\x20\x74\x68\x65\x20\x6c\x65\x66\x74\x20\x6f\x66\x20\x74\x68\x65\x20\x6c\x65\x66\x74\x6d\x6f\x73\x74\x20\x78\x2e\x0a\x0a\x20\x20\x20\x20\x4f\x70\x74\x69\x6f\x6e\x61\x6c\x20\x61\x72\x67\x73\x20\x6c\x6f\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x30\x29\x20\x61\x6e\x64\x20" "\x68\x69\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x6c\x65\x6e\x28\x61\x29\x29\x20\x62\x6f\x75\x6e\x64\x20\x74\x68\x65\x0a\x20\x20\x20\x20\x73\x6c\x69\x63\x65\x20\x6f\x66\x20\x61\x20\x74\x6f\x20\x62\x65\x20\x73\x65\x61\x72\x63\x68\x65\x64\x2e\x0a\x20\x20\x20\x20\x22\x22\x22\x0a\x0a\x20\x20\x20\x20\x6c\x6f\x20\x3d\x20\x62\x69\x73\x65\x63\x74\x5f\x6c\x65\x66\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x2c\x20\x68\x69\x29\x0a\x20\x20\x20\x20\x61\x2e\x69\x6e\x73\x65\x72\x74\x28\x6c\x6f\x2c\x20\x78\x29\x0a\x0a\x0a\x64\x65\x66\x20\x62\x69\x73\x65\x63\x74\x5f\x6c\x65\x66\x74\x28\x61\x2c\x20\x78\x2c\x20\x6c\x6f\x3d\x30\x2c\x20\x68\x69\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x52\x65\x74\x75\x72\x6e\x20\x74\x68\x65\x20\x69\x6e\x64\x65\x78\x20\x77\x68\x65\x72\x65\x20\x74\x6f\x20\x69\x6e\x73\x65\x72\x74\x20\x69\x74\x65\x6d\x20\x78\x20\x69\x6e\x20\x6c\x69\x73\x74\x20\x61\x2c\x20\x61\x73\x73\x75\x6d\x69\x6e\x67\x20\x61\x20\x69\x73\x20\x73\x6f\x72\x74\x65\x64\x2e\x0a\x0a\x20\x20\x20\x20\x54\x68\x65\x20\x72\x65\x74\x75\x72\x6e\x20\x76\x61\x6c\x75\x65\x20\x69\x20\x69\x73\x20\x73\x75\x63\x68\x20\x74\x68\x61\x74\x20\x61\x6c\x6c\x20\x65\x20\x69\x6e\x20\x61\x5b\x3a\x69\x5d\x20\x68\x61\x76\x65\x20\x65\x20\x3c\x20\x78\x2c\x20\x61\x6e\x64\x20\x61\x6c\x6c\x20\x65\x20\x69\x6e\x0a\x20\x20\x20\x20\x61\x5b\x69\x3a\x5d\x20\x68\x61\x76\x65\x20\x65\x20\x3e\x3d\x20\x78\x2e\x20\x20\x53\x6f\x20\x69\x66\x20\x78\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x61\x70\x70\x65\x61\x72\x73\x20\x69\x6e\x20\x74\x68\x65\x20\x6c\x69\x73\x74\x2c\x20\x61\x2e\x69\x6e\x73\x65\x72\x74\x28\x78\x29\x20\x77\x69\x6c\x6c\x0a\x20\x20\x20\x20\x69\x6e\x73\x65\x72\x74\x20\x6a\x75\x73\x74\x20\x62\x65\x66\x6f\x72\x65\x20\x74\x68\x65\x20\x6c\x65\x66\x74\x6d\x6f\x73\x74\x20\x78\x20\x61\x6c\x72\x65\x61\x64\x79\x20\x74\x68\x65\x72\x65\x2e\x0a\x0a\x20\x20\x20\x20\x4f\x70\x74\x69\x6f\x6e\x61\x6c\x20\x61\x72\x67\x73\x20\x6c\x6f\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x30\x29\x20\x61\x6e\x64\x20\x68\x69\x20\x28\x64\x65\x66\x61\x75\x6c\x74\x20\x6c\x65\x6e\x28\x61\x29\x29\x20\x62\x6f\x75\x6e\x64\x20\x74\x68\x65\x0a\x20\x20\x20\x20\x73\x6c\x69\x63\x65\x20\x6f\x66\x20\x61\x20\x74\x6f\x20\x62\x65\x20\x73\x65\x61\x72\x63\x68\x65\x64\x2e\x0a\x20\x20\x20\x20\x22\x22\x22\x0a\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x6f\x20\x3c\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x27\x6c\x6f\x20\x6d\x75\x73\x74\x20\x62\x65\x20\x6e\x6f\x6e\x2d\x6e\x65\x67\x61\x74\x69\x76\x65\x27\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x69\x20\x69\x73\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x69\x20\x3d\x20\x6c\x65\x6e\x28\x61\x29\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6c\x6f\x20\x3c\x20\x68\x69\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6d\x69\x64\x20\x3d\x20\x28\x6c\x6f\x2b\x68\x69\x29\x2f\x2f\x32\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x61\x5b\x6d\x69\x64\x5d\x20\x3c\x20\x78\x3a\x20\x6c\x6f\x20\x3d\x20\x6d\x69\x64\x2b\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x20\x68\x69\x20\x3d\x20\x6d\x69\x64\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x6f\x0a\x0a\x23\x20\x43\x72\x65\x61\x74\x65\x20\x61\x6c\x69\x61\x73\x65\x73\x0a\x62\x69\x73\x65\x63\x74\x20\x3d\x20\x62\x69\x73\x65\x63\x74\x5f\x72\x69\x67\x68\x74\x0a\x69\x6e\x73\x6f\x72\x74\x20\x3d\x20\x69\x6e\x73\x6f\x72\x74\x5f\x72\x69\x67\x68\x74\x0a" },
        {"heapq", "\x23\x20\x48\x65\x61\x70\x20\x71\x75\x65\x75\x65\x20\x61\x6c\x67\x6f\x72\x69\x74\x68\x6d\x20\x28\x61\x2e\x6b\x2e\x61\x2e\x20\x70\x72\x69\x6f\x72\x69\x74\x79\x20\x71\x75\x65\x75\x65\x29\x0a\x64\x65\x66\x20\x68\x65\x61\x70\x70\x75\x73\x68\x28\x68\x65\x61\x70\x2c\x20\x69\x74\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x50\x75\x73\x68\x20\x69\x74\x65\x6d\x20\x6f\x6e\x74\x6f\x20\x68\x65\x61\x70\x2c\x20\x6d\x61\x69\x6e\x74\x61\x69\x6e\x69\x6e\x67\x20\x74\x68\x65\x20\x68\x65\x61\x70\x20\x69\x6e\x76\x61\x72\x69\x61\x6e\x74\x2e\x22\x22\x22\x0a\x20\x20\x20\x20\x68\x65\x61\x70\x2e\x61\x70\x70\x65\x6e\x64\x28\x69\x74\x65\x6d\x29\x0a\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x64\x6f\x77\x6e\x28\x68\x65\x61\x70\x2c\x20\x30\x2c\x20\x6c\x65\x6e\x28\x68\x65\x61\x70\x29\x2d\x31\x29\x0a\x0a\x64\x65\x66\x20\x68\x65\x61\x70\x70\x6f\x70\x28\x68\x65\x61\x70\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x50\x6f\x70\x20\x74\x68\x65\x20\x73\x6d\x61\x6c\x6c\x65\x73\x74\x20\x69\x74\x65\x6d\x20\x6f\x66\x66\x20\x74\x68\x65\x20\x68\x65\x61\x70\x2c\x20\x6d\x61\x69\x6e\x74\x61\x69\x6e\x69\x6e\x67\x20\x74\x68\x65\x20\x68\x65\x61\x70\x20\x69\x6e\x76\x61\x72\x69\x61\x6e\x74\x2e\x22\x22\x22\x0a\x20\x20\x20\x20\x6c\x61\x73\x74\x65\x6c\x74\x20\x3d\x20\x68\x65\x61\x70\x2e\x70\x6f\x70\x28\x29\x20\x20\x20\x20\x23\x20\x72\x61\x69\x73\x65\x73\x20\x61\x70\x70\x72\x6f\x70\x72\x69\x61\x74\x65\x20\x49\x6e\x64\x65\x78\x45\x72\x72\x6f\x72\x20\x69\x66\x20\x68\x65\x61\x70\x20\x69\x73\x20\x65\x6d\x70\x74\x79\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x65\x61\x70\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x69\x74\x65\x6d\x20\x3d\x20\x68\x65\x61\x70\x5b\x30\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x30\x5d\x20\x3d\x20\x6c\x61\x73\x74\x65\x6c\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x75\x70\x28\x68\x65\x61\x70\x2c\x20\x30\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x75\x72\x6e\x69\x74\x65\x6d\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x61\x73\x74\x65\x6c\x74\x0a\x0a\x64\x65\x66\x20\x68\x65\x61\x70\x72\x65\x70\x6c\x61\x63\x65\x28\x68\x65\x61\x70\x2c\x20\x69\x74\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x50\x6f\x70\x20\x61\x6e\x64\x20\x72\x65\x74\x75\x72\x6e\x20\x74\x68\x65\x20\x63\x75\x72\x72\x65\x6e\x74\x20\x73\x6d\x61\x6c\x6c\x65\x73\x74\x20\x76\x61\x6c\x75\x65\x2c\x20\x61\x6e\x64\x20\x61\x64\x64\x20\x74\x68\x65\x20\x6e\x65\x77\x20\x69\x74\x65\x6d\x2e\x0a\x0a\x20\x20\x20\x20\x54\x68\x69\x73\x20\x69\x73\x20\x6d\x6f\x72\x65\x20\x65\x66\x66\x69\x63\x69\x65\x6e\x74\x20\x74\x68\x61\x6e\x20\x68\x65\x61\x70\x70\x6f\x70\x28\x29\x20\x66\x6f\x6c\x6c\x6f\x77\x65\x64\x20\x62\x79\x20\x68\x65\x61\x70\x70\x75\x73\x68\x28\x29\x2c\x20\x61\x6e\x64\x20\x63\x61\x6e\x20\x62\x65\x0a\x20\x20\x20\x20\x6d\x6f\x72\x65\x20\x61\x70\x70\x72\x6f\x70\x72\x69\x61\x74\x65\x20\x77\x68\x65\x6e\x20\x75\x73\x69\x6e\x67\x20\x61\x20\x66\x69\x78\x65\x64\x2d\x73\x69\x7a\x65\x20\x68\x65\x61\x70\x2e\x20\x20\x4e\x6f\x74\x65\x20\x74\x68\x61\x74\x20\x74\x68\x65\x20\x76\x61\x6c\x75\x65\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x65\x64\x20\x6d\x61\x79\x20\x62\x65\x20\x6c\x61\x72\x67\x65\x72\x20\x74\x68\x61\x6e\x20\x69\x74\x65\x6d\x21\x20\x20\x54\x68\x61\x74\x20\x63\x6f\x6e\x73\x74\x72\x61\x69\x6e\x73\x20\x72\x65\x61\x73\x6f\x6e\x61\x62\x6c\x65\x20\x75\x73\x65\x73\x20\x6f\x66\x0a\x20\x20\x20\x20\x74\x68\x69\x73\x20\x72\x6f\x75\x74\x69\x6e\x65\x20\x75\x6e\x6c\x65\x73\x73\x20\x77\x72\x69\x74\x74\x65\x6e\x20\x61\x73\x20\x70\x61\x72\x74\x20\x6f\x66\x20\x61\x20\x63\x6f\x6e\x64\x69\x74\x69\x6f\x6e\x61\x6c\x20\x72\x65\x70\x6c\x61\x63\x65\x6d\x65\x6e\x74\x3a\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x74\x65\x6d\x20\x3e\x20\x68\x65\x61\x70\x5b\x30\x5d\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x74\x65\x6d\x20\x3d\x20\x68\x65\x61\x70\x72\x65\x70\x6c\x61\x63\x65\x28\x68\x65\x61\x70\x2c\x20\x69\x74\x65\x6d\x29\x0a\x20\x20\x20\x20\x22\x22\x22\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x69\x74\x65\x6d\x20\x3d\x20\x68\x65\x61\x70\x5b\x30\x5d\x20\x20\x20\x20\x23\x20\x72\x61\x69\x73\x65\x73\x20\x61\x70\x70\x72\x6f\x70\x72\x69\x61\x74\x65\x20\x49\x6e\x64\x65\x78\x45\x72\x72\x6f\x72\x20\x69\x66\x20\x68\x65\x61\x70\x20\x69\x73\x20\x65\x6d\x70\x74\x79\x0a\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x30\x5d\x20\x3d\x20\x69\x74\x65\x6d\x0a\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x75\x70\x28\x68\x65\x61\x70\x2c\x20\x30\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x75\x72\x6e\x69\x74\x65\x6d\x0a\x0a\x64\x65\x66\x20\x68\x65\x61\x70\x70\x75\x73\x68\x70\x6f\x70\x28\x68\x65\x61\x70\x2c\x20\x69\x74\x65\x6d\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x46\x61\x73\x74\x20\x76\x65\x72\x73\x69\x6f\x6e\x20\x6f\x66\x20\x61\x20\x68\x65\x61\x70\x70\x75\x73\x68\x20\x66\x6f\x6c\x6c\x6f\x77\x65\x64\x20\x62\x79\x20\x61\x20\x68\x65\x61\x70\x70\x6f\x70\x2e\x22\x22\x22\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x65\x61\x70\x20\x61\x6e\x64\x20\x68\x65\x61\x70\x5b\x30\x5d\x20\x3c\x20\x69\x74\x65\x6d\x3a\x0a\x20\x20\x20\x20\x20\x20\x20" "\x20\x69\x74\x65\x6d\x2c\x20\x68\x65\x61\x70\x5b\x30\x5d\x20\x3d\x20\x68\x65\x61\x70\x5b\x30\x5d\x2c\x20\x69\x74\x65\x6d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x75\x70\x28\x68\x65\x61\x70\x2c\x20\x30\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x74\x65\x6d\x0a\x0a\x64\x65\x66\x20\x68\x65\x61\x70\x69\x66\x79\x28\x78\x29\x3a\x0a\x20\x20\x20\x20\x22\x22\x22\x54\x72\x61\x6e\x73\x66\x6f\x72\x6d\x20\x6c\x69\x73\x74\x20\x69\x6e\x74\x6f\x20\x61\x20\x68\x65\x61\x70\x2c\x20\x69\x6e\x2d\x70\x6c\x61\x63\x65\x2c\x20\x69\x6e\x20\x4f\x28\x6c\x65\x6e\x28\x78\x29\x29\x20\x74\x69\x6d\x65\x2e\x22\x22\x22\x0a\x20\x20\x20\x20\x6e\x20\x3d\x20\x6c\x65\x6e\x28\x78\x29\x0a\x20\x20\x20\x20\x23\x20\x54\x72\x61\x6e\x73\x66\x6f\x72\x6d\x20\x62\x6f\x74\x74\x6f\x6d\x2d\x75\x70\x2e\x20\x20\x54\x68\x65\x20\x6c\x61\x72\x67\x65\x73\x74\x20\x69\x6e\x64\x65\x78\x20\x74\x68\x65\x72\x65\x27\x73\x20\x61\x6e\x79\x20\x70\x6f\x69\x6e\x74\x20\x74\x6f\x20\x6c\x6f\x6f\x6b\x69\x6e\x67\x20\x61\x74\x0a\x20\x20\x20\x20\x23\x20\x69\x73\x20\x74\x68\x65\x20\x6c\x61\x72\x67\x65\x73\x74\x20\x77\x69\x74\x68\x20\x61\x20\x63\x68\x69\x6c\x64\x20\x69\x6e\x64\x65\x78\x20\x69\x6e\x2d\x72\x61\x6e\x67\x65\x2c\x20\x73\x6f\x20\x6d\x75\x73\x74\x20\x68\x61\x76\x65\x20\x32\x2a\x69\x20\x2b\x20\x31\x20\x3c\x20\x6e\x2c\x0a\x20\x20\x20\x20\x23\x20\x6f\x72\x20\x69\x20\x3c\x20\x28\x6e\x2d\x31\x29\x2f\x32\x2e\x20\x20\x49\x66\x20\x6e\x20\x69\x73\x20\x65\x76\x65\x6e\x20\x3d\x20\x32\x2a\x6a\x2c\x20\x74\x68\x69\x73\x20\x69\x73\x20\x28\x32\x2a\x6a\x2d\x31\x29\x2f\x32\x20\x3d\x20\x6a\x2d\x31\x2f\x32\x20\x73\x6f\x0a\x20\x20\x20\x20\x23\x20\x6a\x2d\x31\x20\x69\x73\x20\x74\x68\x65\x20\x6c\x61\x72\x67\x65\x73\x74\x2c\x20\x77\x68\x69\x63\x68\x20\x69\x73\x20\x6e\x2f\x2f\x32\x20\x2d\x20\x31\x2e\x20\x20\x49\x66\x20\x6e\x20\x69\x73\x20\x6f\x64\x64\x20\x3d\x20\x32\x2a\x6a\x2b\x31\x2c\x20\x74\x68\x69\x73\x20\x69\x73\x0a\x20\x20\x20\x20\x23\x20\x28\x32\x2a\x6a\x2b\x31\x2d\x31\x29\x2f\x32\x20\x3d\x20\x6a\x20\x73\x6f\x20\x6a\x2d\x31\x20\x69\x73\x20\x74\x68\x65\x20\x6c\x61\x72\x67\x65\x73\x74\x2c\x20\x61\x6e\x64\x20\x74\x68\x61\x74\x27\x73\x20\x61\x67\x61\x69\x6e\x20\x6e\x2f\x2f\x32\x2d\x31\x2e\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x65\x76\x65\x72\x73\x65\x64\x28\x72\x61\x6e\x67\x65\x28\x6e\x2f\x2f\x32\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x75\x70\x28\x78\x2c\x20\x69\x29\x0a\x0a\x23\x20\x27\x68\x65\x61\x70\x27\x20\x69\x73\x20\x61\x20\x68\x65\x61\x70\x20\x61\x74\x20\x61\x6c\x6c\x20\x69\x6e\x64\x69\x63\x65\x73\x20\x3e\x3d\x20\x73\x74\x61\x72\x74\x70\x6f\x73\x2c\x20\x65\x78\x63\x65\x70\x74\x20\x70\x6f\x73\x73\x69\x62\x6c\x79\x20\x66\x6f\x72\x20\x70\x6f\x73\x2e\x20\x20\x70\x6f\x73\x0a\x23\x20\x69\x73\x20\x74\x68\x65\x20\x69\x6e\x64\x65\x78\x20\x6f\x66\x20\x61\x20\x6c\x65\x61\x66\x20\x77\x69\x74\x68\x20\x61\x20\x70\x6f\x73\x73\x69\x62\x6c\x79\x20\x6f\x75\x74\x2d\x6f\x66\x2d\x6f\x72\x64\x65\x72\x20\x76\x61\x6c\x75\x65\x2e\x20\x20\x52\x65\x73\x74\x6f\x72\x65\x20\x74\x68\x65\x0a\x23\x20\x68\x65\x61\x70\x20\x69\x6e\x76\x61\x72\x69\x61\x6e\x74\x2e\x0a\x64\x65\x66\x20\x5f\x73\x69\x66\x74\x64\x6f\x77\x6e\x28\x68\x65\x61\x70\x2c\x20\x73\x74\x61\x72\x74\x70\x6f\x73\x2c\x20\x70\x6f\x73\x29\x3a\x0a\x20\x20\x20\x20\x6e\x65\x77\x69\x74\x65\x6d\x20\x3d\x20\x68\x65\x61\x70\x5b\x70\x6f\x73\x5d\x0a\x20\x20\x20\x20\x23\x20\x46\x6f\x6c\x6c\x6f\x77\x20\x74\x68\x65\x20\x70\x61\x74\x68\x20\x74\x6f\x20\x74\x68\x65\x20\x72\x6f\x6f\x74\x2c\x20\x6d\x6f\x76\x69\x6e\x67\x20\x70\x61\x72\x65\x6e\x74\x73\x20\x64\x6f\x77\x6e\x20\x75\x6e\x74\x69\x6c\x20\x66\x69\x6e\x64\x69\x6e\x67\x20\x61\x20\x70\x6c\x61\x63\x65\x0a\x20\x20\x20\x20\x23\x20\x6e\x65\x77\x69\x74\x65\x6d\x20\x66\x69\x74\x73\x2e\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x70\x6f\x73\x20\x3e\x20\x73\x74\x61\x72\x74\x70\x6f\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x70\x61\x72\x65\x6e\x74\x70\x6f\x73\x20\x3d\x20\x28\x70\x6f\x73\x20\x2d\x20\x31\x29\x20\x3e\x3e\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x70\x61\x72\x65\x6e\x74\x20\x3d\x20\x68\x65\x61\x70\x5b\x70\x61\x72\x65\x6e\x74\x70\x6f\x73\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x65\x77\x69\x74\x65\x6d\x20\x3c\x20\x70\x61\x72\x65\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x70\x6f\x73\x5d\x20\x3d\x20\x70\x61\x72\x65\x6e\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x70\x6f\x73\x20\x3d\x20\x70\x61\x72\x65\x6e\x74\x70\x6f\x73\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6f\x6e\x74\x69\x6e\x75\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x62\x72\x65\x61\x6b\x0a\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x70\x6f\x73\x5d\x20\x3d\x20\x6e\x65\x77\x69\x74\x65\x6d\x0a\x0a\x64\x65\x66\x20\x5f\x73\x69\x66\x74\x75\x70\x28\x68\x65\x61\x70\x2c\x20\x70\x6f\x73\x29\x3a\x0a\x20\x20\x20\x20\x65\x6e\x64\x70\x6f\x73\x20\x3d\x20\x6c\x65\x6e\x28\x68\x65\x61\x70\x29\x0a\x20\x20\x20\x20\x73\x74\x61\x72\x74\x70\x6f\x73\x20\x3d\x20\x70\x6f\x73\x0a\x20\x20\x20\x20\x6e\x65\x77\x69\x74\x65\x6d\x20\x3d\x20\x68" "\x65\x61\x70\x5b\x70\x6f\x73\x5d\x0a\x20\x20\x20\x20\x23\x20\x42\x75\x62\x62\x6c\x65\x20\x75\x70\x20\x74\x68\x65\x20\x73\x6d\x61\x6c\x6c\x65\x72\x20\x63\x68\x69\x6c\x64\x20\x75\x6e\x74\x69\x6c\x20\x68\x69\x74\x74\x69\x6e\x67\x20\x61\x20\x6c\x65\x61\x66\x2e\x0a\x20\x20\x20\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x3d\x20\x32\x2a\x70\x6f\x73\x20\x2b\x20\x31\x20\x20\x20\x20\x23\x20\x6c\x65\x66\x74\x6d\x6f\x73\x74\x20\x63\x68\x69\x6c\x64\x20\x70\x6f\x73\x69\x74\x69\x6f\x6e\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x3c\x20\x65\x6e\x64\x70\x6f\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x53\x65\x74\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x74\x6f\x20\x69\x6e\x64\x65\x78\x20\x6f\x66\x20\x73\x6d\x61\x6c\x6c\x65\x72\x20\x63\x68\x69\x6c\x64\x2e\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x69\x67\x68\x74\x70\x6f\x73\x20\x3d\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x2b\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x72\x69\x67\x68\x74\x70\x6f\x73\x20\x3c\x20\x65\x6e\x64\x70\x6f\x73\x20\x61\x6e\x64\x20\x6e\x6f\x74\x20\x68\x65\x61\x70\x5b\x63\x68\x69\x6c\x64\x70\x6f\x73\x5d\x20\x3c\x20\x68\x65\x61\x70\x5b\x72\x69\x67\x68\x74\x70\x6f\x73\x5d\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x3d\x20\x72\x69\x67\x68\x74\x70\x6f\x73\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x4d\x6f\x76\x65\x20\x74\x68\x65\x20\x73\x6d\x61\x6c\x6c\x65\x72\x20\x63\x68\x69\x6c\x64\x20\x75\x70\x2e\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x70\x6f\x73\x5d\x20\x3d\x20\x68\x65\x61\x70\x5b\x63\x68\x69\x6c\x64\x70\x6f\x73\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x70\x6f\x73\x20\x3d\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x68\x69\x6c\x64\x70\x6f\x73\x20\x3d\x20\x32\x2a\x70\x6f\x73\x20\x2b\x20\x31\x0a\x20\x20\x20\x20\x23\x20\x54\x68\x65\x20\x6c\x65\x61\x66\x20\x61\x74\x20\x70\x6f\x73\x20\x69\x73\x20\x65\x6d\x70\x74\x79\x20\x6e\x6f\x77\x2e\x20\x20\x50\x75\x74\x20\x6e\x65\x77\x69\x74\x65\x6d\x20\x74\x68\x65\x72\x65\x2c\x20\x61\x6e\x64\x20\x62\x75\x62\x62\x6c\x65\x20\x69\x74\x20\x75\x70\x0a\x20\x20\x20\x20\x23\x20\x74\x6f\x20\x69\x74\x73\x20\x66\x69\x6e\x61\x6c\x20\x72\x65\x73\x74\x69\x6e\x67\x20\x70\x6c\x61\x63\x65\x20\x28\x62\x79\x20\x73\x69\x66\x74\x69\x6e\x67\x20\x69\x74\x73\x20\x70\x61\x72\x65\x6e\x74\x73\x20\x64\x6f\x77\x6e\x29\x2e\x0a\x20\x20\x20\x20\x68\x65\x61\x70\x5b\x70\x6f\x73\x5d\x20\x3d\x20\x6e\x65\x77\x69\x74\x65\x6d\x0a\x20\x20\x20\x20\x5f\x73\x69\x66\x74\x64\x6f\x77\x6e\x28\x68\x65\x61\x70\x2c\x20\x73\x74\x61\x72\x74\x70\x6f\x73\x2c\x20\x70\x6f\x73\x29" },
        {"functools", "\x64\x65\x66\x20\x63\x61\x63\x68\x65\x28\x66\x29\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x77\x72\x61\x70\x70\x65\x72\x28\x2a\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x68\x61\x73\x61\x74\x74\x72\x28\x66\x2c\x20\x27\x5f\x5f\x63\x61\x63\x68\x65\x5f\x5f\x27\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x2e\x5f\x5f\x63\x61\x63\x68\x65\x5f\x5f\x20\x3d\x20\x7b\x7d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6b\x65\x79\x20\x3d\x20\x61\x72\x67\x73\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6b\x65\x79\x20\x6e\x6f\x74\x20\x69\x6e\x20\x66\x2e\x5f\x5f\x63\x61\x63\x68\x65\x5f\x5f\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x2e\x5f\x5f\x63\x61\x63\x68\x65\x5f\x5f\x5b\x6b\x65\x79\x5d\x20\x3d\x20\x66\x28\x2a\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x2e\x5f\x5f\x63\x61\x63\x68\x65\x5f\x5f\x5b\x6b\x65\x79\x5d\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x77\x72\x61\x70\x70\x65\x72" },
        {"builtins", "\x69\x6d\x70\x6f\x72\x74\x20\x73\x79\x73\x20\x61\x73\x20\x5f\x73\x79\x73\x0a\x0a\x64\x65\x66\x20\x70\x72\x69\x6e\x74\x28\x2a\x61\x72\x67\x73\x2c\x20\x73\x65\x70\x3d\x27\x20\x27\x2c\x20\x65\x6e\x64\x3d\x27\x5c\x6e\x27\x29\x3a\x0a\x20\x20\x20\x20\x73\x20\x3d\x20\x73\x65\x70\x2e\x6a\x6f\x69\x6e\x28\x5b\x73\x74\x72\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x61\x72\x67\x73\x5d\x29\x0a\x20\x20\x20\x20\x5f\x73\x79\x73\x2e\x73\x74\x64\x6f\x75\x74\x2e\x77\x72\x69\x74\x65\x28\x73\x20\x2b\x20\x65\x6e\x64\x29\x0a\x0a\x64\x65\x66\x20\x72\x6f\x75\x6e\x64\x28\x78\x2c\x20\x6e\x64\x69\x67\x69\x74\x73\x3d\x30\x29\x3a\x0a\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x6e\x64\x69\x67\x69\x74\x73\x20\x3e\x3d\x20\x30\x0a\x20\x20\x20\x20\x69\x66\x20\x6e\x64\x69\x67\x69\x74\x73\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x6e\x74\x28\x78\x20\x2b\x20\x30\x2e\x35\x29\x20\x69\x66\x20\x78\x20\x3e\x3d\x20\x30\x20\x65\x6c\x73\x65\x20\x69\x6e\x74\x28\x78\x20\x2d\x20\x30\x2e\x35\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x78\x20\x3e\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x6e\x74\x28\x78\x20\x2a\x20\x31\x30\x2a\x2a\x6e\x64\x69\x67\x69\x74\x73\x20\x2b\x20\x30\x2e\x35\x29\x20\x2f\x20\x31\x30\x2a\x2a\x6e\x64\x69\x67\x69\x74\x73\x0a\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x6e\x74\x28\x78\x20\x2a\x20\x31\x30\x2a\x2a\x6e\x64\x69\x67\x69\x74\x73\x20\x2d\x20\x30\x2e\x35\x29\x20\x2f\x20\x31\x30\x2a\x2a\x6e\x64\x69\x67\x69\x74\x73\x0a\x0a\x64\x65\x66\x20\x61\x62\x73\x28\x78\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x2d\x78\x20\x69\x66\x20\x78\x20\x3c\x20\x30\x20\x65\x6c\x73\x65\x20\x78\x0a\x0a\x64\x65\x66\x20\x6d\x61\x78\x28\x2a\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x61\x72\x67\x73\x29\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x54\x79\x70\x65\x45\x72\x72\x6f\x72\x28\x27\x6d\x61\x78\x20\x65\x78\x70\x65\x63\x74\x65\x64\x20\x31\x20\x61\x72\x67\x75\x6d\x65\x6e\x74\x73\x2c\x20\x67\x6f\x74\x20\x30\x27\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x61\x72\x67\x73\x29\x20\x3d\x3d\x20\x31\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x72\x67\x73\x20\x3d\x20\x61\x72\x67\x73\x5b\x30\x5d\x0a\x20\x20\x20\x20\x61\x72\x67\x73\x20\x3d\x20\x69\x74\x65\x72\x28\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x6e\x65\x78\x74\x28\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x72\x65\x73\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x27\x6d\x61\x78\x28\x29\x20\x61\x72\x67\x20\x69\x73\x20\x61\x6e\x20\x65\x6d\x70\x74\x79\x20\x73\x65\x71\x75\x65\x6e\x63\x65\x27\x29\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x54\x72\x75\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x20\x3d\x20\x6e\x65\x78\x74\x28\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x72\x65\x61\x6b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x3e\x20\x72\x65\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x69\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x6d\x69\x6e\x28\x2a\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x61\x72\x67\x73\x29\x20\x3d\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x54\x79\x70\x65\x45\x72\x72\x6f\x72\x28\x27\x6d\x69\x6e\x20\x65\x78\x70\x65\x63\x74\x65\x64\x20\x31\x20\x61\x72\x67\x75\x6d\x65\x6e\x74\x73\x2c\x20\x67\x6f\x74\x20\x30\x27\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x61\x72\x67\x73\x29\x20\x3d\x3d\x20\x31\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x72\x67\x73\x20\x3d\x20\x61\x72\x67\x73\x5b\x30\x5d\x0a\x20\x20\x20\x20\x61\x72\x67\x73\x20\x3d\x20\x69\x74\x65\x72\x28\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x6e\x65\x78\x74\x28\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x72\x65\x73\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x56\x61\x6c\x75\x65\x45\x72\x72\x6f\x72\x28\x27\x6d\x69\x6e\x28\x29\x20\x61\x72\x67\x20\x69\x73\x20\x61\x6e\x20\x65\x6d\x70\x74\x79\x20\x73\x65\x71\x75\x65\x6e\x63\x65\x27\x29\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x54\x72\x75\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x20\x3d\x20\x6e\x65\x78\x74\x28\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x72\x65\x61\x6b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x3c\x20\x72\x65\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20" "\x20\x72\x65\x73\x20\x3d\x20\x69\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x61\x6c\x6c\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x69\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x54\x72\x75\x65\x0a\x0a\x64\x65\x66\x20\x61\x6e\x79\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x54\x72\x75\x65\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x0a\x64\x65\x66\x20\x65\x6e\x75\x6d\x65\x72\x61\x74\x65\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x2c\x20\x73\x74\x61\x72\x74\x3d\x30\x29\x3a\x0a\x20\x20\x20\x20\x6e\x20\x3d\x20\x73\x74\x61\x72\x74\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x65\x6c\x65\x6d\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x6e\x2c\x20\x65\x6c\x65\x6d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x2b\x2b\x6e\x0a\x0a\x64\x65\x66\x20\x73\x75\x6d\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x20\x2b\x3d\x20\x69\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x0a\x64\x65\x66\x20\x6d\x61\x70\x28\x66\x2c\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x66\x28\x69\x29\x0a\x0a\x64\x65\x66\x20\x66\x69\x6c\x74\x65\x72\x28\x66\x2c\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x66\x28\x69\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x69\x0a\x0a\x64\x65\x66\x20\x7a\x69\x70\x28\x61\x2c\x20\x62\x29\x3a\x0a\x20\x20\x20\x20\x61\x20\x3d\x20\x69\x74\x65\x72\x28\x61\x29\x0a\x20\x20\x20\x20\x62\x20\x3d\x20\x69\x74\x65\x72\x28\x62\x29\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x54\x72\x75\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x69\x20\x3d\x20\x6e\x65\x78\x74\x28\x61\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x62\x69\x20\x3d\x20\x6e\x65\x78\x74\x28\x62\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x61\x69\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x20\x6f\x72\x20\x62\x69\x20\x69\x73\x20\x53\x74\x6f\x70\x49\x74\x65\x72\x61\x74\x69\x6f\x6e\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x62\x72\x65\x61\x6b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x61\x69\x2c\x20\x62\x69\x0a\x0a\x64\x65\x66\x20\x72\x65\x76\x65\x72\x73\x65\x64\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x61\x20\x3d\x20\x6c\x69\x73\x74\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x0a\x20\x20\x20\x20\x61\x2e\x72\x65\x76\x65\x72\x73\x65\x28\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x0a\x0a\x64\x65\x66\x20\x73\x6f\x72\x74\x65\x64\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x2c\x20\x72\x65\x76\x65\x72\x73\x65\x3d\x46\x61\x6c\x73\x65\x2c\x20\x6b\x65\x79\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x61\x20\x3d\x20\x6c\x69\x73\x74\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x0a\x20\x20\x20\x20\x61\x2e\x73\x6f\x72\x74\x28\x72\x65\x76\x65\x72\x73\x65\x3d\x72\x65\x76\x65\x72\x73\x65\x2c\x20\x6b\x65\x79\x3d\x6b\x65\x79\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x0a\x0a\x23\x23\x23\x23\x23\x20\x73\x74\x72\x20\x23\x23\x23\x23\x23\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x73\x65\x70\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x66\x6c\x61\x67\x20\x3d\x20\x73\x65\x70\x20\x69\x73\x20\x4e\x6f\x6e\x65\x0a\x20\x20\x20\x20\x73\x65\x70\x20\x3d\x20\x73\x65\x70\x20\x6f\x72\x20\x27\x20\x27\x0a\x20\x20\x20\x20\x69\x66\x20\x73\x65\x70\x20\x3d\x3d\x20\x22\x22\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x69\x73\x74\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x69\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x69\x20\x3c\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x5b\x69\x3a\x69\x2b\x6c\x65\x6e\x28\x73\x65\x70\x29\x5d\x20\x3d\x3d\x20\x73\x65\x70\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x65\x6c\x66\x5b\x3a\x69\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x20\x3d\x20\x73\x65\x6c\x66\x5b\x69\x2b\x6c\x65\x6e\x28\x73\x65" "\x70\x29\x3a\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x2b\x2b\x69\x0a\x20\x20\x20\x20\x72\x65\x73\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x66\x6c\x61\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x69\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x65\x73\x20\x69\x66\x20\x69\x20\x21\x3d\x20\x27\x27\x5d\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x73\x74\x72\x2e\x73\x70\x6c\x69\x74\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x73\x3a\x20\x73\x74\x72\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x73\x29\x20\x69\x73\x20\x6e\x6f\x74\x20\x73\x74\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x69\x73\x65\x20\x54\x79\x70\x65\x45\x72\x72\x6f\x72\x28\x27\x6d\x75\x73\x74\x20\x62\x65\x20\x73\x74\x72\x2c\x20\x6e\x6f\x74\x20\x27\x20\x2b\x20\x74\x79\x70\x65\x28\x73\x29\x2e\x5f\x5f\x6e\x61\x6d\x65\x5f\x5f\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x73\x20\x3d\x3d\x20\x27\x27\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x2b\x20\x31\x0a\x20\x20\x20\x20\x72\x65\x73\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x69\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x69\x20\x3c\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x5b\x69\x3a\x69\x2b\x6c\x65\x6e\x28\x73\x29\x5d\x20\x3d\x3d\x20\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x2b\x2b\x72\x65\x73\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x20\x2b\x3d\x20\x6c\x65\x6e\x28\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x2b\x2b\x69\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x73\x0a\x73\x74\x72\x2e\x63\x6f\x75\x6e\x74\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x2a\x61\x72\x67\x73\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x27\x7b\x7d\x27\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x61\x72\x67\x73\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x20\x3d\x20\x73\x65\x6c\x66\x2e\x72\x65\x70\x6c\x61\x63\x65\x28\x27\x7b\x7d\x27\x2c\x20\x73\x74\x72\x28\x61\x72\x67\x73\x5b\x69\x5d\x29\x2c\x20\x31\x29\x0a\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x61\x72\x67\x73\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x20\x3d\x20\x73\x65\x6c\x66\x2e\x72\x65\x70\x6c\x61\x63\x65\x28\x27\x7b\x27\x2b\x73\x74\x72\x28\x69\x29\x2b\x27\x7d\x27\x2c\x20\x73\x74\x72\x28\x61\x72\x67\x73\x5b\x69\x5d\x29\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x0a\x73\x74\x72\x2e\x66\x6f\x72\x6d\x61\x74\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x63\x68\x61\x72\x73\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x63\x68\x61\x72\x73\x20\x3d\x20\x63\x68\x61\x72\x73\x20\x6f\x72\x20\x27\x20\x5c\x74\x5c\x6e\x5c\x72\x27\x0a\x20\x20\x20\x20\x69\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x69\x20\x3c\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x61\x6e\x64\x20\x73\x65\x6c\x66\x5b\x69\x5d\x20\x69\x6e\x20\x63\x68\x61\x72\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x2b\x2b\x69\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x5b\x69\x3a\x5d\x0a\x73\x74\x72\x2e\x6c\x73\x74\x72\x69\x70\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x63\x68\x61\x72\x73\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x63\x68\x61\x72\x73\x20\x3d\x20\x63\x68\x61\x72\x73\x20\x6f\x72\x20\x27\x20\x5c\x74\x5c\x6e\x5c\x72\x27\x0a\x20\x20\x20\x20\x6a\x20\x3d\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x2d\x20\x31\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6a\x20\x3e\x3d\x20\x30\x20\x61\x6e\x64\x20\x73\x65\x6c\x66\x5b\x6a\x5d\x20\x69\x6e\x20\x63\x68\x61\x72\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x2d\x2d\x6a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x5b\x3a\x6a\x2b\x31\x5d\x0a\x73\x74\x72\x2e\x72\x73\x74\x72\x69\x70\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x63\x68\x61\x72\x73\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x63\x68\x61\x72\x73\x20\x3d\x20\x63\x68\x61\x72\x73\x20\x6f\x72\x20\x27\x20\x5c\x74\x5c\x6e\x5c\x72\x27\x0a\x20\x20\x20\x20\x69\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x69\x20\x3c\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x61\x6e\x64\x20\x73\x65\x6c\x66\x5b\x69\x5d\x20\x69\x6e\x20\x63\x68\x61\x72\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x2b\x2b\x69\x0a\x20\x20\x20\x20\x6a\x20\x3d\x20\x6c" "\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x2d\x20\x31\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6a\x20\x3e\x3d\x20\x30\x20\x61\x6e\x64\x20\x73\x65\x6c\x66\x5b\x6a\x5d\x20\x69\x6e\x20\x63\x68\x61\x72\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x2d\x2d\x6a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x5b\x69\x3a\x6a\x2b\x31\x5d\x0a\x73\x74\x72\x2e\x73\x74\x72\x69\x70\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x77\x69\x64\x74\x68\x3a\x20\x69\x6e\x74\x29\x3a\x0a\x20\x20\x20\x20\x64\x65\x6c\x74\x61\x20\x3d\x20\x77\x69\x64\x74\x68\x20\x2d\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x64\x65\x6c\x74\x61\x20\x3c\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x30\x27\x20\x2a\x20\x64\x65\x6c\x74\x61\x20\x2b\x20\x73\x65\x6c\x66\x0a\x73\x74\x72\x2e\x7a\x66\x69\x6c\x6c\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x77\x69\x64\x74\x68\x3a\x20\x69\x6e\x74\x2c\x20\x66\x69\x6c\x6c\x63\x68\x61\x72\x3d\x27\x20\x27\x29\x3a\x0a\x20\x20\x20\x20\x64\x65\x6c\x74\x61\x20\x3d\x20\x77\x69\x64\x74\x68\x20\x2d\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x64\x65\x6c\x74\x61\x20\x3c\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x0a\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x6c\x65\x6e\x28\x66\x69\x6c\x6c\x63\x68\x61\x72\x29\x20\x3d\x3d\x20\x31\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x69\x6c\x6c\x63\x68\x61\x72\x20\x2a\x20\x64\x65\x6c\x74\x61\x20\x2b\x20\x73\x65\x6c\x66\x0a\x73\x74\x72\x2e\x72\x6a\x75\x73\x74\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x77\x69\x64\x74\x68\x3a\x20\x69\x6e\x74\x2c\x20\x66\x69\x6c\x6c\x63\x68\x61\x72\x3d\x27\x20\x27\x29\x3a\x0a\x20\x20\x20\x20\x64\x65\x6c\x74\x61\x20\x3d\x20\x77\x69\x64\x74\x68\x20\x2d\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x64\x65\x6c\x74\x61\x20\x3c\x3d\x20\x30\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x0a\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x6c\x65\x6e\x28\x66\x69\x6c\x6c\x63\x68\x61\x72\x29\x20\x3d\x3d\x20\x31\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x20\x2b\x20\x66\x69\x6c\x6c\x63\x68\x61\x72\x20\x2a\x20\x64\x65\x6c\x74\x61\x0a\x73\x74\x72\x2e\x6c\x6a\x75\x73\x74\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x23\x23\x23\x23\x23\x20\x6c\x69\x73\x74\x20\x23\x23\x23\x23\x23\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66\x3a\x20\x27\x5b\x27\x20\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x72\x65\x70\x72\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x5d\x29\x20\x2b\x20\x27\x5d\x27\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x6a\x73\x6f\x6e\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66\x3a\x20\x27\x5b\x27\x20\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x69\x2e\x5f\x5f\x6a\x73\x6f\x6e\x5f\x5f\x28\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x5d\x29\x20\x2b\x20\x27\x5d\x27\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x6a\x73\x6f\x6e\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66\x3a\x20\x27\x5b\x27\x20\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x69\x2e\x5f\x5f\x6a\x73\x6f\x6e\x5f\x5f\x28\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x5d\x29\x20\x2b\x20\x27\x5d\x27\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x3d\x3d\x20\x31\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x28\x27\x20\x2b\x20\x72\x65\x70\x72\x28\x73\x65\x6c\x66\x5b\x30\x5d\x29\x20\x2b\x20\x27\x2c\x29\x27\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x27\x28\x27\x20\x2b\x20\x27\x2c\x20\x27\x2e\x6a\x6f\x69\x6e\x28\x5b\x72\x65\x70\x72\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x73\x65\x6c\x66\x5d\x29\x20\x2b\x20\x27\x29\x27\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x71\x73\x6f\x72\x74\x28\x61\x3a\x20\x6c\x69\x73\x74\x2c\x20\x4c\x3a\x20\x69\x6e\x74\x2c\x20\x52\x3a\x20\x69\x6e\x74\x2c\x20\x6b\x65\x79\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x4c\x20\x3e\x3d\x20\x52\x3a\x20\x72\x65\x74\x75\x72\x6e\x3b\x0a\x20\x20\x20\x20\x6d\x69\x64\x20\x3d\x20\x61\x5b\x28\x52\x2b\x4c\x29\x2f\x2f\x32\x5d\x3b\x0a\x20\x20\x20\x20\x6d\x69\x64\x20\x3d\x20\x6b\x65\x79\x28\x6d\x69\x64\x29\x0a\x20\x20\x20\x20\x69\x2c\x20\x6a\x20\x3d\x20\x4c\x2c\x20\x52\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x69\x3c\x3d\x6a\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6b\x65\x79\x28\x61\x5b\x69\x5d\x29\x3c\x6d\x69\x64\x3a\x20\x2b\x2b\x69\x3b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6b\x65\x79\x28\x61\x5b\x6a\x5d\x29\x3e\x6d\x69\x64\x3a" "\x20\x2d\x2d\x6a\x3b\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x3c\x3d\x6a\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x5b\x69\x5d\x2c\x20\x61\x5b\x6a\x5d\x20\x3d\x20\x61\x5b\x6a\x5d\x2c\x20\x61\x5b\x69\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x2b\x2b\x69\x3b\x20\x2d\x2d\x6a\x3b\x0a\x20\x20\x20\x20\x5f\x5f\x71\x73\x6f\x72\x74\x28\x61\x2c\x20\x4c\x2c\x20\x6a\x2c\x20\x6b\x65\x79\x29\x0a\x20\x20\x20\x20\x5f\x5f\x71\x73\x6f\x72\x74\x28\x61\x2c\x20\x69\x2c\x20\x52\x2c\x20\x6b\x65\x79\x29\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x72\x65\x76\x65\x72\x73\x65\x3d\x46\x61\x6c\x73\x65\x2c\x20\x6b\x65\x79\x3d\x4e\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x6b\x65\x79\x20\x69\x73\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6b\x65\x79\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x78\x3a\x78\x0a\x20\x20\x20\x20\x5f\x5f\x71\x73\x6f\x72\x74\x28\x73\x65\x6c\x66\x2c\x20\x30\x2c\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x2d\x31\x2c\x20\x6b\x65\x79\x29\x0a\x20\x20\x20\x20\x69\x66\x20\x72\x65\x76\x65\x72\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x72\x65\x76\x65\x72\x73\x65\x28\x29\x0a\x6c\x69\x73\x74\x2e\x73\x6f\x72\x74\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x2c\x20\x6a\x20\x69\x6e\x20\x7a\x69\x70\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x21\x3d\x20\x6a\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x20\x3c\x20\x6a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x3c\x20\x6c\x65\x6e\x28\x6f\x74\x68\x65\x72\x29\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x6c\x74\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x6c\x74\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x2c\x20\x6a\x20\x69\x6e\x20\x7a\x69\x70\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x21\x3d\x20\x6a\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x20\x3e\x20\x6a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x3e\x20\x6c\x65\x6e\x28\x6f\x74\x68\x65\x72\x29\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x67\x74\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x67\x74\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x2c\x20\x6a\x20\x69\x6e\x20\x7a\x69\x70\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x21\x3d\x20\x6a\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x20\x3c\x3d\x20\x6a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x3c\x3d\x20\x6c\x65\x6e\x28\x6f\x74\x68\x65\x72\x29\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x6c\x65\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x6c\x65\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x64\x65\x66\x20\x5f\x5f\x66\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x2c\x20\x6a\x20\x69\x6e\x20\x7a\x69\x70\x28\x73\x65\x6c\x66\x2c\x20\x6f\x74\x68\x65\x72\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x20\x21\x3d\x20\x6a\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x20\x3e\x3d\x20\x6a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x3e\x3d\x20\x6c\x65\x6e\x28\x6f\x74\x68\x65\x72\x29\x0a\x74\x75\x70\x6c\x65\x2e\x5f\x5f\x67\x65\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x6c\x69\x73\x74\x2e\x5f\x5f\x67\x65\x5f\x5f\x20\x3d\x20\x5f\x5f\x66\x0a\x0a\x74\x79\x70\x65\x2e\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x20\x3d\x20\x6c\x61\x6d\x62\x64\x61\x20\x73\x65\x6c\x66\x3a\x20\x22\x3c\x63\x6c\x61\x73\x73\x20\x27\x22\x20\x2b\x20\x73\x65\x6c\x66\x2e\x5f\x5f\x6e\x61\x6d\x65\x5f\x5f\x20\x2b\x20\x22\x27\x3e\x22\x0a\x0a\x64\x65\x66\x20\x68\x65\x6c\x70\x28\x6f\x62\x6a\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x68\x61\x73\x61\x74\x74\x72\x28\x6f\x62\x6a\x2c\x20\x27\x5f\x5f\x66\x75\x6e\x63\x5f\x5f\x27\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x62\x6a\x20\x3d\x20\x6f\x62\x6a\x2e\x5f\x5f\x66\x75\x6e\x63\x5f\x5f\x0a\x20\x20\x20\x20\x70\x72\x69\x6e\x74\x28\x6f\x62\x6a\x2e\x5f\x5f\x73\x69\x67\x6e\x61\x74\x75\x72\x65\x5f\x5f\x29\x0a\x20\x20\x20\x20\x70\x72\x69\x6e\x74\x28\x6f\x62\x6a\x2e\x5f\x5f\x64\x6f\x63\x5f\x5f\x29\x0a\x0a\x64\x65\x6c\x20\x5f\x5f\x66\x0a\x0a\x66\x72\x6f\x6d\x20\x5f\x6c\x6f\x6e\x67\x20\x69\x6d\x70\x6f\x72\x74\x20\x6c\x6f\x6e\x67" },
        {"this", "\x70\x72\x69\x6e\x74\x28\x22\x22\x22\x54\x68\x65\x20\x5a\x65\x6e\x20\x6f\x66\x20\x50\x79\x74\x68\x6f\x6e\x2c\x20\x62\x79\x20\x54\x69\x6d\x20\x50\x65\x74\x65\x72\x73\x0a\x0a\x42\x65\x61\x75\x74\x69\x66\x75\x6c\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x75\x67\x6c\x79\x2e\x0a\x45\x78\x70\x6c\x69\x63\x69\x74\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x69\x6d\x70\x6c\x69\x63\x69\x74\x2e\x0a\x53\x69\x6d\x70\x6c\x65\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x63\x6f\x6d\x70\x6c\x65\x78\x2e\x0a\x43\x6f\x6d\x70\x6c\x65\x78\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x63\x6f\x6d\x70\x6c\x69\x63\x61\x74\x65\x64\x2e\x0a\x46\x6c\x61\x74\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x6e\x65\x73\x74\x65\x64\x2e\x0a\x53\x70\x61\x72\x73\x65\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x64\x65\x6e\x73\x65\x2e\x0a\x52\x65\x61\x64\x61\x62\x69\x6c\x69\x74\x79\x20\x63\x6f\x75\x6e\x74\x73\x2e\x0a\x53\x70\x65\x63\x69\x61\x6c\x20\x63\x61\x73\x65\x73\x20\x61\x72\x65\x6e\x27\x74\x20\x73\x70\x65\x63\x69\x61\x6c\x20\x65\x6e\x6f\x75\x67\x68\x20\x74\x6f\x20\x62\x72\x65\x61\x6b\x20\x74\x68\x65\x20\x72\x75\x6c\x65\x73\x2e\x0a\x41\x6c\x74\x68\x6f\x75\x67\x68\x20\x70\x72\x61\x63\x74\x69\x63\x61\x6c\x69\x74\x79\x20\x62\x65\x61\x74\x73\x20\x70\x75\x72\x69\x74\x79\x2e\x0a\x45\x72\x72\x6f\x72\x73\x20\x73\x68\x6f\x75\x6c\x64\x20\x6e\x65\x76\x65\x72\x20\x70\x61\x73\x73\x20\x73\x69\x6c\x65\x6e\x74\x6c\x79\x2e\x0a\x55\x6e\x6c\x65\x73\x73\x20\x65\x78\x70\x6c\x69\x63\x69\x74\x6c\x79\x20\x73\x69\x6c\x65\x6e\x63\x65\x64\x2e\x0a\x49\x6e\x20\x74\x68\x65\x20\x66\x61\x63\x65\x20\x6f\x66\x20\x61\x6d\x62\x69\x67\x75\x69\x74\x79\x2c\x20\x72\x65\x66\x75\x73\x65\x20\x74\x68\x65\x20\x74\x65\x6d\x70\x74\x61\x74\x69\x6f\x6e\x20\x74\x6f\x20\x67\x75\x65\x73\x73\x2e\x0a\x54\x68\x65\x72\x65\x20\x73\x68\x6f\x75\x6c\x64\x20\x62\x65\x20\x6f\x6e\x65\x2d\x2d\x20\x61\x6e\x64\x20\x70\x72\x65\x66\x65\x72\x61\x62\x6c\x79\x20\x6f\x6e\x6c\x79\x20\x6f\x6e\x65\x20\x2d\x2d\x6f\x62\x76\x69\x6f\x75\x73\x20\x77\x61\x79\x20\x74\x6f\x20\x64\x6f\x20\x69\x74\x2e\x0a\x41\x6c\x74\x68\x6f\x75\x67\x68\x20\x74\x68\x61\x74\x20\x77\x61\x79\x20\x6d\x61\x79\x20\x6e\x6f\x74\x20\x62\x65\x20\x6f\x62\x76\x69\x6f\x75\x73\x20\x61\x74\x20\x66\x69\x72\x73\x74\x20\x75\x6e\x6c\x65\x73\x73\x20\x79\x6f\x75\x27\x72\x65\x20\x44\x75\x74\x63\x68\x2e\x0a\x4e\x6f\x77\x20\x69\x73\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x6e\x65\x76\x65\x72\x2e\x0a\x41\x6c\x74\x68\x6f\x75\x67\x68\x20\x6e\x65\x76\x65\x72\x20\x69\x73\x20\x6f\x66\x74\x65\x6e\x20\x62\x65\x74\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x2a\x72\x69\x67\x68\x74\x2a\x20\x6e\x6f\x77\x2e\x0a\x49\x66\x20\x74\x68\x65\x20\x69\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x61\x74\x69\x6f\x6e\x20\x69\x73\x20\x68\x61\x72\x64\x20\x74\x6f\x20\x65\x78\x70\x6c\x61\x69\x6e\x2c\x20\x69\x74\x27\x73\x20\x61\x20\x62\x61\x64\x20\x69\x64\x65\x61\x2e\x0a\x49\x66\x20\x74\x68\x65\x20\x69\x6d\x70\x6c\x65\x6d\x65\x6e\x74\x61\x74\x69\x6f\x6e\x20\x69\x73\x20\x65\x61\x73\x79\x20\x74\x6f\x20\x65\x78\x70\x6c\x61\x69\x6e\x2c\x20\x69\x74\x20\x6d\x61\x79\x20\x62\x65\x20\x61\x20\x67\x6f\x6f\x64\x20\x69\x64\x65\x61\x2e\x0a\x4e\x61\x6d\x65\x73\x70\x61\x63\x65\x73\x20\x61\x72\x65\x20\x6f\x6e\x65\x20\x68\x6f\x6e\x6b\x69\x6e\x67\x20\x67\x72\x65\x61\x74\x20\x69\x64\x65\x61\x20\x2d\x2d\x20\x6c\x65\x74\x27\x73\x20\x64\x6f\x20\x6d\x6f\x72\x65\x20\x6f\x66\x20\x74\x68\x6f\x73\x65\x21\x22\x22\x22\x29" },
        {"random", "\x5f\x69\x6e\x73\x74\x20\x3d\x20\x52\x61\x6e\x64\x6f\x6d\x28\x29\x0a\x0a\x73\x65\x65\x64\x20\x3d\x20\x5f\x69\x6e\x73\x74\x2e\x73\x65\x65\x64\x0a\x72\x61\x6e\x64\x6f\x6d\x20\x3d\x20\x5f\x69\x6e\x73\x74\x2e\x72\x61\x6e\x64\x6f\x6d\x0a\x75\x6e\x69\x66\x6f\x72\x6d\x20\x3d\x20\x5f\x69\x6e\x73\x74\x2e\x75\x6e\x69\x66\x6f\x72\x6d\x0a\x72\x61\x6e\x64\x69\x6e\x74\x20\x3d\x20\x5f\x69\x6e\x73\x74\x2e\x72\x61\x6e\x64\x69\x6e\x74\x0a\x0a\x64\x65\x66\x20\x73\x68\x75\x66\x66\x6c\x65\x28\x4c\x29\x3a\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x4c\x29\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6a\x20\x3d\x20\x72\x61\x6e\x64\x69\x6e\x74\x28\x69\x2c\x20\x6c\x65\x6e\x28\x4c\x29\x20\x2d\x20\x31\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x4c\x5b\x69\x5d\x2c\x20\x4c\x5b\x6a\x5d\x20\x3d\x20\x4c\x5b\x6a\x5d\x2c\x20\x4c\x5b\x69\x5d\x0a\x0a\x64\x65\x66\x20\x63\x68\x6f\x69\x63\x65\x28\x4c\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x4c\x5b\x72\x61\x6e\x64\x69\x6e\x74\x28\x30\x2c\x20\x6c\x65\x6e\x28\x4c\x29\x20\x2d\x20\x31\x29\x5d" },
        {"collections", "\x63\x6c\x61\x73\x73\x20\x5f\x4c\x69\x6e\x6b\x65\x64\x4c\x69\x73\x74\x4e\x6f\x64\x65\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x70\x72\x65\x76\x2c\x20\x6e\x65\x78\x74\x2c\x20\x76\x61\x6c\x75\x65\x29\x20\x2d\x3e\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x70\x72\x65\x76\x20\x3d\x20\x70\x72\x65\x76\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x6e\x65\x78\x74\x20\x3d\x20\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x76\x61\x6c\x75\x65\x20\x3d\x20\x76\x61\x6c\x75\x65\x0a\x0a\x63\x6c\x61\x73\x73\x20\x64\x65\x71\x75\x65\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3d\x4e\x6f\x6e\x65\x29\x20\x2d\x3e\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x20\x3d\x20\x5f\x4c\x69\x6e\x6b\x65\x64\x4c\x69\x73\x74\x4e\x6f\x64\x65\x28\x4e\x6f\x6e\x65\x2c\x20\x4e\x6f\x6e\x65\x2c\x20\x4e\x6f\x6e\x65\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x20\x3d\x20\x5f\x4c\x69\x6e\x6b\x65\x64\x4c\x69\x73\x74\x4e\x6f\x64\x65\x28\x4e\x6f\x6e\x65\x2c\x20\x4e\x6f\x6e\x65\x2c\x20\x4e\x6f\x6e\x65\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x20\x3d\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x2e\x70\x72\x65\x76\x20\x3d\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x3d\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x76\x61\x6c\x75\x65\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x61\x70\x70\x65\x6e\x64\x28\x76\x61\x6c\x75\x65\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x65\x74\x69\x74\x65\x6d\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x69\x6e\x64\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x30\x20\x3c\x3d\x20\x69\x6e\x64\x65\x78\x20\x3c\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x5f\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x69\x6e\x64\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x6e\x6f\x64\x65\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6e\x6f\x64\x65\x2e\x76\x61\x6c\x75\x65\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x73\x65\x74\x69\x74\x65\x6d\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x69\x6e\x64\x65\x78\x2c\x20\x76\x61\x6c\x75\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x30\x20\x3c\x3d\x20\x69\x6e\x64\x65\x78\x20\x3c\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x5f\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x69\x6e\x64\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x6e\x6f\x64\x65\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x2e\x76\x61\x6c\x75\x65\x20\x3d\x20\x76\x61\x6c\x75\x65\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x64\x65\x6c\x69\x74\x65\x6d\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x69\x6e\x64\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x30\x20\x3c\x3d\x20\x69\x6e\x64\x65\x78\x20\x3c\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x5f\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x69\x6e\x64\x65\x78\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x6e\x6f\x64\x65\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x2e\x70\x72\x65\x76\x2e\x6e\x65\x78\x74\x20\x3d\x20\x6e\x6f\x64\x65\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x2e\x6e\x65\x78\x74\x2e\x70\x72\x65\x76\x20\x3d\x20\x6e\x6f\x64\x65\x2e\x70\x72\x65\x76\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x2d\x3d\x20\x31\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6c\x65\x61\x72\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x20\x3d\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x2e\x70\x72\x65\x76\x20\x3d\x20\x73\x65\x6c\x66\x2e\x68\x65" "\x61\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x3d\x20\x30\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x65\x78\x74\x65\x6e\x64\x28\x73\x65\x6c\x66\x2c\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x76\x61\x6c\x75\x65\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x61\x70\x70\x65\x6e\x64\x28\x76\x61\x6c\x75\x65\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x61\x70\x70\x65\x6e\x64\x28\x73\x65\x6c\x66\x2c\x20\x76\x61\x6c\x75\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x5f\x4c\x69\x6e\x6b\x65\x64\x4c\x69\x73\x74\x4e\x6f\x64\x65\x28\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x2e\x70\x72\x65\x76\x2c\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x2c\x20\x76\x61\x6c\x75\x65\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x2e\x70\x72\x65\x76\x2e\x6e\x65\x78\x74\x20\x3d\x20\x6e\x6f\x64\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x2e\x70\x72\x65\x76\x20\x3d\x20\x6e\x6f\x64\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x2b\x3d\x20\x31\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x61\x70\x70\x65\x6e\x64\x6c\x65\x66\x74\x28\x73\x65\x6c\x66\x2c\x20\x76\x61\x6c\x75\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x5f\x4c\x69\x6e\x6b\x65\x64\x4c\x69\x73\x74\x4e\x6f\x64\x65\x28\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2c\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x2c\x20\x76\x61\x6c\x75\x65\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x2e\x70\x72\x65\x76\x20\x3d\x20\x6e\x6f\x64\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x20\x3d\x20\x6e\x6f\x64\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x2b\x3d\x20\x31\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x70\x6f\x70\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x3e\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x2e\x70\x72\x65\x76\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x2e\x70\x72\x65\x76\x2e\x6e\x65\x78\x74\x20\x3d\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x2e\x70\x72\x65\x76\x20\x3d\x20\x6e\x6f\x64\x65\x2e\x70\x72\x65\x76\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x2d\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6e\x6f\x64\x65\x2e\x76\x61\x6c\x75\x65\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x70\x6f\x70\x6c\x65\x66\x74\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x3e\x20\x30\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x2e\x6e\x65\x78\x74\x2e\x70\x72\x65\x76\x20\x3d\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x20\x3d\x20\x6e\x6f\x64\x65\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x20\x2d\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6e\x6f\x64\x65\x2e\x76\x61\x6c\x75\x65\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x63\x6f\x70\x79\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x65\x77\x5f\x6c\x69\x73\x74\x20\x3d\x20\x64\x65\x71\x75\x65\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x76\x61\x6c\x75\x65\x20\x69\x6e\x20\x73\x65\x6c\x66\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x65\x77\x5f\x6c\x69\x73\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x76\x61\x6c\x75\x65\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6e\x65\x77\x5f\x6c\x69\x73\x74\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x73\x69\x7a\x65\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x74\x65\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x6e\x6f\x64\x65\x20\x69\x73\x20\x6e\x6f\x74\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x79\x69\x65\x6c\x64\x20\x6e\x6f\x64\x65\x2e\x76\x61\x6c\x75\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x6f\x64\x65\x20\x3d\x20\x6e\x6f\x64\x65\x2e\x6e\x65\x78\x74\x0a\x0a\x20\x20\x20\x20\x64\x65" "\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x20\x2d\x3e\x20\x73\x74\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x20\x3d\x20\x6c\x69\x73\x74\x28\x73\x65\x6c\x66\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x22\x64\x65\x71\x75\x65\x28\x7b\x61\x7d\x29\x22\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x5f\x5f\x6f\x3a\x20\x6f\x62\x6a\x65\x63\x74\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x69\x73\x69\x6e\x73\x74\x61\x6e\x63\x65\x28\x5f\x5f\x6f\x2c\x20\x64\x65\x71\x75\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x29\x20\x21\x3d\x20\x6c\x65\x6e\x28\x5f\x5f\x6f\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x74\x31\x2c\x20\x74\x32\x20\x3d\x20\x73\x65\x6c\x66\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x2c\x20\x5f\x5f\x6f\x2e\x68\x65\x61\x64\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x74\x31\x20\x69\x73\x20\x6e\x6f\x74\x20\x73\x65\x6c\x66\x2e\x74\x61\x69\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x31\x2e\x76\x61\x6c\x75\x65\x20\x21\x3d\x20\x74\x32\x2e\x76\x61\x6c\x75\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x74\x31\x2c\x20\x74\x32\x20\x3d\x20\x74\x31\x2e\x6e\x65\x78\x74\x2c\x20\x74\x32\x2e\x6e\x65\x78\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x54\x72\x75\x65\x0a\x0a\x64\x65\x66\x20\x43\x6f\x75\x6e\x74\x65\x72\x28\x69\x74\x65\x72\x61\x62\x6c\x65\x29\x3a\x0a\x20\x20\x20\x20\x61\x20\x3d\x20\x7b\x7d\x0a\x20\x20\x20\x20\x66\x6f\x72\x20\x78\x20\x69\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x78\x20\x69\x6e\x20\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x5b\x78\x5d\x20\x2b\x3d\x20\x31\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x5b\x78\x5d\x20\x3d\x20\x31\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x61\x0a\x0a\x63\x6c\x61\x73\x73\x20\x64\x65\x66\x61\x75\x6c\x74\x64\x69\x63\x74\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x29\x20\x2d\x3e\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x20\x3d\x20\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x20\x3d\x20\x7b\x7d\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x67\x65\x74\x69\x74\x65\x6d\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6b\x65\x79\x20\x6e\x6f\x74\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x6b\x65\x79\x5d\x20\x3d\x20\x73\x65\x6c\x66\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x6b\x65\x79\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x73\x65\x74\x69\x74\x65\x6d\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x2c\x20\x76\x61\x6c\x75\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x61\x5b\x6b\x65\x79\x5d\x20\x3d\x20\x76\x61\x6c\x75\x65\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x72\x65\x70\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x20\x2d\x3e\x20\x73\x74\x72\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x66\x22\x64\x65\x66\x61\x75\x6c\x74\x64\x69\x63\x74\x28\x7b\x73\x65\x6c\x66\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x7d\x2c\x20\x7b\x73\x65\x6c\x66\x2e\x5f\x61\x7d\x29\x22\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x65\x71\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x5f\x5f\x6f\x3a\x20\x6f\x62\x6a\x65\x63\x74\x29\x20\x2d\x3e\x20\x62\x6f\x6f\x6c\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x6f\x74\x20\x69\x73\x69\x6e\x73\x74\x61\x6e\x63\x65\x28\x5f\x5f\x6f\x2c\x20\x64\x65\x66\x61\x75\x6c\x74\x64\x69\x63\x74\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c\x73\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x20\x21\x3d\x20\x5f\x5f\x6f\x2e\x64\x65\x66\x61\x75\x6c\x74\x5f\x66\x61\x63\x74\x6f\x72\x79\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x46\x61\x6c" "\x73\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x20\x3d\x3d\x20\x5f\x5f\x6f\x2e\x5f\x61\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x74\x65\x72\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x74\x65\x72\x28\x73\x65\x6c\x66\x2e\x5f\x61\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x63\x6f\x6e\x74\x61\x69\x6e\x73\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6b\x65\x79\x20\x69\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x6c\x65\x6e\x5f\x5f\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x2e\x5f\x61\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x6b\x65\x79\x73\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x6b\x65\x79\x73\x28\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x76\x61\x6c\x75\x65\x73\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x76\x61\x6c\x75\x65\x73\x28\x29\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x69\x74\x65\x6d\x73\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x69\x74\x65\x6d\x73\x28\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x70\x6f\x70\x28\x73\x65\x6c\x66\x2c\x20\x6b\x65\x79\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x61\x2e\x70\x6f\x70\x28\x6b\x65\x79\x29\x0a" },
        {"pickle", "\x69\x6d\x70\x6f\x72\x74\x20\x6a\x73\x6f\x6e\x0a\x69\x6d\x70\x6f\x72\x74\x20\x62\x75\x69\x6c\x74\x69\x6e\x73\x0a\x0a\x5f\x42\x41\x53\x49\x43\x5f\x54\x59\x50\x45\x53\x20\x3d\x20\x5b\x69\x6e\x74\x2c\x20\x66\x6c\x6f\x61\x74\x2c\x20\x73\x74\x72\x2c\x20\x62\x6f\x6f\x6c\x2c\x20\x74\x79\x70\x65\x28\x4e\x6f\x6e\x65\x29\x5d\x0a\x0a\x64\x65\x66\x20\x5f\x66\x69\x6e\x64\x5f\x63\x6c\x61\x73\x73\x28\x70\x61\x74\x68\x3a\x20\x73\x74\x72\x29\x3a\x0a\x20\x20\x20\x20\x69\x66\x20\x22\x2e\x22\x20\x6e\x6f\x74\x20\x69\x6e\x20\x70\x61\x74\x68\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x67\x20\x3d\x20\x67\x6c\x6f\x62\x61\x6c\x73\x28\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x70\x61\x74\x68\x20\x69\x6e\x20\x67\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x67\x5b\x70\x61\x74\x68\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x62\x75\x69\x6c\x74\x69\x6e\x73\x2e\x5f\x5f\x64\x69\x63\x74\x5f\x5f\x5b\x70\x61\x74\x68\x5d\x0a\x20\x20\x20\x20\x6d\x6f\x64\x6e\x61\x6d\x65\x2c\x20\x6e\x61\x6d\x65\x20\x3d\x20\x70\x61\x74\x68\x2e\x73\x70\x6c\x69\x74\x28\x22\x2e\x22\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x5f\x69\x6d\x70\x6f\x72\x74\x5f\x5f\x28\x6d\x6f\x64\x6e\x61\x6d\x65\x29\x2e\x5f\x5f\x64\x69\x63\x74\x5f\x5f\x5b\x6e\x61\x6d\x65\x5d\x0a\x0a\x64\x65\x66\x20\x5f\x66\x69\x6e\x64\x5f\x5f\x6e\x65\x77\x5f\x5f\x28\x63\x6c\x73\x29\x3a\x0a\x20\x20\x20\x20\x77\x68\x69\x6c\x65\x20\x63\x6c\x73\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x64\x20\x3d\x20\x63\x6c\x73\x2e\x5f\x5f\x64\x69\x63\x74\x5f\x5f\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x22\x5f\x5f\x6e\x65\x77\x5f\x5f\x22\x20\x69\x6e\x20\x64\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x64\x5b\x22\x5f\x5f\x6e\x65\x77\x5f\x5f\x22\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6c\x73\x20\x3d\x20\x63\x6c\x73\x2e\x5f\x5f\x62\x61\x73\x65\x5f\x5f\x0a\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x46\x61\x6c\x73\x65\x0a\x0a\x63\x6c\x61\x73\x73\x20\x5f\x50\x69\x63\x6b\x6c\x65\x72\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x62\x6a\x29\x20\x2d\x3e\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x6f\x62\x6a\x20\x3d\x20\x6f\x62\x6a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x72\x61\x77\x5f\x6d\x65\x6d\x6f\x20\x3d\x20\x7b\x7d\x20\x20\x23\x20\x69\x64\x20\x2d\x3e\x20\x69\x6e\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x6d\x65\x6d\x6f\x20\x3d\x20\x5b\x5d\x20\x20\x20\x20\x20\x20\x23\x20\x69\x6e\x74\x20\x2d\x3e\x20\x6f\x62\x6a\x65\x63\x74\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x77\x72\x61\x70\x28\x73\x65\x6c\x66\x2c\x20\x6f\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x29\x20\x69\x6e\x20\x5f\x42\x41\x53\x49\x43\x5f\x54\x59\x50\x45\x53\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6f\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x29\x20\x69\x73\x20\x74\x79\x70\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x22\x74\x79\x70\x65\x22\x2c\x20\x6f\x2e\x5f\x5f\x6e\x61\x6d\x65\x5f\x5f\x5d\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x6e\x64\x65\x78\x20\x3d\x20\x73\x65\x6c\x66\x2e\x72\x61\x77\x5f\x6d\x65\x6d\x6f\x2e\x67\x65\x74\x28\x69\x64\x28\x6f\x29\x2c\x20\x4e\x6f\x6e\x65\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x69\x6e\x64\x65\x78\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x6e\x64\x65\x78\x20\x3d\x20\x6c\x65\x6e\x28\x73\x65\x6c\x66\x2e\x6d\x65\x6d\x6f\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x6d\x65\x6d\x6f\x2e\x61\x70\x70\x65\x6e\x64\x28\x72\x65\x74\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x72\x61\x77\x5f\x6d\x65\x6d\x6f\x5b\x69\x64\x28\x6f\x29\x5d\x20\x3d\x20\x69\x6e\x64\x65\x78\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x29\x20\x69\x73\x20\x74\x75\x70\x6c\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x22\x74\x75\x70\x6c\x65\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x5b\x73\x65\x6c\x66\x2e\x77\x72\x61\x70\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x6f\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x29\x20\x69\x73\x20\x62\x79\x74\x65\x73\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x22\x62\x79\x74\x65\x73\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20" "\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x5b\x6f\x5b\x6a\x5d\x20\x66\x6f\x72\x20\x6a\x20\x69\x6e\x20\x72\x61\x6e\x67\x65\x28\x6c\x65\x6e\x28\x6f\x29\x29\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x29\x20\x69\x73\x20\x6c\x69\x73\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x22\x6c\x69\x73\x74\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x5b\x73\x65\x6c\x66\x2e\x77\x72\x61\x70\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x6f\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x29\x20\x69\x73\x20\x64\x69\x63\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x22\x64\x69\x63\x74\x22\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x5b\x5b\x73\x65\x6c\x66\x2e\x77\x72\x61\x70\x28\x6b\x29\x2c\x20\x73\x65\x6c\x66\x2e\x77\x72\x61\x70\x28\x76\x29\x5d\x20\x66\x6f\x72\x20\x6b\x2c\x76\x20\x69\x6e\x20\x6f\x2e\x69\x74\x65\x6d\x73\x28\x29\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x30\x20\x3d\x20\x6f\x2e\x5f\x5f\x63\x6c\x61\x73\x73\x5f\x5f\x2e\x5f\x5f\x6e\x61\x6d\x65\x5f\x5f\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x68\x61\x73\x61\x74\x74\x72\x28\x6f\x2c\x20\x22\x5f\x5f\x67\x65\x74\x6e\x65\x77\x61\x72\x67\x73\x5f\x5f\x22\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x31\x20\x3d\x20\x6f\x2e\x5f\x5f\x67\x65\x74\x6e\x65\x77\x61\x72\x67\x73\x5f\x5f\x28\x29\x20\x20\x20\x20\x20\x23\x20\x61\x6e\x20\x69\x74\x65\x72\x61\x62\x6c\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x31\x20\x3d\x20\x5b\x73\x65\x6c\x66\x2e\x77\x72\x61\x70\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x5f\x31\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x31\x20\x3d\x20\x4e\x6f\x6e\x65\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x2e\x5f\x5f\x64\x69\x63\x74\x5f\x5f\x20\x69\x73\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x32\x20\x3d\x20\x4e\x6f\x6e\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x32\x20\x3d\x20\x7b\x7d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x6b\x2c\x76\x20\x69\x6e\x20\x6f\x2e\x5f\x5f\x64\x69\x63\x74\x5f\x5f\x2e\x69\x74\x65\x6d\x73\x28\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x5f\x32\x5b\x6b\x5d\x20\x3d\x20\x73\x65\x6c\x66\x2e\x77\x72\x61\x70\x28\x76\x29\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x5f\x30\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x5f\x31\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x5f\x32\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x72\x75\x6e\x5f\x70\x69\x70\x65\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x20\x3d\x20\x73\x65\x6c\x66\x2e\x77\x72\x61\x70\x28\x73\x65\x6c\x66\x2e\x6f\x62\x6a\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5b\x6f\x2c\x20\x73\x65\x6c\x66\x2e\x6d\x65\x6d\x6f\x5d\x0a\x0a\x0a\x0a\x63\x6c\x61\x73\x73\x20\x5f\x55\x6e\x70\x69\x63\x6b\x6c\x65\x72\x3a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x5f\x5f\x69\x6e\x69\x74\x5f\x5f\x28\x73\x65\x6c\x66\x2c\x20\x6f\x62\x6a\x2c\x20\x6d\x65\x6d\x6f\x3a\x20\x6c\x69\x73\x74\x29\x20\x2d\x3e\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x6f\x62\x6a\x20\x3d\x20\x6f\x62\x6a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x6d\x65\x6d\x6f\x20\x3d\x20\x6d\x65\x6d\x6f\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x75\x6e\x77\x72\x61\x70\x70\x65\x64\x20\x3d\x20\x5b\x4e\x6f\x6e\x65\x5d\x20\x2a\x20\x6c\x65\x6e\x28\x6d\x65\x6d\x6f\x29\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x74\x61\x67\x28\x73\x65\x6c\x66\x2c\x20\x69\x6e\x64\x65\x78\x2c\x20\x6f\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x73\x65\x6c\x66\x2e\x5f\x75\x6e\x77\x72\x61\x70\x70\x65\x64\x5b\x69\x6e\x64\x65\x78\x5d\x20\x69\x73\x20\x4e\x6f\x6e\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x5f\x75\x6e\x77\x72\x61\x70\x70\x65\x64\x5b\x69\x6e\x64\x65\x78\x5d\x20\x3d\x20\x6f\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x75\x6e\x77\x72\x61\x70\x28\x73\x65\x6c\x66\x2c\x20\x6f\x2c\x20\x69\x6e\x64\x65\x78\x3d\x4e" "\x6f\x6e\x65\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x29\x20\x69\x6e\x20\x5f\x42\x41\x53\x49\x43\x5f\x54\x59\x50\x45\x53\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6f\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x6f\x29\x20\x69\x73\x20\x6c\x69\x73\x74\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x5b\x30\x5d\x20\x3d\x3d\x20\x22\x74\x79\x70\x65\x22\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x66\x69\x6e\x64\x5f\x63\x6c\x61\x73\x73\x28\x6f\x5b\x31\x5d\x29\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x72\x65\x66\x65\x72\x65\x6e\x63\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x74\x79\x70\x65\x28\x6f\x5b\x30\x5d\x29\x20\x69\x73\x20\x69\x6e\x74\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x69\x6e\x64\x65\x78\x20\x69\x73\x20\x4e\x6f\x6e\x65\x20\x20\x20\x20\x23\x20\x69\x6e\x64\x65\x78\x20\x73\x68\x6f\x75\x6c\x64\x20\x62\x65\x20\x4e\x6f\x6e\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x6e\x64\x65\x78\x20\x3d\x20\x6f\x5b\x30\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x65\x6c\x66\x2e\x5f\x75\x6e\x77\x72\x61\x70\x70\x65\x64\x5b\x69\x6e\x64\x65\x78\x5d\x20\x69\x73\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6f\x20\x3d\x20\x73\x65\x6c\x66\x2e\x6d\x65\x6d\x6f\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x6f\x29\x20\x69\x73\x20\x6c\x69\x73\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x6f\x5b\x30\x5d\x29\x20\x69\x73\x20\x73\x74\x72\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x6f\x2c\x20\x69\x6e\x64\x65\x78\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x73\x65\x6c\x66\x2e\x5f\x75\x6e\x77\x72\x61\x70\x70\x65\x64\x5b\x69\x6e\x64\x65\x78\x5d\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x5f\x75\x6e\x77\x72\x61\x70\x70\x65\x64\x5b\x69\x6e\x64\x65\x78\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x63\x6f\x6e\x63\x72\x65\x74\x65\x20\x72\x65\x66\x65\x72\x65\x6e\x63\x65\x20\x74\x79\x70\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x5b\x30\x5d\x20\x3d\x3d\x20\x22\x74\x75\x70\x6c\x65\x22\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x74\x75\x70\x6c\x65\x28\x5b\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x6f\x5b\x31\x5d\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x67\x28\x69\x6e\x64\x65\x78\x2c\x20\x72\x65\x74\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x5b\x30\x5d\x20\x3d\x3d\x20\x22\x62\x79\x74\x65\x73\x22\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x62\x79\x74\x65\x73\x28\x6f\x5b\x31\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x67\x28\x69\x6e\x64\x65\x78\x2c\x20\x72\x65\x74\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x5b\x30\x5d\x20\x3d\x3d\x20\x22\x6c\x69\x73\x74\x22\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x5b\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x67\x28\x69\x6e\x64\x65\x78\x2c\x20\x72\x65\x74\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x6f\x5b\x31\x5d\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x2e\x61\x70\x70\x65\x6e\x64\x28\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x69\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x72\x65\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6f\x5b\x30\x5d\x20\x3d\x3d\x20\x22\x64\x69\x63\x74\x22\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x20\x3d\x20\x7b\x7d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x67\x28\x69\x6e\x64\x65\x78\x2c\x20\x72\x65\x74\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x6b\x2c\x76\x20\x69\x6e\x20\x6f\x5b\x31\x5d\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x5b\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x6b\x29\x5d\x20\x3d\x20\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x76\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20" "\x72\x65\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x67\x65\x6e\x65\x72\x69\x63\x20\x6f\x62\x6a\x65\x63\x74\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6c\x73\x2c\x20\x6e\x65\x77\x61\x72\x67\x73\x2c\x20\x73\x74\x61\x74\x65\x20\x3d\x20\x6f\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6c\x73\x20\x3d\x20\x5f\x66\x69\x6e\x64\x5f\x63\x6c\x61\x73\x73\x28\x6f\x5b\x30\x5d\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x63\x72\x65\x61\x74\x65\x20\x75\x6e\x69\x6e\x69\x74\x69\x61\x6c\x69\x7a\x65\x64\x20\x69\x6e\x73\x74\x61\x6e\x63\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x65\x77\x5f\x66\x20\x3d\x20\x5f\x66\x69\x6e\x64\x5f\x5f\x6e\x65\x77\x5f\x5f\x28\x63\x6c\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x6e\x65\x77\x61\x72\x67\x73\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x65\x77\x61\x72\x67\x73\x20\x3d\x20\x5b\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x69\x29\x20\x66\x6f\x72\x20\x69\x20\x69\x6e\x20\x6e\x65\x77\x61\x72\x67\x73\x5d\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x6e\x73\x74\x20\x3d\x20\x6e\x65\x77\x5f\x66\x28\x63\x6c\x73\x2c\x20\x2a\x6e\x65\x77\x61\x72\x67\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x65\x6c\x73\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x6e\x73\x74\x20\x3d\x20\x6e\x65\x77\x5f\x66\x28\x63\x6c\x73\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x6c\x66\x2e\x74\x61\x67\x28\x69\x6e\x64\x65\x78\x2c\x20\x69\x6e\x73\x74\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x23\x20\x72\x65\x73\x74\x6f\x72\x65\x20\x73\x74\x61\x74\x65\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x73\x74\x61\x74\x65\x20\x69\x73\x20\x6e\x6f\x74\x20\x4e\x6f\x6e\x65\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x6b\x2c\x76\x20\x69\x6e\x20\x73\x74\x61\x74\x65\x2e\x69\x74\x65\x6d\x73\x28\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x74\x61\x74\x74\x72\x28\x69\x6e\x73\x74\x2c\x20\x6b\x2c\x20\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x76\x29\x29\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x69\x6e\x73\x74\x0a\x0a\x20\x20\x20\x20\x64\x65\x66\x20\x72\x75\x6e\x5f\x70\x69\x70\x65\x28\x73\x65\x6c\x66\x29\x3a\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x73\x65\x6c\x66\x2e\x75\x6e\x77\x72\x61\x70\x28\x73\x65\x6c\x66\x2e\x6f\x62\x6a\x29\x0a\x0a\x0a\x64\x65\x66\x20\x5f\x77\x72\x61\x70\x28\x6f\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x50\x69\x63\x6b\x6c\x65\x72\x28\x6f\x29\x2e\x72\x75\x6e\x5f\x70\x69\x70\x65\x28\x29\x0a\x0a\x64\x65\x66\x20\x5f\x75\x6e\x77\x72\x61\x70\x28\x70\x61\x63\x6b\x65\x64\x3a\x20\x6c\x69\x73\x74\x29\x3a\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x55\x6e\x70\x69\x63\x6b\x6c\x65\x72\x28\x2a\x70\x61\x63\x6b\x65\x64\x29\x2e\x72\x75\x6e\x5f\x70\x69\x70\x65\x28\x29\x0a\x0a\x64\x65\x66\x20\x64\x75\x6d\x70\x73\x28\x6f\x29\x20\x2d\x3e\x20\x62\x79\x74\x65\x73\x3a\x0a\x20\x20\x20\x20\x6f\x20\x3d\x20\x5f\x77\x72\x61\x70\x28\x6f\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x6a\x73\x6f\x6e\x2e\x64\x75\x6d\x70\x73\x28\x6f\x29\x2e\x65\x6e\x63\x6f\x64\x65\x28\x29\x0a\x0a\x64\x65\x66\x20\x6c\x6f\x61\x64\x73\x28\x62\x29\x20\x2d\x3e\x20\x6f\x62\x6a\x65\x63\x74\x3a\x0a\x20\x20\x20\x20\x61\x73\x73\x65\x72\x74\x20\x74\x79\x70\x65\x28\x62\x29\x20\x69\x73\x20\x62\x79\x74\x65\x73\x0a\x20\x20\x20\x20\x6f\x20\x3d\x20\x6a\x73\x6f\x6e\x2e\x6c\x6f\x61\x64\x73\x28\x62\x2e\x64\x65\x63\x6f\x64\x65\x28\x29\x29\x0a\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x5f\x75\x6e\x77\x72\x61\x70\x28\x6f\x29" },

    };
}   // namespace pkpy



namespace pkpy {

#define PY_CLASS(T, mod, name)                  \
    static Type _type(VM* vm) {                 \
        static const StrName __x0(#mod);        \
        static const StrName __x1(#name);       \
        return PK_OBJ_GET(Type, vm->_modules[__x0]->attr(__x1));               \
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

int c99_sizeof(VM*, const Str&);

inline PyObject* py_var(VM* vm, void* p);
inline PyObject* py_var(VM* vm, char* p);

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
    bool operator<(const VoidP& other) const { return ptr < other.ptr; }
    bool operator<=(const VoidP& other) const { return ptr <= other.ptr; }
    bool operator>(const VoidP& other) const { return ptr > other.ptr; }
    bool operator>=(const VoidP& other) const { return ptr >= other.ptr; }


    Str hex() const{
        std::stringstream ss;
        ss << std::hex << reinterpret_cast<intptr_t>(ptr);
        return "0x" + ss.str();
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct C99Struct{
    PY_CLASS(C99Struct, c, struct)

    static constexpr int INLINE_SIZE = 24;

    char _inlined[INLINE_SIZE];
    char* p;
    int size;

    C99Struct(int new_size){
        this->size = new_size;
        if(size <= INLINE_SIZE){
            p = _inlined;
        }else{
            p = (char*)malloc(size);
        }
    }

    template<typename T>
    C99Struct(std::monostate _, const T& data): C99Struct(sizeof(T)){
        static_assert(std::is_pod_v<T>);
        static_assert(!std::is_pointer_v<T>);
        memcpy(p, &data, this->size);
    }

    C99Struct(void* p, int size): C99Struct(size){
        if(p != nullptr) memcpy(this->p, p, size);
    }

    C99Struct(const C99Struct& other): C99Struct(other.p, other.size){}

    ~C99Struct(){ if(p!=_inlined) free(p); }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
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

struct C99ReflType final: ReflType{
    PY_CLASS(C99ReflType, c, _refl)

    C99ReflType(const ReflType& type){
        this->name = type.name;
        this->size = type.size;
        this->fields = type.fields;
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
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
    if(var == vm->None) return nullptr;     // None can be casted to any pointer implicitly
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
    return VAR_T(C99Struct, std::monostate(), data);
}
/*****************************************************************/
struct NativeProxyFuncCBase {
    virtual PyObject* operator()(VM* vm, ArgsView args) = 0;

    static void check_args_size(VM* vm, ArgsView args, int n){
        if (args.size() != n){
            vm->TypeError("expected " + std::to_string(n) + " arguments, got " + std::to_string(args.size()));
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

void add_module_c(VM* vm);

}   // namespace pkpy
namespace pkpy{

    void VoidP::_register(VM* vm, PyObject* mod, PyObject* type){
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

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            VoidP& self = _CAST(VoidP&, obj);
            std::stringstream ss;
            ss << "<void* at " << self.hex();
            if(self.base_offset != 1) ss << ", base_offset=" << self.base_offset;
            ss << ">";
            return VAR(ss.str());
        });

#define BIND_CMP(name, op)  \
        vm->bind##name(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){       \
            if(!is_non_tagged_type(rhs, VoidP::_type(vm))) return vm->NotImplemented;       \
            return VAR(_CAST(VoidP&, lhs) op _CAST(VoidP&, rhs));                           \
        });

        BIND_CMP(__eq__, ==)
        BIND_CMP(__lt__, <)
        BIND_CMP(__le__, <=)
        BIND_CMP(__gt__, >)
        BIND_CMP(__ge__, >=)

#undef BIND_CMP

        vm->bind__hash__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
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

        vm->bind__add__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            VoidP& self = _CAST(VoidP&, lhs);
            i64 offset = CAST(i64, rhs);
            return VAR_T(VoidP, (char*)self.ptr + offset);
        });

        vm->bind__sub__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
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
        BIND_SETGET(void*, "void_p")

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

#undef BIND_SETGET
    }

    void C99Struct::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<-1>(type, [](VM* vm, ArgsView args){
            if(args.size() == 1+1){
                if(is_int(args[1])){
                    int size = _CAST(int, args[1]);
                    return VAR_T(C99Struct, size);
                }
                if(is_non_tagged_type(args[1], vm->tp_str)){
                    const Str& s = _CAST(Str&, args[1]);
                    return VAR_T(C99Struct, (void*)s.data, s.size);
                }
                if(is_non_tagged_type(args[1], vm->tp_bytes)){
                    const Bytes& b = _CAST(Bytes&, args[1]);
                    return VAR_T(C99Struct, (void*)b.data(), b.size());
                }
                vm->TypeError("expected int, str or bytes");
                return vm->None;
            }
            if(args.size() == 1+2){
                void* p = CAST(void*, args[1]);
                int size = CAST(int, args[2]);
                return VAR_T(C99Struct, p, size);
            }
            vm->TypeError("expected 1 or 2 arguments");
            return vm->None;
        });

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

        vm->bind_method<0>(type, "to_string", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR(Str(self.p, self.size));
        });

        vm->bind_method<0>(type, "to_bytes", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            std::vector<char> buffer(self.size);
            memcpy(buffer.data(), self.p, self.size);
            return VAR(Bytes(std::move(buffer)));
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

    void C99ReflType::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_notimplemented_constructor<C99ReflType>(type);

        vm->bind_method<0>(type, "__call__", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR_T(C99Struct, nullptr, self.size);
        });

        vm->bind_method<0>(type, "__repr__", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR("<ctype '" + Str(self.name) + "'>");
        });

        vm->bind_method<0>(type, "name", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR(self.name);
        });

        vm->bind_method<0>(type, "size", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR(self.size);
        });

        vm->bind__getitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj, PyObject* key){
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

    PyObject* void_p_t = mod->attr("void_p");
    for(const char* t: {"char", "uchar", "short", "ushort", "int", "uint", "long", "ulong", "longlong", "ulonglong", "float", "double", "bool"}){
        mod->attr().set(Str(t) + "_", VAR_T(C99ReflType, _refl_types[t]));
        mod->attr().set(Str(t) + "_p", void_p_t);
    }
}

int c99_sizeof(VM* vm, const Str& type){
    auto it = _refl_types.find(type.sv());
    if(it != _refl_types.end()) return it->second.size;
    vm->ValueError("not a valid c99 type");
    return 0;
}

}   // namespace pkpy


namespace pkpy{

struct RangeIter{
    PY_CLASS(RangeIter, builtins, "_range_iterator")
    Range r;
    i64 current;
    RangeIter(Range r) : r(r), current(r.start) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct ArrayIter{
    PY_CLASS(ArrayIter, builtins, "_array_iterator")
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
    PY_CLASS(StringIter, builtins, "_string_iterator")
    PyObject* ref;
    Str* str;
    int index;

    StringIter(PyObject* ref) : ref(ref), str(&PK_OBJ_GET(Str, ref)), index(0) {}

    void _gc_mark() const{ PK_OBJ_MARK(ref); }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
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
            // TODO: optimize this... operator[] is of O(n) complexity
            if(self.index == self.str->u8_length()) return vm->StopIteration;
            return VAR(self.str->u8_getitem(self.index++));
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
        PyObject* ret = vm->_run_top_frame();
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

}	// namespace pkpy


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
        vm->bind_method<0>(type, "start", PK_LAMBDA(VAR(_CAST(ReMatch&, args[0]).start)));
        vm->bind_method<0>(type, "end", PK_LAMBDA(VAR(_CAST(ReMatch&, args[0]).end)));

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

struct Vec4{
    float x, y, z, w;
    Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vec4(const Vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

    Vec4 operator+(const Vec4& v) const { return Vec4(x + v.x, y + v.y, z + v.z, w + v.w); }
    Vec4& operator+=(const Vec4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
    Vec4 operator-(const Vec4& v) const { return Vec4(x - v.x, y - v.y, z - v.z, w - v.w); }
    Vec4& operator-=(const Vec4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
    Vec4 operator*(float s) const { return Vec4(x * s, y * s, z * s, w * s); }
    Vec4& operator*=(float s) { x *= s; y *= s; z *= s; w *= s; return *this; }
    Vec4 operator/(float s) const { return Vec4(x / s, y / s, z / s, w / s); }
    Vec4& operator/=(float s) { x /= s; y /= s; z /= s; w /= s; return *this; }
    Vec4 operator-() const { return Vec4(-x, -y, -z, -w); }
    bool operator==(const Vec4& v) const { return isclose(x, v.x) && isclose(y, v.y) && isclose(z, v.z) && isclose(w, v.w); }
    bool operator!=(const Vec4& v) const { return !isclose(x, v.x) || !isclose(y, v.y) || !isclose(z, v.z) || !isclose(w, v.w); }
    float dot(const Vec4& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
    float length() const { return sqrtf(x * x + y * y + z * z + w * w); }
    float length_squared() const { return x * x + y * y + z * z + w * w; }
    Vec4 normalize() const { float l = length(); return Vec4(x / l, y / l, z / l, w / l); }
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

    Vec2 transform_point(Vec2 vec) const {
        return Vec2(_11 * vec.x + _12 * vec.y + _13, _21 * vec.x + _22 * vec.y + _23);
    }

    Vec2 transform_vector(Vec2 vec) const {
        return Vec2(_11 * vec.x + _12 * vec.y, _21 * vec.x + _22 * vec.y);
    }
};

struct PyVec2;
struct PyVec3;
struct PyVec4;
struct PyMat3x3;
PyObject* py_var(VM*, Vec2);
PyObject* py_var(VM*, const PyVec2&);
PyObject* py_var(VM*, Vec3);
PyObject* py_var(VM*, const PyVec3&);
PyObject* py_var(VM*, Vec4);
PyObject* py_var(VM*, const PyVec4&);
PyObject* py_var(VM*, const Mat3x3&);
PyObject* py_var(VM*, const PyMat3x3&);


struct PyVec2: Vec2 {
    PY_CLASS(PyVec2, linalg, vec2)

    PyVec2() : Vec2() {}
    PyVec2(const Vec2& v) : Vec2(v) {}
    PyVec2(const PyVec2& v) : Vec2(v) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct PyVec3: Vec3 {
    PY_CLASS(PyVec3, linalg, vec3)

    PyVec3() : Vec3() {}
    PyVec3(const Vec3& v) : Vec3(v) {}
    PyVec3(const PyVec3& v) : Vec3(v) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct PyVec4: Vec4{
    PY_CLASS(PyVec4, linalg, vec4)

    PyVec4(): Vec4(){}
    PyVec4(const Vec4& v): Vec4(v){}
    PyVec4(const PyVec4& v): Vec4(v){}

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct PyMat3x3: Mat3x3{
    PY_CLASS(PyMat3x3, linalg, mat3x3)

    PyMat3x3(): Mat3x3(){}
    PyMat3x3(const Mat3x3& other): Mat3x3(other){}
    PyMat3x3(const PyMat3x3& other): Mat3x3(other){}

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

inline void add_module_linalg(VM* vm){
    PyObject* linalg = vm->new_module("linalg");
    PyVec2::register_class(vm, linalg);
    PyVec3::register_class(vm, linalg);
    PyVec4::register_class(vm, linalg);
    PyMat3x3::register_class(vm, linalg);
}

static_assert(sizeof(Py_<PyMat3x3>) <= 64);

}   // namespace pkpy
namespace pkpy{

#define BIND_VEC_ADDR(D)   \
        vm->bind_method<0>(type, "addr", [](VM* vm, ArgsView args){         \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                     \
            return VAR_T(VoidP, &self.x);                                   \
        });

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


    void PyVec2::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<3>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            return VAR(Vec2(x, y));
        });

        vm->bind_method<0>(type, "__getnewargs__", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            return VAR(Tuple({ VAR(self.x), VAR(self.y) }));
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
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

        BIND_VEC_ADDR(2)
        BIND_VEC_VEC_OP(2, __add__, +)
        BIND_VEC_VEC_OP(2, __sub__, -)
        BIND_VEC_FLOAT_OP(2, __mul__, *)
        BIND_VEC_FLOAT_OP(2, __rmul__, *)
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

    void PyVec3::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<4>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            float z = CAST_F(args[3]);
            return VAR(Vec3(x, y, z));
        });

        vm->bind_method<0>(type, "__getnewargs__", [](VM* vm, ArgsView args){
            PyVec3& self = _CAST(PyVec3&, args[0]);
            return VAR(Tuple({ VAR(self.x), VAR(self.y), VAR(self.z) }));
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            PyVec3& self = _CAST(PyVec3&, obj);
            std::stringstream ss;
            ss << "vec3(" << self.x << ", " << self.y << ", " << self.z << ")";
            return VAR(ss.str());
        });

        vm->bind_method<0>(type, "copy", [](VM* vm, ArgsView args){
            PyVec3& self = _CAST(PyVec3&, args[0]);
            return VAR_T(PyVec3, self);
        });

        BIND_VEC_ADDR(3)
        BIND_VEC_VEC_OP(3, __add__, +)
        BIND_VEC_VEC_OP(3, __sub__, -)
        BIND_VEC_FLOAT_OP(3, __mul__, *)
        BIND_VEC_FLOAT_OP(3, __rmul__, *)
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

    void PyVec4::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<1+4>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            float z = CAST_F(args[3]);
            float w = CAST_F(args[4]);
            return VAR(Vec4(x, y, z, w));
        });

        vm->bind_method<0>(type, "__getnewargs__", [](VM* vm, ArgsView args){
            PyVec4& self = _CAST(PyVec4&, args[0]);
            return VAR(Tuple({ VAR(self.x), VAR(self.y), VAR(self.z), VAR(self.w) }));
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            PyVec4& self = _CAST(PyVec4&, obj);
            std::stringstream ss;
            ss << "vec4(" << self.x << ", " << self.y << ", " << self.z << ", " << self.w << ")";
            return VAR(ss.str());
        });

        vm->bind_method<0>(type, "copy", [](VM* vm, ArgsView args){
            PyVec4& self = _CAST(PyVec4&, args[0]);
            return VAR_T(PyVec4, self);
        });

        BIND_VEC_ADDR(4)
        BIND_VEC_VEC_OP(4, __add__, +)
        BIND_VEC_VEC_OP(4, __sub__, -)
        BIND_VEC_FLOAT_OP(4, __mul__, *)
        BIND_VEC_FLOAT_OP(4, __rmul__, *)
        BIND_VEC_FLOAT_OP(4, __truediv__, /)
        BIND_VEC_VEC_OP(4, __eq__, ==)
        BIND_VEC_FIELD(4, x)
        BIND_VEC_FIELD(4, y)
        BIND_VEC_FIELD(4, z)
        BIND_VEC_FIELD(4, w)
        BIND_VEC_FUNCTION_1(4, dot)
        BIND_VEC_FUNCTION_0(4, length)
        BIND_VEC_FUNCTION_0(4, length_squared)
        BIND_VEC_FUNCTION_0(4, normalize)
    }

#undef BIND_VEC_ADDR
#undef BIND_VEC_VEC_OP
#undef BIND_VEC_FLOAT_OP
#undef BIND_VEC_FIELD
#undef BIND_VEC_FUNCTION_0
#undef BIND_VEC_FUNCTION_1

    void PyMat3x3::_register(VM* vm, PyObject* mod, PyObject* type){
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

        vm->bind_method<0>(type, "__getnewargs__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            Tuple t(9);
            for(int i=0; i<9; i++) t[i] = VAR(self.v[i]);
            return VAR(std::move(t));
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

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
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

        vm->bind__getitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj, PyObject* index){
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
        vm->bind_method<1>(type, "__rmul__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[1]);
            f64 other = CAST_F(args[0]);
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
            vm->BinaryOptError("@");
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
            PK_UNUSED(args);
            return VAR_T(PyMat3x3, Mat3x3::zeros());
        });

        vm->bind_func<0>(type, "ones", [](VM* vm, ArgsView args){
            PK_UNUSED(args);
            return VAR_T(PyMat3x3, Mat3x3::ones());
        });

        vm->bind_func<0>(type, "identity", [](VM* vm, ArgsView args){
            PK_UNUSED(args);
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

}   // namespace pkpy


namespace pkpy{

void add_module_easing(VM* vm);

} // namespace pkpy
namespace pkpy{

#if PK_MODULE_EASING

// https://easings.net/

static const double PI = 3.1415926545;

static double easeLinear( double x ) {
    return x;
}

static double easeInSine( double x ) {
    return 1.0 - std::cos( x * PI / 2 );
}

static double easeOutSine( double x ) {
	return std::sin( x * PI / 2 );
}

static double easeInOutSine( double x ) {
	return -( std::cos( PI * x ) - 1 ) / 2;
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
    const double c4 = (2 * PI) / 3;
    if( x == 0 ) {
        return 0;
    } else if( x == 1 ) {
        return 1;
    } else {
        return -std::pow( 2, 10 * x - 10 ) * std::sin( (x * 10 - 10.75) * c4 );
    }
}

static double easeOutElastic( double x ) {
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


#else

void add_module_easing(VM* vm){
    PK_UNUSED(vm);
}

#endif

}   // namespace pkpy


namespace pkpy{
    Bytes _default_import_handler(const Str& name);
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

#endif

} // namespace pkpy
namespace pkpy{

Bytes _default_import_handler(const Str& name){
#if PK_ENABLE_OS
    std::filesystem::path path(name.sv());
    bool exists = std::filesystem::exists(path);
    if(!exists) return Bytes();
    std::string cname = name.str();
    FILE* fp = fopen(cname.c_str(), "rb");
    if(!fp) return Bytes();
    fseek(fp, 0, SEEK_END);
    std::vector<char> buffer(ftell(fp));
    fseek(fp, 0, SEEK_SET);
    size_t sz = fread(buffer.data(), 1, buffer.size(), fp);
    PK_UNUSED(sz);
    fclose(fp);
    return Bytes(std::move(buffer));
#else
    return Bytes();
#endif
};


#if PK_ENABLE_OS
    void FileIO::_register(VM* vm, PyObject* mod, PyObject* type){
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
            size_t sz = fread(buffer.data(), 1, buffer.size(), io.fp);
            PK_UNUSED(sz);
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

        vm->bind_method<0>(type, "__enter__", PK_LAMBDA(vm->None));
    }

    FileIO::FileIO(VM* vm, std::string file, std::string mode): file(file), mode(mode) {
        fp = fopen(file.c_str(), mode.c_str());
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
    vm->bind_builtin_func<2>("open", [](VM* vm, ArgsView args){
        static StrName m_io("io");
        static StrName m_FileIO("FileIO");
        return vm->call(vm->_modules[m_io]->attr(m_FileIO), args[0], args[1]);
    });
#endif
}

void add_module_os(VM* vm){
#if PK_ENABLE_OS
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
#endif
}

}   // namespace pkpy
#ifndef PK_EXPORT

#ifdef _WIN32
#define PK_EXPORT __declspec(dllexport)
#elif __EMSCRIPTEN__
#include <emscripten.h>
#define PK_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define PK_EXPORT __attribute__((visibility("default"))) __attribute__((used))
#endif

#define PK_LEGACY_EXPORT PK_EXPORT inline

#endif



namespace pkpy {

inline CodeObject_ VM::compile(Str source, Str filename, CompileMode mode, bool unknown_global_scope) {
    Compiler compiler(this, source, filename, mode, unknown_global_scope);
    try{
        return compiler.compile();
    }catch(Exception& e){
#if PK_DEBUG_FULL_EXCEPTION
        std::cerr << e.summary() << std::endl;
#endif
        _error(e);
        return nullptr;
    }
}


void init_builtins(VM* _vm);

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


void add_module_timeit(VM* vm);
void add_module_time(VM* vm);
void add_module_sys(VM* vm);
void add_module_json(VM* vm);

void add_module_math(VM* vm);
void add_module_dis(VM* vm);
void add_module_traceback(VM* vm);
void add_module_gc(VM* vm);

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
    void pkpy_vm_compile(pkpy::VM* vm, const char* source, const char* filename, int mode, bool* ok, char** res){
        try{
            pkpy::CodeObject_ code = vm->compile(source, filename, (pkpy::CompileMode)mode);
            *res = code->serialize(vm).c_str_dup();
            *ok = true;
        }catch(pkpy::Exception& e){
            *ok = false;
            *res = e.summary().c_str_dup();
        }catch(std::exception& e){
            *ok = false;
            *res = strdup(e.what());
        }catch(...){
            *ok = false;
            *res = strdup("unknown error");
        }
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

namespace pkpy{

void init_builtins(VM* _vm) {
#define BIND_NUM_ARITH_OPT(name, op)                                                                    \
    _vm->bind##name(_vm->tp_int, [](VM* vm, PyObject* lhs, PyObject* rhs) {                             \
        if(is_int(rhs)) return VAR(_CAST(i64, lhs) op _CAST(i64, rhs));                                 \
        if(is_float(rhs)) return VAR(_CAST(i64, lhs) op _CAST(f64, rhs));                               \
        return vm->NotImplemented;                                                                      \
    });                                                                                                 \
    _vm->bind##name(_vm->tp_float, [](VM* vm, PyObject* lhs, PyObject* rhs) {                           \
        if(is_float(rhs)) return VAR(_CAST(f64, lhs) op _CAST(f64, rhs));                               \
        if(is_int(rhs)) return VAR(_CAST(f64, lhs) op _CAST(i64, rhs));                                 \
        return vm->NotImplemented;                                                                      \
    });

    BIND_NUM_ARITH_OPT(__add__, +)
    BIND_NUM_ARITH_OPT(__sub__, -)
    BIND_NUM_ARITH_OPT(__mul__, *)

#undef BIND_NUM_ARITH_OPT

#define BIND_NUM_LOGICAL_OPT(name, op)   \
    _vm->bind##name(_vm->tp_int, [](VM* vm, PyObject* lhs, PyObject* rhs) { \
        if(is_int(rhs))     return VAR(_CAST(i64, lhs) op _CAST(i64, rhs)); \
        if(is_float(rhs))   return VAR(_CAST(i64, lhs) op _CAST(f64, rhs)); \
        return vm->NotImplemented;                                          \
    });                                                                     \
    _vm->bind##name(_vm->tp_float, [](VM* vm, PyObject* lhs, PyObject* rhs) {   \
        if(is_int(rhs))     return VAR(_CAST(f64, lhs) op _CAST(i64, rhs));     \
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

    _vm->bind_builtin_func<2>("super", [](VM* vm, ArgsView args) {
        vm->check_non_tagged_type(args[0], vm->tp_type);
        Type type = PK_OBJ_GET(Type, args[0]);
        if(!vm->isinstance(args[1], type)){
            Str _0 = obj_type_name(vm, PK_OBJ_GET(Type, vm->_t(args[1])));
            Str _1 = obj_type_name(vm, type);
            vm->TypeError("super(): " + _0.escape() + " is not an instance of " + _1.escape());
        }
        Type base = vm->_all_types[type].base;
        return vm->heap.gcnew(vm->tp_super, Super(args[1], base));
    });

    _vm->bind_builtin_func<2>("isinstance", [](VM* vm, ArgsView args) {
        vm->check_non_tagged_type(args[1], vm->tp_type);
        Type type = PK_OBJ_GET(Type, args[1]);
        return VAR(vm->isinstance(args[0], type));
    });

    _vm->bind_builtin_func<0>("globals", [](VM* vm, ArgsView args) {
        PyObject* mod = vm->top_frame()->_module;
        return VAR(MappingProxy(mod));
    });

    _vm->bind_builtin_func<3>("pow", [](VM* vm, ArgsView args) {
        i64 lhs = CAST(i64, args[0]);   // assume lhs>=0
        i64 rhs = CAST(i64, args[1]);   // assume rhs>=0
        i64 mod = CAST(i64, args[2]);   // assume mod>0, mod*mod should not overflow

        if(rhs <= 0){
            vm->ValueError("pow(): rhs should be positive");
        }

        static const auto _mul = [](i64 a, i64 b, i64 c){
            if(c < 16384) return (a%c) * (b%c) % c;
            i64 res = 0;
            while(b > 0){
                if(b & 1) res = (res + a) % c;
                a = (a << 1) % c;
                b >>= 1;
            }
            return res;
        };

        i64 res = 1;
        lhs %= mod;
        while(rhs){
            if(rhs & 1) res = _mul(res, lhs, mod);
            lhs = _mul(lhs, lhs, mod);
            rhs >>= 1;
        }
        return VAR(res);
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
        if(is_int(args[0])){
            i64 lhs = _CAST(i64, args[0]);
            i64 rhs = CAST(i64, args[1]);
            auto res = std::div(lhs, rhs);
            return VAR(Tuple({VAR(res.quot), VAR(res.rem)}));
        }else{
            DEF_SNAME(__divmod__);
            return vm->call_method(args[0], __divmod__, args[1]);
        }
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

    _vm->bind_builtin_func<1>("repr", PK_LAMBDA(vm->py_repr(args[0])));

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

    _vm->bind_builtin_func<1>("bin", [](VM* vm, ArgsView args) {
        std::stringstream ss;
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

    _vm->bind__eq__(_vm->tp_object, [](VM* vm, PyObject* lhs, PyObject* rhs) { return VAR(lhs == rhs); });
    _vm->bind__hash__(_vm->tp_object, [](VM* vm, PyObject* obj) { return PK_BITS(obj); });

    _vm->cached_object__new__ = _vm->bind_constructor<1>("object", [](VM* vm, ArgsView args) {
        vm->check_non_tagged_type(args[0], vm->tp_type);
        Type t = PK_OBJ_GET(Type, args[0]);
        return vm->heap.gcnew<DummyInstance>(t, {});
    });

    _vm->bind_constructor<2>("type", PK_LAMBDA(vm->_t(args[1])));

    _vm->bind_constructor<-1>("range", [](VM* vm, ArgsView args) {
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

    _vm->bind__iter__(_vm->tp_range, [](VM* vm, PyObject* obj) { return VAR_T(RangeIter, PK_OBJ_GET(Range, obj)); });
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
    _vm->bind_constructor<-1>("int", [](VM* vm, ArgsView args) {
        if(args.size() == 1+0) return VAR(0);
        if(args.size() == 1+1){
            if (is_type(args[1], vm->tp_float)) return VAR((i64)CAST(f64, args[1]));
            if (is_type(args[1], vm->tp_int)) return args[1];
            if (is_type(args[1], vm->tp_bool)) return VAR(_CAST(bool, args[1]) ? 1 : 0);
        }
        if(args.size() > 1+2) vm->TypeError("int() takes at most 2 arguments");
        if (is_type(args[1], vm->tp_str)) {
            int base = 10;
            if(args.size() == 1+2) base = CAST(i64, args[2]);
            const Str& s = CAST(Str&, args[1]);
            try{
                size_t parsed = 0;
                i64 val = Number::stoi(s.str(), &parsed, base);
                PK_ASSERT(parsed == s.length());
                return VAR(val);
            }catch(...){
                vm->ValueError("invalid literal for int(): " + s.escape());
            }
        }
        vm->TypeError("invalid arguments for int()");
        return vm->None;
    });

    _vm->bind_method<0>("int", "bit_length", [](VM* vm, ArgsView args) {
        i64 x = _CAST(i64, args[0]);
        if(x < 0) x = -x;
        int bits = 0;
        while(x){ x >>= 1; bits++; }
        return VAR(bits);
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
            }catch(...){
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
    _vm->bind_constructor<2>("str", PK_LAMBDA(vm->py_str(args[1])));

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

    _vm->bind_method<1>("str", "__rmul__", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        i64 n = CAST(i64, args[1]);
        std::stringstream ss;
        for(i64 i = 0; i < n; i++) ss << self.sv();
        return VAR(ss.str());
    });

    _vm->bind__contains__(_vm->tp_str, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        const Str& self = _CAST(Str&, lhs);
        return VAR(self.index(CAST(Str&, rhs)) != -1);
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

#define BIND_CMP_STR(name, op) \
    _vm->bind##name(_vm->tp_str, [](VM* vm, PyObject* lhs, PyObject* rhs) { \
        if(!is_non_tagged_type(rhs, vm->tp_str)) return vm->NotImplemented; \
        return VAR(_CAST(Str&, lhs) op _CAST(Str&, rhs));                   \
    });

    BIND_CMP_STR(__eq__, ==)
    BIND_CMP_STR(__lt__, <)
    BIND_CMP_STR(__le__, <=)
    BIND_CMP_STR(__gt__, >)
    BIND_CMP_STR(__ge__, >=)
#undef BIND_CMP_STR

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
        if(old.empty()) vm->ValueError("empty substring");
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

    _vm->bind_method<1>("str", "find", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& sub = CAST(Str&, args[1]);
        return VAR(self.index(sub));
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

    _vm->bind_method<0>("str", "lower", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        return VAR(self.lower());
    });

    _vm->bind_method<0>("str", "upper", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        return VAR(self.upper());
    });

    /************ list ************/
    _vm->bind_constructor<-1>("list", [](VM* vm, ArgsView args) {
        if(args.size() == 1+0) return VAR(List());
        if(args.size() == 1+1){
            return vm->py_list(args[1]);
        }
        vm->TypeError("list() takes 0 or 1 arguments");
        return vm->None;
    });

    _vm->bind__contains__(_vm->tp_list, [](VM* vm, PyObject* obj, PyObject* item) {
        List& self = _CAST(List&, obj);
        for(PyObject* i: self) if(vm->py_equals(i, item)) return vm->True;
        return vm->False;
    });

    _vm->bind_method<1>("list", "count", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        int count = 0;
        for(PyObject* i: self) if(vm->py_equals(i, args[1])) count++;
        return VAR(count);
    });

    _vm->bind__eq__(_vm->tp_list, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        List& a = _CAST(List&, lhs);
        if(!is_non_tagged_type(rhs, vm->tp_list)) return vm->NotImplemented;
        List& b = _CAST(List&, rhs);
        if(a.size() != b.size()) return vm->False;
        for(int i=0; i<a.size(); i++){
            if(!vm->py_equals(a[i], b[i])) return vm->False;
        }
        return vm->True;
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
        if(!is_int(rhs)) return vm->NotImplemented;
        int n = _CAST(int, rhs);
        List result;
        result.reserve(self.size() * n);
        for(int i = 0; i < n; i++) result.extend(self);
        return VAR(std::move(result));
    });
    _vm->bind_method<1>("list", "__rmul__", [](VM* vm, ArgsView args) {
        const List& self = _CAST(List&, args[0]);
        if(!is_int(args[1])) return vm->NotImplemented;
        int n = _CAST(int, args[1]);
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

    _vm->bind_method<0>("list", "copy", PK_LAMBDA(VAR(_CAST(List, args[0]))));

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
    _vm->bind_constructor<-1>("tuple", [](VM* vm, ArgsView args) {
        if(args.size() == 1+0) return VAR(Tuple(0));
        if(args.size() == 1+1){
            List list = CAST(List, vm->py_list(args[1]));
            return VAR(Tuple(std::move(list)));
        }
        vm->TypeError("tuple() takes at most 1 argument");
        return vm->None;
    });

    _vm->bind__contains__(_vm->tp_tuple, [](VM* vm, PyObject* obj, PyObject* item) {
        Tuple& self = _CAST(Tuple&, obj);
        for(PyObject* i: self) if(vm->py_equals(i, item)) return vm->True;
        return vm->False;
    });

    _vm->bind_method<1>("tuple", "count", [](VM* vm, ArgsView args) {
        Tuple& self = _CAST(Tuple&, args[0]);
        int count = 0;
        for(PyObject* i: self) if(vm->py_equals(i, args[1])) count++;
        return VAR(count);
    });

    _vm->bind__eq__(_vm->tp_tuple, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        const Tuple& self = _CAST(Tuple&, lhs);
        if(!is_non_tagged_type(rhs, vm->tp_tuple)) return vm->NotImplemented;
        const Tuple& other = _CAST(Tuple&, rhs);
        if(self.size() != other.size()) return vm->False;
        for(int i = 0; i < self.size(); i++) {
            if(!vm->py_equals(self[i], other[i])) return vm->False;
        }
        return vm->True;
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
    _vm->bind_constructor<2>("bool", PK_LAMBDA(VAR(vm->py_bool(args[1]))));
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

    const static auto f_bool_add = [](VM* vm, PyObject* lhs, PyObject* rhs) -> PyObject* {
        int x = (int)_CAST(bool, lhs);
        if(is_int(rhs)) return VAR(x + _CAST(int, rhs));
        if(rhs == vm->True) return VAR(x + 1);
        if(rhs == vm->False) return VAR(x);
        return vm->NotImplemented;
    };

    const static auto f_bool_mul = [](VM* vm, PyObject* lhs, PyObject* rhs) -> PyObject* {
        int x = (int)_CAST(bool, lhs);
        if(is_int(rhs)) return VAR(x * _CAST(int, rhs));
        if(rhs == vm->True) return VAR(x);
        if(rhs == vm->False) return VAR(0);
        return vm->NotImplemented;
    };

    _vm->bind__add__(_vm->tp_bool, f_bool_add);
    _vm->bind_method<1>("bool", "__radd__", [](VM* vm, ArgsView args){
        return f_bool_add(vm, args[0], args[1]);
    });

    _vm->bind__mul__(_vm->tp_bool, f_bool_mul);
    _vm->bind_method<1>("bool", "__rmul__", [](VM* vm, ArgsView args){
        return f_bool_mul(vm, args[0], args[1]);
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
        if(is_non_tagged_type(rhs, vm->tp_bool)) return VAR(lhs == rhs);
        if(is_int(rhs)) return VAR(_CAST(bool, lhs) == (bool)CAST(i64, rhs));
        return vm->NotImplemented;
    });
    _vm->bind__repr__(_vm->_type("ellipsis"), [](VM* vm, PyObject* self) {
        return VAR("...");
    });
    _vm->bind__repr__(_vm->_type("NotImplementedType"), [](VM* vm, PyObject* self) {
        return VAR("NotImplemented");
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

    _vm->bind__eq__(_vm->tp_bytes, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        if(!is_non_tagged_type(rhs, vm->tp_bytes)) return vm->NotImplemented;
        return VAR(_CAST(Bytes&, lhs) == _CAST(Bytes&, rhs));
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
        return VAR(self.attr().contains(CAST(Str&, key)));
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
        return VAR(self.contains(key));
    });

    _vm->bind__iter__(_vm->tp_dict, [](VM* vm, PyObject* obj) {
        const Dict& self = _CAST(Dict&, obj);
        return vm->py_iter(VAR(self.keys()));
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
        const Dict& self = _CAST(Dict&, args[0]);
        return VAR(self.keys());
    });

    _vm->bind_method<0>("dict", "values", [](VM* vm, ArgsView args) {
        const Dict& self = _CAST(Dict&, args[0]);
        return VAR(self.values());
    });

    _vm->bind_method<0>("dict", "items", [](VM* vm, ArgsView args) {
        const Dict& self = _CAST(Dict&, args[0]);
        Tuple items(self.size());
        int j = 0;
        self.apply([&](PyObject* k, PyObject* v){
            items[j++] = VAR(Tuple({k, v}));
        });
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

        self.apply([&](PyObject* k, PyObject* v){
            if(!first) ss << ", ";
            first = false;
            Str key = CAST(Str&, vm->py_repr(k));
            Str value = CAST(Str&, vm->py_repr(v));
            ss << key << ": " << value;
        });

        ss << "}";
        return VAR(ss.str());
    });

    _vm->bind__json__(_vm->tp_dict, [](VM* vm, PyObject* obj) {
        Dict& self = _CAST(Dict&, obj);
        std::stringstream ss;
        ss << "{";
        bool first = true;

        self.apply([&](PyObject* k, PyObject* v){
            if(!first) ss << ", ";
            first = false;
            Str key = CAST(Str&, k).escape(false);
            Str value = CAST(Str&, vm->py_json(v));
            ss << key << ": " << value;
        });

        ss << "}";
        return VAR(ss.str());
    });

    _vm->bind__eq__(_vm->tp_dict, [](VM* vm, PyObject* a, PyObject* b) {
        Dict& self = _CAST(Dict&, a);
        if(!is_non_tagged_type(b, vm->tp_dict)) return vm->NotImplemented;
        Dict& other = _CAST(Dict&, b);
        if(self.size() != other.size()) return vm->False;
        for(int i=0; i<self._capacity; i++){
            auto item = self._items[i];
            if(item.first == nullptr) continue;
            PyObject* value = other.try_get(item.first);
            if(value == nullptr) return vm->False;
            if(!vm->py_equals(item.second, value)) return vm->False;
        }
        return vm->True;
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
    
    _vm->_t(_vm->tp_function)->attr().set("__doc__", _vm->property([](VM* vm, ArgsView args) {
        Function& func = _CAST(Function&, args[0]);
        return VAR(func.decl->docstring);
    }));

    _vm->_t(_vm->tp_native_func)->attr().set("__doc__", _vm->property([](VM* vm, ArgsView args) {
        NativeFunc& func = _CAST(NativeFunc&, args[0]);
        if(func.decl != nullptr) return VAR(func.decl->docstring);
        return VAR("");
    }));

    _vm->_t(_vm->tp_function)->attr().set("__signature__", _vm->property([](VM* vm, ArgsView args) {
        Function& func = _CAST(Function&, args[0]);
        return VAR(func.decl->signature);
    }));

    _vm->_t(_vm->tp_native_func)->attr().set("__signature__", _vm->property([](VM* vm, ArgsView args) {
        NativeFunc& func = _CAST(NativeFunc&, args[0]);
        if(func.decl != nullptr) return VAR(func.decl->signature);
        return VAR("unknown(*args, **kwargs)");
    }));

    RangeIter::register_class(_vm, _vm->builtins);
    ArrayIter::register_class(_vm, _vm->builtins);
    StringIter::register_class(_vm, _vm->builtins);
    Generator::register_class(_vm, _vm->builtins);
}


void add_module_timeit(VM* vm){
    PyObject* mod = vm->new_module("timeit");
    vm->bind_func<2>(mod, "timeit", [](VM* vm, ArgsView args) {
        PyObject* f = args[0];
        i64 iters = CAST(i64, args[1]);
        auto now = std::chrono::system_clock::now();
        for(i64 i=0; i<iters; i++) vm->call(f);
        auto end = std::chrono::system_clock::now();
        f64 elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - now).count() / 1000.0;
        return VAR(elapsed);
    });
}

void add_module_time(VM* vm){
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


void add_module_sys(VM* vm){
    PyObject* mod = vm->new_module("sys");
    PyREPL::register_class(vm, mod);
    vm->setattr(mod, "version", VAR(PK_VERSION));

    PyObject* stdout_ = vm->heap.gcnew<DummyInstance>(vm->tp_object, {});
    PyObject* stderr_ = vm->heap.gcnew<DummyInstance>(vm->tp_object, {});
    vm->setattr(mod, "stdout", stdout_);
    vm->setattr(mod, "stderr", stderr_);

    vm->bind_func<1>(stdout_, "write", [](VM* vm, ArgsView args) {
        vm->_stdout(vm, CAST(Str&, args[0]));
        return vm->None;
    });

    vm->bind_func<1>(stderr_, "write", [](VM* vm, ArgsView args) {
        vm->_stderr(vm, CAST(Str&, args[0]));
        return vm->None;
    });
}

void add_module_json(VM* vm){
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
        return VAR(Tuple({VAR(f), VAR(i)}));
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

void add_module_dis(VM* vm){
    PyObject* mod = vm->new_module("dis");

    static const auto get_code = [](VM* vm, PyObject* obj)->CodeObject_{
        if(is_type(obj, vm->tp_str)){
            const Str& source = CAST(Str, obj);
            return vm->compile(source, "<dis>", EXEC_MODE);
        }
        PyObject* f = obj;
        if(is_type(f, vm->tp_bound_method)) f = CAST(BoundMethod, obj).func;
        return CAST(Function&, f).decl->code;
    };

    vm->bind_func<1>(mod, "dis", [](VM* vm, ArgsView args) {
        CodeObject_ code = get_code(vm, args[0]);
        vm->_stdout(vm, vm->disassemble(code));
        return vm->None;
    });

    vm->bind_func<1>(mod, "_s", [](VM* vm, ArgsView args) {
        CodeObject_ code = get_code(vm, args[0]);
        return VAR(code->serialize(vm));
    });
}

void add_module_gc(VM* vm){
    PyObject* mod = vm->new_module("gc");
    vm->bind_func<0>(mod, "collect", PK_LAMBDA(VAR(vm->heap.collect())));
}


void VM::post_init(){
    init_builtins(this);

    _t(tp_object)->attr().set("__class__", property(PK_LAMBDA(vm->_t(args[0]))));
    _t(tp_type)->attr().set("__base__", property([](VM* vm, ArgsView args){
        const PyTypeInfo& info = vm->_all_types[PK_OBJ_GET(Type, args[0])];
        return info.base.index == -1 ? vm->None : vm->_all_types[info.base].obj;
    }));
    _t(tp_type)->attr().set("__name__", property([](VM* vm, ArgsView args){
        const PyTypeInfo& info = vm->_all_types[PK_OBJ_GET(Type, args[0])];
        return VAR(info.name);
    }));

    _t(tp_bound_method)->attr().set("__self__", property([](VM* vm, ArgsView args){
        return CAST(BoundMethod&, args[0]).self;
    }));
    _t(tp_bound_method)->attr().set("__func__", property([](VM* vm, ArgsView args){
        return CAST(BoundMethod&, args[0]).func;
    }));

    bind__eq__(tp_bound_method, [](VM* vm, PyObject* lhs, PyObject* rhs){
        if(!is_non_tagged_type(rhs, vm->tp_bound_method)) return vm->NotImplemented;
        return VAR(_CAST(BoundMethod&, lhs) == _CAST(BoundMethod&, rhs));
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
        if(is_tagged(args[0]) || !args[0]->is_attr_valid()) return vm->None;
        return VAR(MappingProxy(args[0]));
    }));

#if !PK_DEBUG_NO_BUILTINS
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
    add_module_timeit(this);

    for(const char* name: {"this", "functools", "collections", "heapq", "bisect", "pickle", "_long"}){
        _lazy_modules[name] = kPythonLibs[name];
    }

    try{
        CodeObject_ code = compile(kPythonLibs["builtins"], "<builtins>", EXEC_MODE);
        this->_exec(code, this->builtins);
        code = compile(kPythonLibs["_set"], "<set>", EXEC_MODE);
        this->_exec(code, this->builtins);
    }catch(Exception& e){
        std::cerr << e.summary() << std::endl;
        std::cerr << "failed to load builtins module!!" << std::endl;
        exit(1);
    }

    if(enable_os){
        add_module_io(this);
        add_module_os(this);
        _import_handler = _default_import_handler;
    }

    add_module_linalg(this);
    add_module_easing(this);
#endif
}

}   // namespace pkpy

#endif // POCKETPY_H