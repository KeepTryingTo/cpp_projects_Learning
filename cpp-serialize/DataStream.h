
#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <string.h>
#include <set>


using namespace std;

namespace ktg {
    namespace serialize {
        class DataStream {
            public:
                enum DataType {
                    BOOL = 0,
                    CHAR,
                    INT16,
                    INT32,
                    INT64,
                    FLOAT,
                    DOUBLE,
                    STRING,
                    VECTOR,
                    LIST,
                    SET,
                    MAP,
                    CUSTOM
                };

                DataStream() : m_pos(0){}
                ~DataStream(){}
                void write(const char * data, int len);
                void write(bool value);
                void write(char value);
                void write(int16_t value);
                void write(int32_t value);
                void write(int64_t value);
                void write(const char * value);
                void write(const string & value);
                void write(float value);
                void write(double value);

                template<typename T>
                void write(const vector<T>&value);
                template<typename T>
                void write(const set<T>&value);
                template<typename T1, typename T2>
                void write(const map<T1,T2>&value);
                template<typename T>
                void write(const list<T>&value);

                bool read(bool & value);
                bool read(char & value);
                bool read(int16_t & value);
                bool read(int32_t & value);
                bool read(int64_t & value);
                bool read(float & value);
                bool read(double & value);
                bool read(string & value);

                template<typename T>
                bool read(vector<T>&value);
                template<typename T>
                bool read(list<T>&value);
                template<typename T>
                bool read(set<T>&value);
                template<typename T1, typename T2>
                bool read(map<T1,T2>&value);

                void display()const;

                DataStream & operator << (bool value);
                DataStream & operator << (char value);
                DataStream & operator << (int16_t value);
                DataStream & operator << (int32_t value);
                DataStream & operator << (int64_t value);
                DataStream & operator << (const char * value);
                DataStream & operator << (const string & value);
                DataStream & operator << (float value);
                DataStream & operator << (double value);
                template<typename T>
                DataStream & operator << (const set<T> &value);
                template<typename T1, typename T2>
                DataStream & operator << (const map<T1,T2> & value);
                template<typename T>
                DataStream & operator << (const vector<T> & value);
                template<typename T>
                DataStream & operator << (const list<T> & value);

                DataStream & operator >> (bool & value);
                DataStream & operator >> (char & value);
                DataStream & operator >> (int16_t & value);
                DataStream & operator >> (int32_t & value);
                DataStream & operator >> (int64_t & value);
                DataStream & operator >> (string & value);
                DataStream & operator >> (float & value);
                DataStream & operator >> (double & value);
                template<typename T1, typename T2>
                DataStream & operator >> (map<T1,T2> & value);
                template<typename T>
                DataStream & operator >> (vector<T> & value);
                template<typename T>
                DataStream & operator >> (set<T> & value);
                template<typename T>
                DataStream & operator >> (list<T> & value);
            
            private:
                void reserve(int len);
            private:
                vector<char>m_buf;
                int m_pos;
        };

        void DataStream::reserve(int len){
            int size = m_buf.size();
            int cap = m_buf.capacity();
            if(size + len > cap){
                while(size + len > cap){
                    if(cap == 0){
                        cap = 1;
                    }else{
                        cap *= 2;
                    }
                }
                m_buf.reserve(cap);
            }
        }
        void DataStream::write(const char * data, int len){
            reserve(len);
            int size = m_buf.size();
            m_buf.resize(size + len);

            memcpy(&m_buf[size], data, len);
        }

        void DataStream::write(bool value){
            char type = DataStream::BOOL;
            write((char *)&type, sizeof(char));
            write((char *)&value, sizeof(char));
        }

        bool DataStream::read(bool & value){
            if(m_buf[m_pos] != DataType::BOOL){
                return false;
            }
            ++m_pos;
            value = m_buf[m_pos];
            ++m_pos;
            return true;
        }

        void DataStream::write(char value){
            char type = DataStream::CHAR;
            write((char *)&type, sizeof(char));
            write((char *)&value, sizeof(char));
        }
        bool DataStream::read(char & value){
            if(m_buf[m_pos] != DataType::CHAR){
                return false;
            }
            ++m_pos;
            value = m_buf[m_pos];
            ++m_pos;
            return true;
        }
                
        void DataStream::write(int16_t value){
            char type = DataStream::INT16;
            write((char *)&type, sizeof(char));
            write((char *)&value, sizeof(int16_t));
        }
        bool DataStream::read(int16_t & value){
            if(m_buf[m_pos] != DataType::INT16){
                return false;
            }
            ++m_pos;
            value = *((int16_t *)(&m_buf[m_pos]));
            m_pos += 2;
            return true;
        }
                
        void DataStream::write(int32_t value){
            char type = DataStream::INT32;
            write((char *)&type, sizeof(char));
            write((char *)&value, sizeof(int32_t));
        }
        bool DataStream::read(int32_t & value){
            if(m_buf[m_pos] != DataType::INT32){
                return false;
            }
            ++m_pos;
            value = *((int32_t *)(&m_buf[m_pos]));
            m_pos += 4;
            return true;
        }
                
        void DataStream::write(int64_t value){
            char type = DataStream::INT64;
            write((char *)&type, sizeof(char));
            write((char *)&value, sizeof(int64_t));
        }
        bool DataStream::read(int64_t & value){
            if(m_buf[m_pos] != DataType::INT64){
                return false;
            }
            ++m_pos;
            value = *((int64_t *)(&m_buf[m_pos]));
            m_pos += 8;
            return true;
        }
                
        void DataStream::write(const char * value){
            char type = DataStream::STRING;
            write((char *)&type, sizeof(char));
            int len = strlen(value);
            write(len);
            write(value, len);
        }
        
        void DataStream::write(const string & value){
            char type = DataStream::STRING;
            write((char *)&type, sizeof(char));
            int len = value.size();
            write(len);
            write(value.data(), len);
        }
        bool DataStream::read(string & value){
            if(m_buf[m_pos] != DataType::STRING){
                return false;
            }
            ++m_pos;
            int len;
            read(len);
            if(len < 0){
                return false;
            }
            value.assign((char *)&(m_buf[m_pos]), len);
            m_pos += len;
            return true;
        }
               
        void DataStream::write(float value){
            char type = DataStream::FLOAT;
            write((char *)&type, sizeof(char));
            write((char *)&value, sizeof(float));
        }
        bool DataStream::read(float & value){
            if(m_buf[m_pos] != DataType::FLOAT){
                return false;
            }
            ++m_pos;
            value = *((float *)(&m_buf[m_pos]));
            m_pos += 4;
            return true;
        }
        void DataStream::write(double value){
            char type = DataStream::DOUBLE;
            write((char *)&type, sizeof(char));
            write((char *)&value, sizeof(double));
        }
        bool DataStream::read(double & value){
            if(m_buf[m_pos] != DataType::DOUBLE){
                return false;
            }
            ++m_pos;
            value = *((double *)(&m_buf[m_pos]));
            m_pos += 8;
            return true;
        }

        template<typename T>
        void DataStream::write(const vector<T>&value){
            char type = DataType::VECTOR;
            write((char*)&type, sizeof(type));
            int len = value.size();
            write(len);
            for(int i = 0; i < value.size(); i++){
                write(value[i]);
            }
        }
        template<typename T>
        void DataStream::write(const set<T>&value){
            char type = DataType::SET;
            write((char*)&type, sizeof(type));
            int len = value.size();
            write(len);
            for(auto it = value.begin(); it != value.end(); it++){
                write(*it);
            }
        }
        template<typename T1, typename T2>
        void DataStream::write(const map<T1,T2>&value){
            char type = DataType::MAP;
            write((char*)&type, sizeof(type));
            int len = value.size();
            write(len);
            for(auto it : value){
                T1 key = it.first;
                T2 val = it.second;
                write(key);
                write(val);
            }
        }
        template<typename T>
        void DataStream::write(const list<T>&value){
            char type = DataType::VECTOR;
            write((char*)&type, sizeof(type));
            int len = value.size();
            write(len);
            for(auto it = value.begin(); it != value.end(); it++){
                write(*it);
            }
        }

        template<typename T>
        bool DataStream::read(vector<T>&value){
            value.clear();
            if(m_buf[m_pos] != DataType::VECTOR){
                return false;
            }
            ++m_pos;
            int len;
            read(len);
            for(int i = 0; i < len; i++){
                T v;
                read(v);
                value.push_back(v);
            }
            m_pos += len;
            return true;
        }
        template<typename T>
        bool DataStream::read(list<T>&value){
            value.clear();
            if(m_buf[m_pos] != DataType::LIST){
                return false;
            }
            ++m_pos;
            int len;
            read(len);
            for(int i = 0; i < len; i++){
                T v;
                read(v);
                value.push_back(v);
            }
            m_pos += len;
            return true;
        }
        template<typename T>
        bool DataStream::read(set<T>&value){
            value.clear();
            if(m_buf[m_pos] != DataType::SET){
                return false;
            }
            ++m_pos;
            int len;
            read(len);
            for(int i = 0; i < len; i++){
                T v;
                read(v);
                value.insert(v);
            }
            m_pos += len;
            return true;
        }

        template<typename T1, typename T2>
        bool DataStream::read(map<T1,T2>&value){
            value.clear();
            if(m_buf[m_pos] != DataType::MAP){
                return false;
            }
            ++m_pos;
            int len;
            read(len);
            for(int i = 0; i < len; i += 2){
                T1 key;
                read(key);
                T2 val;
                read(val);
                value[key] = val;
            }
            m_pos += len;
            return true;
        }

        void DataStream::display()const {
            int size = m_buf.size();
            cout<<"buf size = "<<size<<endl;

            for(int i = 0; i < size; ){
                switch((DataType)m_buf[i]){
                    case DataType::BOOL:
                        if(m_buf[++i]){
                            cout<<"true"<<endl;
                        }else{
                            cout<<"false"<<endl;
                        }
                        i++;
                        break;
                    case DataType::CHAR:
                        cout<<m_buf[++i]<<endl;
                        i++;
                        break;
                    case DataType::INT16:
                        cout<<*((int16_t *)(&m_buf[++i]))<<endl;
                        i += 2;
                        break;
                    case DataType::INT32:
                        cout<<*((int32_t *)(&m_buf[++i]))<<endl;
                        i += 4;
                        break;
                    case DataType::INT64:
                        cout<<*((int64_t *)(&m_buf[++i]))<<endl;
                        i += 8;
                        break;
                    case DataType::FLOAT:
                        cout<<*((float *)(&m_buf[++i]))<<endl;
                        i += 4;
                        break;
                    case DataType::DOUBLE:
                        cout<<*((double *)(&m_buf[++i]))<<endl;
                        i += 8;
                        break;
                    case DataType::STRING:
                        if((DataType)m_buf[++i] == DataType::INT32){
                            int len = *((int32_t *)(&m_buf[++i]));
                            i += 4;
                            cout<<string(&m_buf[i], len)<<endl;
                            i += len;
                        }else{
                            throw logic_error("pase string error");
                        }
                        break;
                    case DataType::SET:
                        break;
                    case DataType::LIST:
                        break;
                    case DataType::MAP:
                        break;
                    case DataType::CUSTOM:
                        break;
                    default:
                        break;
                }
            }
        }

        DataStream & DataStream::operator << (bool value){
            write(value);
            return *this;
        }
        DataStream & DataStream::operator << (char value){
            write(value);
            return *this;
        }
        DataStream & DataStream::operator << (int16_t value){
            write(value);
            return *this;
        }
        DataStream & DataStream::operator << (int32_t value){
            write(value);
            return *this;
        }
        DataStream & DataStream::operator << (int64_t value){
            write(value);
            return *this;
        }
        DataStream & DataStream::operator << (const char * value){
            write(value);
            return *this;
        }
        DataStream & DataStream::operator << (const string & value){
            write(value);
            return *this;
        }
        DataStream & DataStream::operator << (float value){
            write(value);
            return *this;
        }
        DataStream & DataStream::operator << (double value){
            write(value);
            return *this;
        }
        template<typename T>
        DataStream & DataStream::operator << (const set<T> &value){
            write(value);
            return *this;
        }
        template<typename T1, typename T2>
        DataStream & DataStream::operator << (const map<T1,T2> & value){
            write(value);
            return *this;
        }
        template<typename T>
        DataStream & DataStream::operator << (const vector<T> & value){
            write(value);
            return *this;
        }
        template<typename T>
        DataStream & DataStream::operator << (const list<T> & value){
            write(value);
            return *this;
        }

        DataStream & DataStream::operator >> (bool & value){
            read(value);
            return *this;
        }
        DataStream & DataStream::operator >> (char & value){
            read(value);
            return *this;
        }
        DataStream & DataStream::operator >> (int16_t & value){
            read(value);
            return *this;
        }
        DataStream & DataStream::operator >> (int32_t & value){
            read(value);
            return *this;
        }
        DataStream & DataStream::operator >> (int64_t & value){
            read(value);
            return *this;
        }
        DataStream & DataStream::operator >> (string & value){
            read(value);
            return *this;
        }
        DataStream & DataStream::operator >> (float & value){
            read(value);
            return *this;
        }
        DataStream & DataStream::operator >> (double & value){
            read(value);
            return *this;
        }
        template<typename T1, typename T2>
        DataStream & operator >> (map<T1,T2> & value){
            read(value);
            return *this;
        }
        template<typename T>
        DataStream & operator >> (vector<T> & value){
            read(value);
            return *this;
        }
        template<typename T>
        DataStream & operator >> (set<T> & value){
            read(value);
            return *this;
        }
        template<typename T>
        DataStream & operator >> (list<T> & value){
            read(value);
            return *this;
        }
    }
}