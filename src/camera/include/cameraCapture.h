#ifndef CAMERACAPTURE_H
#define CAMERACAPTURE_H
#include <opencv2/opencv.hpp>
#include "GalaxyIncludes.h"
class DaHengCamera
{
public:
    DaHengCamera();
    ~DaHengCamera();

    bool open(const std::string& sn);// 打开相机，指定序列号
    void close();               // 关闭相机

    bool grabFrame(cv::Mat& out);   // 单张抓取（含转换)
    void videoDisplay();            // 连续采集并显示
    void resizeKeep(const cv::Mat& input, cv::Mat& output, int w, int h);
    bool opened() const { return m_isOpened; } 
private:
    bool initGigEOptimize();    // 千兆网优化

private:
    CGXDevicePointer m_dev;
    CGXStreamPointer m_stream;
    bool m_isOpened = false;
};

class RGBPicProcessor
{
public:
    static void GrabCutSeg(const cv::Mat& image, cv::Mat& outputMask);
    static void resizeKeep(const cv::Mat& input, cv::Mat& output, int w, int h);
    static cv::Mat StitchImages(const cv::Mat& img1, const cv::Mat& img2);
    void cameraInit();
    void captureFrame1();
    void captureFrame2();
public:
    DaHengCamera camera1;
    DaHengCamera camera2;
    cv::Mat m_frame1;
    cv::Mat m_frame2;
    cv::Rect m_rect;
    cv::Mat m_fullframe;
    cv::Mat m_GrabCutMask;
};

#endif
