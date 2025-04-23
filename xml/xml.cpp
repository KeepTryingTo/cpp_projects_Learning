
#include <sstream>
using namespace std;

#include "xml.h"
using namespace ktg;


XML::XML() : m_name(nullptr), m_attrs(nullptr), m_text(nullptr), m_children(nullptr){

}
XML::XML(const char * name) : m_attrs(nullptr), m_text(nullptr), m_children(nullptr){
    m_name = new string(name);
}
XML::XML(const string & name) : m_attrs(nullptr), m_text(nullptr), m_children(nullptr){
    m_name = new string(name);
}
XML::XML(const XML & other){
    m_name = other.m_name;
    m_attrs = other.m_attrs;
    m_children = other.m_children;
    m_text = other.m_text;
}

string XML::name()const{
    if(m_name == nullptr)return "";
    return *m_name;
}
void XML::setName(const string & name){
    if(m_name != nullptr){
        delete m_name;
        m_name = nullptr;
    }
    m_name = new string(name);
}

string XML::text() const{
    if(m_text == nullptr)return "";
    return *m_text;
}
void XML::setText(const string & text){
    if(m_text != nullptr){
        delete m_text;
        m_text = nullptr;
    }
    m_text = new string(text);
}

Value & XML::attr(const string & key){
    if(m_attrs == nullptr){
        m_attrs = new map<string,Value>();
    }
    if(m_attrs -> find(key) == m_attrs -> end())return "";
    return (*m_attrs)[key];
}
void XML::setAttr(const string & key, const Value & val){
    if(m_attrs == nullptr){
        m_attrs = new map<string,Value>();
    }
    if(m_attrs -> find(key) == m_attrs -> end()){
        (*m_attrs)[key] = val;
    }
}

string XML::str()const{
    if(m_name == nullptr){
        throw logic_error("element name is not exist!");
    }
    stringstream ss;
    ss << "<";
    ss << *m_name;
    if(m_attrs != nullptr){
        for(auto it = m_attrs -> begin(); it != m_attrs -> end(); it++){
            ss << " "<< it -> first << "=\""<< string(it -> second)<<" \"";
        }
    }
    ss<<">";
    if(m_children != nullptr){
        for(auto it = m_children -> begin(); it != m_children -> end(); it++){
            ss << it -> str();
        }
    }
    if(m_text != nullptr){
        ss << *m_text;
    }
    ss << "</"<<*m_name <<">";
    return ss.str();
}

void XML::append(const XML & child){
    if(m_children == nullptr){
        m_children = new list<XML>();
    }
    m_children -> push_back(child);
}

XML & XML::operator [] (int index){
    if(index < 0){
        throw logic_error("index is less zero");
    }
    if(m_children == nullptr){
        m_children = new list<XML>();
    }
    int size = m_children -> size();
    if(index >= 0 && index < size){
        auto it = m_children -> begin();
        for(int i = 0; i < index; i++){
            it++;
        }
        return *it;
    }
    if(index >= size){
        for(int i = size; i <= index; i++){
            m_children -> push_back(XML());
        }
    }
    return m_children -> back();
}

XML & XML::operator [] (const char * name){
    return (*this)[string(name)];
}

XML & XML::operator [] (const string & name){
    if(m_children == nullptr){
        m_children = new list<XML>();
    }
    for(auto it = m_children -> begin(); it != m_children -> end(); it++){
        if(it -> name() == name){
            return *it;
        }
    }
    m_children -> push_back(XML(name));
    return m_children -> back();
}

XML & XML::operator = (const XML & other){
    clear();
    m_name = other.m_name;
    m_attrs = other.m_attrs;
    m_children = other.m_children;
    m_text = other.m_text;
    return *this;
}

void XML::clear(){
    if(m_name){
        delete m_name;
        m_name = nullptr;
    }
    if(m_attrs){
        delete m_attrs;
        m_attrs = nullptr;
    }
    if(m_text){
        delete m_text;
        m_text = nullptr;
    }
    if(m_children){
        for(auto it = m_children -> begin(); it != m_children -> end(); it++){
            it -> clear();
        }
        delete m_children;
        m_children = nullptr;
    }
}

void XML::remove(int index){
    if(m_children == nullptr){
        return;
    }
    if(index < 0 || index >= m_children -> size()){
        throw logic_error("index is out of range");
    }
    auto it = m_children -> begin();
    for(int i = 0; i < index; i++){
        it++;
    }
    it -> clear();
    m_children -> erase(it);
}
void XML::remove(const char * name){
    remove(string(name));
}
void XML::remove(const string & name){
    if(m_children == nullptr){
        return;
    }
    for(auto it = m_children -> begin(); it != m_children -> end();){
        if(it -> name() == name){
            it -> clear();
            it = m_children -> erase(it);
        }else{
            it++;
        }
    }
}