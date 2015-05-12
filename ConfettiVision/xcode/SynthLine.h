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

struct SynthPitchBlock {
    int startTime;
    int endTime;
    float pitch;
    float volume;
};
struct SynthPitchBend {
    int startTime;
    int endTime;
    float startPitch;
    float endPitch;
    float volume;
};

class SynthLine {

public:
    // Smart pointer constructors
    static SynthLineRef create(int pitch, int octaves, float volume) { return std::make_shared<SynthLine>(pitch, octaves, volume); }
    
public:
    SynthLine(int note, int octaves, float volume);
    void init();
    void update();
    void draw(float w, float h);
    void addPitch(int pitch, float volume);
    void changePitch(int toPitch, int time, float volume);
    void stop();
    bool isAlive() { return this->alive; }
    
private:
    ci::Vec2f position;
    std::deque<SynthPitchBlock> pitchBlocks;
    std::deque<SynthPitchBend> pitchBends;
    int age = 0;
    int octaves;
    float currentPitch;
    int stopAge;
    int lastUpdate;
    
    bool alive = true;
    bool mute = false;
    int ageLimit;
};


