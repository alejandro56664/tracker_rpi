#ifndef PLOTVEL_H
#define PLOTVEL_H

#include <QMainWindow>
#include <QTimer>
#include <QComboBox>
#include "qcustomplot.h" // the header file of QCustomPlot. Don't forget to add it to your project, if you use an IDE, so it gets compiled.
#include "track.h"
#include "tracklist.h"
#include "videocore.h"

#define PATH            0
#define VELOCITY        1
#define ACCELERATION    2

#define DATA_COMPONENT_X     0
#define DATA_COMPONENT_Y     1
#define DATA_MAGNITUDE       2

class PlotWindow : public QWidget
{
  Q_OBJECT

public:
  PlotWindow(QWidget *parent = 0, VideoCore* _player=NULL, TrackList* lista=NULL);
  ~PlotWindow();

  void setVideoPlayer(VideoCore* _player);
  void setTrackList(TrackList* _trackers);


  void setupItemDemo(QCustomPlot *customPlot);

private slots:

  void on_TrackersAvailables_actived(int index);
  void on_DataComponents_actived(int component);
  void on_DataType_actived(int type);

  void changeGraph();

  void cb_addNewTracker(int index);
  void cb_removeTracker(int index);

  void updateGraph();

  void axisLabelDoubleClick(QCPAxis* axis, QCPAxis::SelectablePart part);
  void legendDoubleClick(QCPLegend* legend, QCPAbstractLegendItem* item);
  void selectionChanged();
  void mousePress();
  void mouseWheel();
  void addRandomGraph();
  void removeSelectedGraph();
  void removeAllGraphs();
  void contextMenuRequest(QPoint pos);
  void moveLegend();
  void loadGraph();
  void addTracer(float pos_ini);

private:
  PlotWindow *ui;
  QCustomPlot *customPlot;
  QCPItemTracer *itemDemoPhaseTracer;

  QComboBox *cb_TrackersAvailables;
  QComboBox *cb_TrackersDataType;
  QComboBox *cb_DataComponents;

  VideoCore* Player;
  TrackList* Trackers;

  int index_tracker_selected;
  int type_graph;
  int data_component;


  bool listen_signal;

  int frame;

  void setStylePlot();
  void setStyleGraph();
};
#endif // PLOTVEL_H
