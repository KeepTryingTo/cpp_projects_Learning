#include <iostream>
#include <algorithm>

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
}