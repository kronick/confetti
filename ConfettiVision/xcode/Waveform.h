//
//  Waveform.h
//  ConfettiVision
//
//  Created by Disk Cactus on 5/8/15.
//
//

#pragma once

#include <memory>
#include "cinder/Cinder.h"


class Waveform;
typedef std::shared_ptr<Waveform> WaveformRef;

class Waveform {
    
public:
    // Smart pointer constructors
    static WaveformRef create(float x, float y, float size, float r) { return std::make_shared<Waveform>(x, y, size, r); }
    
public:
    Waveform(float x, float y, float size, float r);
    void init();
    void update();
    void draw();
    
private:
    ci::Vec2f position;
    float rotation;
    int age;
    
    int ageLimit;
};


