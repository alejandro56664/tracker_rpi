#include "track.h"
#include <iostream>
#include <sstream>
#include <cmath>

using namespace std;
using namespace cv;

Point2i Track::origin = Point2i(0,0);
float Track::px2unit = 0.01;
int Track::typeunit = TRACKUNIT_METER;
float Track::samplingRate = 30.0;

Track::Track(string label, Point2i c, int _framestarttrack, Scalar color ) {
    etiqueta=label;
    labelVisible = true;
    pathVisible = true;
    frameStartTrack=_framestarttrack;
    frameEndTrack = 32767; //valor maximo posible
    setColor(color);
    setCenter(c);
    //se agrega la posición inicial en el 'path'
    path_px.push_back(c);
    path.push_back(posRelative(c));
    velocity.push_back(Point2f(0.0,0.0));
    //cout << "(Track:: Track) Se creo el objeto: "<< etiqueta << " en: "
    //     << " ("<<c.x<<","<<c.y<<")"<<endl;
}

Track::Track() {

    etiqueta = "";
    setCenter(Point2i(0,0));
    labelVisible = false;
}

Scalar* Track::getColor(uint tipo){
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

void Track::setColor(Scalar newcolor){
    color_normal = newcolor;
    color_select = newcolor+Scalar(0,50,0);
    color_path = newcolor-Scalar(0,30,0);;

}

void Track::setCenter(Point2i p){
    centro = p;
    p1 = Point2i(centro.x-DEF_ROI1/2, centro.y-DEF_ROI1/2 );
    p2 = Point2i(centro.x+DEF_ROI1/2, centro.y+DEF_ROI1/2 );
    pos_text.x = centro.x;
    pos_text.y= centro.y+ DEF_ROI1/2+15;
}

bool Track::evaluePoint(Point2i p){
    bool result = false;
    if (p.x < p2.x && p.x > p1.x){
        if (p.y < p2.y && p.y > p1.y){
            result = true;
        }
     }
    return result;
}

void Track::resetMotion(){
    path_px.clear();
    path.clear();
    velocity.clear();

}

Point2i* Track::getPosinPath(int frame){
    //PULIR ESTA MUY CHAMBON
    Point2i* result;
    int size = path_px.size()-1;
    int relativeframe = (frame-frameStartTrack);

    if (relativeframe>0)
        result = &path_px[(relativeframe<size)?relativeframe:size];
    else
        result=NULL;

    return result;

}

void Track::setPosinPath(uint frame, Point2i p){
    int size = path_px.size();
    int relativeframe;
    Point2i P;
        relativeframe = frame - frameStartTrack;
        //cout<< "setPosicion relativeframe: "<<relativeframe<<" size:"<<size<<endl;
        if (relativeframe <= size){
             path_px[relativeframe]=p;
             path[relativeframe] = posRelative(p);
        }else{
            //se rellena las posiciones intermedias con la ultima posición
            for(int i=size;i<=(relativeframe-size);i++){
                P = path_px[size-1];
                path_px.push_back(P);
               path.push_back(posRelative(P));
            }
            path_px.push_back(p);
            path.push_back(posRelative(p));
        }

}

void Track::estimateRelativePosition(){
    int size_px = path_px.size();
    path.clear();
    //cout<<"Inicia actualizacion Posicion relativa"<< endl;
    if(path.size()==0){
        for(int i=0;i<size_px;i++){
           path.push_back(posRelative(path_px[i]));
           //cout<<"Pos(px): ("<<path_px[i].x<<","<<path_px[i].y<<"), Pos(units): "<<path[i].x<<","<<path[i].y<<endl;
        }
    }
}

void Track::estimateMotion(){
     int size;
     Point2f Vi;
     size = path_px.size();
     //cout<<"estimateMotion size: "<<size<<endl;
     //cout<<"Vel(units/s): ("<<velocity[0].x<<","<<velocity[0].y<<")"<<endl;
     if(size>1){
         for(int i=1;i<=size;i++){
             Point2f Pi = path[i];
             Point2f Pi_1 = path[i-1];
             Vi.x = (Pi.x - Pi_1.x)*samplingRate;
             Vi.y = (Pi.y - Pi_1.y)*samplingRate;
             if(velocity.size()<= i){
                 velocity.push_back(Vi);
             }else{
                 velocity[i] = Vi;
             }
             //cout<<"Vel(units/s): ("<<Vi.x<<","<<Vi.y<<")"<<endl;
         }

     }
 }
void Track::setOrigin(Point2i _origin){
    origin=_origin;
    //cout<<"Se estableció el origen en: ("<< origin.x <<","<< origin.y<<")"<< endl;
}
void Track::setMeasure(float pixels, float measure, int unit ) {
    px2unit=measure/pixels;
    typeunit = unit;
    //cout<<"Se estableció el factor de conversión: "<< px2unit <<"unidades/pixel"<< endl;
}
void Track::setSamplingRate(float _samplingrate) {
    samplingRate=_samplingrate;
    //cout<<"Se estableció el periodo de muestreo en: "<< samplingRate <<" seg"<< endl;
}

void Track::setEndTrack(int endframe){
    int new_size;
    frameEndTrack = endframe;
    new_size = frameEndTrack - frameStartTrack;
    path_px.resize(new_size);
    path.resize(new_size);
    velocity.resize(new_size);
}

