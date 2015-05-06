//
//  OSCMessenger.h
//  ConfettiVision
//
//  Created by Disk Cactus on 5/5/15.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "OscSender.h"
#include "OscListener.h"
#include "Particle.h"

class OSCMessenger {
  public:
    OSCMessenger() { OSCMessenger(3000); }
    OSCMessenger(int port);
    int sendMessagesForParticle(Particle &p);
    int sendMessageForBang(float scale);
    
    void setVideoSize(const ci::Vec2f videoSize);
    void setChord(const std::vector<int> chord);
    
  private:
    int port;
    std::vector<int> chord;
    
    ci::osc::Sender sender;
    ci::osc::Listener listener;
    
    
    ci::Vec2f videoSize;
};