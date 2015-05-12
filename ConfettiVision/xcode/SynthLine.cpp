//
//  SynthLine.cpp
//  ConfettiVision
//
//  Created by Disk Cactus on 5/8/15.
//
//

#include "SynthLine.h"

using namespace ci;
using namespace std;
SynthLine::SynthLine(int pitch, int octaves, float volume) {
    this->octaves = octaves;
    this->addPitch(pitch, volume);
    this->alive = true;
}

void SynthLine::init() {
    
}

void SynthLine::stop() {
    //this->alive = false;
    if(this->age > 0) this->mute = true;
    this->stopAge = this->age;
    
}

void SynthLine::addPitch(int pitch, float volume) {
    if(this->mute) return;
    
    this->currentPitch = pitch;
    SynthPitchBlock b;
    b.startTime = this->age;
    b.endTime = this->age;
    b.pitch = pitch;
    b.volume = volume;
    
    this->pitchBlocks.push_back(b);
    this->lastUpdate = this->age;
}

void SynthLine::changePitch(int toPitch, int time, float volume) {
    if((int)toPitch == (int)this->currentPitch) return;
    if(this->mute) return;
    
    // Create a new pitch bend
    SynthPitchBend b;
    b.startTime = this->age;
    b.endTime = this->age + time;
    b.startPitch = this->currentPitch;
    b.endPitch = toPitch;
    b.volume = volume;
    
    if(this->pitchBends.size() > 0 && this->pitchBends.back().endTime > this->age) {
        // Change pitch bend start/end to handle an incomplete pitch bend
        int oldDuration = this->pitchBends.back().endTime - this->pitchBends.back().startTime;
        int newDuration = this->age - this->pitchBends.back().startTime;
        float pitchChange = this->pitchBends.back().endPitch - this->pitchBends.back().startPitch;
        this->pitchBends.back().endPitch = newDuration / (float)oldDuration * pitchChange + this->pitchBends.back().startPitch;
        this->currentPitch = this->pitchBends.back().endPitch;
        b.startPitch = this->currentPitch;
        this->pitchBends.back().endTime = this->age;
    }
    this->pitchBends.push_back(b);
    
    // Create a new pitch block ahead of time
    SynthPitchBlock p;
    p.startTime = b.endTime;
    p.pitch = b.endPitch;
    p.endTime = b.endTime;
    p.volume = volume;
    this->pitchBlocks.push_back(p);
    this->currentPitch = p.pitch;
    
    this->lastUpdate = this->age;
    
}

void SynthLine::update() {
    // Move all points to the left n pixels
    this->age++;
    
    // Check if this has been updated in a while
    if(this->age > 120)
        this->mute = true;
    
    // Keep the last pitch playing
    if(!this->mute)
        this->pitchBlocks[this->pitchBlocks.size()-1].endTime = max(this->age, this->pitchBlocks[this->pitchBlocks.size()-1].endTime);
}

void SynthLine::draw(float w, float h) {
    if(!this->isAlive()) return;
    float tickWidth = w / 4.0f;
    for(auto b = this->pitchBlocks.begin(); b != this->pitchBlocks.end();) {
        if(b->endTime < (this->age - tickWidth)) {  // Delete it if it's past the left edge boundary
            b = this->pitchBlocks.erase(b);
        }
        else {
            // DRAW THIS BLOCK
            float y = h - b->pitch / (this->octaves * 6.0f) * h;
            float left = max(0.0f, (1 - (this->age - b->startTime) / tickWidth) * w);
            float right = min(w, (1 - (this->age - b->endTime) / tickWidth) * w);
            float thickness = h/60.0f * b->volume / 4.0f;
            gl::drawSolidRect(Rectf(left, y-thickness, right, y+thickness));
            //gl::drawSolidCircle(Vec2f(left, y), thickness);
            //gl::drawLine(Vec2f(left, y), Vec2f(right, y));
            
            ++b;
        }
    }
    for(auto b = this->pitchBends.begin(); b != this->pitchBends.end();) {
        if(b->endTime < (this->age - tickWidth)) {  // Delete it if it's past the left edge boundary
            b = this->pitchBends.erase(b);
        }
        else {
            // DRAW THIS LINE
            gl::lineWidth(6 * b->volume / 4.0f);
            float start_y = h - b->startPitch / (this->octaves * 6.0f) * h;
            float end_y = h - b->endPitch / (this->octaves * 6.0f) * h;
            float left = (1 - (this->age - b->startTime) / tickWidth) * w;
            float right = (1 - (this->age - b->endTime) / tickWidth) * w;
            gl::drawLine(Vec2f(left, start_y), Vec2f(right, end_y));
            
            ++b;
        }
    }
}