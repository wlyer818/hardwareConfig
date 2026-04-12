#include <cameraCapture.h>
#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <yoloonnxdetector.h>
#include <memory>
#include <map>
#include <set>
#include <picprocess.h>

using namespace std;
using namespace cv;
//尽量使用返回值而不是传入参数的方式来获取结果，除非需要返回多个结果或者需要修改输入参数的值。
int testGrabFrame(){
    std::unique_ptr<DaHengCamera> camera = std::make_unique<DaHengCamera>();
    if(camera->open("GCA25100011")) { // 替换为实际的相机序列号
        cout << "Camera opened successfully." << endl;
    } else {
        cout << "Failed to open camera." << endl;
        return -1;
    }
    cv::Mat frame;
    camera->grabFrame(frame);
    frame = PicProcessor::roatateImage(frame, -90);
    if(frame.empty()) {
        cout << "Failed to grab frame from camera." << endl;
        return -1;
    }
    cv::imwrite("frame.png", frame);
    camera->resizeKeep(frame, frame, 800, 600); // 可选：调整图像大小以适合显示窗口
    if (!frame.empty()) {
        cv::imshow("DaHeng Camera Frame", frame);
        cv::waitKey(0); // 等待按键
    } else {
        cout << "Failed to grab frame from camera." << endl;
    }
    return 0;
}
int testVideoDisplay(){
    std::shared_ptr<DaHengCamera> camera = std::make_shared<DaHengCamera>();
    camera->open("GCA25100011"); // 替换为实际的相机序列号
    camera->videoDisplay();
    return 0;
}
// 目标检测测试Video
void testDetectVideo(){

    DaHengCamera camera;
    camera.open("GCA25100011"); // 替换为实际的相机序列号
    // 创建检测器实例
    YoloOnnxDetector detector("D:\\Users\\LeyanWang\\Desktop\\git_test\\checkpoints\\rgbpicdetect.onnx");
    {

        cv::Mat frame;
        camera.grabFrame(frame);
        frame = PicProcessor::roatateImage(frame, -90);
        PicProcessor::resizeKeep(frame, frame, 800, 600); // 可选：调整图像大小以适合显示窗口
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
        cv::waitKey(0); // 等待按键
    }

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

int main(){
    // testGrabFrame();
    // testVideoDisplay();
    testDetectVideo();
    return 0;
}