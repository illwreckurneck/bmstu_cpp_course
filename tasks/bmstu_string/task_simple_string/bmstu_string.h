#pragma once

#include <exception>
#include <iostream>
#include <cstring>
#include <initializer_list>
#include <utility>
#include <algorithm>
#include <string>
#include <sstream>

namespace bmstu
{
template <typename T>
class simple_basic_string;

typedef simple_basic_string<char> string;
typedef simple_basic_string<wchar_t> wstring;
typedef simple_basic_string<char16_t> u16string;
typedef simple_basic_string<char32_t> u32string;

template <typename T>
class simple_basic_string
{
   public:
    /// Конструктор по умолчанию
    simple_basic_string() : ptr_(empty_buffer()), size_(0), capacity_(1) {}

    /// Конструктор с размером
    simple_basic_string(size_t size) : ptr_(new T[size + 1]), size_(size), capacity_(size + 1)
    {
        std::fill_n(ptr_, size, T(' '));
        ptr_[size] = 0;
    }

    /// Конструктор со списком инициализации
    simple_basic_string(std::initializer_list<T> il)
        : ptr_(new T[il.size() + 1]), size_(il.size()), capacity_(il.size() + 1)
    {
        std::copy(il.begin(), il.end(), ptr_);
        ptr_[size_] = 0;
    }

    /// Конструктор с параметром си-строки
    simple_basic_string(const T* c_str) 
    {
        size_ = strlen_(c_str);
        capacity_ = size_ + 1;
        ptr_ = new T[capacity_];
        std::copy(c_str, c_str + size_, ptr_);
        ptr_[size_] = 0;
    }

    /// Конструктор копирования
    simple_basic_string(const simple_basic_string& other)
        : ptr_(new T[other.capacity_]), size_(other.size_), capacity_(other.capacity_)
    {
        std::copy(other.ptr_, other.ptr_ + size_, ptr_);
        ptr_[size_] = 0;
    }

    /// Перемещающий конструктор
    simple_basic_string(simple_basic_string&& other) noexcept
        : ptr_(other.ptr_), size_(other.size_), capacity_(other.capacity_)
    {
        other.ptr_ = empty_buffer();
        other.size_ = 0;
        other.capacity_ = 1;
    }

    /// Деструктор
    ~simple_basic_string()
    {
        if (ptr_ != empty_buffer())
        {
            delete[] ptr_;
        }
    }

    /// Геттер на си-строку
    const T* c_str() const { return ptr_; }

    size_t size() const { return size_; }

    bool empty() const { return size_ == 0; }

    /// Оператор перемещающего присваивания
    simple_basic_string& operator=(simple_basic_string&& other) noexcept
    {
        if (this != &other)
        {
            if (ptr_ != empty_buffer())
            {
                delete[] ptr_;
            }
            ptr_ = other.ptr_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.ptr_ = empty_buffer();
            other.size_ = 0;
            other.capacity_ = 1;
        }
        return *this;
    }

    /// Оператор копирующего присваивания си-строки
    simple_basic_string& operator=(const T* c_str)
    {
        if (c_str == nullptr) {
            clear();
            return *this;
        }
        
        size_t new_size = strlen_(c_str);
        if (new_size + 1 > capacity_)
        {
            capacity_ = new_size + 1;
            T* new_ptr = new T[capacity_];
            if (ptr_ != empty_buffer())
            {
                delete[] ptr_;
            }
            ptr_ = new_ptr;
        }
        size_ = new_size;
        std::copy(c_str, c_str + size_, ptr_);
        ptr_[size_] = 0;
        return *this;
    }

    /// Оператор копирующего присваивания
    simple_basic_string& operator=(const simple_basic_string& other)
    {
        if (this != &other)
        {
            if (other.size_ + 1 > capacity_)
            {
                capacity_ = other.size_ + 1;
                T* new_ptr = new T[capacity_];
                if (ptr_ != empty_buffer())
                {
                    delete[] ptr_;
                }
                ptr_ = new_ptr;
            }
            size_ = other.size_;
            std::copy(other.ptr_, other.ptr_ + size_, ptr_);
            ptr_[size_] = 0;
        }
        return *this;
    }

    friend simple_basic_string<T> operator+(const simple_basic_string<T>& left,
                                            const simple_basic_string<T>& right)
    {
        simple_basic_string<T> result;
        result.size_ = left.size_ + right.size_;
        result.capacity_ = result.size_ + 1;
        result.ptr_ = new T[result.capacity_];
        std::copy(left.ptr_, left.ptr_ + left.size_, result.ptr_);
        std::copy(right.ptr_, right.ptr_ + right.size_, result.ptr_ + left.size_);
        result.ptr_[result.size_] = 0;
        return result;
    }

    template <typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits>& 
    operator<<(std::basic_ostream<CharT, Traits>& os, 
               const simple_basic_string& obj)
    {
        for (size_t i = 0; i < obj.size_; ++i)
        {
            os << obj.ptr_[i];
        }
        return os;
    }

    template <typename CharT, typename Traits>
    friend std::basic_istream<CharT, Traits>& 
    operator>>(std::basic_istream<CharT, Traits>& is, 
               simple_basic_string& obj)
    {
        // Читаем посимвольно до конца потока
        obj.clear();
        T ch;
        
        while (is.get(ch)) {
            obj += ch;
            // Если следующий символ - EOF, выходим
            if (is.peek() == EOF) {
                break;
            }
        }
        
        return is;
    }

    simple_basic_string& operator+=(const simple_basic_string& other)
    {
        if (this == &other) {
            // Самоприсваивание, создаем копию
            simple_basic_string copy = other;
            return *this += copy;
        }
        
        if (size_ + other.size_ + 1 > capacity_)
        {
            capacity_ = size_ + other.size_ + 1;
            T* new_ptr = new T[capacity_];
            std::copy(ptr_, ptr_ + size_, new_ptr);
            if (ptr_ != empty_buffer())
            {
                delete[] ptr_;
            }
            ptr_ = new_ptr;
        }
        std::copy(other.ptr_, other.ptr_ + other.size_, ptr_ + size_);
        size_ += other.size_;
        ptr_[size_] = 0;
        return *this;
    }

    simple_basic_string& operator+=(T symbol)
    {
        if (size_ + 2 > capacity_)
        {
            capacity_ = (capacity_ == 0) ? 2 : capacity_ * 2;
            T* new_ptr = new T[capacity_];
            std::copy(ptr_, ptr_ + size_, new_ptr);
            if (ptr_ != empty_buffer())
            {
                delete[] ptr_;
            }
            ptr_ = new_ptr;
        }
        ptr_[size_] = symbol;
        ++size_;
        ptr_[size_] = 0;
        return *this;
    }

    T& operator[](size_t index) noexcept
    {
        return ptr_[index];
    }

    const T& operator[](size_t index) const noexcept
    {
        return ptr_[index];
    }

    T& at(size_t index)
    {
        if (index >= size_)
        {
            throw std::out_of_range("Wrong index");
        }
        return ptr_[index];
    }

    const T& at(size_t index) const
    {
        if (index >= size_)
        {
            throw std::out_of_range("Wrong index");
        }
        return ptr_[index];
    }

    T* data() { return ptr_; }

    const T* data() const { return ptr_; }

    void clear()
    {
        size_ = 0;
        if (capacity_ > 0 && ptr_ != empty_buffer())
        {
            ptr_[0] = 0;
        }
    }

   private:
    static T* empty_buffer()
    {
        // Статический буфер для пустой строки
        static T empty[1] = {0};
        return empty;
    }
    
    static size_t strlen_(const T* str)
    {
        if (!str) return 0;
        size_t len = 0;
        while (str[len] != 0)
        {
            ++len;
        }
        return len;
    }

    T* ptr_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;
};
}  // namespace bmstu
