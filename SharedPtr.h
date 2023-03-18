#pragma once
#include<memory>
#include<iostream>

namespace test{
    class ControlBlockBase{
        template<typename T>
        friend class WeakPtr;
        template<typename T>
        friend class SharedPtr;
    private: 
        long m_sharedCount;
        long m_weakCount;
    protected:
        void AcquireShared()
        {
            ++m_sharedCount;
        }

        void AcquireWeak()
        {
            ++m_weakCount;
        }

        void ReleaseShared()
        {
            if(--m_sharedCount == 0){
                Delete();
#ifdef DEBUG
    #if DEBUG == 1
                std::cout << "Delete the object" << std::endl;
    #endif
#endif
                if(m_weakCount == 0){
                    delete this;
#ifdef DEBUG
    #if DEBUG == 1
                    std::cout << "Delete the control block" << std::endl;
    #endif
#endif
                }
            }
        }
        void ReleaseWeak()
        {
            if(--m_weakCount == 0 && m_sharedCount == 0){
                delete this;
            }
        }

        long SharedCount()
        {
            return m_sharedCount;
        }

        long WeakCount()
        {
            return m_weakCount;
        }

    public:
        virtual void Delete() = 0;
        ControlBlockBase():m_sharedCount(0), m_weakCount(0)
        {
#ifdef DEBUG
    #if DEBUG == 1
                std::cout << "Create the contol block!" << std::endl;
    #endif
#endif
        }
        virtual ~ControlBlockBase(){}
    };

    template<typename T>
    class DefaultControlBlock : public ControlBlockBase{
    protected:
        T* m_Resource;
        void Delete() override {delete m_Resource;}
    public:
        DefaultControlBlock(T* ptr):m_Resource(ptr), ControlBlockBase(){}
    };

    template<typename T, typename D>
    class RegularControlBlock:public DefaultControlBlock<T>
    {
    private: 
        using DefaultControlBlock<T>::m_Resource;
        D m_Deleter;
    public:
        RegularControlBlock(T* ptr, const D& deleter)
        :DefaultControlBlock<T>(ptr), m_Deleter(deleter){}

        RegularControlBlock(T* ptr, D&& deleter)
        :DefaultControlBlock<T>(ptr), m_Deleter(std::move(deleter)){}

        void Delete() override
        {
            if(m_Resource != nullptr){
                m_Deleter(m_Resource);
            }
        }

    };

    template<typename T> 
    class SharedPtr{
        template<typename U>
        friend class test::SharedPtr;
        template<typename U>
        friend class WeakPtr;
    private:
        T* m_Resource;
        ControlBlockBase *m_Block;
    public:
        SharedPtr():m_Block(nullptr), m_Resource(nullptr){}

        SharedPtr(std::nullptr_t):SharedPtr(){}

        explicit SharedPtr(T* ptr)
        :m_Block(new DefaultControlBlock<T>(ptr)), m_Resource(ptr){InitHelper();}

        template<typename D>
        SharedPtr(T* ptr, D deleter)
        :m_Block(new RegularControlBlock<T, D>(ptr, std::move(deleter))), m_Resource(ptr){InitHelper();}

        SharedPtr(const SharedPtr &other)
        :m_Resource(other.m_Resource), m_Block(other.m_Block){InitHelper();}

        template<typename R>
        SharedPtr(const SharedPtr<R> &other)
        :m_Resource(other.m_Resource), m_Block(other.m_Block){InitHelper();}

        SharedPtr(SharedPtr&& other)
        :m_Resource(other.m_Resource), m_Block(other.m_Block)
        {
            other.m_Resource = nullptr;
            other.m_Block = nullptr;
        }

        template<typename R>
        SharedPtr(const SharedPtr<R>&& other)
        :m_Resource(other.m_Resource), m_Block(other.m_Block)
        {
            other.m_Resource = nullptr;
            other.m_Block = nullptr;
        }

        template<typename R>
        SharedPtr& Assign(const SharedPtr<R> &other)
        {
            if(m_Resource == other.m_Resource){
                return *this;
            }
            if(m_Block != nullptr){
                m_Block->ReleaseShared();
            }
            m_Block = other.m_Block;
            m_Resource = other.m_Resource;
            if(other.m_Block != nullptr){
                m_Block -> AcquireShared();
            }
            return *this;
        }

        template<typename R>
        SharedPtr& Assign(SharedPtr<R>&& other)
        {
            if(m_Resource == other.m_Resource){
                return *this;
            }
            if(m_Block != nullptr){
                m_Block->ReleaseShared();
            }
            m_Block = other.m_Block;
            m_Resource = other.m_Resource;
            other.m_Block = nullptr;
            other.m_Resource = nullptr;
            return *this;
        }

        SharedPtr& operator=(const SharedPtr& other)
        {
            return Assign(other);
        }

        template<typename R>
        SharedPtr& operator=(const SharedPtr<R>& other)
        {
            return Assign(other);
        }

        SharedPtr& operator=(SharedPtr&& other)
        {
            return Assign(std::move(other));
        }

        template<typename R>
        SharedPtr& operator=(SharedPtr<R>&& other)
        {
            return Assign(std::move(other));
        }

        ~SharedPtr()
        {
            if(m_Block){
                m_Block -> ReleaseShared();
            }
        }

        T& operator*() const { return *m_Resource; }

        T* get() const { return m_Resource; }

        T* operator->() const { return get(); }

        operator bool() const { return !!m_Resource; }

        std::size_t use_count() const
        {
            if (m_Block == nullptr)
                return 0;
            return m_Block -> SharedCount();
        }

        template<typename D = std::default_delete<T>>
        void reset(T* ptr = nullptr, D deleter = D())
        {
            if(m_Block != nullptr){
                m_Block -> ReleaseShared();
            }
            try{
                if(ptr == nullptr){
                    m_Block = nullptr;
                    m_Resource = nullptr;
                }else{
                    m_Resource = ptr;
                    m_Block = new RegularControlBlock<T, D>(ptr, std::move(deleter));
                    m_Block -> AcquireShared();
                }
            }catch(...){
                deleter(ptr);
                throw;
            }
        }

    private:
        void InitHelper()
        {
            if(m_Block != nullptr){
                m_Block -> AcquireShared();
            }
        }

        SharedPtr(T* ptr, ControlBlockBase* block)
        :m_Resource(ptr), m_Block(block){ InitHelper(); }

    };

    template<typename T>
    class WeakPtr{
        template<typename U>
        friend class WeakPtr;
    private:
        ControlBlockBase *m_Block;
        T* m_Resource;
    public:
        WeakPtr():m_Block(nullptr), m_Resource(nullptr){}

        WeakPtr(std::nullptr_t):WeakPtr(){}

        template<typename R>
        WeakPtr(const SharedPtr<R>& s_Ptr)
        :m_Resource(s_Ptr.m_Resource), m_Block(s_Ptr.m_Block)
        {
            if(m_Block){
                m_Block -> AcquireWeak();
            }
        }

        WeakPtr(const WeakPtr& other)
        :m_Resource(other.m_Resource), m_Block(other.m_Block)
        {
            if(m_Block != nullptr){
                m_Block -> AcquireWeak();
            }
        }

        template<typename R>
        WeakPtr(const WeakPtr<R>& other)
        :m_Resource(other.m_Resource), m_Block(other.m_Block)
        {
            if(m_Block != nullptr){
                m_Block -> AcquireWeak();
            }
        }

        WeakPtr(WeakPtr&& other)
        :m_Resource(other.m_Resource), m_Block(other.m_Block)
        {
            other.m_Resource = nullptr;
            other.m_Block = nullptr;
        }

        template<typename R>
        WeakPtr(WeakPtr<R>&& other)
        :m_Resource(other.m_Resource), m_Block(other.m_Block)
        {
            other.m_Resource = nullptr;
            other.m_Block = nullptr;
        }

        WeakPtr& Assign(const WeakPtr& other)
        {
            if(m_Resource == other.m_Resource){
                return *this;
            }
            if(m_Block != nullptr){
                m_Block -> ReleaseWeak();
            }
            m_Block = other.m_Block;
            m_Resource = other.m_Resource;
            if(m_Block){
                m_Block -> AcquireWeak();
            }
            return *this;
        }

        template<typename U>
        WeakPtr& Assign(WeakPtr<U>&& other)
        {
            if(m_Resource == other.m_Resource){
                return *this;
            }
            if(m_Block != nullptr){
                m_Block -> ReleaseWeak();
            }
            m_Block = other.m_Block;
            m_Resource = m_Resource;
            other.m_Block = nullptr;
            other.m_Resource = nullptr;
            return *this;
        }

        WeakPtr& operator=(const WeakPtr& other){ return Assign(other); }

        template<typename R>
        WeakPtr& operator=(const WeakPtr<R>& other){ return Assign(other); }


        WeakPtr& operator=(WeakPtr&& other){ return Assign(std::move(other)); }

        template<typename R>
        WeakPtr& operator=(WeakPtr<R>&& other){ return Assign(std::move(other)); }

        template<typename R>
        WeakPtr& operator=(const SharedPtr<R>& other)
        {
            WeakPtr<T> tmp(other);
            *this = tmp;
            return *this;
        }

        ~WeakPtr()
        {
            if(m_Block){
                m_Block -> ReleaseWeak();
            }
        }

        bool IsExpired() const
        {
            if(m_Block == nullptr){
                return true;
            }
            return m_Block -> SharedCount() == 0;
        }

        SharedPtr<T> Lock() const
        {
            if(!IsExpired()){
                return SharedPtr<T>(m_Resource, m_Block);
            }
            return SharedPtr<T>();
        }
    };    
}