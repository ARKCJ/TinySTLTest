#pragma once
#include<stddef.h>
#include"OutOfRange.h"

namespace test{
    
    template<typename Array>
    class ArrayIterator
    {
    public:
        using ValueType = typename Array::ValueType;
        using RefType = typename Array::ValueType&;
        using PointerType = typename Array::ValueType*;
        using ConstRefType = const RefType;
        using ConstPointerType = const PointerType;
    private:
        mutable PointerType m_ptr;
    public:
        ArrayIterator(PointerType ptr):m_ptr(ptr){}
        
        ArrayIterator& operator++()
        {
            return const_cast<ArrayIterator&>(++(*const_cast<const ArrayIterator*>(this)));
        }

        ArrayIterator operator++(int)
        {
            return const_cast<ArrayIterator>((*const_cast<const ArrayIterator*>(this))++);
        }

        const ArrayIterator& operator++() const
        {
            ++m_ptr;
            return *this;
        }

        const ArrayIterator operator++(int) const
        {
            ArrayIterator tmp = *this;
            operator++();
            return tmp;
        }

        ArrayIterator& operator--()
        {
            return const_cast<ArrayIterator&>(--(*const_cast<const ArrayIterator*>(this)));;
        }

        ArrayIterator operator--(int)
        {
            return const_cast<ArrayIterator>((*const_cast<const ArrayIterator*>(this))--);
        }

        ArrayIterator& operator--() const
        {
            --m_ptr;
            return *this;
        }

        ArrayIterator operator--(int) const
        {
            ArrayIterator tmp = *this;
            operator--();
            return tmp;
        }

        PointerType operator->()
        {
            return const_cast<PointerType>((*const_cast<const ArrayIterator*>(this)).operator->());
        }

        ConstPointerType operator->() const
        {
            return m_ptr;
        }

        RefType operator*()
        {
            return const_cast<RefType>((*const_cast<const ArrayIterator*>(this)).operator*());
        }

        ConstRefType operator*() const
        {
            return *m_ptr;
        }

        bool operator==(const ArrayIterator& other) const
        {
            return m_ptr == other.m_ptr;
        }

        bool operator!=(const ArrayIterator& other) const
        {
            return !(*this == other);
        }

    };

    template<typename T, size_t size>
    class Array
    {
    public:
        using ValueType = T;
        typedef ValueType& RefType;
        typedef const ValueType& ConstRefType;
        using Iterator = ArrayIterator<Array<T, size>>;
        typedef const Iterator ConstIterator;
    private:
        ValueType m_Data[size];
    public:
        Array() = default;
        template<typename R>
        Array(const std::initializer_list<R>& list)
        {
            auto iter1 = begin();
            for(auto iter2 = list.begin(); iter1 != end() && iter2 != list.end(); ++iter1, ++iter2){
                *iter1 = *iter2;
            }
        }
        template<typename R>
        Array(std::initializer_list<R>&& list)
        {
            auto iter1 = begin();
            for(auto iter2 = list.begin(); iter1 != end() && iter2 != list.end(); ++iter1, ++iter2){
                *iter1 = std::move(*iter2);
            }
        }
        RefType operator[](size_t s)
        {
            return const_cast<RefType>((*const_cast<const Array<T, size>*>(this))[s]);
        }

        ConstRefType operator[](size_t s) const
        {
#ifndef RELEASE
            if(s < 0 || s >= size){
                throw test::OutOfRange(size, s);
            }
#endif
            return m_Data[s];
        }

        T* Data(){ return m_Data; }
        const T* Data() const { return m_Data; }
        constexpr size_t Size() const{ return size; }

        Iterator begin()
        {
            return Iterator(m_Data);
        }
        Iterator end()
        {
            return Iterator(m_Data + size);
        }
        ConstIterator cbegin() const
        {
            return Iterator(const_cast<ValueType*>(m_Data));
        }
        ConstIterator cend() const
        {
            return Iterator(const_cast<ValueType*>(m_Data) + size);
        }

    };
}

