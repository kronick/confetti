//
//  BassString.h
//  ConfettiVision
//
//  Created by Disk Cactus on 5/8/15.
//
//

#pragma once

#include <memory>
#include "cinder/Cinder.h"


class BassString;
typedef std::shared_ptr<BassString> BassStringRef;

class BassString {
    
public:
    // Smart pointer constructors
    static BassStringRef create(float pitch) { return std::make_shared<BassString>(pitch); }
    
public:
    BassString(float pitch);
    void init();
    void update();
    void draw(float x, float y, float w, float h);
    void pluck(float amp);
    
    
private:
    float pitch;
    ci::Vec2f position;
    ci::Vec2f size;
    float amplitude;
    
    int age;
    int pluckedAt;
    int pluckDuration = 60;
    ci::Path2d path;
};
