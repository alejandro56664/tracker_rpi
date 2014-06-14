#pragma once
#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
class CVImageWidget : public QWidget
{
    Q_OBJECT
private:
    float factor_resize;
    bool mouse_down;
public:
    explicit CVImageWidget(QWidget *parent = 0) : QWidget(parent) {factor_resize=1.0;}

    QSize sizeHint() const { return _qimage.size(); }
    QSize minimumSizeHint() const { return _qimage.size(); }
    void scaleImage(double factor){ factor_resize=factor; }

signals:
    void onMouseDown(Point2i position);
    void onMouseUp(Point2i position);
    void onMouseDrag(Point2i position);
public slots:

    void showImage(const Mat& image) {
        // Convert the image to the RGB888 format
        switch (image.type()) {
        case CV_8UC1:
            cvtColor(image, _tmp, CV_GRAY2RGB);
            break;
        case CV_8UC3:
            cvtColor(image, _tmp, CV_BGR2RGB);
            break;
        }

        // QImage needs the data to be stored continuously in memory
        assert(_tmp.isContinuous());
        // Assign OpenCV's image buffer to the QImage. Note that the bytesPerLine parameter
        // (http://qt-project.org/doc/qt-4.8/qimage.html#QImage-6) is 3*width because each pixel
        // has three bytes.
        _qimage = QImage(_tmp.data, _tmp.cols, _tmp.rows, _tmp.cols*3, QImage::Format_RGB888);
        //_qimage = _qimage.scaled(_qimage.size()*factor_resize, Qt::KeepAspectRatio,Qt::FastTransformation);
        //this->setFixedSize(image.cols, image.rows);
        this->setFixedSize(_qimage.size());
        repaint();
    }

protected:

    QImage _qimage, _qimage_sca;
    cv::Mat _tmp;

    ///MEJORAR MANEJO DE EVENTOS
    void paintEvent(QPaintEvent* /*event*/) {
        // Display the image
        QPainter painter(this);
        painter.drawImage(QPoint(0,0), _qimage);  
        painter.end();
    }

    void mousePressEvent( QMouseEvent* event ){

        Point2i tmp;
        tmp.x = event->x();
        tmp.y = event->y();

        emit onMouseDown(tmp);
        mouse_down = true;

    }

    void mouseReleaseEvent( QMouseEvent* event ){

        Point2i tmp;
        tmp.x = event->x();
        tmp.y = event->y();

        emit onMouseUp(tmp);
        mouse_down = false;
    }

    void mouseMoveEvent( QMouseEvent* event ){

        Point2i tmp;
        tmp.x = event->x();
        tmp.y = event->y();

        if(mouse_down) emit onMouseDrag(tmp);
    }



};
