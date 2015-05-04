//
//  ConfettiVisionApp.h
//  ConfettiVision
//
//  Created by Disk Cactus on 5/1/15.
//
//

#pragma once
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/qtime/QuickTime.h"
#include "CinderOpenCv.h"
#include "OscSender.h"
#include "cinder/params/Params.h"
#include "cinder/System.h"
#include "CinderConfig.h"

#include "RtMidi.h"

#include <thread>

#include "ParticleCollection.h"
#include "CameraCapture.h"

enum class Mode { CAPTURE, REVIEW, PLAYBACK };

class ConfettiVisionApp : public ci::app::AppNative {
public:
    void setup();
    void prepareSettings(Settings *settings);
    void setupParams();
    void mouseDown( ci::app::MouseEvent event );
    void mouseMove(ci::app::MouseEvent event);
    void keyDown( ci::app::KeyEvent event );
    void update();
    void updateVideoPlayback();

    ci::gl::Texture processFrame(ci::gl::Texture& frame, ParticleCollectionRef particleCollection);
    
    void draw();
    void drawPlayback();
    void drawCapture();
    void drawReview();
    
    void fileDrop( ci::app::FileDropEvent event );
    void loadMovieFile( const ci::fs::path &path );
    void resetPop();
    
    void drawDebugString(const std::string &str, const ci::Vec2f &baseline);
    void setMode(Mode m);
    
    void sendMessagesForParticle(Particle &p, int channel);
    
    cv::Mat getHLSImage(ci::Surface surface);

public:
    ci::gl::Texture         mFrameTexture;
    ci::gl::TextureRef      cameraPreview;
    ci::Surface             mFrameSurface;
    ci::gl::Fbo             hlsFramebuffer;
    
    ci::gl::GlslProgRef     rgb2hlsShader;
    
    //qtime::MovieGlRef   mMovie;
    //qtime::MovieSurfaceRef mMovie;
    cv::VideoCapture    cvMovie;
    ci::Font debugFont;
    ci::gl::TextureFontRef debugTextureFont;
    
    ci::params::InterfaceGlRef  params;
    ci::config::ConfigRef       config;
    std::string                 configFilename;
    
    std::vector<ci::fs::path> mMoviePaths;
    
    ParticleCollectionRef redParticles, greenParticles, blueParticles, yellowParticles;
    
    ci::osc::Sender oscSender;
    
    CameraCapture camera;
    
    bool poppedYet = false;
    int poppedStartThreshold = 5;
    int poppedEndThreshold = 3;
    
    float paramExposure;
    float paramGain;
    float paramGammaY;
    float paramGammaC;
    int paramFramerate = 400;
    float paramWB_r, paramWB_g, paramWB_b;
    float paramSharpness;
    bool paramHDR = false;
    int paramHDRExposure = 60;
    int paramHDRKneepoint = 40;
    
    bool isPlaybackPlaying = false;
    int playbackSpeed = 1;
    
    Mode currentMode = Mode::PLAYBACK;
    
    ci::Vec2f mousePosition;

    
};