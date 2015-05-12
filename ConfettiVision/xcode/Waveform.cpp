//
//  Waveform.cpp
//  ConfettiVision
//
//  Created by Disk Cactus on 5/8/15.
//
//

#include "Waveform.h"

using namespace std;
using namespace ci;

Waveform::Waveform(DataSourceRef file) {
    audio::SourceFileRef sourceFile = audio::SourceFile::create(file);
    this->sampleBuffer = sourceFile->loadBuffer();
}

void Waveform::init() {
    
}
void Waveform::update() {
    this->age++;
}

void Waveform::highlight(float start, float end, float volume) {
    start = min(start, 1.0f);
    start = max(start, 0.0f);
    end = min(end, 1.0f);
    end = max(end, 0.0f);
    this->start = start * (this->sampleBuffer->getNumFrames()-1);
    this->end = end * (this->sampleBuffer->getNumFrames()-1);
    this->volume = volume;

    this->lastUpdate = this->age;
}

void Waveform::draw(float w, float h) {
    this->update();
    
    // Set alpha
    float alpha = 1 - ((this->age - this->lastUpdate) / 10.0f);
    gl::color(COLOR_WHITE.r, COLOR_WHITE.g, COLOR_WHITE.b, alpha);
    
    Path2d path;
    float m = h/2;
    path.moveTo(0, (*(this->sampleBuffer))[this->start] * h/2 + m);
    int dir = this->end > this->start ? 1 : -1;
    int width = dir * (this->end - this->start);
    int step = 16;
    // Draw two sets of lines
    for(int i=this->start; i<this->end; i += step * dir) {
        path.lineTo((float)(i-this->start) / width * w, m + abs((*(this->sampleBuffer))[i]) * h/2 * this->volume);
    }
    for(int i=this->end - step; i>=this->start; i -= step * dir) {
        path.lineTo((float)(i-this->start) / width * w, m - abs((*(this->sampleBuffer))[i]) * h/2 * this->volume);
    }
    gl::drawSolid(path);
}