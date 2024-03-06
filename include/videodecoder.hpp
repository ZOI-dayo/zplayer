#pragma once

extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

#include <thread>
#include "threadsafe_queue.hpp"

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
  int width();
  int height();
  void decode(ThreadsafeQueue<AVFrame*> *frame_queue, int max_frames);
  // void seek(int64_t timestamp);
};

class VideoDecoder {
private:
  std::thread decode_thread;
public:
  ThreadsafeQueue<AVFrame*> *frame_queue;
  VideoFile *video_file;
  VideoDecoder(char* file_path);
  ~VideoDecoder();
};

}

