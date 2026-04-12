#include "picprocess.h"
/*********************************
 * 工具函数：图像拼接
 ********************************/
cv::Mat PicProcessor::StitchImages(const cv::Mat& img1, const cv::Mat& img2)
{
    if(img1.empty() || img2.empty())
    {
        std::cout << "[Stitch]:image is empty" << std::endl;
        return cv::Mat();
    }
    if(img1.size() != img2.size())
    {
        std::cout << "[Stitch]:image size if not same" << std::endl;
        return cv::Mat();
    }
    if(img1.type() != img2.type())
    {
        std::cout << "[Stitch]:The input image types are inconsistent" << std::endl;
        return cv::Mat();
    }
    if(img1.channels() != 3)
    {
        std::cout << "[Stitch]:The input image channels are not 3" << std::endl;
        return cv::Mat();
    }
    if(img1.cols < 100 || img1.rows < 100)
    {
        std::cout << "[Stitch]:The input image size is too small" << std::endl;
        return cv::Mat();
    }
    if(img1.cols > 10000 || img1.rows > 10000)
    {
        std::cout << "[Stitch]:The input image size is too large" << std::endl;
        return cv::Mat();
    }
    if(img1.cols != img2.cols || img1.rows != img2.rows)
    {
        std::cout << "[Stitch]:The input image sizes are not the same" << std::endl;
        return cv::Mat();
    }
    if(img1.type() != img2.type())
    {
        std::cout << "[Stitch]:The input image types are inconsistent" << std::endl;
        return cv::Mat();
    }
    if(img1.channels() != 3)
    {
        std::cout << "[Stitch]:The input image channels are not 3" << std::endl;
        return cv::Mat();
    }
    if(img1.cols < 100 || img1.rows < 100)
    {
        std::cout << "[Stitch]:The input image size is too small" << std::endl;
        return cv::Mat();
    }
    if(img1.cols > 10000 || img1.rows > 10000)
    {
        std::cout << "[Stitch]:The input image size is too large" << std::endl;
        return cv::Mat();
    }
    if(img1.cols != img2.cols || img1.rows != img2.rows)
    {
        std::cout << "[Stitch]:The input image sizes are not the same" << std::endl;
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
 * GrabCut：前景分割
 *********************************/
void PicProcessor::GrabCutSeg(const cv::Mat& image, cv::Mat& outputMask,const cv::Rect rect)
{
    //固定初始化
    cv::theRNG().state = 0x12345678;
    // Step 1: GrabCut 初始 mask
    cv::Mat mask(image.size(), CV_8UC1, cv::GC_BGD);
    cv::Mat bgModel, fgModel;


    cv::grabCut(image, mask, rect, bgModel, fgModel, 5, cv::GC_INIT_WITH_RECT);

    // Step 2: 生成二值 mask（前景=白 255，背景=黑 0）
    cv::Mat binMask(image.size(), CV_8UC1);

    // GrabCut foreground mask: GC_FGD = 1, GC_PR_FGD = 3
    binMask = (mask == cv::GC_FGD) | (mask == cv::GC_PR_FGD);

    // Convert 0/1 → 0/255
    binMask = binMask * 255;
    //填充内部黑洞
    FillHoles(binMask,binMask);
    outputMask = binMask;
}
/*********************************
 * GrabCut：前景分割(背景可选黑或白)
 *********************************/
void PicProcessor::GrabCutSegOri(const cv::Mat& image, cv::Mat& outputMask,const cv::Rect rect,std::string BackColor)
{
    // 固定随机种子
    cv::theRNG().state = 0x12345678;

    // Step1：初始化 mask
    cv::Mat mask(image.size(), CV_8UC1, cv::GC_BGD);
    cv::Mat bgModel, fgModel;

    // GrabCut 分割
    cv::grabCut(image, mask, rect, bgModel, fgModel, 5, cv::GC_INIT_WITH_RECT);

    // Step2：生成前景二值mask
    cv::Mat binMask;
    binMask = (mask == cv::GC_FGD) | (mask == cv::GC_PR_FGD);

    // 0/1 → 0/255
    binMask.convertTo(binMask, CV_8UC1, 255);

    // 填充内部空洞
    FillHoles(binMask, binMask);

    if(BackColor == "black")
    {
        // Step3：创建黑色背景图
        outputMask = cv::Mat(image.size(), image.type(), cv::Scalar(0,0,0));
    }
    else if(BackColor == "white")
    {
        // Step3：创建白色背景图
        outputMask = cv::Mat(image.size(), image.type(), cv::Scalar(255,255,255));
    }

    // Step4：拷贝前景到白色背景
    image.copyTo(outputMask, binMask);
}

//形态学洪水填充限定边界
void PicProcessor::FillHoles(const cv::Mat &src, cv::Mat &dst)
{
    CV_Assert(src.channels() == 1);

    // Step 1: 准备图像
    cv::Mat temp;
    src.copyTo(temp);

    // Step 2：floodFill 但只填内部洞
    // 背景外部可能在图四周，这里从边界扫描
    cv::Mat backgroundMask = cv::Mat::zeros(src.size(), CV_8U);

    // 扫描四条边框 floodFill
    for(int x = 0; x < src.cols; x++)
    {
        if (temp.at<uchar>(0, x) == 0)
            cv::floodFill(temp, cv::Point(x, 0), 128);

        if (temp.at<uchar>(src.rows-1, x) == 0)
            cv::floodFill(temp, cv::Point(x, src.rows-1), 128);
    }

    for(int y = 0; y < src.rows; y++)
    {
        if (temp.at<uchar>(y, 0) == 0)
            cv::floodFill(temp, cv::Point(y, 0), 128);

        if (temp.at<uchar>(y, src.cols-1) == 0)
            cv::floodFill(temp, cv::Point(y, src.cols-1), 128);
    }

    // Step 3：temp 中 =128 的区域是真背景
    // temp 中 =0 的点是洞 → 填白
    dst = src.clone();
    dst.setTo(255, temp == 0);
}
//边缘提取函数（无滤波）
void PicProcessor::edgeExtraction_old(const cv::Mat& src,
                                     cv::Mat& binary,
                                     std::vector<cv::Point>& topEdge,
                                     std::vector<cv::Point>& bottomEdge)
{
    CV_Assert(!src.empty());
    CV_Assert(src.type() == CV_8UC1);
    binary = cv::Mat::zeros(src.size(), CV_8UC1);
    binary.setTo(0);
    topEdge.clear();
    bottomEdge.clear();

    for (int col = 0; col < src.cols; col++)
    {
        int topRow = -1;
        int bottomRow = -1;

        // 查找上边界
        for (int row = 0; row < src.rows; row++)
        {
            if (src.at<uchar>(row, col) > 200)
            {
                topRow = row;
                break;
            }
        }

        // 查找下边界
        for (int row = src.rows - 1; row >= 0; row--)
        {
            if (src.at<uchar>(row, col) > 200)
            {
                bottomRow = row;
                break;
            }
        }

        // ⭐ 满足 “成对 + row 合法” 才记录
        if (topRow > 0 && bottomRow > 0)
        {
            binary.at<uchar>(topRow, col) = 255;
            binary.at<uchar>(bottomRow, col) = 255;

            topEdge.emplace_back(col, topRow);
            bottomEdge.emplace_back(col, bottomRow);
        }

        // 否则：跳过本列，直接忽略
    }
}
void PicProcessor::edgeExtraction_stable(
    const cv::Mat& src,
    cv::Mat& binary,
    std::vector<cv::Point>& topEdge,
    std::vector<cv::Point>& bottomEdge)
{
    CV_Assert(!src.empty());
    CV_Assert(src.type() == CV_8UC1);

    binary = cv::Mat::zeros(src.size(), CV_8UC1);
    topEdge.clear();
    bottomEdge.clear();

    const int rows = src.rows;
    const int cols = src.cols;
    const uchar thresh = 200;

    // ==================== 逐列 scanline ====================
    for (int col = 0; col < cols; ++col)
    {
        int topRow = -1;
        int bottomRow = -1;

        // ---------- 找 top ----------
        for (int row = 0; row < rows; ++row)
        {
            if (src.at<uchar>(row, col) > thresh)
            {
                topRow = row;
                break;
            }
        }

        // ---------- 找 bottom ----------
        for (int row = rows - 1; row >= 0; --row)
        {
            if (src.at<uchar>(row, col) > thresh)
            {
                bottomRow = row;
                break;
            }
        }

        // ---------- 只要合法就记录（不整列跳过） ----------
        if (topRow >= 0)
        {
            topEdge.emplace_back(col, topRow);
            binary.at<uchar>(topRow, col) = 255;
        }

        if (bottomRow >= 0)
        {
            bottomEdge.emplace_back(col, bottomRow);
            binary.at<uchar>(bottomRow, col) = 255;
        }
    }
}

//边缘提取函数+滤波
void PicProcessor::edgeExtraction(
    const cv::Mat& src,
    cv::Mat& binary,
    std::vector<cv::Point>& topEdge,
    std::vector<cv::Point>& bottomEdge)
{
    CV_Assert(!src.empty());
    CV_Assert(src.type() == CV_8UC1);

    binary = cv::Mat::zeros(src.size(), CV_8UC1);
    //     binary.setTo(0);
    topEdge.clear();
    bottomEdge.clear();

    std::vector<EdgePair> rawPairs;
    rawPairs.reserve(src.cols);

    // ===== 逐列提取原始点对 =====
    for (int col = 0; col < src.cols; col++)
    {
        int topRow = -1;
        int bottomRow = -1;

        for (int row = 0; row < src.rows; row++)
        {
            if (src.at<uchar>(row, col) > 200)
            {
                topRow = row;
                break;
            }
        }

        for (int row = src.rows - 1; row >= 0; row--)
        {
            if (src.at<uchar>(row, col) > 200)
            {
                bottomRow = row;
                break;
            }
        }

        if (topRow >= 0 && bottomRow >= 0 && bottomRow > topRow)
        {
            rawPairs.push_back({col, topRow, bottomRow});
        }
    }

    // ===== 点对级滤波 =====
    std::vector<EdgePair> filteredPairs = filterEdgePairs(rawPairs);

    // ===== 输出 =====
    for (auto& p : filteredPairs)
    {
        binary.at<uchar>(p.top, p.col) = 255;
        binary.at<uchar>(p.bottom, p.col) = 255;

        topEdge.emplace_back(p.col, p.top);
        bottomEdge.emplace_back(p.col, p.bottom);
    }

}

//直线度计算
LineFitResult PicProcessor::evaluateStraightness(
    const std::vector<cv::Point>& topEdge,
    const std::vector<cv::Point>& bottomEdge)
{
    /* ==================== 1. 安全检查 ==================== */

    if (topEdge.empty() || bottomEdge.empty())
    {
        std::cerr << "[Error] Edge vectors are empty.\n";
        return {0.0, 0.0, cv::Vec4f(0,0,0,0)};
    }

    size_t N = std::min(topEdge.size(), bottomEdge.size());
    if (N < 2)
    {
        std::cerr << "[Error] Not enough points.\n";
        return {0.0, 0.0, cv::Vec4f(0,0,0,0)};
    }

    /* ==================== 2. 计算中点 ==================== */

    std::vector<cv::Point2d> midPoints;
    midPoints.reserve(N);

    for (size_t i = 0; i < N; ++i)
    {
        midPoints.emplace_back(
            0.5 * (topEdge[i].x + bottomEdge[i].x),
            0.5 * (topEdge[i].y + bottomEdge[i].y)
            );
    }
    std::cout<<"midpointsSize:"<<midPoints.size()<<std::endl;
    /* ==================== 3. 鲁棒直线拟合（double） ==================== */

    cv::Vec4d line;   // ★ 必须用 double
    // cv::fitLine(midPoints, line, cv::DIST_HUBER, 0, 0.01, 0.01);
    cv::fitLine(midPoints, line, cv::DIST_L2, 0, 0, 0);


    double vx = line[0];
    double vy = line[1];
    double x0 = line[2];
    double y0 = line[3];

    /* ==================== 4. 方向归一 + 符号统一 ==================== */

    double norm = std::sqrt(vx * vx + vy * vy);
    if (norm > 1e-12)
    {
        vx /= norm;
        vy /= norm;
    }

    // 强制方向唯一（防止符号翻转）
    if (vx < 0.0)
    {
        vx = -vx;
        vy = -vy;
    }

    /* ==================== 5. 点到线距离（double） ==================== */

    std::vector<double> distances;
    distances.reserve(N);

    for (const auto& p : midPoints)
    {
        double d = std::abs(vx * (y0 - p.y) - vy * (x0 - p.x));
        distances.push_back(d);
    }

    /* ==================== 6. 最大偏差（稳定） ==================== */

    double maxDev = *std::max_element(distances.begin(), distances.end());

    /* ==================== 7. RMS（Kahan 稳定求和） ==================== */

    std::sort(distances.begin(), distances.end());

    double sum = 0.0;
    for (double d : distances)
        sum += d * d;

    double rms = std::sqrt(sum / distances.size());


    /* ==================== 8. 返回（兼容旧接口） ==================== */
    /* ==================== 保存 midPoints 到文件 ==================== */

    // static int dumpIndex = 0;   // ★ 进程内递增
    // std::ostringstream filename;
    // filename << "midpoints_" << std::setw(4) << std::setfill('0')
    //          << dumpIndex++ << ".txt";

    // std::ofstream ofs(filename.str());
    // if (!ofs.is_open())
    // {
    //     std::cerr << "[Error] Cannot open file: " << filename.str() << std::endl;
    // }
    // else
    // {
    //     ofs << std::fixed << std::setprecision(8);

    //     for (size_t i = 0; i < midPoints.size(); ++i)
    //     {
    //         ofs << i << ", "
    //             << midPoints[i].x << ", "
    //             << midPoints[i].y << "\n";
    //     }

    //     ofs.close();
    // }



    // 如果你的 LineFitResult 里还是 Vec4f，这里只在“最后一步”降精度
    return {
        maxDev,
        rms,
        cv::Vec4f(
            static_cast<float>(vx),
            static_cast<float>(vy),
            static_cast<float>(x0),
            static_cast<float>(y0))
    };
}


//更改图像尺寸
void PicProcessor::resizeKeep(const cv::Mat& input, cv::Mat& output, int w, int h)
{
    using namespace std;
    double sx = (double)w / input.cols;
    double sy = (double)h / input.rows;
    double s = min(sx, sy);

    cv::resize(input, output, cv::Size(input.cols * s, input.rows * s), 0, 0, cv::INTER_AREA);
}

// 中位数
double PicProcessor:: median(std::vector<int>& v)
{
    size_t n = v.size() / 2;
    std::nth_element(v.begin(), v.begin() + n, v.end());
    return v[n];
}

// MAD(中位数绝对偏差)
double PicProcessor::mad(const std::vector<int>& v, double med)
{
    std::vector<int> dev;
    dev.reserve(v.size());
    for (int x : v)
        dev.push_back(std::abs(x - static_cast<int>(med)));
    return median(dev);
}
//上下边缘提取点滤波函数
std::vector<EdgePair> PicProcessor::filterEdgePairs(
    const std::vector<EdgePair>& input)
{
    if (input.size() < 10)
        return input;

    // ===== 1. 高度一致性（MAD）=====
    std::vector<int> centers;
    centers.reserve(input.size());

    for (auto& p : input)
    {
        centers.push_back((p.top + p.bottom) / 2);
    }

    double c_med = median(centers);
    double c_mad = mad(centers, c_med);

    if (c_mad < 1.0)
        c_mad = 1.0;

    std::vector<EdgePair> stage1;

    for (size_t i = 0; i < input.size(); i++)
    {
        if (std::abs(centers[i] - c_med) < 15.0 * c_mad)
        {
            stage1.push_back(input[i]);
        }
    }

    // ===== 2. 列方向连续性 =====
    const int T = 4;

    std::vector<EdgePair> result;
    result.push_back(stage1.front());

    EdgePair pref = stage1.front();  // 前参考
    EdgePair ref  = stage1.front();  // 当前参考

    for (size_t i = 1; i < stage1.size(); i++)
    {
        const auto& curr = stage1[i];

        bool cond1 = std::abs(curr.top - ref.top) < T &&
                    std::abs(curr.bottom - ref.bottom) < T;

        bool cond2 = std::abs(curr.top - pref.top) < T &&
                    std::abs(curr.bottom - pref.bottom) < T;

        if (cond1 && cond2)
        {
            result.push_back(curr);
        }
        
        pref = ref;   // 更新前参考
        ref  = curr;  // 更新当前参考
    }

    return result;
}
void PicProcessor::cannyEdgeExtraction(const cv::Mat& src,
                                            cv::Mat& binary,
                                        std::vector<EdgePair>& rawPairs)
{
    CV_Assert(!src.empty());
    CV_Assert(src.type() == CV_8UC1);

    rawPairs.clear();
    rawPairs.reserve(src.cols);

    cv::Mat edges;

    // Canny 边缘检测
    cv::Canny(src, edges, 100, 200);
    binary = edges;

    // 逐列扫描提取上下边缘
    for (int col = 0; col < edges.cols; col++)
    {
        int topRow = -1;
        int bottomRow = -1;

        // 从上往下找
        for (int row = 0; row < edges.rows; row++)
        {
            if (edges.at<uchar>(row, col) > 0)
            {
                topRow = row;
                break;
            }
        }

        // 从下往上找
        for (int row = edges.rows - 1; row >= 0; row--)
        {
            if (edges.at<uchar>(row, col) > 0)
            {
                bottomRow = row;
                break;
            }
        }

        if (topRow >= 0 && bottomRow >= 0 && bottomRow > topRow)
        {
            rawPairs.push_back({col, topRow, bottomRow});
        }
    }
}
void PicProcessor::filterEdgePairsToImage(const cv::Mat& src,
                                          const std::vector<EdgePair>& rawPairs,
                                          cv::Mat& binary)
{
    CV_Assert(!src.empty());

    binary = cv::Mat::zeros(src.size(), CV_8UC1);


    // 点对级滤波
    std::vector<EdgePair> filteredPairs = filterEdgePairs(rawPairs);

    // 生成输出
    for (const auto& p : filteredPairs)
    {
        binary.at<uchar>(p.top, p.col) = 255;
        binary.at<uchar>(p.bottom, p.col) = 255;
    }
}
std::vector<double> PicProcessor::savitzkyGolaySmooth(const std::vector<double>& y){
        int n = y.size();
    std::vector<double> out(n);

    if (n < 5)
        return y;

    const double c[5] = { -3,12,17,12,-3 };
    const double norm = 35.0;

    for (int i = 2; i < n - 2; i++)
    {
        double sum = 0;
        for (int j = -2; j <= 2; j++)
        {
            sum += c[j + 2] * y[i + j];
        }
        out[i] = sum / norm;
    }

    // 边界直接复制
    out[0] = y[0];
    out[1] = y[1];
    out[n - 2] = y[n - 2];
    out[n - 1] = y[n - 1];

    return out;
}
std::vector<EdgePair> PicProcessor::filterEdgePairsPlus3(
    const std::vector<EdgePair>& input)
{
    if (input.size() < 10)
        return input;

    // ===== 1. 高度一致性（MAD）=====
    std::vector<int> heights;
    heights.reserve(input.size());

    for (auto& p : input)
        heights.push_back(p.bottom - p.top);

    double h_med = median(heights);
    double h_mad = mad(heights, h_med);

    if (h_mad < 1.0)
        h_mad = 1.0;

    std::vector<EdgePair> stage1;

    for (auto& p : input)
    {
        if (std::abs((p.bottom - p.top) - h_med) < 3.0 * h_mad)
            stage1.push_back(p);
    }

    if (stage1.size() < 5)
        return stage1;

    // ===== 2. 列方向连续性 =====
    const int T = 8;

    std::vector<EdgePair> stage2;
    stage2.push_back(stage1.front());

    for (size_t i = 1; i < stage1.size(); i++)
    {
        const auto& prev = stage2.back();
        const auto& curr = stage1[i];

        if (std::abs(curr.top - prev.top) < T &&
            std::abs(curr.bottom - prev.bottom) < T)
        {
            stage2.push_back(curr);
        }
    }

    if (stage2.size() < 5)
        return stage2;

    // ===== 3. Savitzky-Golay 平滑 =====
    std::vector<double> topVec;
    std::vector<double> bottomVec;

    for (auto& p : stage2)
    {
        topVec.push_back(p.top);
        bottomVec.push_back(p.bottom);
    }

    std::vector<double> topSmooth = savitzkyGolaySmooth(topVec);
    std::vector<double> bottomSmooth = savitzkyGolaySmooth(bottomVec);

    std::vector<EdgePair> result = stage2;

    for (size_t i = 0; i < result.size(); i++)
    {
        result[i].top = static_cast<int>(topSmooth[i]);
        result[i].bottom = static_cast<int>(bottomSmooth[i]);
    }

    return result;
}
/*********************************
 * RGB转灰度
 * Gray = 0.299R + 0.587G + 0.114B
 *********************************/
void PicProcessor::RGB2Gray(const cv::Mat& image, cv::Mat& grayImage)
{
   if (image.empty()) {
       std::cerr << "Error: Could not open or find the image!" << std::endl;
       return ;
   }
   // 确保输入图像为彩色图像
   if (image.channels() != 3) {
       std::cerr << "Error: Input image is not a color image!" << std::endl;
       return ;
   }

   // 创建用于存储灰度图像的矩阵
   grayImage = cv::Mat(image.size(), CV_8UC1);

   // 遍历图像每个像素，手动计算灰度值
   for (int row = 0; row < image.rows; row++) {
       for (int col = 0; col < image.cols; col++) {
           // 获取 BGR 像素值
           cv::Vec3b pixel = image.at<cv::Vec3b>(row, col);
           uchar blue = pixel[0];   // B
           uchar green = pixel[1];  // G
           uchar red = pixel[2];    // R
           // 计算灰度值
           uchar gray = static_cast<uchar>(0.299 * red + 0.587 * green + 0.144 * blue);
        //    uchar gray = static_cast<uchar>(1 * red + 1 * green + 1 * blue);
           // 设置灰度图像的像素值
           grayImage.at<uchar>(row, col) = gray;
       }
   }

}

void PicProcessor::applyGaussianBlur(const cv::Mat& src, cv::Mat& dst, int kernelSize){
    cv::GaussianBlur(src, dst, cv::Size(kernelSize, kernelSize), 0);
}
void PicProcessor::applyMedianBlur(const cv::Mat& src, cv::Mat& dst, int kernelSize){
    cv::medianBlur(src, dst, kernelSize);
}
/*
@param diameter: 邻域直径，必须为正奇数
@param sigmaColor: 颜色空间的标准差，较大的值会使更远的颜色对当前像素产生更大的影响，较小的值会使只相近颜色产生影响
@param sigmaSpace: 坐标空间的标准差，较大的值会使更远的像素对当前像素产生更大的影响，较小的值会使只相近像素产生影响
*/
void PicProcessor::applyBilateralFilter(const cv::Mat& src, cv::Mat& dst, int diameter = 9, double sigmaColor = 75, double sigmaSpace = 75){
    cv::bilateralFilter(src, dst, diameter, sigmaColor, sigmaSpace);
}

cv::Mat PicProcessor::roatateImage(const cv::Mat &img, int angle)
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
