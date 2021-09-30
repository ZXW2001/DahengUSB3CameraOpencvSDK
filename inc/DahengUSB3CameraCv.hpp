/**
 * @file DahengUSB3CameraCv.hpp
 * @author ZXW2600 (zhaoxinwei74@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-29
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __DAHENGUSB3CAMERACV_HPP__
#define __DAHENGUSB3CAMERACV_HPP__

#include <iostream>

#include "DoubleBuffer.hpp"
#include "DxImageProc.h"
#include "GxIAPI.h"

#include <opencv2/opencv.hpp>

class GalaxyCamera
{
private:
    GX_STATUS status = GX_STATUS_SUCCESS;
    GX_DEV_HANDLE hDevice = NULL;
    GX_OPEN_PARAM stOpenParam;
    uint32_t nDeviceNum = 0;

    DoubleBuffer<cv::Mat> image_buffer_BGR;

    static void GX_STDC OnFrameCallbackFun(GX_FRAME_CALLBACK_PARAM *pFrame);
    int setGalaxyParmInt(GX_FEATURE_ID_CMD parms_id, int64_t parms_value);
    int setGalaxyParmFloat(GX_FEATURE_ID_CMD parms_id, double parms_value);

public:
    GalaxyCamera(int galaxy_camera_index);

    ~GalaxyCamera();
    int open(int galaxy_camera_index);
    // bool isOpened();
    int realease();
    bool read(cv::Mat &image);

    int setCameraParms(cv::String config_file_path);

    int setAutoWhiteBalance(GX_BALANCE_WHITE_AUTO_ENTRY AWB_mode, GX_AWB_LAMP_HOUSE_ENTRY AWB_entry);

    int setAutoExposure(bool auto_exposure, bool once = false, float exposure_time_min = -1, float exposure_time_max = -1);
    int setExposureTime(double exposure_time);
    int setAutoGain(bool auto_gain, bool once = false, float gain_min = -1, float gain_max = -1);
    int setGain(double gain);

    // GalaxyCamera &operator>>(cv::Mat &image);
};

#endif // __DAHENGUSB3CAMERACV_HPP__
