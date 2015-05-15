#include "ConfettiVisionApp.h"

using namespace ci;
using namespace ci::app;
using namespace std;



void ConfettiVisionApp::prepareSettings(Settings * settings) {
    settings->enableHighDensityDisplay( true );
    settings->setWindowSize(1280, 720);
    settings->setWindowPos(0, 0);
    settings->setFullScreen();
    //settings->setWindowPos(0, 0);
}

void ConfettiVisionApp::setup()
{
    setFrameRate( 60.0f );
    
    this->debugFont = Font("Letter Gothic Std Bold", 12);
    this->debugTextureFont = gl::TextureFont::create(Font("Avenir Black", 12*2));
    
    textureFonts[12] = gl::TextureFont::create(Font("Avenir Black", 12*2));
    textureFonts[16] = gl::TextureFont::create(Font("Avenir Black", 16*2));
    textureFonts[20] = gl::TextureFont::create(Font("Avenir Black", 20*2));
    textureFonts[24] = gl::TextureFont::create(Font("Avenir Black", 24*2));
    textureFonts[36] = gl::TextureFont::create(Font("Avenir Black", 36*2));
    textureFonts[48] = gl::TextureFont::create(Font("Avenir Black", 48*2));
    
    this->configFilename = "/Users/diskcactus1/Desktop/config.xml";
    
    // Set up Movie
    // -------------------------------------
    //fs::path moviePath = getOpenFilePath();
    mMoviePaths.push_back(fs::path("/Users/diskcactus1/Desktop/balloon_3.mov"));
    mMoviePaths.push_back(fs::path("/Users/diskcactus1/Desktop/balloon_4.mov"));
    mMoviePaths.push_back(fs::path("/Users/diskcactus1/Desktop/balloon_5.mov"));
    mMoviePaths.push_back(fs::path("/Users/diskcactus1/Desktop/balloon_9.mov"));
    mMoviePaths.push_back(fs::path("/Users/diskcactus1/Desktop/balloon_12.mov"));
    mMoviePaths.push_back(fs::path("/Users/diskcactus1/Desktop/balloon_13.mov"));
    mMoviePaths.push_back(fs::path("/Users/diskcactus1/Desktop/balloon_14.mov"));
    mMoviePaths.push_back(fs::path("/Users/diskcactus1/Desktop/balloon_15.mov"));
    mMoviePaths.push_back(fs::path("/Users/diskcactus1/Desktop/balloon_16.mov"));
//    mMoviePaths.push_back(fs::path(loadResource("640x480_2.mp4")->getFilePath()));
//    mMoviePaths.push_back(fs::path(loadResource("640x480_3.mp4")->getFilePath()));
//    mMoviePaths.push_back(fs::path(loadResource("640x480.mp4")->getFilePath()));
//    mMoviePaths.push_back(fs::path(loadResource("640x480_4.mp4")->getFilePath()));
//    mMoviePaths.push_back(fs::path(loadResource("sorted-colors.png")->getFilePath()));
//    mMoviePaths.push_back(fs::path(loadResource("ximea_1.mov")->getFilePath()));
//    mMoviePaths.push_back(fs::path(loadResource("ximea_2.mpg")->getFilePath()));
//    mMoviePaths.push_back(fs::path(loadResource("ximea_3.mpg")->getFilePath()));
//    mMoviePaths.push_back(fs::path(loadResource("ximea_4.mpg")->getFilePath()));
//    mMoviePaths.push_back(fs::path(loadResource("ximea_5.mpg")->getFilePath()));
//    mMoviePaths.push_back(fs::path(loadResource("ximea_6.mpg")->getFilePath()));
//    mMoviePaths.push_back(fs::path(loadResource("ximea_7.mpg")->getFilePath()));

    this->messenger = OSCMessenger(3000, 3002);
    this->controlListener.setup(3001);
    //this->oscListener.registerMessageReceived([this](const osc::Message * m) { this->messageReceived(m); });
    
    this->balloon = Balloon::create();
    this->balloon->messenger = &this->messenger;
    this->balloon->parent = this;
    
    if (!mMoviePaths.empty()) {
        this->balloon->loadMovieFile(mMoviePaths[0]);
    }
    
    // Load config and set up sliders
    this->setupParams();
    
    // Set saved thresholds now that config has loaded
    this->setThresholds();
    
}

void ConfettiVisionApp::setThresholds() {
    this->balloon->particleCollections[ParticleColor::RED]->threshold = this->paramThresholdR;
    this->balloon->particleCollections[ParticleColor::GREEN]->threshold = this->paramThresholdG;
    this->balloon->particleCollections[ParticleColor::BLUE]->threshold = this->paramThresholdB;
    this->balloon->particleCollections[ParticleColor::YELLOW]->threshold = this->paramThresholdY;
}

void ConfettiVisionApp::mouseDown( MouseEvent event )
{
    this->draggingMouse = true;
    this->mousePosition = Vec2f(event.getX(),event.getY());
    if(event.isRight())
        this->scrubbingMouse = true;
    
    hideCursor();
}

void ConfettiVisionApp::mouseUp(MouseEvent event) {
    this->draggingMouse = false;
    this->scrubbingMouse = false;
    showCursor();
}

void ConfettiVisionApp::mouseMove( MouseEvent event ){
    this->mousePosition = Vec2f(event.getX(),event.getY());
}

void ConfettiVisionApp::mouseDrag( MouseEvent event ){
    this->mousePosition = Vec2f(event.getX(),event.getY());
}

void ConfettiVisionApp::messageReceived(const osc::Message *m) {
    console() << m->getAddress() << endl;
    int channel = m->getArgAsInt32(0);
    int value = m->getArgAsInt32(1);
    
    ColorThreshold *thresholds [4] = {&(this->paramThresholdR), &(this->paramThresholdY), &(this->paramThresholdG), &(this->paramThresholdB)};
    ParticleColor colors [4] = {ParticleColor::RED, ParticleColor::YELLOW, ParticleColor::GREEN, ParticleColor::BLUE};
    ColorThreshold *threshold = thresholds[this->currentCalibrationColor % 4];
    ParticleColor color = colors[this->currentCalibrationColor % 4];

    if(value >= 127) {
        switch(channel) {
            case 45:    // RECORD
            {
                if (this->currentMode == Mode::PLAYBACK && this->paramCalibrate) {
                    if(this->camera.getCapturedFrames().size() > 0)
                        this->camera.saveBuffer("~/Desktop/balloon_" + toString(this->balloon->getID()));
                }
                else if(this->currentMode == Mode::CAPTURE) {
                    this->triggerAndCreateBalloon();
                }
                break;
            }
            case 41:    // PLAY
            {
                if(this->currentMode == Mode::PLAYBACK)
                    this->balloon->play();
                break;
            }
            case 42:    // STOP
            {
                if(this->currentMode == Mode::PLAYBACK)
                    this->balloon->pause();
                break;
            }
            case 43:    // REWIND
            {
                if(this->currentMode == Mode::PLAYBACK)
                    this->balloon->reset();
                break;
            }
            case 46:    // SET - set balloon playback start point
            {
                if(this->currentMode == Mode::PLAYBACK)
                    this->setMode(Mode::CAPTURE);
                else if(this->currentMode == Mode::CAPTURE)
                    this->setMode(Mode::PLAYBACK);
                break;
            }
            case 58:
            {
                this->autoplay = true;
                this->balloon->loadMovieFile(this->mMoviePaths[--this->autoplayIndex % this->mMoviePaths.size()]);
                break;
            }
            case 59:
            {
                this->autoplay = true;
                this->balloon->loadMovieFile(this->mMoviePaths[++this->autoplayIndex % this->mMoviePaths.size()]);
                break; 
            }
            case 32:
            {
                // SOLO RED
                if(this->soloColor == ParticleColor::RED) {
                    this->soloOn = false;
                }
                else {
                    this->soloColor = ParticleColor::RED;
                    this->soloOn = true;
                }
                break;
            }
            case 33:
            {
                // SOLO YELLOW
                if(this->soloColor == ParticleColor::YELLOW) {
                    this->soloOn = false;
                }
                else {
                    this->soloColor = ParticleColor::YELLOW;
                    this->soloOn = true;
                }
                break;
            }
            case 34:
            {
                // SOLO GREEN
                if(this->soloColor == ParticleColor::GREEN) {
                    this->soloOn = false;
                }
                else {
                    this->soloColor = ParticleColor::GREEN;
                    this->soloOn = true;
                }
                break;
            }
            case 35:
            {
                // SOLO BLUE
                if(this->soloColor == ParticleColor::BLUE) {
                    this->soloOn = false;
                }
                else {
                    this->soloColor = ParticleColor::BLUE;
                    this->soloOn = true;
                }
                break;
            }

        }
    }
    
    if(channel == 60) { // "set" button turns on/off calibration mode
        this->setCalibrate(value >= 127);
    }
    
    if(channel == 44) { // fast forward button turns on/off scrub mode
        this->paramScrub = (value >= 127);
    }
    
    if(this->paramScrub && this->currentMode == Mode::PLAYBACK && channel == 16) {
        float p = value - 64;
        p = p > 0 ? pow(p, 1.4) : -pow(abs(p), 1.4);
        int s = lmap((float)p, -338.0f, 338.0f, -24.0f, 24.0f);
        this->balloon->setSpeed(s);
        console() << s << endl;
    }
    
    if(this->paramCalibrate && this->currentMode == Mode::CAPTURE) {
        // Allow setting of the camera gain
        if(channel == 23) {
            this->paramGain = lmap((float)value, 0.0f, 127.0f, -3.4f, 7.4f);
            this->camera.setGain(this->paramGain);
        }
    }
    
    
    // Only set calibration if debugmode is on
    if(!this->paramCalibrate || !(this->currentMode == Mode::PLAYBACK))
        return;
    
    switch(channel) {
        case 61:
        {
            if(value < 127) break;
            // Calibrate PREVIOUS color
            this->currentCalibrationColor--;
            if(this->currentCalibrationColor < 0)
                this->currentCalibrationColor = 4 - this->currentCalibrationColor;
            break;
        }
        case 62:
        {
            if(value < 127) break;
            // Calibrate NEXT color
            this->currentCalibrationColor++;
            this->currentCalibrationColor %= 4;
            break;
        }
        case 0:
            threshold->h_min = value*2;
            this->balloon->particleCollections[color]->threshold = *threshold;
            break;
        case 1:
            threshold->h_max = value*2;
            this->balloon->particleCollections[color]->threshold = *threshold;
            break;
        case 2:
            threshold->l_min = value*2;
            this->balloon->particleCollections[color]->threshold = *threshold;
            break;
        case 3:
            threshold->l_max = value*2;
            this->balloon->particleCollections[color]->threshold = *threshold;
            break;
        case 4:
            threshold->s_min = value*2;
            this->balloon->particleCollections[color]->threshold = *threshold;
            break;
        case 5:
            threshold->s_max = value*2;
            this->balloon->particleCollections[color]->threshold = *threshold;
            break;
    }
    
}

void ConfettiVisionApp::setCalibrate(bool b) {
    this->paramCalibrate = b;
    this->balloon->setDebug(paramCalibrate);
}

void ConfettiVisionApp::triggerAndCreateBalloon() {
    this->camera.postTrigger();
    this->balloon = Balloon::create(this->camera.getCapturedFrames());
    this->balloon->messenger = &this->messenger;
    this->balloon->parent = this;
    this->setThresholds();
    this->setMode(Mode::PLAYBACK);
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
            this->autoplay = false;
            break;
        }
    }

}

void ConfettiVisionApp::update()
{
    // Handle any incoming messages
    while(this->controlListener.hasWaitingMessages()) {
        osc::Message m;
        this->controlListener.getNextMessage(&m);
        this->messageReceived(&m);
            
    }
    switch(this->currentMode) {
        case Mode::PLAYBACK:
        {
            if(this->scrubbingMouse) {
                this->balloon->seekFrame(this->mousePosition.x / getWindowWidth() * this->balloon->getFramecount());
                //this->balloon->pause();
            }
            this->balloon->update();
            if(this->autoplay && this->balloon->hasLooped() && this->balloon->getCurrentFrameNumber() <= 0) {
                this->balloon->loadMovieFile(this->mMoviePaths[++this->autoplayIndex % this->mMoviePaths.size()]);
            }
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
        Rectf cameraRect(getWindowWidth()/2 - this->cameraPreview->getWidth() * scale_factor / 2,
                              getWindowHeight()/2 - this->cameraPreview->getHeight() * scale_factor / 2,
                              getWindowWidth()/2 + this->cameraPreview->getWidth() * scale_factor / 2,
                              getWindowHeight()/2 + this->cameraPreview->getHeight() * scale_factor / 2);
//        Rectf cameraRect(getWindowWidth()/2 + this->cameraPreview->getWidth() * scale_factor / 2,
//                         getWindowHeight()/2 + this->cameraPreview->getHeight() * scale_factor / 2,
//                         getWindowWidth()/2 - this->cameraPreview->getWidth() * scale_factor / 2,
//                         getWindowHeight()/2 - this->cameraPreview->getHeight() * scale_factor / 2);
        gl::draw(this->cameraPreview, cameraRect);
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
    

    //float topPaneHeight = getWindowHeight()*0.75;
    float topPaneHeight = getWindowHeight() * this->topPanePercent;
    if(!this->drawMusicGraphics)
        topPaneHeight = getWindowHeight();

    // Draw the per-color preview channels
    if(this->drawMusicGraphics) {
        gl::pushMatrices();
            gl::translate(0, topPaneHeight);
            this->balloon->drawMusicGraphics(getWindowWidth(), getWindowHeight()  - topPaneHeight);
        gl::popMatrices();
    }
    
    
    // Draw the top info pane
    gl::color(COLOR_GREY);
    gl::drawSolidRect(Rectf(0,0, getWindowWidth(), topPaneHeight));
    
    
    gl::color(COLOR_WHITE);
    gl::Texture sourceVid = this->balloon->getTexture();
    Vec2f videoSize(640,480);
    if(sourceVid)
        videoSize = Vec2f(sourceVid.getWidth(), sourceVid.getHeight());
    float videoPadding = 0.1 * topPaneHeight;
    //float videoPadding = 0;
    float scale_factor = (topPaneHeight - videoPadding*2) / videoSize.y;
    float video_w = videoSize.x * scale_factor;
    float video_h = videoSize.y * scale_factor;
    float progressBarHeight = videoPadding / 4.0f;
    this->progressBarHeight = progressBarHeight;
    
    float channel_pane_w = (getWindowWidth() - video_w) / 2.0f;
    float channel_pane_h = topPaneHeight - progressBarHeight;
    float channel_padding_y = channel_pane_h / 32.0f;
    float channel_h = (channel_pane_h - channel_padding_y * 5) / 4.0f;
    float channel_w = channel_h * video_w / video_h;
    float channel_padding_x = (channel_pane_w - channel_w) / 4.0f;
    
    // FOUR UP SCALE CALCULATIONS
    if(this->fourUpChannels) {
        videoPadding = 0.02 * topPaneHeight;
        scale_factor = ((getWindowWidth() / 2) - (videoPadding * 2)) / videoSize.x;
        video_w = videoSize.x * scale_factor;
        video_h = videoSize.y * scale_factor;
        channel_pane_w = (getWindowWidth() / 2);
        channel_pane_h = topPaneHeight - progressBarHeight;
        channel_padding_x = channel_pane_w / 32.0f;
        channel_w = (channel_pane_w - (channel_padding_x * 3)) / 2.0f;
        channel_h = channel_w * video_h / video_w;
        channel_padding_y = channel_padding_x;
    }
    
    
    if(sourceVid) {
        gl::color(COLOR_DARKGREY);
        float leftShift = 0;
        if(this->paramCalibrate)
            leftShift = video_w/2;
        Rectf videoRect = Rectf(getWindowWidth() / 2 - video_w/2 + leftShift, videoPadding,
                                getWindowWidth() / 2 + video_w/2 + leftShift, videoPadding + video_h);
        if(this->fourUpChannels) {
            videoRect = Rectf(getWindowWidth() * 0.75 - video_w / 2, topPaneHeight / 2 - video_h / 2,
                              getWindowWidth() * 0.75 + video_w / 2, topPaneHeight / 2 + video_h / 2);
        }
//        Rectf videoRect = Rectf(getWindowWidth() / 2 + video_w/2 + leftShift, videoPadding + video_h,
//                                getWindowWidth() / 2 - video_w/2 + leftShift, videoPadding + 0);
        gl::drawSolidRect(videoRect.inflated(Vec2f(video_w * 0.01f, video_w * 0.01f)));
        gl::color(COLOR_WHITE);
        gl::draw(sourceVid, videoRect);
    }
    
    
    // Draw progress bar across top
    gl::color(COLOR_WHITE);
    gl::drawSolidRect(Rectf(0,0, getWindowWidth(), progressBarHeight));
    
    gl::color(COLOR_DARKGREY);
    gl::drawSolidRect(Rectf(0,0, getWindowWidth() * ((float)this->balloon->getCurrentFrameNumber() / this->balloon->getFramecount()), progressBarHeight));
    
    if(this->balloon->getPoppedFrame() > 0) {
        float x = this->balloon->getPoppedFrame() / (float)this->balloon->getFramecount() * getWindowWidth();
        gl::lineWidth(2);
        
        gl::color(COLOR_WHITE);
        gl::drawLine(Vec2f(x, 0), Vec2f(x, progressBarHeight*0.5));
        
        gl::color(COLOR_DARKGREY);
        gl::drawLine(Vec2f(x, progressBarHeight*0.5), Vec2f(x, progressBarHeight));
    }
    
    // Draw the info text
    gl::color(COLOR_WHITE);
    gl::pushMatrices();
    {
        gl::translate(videoPadding + getWindowWidth() / 2 + video_w/2, videoPadding * 2);
        drawString("BALLOON #" + toString(this->balloon->getID()), Vec2f(0,36), 36);
        char timecodeBuffer[255];
        snprintf(timecodeBuffer, 255, "%4.3f", (this->balloon->getCurrentFrameNumber() / 500.0f));
        if(this->balloon->isReady())
            drawString(string(timecodeBuffer) + " SECONDS", Vec2f(0, 70), 20);
        else {
            drawString("LOADING...", Vec2f(0, 70), 20);
        }
        drawString("FRAME " + toString(this->balloon->getCurrentFrameNumber()) + " / " + toString(this->balloon->getFramecount()), Vec2f(0, 90), 20);
    }
    gl::popMatrices();
    
    // Draw the individual channel previews
    gl::pushMatrices();
    {
        gl::pushMatrices();
        {
            ParticleColor colors [4] = {ParticleColor::RED, ParticleColor::YELLOW, ParticleColor::GREEN, ParticleColor::BLUE};
            ParticleColor calibrationColor = colors[this->currentCalibrationColor % 4];
            
            
            if(this->paramCalibrate) {
                switch(calibrationColor) {
                    case ParticleColor::RED:
                        gl::color(COLOR_RED);
                        break;
                    case ParticleColor::YELLOW:
                        gl::color(COLOR_YELLOW);
                        break;
                    case ParticleColor::GREEN:
                        gl::color(COLOR_GREEN);
                        break;
                    case ParticleColor::BLUE:
                        gl::color(COLOR_BLUE);
                        break;
                }
                float scale = this->fourUpChannels ? 2 : 4;
                gl::pushMatrices();
                gl::translate(channel_padding_x, channel_padding_y + progressBarHeight);
                gl::drawSolidRect(Rectf(-channel_w*scale*.02, -channel_h*scale*.02, channel_w *scale* 1.02,channel_h *scale* 1.02));
                
                gl::color(COLOR_WHITE);
                this->balloon->drawChannelGraph(calibrationColor, channel_w*4, channel_h*4);
                gl::popMatrices();
            }
            else if(this->fourUpChannels) {
                gl::pushMatrices();
                {
                    gl::translate(getWindowWidth() * 0.25 - channel_w - channel_padding_x/2,
                                  topPaneHeight / 2 - channel_h - channel_padding_y/2);
                    this->balloon->drawChannelGraph(ParticleColor::RED, channel_w, channel_h);
                }
                gl::popMatrices();
                gl::pushMatrices();
                {
                    gl::translate(getWindowWidth() * 0.25 + channel_padding_x/2,
                                  topPaneHeight / 2 - channel_h - channel_padding_y/2);
                    this->balloon->drawChannelGraph(ParticleColor::YELLOW, channel_w, channel_h);
                }
                gl::popMatrices();
                gl::pushMatrices();
                {
                    gl::translate(getWindowWidth() * 0.25 - channel_w - channel_padding_x/2,
                                  topPaneHeight / 2 + channel_padding_y/2);
                    this->balloon->drawChannelGraph(ParticleColor::GREEN, channel_w, channel_h);
                }
                gl::popMatrices();
                gl::pushMatrices();
                {
                    gl::translate(getWindowWidth() * 0.25 + channel_padding_x/2,
                                  topPaneHeight / 2 + channel_padding_y/2);
                    this->balloon->drawChannelGraph(ParticleColor::BLUE, channel_w, channel_h);
                }
                gl::popMatrices();
            }
            else {
                gl::translate(channel_padding_x, channel_padding_y + progressBarHeight);
                this->balloon->drawChannelGraph(ParticleColor::RED, channel_w, channel_h);
                gl::translate(0, channel_h + channel_padding_y);
                this->balloon->drawChannelGraph(ParticleColor::YELLOW, channel_w, channel_h);
                gl::translate(0, channel_h + channel_padding_y);
                this->balloon->drawChannelGraph(ParticleColor::GREEN, channel_w, channel_h);
                gl::translate(0, channel_h + channel_padding_y);
                this->balloon->drawChannelGraph(ParticleColor::BLUE, channel_w, channel_h);
            }
        }
        gl::popMatrices();
    }
    gl::popMatrices();
    
    
}

void ConfettiVisionApp::keyDown(cinder::app::KeyEvent event) {
    if(event.getChar() == 'f') {
        setFullScreen( ! isFullScreen() );
    }
    else if(event.getChar() == 't') {
//        if(this->camera.isRecording()) {
//            this->camera.stopTrigger();
//            this->setMode(Mode::REVIEW);
//        }
//        else {
//            this->camera.startTrigger();
//        }
        this->triggerAndCreateBalloon();
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
    this->balloon->loadMovieFile(path);
}

void ConfettiVisionApp::drawDebugString(const std::string &str, const Vec2f &baseline) {
    // Retina resolution!
    gl::color( ColorA( 0,0,0, 1.0f ) );
    debugTextureFont->drawString(str, baseline, gl::TextureFont::DrawOptions().scale( 0.5f ).pixelSnap( false ));
    gl::color( Color( 1, 1,1 ) );
}

void ConfettiVisionApp::drawString(const std::string &str, const ci::Vec2f &baseLine, int fontSize) {
    gl::SaveColorState s;
    gl::TextureFontRef font = textureFonts[fontSize];
    if(!font) return;
    // Retina resolution!
    gl::color( COLOR_DARKGREY );
    font->drawString(str, baseLine, gl::TextureFont::DrawOptions().scale( 0.5f ).pixelSnap( false ));
}


void ConfettiVisionApp::setupParams() {
    // Set up Params control panel
    // ---------------------------------------
   	this->params = params::InterfaceGl::create( getWindow(), "Settings", toPixels( Vec2i( 200, 250 ) ) );
    this->params->minimize();
    this->config = config::Config::create(this->params);
    
    params->addButton("Restart", [this]() { this->balloon->reset(); } );
    params->addButton("Pause ||", [this]() { this->balloon->pause(); } );
    params->addButton("Play [>]", [this]() { this->balloon->play(); } );
    params->addButton("Save Config", [this]() {
        this->config->save(fs::path(this->configFilename));
        console() << "Saved config to:" << ( fs::path(this->configFilename)) << endl;
    });
    params->addSeparator();
    params->addButton("File 1", [this]() { this->balloon->loadMovieFile(mMoviePaths[0]); } );
    params->addButton("File 2", [this]() { this->balloon->loadMovieFile(mMoviePaths[1]); } );
    params->addButton("File 3", [this]() { this->balloon->loadMovieFile(mMoviePaths[2]); } );
    params->addButton("File 4", [this]() { this->balloon->loadMovieFile(mMoviePaths[3]); } );
    params->addButton("File 5", [this]() { this->balloon->loadMovieFile(mMoviePaths[4]); } );
    params->addButton("File 6", [this]() { this->balloon->loadMovieFile(mMoviePaths[5]); } );
    params->addButton("File 7", [this]() { this->balloon->loadMovieFile(mMoviePaths[6]); } );
    params->addButton("File 8", [this]() { this->balloon->loadMovieFile(mMoviePaths[7]); } );
    params->addButton("File 9", [this]() { this->balloon->loadMovieFile(mMoviePaths[8]); } );
   
    params->addSeparator();
    params->addButton("CAPTURE", [this]() { this->setMode(Mode::CAPTURE); } );
    params->addButton("REVIEW", [this]() { this->setMode(Mode::REVIEW); } );
    params->addButton("PLAYBACK", [this]() { this->setMode(Mode::PLAYBACK); } );
    params->addParam("Calibrate", &paramCalibrate).updateFn([this] { this->setCalibrate(paramCalibrate);  });
    params->addSeparator();
    
    params->addParam("Top Pane %", &this->topPanePercent).min(0.01).max(1.0).step(0.01);
    params->addParam("Music Graphics", &this->drawMusicGraphics);
    params->addParam("4UP", &this->fourUpChannels);
    
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
    
    
    params->addButton("TRIGGER & SAVE", [this]() { this->triggerAndCreateBalloon(); } );
    //params->addButton("Trigger STOP", [this]() { this->camera.stopTrigger(); } );
    //params->addButton("Save Buffer", [this]() { this->camera.saveBuffer("~/Desktop/out.mpg"); } );
    params->addSeparator();
    
    config->newNode("Color thresholds");
    params->addSeparator();
    params->addText("Red");
    config->addParam("R Hue min", &paramThresholdR.h_min).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::RED]->threshold = this->paramThresholdR;
    });
    config->addParam("R Hue max", &paramThresholdR.h_max).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::RED]->threshold = this->paramThresholdR;
    });
    config->addParam("R Saturation min", &paramThresholdR.s_min).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::RED]->threshold = this->paramThresholdR;
    });
    config->addParam("R Saturation max", &paramThresholdR.s_max).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::RED]->threshold = this->paramThresholdR;
    });
    config->addParam("R Lightness min", &paramThresholdR.l_min).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::RED]->threshold = this->paramThresholdR;
    });
    config->addParam("R Lightness max", &paramThresholdR.l_max).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::RED]->threshold = this->paramThresholdR;
    });
    params->addSeparator();
    params->addText("Green");
    config->addParam("G Hue min", &paramThresholdG.h_min).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::GREEN]->threshold = this->paramThresholdG;
    });
    config->addParam("G Hue max", &paramThresholdG.h_max).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::GREEN]->threshold = this->paramThresholdG;
    });
    config->addParam("G Saturation min", &paramThresholdG.s_min).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::GREEN]->threshold = this->paramThresholdG;
    });
    config->addParam("G Saturation max", &paramThresholdG.s_max).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::GREEN]->threshold = this->paramThresholdG;
    });
    config->addParam("G Lightness min", &paramThresholdG.l_min).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::GREEN]->threshold = this->paramThresholdG;
    });
    config->addParam("G Lightness max", &paramThresholdG.l_max).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::GREEN]->threshold = this->paramThresholdG;
    });
    params->addSeparator();
    params->addText("Blue");
    config->addParam("B Hue min", &paramThresholdB.h_min).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::BLUE]->threshold = this->paramThresholdB;
    });
    config->addParam("B Hue max", &paramThresholdB.h_max).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::BLUE]->threshold = this->paramThresholdB;
    });
    config->addParam("B Saturation min", &paramThresholdB.s_min).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::BLUE]->threshold = this->paramThresholdB;
    });
    config->addParam("B Saturation max", &paramThresholdB.s_max).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::BLUE]->threshold = this->paramThresholdB;
    });
    config->addParam("B Lightness min", &paramThresholdB.l_min).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::BLUE]->threshold = this->paramThresholdB;
    });
    config->addParam("B Lightness max", &paramThresholdB.l_max).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::BLUE]->threshold = this->paramThresholdB;
    });
    params->addSeparator();
    params->addText("Yellow");
    config->addParam("Y Hue min", &paramThresholdY.h_min).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::YELLOW]->threshold = this->paramThresholdY;
    });
    config->addParam("Y Hue max", &paramThresholdY.h_max).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::YELLOW]->threshold = this->paramThresholdY;
    });
    config->addParam("Y Saturation min", &paramThresholdY.s_min).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::YELLOW]->threshold = this->paramThresholdY;
    });
    config->addParam("Y Saturation max", &paramThresholdY.s_max).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::YELLOW]->threshold = this->paramThresholdY;
    });
    config->addParam("Y Lightness min", &paramThresholdY.l_min).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::YELLOW]->threshold = this->paramThresholdY;
    });
    config->addParam("Y Lightness max", &paramThresholdY.l_max).min(0).max(255).updateFn([this] {
        this->balloon->particleCollections[ParticleColor::YELLOW]->threshold = this->paramThresholdY;
    });
    params->addSeparator();
    
    if(fs::exists(fs::path(this->configFilename) ))
        this->config->load(fs::path(this->configFilename));
}

CINDER_APP_NATIVE( ConfettiVisionApp, RendererGl( RendererGl::AA_MSAA_2 ) )
