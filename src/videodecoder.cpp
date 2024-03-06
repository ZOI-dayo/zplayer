#include "videodecoder.hpp"
#include <iostream>

namespace video_decoder {

VideoFile::VideoFile(char* file_path) {
  format_context = nullptr;
  if (avformat_open_input(&format_context, file_path, nullptr, nullptr)) {
    throw std::runtime_error("ファイルを開けませんでした");
  }

  video_stream_index = get_first_stream(format_context, AVMEDIA_TYPE_VIDEO);
  if(video_stream_index < 0) throw std::runtime_error("動画ストリームが見つかりませんでした");

  AVStream *video_stream = format_context->streams[video_stream_index];
  codec_parameters = video_stream->codecpar;
  AVCodec const *codec = avcodec_find_decoder(codec_parameters->codec_id);
  if (codec == nullptr) {
    throw std::runtime_error("コーデックが見つかりませんでした");
  }

  codec_context = avcodec_alloc_context3(codec);
  if (avcodec_parameters_to_context(codec_context, codec_parameters)) {
    throw std::runtime_error("コーデックパラメータのコンテキストへの複製に失敗しました");
  }
  if(avcodec_open2(codec_context, codec, nullptr)) {
    throw std::runtime_error("コーデックのオープンに失敗しました");
  }
}

VideoFile::~VideoFile() {
  avformat_close_input(&format_context);
}

int VideoFile::get_first_stream(AVFormatContext *format_context, AVMediaType type) {
  for (int i = 0; i < format_context->nb_streams; i++) {
    if (format_context->streams[i]->codecpar->codec_type == type) {
      return i;
    }
  }
  throw std::runtime_error("ストリームが見つかりませんでした");
}

int VideoFile::width() {
  return codec_parameters->width;
}
int VideoFile::height() {
  return codec_parameters->height;
}

void VideoFile::decode(ThreadsafeQueue<AVFrame*> *frame_queue, int max_frames) {
  int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width(), height(), 1);
  uint8_t *buffer = (uint8_t *)av_malloc(num_bytes * sizeof(uint8_t));
  AVPacket *packet = av_packet_alloc();
  if(packet == nullptr) {
    std::cerr << "パケットの作成に失敗しました" << std::endl;
    return;
  }
  SwsContext *sws_context = nullptr;
  AVFrame *raw_frame = av_frame_alloc();
  while(true) {
    while(true) {
      if(av_read_frame(format_context, packet) != 0) {
        // std::cerr << "パケットの作成に失敗しました" << std::endl;
        continue;
      }
      if(packet->stream_index != video_stream_index) {
        // std::cerr << "ビデオストリームのパケットが見つかりませんでした" << std::endl;
        continue;
      }
      if(avcodec_send_packet(codec_context, packet) != 0) {
        // std::cerr << "パケットの送信に失敗しました" << std::endl;
        continue;
      }

      if(avcodec_receive_frame(codec_context, raw_frame)) {
        // std::cerr << "フレームの取得に失敗しました" << std::endl;
        continue;
      }
      sws_context = sws_getContext(width(), height(), codec_context->pix_fmt,
                                   width(), height(), AV_PIX_FMT_RGB24,
                                   SWS_BILINEAR, NULL, NULL, NULL);
      if (sws_context == nullptr) {
        // std::cout << "SWSコンテキストの作成に失敗しました" << std::endl;
        continue;
      }
      break;
    }

    AVFrame *frame = av_frame_alloc();

    av_image_fill_arrays(frame->data, frame->linesize, buffer, AV_PIX_FMT_RGB24, width(), height(), 1);

    sws_scale(sws_context, raw_frame->data, raw_frame->linesize, 0, height(), frame->data, frame->linesize);

    AVRational time_base = format_context->streams[video_stream_index]->time_base;
    av_dict_set(&frame->metadata, "timestamp", std::to_string((double)raw_frame->pts * time_base.num / time_base.den).c_str(), 0);

    frame_queue->push(frame);

    while(frame_queue->size() > max_frames) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  // これ一生実行されないけどどうする
  av_packet_free(&packet);
  av_frame_free(&raw_frame);
  av_free(sws_context);
  av_free(buffer);
}

/*
void VideoFile::seek(int64_t timestamp) {
  // av_seek_frame(format_context, video_stream_index, timestamp, AVSEEK_FLAG_BACKWARD);
  avformat_seek_file(format_context, video_stream_index, timestamp, timestamp, timestamp, AVSEEK_FLAG_BACKWARD);
}
*/

VideoDecoder::VideoDecoder(char* file_path) {
  video_file = new VideoFile(file_path);
  frame_queue = new ThreadsafeQueue<AVFrame*>();
  decode_thread = std::thread([this] {
    video_file->decode(frame_queue, 100);
  });
}

VideoDecoder::~VideoDecoder() {
  delete video_file;
  delete frame_queue;
  decode_thread.join();
}

}

