#include "videodecoder.hpp"
#include <iostream>

int get_first_video_stream(AVFormatContext *format_context) {
  for (int i = 0; i < format_context->nb_streams; i++) {
    if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      return i;
    }
  }
  std::cerr << "動画ストリームが見つかりませんでした" << std::endl;
  return -1;
}
