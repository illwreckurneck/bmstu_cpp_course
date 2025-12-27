#pragma once

#include <exception>
#include <utility>
#include <cstddef>
#include <new>

namespace bmstu
{
template <typename T>
class stack
{
private:
    T* data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;

    void reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) return;
        
        T* new_data = static_cast<T*>(::operator new(new_capacity * sizeof(T)));
        
        for (size_t i = 0; i < size_; ++i) {
            new (&new_data[i]) T(std::move(data_[i]));
            data_[i].~T();
        }
        
        ::operator delete(data_);
        data_ = new_data;
        capacity_ = new_capacity;
    }

public:
    stack() = default;
    
    ~stack() {
        for (size_t i = 0; i < size_; ++i) data_[i].~T();
        ::operator delete(data_);
    }
    
    stack(const stack& other) : data_(nullptr), size_(0), capacity_(0) {
        if (other.size_ > 0) {
            capacity_ = other.size_;
            data_ = static_cast<T*>(::operator new(capacity_ * sizeof(T)));
            for (size_t i = 0; i < other.size_; ++i) {
                new (&data_[size_++]) T(other.data_[i]);
            }
        }
    }
    
    stack& operator=(const stack& other) {
        if (this != &other) {
            for (size_t i = 0; i < size_; ++i) data_[i].~T();
            size_ = 0;
            
            if (capacity_ < other.size_) {
                ::operator delete(data_);
                capacity_ = other.size_;
                data_ = static_cast<T*>(::operator new(capacity_ * sizeof(T)));
            }
            
            for (size_t i = 0; i < other.size_; ++i) {
                new (&data_[size_++]) T(other.data_[i]);
            }
        }
        return *this;
    }
    
    stack(stack&& other) noexcept 
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }
    
    stack& operator=(stack&& other) noexcept {
        if (this != &other) {
            for (size_t i = 0; i < size_; ++i) data_[i].~T();
            ::operator delete(data_);
            
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }
    
    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }
    
    void clear() {
        for (size_t i = 0; i < size_; ++i) data_[i].~T();
        size_ = 0;
    }
    
    void push(const T& value) {
        if (size_ == capacity_) reserve(capacity_ + 1);  // ЛИНЕЙНЫЙ РОСТ!
        new (&data_[size_++]) T(value);
    }
    
    void push(T&& value) {
        if (size_ == capacity_) reserve(capacity_ + 1);  // ЛИНЕЙНЫЙ РОСТ!
        new (&data_[size_++]) T(std::move(value));
    }
    
    template <typename... Args>
    void emplace(Args&&... args) {
        if (size_ == capacity_) reserve(capacity_ + 1);  // ЛИНЕЙНЫЙ РОСТ!
        new (&data_[size_++]) T(std::forward<Args>(args)...);
    }
    
    void pop() {
        if (size_ == 0) throw std::underflow_error("Stack is empty");
        data_[--size_].~T();
    }
    
    T& top() {
        if (size_ == 0) throw std::underflow_error("Stack is empty");
        return data_[size_ - 1];
    }
    
    const T& top() const {
        if (size_ == 0) throw std::underflow_error("Stack is empty");
        return data_[size_ - 1];
    }
};
}
