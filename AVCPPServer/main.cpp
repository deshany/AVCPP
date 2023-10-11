#include "Core/VideoCapture.h"

void TestVideoCap(){

    CVideoCapture VC;
    VC.OpenVideoDevice("/dev/video0");
//    VC.GetDeviceSupportFormat();
    VC.SetCapturePixelFormat(1280, 720);
    VC.RequestKernelBufferAndMapToUserSpace();
    VC.StartCapture();
    VC.CaptureOneFrame();
    VC.StopCapture();
    VC.ReleaseMap();
    VC.CloseVideoDevice();

}
int main() {
    TestVideoCap();
    return 0;
}
