#ifndef MAIN_H
#define MAIN_H

// main.h
// 10/27/2014 jichi

#include <QtCore>
#include <QtGui>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
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

  // See: qtmel/src/encoder/encoder.cpp
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
    //m_videoCodecContext->codec_id = m_outputFormat->video_codec;
    //qDebug() << "video codec id" << m_videoCodecContext->codec_id;
    m_videoCodecContext->codec_id =  AV_CODEC_ID_H264;

    m_videoCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;

    //m_videoCodecContext->width = videoSize().width();
    //m_videoCodecContext->height = videoSize().height();
    m_videoCodecContext->width = 1280;
    m_videoCodecContext->height = 720;

    //m_videoCodecContext->pix_fmt = (PixelFormat)outputPixelFormat();
    m_videoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    //m_videoCodecContext->time_base.den = fixedFrameRate() != -1 ? fixedFrameRate() : 1000;
    m_videoCodecContext->time_base.den = 1000;
    m_videoCodecContext->time_base.num = 1;

    //applyVideoCodecSettings();
    // http://stackoverflow.com/questions/3553003/encoding-h-264-with-libavcodec-x264
    m_videoCodecContext->bit_rate = 5000 * 1000;
    auto ctx = m_videoCodecContext;
    ctx->bit_rate_tolerance;
    ctx->rc_max_rate = 0;
    ctx->rc_buffer_size = 0;
    ctx->gop_size = 40;
    ctx->max_b_frames = 3;
    ctx->b_frame_strategy = 1;
    ctx->coder_type = 1;
    ctx->me_cmp = 1;
    ctx->me_range = 16;
    ctx->qmin = 10;
    ctx->qmax = 51;
    ctx->scenechange_threshold = 40;
    ctx->flags |= CODEC_FLAG_LOOP_FILTER;
    ctx->me_method = ME_HEX;
    ctx->me_subpel_quality = 5;
    ctx->i_quant_factor = 0.71;
    ctx->qcompress = 0.6;
    ctx->max_qdiff = 4;
    //ctx->directpred = 1;
    //ctx->flags2 |= CODEC_FLAG2_FASTPSKIP;

    // Open video stream
    AVCodec *m_videoCodec = avcodec_find_encoder(m_videoCodecContext->codec_id);
    if (!m_videoCodec) {
      //q_ptr->setError(Encoder::VideoEncoderNotFoundError, tr("Unable to find video encoder by codec id."));
      qDebug() << "Unable to find video encoder by codec id.";
      return false;
    }

    // open the codec
    if (avcodec_open2(m_videoCodecContext, m_videoCodec, NULL) < 0) {
      //q_ptr->setError(Encoder::InvalidVideoCodecError, tr("Unable to open video codec."));
      qDebug() << "Unable to open video codec.";
      return false;
    }

    //allocate frame buffer
    //int m_videoBufferSize = videoSize().width() * videoSize().height() * 1.5;
    int m_videoBufferSize = m_videoCodecContext->width * m_videoCodecContext->height * 1.5;
    //AVBuffer *m_videoBuffer = new AVBuffer[m_videoBufferSize];
    AVBufferRef *m_videoBuffer = av_buffer_alloc(m_videoBufferSize);

    //init frame
    AVFrame *m_videoPicture = avcodec_alloc_frame();

    int size = avpicture_get_size(m_videoCodecContext->pix_fmt, m_videoCodecContext->width, m_videoCodecContext->height);
    //AVBuffer *m_pictureBuffer = new AVBuffer[size];
    AVBufferRef *m_pictureBuffer = av_buffer_alloc(size);

    // Setup the planes
    avpicture_fill((AVPicture *)m_videoPicture, m_pictureBuffer->data, m_videoCodecContext->pix_fmt, m_videoCodecContext->width, m_videoCodecContext->height);

    // Finalize format
    avformat_write_header(m_formatContext, 0);

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
