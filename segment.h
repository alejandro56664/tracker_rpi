#ifndef SEGMENT_H
#define SEGMENT_H

#include <QList>
#include <opencv2/opencv.hpp>
#include <string>

#define DES_TEXT_RIGHT  10
#define DES_TEXT_UP  20

#define TRACKCOLOR_NORMAL 0
#define TRACKCOLOR_SELECT 1
#define TRACKCOLOR_PATH 2
using namespace std;
using namespace cv;
class Segment
{
public:
   Segment(string label, Point2i p1, Point2i p2 , Scalar color);
   Segment();
   string label() const         { return etiqueta; }
   string value()               { return values; }
   double value_f()               { return valuef; }
   void setLabel(string label)  {etiqueta = label;}

   float evaluePoint(Point2i p);

   Point2i* getStartPoint()            { return &seg_ini; }
   Point2i* getEndPoint()              { return &seg_end; }
   Point2i* getPostext()               { return &pos_text; }

   void setStartPoint(Point2i p)       {  seg_ini=p; }
   void setEndPoint(Point2i p)         {  seg_end=p; }

   void setColor(Scalar newcolor);
   Scalar* getColor(uint tipo);


   void setLabelVisible(bool state) { labelVisible = state; }
   bool isLabelVisible()   { return labelVisible; }

   void setValueVisible(bool state) { valueVisible = state; }
   bool isValueVisible()   { return valueVisible; }

   void updateValue();


private:
   string etiqueta;
   string values;

   Point2i seg_ini;
   Point2i seg_end;
   Point2i pos_text;

   Scalar color_normal;
   Scalar color_select;
   Scalar color_path;

   float  m;
   float  b;
   float valuef;
   bool isVertical;
   bool labelVisible;
   bool valueVisible;

};

class SegmentList
{
public:

   SegmentList();

   Segment* searchByLabel(const char *label);
   Segment* searchByPoint(Point2i p);
   Segment* searchByIndex(int index);
   Segment* getcurrentSegment();

   void setcurrentSegment(Segment* current);
   void addSegment(Segment* nuevo);
   void delSegment(string label);
   void updateSegments();
   int size();


private:
  QList<Segment *> content;
  Segment* currentSegment;
};

#endif // SEGMENT_H
