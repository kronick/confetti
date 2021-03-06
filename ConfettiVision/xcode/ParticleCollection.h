//
//  ParticleCollection.h
//  ConfettiVision
//
//  Created by Sam Kronick on 4/23/15.
//
//

#pragma once
#include <memory>
#include <algorithm>
#include "cinder/Cinder.h"
#include "Particle.h"
#include "CinderOpenCv.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include <time.h>

class ParticleCollection;
typedef std::shared_ptr<ParticleCollection> ParticleCollectionRef;

struct ColorThreshold {
    int h_min = 0;
    int h_max = 179;
    int s_min = 54;
    int s_max = 255;
    int l_min = 0;
    int l_max = 150;
};

class ParticleCollection {
  public:
    // Smart Pointer Constructors
    static ParticleCollectionRef create(ParticleColor color) { return std::make_shared<ParticleCollection>(color); }
    
  public:
    // Constructors
    ParticleCollection() {}
    ParticleCollection(ParticleColor color);
    
    void resetParticles();
    
    void findParticles(const cv::Mat& frame);
    int getParticleCount() { return this->particles.size(); }
    int getActiveParticleCount();
    std::vector<ParticleRef> getParticles() { return this->particles; };
    ci::ImageSourceRef getDebugView() { return ci::fromOcv(this->debugView); };
    void setDebugView(bool v) { this->debugOn = v; };
    void setDebugColor(int r, int g, int b);
    void setThresholds(int h_min, int h_max, int l_min, int l_max, int s_min, int s_max) {
        this->threshold.h_min = h_min;
        this->threshold.h_max = h_max;
        this->threshold.l_min = l_min;
        this->threshold.l_max = l_max;
        this->threshold.s_min = s_min;
        this->threshold.s_max = s_max;
    }
    
  public:
    ColorThreshold          threshold;
    ParticleColor           color;
    int                     particleAreaMin = 40;   // 20
    int                     particleAreaMax = 3000; // 1200
    float                   particleSeparationThreshold = 1000;
    int                     largestParticleArea;
    
    int                   deadZoneTop = 48;
    
    std::vector<ParticleRef>   movedParticles;
    std::vector<ParticleRef>   newParticles;
  
  private:
    std::vector<ParticleRef>   particles;
    
    cv::Mat                 debugView;
    bool                    debugOn = false;
    cv::Scalar              debugColor;
};
