#pragma once

#include <stdexcept>
#include <memory>
#include <cstdio>

namespace test
{

class BadCast : public std::exception
{
public:
    const char* what() const noexcept override
    {
        return "BadCast Exception.";
    }
};

class Any
{   
private:
    class TypeInternal
    {
    public:
        TypeInternal() = default;
        virtual ~TypeInternal() = default;
        virtual const std::type_info& GetType() const = 0;
        virtual std::unique_ptr<TypeInternal> Clone() const = 0;
    };

    template<typename T>
    class TypeImpl : public TypeInternal
    {
    public:
        T m_Data;
        explicit TypeImpl(const T& value):m_Data(value){}

        explicit TypeImpl(T&& value):m_Data(std::move(value)){}

        template<typename... Args>
        explicit TypeImpl(Args&&... args):m_Data(std::forward<Args>(args)...){}

        const std::type_info& GetType() const override { return typeid(T); }

        std::unique_ptr<TypeInternal> Clone() const override
        {
            return std::unique_ptr<TypeInternal>(new TypeImpl(m_Data));
        }
    };
private:
    std::unique_ptr<TypeInternal> m_Data;
public:
    Any() = default;

    template<typename T, typename 
        std::enable_if<
            !std::is_same<typename std::decay<T>::type, Any
        >::value, bool>::type = true>
    Any(T&& value):m_Data(new TypeImpl<typename std::decay<T>::type>(std::forward<T>(value))){}

    template<typename T, typename 
        std::enable_if<
            !std::is_same<typename std::decay<T>::type, Any
        >::value, bool>::type = true>
    Any& operator=(T&& value)
    {
        m_Data.reset(new TypeImpl<typename std::decay<T>::type>(std::forward<T>(value)));
        return *this;
    }

    Any(Any& other):m_Data(other.m_Data -> Clone()){}

    Any(Any&& other):m_Data(other.m_Data.release()){}

    Any& operator=(Any& other)
    {
        m_Data = (other.m_Data) -> Clone();
        return *this;
    }

    Any& operator=(Any&& other)
    {
        m_Data.reset(other.m_Data.release());
        return *this;
    }

    bool HasValue() const{ return m_Data.operator bool(); }

    const std::type_info& Type() const { return HasValue() ? m_Data -> GetType() : typeid(void); }

    void Reset(){ m_Data.reset(); }

    void Swap(Any& other){ m_Data.swap(other.m_Data); }

    template<typename T, typename... Args>
    void Emplace(Args&&...args)
    {
        m_Data.reset(new TypeImpl<T>(std::forward<Args>(args)...));
    }

    template<typename T>
    T& Cast()
    {
        if(CheckType<T>() == true && IsNull() == false){
            return (static_cast<TypeImpl<T>*>(m_Data.get())) -> m_Data;
        }
        throw BadCast();
    }

    template<typename T>
    const T& Cast() const
    {
        if(CheckType<T>() == true && IsNull() == false){
            return (static_cast<const TypeImpl<T>*>(m_Data.get())) -> m_Data;
        }
        throw BadCast();
    }
private:
    template<typename T>
    bool CheckType() const
    {
        if((m_Data -> GetType()).hash_code() != typeid(T).hash_code()){
            fprintf(stderr,"in file %s:%d\n%s => %s type cast error!\n", __FILE__, __LINE__, (m_Data -> GetType()).name(), typeid(T).name());
            return false;
        }
        return true;
    }

    bool IsNull() const
    {
        if(!m_Data){
            fprintf(stderr, "in file %s:%d\n nullptr data",__FILE__, __LINE__);
			return true;
        }
        return false;
    }
};

inline void Swap(Any& lhs, Any& rhs)
{ lhs.Swap(rhs); }

template<typename T>
T& AnyCast(Any& obj)
{
    return obj.Cast<T>();
}

template<typename T>
const T& AnyCast(const Any& obj)
{
    return obj.Cast<T>();
}
}