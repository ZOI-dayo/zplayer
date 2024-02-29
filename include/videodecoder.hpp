#pragma once

extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

namespace video_decoder {

class VideoFile {
private:
  AVFormatContext *format_context;
  int get_first_stream(AVFormatContext *format_context, AVMediaType type);
  int video_stream_index;
  AVCodecParameters *codec_parameters;
  AVCodecContext *codec_context;
public:
  VideoFile(char* file_path);
  ~VideoFile();
  void get_frame(AVFrame *frame);
  int width();
  int height();
  void seek(int64_t timestamp);
};

}

