#ifndef MAIN_H
#define MAIN_H

// main.h
// 10/27/2014 jichi

#include <QtCore>
#include <QtGui>

class Recorder : public QDialog
{
  Q_OBJECT

  QTimer *videoTimer_;
public:
  Recorder()
  {
    videoTimer_ = new QTimer(this);
    videoTimer_->setSingleShot(false);
    videoTimer_->setInterval(1000/24.); // 24 per second
    connect(videoTimer_, SIGNAL(timeout()), SLOT(recordVideo()));
  }

  void startRecordingVideo() { videoTimer_->start(); }

private slots:
  void recordVideo()
  {
    QWidget *w = qApp->desktop();
    QPixmap pm = QPixmap::grabWidget(w);
    pm.save("avtest.png");
  }
};


#endif // MAIN_H
