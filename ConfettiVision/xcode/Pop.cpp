//
//  Pop.cpp
//  ConfettiVision
//
//  Created by Disk Cactus on 5/8/15.
//
//

#include "Pop.h"

using namespace ci;
using namespace std;
Pop::Pop(float x, float y, float size, float r) {
    this->position = Vec2f(x,y);
    this->rotation = r;
    this->size = size;
    this->age = 0;
    this->spin = randFloat(-0.1, 0.1);
}

void Pop::update() {
    if(!this->isAlive()) return;
    this->age++;
    this->rotation += this->spin;
    
}

void Pop::draw(float w, float h) {
    gl::SaveColorState save;

    if(!this->isAlive()) return;
    Path2d tri;
    float s = min(this->age / 3.0f * this->size, this->size);
    if(this->ageLimit - this->age < 6) s = (this->ageLimit - this->age) / 6.0f * this->size;
    
    tri.moveTo(Vec2f(this->position.x * w, this->position.y * h) + Vec2f(s * cos(this->rotation), s * sin(this->rotation)));
    tri.lineTo(Vec2f(this->position.x * w, this->position.y * h) + Vec2f(s * cos(this->rotation + M_PI * 2 / 3), s * sin(this->rotation + M_PI * 2 / 3)));
    tri.lineTo(Vec2f(this->position.x * w, this->position.y * h) + Vec2f(s * cos(this->rotation - M_PI * 2 / 3), s * sin(this->rotation - M_PI * 2 / 3)));
    
    gl::drawSolid(tri);
}

