#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>

#include "inifile.h"
using namespace ktg;


Value::Value(){

}
Value::Value(bool value){
    *this = value;
}
Value::Value(int value){
    *this = value;
}
Value::Value(double value){
    *this = value;
}
Value::Value(const char * value){
    m_value = value;
}
Value::Value(const string & value){
    m_value = value;
}
Value::~Value(){
    
}

Value & Value::operator = (bool value){
    m_value = value ? "true" : "false";
    return *this;
}
Value & Value::operator = (int value){
    stringstream ss;
    ss << value;
    m_value == ss.str();
    return *this;
}
Value & Value::operator = (double value){
    stringstream ss;
    ss << value;
    m_value = ss.str();
    return *this;
}
Value & Value::operator = (const char * value){
    m_value = value;
    return *this;
}
Value & Value::operator = (const string & value){
    m_value = value;
    return *this;
}
Value & Value::operator = (const Value & value){
    m_value = value.m_value;
    return *this;
}


bool Value::operator == (const Value & other){
    return m_value == other.m_value;
}
bool Value::operator != (const Value & other){
    return !(m_value == other.m_value);
}

Value::operator bool(){
    if(m_value == "true"){
        return true;
    }
    return false;
}
Value::operator int(){
    return atoi(m_value.c_str());
}
Value::operator double(){
    return atof(m_value.c_str());
}
Value::operator string(){
    return m_value;
}

IniFile::IniFile(){}
IniFile::~IniFile(){}
bool IniFile::load(const string & filename){
    m_filename = filename;
    ifstream fin(filename);
    if(fin.fail()){
        return false;
    }
    
    string line;
    string section;
    while(getline(fin, line)){
        line = trim(line);
        if(line == ""){
            continue;
        }
       
        if(line[0] == '['){
            int pos = line.find_first_of(']');
            section = line.substr(1, pos - 1);
            section = trim(section);
            m_sections[section] = Sections();
        }else{
            int pos = line.find_first_of('=');
            string key = line.substr(0, pos);
            string value = line.substr(pos + 1, line.size() - pos);
            key = trim(key);
            value = trim(value);
            m_sections[section][key] = value;
        }
    }
    fin.close();
    return true;
}

string IniFile::trim(string s){
    if(s.empty())return s;
    s.erase(0, s.find_first_not_of(" \n\r"));
    s.erase(s.find_last_not_of(" \n\r") + 1);
    return s;
}

Value & IniFile::get(const string & section, const string & key){
    return m_sections[section][key];
}

void IniFile::set(const string & section, const string & key, const Value & value){
    m_sections[section][key] = value;
}

bool IniFile::has(const string & section, const string & key){
    if(m_sections.find(section) == m_sections.end()){
        cout<<"not exist sections"<<endl;
        return false;
    }
    if(m_sections[section].find(key) == m_sections[section].end()){
        cout<<"not exist key"<<endl;
        return false;
    }
    return true;
}

void IniFile::remove(const string & section){
    if(m_sections.find(section) == m_sections.end()){
        return;
    }
    m_sections.erase(section);
}
void IniFile::remove(const string & section, const string & key){
    if(!has(section, key)){
        cout<<"not exist section or key"<<endl;
        return;
    }
    m_sections[section].erase(key);
}

void IniFile::clear(){
    m_sections.clear();
}

void IniFile::display(){
    cout<<str()<<endl;
}

string IniFile::str(){
    stringstream ss;
    for(map<string,Sections>::iterator it = m_sections.begin(); it != m_sections.end(); it++){
        ss<<'['<<it -> first<<']'<<endl;
        for(map<string,Value>::iterator iter = it -> second.begin(); iter != it -> second.end(); iter++){
            ss<<iter -> first<<" = "<<string(iter -> second)<<endl;
        }
    }
    return ss.str();
}

bool IniFile::save(const string & filename){
    ofstream fout(filename);
    if(fout.fail()){
        cout<<"open file is failed"<<endl;
        return false;
    }
    fout<<str();
    fout.close();
    return true;
}