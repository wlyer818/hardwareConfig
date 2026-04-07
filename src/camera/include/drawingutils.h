#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <unordered_map>
#include "common.h"
#include "autobackend.h"
void plot_results(cv::Mat img,
                  std::vector<YoloResults>& results,
                  std::vector<cv::Scalar> color,
                  std::unordered_map<int, std::string>& names,
                  const cv::Size& shape);

void plot_masks(cv::Mat img,
                std::vector<YoloResults>& results,
                std::vector<cv::Scalar> color,
                std::unordered_map<int, std::string>& names);

void plot_keypoints(cv::Mat& image,
                    const std::vector<YoloResults>& results,
                    const cv::Size& shape);
std::vector<cv::Scalar> generateRandomColors(int class_names_num, int numChannels);
cv::Scalar generateRandomColor(int numChannels);
//将当前日期转化为字符串
std::string getCurrentDateTimeString();
//在图片上绘制矩形
void drawRectangle(cv::Mat& image, const cv::Rect& rect, const cv::Scalar& color, int thickness = 2);