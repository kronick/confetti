//
//  Balloon.cpp
//  ConfettiVision
//
//  Created by Disk Cactus on 5/5/15.
//
//

#include "Balloon.h"

using namespace ci;
using namespace std;

int Balloon::last_balloon_ID;

Balloon::Balloon() {
    this->init();
}

Balloon::Balloon(vector<cv::Mat> frames) {
    this->init();
    this->frames = frames;
    this->loaded = true;
}

Balloon::~Balloon() {
    if(this->loadMovieThread.joinable())
        this->loadMovieThread.join();
}

void Balloon::init() {
    this->frames = vector<cv::Mat>();
    this->ID = last_balloon_ID++;
    
    this->particleCollections[ParticleColor::RED] = ParticleCollection::create(ParticleColor::RED);
    this->particleCollections[ParticleColor::RED]->setDebugColor(0,0,255);
    this->particleCollections[ParticleColor::RED]->setThresholds(117, 179, 0, 150, 39, 255);
    this->particleCollections[ParticleColor::GREEN] = ParticleCollection::create(ParticleColor::GREEN);
    this->particleCollections[ParticleColor::GREEN]->setDebugColor(0,255,0);
    this->particleCollections[ParticleColor::GREEN]->setThresholds(52, 86, 0, 247, 54, 255);
    this->particleCollections[ParticleColor::BLUE] = ParticleCollection::create(ParticleColor::BLUE);
    this->particleCollections[ParticleColor::BLUE]->setDebugColor(255,0,0);
    this->particleCollections[ParticleColor::BLUE]->setThresholds(0, 57, 0, 96, 0, 255);
    this->particleCollections[ParticleColor::YELLOW] = ParticleCollection::create(ParticleColor::YELLOW);
    this->particleCollections[ParticleColor::YELLOW]->setDebugColor(0,255,255);
    this->particleCollections[ParticleColor::YELLOW]->setThresholds(103, 117, 0, 168, 121, 255);
    
    // Load Shaders and setup framebuffer
    try {
        this->rgb2hlsShader = gl::GlslProg::create(app::loadResource("passthru_vert.glsl"), app::loadResource("rgb2hls_frag.glsl"));
    }
    catch(gl::GlslProgCompileExc &exc) {
        app::console() << "Shader compile error: " << endl;
        app::console() << exc.what();
    }
    //gl::Fbo::Format format;
    this->hlsFramebuffer = gl::Fbo(640,480, false);
    
    
    // Create bass strings and waveform
    for(int i=0; i<5; i++) {
        this->bassStrings.push_back(BassString::create(i+1));
    }
    this->waveform = Waveform::create(app::loadResource("laugh.wav"));
}

void Balloon::update() {
    // Make sure we're the frames are ready to play and not paused
    if(!this->isReady() || this->frames.size() == 0) {
        // Draw loading texture
        return;
    }

    // Make sure we're within reasonable frame bounds
    if(this->currentFrameNumber < 0)
        this->currentFrameNumber = 0;
    if(this->currentFrameNumber > (this->frames.size() - 1))
        this->currentFrameNumber = this->frames.size() - 1;
    
    // Create a surface & texture out of the current frame
    this->currentFrameMat = this->frames[this->currentFrameNumber];
    this->currentFrameSurface = fromOcv(this->currentFrameMat);
    this->currentFrameTexture = gl::Texture(this->currentFrameSurface);
    
    // Process this frame to and start threaded populating of particle collections
    vector<thread> threads;
    map<ParticleColor, cv::Mat> hlsMats;
    for(auto c = this->particleCollections.begin(); c != this->particleCollections.end(); ++c) {
        hlsMats[c->first] = toOcv(this->processFrame(this->currentFrameTexture, c->first));
        threads.push_back(thread(&ParticleCollection::findParticles, c->second, ref(hlsMats[c->first])));
    }
    // Wait for all processing threads to complete
    for(auto &t : threads)
        t.join();
    
    // Check for a big blob to indicate that the balloon has not yet popped
    int largestBlob = 0;
    for(auto c = this->particleCollections.begin(); c != this->particleCollections.end(); ++c) {
        largestBlob = max(largestBlob, c->second->largestParticleArea);
    }
    this->largestBalloonSize = max(this->largestBalloonSize, largestBlob);
    
    if(largestBlob >= this->poppedEndThreshold) {
        this->seenABalloon = true;
    }
    if(largestBlob < this->poppedStartThreshold && !this->poppedYet && this->seenABalloon) {
        // The balloon has popped!
        this->setUniversalConstants();
        // TODO: Send different bangs based on the size of the balloon
        this->messenger->sendMessageForBang(1.0f);
        app::console() << "BANG!!" << endl;
        this->poppedYet = true;
        this->poppedFrame = this->currentFrameNumber;
    }
    else if(!this->poppedYet) {
        // Don't track particles until the pop happens
        for(auto c = this->particleCollections.begin(); c != this->particleCollections.end(); ++c) {
            c->second->resetParticles();
        }
    }

    
    // Now send messages for each particle if the balloon has popped, isn't paused, and isn't rewindingf
    if(this->poppedYet && !this->paused && this->playbackRate > 0) {
        for(auto c = this->particleCollections.begin(); c != this->particleCollections.end(); ++c) {
            int messages_sent = 0;
            for(auto &p : c->second->getParticles()) {
                messages_sent += this->messenger->sendMessagesForParticle(*p);
                if(messages_sent > this->message_per_color_limit) break;
            }
            //app::console() << "Sent " << messages_sent << " OSC messages for color #" << int(c->first) << endl;
        }
    }
    
    // Check to see if balloon should warp speed reverse playback
    if(this->currentFrameNumber == this->frames.size() - 1) {
        // At the end of the video. Warp speed reverse!
        this->playbackRate = this->speed_rewind;
    }
    else if(this->playbackRate > 0) {
        // Set the playback rate based on whether or not the balloon has popped
        if(!this->poppedYet)
            this->playbackRate = this->speed_forward;
        else
            this->playbackRate = 1;
    }
    else {
        if(this->currentFrameNumber <= 0) {
            if(this->loop) {
                this->reset();
                this->playbackRate = 1;
            }
        }
    }
    
    // Advance the playhead for the next cycle unless playback is paused
    if(!this->paused)
        this->currentFrameNumber += this->playbackRate;
    
    this->updateGraphics();
}
void Balloon::play() {
    this->paused = false;
}
void Balloon::pause() {
    this->paused = true;
}
void Balloon::reset() {
    this->currentFrameNumber = 0;
    this->poppedYet = false;
    this->seenABalloon = false;
    this->largestBalloonSize = 0;
    this->poppedFrame = -1;
}


gl::Texture Balloon::processFrame(ci::gl::Texture& frame, ParticleColor color) {
    // Run the texture through the shader, convert to HLS and threshold to isolate colors
    
    ColorThreshold threshold = this->particleCollections[color]->threshold;
    
    // First bind the texture, fbo, and shader
    gl::SaveFramebufferBinding bindingSaver;
    this->hlsFramebuffer.bindFramebuffer();
    gl::pushMatrices();
    frame.bind();
    this->rgb2hlsShader->bind();
    
    // Send texture and thresholds to shader
    this->rgb2hlsShader->uniform("tex0", 0);
    this->rgb2hlsShader->uniform("thresholdMin", Vec3f(threshold.h_min/255.0f, threshold.l_min/255.0f, threshold.s_min/255.0f));
    this->rgb2hlsShader->uniform("thresholdMax", Vec3f(threshold.h_max/255.0f, threshold.l_max/255.0f, threshold.s_max/255.0f));
    
    // Setup the FBO and draw in the RGB image
    gl::clear(Color(0,0,0));
    gl::setViewport(this->hlsFramebuffer.getBounds());
    gl::setMatricesWindow(this->hlsFramebuffer.getSize(), false);
    gl::color(0,1,0);
    gl::drawSolidRect(Rectf(0,0,640,480));
    gl::color(1,1,1);
    this->rgb2hlsShader->unbind();
    frame.unbind();
    gl::popMatrices();
    
    this->hlsFramebuffer.unbindFramebuffer();
    
    return this->hlsFramebuffer.getTexture();
}



void Balloon::setUniversalConstants() {
    // Use this->largestBalloonSize to set chord, etc
    
    // Choose chord then:
    this->messenger->sendSeed(this->ID);
    this->messenger->setChord(this->ID);
}

void Balloon::setFrames(std::vector<cv::Mat> frames) {
    this->frames = frames;
    this->ID = ++last_balloon_ID;
    this->reset();
}

bool Balloon::loadMovieFile(const ci::fs::path &path) {
    if(this->loading) return false;
    
    try {
        this->cvMovie.open(path.string());
        this->frameCount =   this->cvMovie.get(CV_CAP_PROP_FRAME_COUNT);
        app::console() << "Loaded file: " << path << "(" << this->frameCount << " frames)" << std::endl;

        this->ID = ++last_balloon_ID;
        this->currentFrameNumber = 0;
    }
    catch(...) {
        app::console() << "Unable to load movie file!" << std::endl;
        return false;
    }
    
    this->loading = true;
    this->loaded  = false;
    this->frames.clear();
    
    if(this->loadMovieThread.joinable()) this->loadMovieThread.join();
    this->reset();
    this->loadMovieThread = thread([this]() mutable {
        // Load all frames into a vector<cv::Mat>
        cv::Mat cvFrame;
        while(this->cvMovie.read(cvFrame)) {
            this->frames.push_back(cvFrame.clone());
        }
        app::console() << this->frames.size() << " frames retrieved." << endl;
        this->loading = false;
        this->loaded = true;
        this->frameCount = this->frames.size();
        //this->reset();
    });
    
    
    return true;
}



void Balloon::setDebug(bool d) {
    for(auto c = this->particleCollections.begin(); c != this->particleCollections.end(); ++c) {
        c->second->setDebugView(d);
    }
    this->debugMode = d;
}

void Balloon::updateGraphics() {
    // Get incoming messages
    vector<osc::Message> messages;
    while(this->messenger->hasWaitingMessages()) {
        osc::Message m = this->messenger->getNextMessage();
        messages.push_back(m);
        
        //app::console() << m.getAddress() << endl;
        
        string address = m.getAddress();
        if(address == "/bass") {
            // Pluck a bass string
            int string = m.getArgAsInt32(4);
            this->bassStrings[string]->pluck(m.getArgAsFloat(2));
            
        }
        else if(address == "/pop") {
            // Add a pop item
            int ID = m.getArgAsInt32(0);
            float pan = m.getArgAsFloat(3);
            
            this->pops.push_back(Pop::create((pan+1)/2.0f, randFloat(1.0f), randFloat(20, 40), randFloat(0, M_PI * 2)));
        }
        else if(address == "/synth") {
            // Create a new synthLine
            int ID = m.getArgAsInt32(0);
            int note_index = m.getArgAsInt32(4);
            float volume = m.getArgAsFloat(2);
            this->synthLines[ID] = SynthLine::create(note_index, 3, volume);
        }
        else if(address == "/synthchange") {
            // Find an existing synthLine and add a new point
            int ID = m.getArgAsInt32(0);
            int note_index = m.getArgAsInt32(4);
            float volume = m.getArgAsFloat(2);
            if(this->synthLines.count(ID))
                this->synthLines[ID]->changePitch(note_index, 5, volume);
        }
        else if(address == "/synthoff") {
            // Stop this synthline
            int ID = m.getArgAsInt32(0);
            if(this->synthLines.count(ID))
                this->synthLines[ID]->stop();
        }
        else if(address == "/sampler") {
            // Move to position x on the waveform
            float start = m.getArgAsFloat(1);
            float end = start + m.getArgAsFloat(2);
            float volume = m.getArgAsFloat(4);
            this->waveform->highlight(start, end, volume);
        }
    }
    
    for(auto p = this->pops.begin(); p != this->pops.end();) {
        if((*p)->isAlive()) {
            (*p)->update();
            ++p;
        }
        else {
            p = this->pops.erase(p);
        }
    }
    for(auto &p : this->bassStrings) {
        p->update();
    }
    for(auto it = synthLines.begin(); it != synthLines.end(); ++it) {
        it->second->update();
    }
    
}

void Balloon::drawPreviews(float width, float height) {
    gl::SaveColorState s;
    gl::pushMatrices();
    
    Vec2f videoSize(640,480);
    this->drawWidth = width;
    this->drawHeight = height;
    if(this->frames.size() > 0)
        videoSize = Vec2f(this->frames[0].cols, this->frames[0].rows);
    
    gl::Texture redTex = this->particleCollections[ParticleColor::RED]->getDebugView();
    gl::Texture greenTex = this->particleCollections[ParticleColor::GREEN]->getDebugView();
    gl::Texture blueTex = this->particleCollections[ParticleColor::BLUE]->getDebugView();
    gl::Texture yellowTex = this->particleCollections[ParticleColor::YELLOW]->getDebugView();

    float preview_padding = 0.2 * width / 4;
    float preview_scale_factor = (width/4-2*preview_padding) / videoSize.x;
    float preview_w = videoSize.x * preview_scale_factor;
    float preview_h = videoSize.y * preview_scale_factor;
    
    //Draw red pops
    gl::color(COLOR_RED);
    gl::drawSolidRect(Rectf(0,0,width/4,height));
    // Draw pop
    gl::color(COLOR_WHITE);
    for(auto &p : this->pops) {
        p->draw(width/4, height);
    }
    
//    gl::pushMatrices();
//    {
//        // Translate to TL corner of preview video pane
//        gl::translate(preview_padding, height - preview_h - preview_padding/2);
//        gl::scale(preview_scale_factor, preview_scale_factor);
//        
//        if(this->debugMode) {
//            gl::color(COLOR_WHITE);
//            gl::draw(this->particleCollections[ParticleColor::RED]->getDebugView());
//        }
//        else
//            drawParticles(ParticleColor::RED, videoSize);
//        
//        gl::color(COLOR_RED);
//        gl::lineWidth(1);
//        gl::drawLine(Vec2f(-videoSize.x * .1,videoSize.y * 0.9), Vec2f(videoSize.x * 1.1, videoSize.y * 0.9));
//    }
//    gl::popMatrices();
    
    // Draw Yellow bass lines
    gl::translate(width/4, 0);
    gl::color(COLOR_YELLOW);
    gl::drawSolidRect(Rectf(0,0,width/4,height));
    
    gl::color(COLOR_WHITE);
    int i = 1;
    for(auto &s : this->bassStrings) {
        s->draw(i/6.0f * width/4, 0, width/64.0f, height);
        i++;
    }
    
//    gl::pushMatrices();
//    {
//        // Translate to TL corner of preview video pane
//        gl::translate(preview_padding, height - preview_h - preview_padding/2);
//        gl::scale(preview_scale_factor, preview_scale_factor);
//        
//        if(this->debugMode) {
//            gl::color(COLOR_WHITE);
//            gl::draw(this->particleCollections[ParticleColor::YELLOW]->getDebugView());
//        }
//        else
//            drawParticles(ParticleColor::YELLOW, videoSize);
//        
//        gl::color(COLOR_WHITE);
//        gl::lineWidth(2);
//        // Finish line
//        gl::drawLine(Vec2f(-videoSize.x * .1,videoSize.y * 0.9), Vec2f(videoSize.x * 1.1, videoSize.y * 0.9));
//
//        gl::lineWidth(2);
//        float cols = 5.0f;
//        float dashOn = 4;
//        float dashOff = 8;
//        for(int i=1; i<cols; i++) {
//            // Vertical lines
//            float x = i * videoSize.x / cols;
//            for(int j=0; j<videoSize.y/(dashOn + dashOff); j++) {
//                gl::drawLine(Vec2f(x, j * (dashOn + dashOff)), Vec2f(x, j * (dashOn + dashOff) + dashOn));
//            }
//        }
//    }
//    gl::popMatrices();
    
    // Draw green laugh samples
    gl::translate(width/4, 0);
    gl::color(COLOR_GREEN);
    gl::drawSolidRect(Rectf(0,0,width/4,height));
    
    gl::color(COLOR_WHITE);
    this->waveform->draw(width/4, height);
    
//    gl::pushMatrices();
//    {
//        // Translate to TL corner of preview video pane
//        gl::translate(preview_padding, height - preview_h - preview_padding/2);
//        gl::scale(preview_scale_factor, preview_scale_factor);
//        
//        if(this->debugMode) {
//            gl::color(COLOR_WHITE);
//            gl::draw(this->particleCollections[ParticleColor::GREEN]->getDebugView());
//        }
//        else
//            drawParticles(ParticleColor::GREEN, videoSize);
//        
//        gl::color(COLOR_WHITE);
//        gl::lineWidth(2);
//        float rows = 10.0f;
//        float cols = 13.0f;
//        float dashOn = 4;
//        float dashOff = 8;
//        for(int i=1; i<rows; i++) {
//            // Horizontal lines
//            float y = i * videoSize.y / rows;
//            for(int j=0; j<videoSize.x/(dashOn + dashOff); j++) {
//                gl::drawLine(Vec2f(j * (dashOn + dashOff), y), Vec2f(j * (dashOn + dashOff) + dashOn, y));
//            }
//        }
//        for(int i=1; i<cols; i++) {
//            // Vertical lines
//            float x = i * videoSize.x / cols;
//            for(int j=0; j<videoSize.y/(dashOn + dashOff); j++) {
//                gl::drawLine(Vec2f(x, j * (dashOn + dashOff)), Vec2f(x, j * (dashOn + dashOff) + dashOn));
//            }
//        }
//    }
//    gl::popMatrices();
    
    
    // Draw blue note lines
    gl::translate(width/4, 0);
    gl::color(COLOR_BLUE);
    gl::drawSolidRect(Rectf(0,0,width/4,height));
    
    gl::color(COLOR_WHITE);
    for(auto l : this->synthLines) {
        l.second->draw(width/4, height);
    }
//    gl::pushMatrices();
//    {
//        // Translate to TL corner of preview video pane
//        gl::translate(preview_padding, height - preview_h - preview_padding/2);
//        gl::scale(preview_scale_factor, preview_scale_factor);
//        
//        if(this->debugMode) {
//            gl::color(COLOR_WHITE);
//            gl::draw(this->particleCollections[ParticleColor::BLUE]->getDebugView());
//        }
//        else
//            drawParticles(ParticleColor::BLUE, videoSize);
//        
//        
//        // Draw Grid Lines
//        gl::color(COLOR_WHITE);
//        gl::lineWidth(2);
//        float rows = 15.0f;
//        //float cols = 13.0f;
//        float dashOn = 4;
//        float dashOff = 8;
//        for(int i=1; i<rows; i++) {
//            // Horizontal lines
//            float y = i * videoSize.y / rows;
//            for(int j=0; j<videoSize.x/(dashOn + dashOff); j++) {
//                gl::drawLine(Vec2f(j * (dashOn + dashOff), y), Vec2f(j * (dashOn + dashOff) + dashOn, y));
//            }
//        }
//    }
//    gl::popMatrices();
//    
    
    gl::popMatrices();
}

void Balloon::drawParticles(ParticleColor color, Vec2f size) {
    gl::SaveColorState s;
    gl::color(COLOR_GREY);
    Color drawColor;
    switch(color) {
        case ParticleColor::RED:
            drawColor = COLOR_RED;
            break;
        case ParticleColor::GREEN:
            drawColor = COLOR_GREEN;
            break;
        case ParticleColor::BLUE:
            drawColor = COLOR_BLUE;
            break;
        case ParticleColor::YELLOW:
            drawColor = COLOR_YELLOW;
            break;
    }
    gl::drawSolidRect(Rectf(0,0,size.x, size.y));
    for(auto &p : this->particleCollections[color]->getParticles()) {
        if(p->isActive() && p->isAlive()) {
            
            // Choose alpha color based on age and freshness
            float alpha = min((float)p->age/10.0f, (float)p->freshness);
            alpha *= 0.5f;
            float age_scale = (p->age - 30) / 20.0f;
            alpha *= min(max(age_scale, 1.0f), 2.0f);
            gl::color(drawColor.r, drawColor.g, drawColor.b, alpha);
            
            for(auto &c : p->getContourHistory()) {
                if(c.size() > 0) {
                    Path2d contour(BSpline2f(c, 1, false, true));
                    gl::drawSolid(contour);
                }
            }

            //gl::drawSolidCircle(p->position, 8);
            
            if(p->positionHistory.size() >= 5) {
                Path2d path;
                gl::lineWidth(1);
                int start = p->positionHistory.size() * 0.5;
                int end = p->positionHistory.size() - 1;
                //int step = max((end-start) / 16, 1);
                int step = 8;
                float n = (float)(end-start);
                float width = 3;
                path.moveTo(p->positionHistory[start]);

//                Draw Tail as Streamer
//                for(int i = start; i<=end; i+=step) {
//                    path.lineTo(p->positionHistory[i] + Vec2f(width * (i-start)/n, 0));
//                }
//                for(int i = end; i>=start; i-=step) {
//                    path.lineTo(p->positionHistory[i] - Vec2f(width * (i-start)/n, 0));
//                }
                

                for (int i=start; i<=end; i+= step) {
                    path.lineTo(p->positionHistory[i]);
                }
                
                path.lineTo(p->position);
                

                gl::draw(path);
            }
        }
        
    }
}