#pragma once

#include <exception>
#include <iostream>
#include <algorithm>
#include <utility>
#include <cstring>

namespace bmstu {
    template<typename T>
    class basic_string {
    private:
        static constexpr size_t SSO_SIZE = (sizeof(T*) + 2 * sizeof(size_t)) / sizeof(T) - 1;
        
        union Storage {
            struct Long {
                T* ptr;
                size_t size;
                size_t capacity;
            } long_;
            
            struct Short {
                T data[SSO_SIZE + 1];
                unsigned char size;
            } short_;
            
            Storage() : short_{} {}
        };
        
        Storage storage_;
        bool is_long_;
        
        bool is_long() const { return is_long_; }
        
        T* data() { 
            return is_long() ? storage_.long_.ptr : storage_.short_.data; 
        }
        
        const T* data() const { 
            return is_long() ? storage_.long_.ptr : storage_.short_.data; 
        }
        
        size_t& size_ref() { 
            return is_long() ? storage_.long_.size : 
                reinterpret_cast<size_t&>(storage_.short_.size); 
        }
        
        size_t size_val() const { 
            return is_long() ? storage_.long_.size : storage_.short_.size; 
        }
        
        void destroy() {
            if (is_long()) delete[] storage_.long_.ptr;
        }
        
        void copy_from(const T* str, size_t n) {
            destroy();
            
            if (n <= SSO_SIZE) {
                is_long_ = false;
                std::copy_n(str, n, storage_.short_.data);
                storage_.short_.size = static_cast<unsigned char>(n);
                storage_.short_.data[n] = 0;
            } else {
                is_long_ = true;
                storage_.long_.ptr = new T[n + 1];
                storage_.long_.size = n;
                storage_.long_.capacity = n + 1;
                std::copy_n(str, n, storage_.long_.ptr);
                storage_.long_.ptr[n] = 0;
            }
        }
        
        void move_from(basic_string&& other) {
            if (other.is_long()) {
                is_long_ = true;
                storage_.long_ = other.storage_.long_;
                other.storage_.long_.ptr = nullptr;
            } else {
                is_long_ = false;
                storage_.short_ = other.storage_.short_;
            }
            other.is_long_ = false;
            other.storage_.short_.size = 0;
            other.storage_.short_.data[0] = 0;
        }
        
        void ensure_capacity(size_t n) {
            if (n <= capacity()) return;
            
            if (is_long()) {
                size_t new_cap = std::max(storage_.long_.capacity * 2, n + 1);
                T* new_ptr = new T[new_cap];
                std::copy_n(storage_.long_.ptr, size_val(), new_ptr);
                delete[] storage_.long_.ptr;
                storage_.long_.ptr = new_ptr;
                storage_.long_.capacity = new_cap;
            } else {
                size_t new_cap = n + 1;
                T* new_ptr = new T[new_cap];
                std::copy_n(storage_.short_.data, size_val(), new_ptr);
                is_long_ = true;
                storage_.long_.ptr = new_ptr;
                storage_.long_.size = size_val();
                storage_.long_.capacity = new_cap;
            }
        }
        
        static size_t str_len(const T* str) {
            if (!str) return 0;
            size_t len = 0;
            while (str[len]) ++len;
            return len;
        }
        
    public:
        basic_string() : is_long_(false) {
            storage_.short_.size = 0;
            storage_.short_.data[0] = 0;
        }
        
        basic_string(size_t n, T ch = ' ') {
            if (n <= SSO_SIZE) {
                is_long_ = false;
                std::fill_n(storage_.short_.data, n, ch);
                storage_.short_.size = static_cast<unsigned char>(n);
                storage_.short_.data[n] = 0;
            } else {
                is_long_ = true;
                storage_.long_.ptr = new T[n + 1];
                storage_.long_.size = n;
                storage_.long_.capacity = n + 1;
                std::fill_n(storage_.long_.ptr, n, ch);
                storage_.long_.ptr[n] = 0;
            }
        }
        
        basic_string(const T* str) {
            copy_from(str, str_len(str));
        }
        
        basic_string(const basic_string& other) {
            copy_from(other.data(), other.size_val());
        }
        
        basic_string(basic_string&& other) noexcept {
            move_from(std::move(other));
        }
        
        ~basic_string() { destroy(); }
        
        basic_string& operator=(basic_string other) {
            swap(other);
            return *this;
        }
        
        basic_string& operator=(const T* str) {
            copy_from(str, str_len(str));
            return *this;
        }
        
        void swap(basic_string& other) noexcept {
            std::swap(storage_, other.storage_);
            std::swap(is_long_, other.is_long_);
        }
        
        const T* c_str() const { return data(); }
        size_t size() const { return size_val(); }
        bool empty() const { return size_val() == 0; }
        
        size_t capacity() const {
            return is_long() ? storage_.long_.capacity : SSO_SIZE;
        }
        
        friend basic_string operator+(const basic_string& a, const basic_string& b) {
            basic_string result;
            result.reserve(a.size() + b.size());
            result += a;
            result += b;
            return result;
        }
        
        basic_string& operator+=(const basic_string& other) {
            size_t old_size = size();
            ensure_capacity(old_size + other.size());
            std::copy_n(other.data(), other.size(), data() + old_size);
            size_ref() = old_size + other.size();
            data()[size()] = 0;
            return *this;
        }
        
        basic_string& operator+=(T ch) {
            size_t new_size = size() + 1;
            ensure_capacity(new_size);
            data()[size()] = ch;
            size_ref() = new_size;
            data()[new_size] = 0;
            return *this;
        }
        
        void reserve(size_t n) {
            ensure_capacity(n);
        }
        
        void clear() {
            size_ref() = 0;
            if (data()) data()[0] = 0;
        }
        
        T& operator[](size_t i) { return data()[i]; }
        const T& operator[](size_t i) const { return data()[i]; }
        
        T& at(size_t i) {
            if (i >= size()) throw std::out_of_range("Index out of range");
            return data()[i];
        }
        
        const T& at(size_t i) const {
            if (i >= size()) throw std::out_of_range("Index out of range");
            return data()[i];
        }
        
        friend std::ostream& operator<<(std::ostream& os, const basic_string& str) {
            return os.write(str.data(), str.size());
        }
        
        friend std::istream& operator>>(std::istream& is, basic_string& str) {
            str.clear();
            T ch;
            while (is.get(ch) && !std::isspace(ch)) {
                str += ch;
            }
            return is;
        }
    };
    
    using string = basic_string<char>;
    using wstring = basic_string<wchar_t>;
}

