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

OSCMessenger::OSCMessenger(int sendPort, int listenPort) {
    this->sendPort = sendPort;
    this->listenPort = listenPort;
    if(this->sendPort > 0)
        this->sender.setup("localhost", this->sendPort, false);
    if(this->listenPort > 0) {
        this->listener.setup(this->listenPort);
    }
    
    // Set default parameters
    this->videoSize = Vec2f(640, 480);
    this->chord = {0, 3, 5, 7, 10};

}

void OSCMessenger::setVideoSize(const Vec2f videoSize) {
    this->videoSize = Vec2f(videoSize.x, videoSize.y);
}

void OSCMessenger::setChord(int s) {
    vector< vector<int> > chords;
    chords.push_back({0, 3, 5, 7, 10});
    chords.push_back({0, 4, 7, 10, 12});
    chords.push_back({0, 4, 5, 9, 10});
    
    this->chord = chords[s%chords.size()];
    
}

int OSCMessenger::sendMessageForBang(float scale) {
    // Trigger a bang
    osc::Message message;
    message.setAddress("/bang");
    message.addIntArg(0);
    message.addFloatArg(0);
    message.addFloatArg(4);
    message.addFloatArg(.5);
    this->sender.sendMessage(message);
    
    return 2;
}

int OSCMessenger::sendSeed(int s) {
    osc::Message message;
    message.setAddress("/seed");
    message.addIntArg(s);
    this->sender.sendMessage(message);
    return 1;
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
                //if(p.age % 10 != 0) return 0;
                //message.setAddress("/synth/" + to_string(p.ID));
                message.setAddress("/synthchange");
                message.addIntArg(p.ID);
            }
            else {
                // This is a NEW note for Max to track
                message.setAddress("/synth");
                message.addIntArg(p.ID);
                p.hasNoteBeenSent = true;
            }
            
            int n_octaves = 3;
            int v = (1-(p.position.y / this->videoSize.y)) * this->chord.size() * n_octaves + 3;
            int note_index = v;
            int octave = v / this->chord.size();
            v %= this->chord.size();
            int n = this->chord[v];


            n += 40;
            n += 12*octave;
            float volume = p.velocity.length() + p.freshness;
            float pan = 2*(p.position.x - this->videoSize.x/2) / this->videoSize.x;
            message.addFloatArg(n);         // Note
            message.addFloatArg(volume);      // Volume
            message.addFloatArg(pan);
            message.addIntArg(note_index);
        }
            break;
            
        case ParticleColor::GREEN:
        {
            // Trigger some glitchy samples
            //if(p.hasNoteBeenSent) return 0;
            if(p.age % 2 != 0) return 0;
            // This is a NEW note for Max to track
            message.setAddress("/sampler");
            message.addIntArg(p.ID);
            p.hasNoteBeenSent = true;
            
            
            float offset = p.position.y / (float)this->videoSize.y;
            //float speed = (p.position.x - (float)this->videoWidth/2) / this->videoWidth * 10;
            float speed = p.position.distance(Vec2f(this->videoSize.x / 2, this->videoSize.y/2)) / 100.0f;
            //float speed = p.position.x / (float)this->videoWidth * 2;
            float length = 1- min(p.velocity.length() / 10.0f, 1.0f);
            length = 0.1f;
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
            if(p.hasNoteBeenSent || p.position.y < (this->videoSize.y * .95) || p.getVelocityHistory()[0].y > (this->videoSize.y * 0.95)) return 0;
            
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
            if(p.hasNoteBeenSent || p.age < 20 || p.position.y < this->videoSize.y * .95 ||  p.getVelocityHistory()[0].y > (this->videoSize.y * 0.95)) return 0;
            
            message.setAddress("/bass");
            message.addIntArg(p.ID);
            p.hasNoteBeenSent = true;
            
            
            int n_octaves = 1;
            int v = (p.position.x / this->videoSize.x) * this->chord.size() * n_octaves;
            int octave = v / this->chord.size();
            v %= this->chord.size();
            int n = this->chord[v];
            
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
            message.addIntArg(v);   // Chord index
        }
            break;
            
    }
    this->sender.sendMessage(message);
    
    
    return 1;
}