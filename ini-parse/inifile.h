#pragma once


#include <iostream>
#include <algorithm>
#include <map>
#include <vector>


using namespace std;


namespace ktg {
    class Value {
        public:
            Value();
            Value(bool value);
            Value(int value);
            Value(double value);
            Value(const char * value);
            Value(const string & value);
            ~Value();

            Value & operator = (bool value);
            Value & operator = (int value);
            Value & operator = (double value);
            Value & operator = (const char * value);
            Value & operator = (const string & value);
            Value & operator = (const Value & value);

            bool operator == (const Value & other);
            bool operator != (const Value & other);

            operator bool();
            operator int();
            operator double();
            operator string();

        private:
            string m_value;
    };

    typedef map<string, Value>Sections;
    class IniFile{
        public:
            IniFile();
            ~IniFile();
            bool load(const string & filename);
            bool save(const string & filename);
            string str();
            Value & get(const string & section, const string & key);
            bool has(const string & section, const string & key);
            void remove(const string & section);
            void remove(const string & section, const string & key);
            void clear();

            void set(const string & section, const string & key, const Value & value);
            Sections & operator = (const string & key){
                return m_sections[key];
            }

            void display();
        private:
            string trim(string s);
        private:
            string m_filename;
            map<string, Sections> m_sections;
    };
}