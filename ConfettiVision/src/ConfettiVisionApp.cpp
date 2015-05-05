#include "ConfettiVisionApp.h"

using namespace ci;
using namespace ci::app;
using namespace std;



void ConfettiVisionApp::prepareSettings(Settings * settings) {
    settings->enableHighDensityDisplay( true );
    settings->setWindowSize(1280, 720);
    //settings->setWindowPos(0, 0);
}

void ConfettiVisionApp::setup()
{
    setFrameRate( 60.0f );
    
    this->debugFont = Font("Letter Gothic Std Bold", 12);
    this->debugTextureFont = gl::TextureFont::create(Font("Avenir Black", 12*2));
    
    this->configFilename = "config.xml";
    
    // Set up Movie
    // -------------------------------------
    //fs::path moviePath = getOpenFilePath();
    mMoviePaths.push_back(fs::path(loadResource("640x480_2.mp4")->getFilePath()));
    mMoviePaths.push_back(fs::path(loadResource("640x480_3.mp4")->getFilePath()));
    mMoviePaths.push_back(fs::path(loadResource("640x480.mp4")->getFilePath()));
    mMoviePaths.push_back(fs::path(loadResource("640x480_4.mp4")->getFilePath()));
    mMoviePaths.push_back(fs::path(loadResource("sorted-colors.png")->getFilePath()));
    mMoviePaths.push_back(fs::path(loadResource("ximea_1.mov")->getFilePath()));
    mMoviePaths.push_back(fs::path(loadResource("ximea_2.mpg")->getFilePath()));
    mMoviePaths.push_back(fs::path(loadResource("ximea_3.mpg")->getFilePath()));
    mMoviePaths.push_back(fs::path(loadResource("ximea_4.mpg")->getFilePath()));
    mMoviePaths.push_back(fs::path(loadResource("ximea_5.mpg")->getFilePath()));
    mMoviePaths.push_back(fs::path(loadResource("ximea_6.mpg")->getFilePath()));
    mMoviePaths.push_back(fs::path(loadResource("ximea_7.mpg")->getFilePath()));

    if (!mMoviePaths.empty()) {
        loadMovieFile(mMoviePaths[0]);
    }
    
    redParticles = ParticleCollection::create("red");
    redParticles->setDebugColor(0, 0, 255);
    redParticles->setThresholds(117, 179, 0, 150, 39, 255);
    greenParticles = ParticleCollection::create("green");
    greenParticles->setDebugColor(0, 255, 0);
    greenParticles->setThresholds(52, 86, 0, 247, 54, 255);
    blueParticles = ParticleCollection::create("blue");
    blueParticles->setDebugColor(255, 0, 0);
    blueParticles->setThresholds(0, 57, 0, 96, 0, 255);
    yellowParticles = ParticleCollection::create("yellow");
    yellowParticles->setDebugColor(0, 255, 255);
    yellowParticles->setThresholds(103, 117, 0, 168, 121, 255);
    
    this->setupParams();

    
	this->oscSender.setup( "localhost", 3000, false );
    this->oscListener.setup(3001);

    this->oscListener.registerMessageReceived([this](const osc::Message * m) { this->messageReceived(m); });
    // Load Shaders
    try {
        this->rgb2hlsShader = gl::GlslProg::create(loadResource("passthru_vert.glsl"), loadResource("rgb2hls_frag.glsl"));
    }
    catch(gl::GlslProgCompileExc &exc) {
        console() << "Shader compile error: " << endl;
        console() << exc.what();
    }
    gl::Fbo::Format format;
    this->hlsFramebuffer = gl::Fbo(640,480, false);
}

void ConfettiVisionApp::mouseDown( MouseEvent event )
{
}

void ConfettiVisionApp::mouseMove( MouseEvent event ){
    this->mousePosition = Vec2f(event.getX(),event.getY());
}

void ConfettiVisionApp::messageReceived(const osc::Message *m) {
    console() << m->getAddress() << endl;
    int channel = m->getArgAsInt32(0);
    int value = m->getArgAsInt32(1);
    
    ParticleCollectionRef colors [4] = {redParticles, greenParticles, blueParticles, yellowParticles};
    ParticleCollectionRef colorRef = colors[currentCalibrationColor];
    
    switch(channel) {
        case 58:
        {
            if(value < 127) break;
            // Calibrate PREVIOUS color
            this->currentCalibrationColor--;
            if(this->currentCalibrationColor < 0)
                this->currentCalibrationColor = 4 - this->currentCalibrationColor;
            break;
        }
        case 59:
        {
            if(value < 127) break;
            // Calibrate NEXT color
            this->currentCalibrationColor++;
            this->currentCalibrationColor %= 4;
            break;
        }
        case 0:
            colorRef->threshold.h_min = value*2;
            break;
        case 1:
            colorRef->threshold.h_max = value*2;
            break;
        case 2:
            colorRef->threshold.l_min = value*2;
            break;
        case 3:
            colorRef->threshold.l_max = value*2;
            break;
        case 4:
            colorRef->threshold.s_min = value*2;
            break;
        case 5:
            colorRef->threshold.s_max = value*2;
            break;
    }
    
}

void ConfettiVisionApp::setMode(Mode m ) {
    if(this->currentMode == m) return; // No mode change
    
    Mode oldMode = this->currentMode;
    this->currentMode = m;
    
    if(oldMode == Mode::CAPTURE) {
        // Stop camera stream if we're leaving capture mode
        this->camera.stopStream();
    }
    
    switch(m) {
        case Mode::CAPTURE:
        {
            // Start capture and set paramaters
            this->camera.startStream(this->paramFramerate);
            this->camera.setGain(this->paramGain);
            this->camera.setGammaY(this->paramGammaY);
            this->camera.setGammaC(this->paramGammaC);
            this->camera.setSharpness(this->paramSharpness);
            this->camera.setWhiteBalance(Vec3f(this->paramWB_r, this->paramWB_g, this->paramWB_b));
            break;
        }
        case Mode::REVIEW:
        {
            break;
        }
        case Mode::PLAYBACK:
        {
            break;
        }
    }

}

void ConfettiVisionApp::update()
{
    switch(this->currentMode) {
        case Mode::PLAYBACK:
        {
            this->updateVideoPlayback();
            break;
        }
        case Mode::CAPTURE:
        {
            this->cameraPreview = camera.getPreview();
            break;
        }
        case Mode::REVIEW:
        {
            break;
        }
            
    }
}


void ConfettiVisionApp::updateVideoPlayback() {
    if(this->cvMovie.isOpened()) {
        //mFrameSurface = mMovie->getSurface();
        cv::Mat cvFrame;
        // Loop movie if at the end
        //console() << this->cvMovie.get(CV_CAP_PROP_POS_FRAMES) << "/" << this->cvMovie.get(CV_CAP_PROP_FRAME_COUNT) << endl;
        int movieCurrentFrame = cvMovie.get(CV_CAP_PROP_POS_FRAMES);
        int movieFrameCount =   cvMovie.get(CV_CAP_PROP_FRAME_COUNT);
        bool isStill = movieFrameCount <= 1;
        if(movieCurrentFrame == movieFrameCount && !isStill) {
            this->resetPop();
            this->cvMovie.set(CV_CAP_PROP_POS_FRAMES, 0);
        }
        
        if(!this->poppedYet && isPlaybackPlaying) {
            this->cvMovie.set(CV_CAP_PROP_POS_FRAMES, movieCurrentFrame + 4);
        }
        
        if(!isStill && isPlaybackPlaying)
            this->cvMovie >> cvFrame;
        else {
            if(!cvMovie.retrieve(cvFrame)) {
                cvMovie.read(cvFrame);
            }
        }
        this->mFrameSurface = fromOcv(cvFrame);
        if(this->mFrameSurface) {
            mFrameTexture = gl::Texture(mFrameSurface);
            
            
            // TODO: Move the toOcv call into the processing threads.
            //       This also means this->processFrame will have to allocate
            //       or use a unique texture each time it is called
            cv::Mat redFrame = toOcv(this->processFrame(mFrameTexture, this->redParticles));
            cv::Mat greenFrame = toOcv(this->processFrame(mFrameTexture, this->greenParticles));
            cv::Mat blueFrame = toOcv(this->processFrame(mFrameTexture, this->blueParticles));
            cv::Mat yellowFrame = toOcv(this->processFrame(mFrameTexture, this->yellowParticles));
//            
//            this->redParticles->findParticles(redFrame);
//            this->greenParticles->findParticles(greenFrame);
//            this->blueParticles->findParticles(blueFrame);
//            this->yellowParticles->findParticles(yellowFrame);

            // Process each color in its own thread
            std::thread redThread(&ParticleCollection::findParticles, redParticles, ref(redFrame));
            std::thread greenThread(&ParticleCollection::findParticles, greenParticles, ref(greenFrame));
            std::thread blueThread(&ParticleCollection::findParticles, blueParticles, ref(blueFrame));
            std::thread yellowThread(&ParticleCollection::findParticles, yellowParticles, ref(yellowFrame));

            redThread.join();
            greenThread.join();
            blueThread.join();
            yellowThread.join();
            
            //cv::Mat frame = this->getHLSImage(this->mFrameSurface);
            
//            cv::Mat frame = toOcv(hlsFramebuffer.getTexture());
        

//            // Process each color in its own thread
//            std::thread redThread(&ParticleCollection::findParticles, redParticles, ref(frame));
//            std::thread greenThread(&ParticleCollection::findParticles, greenParticles, ref(frame));
//            std::thread blueThread(&ParticleCollection::findParticles, blueParticles, ref(frame));
//            std::thread yellowThread(&ParticleCollection::findParticles, yellowParticles, ref(frame));
//            
//            redThread.join();
//            greenThread.join();
//            blueThread.join();
//            yellowThread.join();
            
            
            int largestBlob = max(max(max(blueParticles->largestParticleArea, redParticles->largestParticleArea), greenParticles->largestParticleArea), yellowParticles->largestParticleArea);
            int n_particles =  greenParticles->getActiveParticleCount() + redParticles->getActiveParticleCount();
            
            if(largestBlob < poppedStartThreshold && !poppedYet) {
                // Trigger a bang
                osc::Message message;
                message.setAddress("/pop");
                message.addIntArg(0);
                message.addFloatArg(0);
                message.addFloatArg(4);
                message.addFloatArg(.5);
                oscSender.sendMessage(message);
                osc::Message message2;
                message2.setAddress("/pop");
                message2.addIntArg(1);
                message2.addFloatArg(0);
                message2.addFloatArg(2);
                message2.addFloatArg(-.5);
                oscSender.sendMessage(message2);
                poppedYet = true;
                console() << "BANG!!" << endl;
            }
            else if(largestBlob > poppedEndThreshold && poppedYet) {
                // Reset
                poppedYet = false;
                console() << "NEW BALLOON" << endl;
            }
            
            
            
            if(poppedYet) {
                int particle_limit = 20;
                int n_particles = 0;
                // Figure out OSC messages to send for this particle
                for(auto &p : blueParticles->getParticles()) {
                    if(n_particles++ < particle_limit)
                        sendMessagesForParticle(*p, 3);
                }
                n_particles = 0;
                for(auto &p : greenParticles->getParticles()) {
                    if(n_particles++ < particle_limit)
                        sendMessagesForParticle(*p, 1);
                }
                n_particles = 0;
                for(auto &p : redParticles->getParticles()) {
                    if(n_particles++ < particle_limit)
                        sendMessagesForParticle(*p, 2);
                }
                n_particles = 0;
                for(auto &p : yellowParticles->getParticles()) {
                    if(n_particles++ < particle_limit)
                        sendMessagesForParticle(*p, 0);
                }
            }
            
            
        }
    }
}

void ConfettiVisionApp::sendMessagesForParticle(Particle &p, int channel) {
    if(!p.isActive()) return;
    osc::Message message;
    
    switch(channel) {
        case 0:
        {
            // Trigger the smooth synth
            if(p.hasNoteBeenSent) {
                // This is an OLD note and Max should already know its ID
                if(p.age % 10 != 0) return;
                message.setAddress("/synth/" + to_string(p.ID));
            }
            else {
                // This is a NEW note for Max to track
                message.setAddress("/synth");
                message.addIntArg(p.ID);
                p.hasNoteBeenSent = true;
            }
            
            int n_octaves = 2;
            int v = (p.position.y / (float)this->videoHeight) * 5 * n_octaves + 3;
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
            float pan = 2*(p.position.x - this->videoWidth/2) / this->videoWidth;
            message.addFloatArg(n);         // Note
            message.addFloatArg(volume);      // Volume
            message.addFloatArg(pan);
        }
        break;
            
        case 1:
        {
            // Trigger some glitchy samples
            if(p.hasNoteBeenSent && p.age % 3 != 0) return;
            // This is a NEW note for Max to track
            message.setAddress("/sampler");
            message.addIntArg(p.ID);
            p.hasNoteBeenSent = true;
            
            
            float offset = p.position.y / (float)this->videoHeight;
            //float speed = (p.position.x - (float)this->videoWidth/2) / this->videoWidth * 10;
            float speed = p.position.distance(Vec2f(this->videoWidth / 2, this->videoHeight/2)) / 100.0f;
            //float speed = p.position.x / (float)this->videoWidth * 2;
            float length = 1- min(p.velocity.length() / 10.0f, 1.0f);
            length = speed;
            float volume = 1; //p.freshness;
            //length = .1;
            
            message.addFloatArg(offset);
            message.addFloatArg(length);
            message.addFloatArg(speed);
            message.addFloatArg(volume);
            //console() << "Offset: " << offset << " Length: " << length << " Speed: " << speed << endl;
        }
        break;
        case 2:
        {
            // Pop samples when particle hits bottom
            if(p.hasNoteBeenSent || p.position.y < this->videoHeight * .9 || p.getVelocityHistory()[0].y > this->videoHeight) return;
            
            // Only send if this particle hasn't triggered before.
            message.setAddress("/pop");
            //message.addIntArg(p.ID);
            p.hasNoteBeenSent = true;


            
            
            //int index = p.position.y / (float)this->videoHeight * 5;
            int index = 3;
            float pitch = 2*(p.position.x - this->videoWidth/2) / this->videoWidth * 10;
            //float pitch = 0;
            float pan = 2*(p.position.x - this->videoWidth/2) / this->videoWidth;
            
            //float volume = p.velocity.length() / 3.0f + 0.1f;
            float volume = 1.0f;
            //length = .1;
            
            
            message.addIntArg(index);
            message.addFloatArg(pitch);
            message.addFloatArg(volume);
            message.addFloatArg(pan);
        }
        break;
            
        case 3:
        {
            // Trigger the BASS synth when these particles hit the bottom
            if(p.hasNoteBeenSent || p.position.y < this->videoHeight * .9) return;
            
            message.setAddress("/bass");
            message.addIntArg(p.ID);
            p.hasNoteBeenSent = true;
            
            
            int n_octaves = 1;
            int v = ((p.position.x / (float)this->videoWidth)) * 5 * n_octaves;
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
            float pan = 2*(p.position.x - this->videoWidth/2) / this->videoWidth;
            message.addFloatArg(n);         // Note
            message.addFloatArg(volume);      // Volume
            message.addFloatArg(pan);
        }
            break;
      
    }
    oscSender.sendMessage(message);
    
}

gl::Texture ConfettiVisionApp::processFrame(ci::gl::Texture& frame, ParticleCollectionRef particleCollection) {
    // Run the texture through the shader, convert to HLS
    gl::SaveFramebufferBinding bindingSaver;
    this->hlsFramebuffer.bindFramebuffer();
    gl::pushMatrices();
    frame.enableAndBind();
    this->rgb2hlsShader->bind();
    
    this->rgb2hlsShader->uniform("tex0", 0);
    this->rgb2hlsShader->uniform("thresholdMin", Vec3f(particleCollection->threshold.h_min/255.0f, particleCollection->threshold.l_min/255.0f, particleCollection->threshold.s_min/255.0f));
    this->rgb2hlsShader->uniform("thresholdMax", Vec3f(particleCollection->threshold.h_max/255.0f, particleCollection->threshold.l_max/255.0f, particleCollection->threshold.s_max/255.0f));
    gl::clear(Color(0,0,0));
    gl::setViewport( this->hlsFramebuffer.getBounds() );
//    gl::setViewport(this->hlsFramebuffer.getBounds());
    gl::setMatricesWindow( this->hlsFramebuffer.getSize(), false );
    gl::color(0,1,0);
    gl::drawSolidRect(Rectf(0,0,640,480));
    gl::color(1,1,1);
    this->rgb2hlsShader->unbind();
    frame.unbind();
    gl::popMatrices();
    this->hlsFramebuffer.unbindFramebuffer();
    
    return this->hlsFramebuffer.getTexture();
}

cv::Mat ConfettiVisionApp::getHLSImage(Surface surface) {
    cv::Mat frame(toOcv(surface));
    cv::cvtColor(frame, frame, CV_RGB2HLS);

    return frame;
}

void ConfettiVisionApp::draw()
{
    // clear out the window with black
    gl::clear( Color( 1,1,1 ) );
    gl::enableAlphaBlending();
    
    gl::setViewport(Area(0,0, getWindowWidth() * getWindowContentScale(), getWindowHeight() * getWindowContentScale()));
    
    switch(this->currentMode) {
        case Mode::CAPTURE:
        {
            drawCapture();
            break;
        }
        case Mode::REVIEW:
        {
            drawReview();
            break;
        }
        case Mode::PLAYBACK:
        {
            drawPlayback();
            break;
        }
    }
    
    // Draw the params control interface
    params->draw();
    
    drawDebugString(to_string(getAverageFps()), Vec2f(50, getWindowHeight() - 12));
}

void ConfettiVisionApp::drawCapture() {
    if(this->cameraPreview) {
        float scale_factor = getWindowHeight() / (float)this->cameraPreview->getHeight();
        gl::draw(this->cameraPreview,
                 Rectf(getWindowWidth()/2 - this->cameraPreview->getWidth() * scale_factor / 2,
                       getWindowHeight()/2 - this->cameraPreview->getHeight() * scale_factor / 2,
                       getWindowWidth()/2 + this->cameraPreview->getWidth() * scale_factor / 2,
                       getWindowHeight()/2 + this->cameraPreview->getHeight() * scale_factor / 2));
    }
    
    if(this->camera.isRecording()) {
        gl::color(1.0f,0.1f,0.1f);
        gl::drawSolidCircle(Vec2f(getWindowWidth() - 40, getWindowHeight() - 40), 15);
        gl::color(1.0f,1.0f,1.0f);
    }
}

void ConfettiVisionApp::drawReview() {
    vector<cv::Mat> frames = this->camera.getCapturedFrames();
    if(frames.size() == 0)
        return;
    
    int selectedIndex = mousePosition.x / (float)getWindowWidth() * frames.size();
    if(selectedIndex < 0) selectedIndex = 0;
    if(selectedIndex >= frames.size()) selectedIndex = frames.size() - 1;
    
    gl::Texture tex = fromOcv(frames[selectedIndex]);
    float scale_factor = getWindowHeight() / (float)tex.getHeight();
    gl::draw(tex,
             Rectf(getWindowWidth()/2 - tex.getWidth() * scale_factor / 2,
                   getWindowHeight()/2 - tex.getHeight() * scale_factor / 2,
                   getWindowWidth()/2 + tex.getWidth() * scale_factor / 2,
                   getWindowHeight()/2 + tex.getHeight() * scale_factor / 2));
    
//    ci::gl::Texture::create(ci::fromOcv(this->previewImage));
}

void ConfettiVisionApp::drawPlayback() {

    
    gl::Texture redTex = redParticles->getDebugView();
    gl::Texture greenTex = greenParticles->getDebugView();
    gl::Texture blueTex = blueParticles->getDebugView();
    gl::Texture yellowTex = yellowParticles->getDebugView();
    
    int offset_x = 220;
    int offset_y = 20;
    float s = 0.95f;
    int textHeight = 12;
    
    if(redTex && greenTex && blueTex && yellowTex) {
        float scale_factor = ((float)getWindowWidth()-offset_x) / redTex.getWidth() / 4.0f;
        float w = redTex.getWidth() * scale_factor;
        float h = redTex.getHeight() * scale_factor;

        gl::draw(redTex, Rectf(offset_x + w * 0,offset_y, offset_x + w * s + w * 0, offset_y + h * s));
        drawDebugString("Points:       " + to_string(redParticles->getActiveParticleCount()), Vec2f(offset_x+w*0 + w*(1-s)/2,offset_y + h*s + textHeight));
        gl::draw(greenTex, Rectf(offset_x + w * 1,offset_y, offset_x + w * s + w * 1, offset_y + h * s));
        drawDebugString("Points:       " + to_string(greenParticles->getActiveParticleCount()), Vec2f(offset_x+w*1 + w*(1-s)/2,offset_y + h*s + textHeight));
        gl::draw(blueTex, Rectf(offset_x + w * 2,offset_y, offset_x + w * s + w * 2, offset_y + h * s));
        drawDebugString("Points:       " + to_string(blueParticles->getActiveParticleCount()), Vec2f(offset_x+w*2 + w*(1-s)/2,offset_y + h*s + textHeight));
        gl::draw(yellowTex, Rectf(offset_x + w * 3,offset_y, offset_x + w * s + w * 3, offset_y + h * s));
        drawDebugString("Points:       " + to_string(yellowParticles->getActiveParticleCount()), Vec2f(offset_x+w*3 + w*(1-s)/2,offset_y + h*s + textHeight));
    }
    
    if(mFrameSurface) {
        gl::Texture sourceVid(mFrameSurface);
        float scale_factor = 600.0f / sourceVid.getWidth();
        float w = sourceVid.getWidth() * scale_factor;
        float h = sourceVid.getHeight() * scale_factor;
        gl::draw(sourceVid, Rectf(getWindowWidth() / 2 - w/2, getWindowHeight()-h, getWindowWidth() / 2 + w/2, getWindowHeight()));
    }
    
//    if(hlsFramebuffer.getTexture()) {
//        gl::draw(hlsFramebuffer.getTexture());
//        //gl::draw(mFrameTexture);
//    }
}

void ConfettiVisionApp::keyDown(cinder::app::KeyEvent event) {
    if(event.getChar() == 'f') {
        setFullScreen( ! isFullScreen() );
    }
    else if(event.getChar() == 't') {
        if(this->camera.isRecording()) {
            this->camera.stopTrigger();
            this->setMode(Mode::REVIEW);
        }
        else {
            this->camera.startTrigger();
        }
    }
    else if(event.getChar() == 's') {
        this->camera.saveBuffer("~/Desktop/out.mpg");
    }
}

void ConfettiVisionApp::fileDrop(FileDropEvent event)
{
    loadMovieFile(event.getFile(0));
}

void ConfettiVisionApp::loadMovieFile(const fs::path &path)
{
    try {
        
//        mMovie = qtime::MovieSurface::create(path);
//        mMovie->setLoop();
//        mMovie->play();
//        mMovie->setRate(0.5f);
//
        cvMovie.open(path.string());
        
        console() << "Loaded file: " << path << std::endl;
    }
    catch(...) {
        console() << "Unable to load movie file!" << std::endl;
        //mMovie->reset();
    }
    
    mFrameTexture.reset();
}

void ConfettiVisionApp::resetPop() {
    this->playbackSpeed = 1;
    this->poppedYet = false;
}

void ConfettiVisionApp::drawDebugString(const std::string &str, const Vec2f &baseline) {
    // Retina resolution!
    gl::color( ColorA( 0,0,0, 1.0f ) );
    debugTextureFont->drawString(str, baseline, gl::TextureFont::DrawOptions().scale( 0.5f ).pixelSnap( false ));
    gl::color( Color( 1, 1,1 ) );
}


void ConfettiVisionApp::setupParams() {
    // Set up Params control panel
    // ---------------------------------------
   	this->params = params::InterfaceGl::create( getWindow(), "Settings", toPixels( Vec2i( 200, 650 ) ) );
    this->config = config::Config::create(this->params);
    
    params->addButton("Restart", [this]() { this->cvMovie.set(CV_CAP_PROP_POS_FRAMES, 0); } );
    params->addButton("Pause ||", [this]() { this->isPlaybackPlaying = false; } );
    params->addButton("Play [>]", [this]() { this->isPlaybackPlaying = true; } );
    params->addButton("Save Config", [this]() {
        this->config->save(fs::path(this->configFilename));
        console() << "Saved config to:" << ( fs::path(this->configFilename)) << endl;
    });
    params->addSeparator();
    params->addButton("File 1", [this]() { loadMovieFile(mMoviePaths[0]); } );
    params->addButton("File 2", [this]() { loadMovieFile(mMoviePaths[1]); } );
    params->addButton("File 3", [this]() { loadMovieFile(mMoviePaths[2]); } );
    params->addButton("File 4", [this]() { loadMovieFile(mMoviePaths[3]); } );
    params->addButton("File 5", [this]() { loadMovieFile(mMoviePaths[4]); } );
    params->addButton("File 6", [this]() { loadMovieFile(mMoviePaths[5]); } );
    params->addButton("File 7", [this]() { loadMovieFile(mMoviePaths[6]); } );
    params->addButton("File 8", [this]() { loadMovieFile(mMoviePaths[7]); } );
    params->addButton("File 9", [this]() { loadMovieFile(mMoviePaths[8]); } );
    params->addButton("File 10", [this]() { loadMovieFile(mMoviePaths[9]); } );
    params->addButton("File 11", [this]() { loadMovieFile(mMoviePaths[10]); } );
    params->addButton("File 12", [this]() { loadMovieFile(mMoviePaths[11]); } );
    params->addSeparator();
    params->addButton("CAPTURE", [this]() { this->setMode(Mode::CAPTURE); } );
    params->addButton("REVIEW", [this]() { this->setMode(Mode::REVIEW); } );
    params->addButton("PLAYBACK", [this]() { this->setMode(Mode::PLAYBACK); } );
    params->addSeparator();
    
    config->newNode("Camera settings");
    config->addParam("Camera Exposure", &paramExposure).updateFn([this] { this->camera.setExposure(this->paramExposure); } ).min(500).max(500000);
    config->addParam("Camera Gain", &paramGain).updateFn([this] { this->camera.setGain(this->paramGain); } ).min(-10).max(10).step(0.1f);
    config->addParam("Camera Framerate", &paramFramerate).updateFn([this] { this->camera.setFramerate(this->paramFramerate); } ).min(1).max(800).step(10);
    config->addParam("Camera Sharpness", &paramSharpness).updateFn([this] { this->camera.setSharpness(this->paramSharpness); } ).min(-4).max(4).step(0.1f);
    config->addParam("Camera Gamma Y", &paramGammaY).updateFn([this] { this->camera.setGammaY(this->paramGammaY); } ).min(0.3f).max(1).step(0.01f);
    config->addParam("Camera Gamma C", &paramGammaC).updateFn([this] { this->camera.setGammaC(this->paramGammaC); } ).min(0).max(1).step(0.01f);
    //    config->addParam("Camera HDR", &paramHDR).updateFn([this] {
    //        this->camera.setHDR(this->paramHDR);
    //        this->camera.setHDRExposure(this->paramHDRExposure);
    //        this->camera.setHDRKneepoint(this->paramHDRKneepoint);
    //    });
    //    config->addParam("HDR Exposure", &paramHDRExposure).updateFn([this] { this->camera.setHDRExposure(this->paramHDRExposure); }).min(0).max(100).step(1);
    //    config->addParam("HDR Kneepoint", &paramHDRKneepoint).updateFn([this] { this->camera.setHDRKneepoint(this->paramHDRKneepoint); }).min(0).max(100).step(1);
    params->addButton("Manual WB", [this]() {
        this->camera.enableManualWhiteBalance();
        this->paramWB_r = this->camera.getWhiteBalance().x;
        this->paramWB_g = this->camera.getWhiteBalance().y;
        this->paramWB_b = this->camera.getWhiteBalance().z;
        
    } );
    params->addButton("Auto WB", [this]() { this->camera.enableAutoWhiteBalance(); } );
    
    
    config->addParam("WB_R", &paramWB_r).accessors(
                                                   [this](float v) { this->camera.setWhiteBalance(Vec3f(paramWB_r, paramWB_g, paramWB_b)); },
                                                   [this]() { return paramWB_r; });
    config->addParam("WB_G", &paramWB_g).accessors(
                                                   [this](float v) { this->camera.setWhiteBalance(Vec3f(paramWB_r, paramWB_g, paramWB_b)); },
                                                   [this]() { return paramWB_g; });
    config->addParam("WB_B", &paramWB_b).accessors(
                                                   [this](float v) { this->camera.setWhiteBalance(Vec3f(paramWB_r, paramWB_g, paramWB_b)); },
                                                   [this]() { return paramWB_b; });
    
    
    params->addButton("Trigger START", [this]() { this->camera.startTrigger(); } );
    params->addButton("Trigger STOP", [this]() { this->camera.stopTrigger(); } );
    params->addButton("Save Buffer", [this]() { this->camera.saveBuffer("~/Desktop/out.mpg"); } );
    params->addSeparator();
    
    config->newNode("Color thresholds");
    params->addSeparator();
    params->addText("REDS");
    config->addParam("R Hue min", &(redParticles->threshold.h_min)).min(0).max(255);
    config->addParam("R Hue max", &(redParticles->threshold.h_max)).min(0).max(255);
    config->addParam("R Saturation min", &(redParticles->threshold.s_min)).min(0).max(255);
    config->addParam("R Saturation max", &(redParticles->threshold.s_max)).min(0).max(255);
    config->addParam("R Lightness min", &(redParticles->threshold.l_min)).min(0).max(255);
    config->addParam("R Lightness max", &(redParticles->threshold.l_max)).min(0).max(255);
    params->addSeparator();
    params->addText("Green");
    config->addParam("G Hue min", &(greenParticles->threshold.h_min)).min(0).max(255);
    config->addParam("G Hue max", &(greenParticles->threshold.h_max)).min(0).max(255);
    config->addParam("G Saturation min", &(greenParticles->threshold.s_min)).min(0).max(255);
    config->addParam("G Saturation max", &(greenParticles->threshold.s_max)).min(0).max(255);
    config->addParam("G Lightness min", &(greenParticles->threshold.l_min)).min(0).max(255);
    config->addParam("G Lightness max", &(greenParticles->threshold.l_max)).min(0).max(255);
    params->addSeparator();
    params->addText("Blue");
    config->addParam("B Hue min", &(blueParticles->threshold.h_min)).min(0).max(255);
    config->addParam("B Hue max", &(blueParticles->threshold.h_max)).min(0).max(255);
    config->addParam("B Saturation min", &(blueParticles->threshold.s_min)).min(0).max(255);
    config->addParam("B Saturation max", &(blueParticles->threshold.s_max)).min(0).max(255);
    config->addParam("B Lightness min", &(blueParticles->threshold.l_min)).min(0).max(255);
    config->addParam("B Lightness max", &(blueParticles->threshold.l_max)).min(0).max(255);
    params->addSeparator();
    params->addText("Yellow");
    config->addParam("Y Hue min", &(yellowParticles->threshold.h_min)).min(0).max(255);
    config->addParam("Y Hue max", &(yellowParticles->threshold.h_max)).min(0).max(255);
    config->addParam("Y Saturation min", &(yellowParticles->threshold.s_min)).min(0).max(255);
    config->addParam("Y Saturation max", &(yellowParticles->threshold.s_max)).min(0).max(255);
    config->addParam("Y Lightness min", &(yellowParticles->threshold.l_min)).min(0).max(255);
    config->addParam("Y Lightness max", &(yellowParticles->threshold.l_max)).min(0).max(255);
    params->addSeparator();
    
    if(fs::exists(fs::path(this->configFilename) ))
        this->config->load(fs::path(this->configFilename));
}

CINDER_APP_NATIVE( ConfettiVisionApp, RendererGl( RendererGl::AA_NONE ) )
