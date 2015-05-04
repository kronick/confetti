//
//  CameraCapture.h
//  ConfettiVision
//
//  Created by Disk Cactus on 4/30/15.
//
//

#pragma once
#include <m3api/xiApi.h>
#include <memory>
#include <algorithm>
#include "cinder/Cinder.h"
#include "cinder/gl/Texture.h"
#include "Particle.h"
#include "CinderOpenCv.h"
#include <time.h>
#include <chrono>
#include <ctime>

enum class CaptureMode { BUFFER, TRIGGER, REST };

class CameraCapture {
  public:
    CameraCapture();
    ~CameraCapture();
    void startStream(int fps);
    void stopStream();
    ci::gl::TextureRef getPreview();
    void startTrigger();
    void stopTrigger();
    void saveBuffer(std::string filename);
    std::vector<cv::Mat> getCapturedFrames() { return this->capturedFrames; }

    bool setExposure(int uS);
    bool setGain(float gain);
    bool setFramerate(int fps);
    bool setSharpness(float sharp);
    bool setGammaY(float y);
    bool setGammaC(float c);
    bool setHDR(bool hdr);
    bool setHDRExposure(int exposure);
    bool setHDRKneepoint(int kneepoint);
    
    bool enableManualWhiteBalance();
    bool enableAutoWhiteBalance();
    void setWhiteBalance(ci::Vec3f v);
    
    bool isRecording() { return this->mode == CaptureMode::TRIGGER; }
    ci::Vec3f getWhiteBalance();
    
  private:
    std::list<cv::Mat> preBuffer;
    std::list<cv::Mat> postBuffer;
    cv::Mat            previewImage;
    std::vector<cv::Mat> capturedFrames;
    
    CaptureMode mode = CaptureMode::REST;
    int prebuffer_length = 400;
    
    void captureAsync();
    std::thread captureThread;
    bool isCapturing;
    bool isCameraOpen;
    HANDLE ximeaHandle;
    int framerate = 400;
    
    std::chrono::time_point<std::chrono::system_clock> captureStartTime;
    std::chrono::time_point<std::chrono::system_clock> captureEndTime;
    DWORD captureStartFrame = 0;
    DWORD captureEndFrame = 0;
};