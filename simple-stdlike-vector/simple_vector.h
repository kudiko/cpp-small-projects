#pragma once

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>

#include "array_ptr.h"


// enables SimpleVector(Reserve(value)) commands to construct SimpleVector
class ReserveProxyObj
{
    public:
    ReserveProxyObj(size_t reserve_value) : reserve_value_(reserve_value)
    {

    }
    size_t GetReserveValue() const noexcept
    {
        return reserve_value_;
    }
    private:
    size_t reserve_value_;

};

template <typename Type>
class SimpleVector {
private:
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
    // ArrayPtr has default constructor, with nullptr value in it
    ArrayPtr<Type> ptr_;

public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default; 

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : size_(size), capacity_(size), ptr_(size) 
    {
        for (auto it = begin(); it != end(); ++it)
        {
            Type default_value{};
            *it = std::move(default_value);
        }
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : size_(size), capacity_(size), ptr_(size) {
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : ptr_(init.size()) {
        std::copy(init.begin(), init.end(), begin());
        size_ = init.size();
        capacity_ = init.size();
    }

    // enables SimpleVector(Reserve(value)) commands to construct SimpleVector
    SimpleVector(const ReserveProxyObj& reserve_init) : SimpleVector(reserve_init.GetReserveValue())
    {
        Resize(0);
    }

    // copy ctor
    SimpleVector(const SimpleVector& source)
    {
        SimpleVector<Type> copy_of_source(source.capacity_);
        copy_of_source.Resize(source.size_);
        std::copy(source.begin(), source.end(), copy_of_source.begin());
        swap(copy_of_source);
    }

    // copy operator=
    SimpleVector& operator=(const SimpleVector& rhs) 
    {
        if (this == &rhs)
        {
            return *this;
        }
        SimpleVector<Type> tmp(rhs);
        swap(tmp);
        return *this;
    }

    // move ctor
    SimpleVector(SimpleVector&& other)
    {
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
        // ArrayPtr has moving operator= defined
        ptr_ = std::move(other.ptr_);
    }

    // moving = operator
    SimpleVector& operator=(SimpleVector&& other)
    {
        SimpleVector<Type> temp(other);
        swap(temp);
        return *this;
    }

    void swap(SimpleVector& other) noexcept
    {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        ptr_.swap(other.ptr_);
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_)
        {
            throw std::out_of_range("Wrong index entered");
        }
        return ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_)
        {
            throw std::out_of_range("Wrong index entered");
        }
        return ptr_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size > capacity_)
        {
            std::size_t new_capacity = std::max(capacity_ * 2,  new_size);
           
            SimpleVector<Type> temp(new_capacity);
            temp.Resize(new_size);
            std::move(begin(), end(), temp.begin());
            
            this->capacity_ = temp.capacity_;
            this->size_ = temp.size_;
            ptr_.swap(temp.ptr_);
            return;
        }
        // if size_ < new_size and new_size <= capacity_ all other elements in capacity are already default initialized
        size_ = new_size;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return ptr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return ptr_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return cbegin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return cend();
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return ptr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return ptr_.Get() + size_;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    
    void PushBack(const Type& item) {
        if (size_ == capacity_)
        {
            Resize(size_ + 1);
            ptr_[size_ - 1] = item;
            return;
        }
        ptr_[size_] = item;
        ++size_;
    }

    //moving PushBack
    void PushBack(Type&& item)
    {
        if (size_ == capacity_)
        {
            Resize(size_ + 1);
            ptr_[size_ - 1] = std::move(item);
            return;
        }
        ptr_[size_] = std::move(item);
        ++size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        Iterator non_const_pos = const_cast<Iterator>(pos);
        if (size_ == capacity_)
        {
            std::size_t new_capacity = std::max(capacity_ * 2,  size_ + 1);
            SimpleVector<Type> temp(new_capacity);
            temp.Resize(size_ + 1);
            auto paste_it = std::copy(begin(), non_const_pos, temp.begin());
            *paste_it = value;
            std::copy(non_const_pos, end(), std::next(paste_it, 1));
            
            this->capacity_ = temp.capacity_;
            this->size_ = temp.size_;
            ptr_.swap(temp.ptr_);
            return paste_it;
        }
        ++size_;
        std::copy_backward(non_const_pos, std::next(end(), -1), end());
        *non_const_pos = value;
        return non_const_pos;
    }

    // move_friendly insert
    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        Iterator non_const_pos = const_cast<Iterator>(pos);
        if (size_ == capacity_)
        {
            std::size_t new_capacity = std::max(capacity_ * 2,  size_ + 1);
            SimpleVector<Type> temp(new_capacity);
            temp.Resize(size_ + 1);
            auto paste_it = std::move(begin(), non_const_pos, temp.begin());
            *paste_it = std::move(value);
            std::move(non_const_pos, end(), std::next(paste_it, 1));
            
            this->capacity_ = temp.capacity_;
            this->size_ = temp.size_;
            ptr_.swap(temp.ptr_);
            return paste_it;
        }
        ++size_;
        std::move_backward(non_const_pos, std::next(end(), -1), end());
        *non_const_pos = std::move(value);
        return non_const_pos;
    }
    

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
    }

    // move-friendly Erase
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        Iterator non_const_pos = const_cast<Iterator>(pos);
        std::move(std::next(non_const_pos, 1), end(), non_const_pos);
        --size_;
        return non_const_pos;
    }

    // move-friendly Reserve
    void Reserve(size_t new_capacity)
    {   
        if (new_capacity > capacity_)
        {
            SimpleVector<Type> temp(new_capacity);
            temp.Resize(size_);
            std::move(begin(), end(), temp.begin());
            
            this->capacity_ = temp.capacity_;
            this->size_ = temp.size_;
            ptr_.swap(temp.ptr_);
            return;
        }
    }
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize())
    {
        return false;
    }
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs < rhs) || (lhs == rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs <= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
} 

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}
