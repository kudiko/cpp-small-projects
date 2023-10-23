#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
#include <memory>

template <typename T>
class RawMemory {
public:
    RawMemory() = default;

    explicit RawMemory(size_t capacity)
            : buffer_(Allocate(capacity))
            , capacity_(capacity) {
    }

    RawMemory(const RawMemory&) = delete;
    RawMemory& operator=(const RawMemory& rhs) = delete;
    RawMemory(RawMemory&& other) noexcept
    {
        buffer_ = std::move(other.buffer_);
        capacity_ = std::move(other.capacity_);
        other.buffer_ = nullptr;
        other.capacity_ = 0;
    }
    RawMemory& operator=(RawMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            RawMemory new_rawmem(std::move(rhs));
            Swap(new_rawmem);
        }
        return *this;
    }

    ~RawMemory() {
        Deallocate(buffer_);
    }

    T* operator+(size_t offset) noexcept {
        // Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
        assert(offset <= capacity_);
        return buffer_ + offset;
    }

    const T* operator+(size_t offset) const noexcept {
        return const_cast<RawMemory&>(*this) + offset;
    }

    const T& operator[](size_t index) const noexcept {
        return const_cast<RawMemory&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < capacity_);
        return buffer_[index];
    }

    void Swap(RawMemory& other) noexcept {
        std::swap(buffer_, other.buffer_);
        std::swap(capacity_, other.capacity_);
    }

    const T* GetAddress() const noexcept {
        return buffer_;
    }

    T* GetAddress() noexcept {
        return buffer_;
    }

    size_t Capacity() const {
        return capacity_;
    }

private:
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T* Allocate(size_t n) {
        return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
    }

    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T* buf) noexcept {
        operator delete(buf);
    }

    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};

template <typename T>
class Vector {
public:
    Vector() noexcept = default;

    Vector(size_t size) : data_{size}, size_{size}
    {
        std::uninitialized_value_construct_n(data_.GetAddress(), size);
    }

    Vector(const Vector& other): data_{other.size_}, size_{other.size_}
    {
        std::uninitialized_copy_n(other.data_.GetAddress(), other.size_, data_.GetAddress());
    }

    Vector(Vector&& other) noexcept
    {
        data_ = std::move(other.data_);
        size_ = std::move(other.size_);
        other.size_ = 0;
    }

    Vector& operator=(Vector&& other) noexcept
    {
        if (this != &other)
        {
            data_ = std::move(other.data_);
            size_ = std::move(other.size_);
            other.size_ = 0;
        }
        return *this;
    }

    Vector& operator=(const Vector& other)
    {
        if (this != &other)
        {
            if (other.size_ > data_.Capacity())
            {
                Vector temp(other);
                Swap(temp);
            } else {
                if (other.size_ < size_)
                {
                    for (size_t i = 0; i < other.size_; ++i)
                    {
                        this->operator[](i) = other[i];
                    }
                    for (size_t i = other.size_; i < size_ ; ++i)
                    {
                        this->operator[](i).~T();
                    }
                } else {
                    for (size_t i = 0; i < size_; ++i)
                    {
                        this->operator[](i) = other[i];
                    }
                    std::uninitialized_copy_n(other.data_ + size_, other.size_ - size_, data_ + size_);

                }
                size_ = other.size_;
            }
        }
        return *this;
    }

    size_t Size() const noexcept {
        return size_;
    }

    size_t Capacity() const noexcept {
        return data_.Capacity();
    }

    void Resize(size_t new_size)
    {
        if (new_size > size_)
        {
            Reserve(new_size);
            std::uninitialized_value_construct_n(data_ + size_, new_size - size_);
            size_ = new_size;
        } else {
            for (size_t i = new_size; i < size_; ++i)
            {
                this->operator[](i).~T();
            }
            size_ = new_size;
        }
    }

    void PushBack(const T& value)
    {
        EmplaceBack(value);

    }
    void PushBack(T&& value)
    {
        EmplaceBack(std::move(value));
    }

    template <typename... Types>
    T& EmplaceBack(Types&&... par_pack)
    {
        T* ptr_ = nullptr;
        if (size_ == Capacity())
        {
            RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);

            ptr_ = new (new_data + size_) T(std::forward<Types>(par_pack)...);

            FullCopyOrMoveToNewDestination(new_data, data_, size_);

        } else {
            ptr_ = new (data_ + size_) T(std::forward<Types>(par_pack)...);
        }
        ++size_;

        return *ptr_;
    }

    void PopBack() noexcept
    {
        this->operator[](size_ - 1).~T();
        --size_;
    }

    void Reserve(size_t new_capacity)
    {
        if (new_capacity <= data_.Capacity())
        {
            return;
        }

        RawMemory<T> new_data(new_capacity);

        FullCopyOrMoveToNewDestination(new_data, data_, size_);
    }

    const T& operator[](size_t index) const noexcept {
        return const_cast<Vector&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < size_);
        return data_[index];
    }

    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() noexcept
    {
        return data_.GetAddress();
    }
    iterator end() noexcept
    {
        return data_ + size_;
    }
    const_iterator begin() const noexcept
    {
        return data_.GetAddress();
    }
    const_iterator end() const noexcept
    {
        return data_ + size_;
    }
    const_iterator cbegin() const noexcept
    {
        return begin();
    }
    const_iterator cend() const noexcept
    {
        return end();
    }

    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args)
    {
        T* ptr_ = nullptr;

        if (size_ == Capacity())
        {
            RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
            size_t offset = pos - data_.GetAddress();
            ptr_ = new (new_data + offset) T(std::forward<Args>(args)...);
            PartialCopyOrMoveToNewDestination(new_data.GetAddress(), data_.GetAddress(), offset);
            PartialCopyOrMoveToNewDestination(new_data + offset + 1, data_ + offset, size_ - offset);

            std::destroy_n(data_.GetAddress(), size_);
            data_.Swap(new_data);

        } else {
            if (size_ == 0)
            {
                ptr_ = new (data_ + size_) T(std::forward<Args>(args)...);
            } else {
                *(data_ + size_) = std::move(*(data_ + (size_ - 1)));
                ptr_ = std::prev(std::move_backward(const_cast<iterator>(pos), std::prev(end()), end()));

                T temp(std::forward<Args>(args)...);
                new (ptr_) T(std::move(temp));

            }

        }

        ++size_;
        return ptr_;

    }
    iterator Erase(const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        iterator pos_no_const = const_cast<iterator>(pos);
        std::move(std::next(pos_no_const, 1), end(), pos_no_const);
        this->operator[](size_ - 1).~T();
        --size_;
        return pos_no_const;
    }
    iterator Insert(const_iterator pos, const T& value)
    {
        return Emplace(pos, value);
    }
    iterator Insert(const_iterator pos, T&& value)
    {
        return Emplace(pos, std::move(value));
    }


    void Swap(Vector& other) noexcept
    {
        data_.Swap(other.data_);
        std::swap(other.size_, size_);
    }

    ~Vector()
    {
        std::destroy_n(data_.GetAddress(), size_);
    }

private:
    RawMemory<T> data_;
    size_t size_ = 0;

    static void FullCopyOrMoveToNewDestination(RawMemory<T>& dest, RawMemory<T>& src, size_t old_size)
    {
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            std::uninitialized_move_n(src.GetAddress(), old_size, dest.GetAddress());
        } else {
            std::uninitialized_copy_n(src.GetAddress(), old_size, dest.GetAddress());
        }
        std::destroy_n(src.GetAddress(), old_size);
        src.Swap(dest);
    }

    static void PartialCopyOrMoveToNewDestination(T* dest_ptr, T* src_ptr, size_t chunk_size)
    {
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            std::uninitialized_move_n(src_ptr, chunk_size, dest_ptr);
        } else {
            std::uninitialized_copy_n(src_ptr, chunk_size, dest_ptr);
        }
    }
};