#ifndef TRACK_H
#define TRACK_H

#include <QList>
#include <opencv2/opencv.hpp>
#include <string>
#define DEF_ROI1 20
#define TRACKCOLOR_NORMAL 0
#define TRACKCOLOR_SELECT 1
#define TRACKCOLOR_PATH 2

#define TRACKUNIT_METER 1
#define TRACKUNIT_CENTIMETER 100


using namespace std;
using namespace cv;
class Track
{
public:

   Track(string label, Point2i c, int _framestarttrack, Scalar color);
   Track();
   string label() const         { return etiqueta; }
   void setLabel(string label)  {etiqueta = label;}

   bool evaluePoint(Point2i p);

   Point2i* getStartPoint()            { return &p1; }
   Point2i* getEndPoint()              { return &p2; }
   Point2i* getPostext()               { return &pos_text; }
   Point2i* getCenter()            { return &centro; }

   void setCenter(Point2i p);
   void setColor(Scalar newcolor);
   Scalar* getColor(uint tipo);

   inline Point2f posRelative(Point2i p){
       Point2f P;
       P.x = (p.x - origin.x)*px2unit;
       P.y = (-p.y + origin.y)*px2unit;
       return P;

   }
   static void setMeasure(float pixels, float measure, int unit );
   static void setSamplingRate(float _samplingrate);
   static void setOrigin(Point2i _origin);

   void setLabelVisible(bool state) { labelVisible = state; }
   bool isLabelVisible()   { return labelVisible; }
   bool isPathVisible()   { return pathVisible; }
   void estimateMotion();
   void estimateRelativePosition();
   void setPosinPath(uint frame, Point2i p);

   void setEndTrack(int endframe);
   void resetMotion();
   Point2i* getPosinPath(int frame);

   int frameStartTrack;
   int frameEndTrack;

   vector<Point2i> path_px;
   vector<Point2f> path;
   vector<Point2f> velocity;

   static Point2i origin;
   static float px2unit;
   static int typeunit;
   static float samplingRate;

private:
   string etiqueta;

   Point2i centro;
   Point2i p1;
   Point2i p2;
   Point2i pos_text;

   Scalar color_normal;
   Scalar color_select;
   Scalar color_path;

   bool labelVisible;
   bool pathVisible;
};

#endif
