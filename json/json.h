#pragma once

#include <iostream>
#include <algorithm>
#include <vector>
#include <array>
#include <map>

using namespace std;

namespace ktg {
    class Json{
        public:
            enum Type {
                json_null = 0, 
                json_bool,
                json_int,
                json_double,
                json_string,
                json_array,
                json_object
            };

            Json();
            Json(bool value);
            Json(int value);
            Json(double value);
            Json(const char * value);
            Json(const string & value);
            Json(Type type);
            Json(const Json & other);

            operator bool();
            operator int();
            operator double();
            operator string();

            // 添加赋值操作符重载
            Json& operator=(bool value);
            Json& operator=(int value);
            Json& operator=(double value);
            Json& operator=(const char* value);
            Json& operator=(const string& value);
            Json& operator=(const Json& other);
            // void operator = (const Json & other);
            bool operator == (const Json & other);
            bool operator != (const Json & other);

            Json & operator[] (int index);
            void append(const Json & other);

            Json & operator [] (const char * key);
            Json & operator [] (const string & key);

            int size() const{
                return m_value.m_array -> size();
            }

            bool isNull()const {return m_type == json_null;}
            bool isBool()const {return m_type == json_bool;}
            bool isInt()const {return m_type == json_int;}
            bool isDouble()const {return m_type == json_double;}
            bool isString()const {return m_type == json_string;}
            bool isArray()const {return m_type == json_array;}
            bool isObject()const {return m_type == json_object;}

            //显性的类型转换
            bool asBool()const;
            int asInt()const;
            double asDouble()const;
            string asString()const;

            //判断元素是否存在
            bool has(int index);
            bool has(const char * key);
            bool has(const string & key);

            void remove(int index);
            void remove(const char * key);
            void remove(const string & key);

            string str()const;

            typedef vector<Json>::iterator iterator;
            iterator begin(){
                return m_value.m_array -> begin();
            }
            iterator end(){
                return m_value.m_array -> end();
            }


        private:
            void clear();
        
        private:
            union Value{
                bool m_bool;
                int m_int;
                double m_double;
                std::string * m_string;
                std::vector<Json> * m_array;
                std::map<string, Json> * m_object;
            };
        
            Type m_type;
            Value m_value;

    };
}