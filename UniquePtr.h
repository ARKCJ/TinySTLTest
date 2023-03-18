#pragma once
#include<memory>

namespace test{
    template<typename T, typename D = std::default_delete<T>>
    class UniquePtr{
    private:
        T* m_ptr;
        D m_Deleter;
    public:

        UniquePtr()
        :m_ptr(nullptr){}

        explicit UniquePtr(T* ptr)
        :m_ptr(ptr){}

        UniquePtr(const UniquePtr&) = delete;

        UniquePtr(UniquePtr&& other)
        :m_ptr(other.Release()), m_Deleter(other.GetDeleter()){}

        UniquePtr(T *ptr, const D &deleter)
        :m_ptr(ptr), m_Deleter(deleter){}

        UniquePtr(T *ptr, D&& deleter)
        :m_ptr(ptr), m_Deleter(std::move(deleter)){}

        template<typename R, typename O>
        UniquePtr(UniquePtr<R, O>&& other)
        :m_ptr(other.Release()), m_Deleter(other.GetDeleter()){}

        UniquePtr& operator=(const UniquePtr& other) = delete;

        UniquePtr& operator=(UniquePtr&& other)
        {
            Reset(other.Release());
            m_Deleter = other.GetDeleter();
            return *this;
        }

        template<typename R, typename O>
        UniquePtr& operator=(UniquePtr<R, O>&& other)
        {
            Reset(other.Release());
            m_Deleter = other.GetDeleter();
            return *this;
        }

        T* operator->(){ return m_ptr; }

        const T* operator->() const { return m_ptr; }

        T& operator*(){ return *m_ptr; }

        const T& operator*() const { return *m_ptr; }
        
        T* Get() const { return m_ptr; }

        const D& GetDeleter() const { return m_Deleter; }

        bool operator==(const UniquePtr& other) const { return m_ptr == other.m_ptr; }

        bool operator!=(const UniquePtr& other) const { return !(*this == other); }

        T* Release() 
        {
            T* tmp = m_ptr;
            m_ptr = nullptr;
            return tmp;
        }

        void Reset(T *ptr = nullptr) 
        {
            if(ptr == m_ptr){
                return;
            }
            GetDeleter()(m_ptr);
            m_ptr = ptr;
        }

        ~UniquePtr(){ GetDeleter()(m_ptr); }

    };

}

