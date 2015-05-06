//
//  Particle.h
//  ConfettiVision
//
//  Created by Sam Kronick on 4/23/15.
//
//

#pragma once

#include <memory>
#include "cinder/Cinder.h"


class Particle;
typedef std::shared_ptr<Particle> ParticleRef;

enum class ParticleColor { RED, BLUE, GREEN, YELLOW };

class Particle {
   
  public:
    // Smart pointer constructors
    static ParticleRef create() { return std::make_shared<Particle>(); }
    static ParticleRef create(int x, int y, ParticleColor c) { return std::make_shared<Particle>(x,y, c); }
    static ParticleRef create(const ci::Vec2f &v, ParticleColor c) { return std::make_shared<Particle>(v, c); }

  public:
    // Constructors
    Particle();
    Particle(float x, float y, ParticleColor c);
    Particle(const ci::Vec2f &position, ParticleColor c);
    
    std::deque<ci::Vec2f> getVelocityHistory() { return this->velocityHistory; }
    
    void init();
    void update();
    bool isAlive() { return freshness > 0; }
    bool isActive();
    
    void setPosition(float x, float y);
    void setPosition(const ci::Vec2f &position);
    
    ParticleColor getColor() { return this->color; }
    

  public:
    ci::Vec2f position;
    ci::Vec2f velocity;
    int age = 0;
    int positionUpdates = 0;
    int ID;
    bool hasNoteBeenSent = false;
    float freshness = 1;
    float decayRate = 0.05;
    
    std::vector<ci::Vec2f> positionHistory;
    
    ParticleColor color;
    
  private:
    static int last_particle_id;
    std::deque<ci::Vec2f> velocityHistory;
    static const int velocityHistoryLength = 50;
};

