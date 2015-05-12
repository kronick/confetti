//
//  BassString.cpp
//  ConfettiVision
//
//  Created by Disk Cactus on 5/8/15.
//
//

#include "BassString.h"

using namespace ci;

BassString::BassString(float pitch) {
    this->pitch = pitch * 3;
}

void BassString::update() {
    this->age++;

    
    //this->amplitude = 10.0f;
}

void BassString::draw(float x, float y, float w, float h) {
    gl::lineWidth(w * 2.0f);
    
    float amp;
    if(this->age - this->pluckedAt < this->pluckDuration && this->age > this->pluckDuration)
        amp = (1.0f - (float)(this->age - this->pluckedAt) / (float)this->pluckDuration);
    else
        amp = 0;
    
    amp *= this->amplitude;
    // Build a polyline
    this->path.clear();
    this->path.moveTo(x,y);
    for(int i=0; i<=h; i++) {
        this->path.lineTo(
            x + (w * amp / 2.0f) * cos((float)(i + this->age*20)/(float)h * 6 * M_PI * this->pitch / 5.0f),
            i + y);
    }
    //gl::drawLine(Vec2f(x,y), Vec2f(x, y+h));
    
    gl::draw(this->path);
    
}

void BassString::pluck(float amp) {
    this->pluckedAt = this->age;
    this->amplitude = amp;
}