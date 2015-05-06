//
//  Balloon.h
//  ConfettiVision
//
//  Created by Disk Cactus on 5/5/15.
//
//

#pragma once

#include <memory>
#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "CinderOpenCV.h"

#include "ParticleCollection.h"
#include "OSCMessenger.h"

class Balloon;
typedef std::shared_ptr<Balloon> BalloonRef;

class Balloon {
    
  public:
    // Smart pointer constructors
    static BalloonRef create() { return std::make_shared<Balloon>(); }
    static BalloonRef create(std::vector<cv::Mat> frames) { return std::make_shared<Balloon>(frames); }
    
  public:
    // Constructors
    Balloon();
    Balloon(std::vector<cv::Mat> frames);
    ~Balloon();

    void init();
    void reset();
    void play();
    void pause();
    
    void setSpeed(int speed) { this->playbackRate = speed; }
    void update();
    bool isReady() { return this->loaded && !this->loading; }
    bool loadMovieFile(const ci::fs::path &path);
    void useFrames(std::vector<cv::Mat> frames) { this->frames = frames; }

    std::vector<cv::Mat> getFrames() { return this->frames; }
    ci::gl::Texture getTexture() { return this->currentFrameTexture; }
    cv::Mat getMat() { return this->currentFrameMat; }
    ci::Surface getSurface() { return this->currentFrameSurface; }
    
    OSCMessenger* messenger;
    std::map<ParticleColor, ParticleCollectionRef> particleCollections;
    
  private:
    void setUniversalConstants();
    ci::gl::Texture processFrame(ci::gl::Texture& frame, ParticleColor color);
    
  private:
    static int  last_balloon_ID;
    
    bool        poppedYet = false;
    int         poppedStartThreshold = 3000;
    int         poppedEndThreshold = 7000;
    int         largestBalloonSize = 0;
    int         ID;
    
    int         playbackRate = 1;
    int         currentFrameNumber = 0;
    bool        loop = true;
    bool        paused = false;
    bool        loaded = false;
    bool        loading = false;
    
    int         message_per_color_limit = 30;
    
    std::vector<cv::Mat> frames;
    cv::Mat              currentFrameMat;
    ci::Surface          currentFrameSurface;
    ci::gl::Texture      currentFrameTexture;
    cv::VideoCapture     cvMovie;
    std::thread          loadMovieThread;
    
    ci::gl::GlslProgRef     rgb2hlsShader;
    ci::gl::Fbo             hlsFramebuffer;
    
    // Musical paramaters chosen at explosion time
    std::vector<int>    chord;
    int                 basePitch;
    int                 giggleFileIndex;
    int                 popFileIndex;
};