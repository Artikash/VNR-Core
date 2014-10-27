#ifndef MAIN_H
#define MAIN_H

// main.h
// 10/27/2014 jichi

#include <QtCore>
#include <QtGui>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
//#include <libswscale/swscale.h>
} // extern "C"


class Recorder : public QDialog
{
  Q_OBJECT

  QTimer *videoTimer_;
  QString path_;
public:
  Recorder()
  {
    path_  = "avtest.mp4";
    videoTimer_ = new QTimer(this);
    videoTimer_->setSingleShot(false);
    videoTimer_->setInterval(1000/24.); // 24 per second
    connect(videoTimer_, SIGNAL(timeout()), SLOT(recordVideo()));
  }

  bool startRecordingVideo()
  {
    // Init ffmpeg stuff
    avcodec_register_all();
    av_register_all();

    // Format
    AVOutputFormat *m_outputFormat = av_guess_format(NULL, path_.toUtf8().constData(), NULL);
    if (!m_outputFormat) {
      //q_ptr->setError(Encoder::InvalidOutputFormatError, tr("Unable to get an output format by passed filename."));
      qDebug() << "Unable to get an output format by passed filename.";
      return false;
    }

    AVFormatContext *m_formatContext = avformat_alloc_context();
    m_formatContext->oformat = m_outputFormat;

    // Create video
    //AVStream *m_videoStream = av_new_stream(m_formatContext,0);
    AVStream *m_videoStream = avformat_new_stream(m_formatContext, nullptr);
    if (!m_videoStream) {
      //q_ptr->setError(Encoder::InvalidVideoStreamError, tr("Unable to add video stream."));
      qDebug() << "Unable to add video stream.";
      return false;
    }

    // Set up video codec
    AVCodecContext *m_videoCodecContext = m_videoStream->codec;

    //m_videoCodecContext->codec_id = (videoCodec() == EncoderGlobal::DEFAULT_VIDEO_CODEC) ? m_outputFormat->video_codec : static_cast<CodecID>(videoCodec());
    m_videoCodecContext->codec_id = m_outputFormat->video_codec;

    m_videoCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;

    //m_videoCodecContext->width = videoSize().width();
    //m_videoCodecContext->height = videoSize().height();
    m_videoCodecContext->width = 1440;
    m_videoCodecContext->height = 900;

    //m_videoCodecContext->pix_fmt = (PixelFormat)outputPixelFormat();
    m_videoCodecContext->pix_fmt = AV_PIX_FMT_RGBA;
    //m_videoCodecContext->time_base.den = fixedFrameRate() != -1 ? fixedFrameRate() : 1000;
    m_videoCodecContext->time_base.den = 1000;
    m_videoCodecContext->time_base.num = 1;

    //applyVideoCodecSettings();

    // Start timing
    //videoTimer_->start();
    qDebug() << "success";
    return true;
  }

private slots:
  void recordVideo()
  {
    QWidget *w = qApp->desktop();
    QPixmap pm = QPixmap::grabWindow(w->winId());
    pm.save("avtest.png");
  }
};


#endif // MAIN_H
