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
    OSCMessenger() { OSCMessenger(0,0); }
    OSCMessenger(int sendPort, int listenPort);
    int sendMessagesForParticle(Particle &p);
    int sendMessageForBang(float scale);
    int sendSeed(int s);
    
    void setVideoSize(const ci::Vec2f videoSize);
    void setChord(int s);
    
    bool hasWaitingMessages() { return this->listener.hasWaitingMessages(); }
    ci::osc::Message getNextMessage() { ci::osc::Message m; this->listener.getNextMessage(&m); return m; }
    
  private:
    int sendPort, listenPort;
    std::vector<int> chord;
    
    ci::osc::Sender sender;
    ci::osc::Listener listener;
    ci::osc::Listener listener2;
    
    
    ci::Vec2f videoSize;
};