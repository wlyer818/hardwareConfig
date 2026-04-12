#ifndef PICPROCESS_H
#define PICPROCESS_H
#include <opencv2/opencv.hpp>

struct LineFitResult
{
    double maxDeviation;//最大偏差（最大翘曲）
    double rmsDeviation;//RMS偏差（平整度均方根）
    cv::Vec4f lineParam;
};
struct EdgePair
{
    int col;
    int top;
    int bottom;
};
#include <opencv2/opencv.hpp>

//图像处理工具封装类
class PicProcessor
{
public:
    static void GrabCutSeg(const cv::Mat& image, cv::Mat& outputMask,const cv::Rect rect);
    static void GrabCutSegOri(const cv::Mat& image, cv::Mat& outputMask,const cv::Rect rect,std::string BackColor);
    static void resizeKeep(const cv::Mat& input, cv::Mat& output, int w, int h);
    static cv::Mat StitchImages(const cv::Mat& img1, const cv::Mat& img2);
    static void FillHoles(const cv::Mat &src,cv::Mat &dst);
    //边缘提取
    static void edgeExtraction(const cv::Mat &src,
                               cv::Mat& binary,
                               std::vector<cv::Point> &topEdge,
                               std::vector<cv::Point> &bottomEdge);
    static void edgeExtraction_old(const cv::Mat &src,
                               cv::Mat& binary,
                               std::vector<cv::Point> &topEdge,
                               std::vector<cv::Point> &bottomEdge);
    static void edgeExtraction_stable(const cv::Mat &src,
                                   cv::Mat& binary,
                                   std::vector<cv::Point> &topEdge,
                                   std::vector<cv::Point> &bottomEdge);
    //TODO:增加使用canny边缘检测的版本
    static void filterEdgePairsToImage(const cv::Mat& src,
                                       const std::vector<EdgePair>& rawPairs,
                                       cv::Mat& binary);
    static void cannyEdgeExtraction(const cv::Mat& src,
                                        cv::Mat& binary,
                                        std::vector<EdgePair>& rawPairs);                             
    static LineFitResult evaluateStraightness(
                            const std::vector<cv::Point> &topEdge,
                            const std::vector<cv::Point> &bottomEdge);
    static double median(std::vector<int>& v);
    static double mad(const std::vector<int>& v,double med);
    //2级异常点对滤波
    static std::vector<EdgePair> filterEdgePairs(
        const std::vector<EdgePair>& input);
    //Savitzky-Golay平滑
    static std::vector<double> savitzkyGolaySmooth(const std::vector<double>& y);
    //3级异常点对滤波（更严格）
    static std::vector<EdgePair> filterEdgePairsPlus3(
    const std::vector<EdgePair>& input);
    //RGB转灰度
    static void RGB2Gray(const cv::Mat& image, cv::Mat& grayImage);
    //滤波算法
    static void applyGaussianBlur(const cv::Mat& src, cv::Mat& dst, int kernelSize);
    static void applyMedianBlur(const cv::Mat& src, cv::Mat& dst, int kernelSize);
    static void applyBilateralFilter(const cv::Mat& src, cv::Mat& dst, int diameter, double sigmaColor, double sigmaSpace);
    static cv::Mat performPCA(const cv::Mat& data, int k) ;
    static cv::Mat computeLBP(const cv::Mat& src) ;
    static void GrabCutSegImproved(const cv::Mat& image, cv::Mat& outputMask, const cv::Rect rect, std::string BackColor);
    static cv::Mat roatateImage(const cv::Mat &img, int angle);
public:
    cv::Mat m_frame1;
    cv::Mat m_frame2;
    cv::Mat m_fullframe;
    cv::Mat m_GrabCutMask;
};
#endif // PICPROCESS_H