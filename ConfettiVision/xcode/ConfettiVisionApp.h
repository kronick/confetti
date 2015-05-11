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
#include "OscListener.h"
#include "cinder/params/Params.h"
#include "cinder/System.h"
#include "CinderConfig.h"

#include <thread>

#include "ParticleCollection.h"
#include "OSCMessenger.h"
#include "CameraCapture.h"
#include "Balloon.h"

enum class Mode { CAPTURE, REVIEW, PLAYBACK };

class ConfettiVisionApp : public ci::app::AppNative {
public:
    void setup();
    void prepareSettings(Settings *settings);
    void setupParams();
    void mouseDown( ci::app::MouseEvent event );
    void mouseMove(ci::app::MouseEvent event);
    void keyDown( ci::app::KeyEvent event );
    
    void messageReceived(const ci::osc::Message *m);
    
    void update();
    
    void draw();
    void drawPlayback();
    void drawCapture();
    void drawReview();
    
    void fileDrop( ci::app::FileDropEvent event );
    void loadMovieFile( const ci::fs::path &path );
    
    void drawDebugString(const std::string &str, const ci::Vec2f &baseline);
    void drawString(const std::string &str, const ci::Vec2f &baseLine, int fontSize);
    void setMode(Mode m);
    void setCalibrate(bool c);
    
    void triggerAndCreateBalloon();
    void setThresholds();
    

public:
    ci::gl::Texture         mFrameTexture;
    ci::gl::TextureRef      cameraPreview;
    ci::Surface             mFrameSurface;
    
    
    ci::Font debugFont;
    ci::gl::TextureFontRef debugTextureFont;
    std::map<int, ci::gl::TextureFontRef> textureFonts;

    
    ci::params::InterfaceGlRef  params;
    ci::config::ConfigRef       config;
    std::string                 configFilename;
    
    std::vector<ci::fs::path> mMoviePaths;
    
    //ci::osc::Listener oscListener;
    int currentCalibrationColor = 0;
    
    CameraCapture camera;
    OSCMessenger messenger;
    ci::osc::Listener controlListener;
    BalloonRef balloon;
    
    
    bool paramCalibrate = false;
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
    ColorThreshold paramThresholdR;
    ColorThreshold paramThresholdG;
    ColorThreshold paramThresholdB;
    ColorThreshold paramThresholdY;
    
    int videoWidth = 640;
    int videoHeight = 480;
    
    bool isPlaybackPlaying = false;
    
    Mode currentMode = Mode::PLAYBACK;
    
    ci::Vec2f mousePosition;
};
