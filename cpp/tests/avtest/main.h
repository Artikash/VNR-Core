#ifndef MAIN_H
#define MAIN_H

// main.h
// 10/27/2014 jichi

#include <QtCore>
#include <QtGui>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
//#include <libavutil/opt.h>
#include <libswscale/swscale.h>
} // extern "C"


class Recorder : public QDialog
{
  Q_OBJECT

  QTimer *videoTimer_;
  QString path_;

  AVFormatContext *m_formatContext;
  AVStream *m_videoStream;
  AVCodecContext *m_videoCodecContext;
  AVCodec *m_videoCodec;

    AVBufferRef *m_videoBuffer;
    size_t m_videoBufferSize;
    AVFrame *m_videoPicture;
    SwsContext *m_imageConvertContext;
public:
  Recorder()
  {
    path_  = "avtest.mp4";
    videoTimer_ = new QTimer(this);
    videoTimer_->setSingleShot(false);
    //videoTimer_->setInterval(1000/24.); // 24 per second
    videoTimer_->setInterval(1000);
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

    m_formatContext = avformat_alloc_context();
    m_formatContext->oformat = m_outputFormat;

    // http://stackoverflow.com/questions/19146587/how-do-you-enable-veryfast-libx264-preset-from-code-using-libavcodec
    //av_opt_set(m_formatContext->priv_data, "preset", "veryfast", 0);

    // Create video
    //AVStream *m_videoStream = av_new_stream(m_formatContext,0);
    m_videoStream = avformat_new_stream(m_formatContext, nullptr);
    if (!m_videoStream) {
      //q_ptr->setError(Encoder::InvalidVideoStreamError, tr("Unable to add video stream."));
      qDebug() << "Unable to add video stream.";
      return false;
    }

    // Set up video codec
    m_videoCodecContext = m_videoStream->codec;

    //m_videoCodecContext->codec_id = (videoCodec() == EncoderGlobal::DEFAULT_VIDEO_CODEC) ? m_outputFormat->video_codec : static_cast<CodecID>(videoCodec());
    //m_videoCodecContext->codec_id = m_outputFormat->video_codec;
    //qDebug() << "video codec id" << m_videoCodecContext->codec_id;
    m_videoCodecContext->codec_id =  AV_CODEC_ID_H264;

    m_videoCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;

    //m_videoCodecContext->width = videoSize().width();
    //m_videoCodecContext->height = videoSize().height();
    //m_videoCodecContext->width = 1280;
    //m_videoCodecContext->height = 720;

    m_videoCodecContext->width = 720;
    m_videoCodecContext->height = 480;

    //m_videoCodecContext->pix_fmt = (PixelFormat)outputPixelFormat();
    m_videoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    //m_videoCodecContext->time_base.den = fixedFrameRate() != -1 ? fixedFrameRate() : 1000;
    m_videoStream->time_base.den = 1000;
    m_videoStream->time_base.num = 1;

    //applyVideoCodecSettings();
    // http://stackoverflow.com/questions/3553003/encoding-h-264-with-libavcodec-x264
    auto ctx = m_videoCodecContext;
    ctx->bit_rate = 5000 * 1000;
    ctx->bit_rate_tolerance = 0;
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
    m_videoCodec = avcodec_find_encoder(m_videoCodecContext->codec_id);
    if (!m_videoCodec) {
      //q_ptr->setError(Encoder::VideoEncoderNotFoundError, tr("Unable to find video encoder by codec id."));
      qDebug() << "Unable to find video encoder by codec id.";
      return false;
    }

    // open the codec
    // http://stackoverflow.com/questions/19146587/how-do-you-enable-veryfast-libx264-preset-from-code-using-libavcodec
    //AVDictionary *codec_options = nullptr;
    //av_dict_set(&codec_options, "preset", "medium", 0);
    //av_dict_set( &codec_options, "AnyCodecParameter", "Value", 0);
    //if (avcodec_open2(m_videoCodecContext, m_videoCodec, &codec_options) < 0) {
    if (avcodec_open2(m_videoCodecContext, m_videoCodec, nullptr) < 0) {
      //q_ptr->setError(Encoder::InvalidVideoCodecError, tr("Unable to open video codec."));
      qDebug() << "Unable to open video codec.";
      return false;
    }

    //allocate frame buffer
    //int m_videoBufferSize = videoSize().width() * videoSize().height() * 1.5;
    m_videoBufferSize = m_videoCodecContext->width * m_videoCodecContext->height * 1.5;
    //AVBuffer *m_videoBuffer = new AVBuffer[m_videoBufferSize];
    m_videoBuffer = av_buffer_alloc(m_videoBufferSize);

    //init frame
    m_videoPicture = avcodec_alloc_frame();

    int size = avpicture_get_size(m_videoCodecContext->pix_fmt, m_videoCodecContext->width, m_videoCodecContext->height);
    //AVBuffer *m_pictureBuffer = new AVBuffer[size];
    AVBufferRef *m_pictureBuffer = av_buffer_alloc(size);

    // Setup the planes
    avpicture_fill((AVPicture *)m_videoPicture, m_pictureBuffer->data, m_videoCodecContext->pix_fmt, m_videoCodecContext->width, m_videoCodecContext->height);

    if (avio_open(&m_formatContext->pb, path_.toUtf8().constData(), AVIO_FLAG_WRITE) < 0) {
      //q_ptr->setError(Encoder::FileOpenError, QString(tr("Unable to open: %1")).arg(filePath()));
      qDebug() << "Unable to open:" << path_;
      return false;
    }

    // Finalize format

    // See: ffmpeg/doc/examples/muxing.c
    if (m_formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        m_videoCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
    avformat_write_header(m_formatContext, 0);

    qDebug() << "success";
    // Start timing
    videoTimer_->start();
    return true;
  }

public slots:
  void stop()
  {
    av_write_trailer(m_formatContext);
    avcodec_close(m_videoCodecContext);
    av_free(m_videoCodecContext);
    av_free(m_videoStream);
    sws_freeContext(m_imageConvertContext);
    avio_close(m_formatContext->pb);
  }

private slots:
  void recordVideo()
  {
    //enum { pts = -1 }; // timestamp
    QWidget *w = qApp->desktop();
    QImage frame = QPixmap::grabWindow(w->winId()).toImage();
    if (convertImage(frame)) {
      int outSize = avcodec_encode_video(m_videoCodecContext, m_videoBuffer->data, m_videoBufferSize, m_videoPicture);
      if (outSize > 0) {
        // http://stackoverflow.com/questions/6603979/ffmpegavcodec-encode-video-setting-pts-h264
        //if (!isFixedFrameRate())
        AVPacket pkt;
        av_init_packet(&pkt);

        static int64_t pts = 0;
        pkt.pts = m_videoCodecContext->coded_frame->pts =  ++pts;

        if(m_videoCodecContext->coded_frame->key_frame)
          pkt.flags |= AV_PKT_FLAG_KEY;

        pkt.stream_index = m_videoStream->index;
        pkt.data = m_videoBuffer->data;
        pkt.size = outSize;
        av_write_frame(m_formatContext, &pkt);

        //QMutexLocker locker(&m_encodedFrameCountMutex);
        //m_encodedFrameCount++;
      }
    }
  }

  bool convertImage(const QImage &image)
  {
    AVPixelFormat inputFormat = convertImagePixelFormat(image.format());
    if (inputFormat == AV_PIX_FMT_NONE) {
      //q_ptr->setError(Encoder::InvalidInputPixelFormat, tr("Could not convert input pixel format to the ffmpeg's format."));
      qDebug() << "Could not convert input pixel format to the ffmpeg's format.";
      return false;
    }

    m_imageConvertContext = sws_getCachedContext(m_imageConvertContext, image.width(), image.height(),
                           (AVPixelFormat)inputFormat, image.width(), image.height(), m_videoCodecContext->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);

    if (m_imageConvertContext == NULL) {
      //q_ptr->setError(Encoder::InvalidConversionContext, tr("Could not initialize conversion context."));
      qDebug() << "Could not initialize conversion context.";
      return false;
    }

    uint8_t *srcplanes[3];
    srcplanes[0]=(uint8_t*)image.bits();
    srcplanes[1]=0;
    srcplanes[2]=0;

    int srcstride[3];
    srcstride[0]=image.bytesPerLine();
    srcstride[1]=0;
    srcstride[2]=0;

    sws_scale(m_imageConvertContext, srcplanes, srcstride,0, image.height(), m_videoPicture->data, m_videoPicture->linesize);

    return true;
  }

  static AVPixelFormat convertImagePixelFormat(QImage::Format format)
  {
    AVPixelFormat newFormat;

    switch (format) {
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
    case QImage::Format_RGB32:
      newFormat = AV_PIX_FMT_BGRA;
      break;

    case QImage::Format_RGB16:
      newFormat = AV_PIX_FMT_RGB565LE;
      break;

    case QImage::Format_RGB888:
      newFormat = AV_PIX_FMT_RGB24;
      break;

    case QImage::Format_Mono:
    case QImage::Format_MonoLSB:
      newFormat = AV_PIX_FMT_MONOWHITE;
      break;

    default:
      newFormat = AV_PIX_FMT_NONE;
      break;
    }

    return newFormat;
  }
};


#endif // MAIN_H
