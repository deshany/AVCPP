//
// Created by deshany on 2023/10/11.
//

#ifndef AVCPPSERVER_VIDEOCAPTURE_H
#define AVCPPSERVER_VIDEOCAPTURE_H
#include <linux/videodev2.h>

class CVideoCapture {
public:
    CVideoCapture();
    ~CVideoCapture();

public:
    // 打开视频设备
    bool OpenVideoDevice(const char* strDevName);
    // 关闭视频设备
    void CloseVideoDevice() const;
    // 获取设备支持的采集格式
    void GetDeviceSupportFormat() const;
    // 设置采集格式
    bool SetCapturePixelFormat(unsigned int nWidth, unsigned  int nHeight) const;
    // 申请内核缓冲区并映射到用户缓冲区
    bool RequestKernelBufferAndMapToUserSpace();
    // 开始采集
    void StartCapture() const;
    // 采集一帧数据
    bool CaptureOneFrame();
    // 停止采集
    void StopCapture() const;
    // 释放映射
    void ReleaseMap();

private:
    int m_nDevFd;
    unsigned char* m_pMapPtr[4]{};
    unsigned int m_nSize[4]{};
};


#endif //AVCPPSERVER_VIDEOCAPTURE_H
