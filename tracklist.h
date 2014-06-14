#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <QObject>
#include "track.h"
#define TRACKLIST_NEWTRACKER -1


class TrackList: public QObject
{   Q_OBJECT

public:

   TrackList();

   Track* searchByLabel(const char *label);
   Track* searchByPoint(Point2i p);
   Track* searchByIndex(int index);
   Track* getcurrentTrack();

   void setcurrentTrack(Track* current);
   void addTrack(Track* nuevo);
   void rmTrack(string label);
   void resetMotionAll();

   void updateTrackers();
   int size();

private:
  QList<Track *> content;
  Track* currentTrack;

signals:
  void onNewTracker(int index);
  void onRemoveTracker(int index);

};

#endif // TRACK_H
