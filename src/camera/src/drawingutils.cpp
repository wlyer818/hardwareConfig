#include "DrawingUtils.h"
#include <random>

// Define the skeleton and color mappings
std::vector<std::vector<int>> skeleton = {{16, 14}, {14, 12}, {17, 15}, {15, 13}, {12, 13}, {6, 12}, {7, 13}, {6, 7},
                                          {6, 8}, {7, 9}, {8, 10}, {9, 11}, {2, 3}, {1, 2}, {1, 3}, {2, 4}, {3, 5}, {4, 6}, {5, 7}};

std::vector<cv::Scalar> posePalette = {
        cv::Scalar(255, 128, 0), cv::Scalar(255, 153, 51), cv::Scalar(255, 178, 102), cv::Scalar(230, 230, 0), cv::Scalar(255, 153, 255),
        cv::Scalar(153, 204, 255), cv::Scalar(255, 102, 255), cv::Scalar(255, 51, 255), cv::Scalar(102, 178, 255), cv::Scalar(51, 153, 255),
        cv::Scalar(255, 153, 153), cv::Scalar(255, 102, 102), cv::Scalar(255, 51, 51), cv::Scalar(153, 255, 153), cv::Scalar(102, 255, 102),
        cv::Scalar(51, 255, 51), cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255), cv::Scalar(255, 0, 0), cv::Scalar(255, 255, 255)
};

std::vector<int> limbColorIndices = {9, 9, 9, 9, 7, 7, 7, 0, 0, 0, 0, 0, 16, 16, 16, 16, 16, 16, 16};
std::vector<int> kptColorIndices = {16, 16, 16, 16, 16, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 9, 9};



cv::Scalar generateRandomColor(int numChannels) {
    if (numChannels < 1 || numChannels > 3) {
        throw std::invalid_argument("Invalid number of channels. Must be between 1 and 3.");
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 255);

    cv::Scalar color;
    for (int i = 0; i < numChannels; i++) {
        color[i] = dis(gen); // for each channel separately generate value
    }

    return color;
}

std::vector<cv::Scalar> generateRandomColors(int class_names_num, int numChannels) {
    std::vector<cv::Scalar> colors;
    for (int i = 0; i < class_names_num; i++) {
        cv::Scalar color = generateRandomColor(numChannels);
        colors.push_back(color);
    }
    return colors;
}

void plot_masks(cv::Mat img, std::vector<YoloResults>& result, std::vector<cv::Scalar> color,
    std::unordered_map<int, std::string>& names)
{
    cv::Mat mask = img.clone();
    for (int i = 0; i < result.size(); i++)
    {
        float left, top;
        left = result[i].bbox.x;
        top = result[i].bbox.y;
        int color_num = i;
        int& class_idx = result[i].class_idx;
        rectangle(img, result[i].bbox, color[result[i].class_idx], 2);

        // try to get string value corresponding to given class_idx
        std::string class_name;
        auto it = names.find(class_idx);
        if (it != names.end()) {
            class_name = it->second;
        }
        else {
            std::cerr << "Warning: class_idx not found in names for class_idx = " << class_idx << std::endl;
            // then convert it to string anyway
            class_name = std::to_string(class_idx);
        }

        if (result[i].mask.rows && result[i].mask.cols > 0)
        {
            mask(result[i].bbox).setTo(color[result[i].class_idx], result[i].mask);
        }
        std::stringstream labelStream;
        labelStream << class_name << " " << std::fixed << std::setprecision(2) << result[i].conf;
        std::string label = labelStream.str();

    	cv::Size text_size = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.6, 2, nullptr);
        cv::Rect rect_to_fill(left - 1, top - text_size.height - 5, text_size.width + 2, text_size.height + 5);
        cv::Scalar text_color = cv::Scalar(255.0, 255.0, 255.0);
        rectangle(img, rect_to_fill, color[result[i].class_idx], -1);

        putText(img, label, cv::Point(left - 1.5, top - 2.5), cv::FONT_HERSHEY_SIMPLEX, 0.6, text_color, 2);
    }
    addWeighted(img, 0.6, mask, 0.4, 0, img); //add mask to src
    resize(img, img, img.size());
    imshow("img", img);
    cv::waitKey();
}


//void plot_keypoints(cv::Mat& image, const std::vector<std::vector<float>>& keypoints, const cv::Size& shape) {
void plot_keypoints(cv::Mat& image, const std::vector<YoloResults>& results, const cv::Size& shape) {

    int radius = 5;
    bool drawLines = true;

    if (results.empty()) {
        return;
    }

    std::vector<cv::Scalar> limbColorPalette;
    std::vector<cv::Scalar> kptColorPalette;

    for (int index : limbColorIndices) {
        limbColorPalette.push_back(posePalette[index]);
    }

    for (int index : kptColorIndices) {
        kptColorPalette.push_back(posePalette[index]);
    }

    for (const auto& res: results) {
        auto keypoint = res.keypoints;
        bool isPose = keypoint.size() == 51;  // numKeypoints == 17 && keypoints[0].size() == 3;
        drawLines &= isPose;

        // draw points
        for (int i = 0; i < 17; i++) {
            int idx = i * 3;
            int x_coord = static_cast<int>(keypoint[idx]);
            int y_coord = static_cast<int>(keypoint[idx + 1]);

            if (x_coord % shape.width != 0 && y_coord % shape.height != 0) {
                if (keypoint.size() == 3) {
                    float conf = keypoint[2];
                    if (conf < 0.5) {
                        continue;
                    }
                }
                cv::Scalar color_k = isPose ? kptColorPalette[i] : cv::Scalar(0, 0,
                                                                               255);  // Default to red if not in pose mode
                cv::circle(image, cv::Point(x_coord, y_coord), radius, color_k, -1, cv::LINE_AA);
            }
        }
        // draw lines
        if (drawLines) {
            for (int i = 0; i < skeleton.size(); i++) {
                const std::vector<int> &sk = skeleton[i];
                int idx1 = sk[0] - 1;
                int idx2 = sk[1] - 1;

                int idx1_x_pos = idx1 * 3;
                int idx2_x_pos = idx2 * 3;

                int x1 = static_cast<int>(keypoint[idx1_x_pos]);
                int y1 = static_cast<int>(keypoint[idx1_x_pos + 1]);
                int x2 = static_cast<int>(keypoint[idx2_x_pos]);
                int y2 = static_cast<int>(keypoint[idx2_x_pos + 1]);

                float conf1 = keypoint[idx1_x_pos + 2];
                float conf2 = keypoint[idx2_x_pos + 2];

                // Check confidence thresholds
                if (conf1 < 0.5 || conf2 < 0.5) {
                    continue;
                }

                // Check if positions are within bounds
                if (x1 % shape.width == 0 || y1 % shape.height == 0 || x1 < 0 || y1 < 0 ||
                    x2 % shape.width == 0 || y2 % shape.height == 0 || x2 < 0 || y2 < 0) {
                    continue;
                }

                // Draw a line between keypoints
                cv::Scalar color_limb = limbColorPalette[i];
                cv::line(image, cv::Point(x1, y1), cv::Point(x2, y2), color_limb, 2, cv::LINE_AA);
            }
        }
    }
}

// void plot_results(cv::Mat img, std::vector<YoloResults>& results,
//                   std::vector<cv::Scalar> color, std::unordered_map<int, std::string>& names,
//                   const cv::Size& shape
//                   ) {

//     cv::Mat mask = img.clone();

//     int radius = 5;
//     bool drawLines = true;

//     auto raw_image_shape = img.size();
//     std::vector<cv::Scalar> limbColorPalette;
//     std::vector<cv::Scalar> kptColorPalette;

//     for (int index : limbColorIndices) {
//         limbColorPalette.push_back(posePalette[index]);
//     }

//     for (int index : kptColorIndices) {
//         kptColorPalette.push_back(posePalette[index]);
//     }

//     for (const auto& res : results) {
//         float left = res.bbox.x;
//         float top = res.bbox.y;
//         int color_num = res.class_idx;

//         // Draw bounding box
//         rectangle(img, res.bbox, color[res.class_idx], 2);

//         // Try to get the class name corresponding to the given class_idx
//         std::string class_name;
//         auto it = names.find(res.class_idx);
//         if (it != names.end()) {
//             class_name = it->second;
//         }
//         else {
//             std::cerr << "Warning: class_idx not found in names for class_idx = " << res.class_idx << std::endl;
//             // Then convert it to a string anyway
//             class_name = std::to_string(res.class_idx);
//         }

//         // Draw mask if available
//         if (res.mask.rows && res.mask.cols > 0) {
//             mask(res.bbox).setTo(color[res.class_idx], res.mask);
//         }

//         // Create label
//         std::stringstream labelStream;
//         labelStream << class_name << " " << std::fixed << std::setprecision(2) << res.conf;
//         std::string label = labelStream.str();

//         cv::Size text_size = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.6, 2, nullptr);
//         cv::Rect rect_to_fill(left - 1, top - text_size.height - 5, text_size.width + 2, text_size.height + 5);
//         cv::Scalar text_color = cv::Scalar(255.0, 255.0, 255.0);
//         rectangle(img, rect_to_fill, color[res.class_idx], -1);
//         putText(img, label, cv::Point(left - 1.5, top - 2.5), cv::FONT_HERSHEY_SIMPLEX, 0.6, text_color, 2);

//         // Check if keypoints are available
//         if (!res.keypoints.empty()) {
//             auto keypoint = res.keypoints;
//             bool isPose = keypoint.size() == 51;  // numKeypoints == 17 && keypoints[0].size() == 3;
//             drawLines &= isPose;

//             // draw points
//             for (int i = 0; i < 17; i++) {
//                 int idx = i * 3;
//                 int x_coord = static_cast<int>(keypoint[idx]);
//                 int y_coord = static_cast<int>(keypoint[idx + 1]);

//                 if (x_coord % raw_image_shape.width != 0 && y_coord % raw_image_shape.height != 0) {
//                     if (keypoint.size() == 3) {
//                         float conf = keypoint[2];
//                         if (conf < 0.5) {
//                             continue;
//                         }
//                     }
//                     cv::Scalar color_k = isPose ? kptColorPalette[i] : cv::Scalar(0, 0,
//                                                                                   255);  // Default to red if not in pose mode
//                     cv::circle(img, cv::Point(x_coord, y_coord), radius, color_k, -1, cv::LINE_AA);
//                 }
//             }
//             // draw lines
//             if (drawLines) {
//                 for (int i = 0; i < skeleton.size(); i++) {
//                     const std::vector<int> &sk = skeleton[i];
//                     int idx1 = sk[0] - 1;
//                     int idx2 = sk[1] - 1;

//                     int idx1_x_pos = idx1 * 3;
//                     int idx2_x_pos = idx2 * 3;

//                     int x1 = static_cast<int>(keypoint[idx1_x_pos]);
//                     int y1 = static_cast<int>(keypoint[idx1_x_pos + 1]);
//                     int x2 = static_cast<int>(keypoint[idx2_x_pos]);
//                     int y2 = static_cast<int>(keypoint[idx2_x_pos + 1]);

//                     float conf1 = keypoint[idx1_x_pos + 2];
//                     float conf2 = keypoint[idx2_x_pos + 2];

//                     // Check confidence thresholds
//                     if (conf1 < 0.5 || conf2 < 0.5) {
//                         continue;
//                     }

//                     // Check if positions are within bounds
//                     if (x1 % raw_image_shape.width == 0 || y1 % raw_image_shape.height == 0 || x1 < 0 || y1 < 0 ||
//                         x2 % raw_image_shape.width == 0 || y2 % raw_image_shape.height == 0 || x2 < 0 || y2 < 0) {
//                         continue;
//                     }

//                     // Draw a line between keypoints
//                     cv::Scalar color_limb = limbColorPalette[i];
//                     cv::line(img, cv::Point(x1, y1), cv::Point(x2, y2), color_limb, 2, cv::LINE_AA);
//                 }
//             }
//         }
//     }

//     // Combine the image and mask
//     addWeighted(img, 0.6, mask, 0.4, 0, img);
// //    resize(img, img, img.size());
// //    resize(img, img, shape);
// //    // Show the image
// //    imshow("img", img);
// //    cv::waitKey();
// }
void plot_results(cv::Mat img,
                  std::vector<YoloResults>& results,
                  std::vector<cv::Scalar> color,
                  std::unordered_map<int, std::string>& names,
                  const cv::Size& shape)
{
    cv::Mat mask = img.clone();
    cv::Size raw_image_shape = img.size();

    int radius = 5;
    bool drawLines = true;

    // ================= 自适应参数 =================
    int img_w = img.cols;
    int img_h = img.rows;
    int min_dim = std::min(img_w, img_h);

    float scale = min_dim / 640.0f;
    scale = std::clamp(scale, 0.6f, 2.5f);

    double fontScale   = 0.6 * scale;
    int textThickness  = std::max(1, int(2 * scale));
    int boxThickness   = std::max(1, int(2 * scale));
    int padding        = std::max(2, int(5 * scale));
    // =================================================

    // 关键点颜色
    std::vector<cv::Scalar> limbColorPalette;
    std::vector<cv::Scalar> kptColorPalette;

    for (int index : limbColorIndices)
        limbColorPalette.push_back(posePalette[index]);

    for (int index : kptColorIndices)
        kptColorPalette.push_back(posePalette[index]);

    // ================= 主循环 =================
    for (const auto& res : results)
    {
        float left = res.bbox.x;
        float top  = res.bbox.y;

        // 1. 绘制检测框
        cv::rectangle(img, res.bbox, color[res.class_idx], boxThickness);

        // 2. 类别名
        std::string class_name;
        auto it = names.find(res.class_idx);
        class_name = (it != names.end()) ? it->second : std::to_string(res.class_idx);

        // 3. 分割 Mask
        if (res.mask.rows && res.mask.cols > 0)
        {
            mask(res.bbox).setTo(color[res.class_idx], res.mask);
        }

        // 4. 标签文本
        std::stringstream ss;
        ss << class_name << " " << std::fixed << std::setprecision(2) << res.conf;
        std::string label = ss.str();

        // 5. 计算文本尺寸
        cv::Size text_size = cv::getTextSize(
            label,
            cv::FONT_HERSHEY_SIMPLEX,
            fontScale,
            textThickness,
            nullptr
        );

        // 6. 标签背景框（自适应）
        cv::Rect label_rect(
            int(left),
            int(top - text_size.height - padding * 2),
            text_size.width + padding * 2,
            text_size.height + padding * 2
        );

        // 防止越界
        label_rect &= cv::Rect(0, 0, img_w, img_h);

        // 7. 绘制标签
        cv::rectangle(img, label_rect, color[res.class_idx], cv::FILLED);

        cv::putText(
            img,
            label,
            cv::Point(
                label_rect.x + padding,
                label_rect.y + label_rect.height - padding
            ),
            cv::FONT_HERSHEY_SIMPLEX,
            fontScale,
            cv::Scalar(255, 255, 255),
            textThickness,
            cv::LINE_AA
        );

        // ================= 姿态关键点 =================
        if (!res.keypoints.empty())
        {
            auto& keypoint = res.keypoints;
            bool isPose = keypoint.size() == 51;  // 17 * 3
            drawLines &= isPose;

            // 关键点
            for (int i = 0; i < 17; i++)
            {
                int idx = i * 3;
                float conf = keypoint[idx + 2];
                if (conf < 0.5f) continue;

                int x = static_cast<int>(keypoint[idx]);
                int y = static_cast<int>(keypoint[idx + 1]);

                if (x <= 0 || y <= 0 || x >= img_w || y >= img_h) continue;

                cv::Scalar kp_color = isPose ? kptColorPalette[i] : cv::Scalar(0, 0, 255);
                cv::circle(img, {x, y}, radius * scale, kp_color, -1, cv::LINE_AA);
            }

            // 骨架线
            if (drawLines)
            {
                for (size_t i = 0; i < skeleton.size(); i++)
                {
                    int idx1 = (skeleton[i][0] - 1) * 3;
                    int idx2 = (skeleton[i][1] - 1) * 3;

                    float conf1 = keypoint[idx1 + 2];
                    float conf2 = keypoint[idx2 + 2];
                    if (conf1 < 0.5f || conf2 < 0.5f) continue;

                    cv::Point p1(keypoint[idx1], keypoint[idx1 + 1]);
                    cv::Point p2(keypoint[idx2], keypoint[idx2 + 1]);

                    if (!cv::Rect(0, 0, img_w, img_h).contains(p1) ||
                        !cv::Rect(0, 0, img_w, img_h).contains(p2))
                        continue;

                    cv::line(img, p1, p2, limbColorPalette[i], boxThickness, cv::LINE_AA);
                }
            }
        }
    }

    // ================= Mask 融合 =================
    cv::addWeighted(img, 0.6, mask, 0.4, 0, img);
}
//将当前日期转化为字符串
std::string getCurrentDateTimeString(){
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_time_t);
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y_%m_%d_%H_%M_%S");
    return oss.str();
}