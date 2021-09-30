#include "DahengUSB3CameraCv.hpp"
#include "GxIAPI.h"
#include "TimeCostCounter.hpp"
using namespace cv;
//Show error message
#define GX_VERIFY(emStatus)            \
    if (emStatus != GX_STATUS_SUCCESS) \
    {                                  \
        GetErrorString(emStatus);      \
        return emStatus;               \
    }

//----------------------------------------------------------------------------------
/**
\brief  Get description of input error code
\param  emErrorStatus  error code

\return void
*/
//----------------------------------------------------------------------------------
void GetErrorString(GX_STATUS emErrorStatus)
{
    char *error_info = NULL;
    size_t size = 0;
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    // Get length of error description
    emStatus = GXGetLastError(&emErrorStatus, NULL, &size);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        printf("<Error when calling GXGetLastError>\n");
        return;
    }

    // Alloc error resources
    error_info = new char[size];
    if (error_info == NULL)
    {
        printf("<Failed to allocate memory>\n");
        return;
    }

    // Get error description
    emStatus = GXGetLastError(&emErrorStatus, error_info, &size);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        printf("<Error when calling GXGetLastError>\n");
    }
    else
    {
        printf("%s\n", error_info);
    }

    // Realease error resources
    if (error_info != NULL)
    {
        delete[] error_info;
        error_info = NULL;
    }
}

//图像回调处理函数
void GX_STDC GalaxyCamera::OnFrameCallbackFun(GX_FRAME_CALLBACK_PARAM *pFrame)
{
    if (pFrame->status == GX_FRAME_STATUS_SUCCESS)
    {
        DoubleBuffer<cv::Mat> *buffer = (DoubleBuffer<cv::Mat> *)pFrame->pUserParam;
        // Convert RAW8 or RAW16 image to RGB24 image
        switch (pFrame->nPixelFormat)
        {
        case GX_PIXEL_FORMAT_BAYER_GR8:
        case GX_PIXEL_FORMAT_BAYER_RG8:
        case GX_PIXEL_FORMAT_BAYER_GB8:
        case GX_PIXEL_FORMAT_BAYER_BG8:
        {
            cv::Mat image_buffer_Bayer(cv::Size(pFrame->nWidth, pFrame->nHeight), CV_8UC1);
            image_buffer_Bayer.data = (uchar *)pFrame->pImgBuf;
            // Convert to the RGB image
            cv::cvtColor(image_buffer_Bayer, buffer->getSetBuffer(), cv::COLOR_BayerBG2BGR);
            break;
        }
        case GX_PIXEL_FORMAT_BAYER_GR10:
        case GX_PIXEL_FORMAT_BAYER_RG10:
        case GX_PIXEL_FORMAT_BAYER_GB10:
        case GX_PIXEL_FORMAT_BAYER_BG10:
        case GX_PIXEL_FORMAT_BAYER_GR12:
        case GX_PIXEL_FORMAT_BAYER_RG12:
        case GX_PIXEL_FORMAT_BAYER_GB12:
        case GX_PIXEL_FORMAT_BAYER_BG12:
        {
            cv::Mat image_buffer_Bayer(cv::Size(pFrame->nWidth, pFrame->nHeight), CV_16UC1);

            image_buffer_Bayer.data = (uchar *)pFrame->pImgBuf;
            // Convert to the RGB image
            cv::cvtColor(image_buffer_Bayer, buffer->getSetBuffer(), cv::COLOR_BayerBG2BGR);

            break;
        }
        default:
        {
            printf("Error : PixelFormat of this camera is not supported\n");
            return;
        }
        }

        buffer->swap();
    }
}
GalaxyCamera::GalaxyCamera(int galaxy_camera_index)
{
    open(galaxy_camera_index);
}
GalaxyCamera::~GalaxyCamera()
{
    realease();
}

int GalaxyCamera::open(int galaxy_camera_index)
{
    //初始化库
    status = GXInitLib();
    if (status != GX_STATUS_SUCCESS)
    {
        GX_VERIFY(status);
    }
    //枚举设备列表
    status = GXUpdateDeviceList(&nDeviceNum, 1000);
    if ((status != GX_STATUS_SUCCESS) || (nDeviceNum <= 0))
    {
        GX_VERIFY(status);
    }
    status = GXOpenDeviceByIndex(galaxy_camera_index, &hDevice);

    if (status == GX_STATUS_SUCCESS)
    {
        //设置相机的流通道包长属性,提高网络相机的采集性能
        bool bImplementPacketSize = false;
        uint32_t unPacketSize = 0;
        // // 判断设备是否支持流通道数据包功能
        // status = GXIsImplemented(hDevice, GX_INT_GEV_PACKETSIZE,
        //                          &bImplementPacketSize);
        // if (bImplementPacketSize)
        // {
        //     // 获取当前网络环境的最优包长
        //     status = GXGetOptimalPacketSize(hDevice, &unPacketSize);
        //     // 将最优包长设置为当前设备的流通道包长值
        //     status = GXSetInt(hDevice, GX_INT_GEV_PACKETSIZE, unPacketSize);
        // }
        //注册图像处理回调函数
        status = GXRegisterCaptureCallback(hDevice, (&this->image_buffer_BGR), this->OnFrameCallbackFun);
        //发送开采命令
        status = GXSendCommand(hDevice, GX_COMMAND_ACQUISITION_START);
    }
    return 0;
}

int GalaxyCamera::realease()
{
    //发送停采命令
    status = GXSendCommand(hDevice, GX_COMMAND_ACQUISITION_STOP);
    //注销采集回调
    status = GXUnregisterCaptureCallback(hDevice);

    status = GXCloseDevice(hDevice);
    status = GXCloseLib();
    return 0;
}

bool GalaxyCamera::read(cv::Mat &image)
{
    if (image_buffer_BGR.haveUnread())
    {
        image = image_buffer_BGR.get().clone();
        return true;
    }
    else
        return false;
}

int GalaxyCamera::setCameraParms(cv::String config_file_path)
{
    FileStorage fs(config_file_path, FileStorage::READ);
    //设置连续自动白平衡
    status = GXSetEnum(hDevice, GX_ENUM_BALANCE_WHITE_AUTO,
                       GX_BALANCE_WHITE_AUTO_CONTINUOUS);
}

int GalaxyCamera::setGalaxyParmInt(GX_FEATURE_ID_CMD parms_id, int64_t parms_value)
{
    GX_INT_RANGE range;
    GXGetIntRange(hDevice, parms_id, &range);
    if (parms_value < range.nMin || parms_value > range.nMax)
        std::cout << "parms " << parms_id << " out of range" << std::endl;
    else
    {
        GXSetInt(hDevice, parms_id, parms_value);
    }
}

int GalaxyCamera::setGalaxyParmFloat(GX_FEATURE_ID_CMD parms_id, double parms_value)
{
    GX_FLOAT_RANGE range;
    GXGetFloatRange(hDevice, parms_id, &range);
    if (parms_value < range.dMin || parms_value > range.dMax)
        std::cout << "parms " << parms_id << " out of range" << std::endl;
    else
    {
        GXSetFloat(hDevice, parms_id, parms_value);
    }
}

int GalaxyCamera::setAutoWhiteBalance(GX_BALANCE_WHITE_AUTO_ENTRY AWB_mode, GX_AWB_LAMP_HOUSE_ENTRY AWB_entry)
{
    //设置自动白平衡光照环境,比如当前相机所处环境为荧光灯
    status = GXSetEnum(hDevice, GX_ENUM_AWB_LAMP_HOUSE,
                       AWB_entry);
    //设置连续自动白平衡
    status = GXSetEnum(hDevice, GX_ENUM_BALANCE_WHITE_AUTO,
                       AWB_mode);
}

int GalaxyCamera::setAutoExposure(bool auto_exposure, bool once, float exposure_time_min, float exposure_time_max)
{

    //设置连续自动曝光
    if (auto_exposure)
    {
        GX_FLOAT_RANGE shutterRange;

        status = GXGetFloatRange(hDevice, GX_FLOAT_EXPOSURE_TIME,
                                 &shutterRange);

        if (exposure_time_min < 0)
            //设置最小曝光值
            status = GXSetFloat(hDevice, GX_FLOAT_AUTO_EXPOSURE_TIME_MIN,
                                shutterRange.dMin);
        else
            setGalaxyParmFloat(GX_FLOAT_AUTO_EXPOSURE_TIME_MIN, exposure_time_min);

        if (exposure_time_max < 0)
            //设置最大曝光值
            status = GXSetFloat(hDevice, GX_FLOAT_AUTO_EXPOSURE_TIME_MAX,
                                shutterRange.dMax);
        else
            setGalaxyParmFloat(GX_FLOAT_AUTO_EXPOSURE_TIME_MAX, exposure_time_max);

        if (once)
            status = GXSetEnum(hDevice, GX_ENUM_EXPOSURE_AUTO,
                               GX_EXPOSURE_AUTO_ONCE);
        else

            status = GXSetEnum(hDevice, GX_ENUM_EXPOSURE_AUTO,
                               GX_EXPOSURE_AUTO_CONTINUOUS);
    }
    else
        status = GXSetEnum(hDevice, GX_ENUM_EXPOSURE_AUTO,
                           GX_EXPOSURE_AUTO_OFF);
}

int GalaxyCamera::setExposureTime(double exposure_time)
{
    //设置曝光值
    setGalaxyParmFloat(GX_FLOAT_EXPOSURE_TIME, exposure_time);
}
int GalaxyCamera::setAutoGain(bool auto_gain, bool once, float gain_min, float gain_max)
{
    //选择增益通道类型
    status = GXSetEnum(hDevice, GX_ENUM_GAIN_SELECTOR,
                       GX_GAIN_SELECTOR_ALL);
    //status = GXSetEnum(hDevice, GX_ENUM_GAIN_SELECTOR,
    //  GX_GAIN_SELECTOR_RED);
    //status = GXSetEnum(hDevice, GX_ENUM_GAIN_SELECTOR,
    //  GX_GAIN_SELECTOR_GREEN);
    //status = GXSetEnum(hDevice, GX_ENUM_GAIN_SELECTO R,
    //  GX_GAIN_SELECTOR_BLUE);

    //设置连续自动曝光
    if (auto_gain)
    {
        if (once)
            status = GXSetEnum(hDevice, GX_ENUM_GAIN_AUTO,
                               GX_GAIN_AUTO_ONCE);
        else

            status = GXSetEnum(hDevice, GX_ENUM_GAIN_AUTO,
                               GX_GAIN_AUTO_CONTINUOUS);

        GX_FLOAT_RANGE gainRange;

        status = GXGetFloatRange(hDevice, GX_FLOAT_GAIN,
                                 &gainRange);

        if (gain_min < 0)
            //设置最小曝光值
            status = GXSetFloat(hDevice, GX_FLOAT_AUTO_EXPOSURE_TIME_MIN,
                                gainRange.dMin);
        else
            setGalaxyParmFloat(GX_FLOAT_AUTO_EXPOSURE_TIME_MIN, gain_min);
        if (gain_max < 0)
            //设置最大曝光值
            status = GXSetFloat(hDevice, GX_FLOAT_AUTO_EXPOSURE_TIME_MAX,
                                gainRange.dMax);
        else
            setGalaxyParmFloat(GX_FLOAT_AUTO_EXPOSURE_TIME_MAX, gain_max);
    }
    else
        status = GXSetEnum(hDevice, GX_ENUM_GAIN_AUTO,
                           GX_GAIN_AUTO_OFF);
}

int GalaxyCamera::setGain(double gain)
{
    //选择增益通道类型
    status = GXSetEnum(hDevice, GX_ENUM_GAIN_SELECTOR,
                       GX_GAIN_SELECTOR_ALL);
    //status = GXSetEnum(hDevice, GX_ENUM_GAIN_SELECTOR,
    //  GX_GAIN_SELECTOR_RED);
    //status = GXSetEnum(hDevice, GX_ENUM_GAIN_SELECTOR,
    //  GX_GAIN_SELECTOR_GREEN);
    //status = GXSetEnum(hDevice, GX_ENUM_GAIN_SELECTO R,
    //  GX_GAIN_SELECTOR_BLUE);
    //获取增益调节范围
    GX_FLOAT_RANGE gainRange;
    status = GXGetFloatRange(hDevice, GX_FLOAT_GAIN, &gainRange);
    //设置最小增益值
    status = GXSetFloat(hDevice, GX_FLOAT_GAIN, gainRange.dMin);
    //设置最大增益值
    status = GXSetFloat(hDevice, GX_FLOAT_GAIN, gainRange.dMax);
}