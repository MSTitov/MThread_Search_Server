// вставьте сюда ваш код для класса SimpleVector
// внесите необходимые изменения для поддержки move-семантики
#pragma once

#include "array_ptr.h"
#include <algorithm>
#include <cassert>
#include <stdexcept>

struct ReserveProxyObj {
    explicit ReserveProxyObj(size_t capacity_to_reserve)
        : capacity(capacity_to_reserve) {
    }
    size_t capacity;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
    using ItemsPtr = ArrayPtr<Type>;

public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : SimpleVector(size, Type{})  // Делегируем инициализацию конструктору, принимающему size и value
    {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : items_(size)  // может бросить исключение
        , size_(size)
        , capacity_(size)  //
    {
        std::fill(items_.Get(), items_.Get() + size_, value);  // Может бросить исключение
    }

    // Создаёт вектор из initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : items_(init.size())  // Может бросить исключение
        , size_(init.size())
        , capacity_(init.size())  //
    {
        std::copy(init.begin(), init.end(), items_.Get());
    }

    SimpleVector(ReserveProxyObj reserved)
        : items_(reserved.capacity)
        , size_(0)
        , capacity_(reserved.capacity) {
    }

    SimpleVector(const SimpleVector& other)
        : items_(other.size_)  // может бросить исключение
        , size_(other.size_)
        , capacity_(other.size_)  //
    {
        std::copy(other.items_.Get(), other.items_.Get() + size_, items_.Get());  // может бросить исключение
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (&rhs != this) {  // оптимизация присваивания вектора самому себе
            if (rhs.IsEmpty()) {
                // Оптимизация для случая присваивания пустого вектора
                Clear();
            }
            else {
                // Применяем идиому Copy-and-swap
                SimpleVector rhs_copy(rhs);  // может бросить исключение
                swap(rhs_copy);
            }
        }
        return *this;
    }

    SimpleVector(SimpleVector&& other) noexcept
        : items_(other.size_)  // может бросить исключение
        , size_(other.size_)
        , capacity_(other.size_)  //
    {
        std::copy(std::make_move_iterator(other.items_.Get()), std::make_move_iterator(other.items_.Get() + size_),
            items_.Get());
        other.size_ = 0;
        other.capacity_ = 0;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept
    {
        if (&rhs != this)
        {
            if (rhs.IsEmpty()) {
                // Оптимизация для случая присваивания пустого вектора
                Clear();
            }
            else {
                size_ = rhs.size_;
                capacity_ = rhs.capacity_;
                rhs.size_ = 0;
                rhs.capacity_ = 0;
            }
        }
        return *this;
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
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            using namespace std;
            throw out_of_range("Item index is out of range"s);
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            using namespace std;
            throw out_of_range("Item index is out of range"s);
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            auto new_items = ReallocateCopy(new_capacity);  // может бросить исключение

            items_.swap(new_items);
            capacity_ = new_capacity;
        }
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            // Вычисляем вместимость вектора
            const size_t new_capacity = std::max(capacity_ * 2, new_size);

            // Копируем существующие элементы вектора на новое местое
            auto new_items = ReallocateCopy(new_capacity);  // может бросить исключение
            // заполняем добавленные элементы значением по умолчанию
            std::fill(new_items.Get() + size_, new_items.Get() + new_size, std::move(Type{}));  // может бросить исключение

            items_.swap(new_items);
            capacity_ = new_capacity;
        }
        else if (new_size > size_) {
            assert(new_size <= capacity_);
            std::fill(items_.Get() + size_, items_.Get() + new_size, Type{});  // может бросить исключение
        }
        size_ = new_size;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(Type item) {
        const size_t new_size = size_ + 1;
        if (new_size > capacity_) {
            const size_t new_capacity = std::max(capacity_ * 2, new_size);

            auto new_items = ReallocateCopy(new_capacity);  // может бросить исключение
            new_items.Get()[size_] = std::move(item);                  // может бросить исключение

            capacity_ = new_capacity;
            items_.swap(new_items);
        }
        else {
            items_.Get()[size_] = std::move(item);
        }
        size_ = new_size;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(Iterator pos, Type value) {
        assert(begin() <= pos && pos <= end());
        size_t new_size = size_ + 1;
        size_t new_item_offset = pos - cbegin();
        if (new_size <= capacity_) {  // Вместимость вектора достаточна для вставки элемента
            Iterator mutable_pos = begin() + new_item_offset;

            // Копируем элементы, начиная с последнего, чтобы перенести "хвост" вправо
            std::copy_backward(std::make_move_iterator(pos), std::make_move_iterator(end()),
                end() + 1);  // может выбросить исключение
            *mutable_pos = std::move(value);           // может выбросить исключение

        }
        else {  // Требуется перевыделить память
            size_t new_capacity = std::max(capacity_ * 2, new_size);

            ItemsPtr new_items(new_capacity);  // может выбросить исключение
            Iterator new_items_pos = new_items.Get() + new_item_offset;

            // Копируем элементы, предшествующие вставляемому
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(pos),
                new_items.Get());  // может выбросить исключение
// Вставляем элемент в позицию вставки
            *new_items_pos = std::move(value);  // может выбросить исключение
            // Копируем элементы следующие за вставляемым
            std::copy(std::make_move_iterator(pos), std::make_move_iterator(end()),
                new_items_pos + 1);  // может выбросить исключение

            items_.swap(new_items);
            capacity_ = new_capacity;
        }
        size_ = new_size;
        return begin() + new_item_offset;
    }

    // "Удаляет" элемент с конца вектора, не уменьшая его вместимость
    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
    }

    // Удаляет элемент вектора в позиции pos, сдвигая следующие элементы на его место.
    // Итератор pos должен быть итератором, ссылающимся на существующий элемент вектора.
    // Возвращает итератор на элемент, который следует за последним удалённым элементом.
    // Если был удалён последний элемент, должен вернуться итератор end().
    Iterator Erase(Iterator pos) {
        assert(begin() <= pos && pos < end());
        Iterator mutable_pos = begin() + (pos - begin());

        // Переносим "хвост" влево на места удаляемого элемента
        std::copy(std::make_move_iterator(mutable_pos + 1), std::make_move_iterator(end()),
            mutable_pos);  // может выбросить исключение

        --size_;
        return mutable_pos;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }

private:
    // Выделяет копию текущего массива с заданной вместимостью
    ItemsPtr ReallocateCopy(size_t new_capacity) const {
        ItemsPtr new_items(new_capacity);  // может бросить исключение
        size_t copy_size = std::min(new_capacity, size_);
        std::copy(std::make_move_iterator(items_.Get()), std::make_move_iterator(items_.Get() + copy_size),
            new_items.Get());  // может бросить исключение
        return ItemsPtr(new_items.Release());
    }

    ItemsPtr items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs.GetSize() == rhs.GetSize())
        && std::equal(lhs.begin(), lhs.end(), rhs.begin());  // может бросить исключение
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);  // может бросить исключение
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());  // может бросить исключение
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);  // может бросить исключение
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;  // может бросить исключение
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs <= lhs;  // может бросить исключение
}