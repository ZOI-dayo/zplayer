#include <GLFW/glfw3.h>
extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}
#include <iostream>

#include "videodecoder.hpp"

int main(int argc, char *argv[]) {
  const char *input_path = argv[1];
  if (input_path == nullptr) {
    std::cerr << "入力ファイルを指定してください" << std::endl;
    return 1;
  }
  std::cout << "ファイルを開いています..." << std::endl;

  AVFormatContext *format_context = nullptr;
  if (avformat_open_input(&format_context, input_path, nullptr, nullptr) != 0) {
    std::cerr << "ファイルを開けませんでした" << std::endl;
    return 1;
  }

  int video_stream_index = get_first_video_stream(format_context);
  if(video_stream_index < 0) return 1;
  AVStream *video_stream = format_context->streams[video_stream_index];
  AVCodecParameters *codec_parameters = video_stream->codecpar;
  AVCodec const *codec = avcodec_find_decoder(codec_parameters->codec_id);
  if (codec == nullptr) {
    std::cerr << "コーデックが見つかりませんでした" << std::endl;
    return 1;
  }

  AVCodecContext *codec_context = avcodec_alloc_context3(codec);
  if (avcodec_parameters_to_context(codec_context, codec_parameters) < 0) {
    return 1;
  }
  // codec_context->framerate = video_stream->r_frame_rate;
  // codec_context->time_base = video_stream->time_base;
  if(avcodec_open2(codec_context, codec, nullptr) < 0) {
    return 1;
  }
  // codec_context->width = codec_parameters->width;
  // codec_context->height = codec_parameters->height;

  // video_stream->sample_aspect_ratio = codec_context->sample_aspect_ratio;

  AVPacket packet;
  av_init_packet(&packet);
  while(true) {
    if(av_read_frame(format_context, &packet) != 0) {
      std::cerr << "パケットの作成に失敗しました" << std::endl;
      // return 1;
      continue;
    }
    if(packet.stream_index != video_stream_index) {
      std::cerr << "ビデオストリームのパケットが見つかりませんでした" << std::endl;
      // return 1;
      continue;
    }
    if(avcodec_send_packet(codec_context, &packet) != 0) {
      std::cerr << "パケットの送信に失敗しました" << std::endl;
      continue;
    }
    av_packet_unref(&packet);


    AVFrame *frame = av_frame_alloc();
    while(avcodec_receive_frame(codec_context, frame)==0) {
      // on frame decoded
      int width = codec_parameters->width;
      int height = codec_parameters->height;
      std::cout << width << std::endl;
      std::cout << frame->width << std::endl;

      struct SwsContext *sws_context = sws_getContext(width, height, codec_context->pix_fmt,
                                                      width, height, AV_PIX_FMT_RGB24,
                                                      SWS_BILINEAR, NULL, NULL, NULL);
      if (sws_context == nullptr) {
        std::cerr << "SWSコンテキストの作成に失敗しました" << std::endl;
        return 1;
      }
      std::cout << "SWSコンテキストの作成に成功" << std::endl;


      AVFrame *rgb_frame = av_frame_alloc();
      int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1);
      uint8_t *buffer = (uint8_t *)av_malloc(num_bytes * sizeof(uint8_t));
      av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, buffer, AV_PIX_FMT_RGB24, width, height, 1);

      sws_scale(sws_context, frame->data, frame->linesize, 0, height, rgb_frame->data, rgb_frame->linesize);


      // --- OpenGL Start ---

      if(glfwInit() == GL_FALSE) {
        std::cerr << "GLFWの初期化に失敗しました" << std::endl;
        return 1;
      }
      atexit(glfwTerminate);
      GLFWwindow * const window = glfwCreateWindow(640, 480, "Hello World!", NULL, NULL);
      if(window == NULL) {
        std::cerr << "ウィンドウの作成に失敗しました" << std::endl;
        return 1;
      }
      glfwMakeContextCurrent(window);
      glClearColor(.5f,.5f,.0f,.0f);

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      GLuint texture;
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_frame->data[0]);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);
      av_frame_free(&rgb_frame);

      glLoadIdentity();
      while(glfwWindowShouldClose(window) == GL_FALSE){

        glClear(GL_COLOR_BUFFER_BIT);
        // glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, rgb_frame->data[0]);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        glTexCoord2d(0.0, 1.0);
        glVertex2f( -1.0f, -1.0f);
        glTexCoord2d(1.0, 1.0);
        glVertex2f( 1.0f, -1.0f);
        glTexCoord2d(1.0, 0.0);
        glVertex2f( 1.0f, 1.0f);
        glTexCoord2d(0.0, 0.0);
        glVertex2f( -1.0f, 1.0f);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        // glBindTexture(GL_TEXTURE_2D, 0);

        glfwSwapBuffers(window);
        glfwWaitEvents();
      }
      return 0;
      // --- OpenGL End ---
      av_free(buffer);
    }
  }
    /*
  avformat_close_input(&format_context);
  av_frame_free(&frame);
  // av_frame_free(&rgb_frame);
  av_free(buffer);
  avcodec_free_context(&codec_context);
  sws_freeContext(sws_context);
    */

}
