#include "yoloonnxdetector.h"
#include "augment.h"
#include "common.h"
#include <iostream>
#include <random>
#include <drawingutils.h>
#include <opencv2/opencv.hpp>
#include <unordered_map>


YoloOnnxDetector::YoloOnnxDetector(const std::string& modelPath,
                                   const std::string& provider,
                                   const std::string& logId)
    : modelPath_(modelPath)
{
    backend_ = std::make_unique<AutoBackendOnnx>(modelPath.c_str(),
                                                 logId.c_str(),
                                                 provider.c_str());

    int numClasses = backend_->getNc();
    int numChannels = backend_->getCh();

    colors_ = generateRandomColors(numClasses, numChannels);
    names_  = backend_->getNames();
}

bool YoloOnnxDetector::setImage(const cv::Mat& img)
{
    if (img.empty()) return false;
    image_ = img.clone();
    return true;
}

bool YoloOnnxDetector::infer(float confThreshold,
                             float iouThreshold,
                             float maskThreshold,
                             int conversionCode)
{
    if (image_.empty()) {
        std::cerr << "Error: No image set for inference.\n";
        return false;
    }

    results_.clear();
    results_ = backend_->predict_once(image_, confThreshold,
                                      iouThreshold, maskThreshold,
                                      conversionCode);

    return true;
}

const std::vector<YoloResults>& YoloOnnxDetector::getResults() const
{
    return results_;
}

void YoloOnnxDetector::drawResults(cv::Mat& outImg)
{
    if (image_.empty()) return;

    outImg = image_.clone();
    cv::cvtColor(outImg, outImg, cv::COLOR_RGB2BGR);

    cv::Size show_shape = outImg.size();

    plot_results(outImg, const_cast<std::vector<YoloResults>&>(results_),
                 colors_, names_, show_shape);
}

const std::unordered_map<int, std::string>& YoloOnnxDetector::getNames() const
{
    return names_;
}