/**
 * @file cv_imshow.cpp
 * @author ZXW2600 (zhaoxinwei74@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-29
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "DahengUSB3CameraCv.hpp"
#include "TimeCostCounter.hpp"

int main()
{
    GalaxyCamera demo_camera(1);
    // demo_camera.setCameraParms("test");
    demo_camera.setAutoExposure(false);
    demo_camera.setExposureTime(1000);
    demo_camera.setAutoGain(false);
    demo_camera.setGain(16);
    cv::Mat image;
    cv::Mat image_last;
    cv::Mat diff;
    uint image_count = 0;
    TimeCostCounter timer;
    while (true)
    {
        if (demo_camera.read(image))
        {
            {
                cv::imshow("demo_camera", image);
                // if (!image_last.empty())
                // {
                //     cv::subtract(image_last, image, diff);
                //     cv::imshow("sub image", diff);
                // }
                // image_last = image.clone();

                if (image_count++ > 210)
                {
                    timer.elapse();
                    image_count = 0;
                    timer.begin();
                }
            }
        }
        int key = cv::waitKey(1);
        if (key == 'q')
            return 0;
    }

    return 0;
}