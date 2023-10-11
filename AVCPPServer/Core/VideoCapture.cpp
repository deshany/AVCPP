//
// Created by deshany on 2023/10/11.
//

#include "VideoCapture.h"
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <iostream>
#include <cstring>
#include <sys/mman.h>

CVideoCapture::CVideoCapture(){
    m_nDevFd = -1;
}

CVideoCapture::~CVideoCapture() = default;

bool CVideoCapture::OpenVideoDevice(const char *strDevName) {
    if(strDevName == nullptr){
        return false;
    }

    m_nDevFd = open(strDevName, O_RDWR);
    if(m_nDevFd < 0){
        perror("打开设备失败");
        return false;
    }
    return true;
}

void CVideoCapture::CloseVideoDevice() const {
    close(m_nDevFd);
}

void CVideoCapture::GetDeviceSupportFormat() const {
    struct v4l2_fmtdesc v4Fmt{};
    v4Fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int i = 0, nRet;
    while(true)
    {
        v4Fmt.index = i++;
        nRet = ioctl(m_nDevFd, VIDIOC_ENUM_FMT, &v4Fmt);
        if(nRet < 0)
        {
            perror("获取结束或失败");
            break;
        }
        std::cout << "index:" << v4Fmt.index << std::endl;
        std::cout << "flags:" << v4Fmt.flags << std::endl;
        std::cout << "description:" << v4Fmt.description << std::endl;
        std::cout << "pixel-format:" << v4Fmt.pixelformat << std::endl;
    }
}

bool CVideoCapture::SetCapturePixelFormat(unsigned int nWidth, unsigned  int nHeight) const {
    struct v4l2_format v4Fmt{};
    v4Fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4Fmt.fmt.pix.width = nWidth;
    v4Fmt.fmt.pix.height = nHeight;
    v4Fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    int nRet = ioctl(m_nDevFd, VIDIOC_S_FMT, &v4Fmt);
    if(nRet < 0){
       perror("设置格式失败");
       return false;
    }

    memset(&v4Fmt, 0, sizeof(v4Fmt));
    v4Fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    nRet = ioctl(m_nDevFd, VIDIOC_G_FMT, &v4Fmt);
    if(nRet < 0){
        perror("获取格式失败");
        return false;
    }
    if(v4Fmt.fmt.pix.width == nWidth && v4Fmt.fmt.pix.height == nHeight)
    {
        std::cout << "设置格式成功" << std::endl;
        return false;
    }
    return true;
}

bool CVideoCapture::RequestKernelBufferAndMapToUserSpace() {
    struct v4l2_requestbuffers reqbuffer{};
    reqbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuffer.count = 4;                 // 申请4个缓冲区
    reqbuffer.memory = V4L2_MEMORY_MMAP ;// 映射方式
    int nRet  = ioctl(m_nDevFd, VIDIOC_REQBUFS, &reqbuffer);
    if(nRet < 0)
    {
        perror("申请队列空间失败");
        return false;
    }

    struct v4l2_buffer mapBuffer{};
    mapBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    for(int i = 0; i < 4; ++i)
    {
        mapBuffer.index = i;
        nRet = ioctl(m_nDevFd, VIDIOC_QUERYBUF, &mapBuffer);
        if(nRet < 0){
            perror("查询内核空间队列失败");
        }
        m_pMapPtr[i] = (unsigned char*)mmap(nullptr, mapBuffer.length, PROT_READ|PROT_WRITE,
                                            MAP_SHARED, m_nDevFd, mapBuffer.m.offset );
        m_nSize[i] = mapBuffer.length;
        nRet = ioctl(m_nDevFd, VIDIOC_QBUF, &mapBuffer);
        if(nRet < 0){
            perror("放回失败");
        }
    }
    return true;
}

void CVideoCapture::StartCapture() const {
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int nRet = ioctl(m_nDevFd, VIDIOC_STREAMON, &type);
    if(nRet < 0)
    {
        perror("开启失败");
    }
}

void CVideoCapture::StopCapture() const {
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int nRet = ioctl(m_nDevFd, VIDIOC_STREAMOFF, &type);
    if(nRet < 0)
    {
        perror("关闭失败");
    }
}

bool CVideoCapture::CaptureOneFrame() {
   struct v4l2_buffer readBuffer{};
   readBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   int nRet = ioctl(m_nDevFd, VIDIOC_DQBUF, &readBuffer);
   if(nRet < 0)
   {
       perror("读取数据失败");
       return false;
   }

    FILE* pYUVFile = fopen("test.yuv", "w+");
    fwrite(m_pMapPtr[readBuffer.index], readBuffer.length, 1, pYUVFile);
    fclose(pYUVFile);

    //通知内核已经使用完毕
    nRet = ioctl(m_nDevFd, VIDIOC_QBUF, &readBuffer);
    if(nRet < 0)
    {
        perror("放回队列失败");
    }
    return true;
}

void CVideoCapture::ReleaseMap() {
    for (int i = 0; i < 4; i++) {
        munmap(m_pMapPtr[i], m_nSize[i]);
    }
}