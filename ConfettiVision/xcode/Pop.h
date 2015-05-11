//
//  Pop.h
//  ConfettiVision
//
//  Created by Disk Cactus on 5/8/15.
//
//

#pragma once

#include <memory>
#include "cinder/Cinder.h"
#include "cinder/Rand.h"
#include "ColorDefinitions.h"


class Pop;
typedef std::shared_ptr<Pop> PopRef;

class Pop {
    
  public:
    // Smart pointer constructors
    static PopRef create(float x, float y, float size, float r) { return std::make_shared<Pop>(x, y, size, r); }
    
  public:
    Pop(float x, float y, float size, float r);
    void init();
    void update();
    void draw();
    bool isAlive() { return this->age <= this->ageLimit; }
    
  private:
    ci::Vec2f position;
    float rotation;
    float size;
    float spin;

    int age;
    
    int ageLimit = 60;
};


