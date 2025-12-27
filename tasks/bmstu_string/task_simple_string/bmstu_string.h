#pragma once

#include <exception>
#include <iostream>
#include <utility>
#include <algorithm>
#include <initializer_list>

namespace bmstu {
    template<typename T>
    class simple_basic_string {
    public:
        simple_basic_string() : data_(new T[1]), size_(0), capacity_(1) {
            data_[0] = 0;
        }

        simple_basic_string(size_t size) : simple_basic_string(size, ' ') {}

        simple_basic_string(std::initializer_list<T> il) 
            : data_(new T[il.size() + 1]), size_(il.size()), capacity_(il.size() + 1) {
            std::copy(il.begin(), il.end(), data_);
            data_[size_] = 0;
        }

        simple_basic_string(const T* str) : simple_basic_string() {
            if (str) *this = str;
        }

        simple_basic_string(const simple_basic_string& other) 
            : data_(new T[other.capacity_]), size_(other.size_), capacity_(other.capacity_) {
            std::copy_n(other.data_, size_ + 1, data_);
        }

        simple_basic_string(simple_basic_string&& other) noexcept
            : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
            other.reset();
        }

        ~simple_basic_string() {
            delete[] data_;
        }

        simple_basic_string& operator=(simple_basic_string other) noexcept {
            swap(*this, other);
            return *this;
        }

        simple_basic_string& operator=(const T* str) {
            if (!str) {
                clear();
                return *this;
            }
            size_t len = length(str);
            reserve(len + 1);
            size_ = len;
            std::copy_n(str, len, data_);
            data_[len] = 0;
            return *this;
        }

        friend void swap(simple_basic_string& a, simple_basic_string& b) noexcept {
            std::swap(a.data_, b.data_);
            std::swap(a.size_, b.size_);
            std::swap(a.capacity_, b.capacity_);
        }

        const T* c_str() const { return data_; }
        size_t size() const { return size_; }
        bool empty() const { return size_ == 0; }
        T* data() { return data_; }
        const T* data() const { return data_; }

        friend simple_basic_string operator+(const simple_basic_string& lhs, const simple_basic_string& rhs) {
            simple_basic_string result;
            result.reserve(lhs.size_ + rhs.size_ + 1);
            std::copy_n(lhs.data_, lhs.size_, result.data_);
            std::copy_n(rhs.data_, rhs.size_, result.data_ + lhs.size_);
            result.size_ = lhs.size_ + rhs.size_;
            result.data_[result.size_] = 0;
            return result;
        }

        template<typename CharT, typename Traits>
        friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, 
                                                             const simple_basic_string& str) {
            return os.write(str.data_, str.size_);
        }

        template<typename CharT, typename Traits>
        friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, 
                                                             simple_basic_string& str) {
            str.clear();
            T ch;
            while (is.get(ch) && !std::isspace(ch, is.getloc())) {
                str += ch;
            }
            return is;
        }

        simple_basic_string& operator+=(const simple_basic_string& other) {
            reserve(size_ + other.size_ + 1);
            std::copy_n(other.data_, other.size_, data_ + size_);
            size_ += other.size_;
            data_[size_] = 0;
            return *this;
        }

        simple_basic_string& operator+=(T ch) {
            reserve(size_ + 2);
            data_[size_++] = ch;
            data_[size_] = 0;
            return *this;
        }

        T& operator[](size_t i) noexcept { return data_[i]; }
        const T& operator[](size_t i) const noexcept { return data_[i]; }

        T& at(size_t i) {
            if (i >= size_) throw std::out_of_range("Index out of range");
            return data_[i];
        }

        const T& at(size_t i) const {
            if (i >= size_) throw std::out_of_range("Index out of range");
            return data_[i];
        }

        void clear() {
            size_ = 0;
            if (capacity_ > 0) data_[0] = 0;
        }

        void reserve(size_t new_capacity) {
            if (new_capacity <= capacity_) return;
            T* new_data = new T[new_capacity];
            std::copy_n(data_, size_ + 1, new_data);
            delete[] data_;
            data_ = new_data;
            capacity_ = new_capacity;
        }

    private:
        simple_basic_string(size_t size, T fill) 
            : data_(new T[size + 1]), size_(size), capacity_(size + 1) {
            std::fill_n(data_, size, fill);
            data_[size] = 0;
        }

        void reset() {
            data_ = new T[1];
            data_[0] = 0;
            size_ = 0;
            capacity_ = 1;
        }

        static size_t length(const T* str) {
            const T* end = str;
            while (*end) ++end;
            return end - str;
        }

        T* data_;
        size_t size_;
        size_t capacity_;
    };

    typedef simple_basic_string<char> string;
    typedef simple_basic_string<wchar_t> wstring;
    typedef simple_basic_string<char16_t> u16string;
    typedef simple_basic_string<char32_t> u32string;
}