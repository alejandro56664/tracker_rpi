#include "segment.h"
#include "track.h"
#include <iostream>
#include <sstream>
#include <cmath>

using namespace std;
Segment::Segment(string label, Point2i p1, Point2i p2, Scalar color ) {
    int tmp;
    etiqueta=label;
    seg_ini=p1;
    seg_end = p2;
    isVertical = true;
    labelVisible = true;
    valueVisible = true;
    setColor(color);

    m=0.0;
    b=-1.0;

    tmp = (p2.x - p1.x);
    //el 3 es arbirario, diferencia en pixeles de la coordenada x
    if (abs(tmp)>3){
        m = (float)(p2.y - p1.y)/tmp;
        b = p1.y - m*p1.x;
        isVertical = false;
    }
    valuef = sqrt((p2.x - p1.x)*(p2.x - p1.x) + (p2.y - p1.y)*(p2.y - p1.y));

    pos_text.x=((p1.x+p2.x)/2 + DES_TEXT_RIGHT);
    pos_text.y=((p1.y+p2.y)/2 - DES_TEXT_UP);

    updateValue();

    cout << "(Segment::Segment) Se creo el objeto: "<< etiqueta << " m: "<< m <<", b: "<< b
         << " ("<<p1.x<<","<<p1.y<<")-("<<p2.x<<","<<p2.y<<")"<<endl;
}

Segment::Segment() {

    etiqueta = "";
    seg_ini= Point2i(0,0);
    seg_end = Point2i(0,0);
    isVertical = false;
    m = 0.0;
    b = -1.0;
    labelVisible = false;
    valueVisible = false;
}

void Segment::updateValue(){
    //conversion de float a string usando las funciones del c++ estandar
    ostringstream ss;
    String unit = (Track::typeunit==TRACKUNIT_METER)?" m":" cm";
    ss << valuef*Track::px2unit << unit;
    values = ss.str();
}



float Segment::evaluePoint(Point2i p){

    float yc, err;
    if(isVertical){
       err = (float)(p.x -seg_end.x )/seg_end.x ;
    }else{
       yc = m*p.x+b;
       err = (yc-p.y )/p.y ;
    }
    cout <<this->etiqueta<< " ("<<this->getStartPoint()->x <<","<<this->getStartPoint()->y
         <<")-("<<this->getEndPoint()->x <<","<<this->getEndPoint()->y <<")"<<", err: " << err <<" m: "<< m <<", b: "<< b<<endl;
    return err;
}

Scalar* Segment::getColor(uint tipo){
    Scalar* result;
    switch(tipo){
    case TRACKCOLOR_NORMAL:
        result = &color_normal;
        break;
    case TRACKCOLOR_SELECT:
        result = &color_select;
        break;
    case TRACKCOLOR_PATH:
        result = &color_path;
        break;
    }
    return result;

}

void Segment::setColor(Scalar newcolor){
    color_normal = newcolor;
    color_select = newcolor+Scalar(0,50,0);
    color_path = newcolor-Scalar(0,50,0);;

}


SegmentList::SegmentList(){
    currentSegment=NULL;
}

Segment* SegmentList::searchByLabel(const char *label){
    bool found = false;
    Segment* result;
    for (int i = 0; i < content.size(); ++i) {
        if (content.at(i)->label() == label){
            cout << "(SegmentList::searchByLabel) Found "<< content.at(i)->label() << " en la pos " << i << endl;
            found = true;
            result = content.at(i);
            break;
            }
    }
    if (!found) result = NULL;
    return result ;
}

Segment* SegmentList::searchByPoint(Point2i p){
    float err;
    bool found = false;
    Segment* result;
    cout << "(SegmentList::searchByPoint) evalue Point ("<< p.x  <<","<< p.y  << ")"<<endl;
    for (int i = 0; i < content.size(); ++i) {
        err = content.at(i)->evaluePoint(p);

        if (abs(err)<0.09){
            found = true;
            result = content.at(i);
            break;
            }
    }
    if (!found) result = NULL;
    return result;
}

void SegmentList::setcurrentSegment(Segment* current){
    currentSegment = current;
}

Segment* SegmentList::getcurrentSegment(){
    return currentSegment;
}

Segment* SegmentList::searchByIndex(int index){
    return content.at(index);
}

void SegmentList::addSegment(Segment* nuevo){
    content.append(nuevo);
    cout << "(SegmentList::addSegment) Se agrego "<< nuevo->label() <<  endl;
}

void SegmentList::delSegment(string label){
    for (int i = 0; i < content.size(); ++i) {
        if (content.at(i)->label() == label){
            content.removeAt(i);
        }
    }
}

void SegmentList::updateSegments(){
    for (int i = 0; i < content.size(); ++i) {
        content.at(i)->updateValue();
    }
}

int SegmentList::size(){
    return content.size();
}
