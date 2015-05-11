//
//  SynthLine.h
//  ConfettiVision
//
//  Created by Disk Cactus on 5/8/15.
//
//

#pragma once

#include <memory>
#include "cinder/Cinder.h"


class SynthLine;
typedef std::shared_ptr<SynthLine> SynthLineRef;

class SynthLine {
    
public:
    // Smart pointer constructors
    static SynthLineRef create(float x, float y, float size, float r) { return std::make_shared<SynthLine>(x, y, size, r); }
    
public:
    SynthLine(float x, float y, float size, float r);
    void init();
    void update();
    void draw();
    
private:
    ci::Vec2f position;
    float rotation;
    int age;
    
    int ageLimit;
};


