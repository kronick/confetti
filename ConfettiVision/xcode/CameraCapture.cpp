//
//  CameraCapture.cpp
//  ConfettiVision
//
//  Created by Disk Cactus on 4/30/15.
//
//

#include "CameraCapture.h"
using namespace std;

CameraCapture::CameraCapture() {
    this->isCameraOpen = false;
    this->ximeaHandle = NULL;
    XI_RETURN status = XI_OK;
    DWORD dwNumberOfDevices = 0;
    status = xiGetNumberDevices(&dwNumberOfDevices);
    if(status == XI_OK) {
        status = xiOpenDevice(0, &ximeaHandle);
        if(status == XI_OK) {
            this->isCameraOpen = true;
            this->setFramerate(400);
            printf ("Camera successfully opened!\n\n");
        }
        else {
            printf ("Could not open camera device.\n\n");
        }
    }
    else {
        printf("No Camera Found!\n\n");
    }
}
CameraCapture::~CameraCapture() {
    this->stopStream();
}

void CameraCapture::startStream(int fps) {
    this->preBuffer.clear();
    this->postBuffer.clear();
    this->capturedFrames.clear();
    
    this->mode = CaptureMode::BUFFER;
    this->setFramerate(fps);
    
    this->stopStream();
    this->captureThread = std::thread(&CameraCapture::captureAsync, this);
    this->isCapturing = true;
}
void CameraCapture::stopStream() {
    this->isCapturing = false;
    if(this->captureThread.joinable())
        this->captureThread.join();
    
}

ci::gl::TextureRef CameraCapture::getPreview() {

    if(previewImage.rows == 0) return NULL;
    ci::ImageSourceRef img = ci::fromOcv(this->previewImage);
    ci::gl::TextureRef tex = ci::gl::Texture::create(img);
    return tex;
}

void CameraCapture::postTrigger() {
    // Stop capturing and use only the frames in the prebuffer
    this->mode = CaptureMode::REST;
    
    // Build a vector with all the captured frames
    for(auto &f : this->preBuffer)
        this->capturedFrames.push_back(f);
    
    this->preBuffer.clear();
    this->postBuffer.clear();
    this->captureStartFrame = 0;
    this->captureEndFrame = 0;
    
    //this->isCapturing = false;
}

void CameraCapture::startTrigger() {
    this->mode = CaptureMode::TRIGGER;
    this->capturedFrames.clear();
    this->postBuffer.clear();
}

void CameraCapture::stopTrigger() {
    this->mode = CaptureMode::REST;
    DWORD n_frames = this->captureEndFrame - this->captureStartFrame;
    chrono::duration<double> capture_time = this->captureEndTime - this->captureStartTime;
    
    printf("%u frames captured in %f seconds\n", n_frames, capture_time.count());
    printf("Average FPS: %i\n\n", (int)(n_frames / capture_time.count()));
    
    
    // Build a vector with all the captured frames from pre and post buffer
    for(auto &f : this->preBuffer)
        this->capturedFrames.push_back(f);
    for(auto &f : this->postBuffer)
        this->capturedFrames.push_back(f);
    
    this->preBuffer.clear();
    
    this->captureStartFrame = 0;
    this->captureEndFrame = 0;
}

void CameraCapture::saveBuffer(string filename) {
    this->stopStream();

    cv::VideoWriter outputVideo;
    int type = CV_FOURCC('m', 'p', '4', 'v');
    type = CV_FOURCC('j', 'p', 'e', 'g');
    outputVideo.open(filename, type, 24, cv::Size(640, 480), true);
    
    for(auto &f : this->capturedFrames)
        outputVideo << f;
    
    this->startStream(this->framerate);
}

void CameraCapture::captureAsync() {
    // Allocate memory for current frame
    XI_IMG_FORMAT img_format = XI_RGB24;
    XI_IMG frame;
    frame.size = sizeof(XI_IMG);
    frame.bp = NULL;
    frame.bp_size = 0;
    
    
    xiStartAcquisition(this->ximeaHandle);
    xiSetParamInt(this->ximeaHandle, XI_PRM_IMAGE_DATA_FORMAT, img_format);
    
    while(this->isCapturing) {
        xiGetImage(this->ximeaHandle, 5000, &frame);
        cv::Mat cvFrame((int)frame.height, (int)frame.width, CV_8UC3, frame.bp);
        cvFrame = cvFrame.clone();
        
        if(this->mode == CaptureMode::BUFFER) {
            this->preBuffer.push_back(cvFrame);
            if(this->preBuffer.size() > this->prebuffer_length) {
                this->preBuffer.pop_front();
            }
        }
        else if(this->mode == CaptureMode::TRIGGER) {
            if(this->captureStartFrame == 0) {
                // First frame captured. Save the time and frame number
                this->captureStartFrame = frame.nframe;
                this->captureStartTime = chrono::system_clock::now();
            }
            // Update counters for fps calculation
            this->captureEndFrame = frame.nframe;
            this->captureEndTime = chrono::system_clock::now();
            
            this->postBuffer.push_back(cvFrame);
        }
        else if(this->mode == CaptureMode::REST) {
            //
        }
        
        this->previewImage = cvFrame;
    }
    
    xiStopAcquisition(this->ximeaHandle);
}


bool CameraCapture::setExposure(int uS) {
    if(!this->isCameraOpen) return false;
    
    return xiSetParamInt(this->ximeaHandle, XI_PRM_EXPOSURE, uS) == XI_OK;
}

bool CameraCapture::setGain(float gain) {
    if(!this->isCameraOpen) return false;
    
    return xiSetParamFloat(this->ximeaHandle, XI_PRM_GAIN, gain) == XI_OK;
}
bool CameraCapture::setSharpness(float sharp) {
    if(!this->isCameraOpen) return false;
    
    return xiSetParamFloat(this->ximeaHandle, XI_PRM_SHARPNESS, sharp) == XI_OK;
}
bool CameraCapture::setGammaY(float y) {
    if(!this->isCameraOpen) return false;
    
    return xiSetParamFloat(this->ximeaHandle, XI_PRM_GAMMAY, y) == XI_OK;
}
bool CameraCapture::setGammaC(float c) {
    if(!this->isCameraOpen) return false;
    
    return xiSetParamFloat(this->ximeaHandle, XI_PRM_GAMMAC, c) == XI_OK;
}
bool CameraCapture::setHDR(bool hdr) {
    if(!this->isCameraOpen) return false;
    
    bool result = xiSetParamInt(this->ximeaHandle, XI_PRM_HDR, hdr) == XI_OK;
    
    if(hdr)
        xiSetParamInt(this->ximeaHandle, XI_PRM_HDR_KNEEPOINT_COUNT, 1);
    else
        xiSetParamInt(this->ximeaHandle, XI_PRM_HDR_KNEEPOINT_COUNT, 0);
    return result;
}

bool CameraCapture::setHDRExposure(int exposure) {
    if(!this->isCameraOpen) return false;
    
    bool result = xiSetParamInt(this->ximeaHandle, XI_PRM_HDR_T2, exposure) == XI_OK;
    
    return result && this->setFramerate(this->framerate);
}

bool CameraCapture::setHDRKneepoint(int kneepoint) {
    if(!this->isCameraOpen) return false;
    
    bool result = xiSetParamInt(this->ximeaHandle, XI_PRM_KNEEPOINT2, kneepoint) == XI_OK;
    
    return result && this->setFramerate(this->framerate);
}



bool CameraCapture::enableManualWhiteBalance() {
    xiSetParamInt(this->ximeaHandle, XI_PRM_AUTO_WB, 0);
    return xiSetParamInt(this->ximeaHandle, XI_PRM_MANUAL_WB, 1);
}
bool CameraCapture::enableAutoWhiteBalance() {
    xiSetParamInt(this->ximeaHandle, XI_PRM_MANUAL_WB, 0);
    return xiSetParamInt(this->ximeaHandle, XI_PRM_AUTO_WB, 1);
}
ci::Vec3f CameraCapture::getWhiteBalance() {
    float r, g, b;
    xiGetParamFloat(this->ximeaHandle, XI_PRM_WB_KR, &r);
    xiGetParamFloat(this->ximeaHandle, XI_PRM_WB_KG, &g);
    xiGetParamFloat(this->ximeaHandle, XI_PRM_WB_KB, &b);
    
    return ci::Vec3f(r,g,b);
}

void CameraCapture::setWhiteBalance(ci::Vec3f v) {
    xiSetParamInt(this->ximeaHandle, XI_PRM_AUTO_WB, 0);
    xiSetParamFloat(this->ximeaHandle, XI_PRM_WB_KR, v.x);
    xiSetParamFloat(this->ximeaHandle, XI_PRM_WB_KG, v.y);
    xiSetParamFloat(this->ximeaHandle, XI_PRM_WB_KB, v.z);
}

bool CameraCapture::setFramerate(int fps) {
    if(!this->isCameraOpen) return false;
    
    fps *= 1.05;    // Give some time for frame download
    this->framerate = fps;
    this->prebuffer_length = this->prebuffer_seconds * fps;  // Always capture the same amount of time
    return this->setExposure(1000000.0f/fps);
    
//    float min_fps, max_fps;
//    xiGetParamFloat(this->ximeaHandle, XI_PRM_FRAMERATE XI_PRM_INFO_MIN, &min_fps);
//    xiGetParamFloat(this->ximeaHandle, XI_PRM_FRAMERATE XI_PRM_INFO_MAX, &max_fps);
//    ci::app::console() << "Min framerate: " << min_fps << "Max framerate: " << max_fps << endl;
//    
//    if(true || (fps <= max_fps && fps >= min_fps)) {
//        xiSetParamInt(this->ximeaHandle,XI_PRM_ACQ_TIMING_MODE, XI_ACQ_TIMING_MODE_FRAME_RATE);
//        return xiSetParamInt(this->ximeaHandle, XI_PRM_FRAMERATE, fps) == XI_OK;
//    }
//    else {
//        return false;
//    }
}