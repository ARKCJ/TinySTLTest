#pragma once 
#include<stddef.h>
#include"OutOfRange.h"

namespace test{

    template<typename VectorType>
    class VectorIterator{
    public:
        using ValueType = typename VectorType::ValueType;
        using RefType = typename VectorType::ValueType&;
        using PointerType = typename VectorType::ValueType*;
        using ConstRefType = const RefType;
        using ConstPointerType = const PointerType;
    private:
        mutable PointerType m_ptr;
    public:
        VectorIterator(PointerType ptr):m_ptr(ptr){}
        
        VectorIterator& operator++()
        {
            return const_cast<VectorIterator&>(++(*const_cast<const VectorIterator*>(this)));
        }

        VectorIterator operator++(int)
        {
            return const_cast<VectorIterator>((*const_cast<const VectorIterator*>(this))++);
        }

        const VectorIterator& operator++() const
        {
            ++m_ptr;
            return *this;
        }

        const VectorIterator operator++(int) const
        {
            VectorIterator tmp = *this;
            operator++();
            return tmp;
        }

        VectorIterator& operator--()
        {
            return const_cast<VectorIterator&>(--(*const_cast<const VectorIterator*>(this)));;
        }

        VectorIterator operator--(int)
        {
            return const_cast<VectorIterator>((*const_cast<const VectorIterator*>(this))--);
        }

        VectorIterator& operator--() const
        {
            --m_ptr;
            return *this;
        }

        VectorIterator operator--(int) const
        {
            VectorIterator tmp = *this;
            operator--();
            return tmp;
        }

        PointerType operator->()
        {
            return const_cast<PointerType>((*const_cast<const VectorIterator*>(this)).operator->());
        }

        ConstPointerType operator->() const
        {
            return m_ptr;
        }

        RefType operator*()
        {
            return const_cast<RefType>((*const_cast<const VectorIterator*>(this)).operator*());
        }

        ConstRefType operator*() const
        {
            return *m_ptr;
        }

        bool operator==(const VectorIterator& other) const
        {
            return m_ptr == other.m_ptr;
        }

        bool operator!=(const VectorIterator& other) const
        {
            return !(*this == other);
        }

        VectorIterator operator+(size_t size)
        {
            return VectorIterator(m_ptr + size);
        }

        VectorIterator operator-(size_t size)
        {
            return VectorIterator(m_ptr - size);
        }

        const VectorIterator operator+(size_t size) const
        {
            return VectorIterator(m_ptr + size);
        }

        const VectorIterator operator-(size_t size) const
        {
            return VectorIterator(m_ptr - size);
        }

    };


    template<typename T>
    class Vector{
    public:
        using ValueType = T;
        using RefType = ValueType&;
        using ConstRefType = const ValueType&;
        using Iterator = VectorIterator<Vector<ValueType>>;
        typedef const Iterator ConstIterator;
    private:
        ValueType* m_Data;
        size_t m_Capacity;
        size_t m_Size;
    public:
        Vector()
        :m_Size(0), m_Capacity(0), m_Data(nullptr)
        {
            ReAlloc(2);
        }
        
        Vector(const ValueType& value, size_t num)
        :m_Size(0), m_Capacity(0), m_Data(nullptr)
        {
            ReAlloc(num * 2);
            for(;m_Size < num ; m_Size++){
                new (m_Data + m_Size) ValueType(value);
            }
        }
        
        template<typename R>
        Vector(const std::initializer_list<R>& list)
        :m_Size(0), m_Capacity(0), m_Data(nullptr)
        {
            ReAlloc(list.size());
            for(auto iter = list.begin(); iter != list.end(); ++iter){
                new (m_Data + m_Size++) ValueType(*iter);
            }
        }

        template<typename R>
        Vector(std::initializer_list<R>&& list)
        :m_Size(0), m_Capacity(0), m_Data(nullptr)
        {
            ReAlloc(list.size());
            for(auto iter = list.begin(); iter != list.end(); ++iter){
                new (m_Data + m_Size++) ValueType(std::move(*iter));
            }
        }

        Vector(const Vector& other)
        :m_Size(0), m_Capacity(0), m_Data(nullptr)
        {
            ReAlloc(other.m_Capacity);
            for(;m_Size < other.m_Size; ++m_Size){
                new (m_Data + m_Size) ValueType(other[m_Size]);
            }
        }

        Vector(Vector&& other) noexcept
        :m_Size(other.m_Size), m_Capacity(other.m_Capacity), m_Data(other.m_Data)
        {
            other.m_Size = 0;
            other.m_Capacity = 0;
            other.m_Data = nullptr;
        }

        Vector& operator=(const Vector& other)
        {
            if(this == &other){
                return *this;
            }
            if(m_Capacity < other.m_Size){
                ReAlloc(other.m_Capacity);
            }else{
                Clear();
            }
            for(;m_Size < other.m_Size;++m_Size){
                new (m_Data + m_Size) ValueType(other[m_Size]);
            }
            return *this;
        } 

        Vector& operator=(Vector&& other) noexcept
        {
            if(this == &other){
                return *this;
            }
            DeConstruct();
            m_Data = other.m_Data;
            m_Capacity = other.m_Capacity;
            m_Size = other.m_Size;
            other.m_Data = nullptr;
            other.m_Capacity = 0;
            other.m_Size = 0;
        }


        void PushBack(const ValueType& value)
        {
            CheckCapacity();
            new (m_Data + m_Size++) ValueType(value);
        }

        void PushBack(ValueType&& value)
        {
            CheckCapacity();
            new (m_Data + m_Size++) ValueType(std::move(value));
        }

        template<typename... Args>
        void EmplaceBack(Args&&... param)
        {
            CheckCapacity();
            new (m_Data + m_Size++) ValueType(std::forward<Args>(param)...);
        }

        void PopBack()
        {
            if(m_Size == 0){
                throw OutOfRange();
            }
            (*(m_Data + --m_Size)).~ValueType();
        }

        RefType operator[](size_t index)
        {
            return const_cast<RefType>((*const_cast<const Vector<T>*>(this))[index]);
        }

        ConstRefType operator[](size_t index) const
        {
            if(index < 0 || index >= m_Size){
                throw OutOfRange(m_Size, index);
            }
            return *(m_Data + index);
        }

        size_t Size(){ return m_Size; }

        size_t Capacity(){ return m_Capacity; }

        void Clear()
        {
            for(;m_Size > 0; --m_Size){
                (*(m_Data + m_Size - 1)).~ValueType();
            }
        }

        Iterator begin()
        {
            return Iterator(m_Data);
        }

        Iterator end()
        {
            return Iterator(m_Data + m_Size);
        }

        ConstIterator cbegin() const
        {
            return Iterator(const_cast<ValueType*>(m_Data));
        }

        ConstIterator cend() const
        {
            return Iterator(const_cast<ValueType*>(m_Data) + m_Size);
        }

        Iterator Erase(Iterator begin, Iterator end)
        {
            Iterator tmp = begin;
            for(;end != this -> end();++end, ++begin){
               *begin = std::move(*end);
            }
            while(begin != end){
                (*begin).~ValueType();
                --m_Size;
                ++begin;
            }
            return tmp;
        }

        ~Vector()
        {
            DeConstruct();
        }
    private:
        void ReAlloc(size_t newCapacity)
        {
            ValueType* newMemory = static_cast<ValueType*>(::operator new(newCapacity * sizeof(ValueType)));
            size_t newSize = newCapacity < m_Size ? newCapacity : m_Size;
            for(int i = 0; i < newSize; ++i){
                new (newMemory + i) ValueType(std::move(*(m_Data + i)));
            }
            Clear();
            ::operator delete(m_Data, m_Capacity * sizeof(ValueType));
            m_Data = newMemory;
            m_Size = newSize;
            m_Capacity = newCapacity;
        }

        void CheckCapacity()
        {
            if(m_Size == m_Capacity){
                ReAlloc(m_Capacity + m_Capacity / 2 + 1);
            }
        }

        void DeConstruct()
        {
            Clear();
            ::operator delete(m_Data, m_Capacity * sizeof(ValueType));
            m_Capacity = 0;
        }

    };
}