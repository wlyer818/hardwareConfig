#include <cameraCapture.h>
#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <yoloonnxdetector.h>

using namespace std;
using namespace cv;

int testGrabFrame(){
    DaHengCamera camera;
    camera.open("GCA25100011"); // 替换为实际的相机序列号
    cv::Mat frame;
    camera.grabFrame(frame);
    camera.resizeKeep(frame, frame, 800, 600); // 可选：调整图像大小以适合显示窗口
    if (!frame.empty()) {
        cv::imshow("DaHeng Camera Frame", frame);
        cv::waitKey(0); // 等待按键
    } else {
        cout << "Failed to grab frame from camera." << endl;
    }
    return 0;
}
int testVideoDisplay(){
    DaHengCamera camera;
    camera.open("GCA25100011"); // 替换为实际的相机序列号
    camera.videoDisplay();
    return 0;
}
// 目标检测测试Video
void testDetectVideo(){

    DaHengCamera camera;
    camera.open("GCA25100011"); // 替换为实际的相机序列号
    // 创建检测器实例
    YoloOnnxDetector detector("D:\\Users\\LeyanWang\\Desktop\\git_test\\checkpoints\\rgbpicdetect.onnx");
    while (camera.opened())
    {

        cv::Mat frame;
        camera.grabFrame(frame);
        if(frame.empty()) {
            cout << "Failed to grab frame from camera." << endl;
            return;
        }
        // 进行目标检测
        if(detector.setImage(frame)==0){
            cout << "Failed to set image for detection." << endl;
            return;
        }
        detector.infer(0.3f, 0.45f, 0.5f, cv::COLOR_BGR2RGB);
        detector.drawResults(frame);
        // 显示检测结果
        cv::imshow("Detection Result", frame);
    }

}
cv::Mat rotateImage(const cv::Mat& img, int angle)
{
    // 获取图像中心点
    cv::Point2f center(img.cols / 2.0F, img.rows / 2.0F);

    // 获取旋转矩阵
    cv::Mat rotMat = cv::getRotationMatrix2D(center, angle, 1.0);

    // 计算旋转后图像的边界大小，防止裁剪
    double abs_cos = std::abs(rotMat.at<double>(0, 0));
    double abs_sin = std::abs(rotMat.at<double>(0, 1));
    int bound_w = int(img.rows * abs_sin + img.cols * abs_cos);
    int bound_h = int(img.rows * abs_cos + img.cols * abs_sin);

    // 调整旋转矩阵的平移量，使整个图像显示完整
    rotMat.at<double>(0, 2) += bound_w / 2.0 - center.x;
    rotMat.at<double>(1, 2) += bound_h / 2.0 - center.y;

    // 旋转图像
    cv::Mat rotated;
    cv::warpAffine(img, rotated, rotMat, cv::Size(bound_w, bound_h));

    return rotated;
}

void stitchTest(){
    cv::Mat img1 = imread("D:\\Users\\LeyanWang\\Desktop\\git_test\\img1.png");
    cv::Mat img2 = imread("D:\\Users\\LeyanWang\\Desktop\\git_test\\img2.png");
    // cv::resize(img1,img1,Size(0,0),0.5,0.5);
    // cv::resize(img2,img2,Size(0,0),0.5,0.5);
        // 创建 Stitcher（PANORAMA模式是默认，也最适合左右拼接）
    cv::Ptr<cv::Stitcher> stitcher = cv::Stitcher::create(cv::Stitcher::SCANS);
    // 输入图像列表
    std::vector<cv::Mat> imgs = { img1, img2 };
    // 输出结果
    cv::Mat res;
    cv::Stitcher::Status status = stitcher->stitch(imgs, res);

    if (status != cv::Stitcher::OK)
    {
        std::cout << "stitch error num = " << int(status) << std::endl;
        return ;
    }
    if(res.empty()){
        std::cout<<"stitch error"<<std::endl;
        return ;
    }
    else
        cv::imwrite("D:\\Users\\LeyanWang\\Desktop\\git_test\\src\\camera\\file\\res.png",res);
    return ;
}
void raoteTest(){
    cv::Mat img = imread(
        "D:\\Users\\LeyanWang\\Desktop\\git_test\\src\\python_test\\images\\DR_2026-03-27_12-00-23_0002_first.png");
    cv::Mat test = rotateImage(img,-90);
    RGBPicProcessor::resizeKeep(test,test,800,800);
    cv::imshow("test",test);
    cv::waitKey(0);

}
int main(){
    // stitchTest();
    raoteTest();
    return 0;
}