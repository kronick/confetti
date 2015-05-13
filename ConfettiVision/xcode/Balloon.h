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
#include "cinder/Rand.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "CinderOpenCV.h"

#include "ParticleCollection.h"
#include "OSCMessenger.h"

#include "ColorDefinitions.h"

#include "Pop.h"
#include "BassString.h"
#include "Waveform.h"
#include "SynthLine.h"


class ConfettiVisionApp;
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
    void updateGraphics();
    //bool isReady() { return this->loaded && !this->loading; }
    bool isReady() { return this->loaded  || this->loading; }
    bool hasPopped() { return this->poppedYet; }
    bool hasLooped() { return this->loopedYet; }
    bool loadMovieFile(const ci::fs::path &path);
    void setFrames(std::vector<cv::Mat> frames);
    void seekFrame(int f) { if(f < this->frames.size() && f >= 0) this->currentFrameNumber = f;}
    
    void setDebug(bool d);
    bool getDebug() { return this->debugMode; }
    void drawMusicGraphics(float width, float height);
    void drawChannelGraph(ParticleColor color, float width, float height);
    void drawParticles(ParticleColor color, ci::Vec2f size);
    
    std::vector<cv::Mat> getFrames() { return this->frames; }
    ci::gl::Texture getTexture() { return this->currentFrameTexture; }
    cv::Mat getMat() { return this->currentFrameMat; }
    ci::Surface getSurface() { return this->currentFrameSurface; }
    int getID() { return this->ID; }
    int getFramecount() { return this->frameCount; }
    int getCurrentFrameNumber() { return this->currentFrameNumber; }
    int getCurrentSpeed() { return this->playbackRate; }
    int getPoppedFrame() { return this->poppedFrame; }
    
    OSCMessenger* messenger;
    std::map<ParticleColor, ParticleCollectionRef> particleCollections;
    
    ConfettiVisionApp * parent;
    
  private:
    void setUniversalConstants();
    ci::gl::Texture processFrame(ci::gl::Texture& frame, ParticleColor color);
    
  private:
    static int  last_balloon_ID;
    
    bool        debugMode = false;
    bool        poppedYet = false;
    bool        seenABalloon = false;
    int         poppedFrame = -1;
    int         poppedStartThreshold = 3000;
    int         poppedEndThreshold = 7000;
    int         largestBalloonSize = 0;
    int         ID;
    
    int         playbackRate = 1;
    int         currentFrameNumber = 0;
    bool        loop = true;
    bool        loopedYet = false;
    bool        paused = false;
    bool        loaded = false;
    bool        loading = false;
    int         speed_forward = 13;
    int         speed_rewind = -13;
    
    int         message_per_color_limit = 500;
    
    std::vector<cv::Mat> frames;
    int                  frameCount;
    cv::Mat              currentFrameMat;
    ci::Surface          currentFrameSurface;
    ci::gl::Texture      currentFrameTexture;
    cv::VideoCapture     cvMovie;
    std::thread          loadMovieThread;
    
    ci::gl::GlslProgRef     rgb2hlsShader;
    ci::gl::Fbo             hlsFramebuffer;
    
    // Musical paramaters chosen at explosion time
    std::vector<int>    chord;
    
    // Drawables for visualization
    float drawWidth, drawHeight;
    std::vector<PopRef>             pops;
    std::vector<BassStringRef>      bassStrings;
    WaveformRef                     waveform;
    std::map<int, SynthLineRef>     synthLines;

};