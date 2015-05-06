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

    this->messenger = OSCMessenger(3000);
    this->oscListener.setup(3001);
    this->oscListener.registerMessageReceived([this](const osc::Message * m) { this->messageReceived(m); });
    
    this->balloon = Balloon::create();
    this->balloon->messenger = &this->messenger;
    
    if (!mMoviePaths.empty()) {
        this->balloon->loadMovieFile(mMoviePaths[0]);
    }
    
    // Load config and set up sliders
    this->setupParams();
    
    // Set saved thresholds now that config has loaded
    this->balloon->particleCollections[ParticleColor::RED]->threshold = this->paramThresholdR;
    this->balloon->particleCollections[ParticleColor::GREEN]->threshold = this->paramThresholdG;
    this->balloon->particleCollections[ParticleColor::BLUE]->threshold = this->paramThresholdB;
    this->balloon->particleCollections[ParticleColor::YELLOW]->threshold = this->paramThresholdY;
    
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
    
    ColorThreshold *thresholds [4] = {&(this->paramThresholdR), &(this->paramThresholdG), &(this->paramThresholdB), &(this->paramThresholdY)};
    ParticleColor colors [4] = {ParticleColor::RED, ParticleColor::GREEN, ParticleColor::BLUE, ParticleColor::YELLOW};
    ColorThreshold *threshold = thresholds[this->currentCalibrationColor % 4];
    ParticleColor color = colors[this->currentCalibrationColor % 4];
    
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
            this->balloon->update();
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

    
    gl::Texture redTex = this->balloon->particleCollections[ParticleColor::RED]->getDebugView();
    gl::Texture greenTex = this->balloon->particleCollections[ParticleColor::GREEN]->getDebugView();
    gl::Texture blueTex = this->balloon->particleCollections[ParticleColor::BLUE]->getDebugView();
    gl::Texture yellowTex = this->balloon->particleCollections[ParticleColor::YELLOW]->getDebugView();
    
    int offset_x = 220;
    int offset_y = 20;
    float s = 0.95f;
    int textHeight = 12;
    
    if(redTex && greenTex && blueTex && yellowTex) {
        float scale_factor = ((float)getWindowWidth()-offset_x) / redTex.getWidth() / 4.0f;
        float w = redTex.getWidth() * scale_factor;
        float h = redTex.getHeight() * scale_factor;

        gl::draw(redTex, Rectf(offset_x + w * 0,offset_y, offset_x + w * s + w * 0, offset_y + h * s));
        //drawDebugString("Points:       " + to_string(redParticles->getActiveParticleCount()), Vec2f(offset_x+w*0 + w*(1-s)/2,offset_y + h*s + textHeight));
        gl::draw(greenTex, Rectf(offset_x + w * 1,offset_y, offset_x + w * s + w * 1, offset_y + h * s));
        //drawDebugString("Points:       " + to_string(greenParticles->getActiveParticleCount()), Vec2f(offset_x+w*1 + w*(1-s)/2,offset_y + h*s + textHeight));
        gl::draw(blueTex, Rectf(offset_x + w * 2,offset_y, offset_x + w * s + w * 2, offset_y + h * s));
        //drawDebugString("Points:       " + to_string(blueParticles->getActiveParticleCount()), Vec2f(offset_x+w*2 + w*(1-s)/2,offset_y + h*s + textHeight));
        gl::draw(yellowTex, Rectf(offset_x + w * 3,offset_y, offset_x + w * s + w * 3, offset_y + h * s));
        //drawDebugString("Points:       " + to_string(yellowParticles->getActiveParticleCount()), Vec2f(offset_x+w*3 + w*(1-s)/2,offset_y + h*s + textHeight));
    }
    
    gl::Texture sourceVid = this->balloon->getTexture();
    if(sourceVid) {
        float scale_factor = 600.0f / sourceVid.getWidth();
        float w = sourceVid.getWidth() * scale_factor;
        float h = sourceVid.getHeight() * scale_factor;
        gl::draw(sourceVid, Rectf(getWindowWidth() / 2 - w/2, getWindowHeight()-h, getWindowWidth() / 2 + w/2, getWindowHeight()));
    }
 
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
    this->balloon->loadMovieFile(path);
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
    params->addButton("File 10", [this]() { this->balloon->loadMovieFile(mMoviePaths[9]); } );
    params->addButton("File 11", [this]() { this->balloon->loadMovieFile(mMoviePaths[10]); } );
    params->addButton("File 12", [this]() { this->balloon->loadMovieFile(mMoviePaths[11]); } );
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

CINDER_APP_NATIVE( ConfettiVisionApp, RendererGl( RendererGl::AA_NONE ) )
