#pragma once
#include <string>
#include <memory>
#include <opencv2/opencv.hpp>
#include "autobackend.h"
#include "common.h"

class YoloOnnxDetector {
public:
    explicit YoloOnnxDetector(const std::string& modelPath,
                              const std::string& provider = OnnxProviders::CPU,
                              const std::string& logId = "yolo_runtime");

    bool setImage(const cv::Mat& img);

    bool infer(float confThreshold = 0.30f,
               float iouThreshold = 0.45f,
               float maskThreshold = 0.50f,
               int conversionCode = cv::COLOR_BGR2RGB);

    const std::vector<YoloResults>& getResults() const;

    void drawResults(cv::Mat& outImg);
    const std::unordered_map<int, std::string>& getNames() const;
    //仅供测试使用，正式使用时请勿调用
    void setResults(const std::vector<YoloResults>& results){
        results_ = results;
    }
    void setNames(const std::unordered_map<int, std::string>& names){
        names_ = names;
    }
    cv::Mat extractByMask(const cv::Mat& img, const YoloResults& result, bool useROI = true);

public:
    std::string modelPath_;
    cv::Mat image_;
    std::vector<YoloResults> results_;
    std::unique_ptr<AutoBackendOnnx> backend_;

    std::vector<cv::Scalar> colors_;
    std::unordered_map<int, std::string> names_;
};
