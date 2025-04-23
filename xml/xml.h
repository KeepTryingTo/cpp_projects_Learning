#pragma once

#include <iostream>
#include <algorithm>
#include <map>
#include <list>

#include "value.h"
using namespace ktg;

using namespace std;

namespace ktg {
    class XML{
        public:
            XML();
            XML(const char * name);
            XML(const string & name);
            XML(const XML & other);

            string name()const;
            void setName(const string & name);

            string text()const;
            void setText(const string & text);

            Value & attr(const string & key);
            void setAttr(const string & key, const Value & val);

            string str()const;

            void append(const XML & child);
            XML & operator [] (int index);
            XML & operator [] (const char * name);
            XML & operator [] (const string & name);
            XML & operator = (const XML & other);

            void remove(int index);
            void remove(const char * name);
            void remove(const string & name);

            void clear();

            typedef list<XML>::iterator iterator;
            iterator begin(){
                return m_children -> begin();
            }
            iterator end(){
                return m_children -> end();
            }
            iterator erase(iterator it){
                it -> clear();
                return m_children -> erase(it);
            }
        private:
            string * m_name;
            string * m_text;
            std::map<string,Value> * m_attrs;
            std::list<XML> * m_children;
    };
}