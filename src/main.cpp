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
#include <chrono>

#include "videodecoder.hpp"

int video_width, video_height;
void draw(GLFWwindow *window) {

  int width, height;
  glfwGetWindowSize(window, &width, &height);
  float aspect = (float)width / (float)height;
  float video_aspect = (float)video_width / (float)video_height;
  // std::cout << "aspect: " << aspect << ", video_aspect: " << video_aspect << std::endl;

  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  if(aspect > video_aspect) {
    float w = 1.0f / aspect * video_aspect;
    float h = 1.0f;
    glTexCoord2d(0.0, 1.0);
    glVertex2f( -w, -h);
    glTexCoord2d(1.0, 1.0);
    glVertex2f( w, -h);
    glTexCoord2d(1.0, 0.0);
    glVertex2f( w, h);
    glTexCoord2d(0.0, 0.0);
    glVertex2f( -w, h);
  } else {
    float w = 1.0f;
    float h = 1.0f * aspect / video_aspect;
    glTexCoord2d(0.0, 1.0);
    glVertex2f( -w, -h);
    glTexCoord2d(1.0, 1.0);
    glVertex2f( w, -h);
    glTexCoord2d(1.0, 0.0);
    glVertex2f( w, h);
    glTexCoord2d(0.0, 0.0);
    glVertex2f( -w, h);
  }
  glEnd();
  glDisable(GL_TEXTURE_2D);
  glfwSwapBuffers(window);

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width and 
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
  // Re-render the scene because the current frame was drawn for the old resolution
  draw(window);
}

int main(int argc, char *argv[]) {
  char *video_path = argv[1];
  if (video_path == nullptr) {
    std::cerr << "入力ファイルを指定してください" << std::endl;
    return 1;
  }
  std::cout << "ファイルを開いています..." << std::endl;
  video_decoder::VideoFile video(video_path);

  video.seek(600000);

  AVFrame* frame = av_frame_alloc();
  video.get_frame(frame);
  video_width = video.width();
  video_height = video.height();
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
  glfwSwapInterval(1);
  glfwSetFramebufferSizeCallback(window,
                                 framebuffer_size_callback
                                 );

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, video.width(), video.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, frame->data[0]);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);


  glLoadIdentity();

  // timer start
  std::chrono::system_clock::time_point start_time;
  start_time = std::chrono::system_clock::now();

  while(glfwWindowShouldClose(window) == GL_FALSE){

    // std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    // int64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
    // video.seek(timestamp*16000);
    // video.get_frame(frame);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, video.width(), video.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, frame->data[0]);
    draw(window);
    // glfwWaitEvents();
    glfwPollEvents();


  }
  av_frame_free(&frame);
  return 0;
  // --- OpenGL End ---

}
