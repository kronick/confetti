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
}

void Balloon::update() {
    // Make sure we're the frames are ready to play and not paused
    if(!this->isReady()) {
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
    
    if(largestBlob < this->poppedStartThreshold && !this->poppedYet) {
        // The balloon has popped!
        this->setUniversalConstants();
        // TODO: Send different bangs based on the size of the balloon
        this->messenger->sendMessageForBang(1.0f);
        app::console() << "BANG!!" << endl;
        this->poppedYet = true;
    }

    // Now send messages for each particle if the balloon has popped, isn't paused, and isn't rewindingf
    if(this->poppedYet && !this->paused && this->playbackRate > 0) {
        for(auto c = this->particleCollections.begin(); c != this->particleCollections.end(); ++c) {
            int messages_sent = 0;
            for(auto &p : c->second->getParticles()) {
                messages_sent += this->messenger->sendMessagesForParticle(*p);
                if(messages_sent > this->message_per_color_limit) break;
            }
            app::console() << "Sent " << messages_sent << " OSC messages for color #" << int(c->first) << endl;
        }
    }
    
    // Check to see if balloon should warp speed reverse playback
    if(this->currentFrameNumber == this->frames.size() - 1) {
        // At the end of the video. Warp speed reverse!
        this->playbackRate = -10;
    }
    else if(this->playbackRate > 0) {
        // Set the playback rate based on whether or not the balloon has popped
        if(!this->poppedYet)
            this->playbackRate = 5;
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
    this->largestBalloonSize = 0;
}

gl::Texture Balloon::processFrame(ci::gl::Texture& frame, ParticleColor color) {
    // Run the texture through the shader, convert to HLS and threshold to isolate colors
    
    ColorThreshold threshold = this->particleCollections[color]->threshold;
    
    // First bind the texture, fbo, and shader
    gl::SaveFramebufferBinding bindingSaver;
    this->hlsFramebuffer.bindFramebuffer();
    gl::pushMatrices();
    frame.enableAndBind();
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
    // this->messenger->setChord();
}

bool Balloon::loadMovieFile(const ci::fs::path &path) {
    if(this->loading) return false;
    
    try {
        this->cvMovie.open(path.string());
        int frameCount =   this->cvMovie.get(CV_CAP_PROP_FRAME_COUNT);
        app::console() << "Loaded file: " << path << "(" << frameCount << " frames)" << std::endl;
    }
    catch(...) {
        app::console() << "Unable to load movie file!" << std::endl;
        return false;
    }
    
    this->loading = true;
    this->loaded  = false;
    this->frames.clear();
    
    if(this->loadMovieThread.joinable()) this->loadMovieThread.join();
    
    this->loadMovieThread = thread([this]() mutable {
        // Load all frames into a vector<cv::Mat>
        cv::Mat cvFrame;
        while(this->cvMovie.read(cvFrame)) {
            this->frames.push_back(cvFrame.clone());
        }
        app::console() << this->frames.size() << " frames retrieved." << endl;
        this->loading = false;
        this->loaded = true;
        this->reset();
    });
    
    
    return true;
}