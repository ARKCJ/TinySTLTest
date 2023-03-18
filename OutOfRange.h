#pragma once
#include<iostream>

namespace test{
    class OutOfRange{
    public:
        OutOfRange(): m_flag(1){ };
        OutOfRange(int len, int index): m_len(len), m_index(index), m_flag(2){}
    public:
        void what() const
        {
            using namespace::std;
            if(m_flag == 1){
                cout<<"Error: empty array, no elements to pop."<<endl;
            }else if(m_flag == 2){
                cout<<"Error: out of range( array length "<<m_len<<", access index "<<m_index<<" )"<<endl;
            }else{
                cout<<"Unknown exception."<<endl;
            }
        }
    private:
        int m_flag;  //不同的flag表示不同的错误
        int m_len;  //当前数组的长度
        int m_index;  //当前使用的数组下标
    };
}