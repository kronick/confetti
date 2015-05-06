//
//  OSCMessenger.cpp
//  ConfettiVision
//
//  Created by Disk Cactus on 5/5/15.
//
//

#include "OSCMessenger.h"

using namespace ci;
using namespace std;

OSCMessenger::OSCMessenger(int port) {
    this->port = port;
    this->sender.setup("localhost", this->port, false);
    
    // Set default parameters
    this->videoSize = Vec2f(640, 480);
    this->chord = {0, 3, 5, 7, 10};

}

void OSCMessenger::setVideoSize(const Vec2f videoSize) {
    this->videoSize = Vec2f(videoSize.x, videoSize.y);
}

void OSCMessenger::setChord(const vector<int> chord) {
    this->chord = chord;
}

int OSCMessenger::sendMessageForBang(float scale) {
    // Trigger a bang
    osc::Message message;
    message.setAddress("/pop");
    message.addIntArg(0);
    message.addFloatArg(0);
    message.addFloatArg(4);
    message.addFloatArg(.5);
    this->sender.sendMessage(message);
    osc::Message message2;
    message2.setAddress("/pop");
    message2.addIntArg(1);
    message2.addFloatArg(0);
    message2.addFloatArg(2);
    message2.addFloatArg(-.5);
    this->sender.sendMessage(message2);
    
    return 2;
}

int OSCMessenger::sendMessagesForParticle(Particle &p) {
    // Generate OSC message for this particle if it's valid
    // Returns the  number of messages sent
    
    if(!p.isActive()) return 0;
    osc::Message message;
    
    switch(p.getColor()) {
        case ParticleColor::BLUE:
        {
            // Trigger the smooth synth
            if(p.hasNoteBeenSent) {
                // This is an OLD note and Max should already know its ID
                if(p.age % 10 != 0) return 0;
                message.setAddress("/synth/" + to_string(p.ID));
            }
            else {
                // This is a NEW note for Max to track
                message.setAddress("/synth");
                message.addIntArg(p.ID);
                p.hasNoteBeenSent = true;
            }
            
            int n_octaves = 2;
            int v = (p.position.y / this->videoSize.y) * 5 * n_octaves + 3;
            int octave = v / 5;
            v %= 5;
            int n;
            switch(v) {
                    // Major pentatonic: 0, 3, 5, 7, 10
                    // 0, 4, 5, 9, 10
                    // 0, 4, 7, 10, 12
                case 0:
                    n = 0;
                    break;
                case 1:
                    n = 3;
                    break;
                case 2:
                    n = 5;
                    break;
                case 3:
                    n = 7;
                    break;
                case 4:
                    n = 10;
                    break;
            }
            //float volume = 1 -  p.position.y / (float)this->videoHeight;
            n += 40;
            n += 12*octave;
            float volume = p.velocity.length() + p.freshness;
            float pan = 2*(p.position.x - this->videoSize.x/2) / this->videoSize.x;
            message.addFloatArg(n);         // Note
            message.addFloatArg(volume);      // Volume
            message.addFloatArg(pan);
        }
            break;
            
        case ParticleColor::GREEN:
        {
            // Trigger some glitchy samples
            if(p.hasNoteBeenSent && p.age % 3 != 0) return 0;
            // This is a NEW note for Max to track
            message.setAddress("/sampler");
            message.addIntArg(p.ID);
            p.hasNoteBeenSent = true;
            
            
            float offset = p.position.y / (float)this->videoSize.y;
            //float speed = (p.position.x - (float)this->videoWidth/2) / this->videoWidth * 10;
            float speed = p.position.distance(Vec2f(this->videoSize.x / 2, this->videoSize.y/2)) / 100.0f;
            //float speed = p.position.x / (float)this->videoWidth * 2;
            float length = 1- min(p.velocity.length() / 10.0f, 1.0f);
            length = speed;
            //float volume = 1; //p.freshness;
            float volume = p.velocity.length();
            if(volume > 2) volume = 2;
            if(volume < 0.2) volume = 0.2;
            //length = .1;
            
            message.addFloatArg(offset);
            message.addFloatArg(length);
            message.addFloatArg(speed);
            message.addFloatArg(volume);
            //console() << "Offset: " << offset << " Length: " << length << " Speed: " << speed << endl;
        }
            break;
        case ParticleColor::RED:
        {
            // Pop samples when particle hits bottom
            if(p.hasNoteBeenSent || p.position.y < this->videoSize.y * .9 || p.getVelocityHistory()[0].y > this->videoSize.y) return 0;
            
            // Only send if this particle hasn't triggered before.
            message.setAddress("/pop");
            //message.addIntArg(p.ID);
            p.hasNoteBeenSent = true;
            
            
            
            
            //int index = p.position.y / (float)this->videoHeight * 5;
            int index = 3;
            float pitch = 2*(p.position.x - this->videoSize.x/2) / this->videoSize.x * 10;
            //float pitch = 0;
            float pan = 2*(p.position.x - this->videoSize.x/2) / this->videoSize.x;
            
            //float volume = p.velocity.length() / 3.0f + 0.1f;
            float volume = 1.0f;
            //length = .1;
            
            
            message.addIntArg(index);
            message.addFloatArg(pitch);
            message.addFloatArg(volume);
            message.addFloatArg(pan);
        }
            break;
            
        case ParticleColor::YELLOW:
        {
            // Trigger the BASS synth when these particles hit the bottom
            if(p.hasNoteBeenSent || p.position.y < this->videoSize.y * .9) return 0;
            
            message.setAddress("/bass");
            message.addIntArg(p.ID);
            p.hasNoteBeenSent = true;
            
            
            int n_octaves = 1;
            int v = ((p.position.x / (float)this->videoSize.x)) * 5 * n_octaves;
            int octave = v / 5;
            v %= 5;
            int n;
            switch(v) {
                    // Major pentatonic: 0, 3, 5, 7, 10
                    // 0, 4, 5, 9, 10
                    // 0, 4, 7, 10, 12
                case 0:
                    n = 0;
                    break;
                case 1:
                    n = 3;
                    break;
                case 2:
                    n = 5;
                    break;
                case 3:
                    n = 7;
                    break;
                case 4:
                    n = 10;
                    break;
            }
            //float volume = 1 -  p.position.y / (float)this->videoHeight;
            n += 40;
            n += 12*octave;
            float volume = p.velocity.length() * 2;
            if(volume > 2) volume = 2;
            if(volume < 1) volume = 1;
            float pan = 2*(p.position.x - this->videoSize.x/2) / this->videoSize.x;
            message.addFloatArg(n);         // Note
            message.addFloatArg(volume);      // Volume
            message.addFloatArg(pan);
        }
            break;
            
    }
    this->sender.sendMessage(message);
    
    
    return 1;
}