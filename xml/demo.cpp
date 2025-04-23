#include "xml.h"
using namespace ktg;

int main(){
    XML xml("voc");
    cout<<xml.name()<<endl;

    xml.setAttr("name","detection");

    XML child1("object1");
    child1.setAttr("image name","1");
    child1.setAttr("number", "12");
    child1.setText("bound box one");

    XML child2("object2");
    child2.setAttr("image name","2");
    child2.setAttr("number", "12");
    child2.setText("bound box two");

    xml.append(child1);
    xml.append(child2);

    cout<<xml.str()<<endl;

    xml.remove("object2");
    cout<<xml.str()<<endl;
    return 0;
}