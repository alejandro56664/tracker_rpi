#include "plotwindow.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QMetaEnum>
#include <QVector>
#include <iostream>

using namespace std;
PlotWindow::PlotWindow(QWidget *parent, VideoCore *_player, TrackList* lista) :
  QWidget(parent)
{
  //setGeometry(400, 250, 542, 390);
    listen_signal = false;
    type_graph = PATH;
    index_tracker_selected = 0;
    data_component = DATA_COMPONENT_X;
    setVideoPlayer(_player);
    setTrackList(lista);


     customPlot = new QCustomPlot(this);
     cb_TrackersAvailables = new QComboBox();
     cb_TrackersDataType = new QComboBox();
     cb_DataComponents = new QComboBox();

     QGridLayout *layout = new QGridLayout();
     addTracer(0.0);

     layout->addWidget(customPlot,0,0,1,3);
     layout->addWidget(cb_TrackersAvailables,1,0);
     layout->addWidget(cb_TrackersDataType,1,1);
     layout->addWidget(cb_DataComponents,1,2);
     setLayout(layout);
     //configuracion del plot

     setStylePlot();

     cb_TrackersDataType->addItem(tr("Trayectoria"));
     cb_TrackersDataType->addItem(tr("Velocidad"));
     cb_TrackersDataType->addItem(tr("Aceleración"));

     cb_DataComponents->addItem(tr("Componente en x"));
     cb_DataComponents->addItem(tr("Componente en y"));
     cb_DataComponents->addItem(tr("Magnitud"));
     QShortcut *shortcut_change_plot = new QShortcut(QKeySequence("Ctrl+Down"), this);
     QObject::connect(shortcut_change_plot, SIGNAL(activated()), this, SLOT(changeGraph()));
     connect(cb_TrackersAvailables, SIGNAL(activated(int)),this, SLOT(on_TrackersAvailables_actived(int)));
     connect(cb_TrackersDataType, SIGNAL(activated(int)),this, SLOT(on_DataType_actived(int)));
    connect(cb_DataComponents, SIGNAL(activated(int)),this, SLOT(on_DataComponents_actived(int)));



/****************************************************************************************/
     connect(Trackers, SIGNAL(onNewTracker(int)),this, SLOT(cb_addNewTracker(int)));
     connect(Trackers, SIGNAL(onRemoveTracker(int)),this, SLOT(cb_removeTracker(int)));

     connect(Player, SIGNAL(onNextFrame()),this, SLOT(updateGraph()));
     // connect slot that ties some axis selections together (especially opposite axes):
     connect(customPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
     // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
     connect(customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
     connect(customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

     // make bottom and left axes transfer their ranges to top and right axes:
     connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
     connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

     // connect some interaction slots:
     connect(customPlot, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart)));
     connect(customPlot, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*)));

     // connect slot that shows a message in the status bar when a graph is clicked:
     connect(customPlot, SIGNAL(plottableClick(QCPAbstractPlottable*,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*)));

     // setup policy and connect slot for context menu popup:
     customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
     connect(customPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
     //
   std::cout<< "Se creo la ventana de graficas satisfactoriamente"<< std::endl;
}
void PlotWindow::changeGraph(){
    int num_trackers =Trackers->size();
    int max = 3*num_trackers;
    static int cont=0;

    //se guarda una copia del dato, para cuando salga dejarlo intacto
    int index_tracker_selected_back;
    index_tracker_selected_back = index_tracker_selected;

    cont = (cont<max)?cont+1:0;

    data_component = 0;
    index_tracker_selected = (int)(cont/3);
    type_graph = cont%3;
    loadGraph();

    index_tracker_selected = index_tracker_selected_back;

}

void PlotWindow::loadGraph(){
    Track* tmp = Trackers->searchByIndex(index_tracker_selected);
    double xmin = 0.0, xmax = 0.0;
    double ymin = 0.0, ymax = 0.0, tmax=0.0;
    double mmin = 0.0,mmax = 0.0;
    if (!tmp){
        cout<<"El elemnto no existe"<< endl;
    }else {
        string label = tmp->label();
        //se revisa el tipo de grafica seleccionado
        tmp->estimateRelativePosition();
        tmp->estimateMotion();
        removeAllGraphs();
        switch (type_graph) {
            case PATH:{
                int size = tmp->path.size();
                cout<<"desde plot size path:"<<size<<endl;
                QVector<double> x(size), y(size);
                //Se obtienen los datos
                for (int i=0; i<size; i++)
                {
                  x[i] = tmp->path[i].x;
                  y[i] = tmp->path[i].y;

                  xmin = (x[i]<xmin)?x[i]:xmin;
                  ymin = (y[i]<ymin)?y[i]:ymin;

                  xmax = (x[i]>xmax)?x[i]:xmax;
                  ymax = (y[i]>ymax)?y[i]:ymax;
                }

                QCPCurve *trayectoria = new QCPCurve(customPlot->xAxis, customPlot->yAxis);
                customPlot->addPlottable(trayectoria);
                trayectoria->setData(x, y);
                trayectoria->setName(QString::fromStdString(label).append(" - Trayectoria"));
                //trayectoria->setLineStyle((QCPCurve::LineStyle)(rand()%5+1));
                trayectoria->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(rand()%9+1)));
                QPen graphPen;
                graphPen.setColor(QColor(rand()%155+100, rand()%155+100, rand()%155+100));
                graphPen.setWidthF(2);
                trayectoria->setPen(graphPen);
                customPlot->xAxis->setRange(xmin, xmax*1.1);
                customPlot->yAxis->setRange(ymin, ymax*1.1);
                customPlot->xAxis->setLabel("x Axis");
                customPlot->yAxis->setLabel("y Axis");
                //addTracer(x[0]);
                customPlot->replot();

                break;}
            case VELOCITY:

                int size = tmp->velocity.size();
                QVector<double> t(size), x(size), y(size), magn(size);
                float tempx, tempy;
                int i;
                int i_0 = tmp->frameStartTrack;
                //Se obtienen los datos
                for (i=0; i<size; i++)
                {

                  tempx = tmp->velocity[i].x;
                  tempy = tmp->velocity[i].y;

                  x[i] = tempx;
                  y[i] = tempy;
                  t[i] = (i+i_0 )/Track::samplingRate;
                  magn[i] = tempx*tempx + tempy*tempy ;

                  xmin = (x[i]<xmin)?x[i]:xmin;
                  ymin = (y[i]<ymin)?y[i]:ymin;
                  mmin = (magn[i]<mmin)?magn[i]:mmin;

                  xmax = (x[i]>xmax)?x[i]:xmax;
                  ymax = (y[i]>ymax)?y[i]:ymax;
                  tmax = (t[i]>tmax)?t[i]:tmax;
                  mmax = (magn[i]>mmax)?magn[i]:mmax;

                }
                customPlot->addGraph();

                customPlot->graph()->setName(QString::fromStdString(label).append(" - Velocidad"));

                switch(data_component){
                case DATA_COMPONENT_X:
                    customPlot->graph()->setData(t, x);
                    customPlot->xAxis->setRange(0, tmax);
                    customPlot->yAxis->setRange(xmin, xmax*1.1);
                    customPlot->xAxis->setLabel("tiempo(s)");
                    customPlot->yAxis->setLabel("Velocidad en x");
                    break;
                case DATA_COMPONENT_Y:
                    customPlot->graph()->setData(t, y);
                    customPlot->xAxis->setRange(0, tmax);
                    customPlot->yAxis->setRange(ymin, ymax*1.1);
                    customPlot->xAxis->setLabel("tiempo(s)");
                    customPlot->yAxis->setLabel("Velocidad en y");
                    break;
                case DATA_MAGNITUDE:
                    customPlot->graph()->setData(t, magn);
                    customPlot->xAxis->setRange(0, tmax);
                    customPlot->yAxis->setRange(mmin, mmax*1.1);
                    customPlot->xAxis->setLabel("tiempo(s)");
                    customPlot->yAxis->setLabel("Velocidad");
                    break;
                }

                addTracer(t[0]);

                setStyleGraph();

                break;
          }

     }
}

void PlotWindow::on_TrackersAvailables_actived(int index){
    index_tracker_selected = index;
    cout<< "Selecciono el objeto "<< index<< endl;
    listen_signal = true;
    loadGraph();
}

void PlotWindow::on_DataComponents_actived(int component){
    data_component = component;
    loadGraph();
}

void PlotWindow::on_DataType_actived(int type){
    type_graph =  type;
    loadGraph();
}

void PlotWindow::updateGraph() {
    if(listen_signal){
        Track* tmp = Trackers->searchByIndex(index_tracker_selected);
        if (!tmp){

        }else{

        //cout<<"se actualiza el plot"<< endl;
        // cargar la posicion actual del tracker

            int currentFrame = Player->getCurrentFrame();
            if(currentFrame>= tmp->frameStartTrack){
                itemDemoPhaseTracer->setGraphKey(currentFrame/Track::samplingRate);
            }
            customPlot->replot();
          }
       }
}

void PlotWindow::addTracer(float pos_ini){
    customPlot->clearItems();
    QCPItemTracer *phaseTracer = new QCPItemTracer(customPlot);
    customPlot->addItem(phaseTracer);
    itemDemoPhaseTracer = phaseTracer; // so we can access it later in the bracketDataSlot for animation
    phaseTracer->setGraph(customPlot->graph());
    phaseTracer->setGraphKey(pos_ini);
    phaseTracer->setInterpolating(true);
    phaseTracer->setStyle(QCPItemTracer::tsCrosshair);
    phaseTracer->setPen(QPen(Qt::white));
    phaseTracer->setBrush(Qt::white);
    phaseTracer->setSize(10);
}

void PlotWindow::cb_addNewTracker(int index){
    string label = Trackers->searchByIndex(index)->label();
    cb_TrackersAvailables->addItem(QString::fromStdString(label));
    cout<< "Se actualizo los trackers disponibles en combobox"<< endl;
}

void PlotWindow::cb_removeTracker(int index){
    cb_TrackersAvailables->removeItem(index);
    index_tracker_selected = 0;
    cout<< "Se actualizo los trackers disponibles en combobox"<< endl;
}


void PlotWindow::setVideoPlayer(VideoCore *_player){
    Player = _player;
}

void PlotWindow::setTrackList(TrackList* _trackers){
    Trackers = _trackers;

}
PlotWindow::~PlotWindow()
{
  delete ui;
}


void PlotWindow::axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{
  // Set an axis label by double clicking on it
  if (part == QCPAxis::spAxisLabel) // only react when the actual axis label is clicked, not tick label or axis backbone
  {
    bool ok;
    QString newLabel = QInputDialog::getText(this, "Plot", "Nueva Etiqueta", QLineEdit::Normal, axis->label(), &ok);
    if (ok)
    {
      axis->setLabel(newLabel);
     customPlot->replot();
    }
  }
}

void PlotWindow::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
  // Rename a graph by double clicking on its legend item
  Q_UNUSED(legend)
  if (item) // only react if item was clicked (user could have clicked on border padding of legend where there is no item, then item is 0)
  {
    QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
    bool ok;
    QString newName = QInputDialog::getText(this, "Plot", "Nuevo nombre para la grafica", QLineEdit::Normal, plItem->plottable()->name(), &ok);
    if (ok)
    {
      plItem->plottable()->setName(newName);
      customPlot->replot();
    }
  }
}

void PlotWindow::selectionChanged()
{
  /*
   normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
   the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
   and the axis base line together. However, the axis label shall be selectable individually.

   The selection state of the left and right axes shall be synchronized as well as the state of the
   bottom and top axes.

   Further, we want to synchronize the selection of the graphs with the selection state of the respective
   legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
   or on its legend item.
  */

  // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || customPlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    customPlot->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    customPlot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }
  // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || customPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    customPlot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    customPlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }

  // synchronize selection of graphs with selection of corresponding legend items:
  for (int i=0; i<customPlot->graphCount(); ++i)
  {
    QCPGraph *graph = customPlot->graph(i);
    QCPPlottableLegendItem *item = customPlot->legend->itemWithPlottable(graph);
    if (item->selected() || graph->selected())
    {
      item->setSelected(true);
      graph->setSelected(true);
    }
  }
}

void PlotWindow::mousePress()
{
  // if an axis is selected, only allow the direction of that axis to be dragged
  // if no axis is selected, both directions may be dragged

  if (customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    customPlot->axisRect()->setRangeDrag(customPlot->xAxis->orientation());
  else if (customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    customPlot->axisRect()->setRangeDrag(customPlot->yAxis->orientation());
  else
    customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void PlotWindow::mouseWheel()
{
  // if an axis is selected, only allow the direction of that axis to be zoomed
  // if no axis is selected, both directions may be zoomed

  if (customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    customPlot->axisRect()->setRangeZoom(customPlot->xAxis->orientation());
  else if (customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
   customPlot->axisRect()->setRangeZoom(customPlot->yAxis->orientation());
  else
    customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void PlotWindow::addRandomGraph()
{
  int n = 50; // number of points in graph
  double xScale = (rand()/(double)RAND_MAX + 0.5)*2;
  double yScale = (rand()/(double)RAND_MAX + 0.5)*2;
  double xOffset = (rand()/(double)RAND_MAX - 0.5)*4;
  double yOffset = (rand()/(double)RAND_MAX - 0.5)*5;
  double r1 = (rand()/(double)RAND_MAX - 0.5)*2;
  double r2 = (rand()/(double)RAND_MAX - 0.5)*2;
  double r3 = (rand()/(double)RAND_MAX - 0.5)*2;
  double r4 = (rand()/(double)RAND_MAX - 0.5)*2;
  QVector<double> x(n), y(n);
  for (int i=0; i<n; i++)
  {
    x[i] = (i/(double)n-0.5)*10.0*xScale + xOffset;
    y[i] = (sin(x[i]*r1*5)*sin(cos(x[i]*r2)*r4*3)+r3*cos(sin(x[i])*r4*2))*yScale + yOffset;
  }

  customPlot->addGraph();
  customPlot->graph()->setName(QString("New graph %1").arg(customPlot->graphCount()-1));
  setStyleGraph();

}

void PlotWindow::setStyleGraph(){
    //customPlot->graph()->setLineStyle((QCPGraph::LineStyle)(rand()%5+1));
    customPlot->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(rand()%9+1)));
    QPen graphPen;
    graphPen.setColor(QColor(rand()%155+100, rand()%155+100, rand()%155+100));
    graphPen.setWidthF(2);
    customPlot->graph()->setPen(graphPen);
    customPlot->replot();
}

void PlotWindow::setStylePlot(){
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);

    customPlot->axisRect()->setupFullAxesBox();

    customPlot->xAxis->setRange(-1, 1);
    customPlot->yAxis->setRange(-1, 1);
    customPlot->xAxis->setLabel("x Axis");
    customPlot->yAxis->setLabel("y Axis");
    customPlot->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);

    customPlot->legend->setFont(legendFont);
    customPlot->legend->setSelectedFont(legendFont);
    customPlot->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items
    // set some pens, brushes and backgrounds:
    customPlot->xAxis->setBasePen(QPen(Qt::white, 1));
    customPlot->yAxis->setBasePen(QPen(Qt::white, 1));
    customPlot->xAxis->setTickPen(QPen(Qt::white, 1));
    customPlot->yAxis->setTickPen(QPen(Qt::white, 1));
    customPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));
    customPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));
    customPlot->xAxis->setTickLabelColor(Qt::white);
    customPlot->yAxis->setTickLabelColor(Qt::white);
    customPlot->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    customPlot->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    customPlot->xAxis->grid()->setSubGridVisible(true);
    customPlot->yAxis->grid()->setSubGridVisible(true);
    customPlot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
    customPlot->yAxis->grid()->setZeroLinePen(Qt::NoPen);
    customPlot->xAxis->setSelectedLabelColor(QColor(0, 240, 0));
    customPlot->yAxis->setSelectedLabelColor(QColor(0, 240, 0));
    customPlot->xAxis->setSelectedTickLabelColor(QColor(0, 240, 0));
    customPlot->yAxis->setSelectedTickLabelColor(QColor(0, 240, 0));
    customPlot->xAxis->setSelectedTickPen(QColor(0, 240, 0));
    customPlot->yAxis->setSelectedTickPen(QColor(0, 240, 0));
    customPlot->xAxis->setSelectedBasePen(QColor(0, 240, 0));
    customPlot->yAxis->setSelectedBasePen(QColor(0, 240, 0));
    customPlot->xAxis->setSelectedSubTickPen(QColor(0, 240, 0));
    customPlot->yAxis->setSelectedSubTickPen(QColor(0, 240, 0));
    customPlot->xAxis->setLabelColor(Qt::white);
    customPlot->yAxis->setLabelColor(Qt::white);
    //customPlot->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    //customPlot->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    QLinearGradient plotGradient;
    plotGradient.setStart(0, 0);
    plotGradient.setFinalStop(0, 350);
    plotGradient.setColorAt(0, QColor(100, 100, 100));
    plotGradient.setColorAt(1, QColor(70, 70, 70));
    customPlot->setBackground(plotGradient);
    QLinearGradient axisRectGradient;
    axisRectGradient.setStart(0, 0);
    axisRectGradient.setFinalStop(0, 350);
    axisRectGradient.setColorAt(0, QColor(90, 90, 90));
    axisRectGradient.setColorAt(1, QColor(40, 40, 40));
    customPlot->axisRect()->setBackground(axisRectGradient);
}

void PlotWindow::removeSelectedGraph()
{
  if (customPlot->selectedGraphs().size() > 0)
  {
    customPlot->removeGraph(customPlot->selectedGraphs().first());
    customPlot->replot();
  }else if (customPlot->selectedPlottables().size() > 0)
  {
    customPlot->removePlottable(customPlot->selectedPlottables().first());
    customPlot->replot();
  }
}

void PlotWindow::removeAllGraphs()
{
  customPlot->clearPlottables();
  customPlot->clearGraphs();
  customPlot->replot();
}

void PlotWindow::contextMenuRequest(QPoint pos)
{
  QMenu *menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);

  if (customPlot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
  {
    menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
    menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
    menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
    menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
    menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
  } else  // general context menu on graphs requested
  {
    menu->addAction("Add random graph", this, SLOT(addRandomGraph()));
    if (customPlot->selectedGraphs().size() > 0 || customPlot->selectedPlottables().size() > 0 )
      menu->addAction("Eliminar grafica seleccionada", this, SLOT(removeSelectedGraph()));
    if (customPlot->graphCount() > 0)
      menu->addAction("Eliminar todas las graficas", this, SLOT(removeAllGraphs()));
  }

  menu->popup(customPlot->mapToGlobal(pos));
}

void PlotWindow::moveLegend()
{
  if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
  {
    bool ok;
    int dataInt = contextAction->data().toInt(&ok);
    if (ok)
    {
     customPlot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
     customPlot->replot();
    }
  }
}



