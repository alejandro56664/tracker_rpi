#include "tracklist.h"

TrackList::TrackList()
{
    currentTrack=NULL;
}

Track* TrackList::searchByLabel(const char *label){
    bool found = false;
    Track* result;
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

Track* TrackList::searchByPoint(Point2i p){
    bool found = false;
    Track* result;
    //cout << "(TrackList::searchByPoint) evalue Point ("<< p.x  <<","<< p.y  << ")"<<endl;
    for (int i = 0; i < content.size(); ++i) {
        if(content.at(i)->evaluePoint(p)){
            found = true;
            result = content.at(i);
            break;
            }
    }
    if (!found) result = NULL;
    return result;
}

void TrackList::setcurrentTrack(Track* current){
    currentTrack = current;
}

Track* TrackList::getcurrentTrack(){
    return currentTrack;
}

Track* TrackList::searchByIndex(int index){
    Track* result;
    if(content.size()>index){
        result = content.at(index);
    } else {
        result = NULL;
    }
    return result;
}

void TrackList::addTrack(Track* nuevo){
    content.append(nuevo);
    emit onNewTracker(content.size()-1);
    cout << "(TrackList::addSTrack) Se agrego "<< nuevo->label() << " se emite la señal onNewTracker()"<< endl;
}

void TrackList::rmTrack(string label){
    cout<<"(TrackList::rmTracker) se va a eliminar un elemento"<<endl;
    for (int i = 0; i < content.size(); ++i) {
        if (content.at(i)->label() == label){
            content.removeAt(i);
            emit onRemoveTracker(i);
        }
    }

}

void TrackList::resetMotionAll(){
    for (int i = 0; i < content.size(); ++i) {
        content.at(i)->resetMotion();
    }
}

void TrackList::updateTrackers(){
    for (int i = 0; i < content.size(); ++i) {
        content.at(i)->estimateRelativePosition();
        content.at(i)->estimateMotion();
    }
}

int TrackList::size(){
    return content.size();
}
