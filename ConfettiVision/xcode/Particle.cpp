//
//  Particle.cpp
//  ConfettiVision
//
//  Created by Sam Kronick on 4/23/15.
//
//

#include "Particle.h"
int Particle::last_particle_id;

Particle::Particle() {
    this->init();
}

Particle::Particle(float x, float y) {
    this->init();
    this->position = ci::Vec2f(x,y);
}

Particle::Particle(const ci::Vec2f &position) {
    this->init();
    this->position = position;
}

void Particle::init() {
    this->ID = Particle::last_particle_id++;
    
    this->position = ci::Vec2f(0,0);
    this->velocity = ci::Vec2f(0,0);

    for(int i=0; i<Particle::velocityHistoryLength; i++) {
        this->velocityHistory.push_back(ci::Vec2f(0,0));
    }
}


void Particle::update() {
    this->age++;
    this->freshness -= this->decayRate;
    if(this->freshness < 0) this->freshness = 0;
    
    // Calculate average velocity
    if(this->velocityHistory.size() > 0) {
        ci::Vec2f v_avg(0,0);
        for(auto &v : this->velocityHistory) {
            v_avg += v;
        }
        this->velocity = v_avg / this->velocityHistory.size();
        this->position += velocity * freshness;
        this->positionHistory.push_back(ci::Vec2f(this->position));
    }
}

bool Particle::isActive() {
    //return (this->age > 36)  || ((this->age / (float)this->positionUpdates) < 1.5f);
    return (this->age > 30) || ((this->age / (float)this->positionUpdates) < 1.5f);
}

void Particle::setPosition(float x, float y) {
    this->setPosition(ci::Vec2f(x,y));
}

void Particle::setPosition(const ci::Vec2f &position) {
    this->velocityHistory.push_back(position - this->position);
    if(this->velocityHistory.size() > Particle::velocityHistoryLength)
        this->velocityHistory.pop_front();
    
    this->position = position;
    this->freshness = 1;
    this->positionUpdates++;

}