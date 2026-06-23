#pragma once
#include <cstdint>
#include <typeinfo>
#include <memory>
#include <octave/oct.h>

// Boxes a heap-allocated C++ object pointer as an octave_uint64 scalar.
// The signature (MAGIC ^ typeid hash) guards against stale or mistyped handles.

static const uint64_t HANDLE_MAGIC = 0xDEADBEEFC0FFEE00ULL;

template <typename T>
class class_handle
{
public:
    explicit class_handle(T *p, bool own_obj = true) : sig_(make_sig()), ptr_(p), own_obj_(own_obj)
    {
    }

    ~class_handle()
    {
        sig_ = 0;
        if (own_obj_)
            delete ptr_;
        ptr_ = nullptr;
    }

    bool is_valid() const { return ptr_ != nullptr && sig_ == make_sig(); }
    T *ptr() { return ptr_; }

private:
    uint64_t make_sig() const
    {
        return HANDLE_MAGIC ^ static_cast<uint64_t>(typeid(T).hash_code());
    }

    uint64_t sig_;
    T *ptr_;
    bool own_obj_;
};

template <typename T>
class shared_class_handle
{
public:
    typedef std::shared_ptr<T> ptr_t;

    explicit shared_class_handle(ptr_t p) : sig_(make_sig()), ptr_(p) { }

    bool is_valid() const { return ptr_ && sig_ == make_sig(); }
    T *ptr() { return ptr_.get(); }
    ptr_t shared_ptr() { return ptr_; }

private:
    uint64_t make_sig() const
    {
        return HANDLE_MAGIC ^ static_cast<uint64_t>(typeid(T).hash_code());
    }

    uint64_t sig_;
    ptr_t ptr_;
};

// Create a handle from a heap-allocated object pointer; returns octave_uint64.
template <typename T>
octave_value handle_create(T *ptr, bool ownit = true)
{
    uint64_t raw = reinterpret_cast<uint64_t>(new class_handle<T>(ptr, ownit));
    return octave_value(octave_uint64(raw));
}

// Create a handle from a heap-allocated shared pointer; returns octave_uint64.
template <typename T>
octave_value shared_handle_create(std::shared_ptr<T> ptr)
{
    uint64_t raw = reinterpret_cast<uint64_t>(new shared_class_handle<T>(ptr));
    return octave_value(octave_uint64(raw));
}

// Extract the class_handle<T>* from an octave_value; errors if invalid.
template <typename T>
class_handle<T> *handle_cast(const octave_value &val)
{
    if (!val.is_uint64_type() || !val.is_scalar_type())
        error("invalid handle: expected uint64 scalar");

    uint64_t raw = val.uint64_value();
    if (raw == 0)
        error("null handle");

    auto *h = reinterpret_cast<class_handle<T> *>(raw);
    if (!h->is_valid())
        error("invalid or stale handle (wrong type or already deleted)");

    return h;
}

template <typename T>
shared_class_handle<T> *shared_handle_cast(const octave_value &val)
{
    if (!val.is_uint64_type() || !val.is_scalar_type())
        error("invalid handle: expected uint64 scalar");

    uint64_t raw = val.uint64_value();
    if (raw == 0)
        error("null handle");

    auto *h = reinterpret_cast<shared_class_handle<T> *>(raw);
    if (!h->is_valid())
        error("invalid or stale handle (wrong type or already deleted)");

    return h;
}

// Return the wrapped C++ pointer; errors if handle is invalid.
template <typename T>
T *handle_obj(const octave_value &val)
{
    return handle_cast<T>(val)->ptr();
}
template <typename T>
typename shared_class_handle<T>::ptr_t shared_handle_obj(const octave_value &val)
{
    return shared_handle_cast<T>(val)->shared_ptr();
}

// Delete the C++ object and the handle wrapper.
// The caller must zero its stored octave_uint64 afterward.
template <typename T>
void handle_destroy(const octave_value &val)
{
    delete handle_cast<T>(val);
}
template <typename T>
void shared_handle_destroy(const octave_value &val)
{
    delete shared_handle_cast<T>(val);
}
