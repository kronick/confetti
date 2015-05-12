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
#include "cinder/audio/SamplePlayerNode.h"
#include "cinder/audio/Source.h"
#include "ColorDefinitions.h"


class Waveform;
typedef std::shared_ptr<Waveform> WaveformRef;

class Waveform {
    
public:
    // Smart pointer constructors
    static WaveformRef create(ci::DataSourceRef file) { return std::make_shared<Waveform>(file); }
    
public:
    Waveform(ci::DataSourceRef file);
    void init();
    void update();
    void draw(float w, float h);
    void highlight(float start, float end, float volume);
    
private:
    ci::Vec2f position;
    float rotation;
    int age;
    
    int start, end;
    ci::audio::BufferRef sampleBuffer;
    float volume;
    int lastUpdate;
    
    int ageLimit;
};


