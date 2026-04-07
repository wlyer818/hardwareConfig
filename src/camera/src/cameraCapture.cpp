#include "cameraCapture.h"
#include "GalaxyIncludes.h"
DaHengCamera::DaHengCamera() : m_isOpened(false)
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
    IGXFactory::GetInstance().Init();
}

DaHengCamera::~DaHengCamera()
{
    close();
    IGXFactory::GetInstance().Uninit();
}

bool DaHengCamera::open(const std::string& sn)
{
    gxdeviceinfo_vector vec;
    IGXFactory::GetInstance().UpdateDeviceList(1000, vec);
    if (vec.empty())
        return false;

    for (auto& dev : vec)
    {
        if (dev.GetSN().c_str() == sn)
        {
            m_dev = IGXFactory::GetInstance().OpenDeviceBySN(
                dev.GetSN(), GX_ACCESS_EXCLUSIVE);

            m_stream = m_dev->OpenStream(0);
            initGigEOptimize();

            m_stream->StartGrab();
            m_dev->GetRemoteFeatureControl()
                ->GetCommandFeature("AcquisitionStart")->Execute();

            m_isOpened = true;
            return true;
        }
    }
    return false; // 没找到SN
}


void DaHengCamera::close()
{
    if (!m_isOpened) return;

    m_dev->GetRemoteFeatureControl()
        ->GetCommandFeature("AcquisitionStop")->Execute();
    m_stream->StopGrab();
    m_stream->Close();
    m_dev->Close();

    m_isOpened = false;
}

bool DaHengCamera::initGigEOptimize()
{
    if (m_dev->GetDeviceInfo().GetDeviceClass() == GX_DEVICE_CLASS_GEV)
    {
        auto pCtl = m_dev->GetRemoteFeatureControl();
        if (pCtl->IsImplemented("GevSCPSPacketSize"))
        {
            int pkt = m_stream->GetOptimalPacketSize();
            pCtl->GetIntFeature("GevSCPSPacketSize")->SetValue(pkt);
            return true;
        }
    }
    return false;
}

bool DaHengCamera::grabFrame(cv::Mat& out)
{
    if (!m_isOpened) return false;

    CImageDataPointer img = m_stream->GetImage(100);
    if (img == nullptr || img->GetStatus() != GX_FRAME_STATUS_SUCCESS)
        return false;

    void* pRgb = img->ConvertToRGB24(GX_BIT_0_7, GX_RAW2RGB_NEIGHBOUR, false);
    if (!pRgb) return false;

    cv::Mat frame(img->GetHeight(), img->GetWidth(), CV_8UC3, pRgb);

    // ⭐ 关键：RGB -> BGR
    cv::cvtColor(frame, out, cv::COLOR_RGB2BGR);

    return true;
}

void DaHengCamera::videoDisplay()
{
    if (!m_isOpened) {
        if (!open("GCB20090044")) throw std::runtime_error("无法打开大恒相机");
    }

    cv::namedWindow("DaHeng", cv::WINDOW_AUTOSIZE);
    cv::Mat frame;

    while (cv::waitKey(1) != 27)
    {
        if (grabFrame(frame))
        {
            resizeKeep(frame, frame, 800, 600);
            cv::imshow("DaHeng", frame);
        }
    }
}

/*********************************
 * 工具函数：图像拼接
 ********************************/
cv::Mat RGBPicProcessor::StitchImages(const cv::Mat& img1, const cv::Mat& img2)
{
    if(img1.empty() || img2.empty())
    {
        std::cout << "输入图像为空" << std::endl;
        return cv::Mat();
    }
    if(img1.size() != img2.size())
    {
        std::cout << "输入图像尺寸不一致" << std::endl;
        return cv::Mat();
    }
    if(img1.type() != img2.type())
    {
        std::cout << "输入图像类型不一致" << std::endl;
        return cv::Mat();
    }
    if(img1.channels() != 3)
    {
        std::cout << "输入图像通道数不为3" << std::endl;
        return cv::Mat();
    }
    if(img1.cols < 100 || img1.rows < 100)
    {
        std::cout << "输入图像尺寸过小" << std::endl;
        return cv::Mat();
    }
    if(img1.cols > 10000 || img1.rows > 10000)
    {
        std::cout << "输入图像尺寸过大" << std::endl;
        return cv::Mat();
    }
    if(img1.cols != img2.cols || img1.rows != img2.rows)
    {
        std::cout << "输入图像尺寸不一致" << std::endl;
        return cv::Mat();
    }
    if(img1.type() != img2.type())
    {
        std::cout << "输入图像类型不一致" << std::endl;
        return cv::Mat();
    }
    if(img1.channels() != 3)
    {
        std::cout << "输入图像通道数不为3" << std::endl;
        return cv::Mat();
    }
    if(img1.cols < 100 || img1.rows < 100)
    {
        std::cout << "输入图像尺寸过小" << std::endl;
        return cv::Mat();
    }
    if(img1.cols > 10000 || img1.rows > 10000)
    {
        std::cout << "输入图像尺寸过大" << std::endl;
        return cv::Mat();
    }
    if(img1.cols != img2.cols || img1.rows != img2.rows)
    {
        std::cout << "输入图像尺寸不一致" << std::endl;
        return cv::Mat();
    }
    // 创建 Stitcher（PANORAMA模式是默认，也最适合左右拼接）
    cv::Ptr<cv::Stitcher> stitcher = cv::Stitcher::create(cv::Stitcher::PANORAMA);

    // 输入图像列表
    std::vector<cv::Mat> imgs = { img1, img2 };

    // 输出结果
    cv::Mat pano;
    cv::Stitcher::Status status = stitcher->stitch(imgs, pano);

    if (status != cv::Stitcher::OK)
    {
        std::cout << "拼接失败，错误码 = " << int(status) << std::endl;
        return cv::Mat();
    }

    return pano; // 返回拼接结果
}

/*********************************
 * GrabCut
 *********************************/
void RGBPicProcessor::GrabCutSeg(const cv::Mat& image, cv::Mat& outputMask)
{
    cv::Mat mask(image.size(), CV_8UC1, cv::GC_BGD);
    cv::Mat bgModel, fgModel;

    cv::Rect rect(0, 240, image.cols - 0, image.rows - 390);

    cv::grabCut(image, mask, rect, bgModel, fgModel, 5, cv::GC_INIT_WITH_RECT);

    cv::Mat whiteBg(image.size(), image.type(), cv::Scalar(255,255,255));
    cv::Mat binMask = (mask == cv::GC_FGD) | (mask == cv::GC_PR_FGD);

    image.copyTo(whiteBg, binMask);
    outputMask = whiteBg;
}

/*********************************
 * 等比缩放
 *********************************/
void DaHengCamera::resizeKeep(const cv::Mat& input, cv::Mat& output, int w, int h)
{
    using namespace std;
    double sx = (double)w / input.cols;
    double sy = (double)h / input.rows;
    double s = min(sx, sy);

    cv::resize(input, output, cv::Size(input.cols * s, input.rows * s), 0, 0, cv::INTER_AREA);
}
void RGBPicProcessor::resizeKeep(const cv::Mat& input, cv::Mat& output, int w, int h)
{
    using namespace std;
    double sx = (double)w / input.cols;
    double sy = (double)h / input.rows;
    double s = min(sx, sy);

    cv::resize(input, output, cv::Size(input.cols * s, input.rows * s), 0, 0, cv::INTER_AREA);
}
void RGBPicProcessor::cameraInit()
{
    // 初始化相机
    camera1.open("GCB20090044");
    // camera2.open();
}

void RGBPicProcessor::captureFrame1()
{
    // 捕获帧
    camera1.grabFrame(m_frame1);

}
void RGBPicProcessor::captureFrame2()
{
    // 捕获帧
    camera2.grabFrame(m_frame2);

}