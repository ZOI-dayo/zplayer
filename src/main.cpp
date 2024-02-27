extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}
#include <iostream>

int main(int argc, char *argv[]) {
  const char *input_path = argv[1];
  AVFormatContext *format_context = nullptr;
  if (avformat_open_input(&format_context, input_path, nullptr, nullptr) != 0) {
    std::cerr << "ファイルを開けませんでした" << std::endl;
    return 1;
  }
  std::cout << format_context->iformat->name << std::endl;
}
