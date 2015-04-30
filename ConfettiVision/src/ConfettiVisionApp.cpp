#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/qtime/QuickTime.h"
#include "CinderOpenCv.h"
#include "OscSender.h"
#include "cinder/params/Params.h"
#include "cinder/System.h"

#include <thread>

#include "ParticleCollection.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class ConfettiVisionApp : public AppNative {
  public:
	void setup();
    void prepareSettings(Settings *settings);
	void mouseDown( MouseEvent event );
    void keyDown( KeyEvent event );
	void update();
	void draw();
    
    void fileDrop( FileDropEvent event );
    void loadMovieFile( const fs::path &path );
    
    void drawDebugString(const std::string &str, const Vec2f &baseline);
    
    void sendMessagesForParticle(Particle &p, int channel);
    
    cv::Mat getHLSImage(Surface surface);
    
    gl::Texture         mFrameTexture;
    Surface             mFrameSurface;
    //qtime::MovieGlRef   mMovie;
    //qtime::MovieSurfaceRef mMovie;
    cv::VideoCapture    cvMovie;
    Font debugFont;
    gl::TextureFontRef debugTextureFont;
    
  	params::InterfaceGlRef	mParams;
    
    std::vector<fs::path> mMoviePaths;
    
    ParticleCollectionRef redParticles, greenParticles, blueParticles, yellowParticles;
    
    osc::Sender oscSender;
    
    bool poppedYet = false;
    int poppedStartThreshold = 5;
    int poppedEndThreshold = 3;
    
};
void ConfettiVisionApp::prepareSettings(Settings * settings) {
    settings->enableHighDensityDisplay( true );
    settings->setWindowSize(1280, 720);
    settings->setWindowPos(0, 0);
}

void ConfettiVisionApp::setup()
{

    setFrameRate( 60.0f );
    
    this->debugFont = Font("Letter Gothic Std Bold", 12);
    this->debugTextureFont = gl::TextureFont::create(Font("Letter Gothic Std Bold", 12*2));
    
    // Set up Movie
    // -------------------------------------
    //fs::path moviePath = getOpenFilePath();
    mMoviePaths.push_back(fs::path("/Users/kronick/Documents/💾🌵/projects/google sound burst/test vids/640x480_2.mp4"));
    mMoviePaths.push_back(fs::path("/Users/kronick/Documents/💾🌵/projects/google sound burst/test vids/640x480_3.mp4"));
    mMoviePaths.push_back(fs::path("/Users/kronick/Documents/💾🌵/projects/google sound burst/test vids/640x480.mp4"));
    mMoviePaths.push_back(fs::path("/Users/kronick/Documents/💾🌵/projects/google sound burst/test vids/640x480_4.mp4"));
    mMoviePaths.push_back(fs::path("/Users/kronick/Documents/💾🌵/projects/google sound burst/test vids/sorted-colors.png"));

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
    
    // Set up Params control panel
    // ---------------------------------------
   	mParams = params::InterfaceGl::create( getWindow(), "App parameters", toPixels( Vec2i( 200, 650 ) ) );
    //mParams->addButton("Restart", [this]() { mMovie->seekToStart(); } );
    //mParams->addButton("Pause ||", [this]() { mMovie->stop(); } );
    //mParams->addButton("Play [>]", [this]() { mMovie->play(); } );
    mParams->addButton("File 1", [this]() { loadMovieFile(mMoviePaths[0]); } );
    mParams->addButton("File 2", [this]() { loadMovieFile(mMoviePaths[1]); } );
    mParams->addButton("File 3", [this]() { loadMovieFile(mMoviePaths[2]); } );
    mParams->addButton("File 4", [this]() { loadMovieFile(mMoviePaths[3]); } );
    mParams->addButton("File 5", [this]() { loadMovieFile(mMoviePaths[4]); } );
    mParams->addSeparator();
    mParams->addText("REDS");
    mParams->addParam("R Hue min", &(redParticles->threshold.h_min));
    mParams->addParam("R Hue max", &(redParticles->threshold.h_max));
    mParams->addParam("R Saturation min", &(redParticles->threshold.s_min));
    mParams->addParam("R Saturation max", &(redParticles->threshold.s_max));
    mParams->addParam("R Lightness min", &(redParticles->threshold.l_min));
    mParams->addParam("R Lightness max", &(redParticles->threshold.l_max));
    mParams->addSeparator();
    mParams->addText("Green");
    mParams->addParam("G Hue min", &(greenParticles->threshold.h_min));
    mParams->addParam("G Hue max", &(greenParticles->threshold.h_max));
    mParams->addParam("G Saturation min", &(greenParticles->threshold.s_min));
    mParams->addParam("G Saturation max", &(greenParticles->threshold.s_max));
    mParams->addParam("G Lightness min", &(greenParticles->threshold.l_min));
    mParams->addParam("G Lightness max", &(greenParticles->threshold.l_max));
    mParams->addSeparator();
    mParams->addText("Blue");
    mParams->addParam("B Hue min", &(blueParticles->threshold.h_min));
    mParams->addParam("B Hue max", &(blueParticles->threshold.h_max));
    mParams->addParam("B Saturation min", &(blueParticles->threshold.s_min));
    mParams->addParam("B Saturation max", &(blueParticles->threshold.s_max));
    mParams->addParam("B Lightness min", &(blueParticles->threshold.l_min));
    mParams->addParam("B Lightness max", &(blueParticles->threshold.l_max));
    mParams->addSeparator();
    mParams->addText("Yellow");
    mParams->addParam("Y Hue min", &(yellowParticles->threshold.h_min));
    mParams->addParam("Y Hue max", &(yellowParticles->threshold.h_max));
    mParams->addParam("Y Saturation min", &(yellowParticles->threshold.s_min));
    mParams->addParam("Y Saturation max", &(yellowParticles->threshold.s_max));
    mParams->addParam("Y Lightness min", &(yellowParticles->threshold.l_min));
    mParams->addParam("Y Lightness max", &(yellowParticles->threshold.l_max));
    mParams->addSeparator();


    
    int port = 3000;
	// assume the broadcast address is this machine's IP address but with 255 as the final value
	// so to multicast from IP 192.168.1.100, the host should be 192.168.1.255
	string host = System::getIpAddress();
	//if( host.rfind( '.' ) != string::npos )
	//	host.replace( host.rfind( '.' ) + 1, 3, "255" );
	oscSender.setup( "localhost", port, true );
    
}

void ConfettiVisionApp::mouseDown( MouseEvent event )
{
}

void ConfettiVisionApp::update()
{
    static int curr_frame = 0;
    
    if(this->cvMovie.isOpened()) {
        //mFrameSurface = mMovie->getSurface();
        cv::Mat cvFrame;
        // Loop movie if at the end
        //console() << this->cvMovie.get(CV_CAP_PROP_POS_FRAMES) << "/" << this->cvMovie.get(CV_CAP_PROP_FRAME_COUNT) << endl;
        int movieCurrentFrame = cvMovie.get(CV_CAP_PROP_POS_FRAMES);
        int movieFrameCount =   cvMovie.get(CV_CAP_PROP_FRAME_COUNT);
        bool isStill = movieFrameCount == 1;
        if(movieCurrentFrame == movieFrameCount && !isStill)
            this->cvMovie.set(CV_CAP_PROP_POS_FRAMES, 0);
        
        if(!isStill)
            this->cvMovie >> cvFrame;
        else {
            if(!cvMovie.retrieve(cvFrame)) {
                cvMovie.read(cvFrame);
            }
        }
        this->mFrameSurface = fromOcv(cvFrame);
        if(this->mFrameSurface) {
            //mFrameTexture = gl::Texture(mFrameSurface);
            cv::Mat frame = getHLSImage(this->mFrameSurface);
            // Process each color in its own thread
            std::thread redThread(&ParticleCollection::findParticles, redParticles, ref(frame));
            std::thread greenThread(&ParticleCollection::findParticles, greenParticles, ref(frame));
            std::thread blueThread(&ParticleCollection::findParticles, blueParticles, ref(frame));
            std::thread yellowThread(&ParticleCollection::findParticles, yellowParticles, ref(frame));
          
            redThread.join();
            greenThread.join();
            blueThread.join();
            yellowThread.join();
            
            
            int n_particles =  greenParticles->getActiveParticleCount() + redParticles->getActiveParticleCount();
            
            if(n_particles > poppedStartThreshold && !poppedYet) {
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
            else if(n_particles <= poppedEndThreshold && poppedYet) {
                // Reset
                poppedYet = false;
                console() << "NEW BALLOON" << endl;
            }

            if(poppedYet) {
                
                // Figure out OSC messages to send for this particle
                for(auto &p : blueParticles->getParticles()) {
                    sendMessagesForParticle(*p, 0);
                }
                for(auto &p : greenParticles->getParticles()) {
                    sendMessagesForParticle(*p, 1);
                }
                for(auto &p : redParticles->getParticles()) {
                    sendMessagesForParticle(*p, 2);
                }
                for(auto &p : yellowParticles->getParticles()) {
                    sendMessagesForParticle(*p, 3);
                }
            }
            
            
        }
    }
    
    //console() << getAverageFps() << std::endl;
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
            
            int n_octaves = 4;
            int v = (p.position.y / (float)mFrameSurface.getHeight()) * 5 * n_octaves + 3;
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
                    n = 4;
                    break;
                case 2:
                    n = 5;
                    break;
                case 3:
                    n = 9;
                    break;
                case 4:
                    n = 10;
                    break;
            }
            //float volume = 1 -  p.position.y / (float)mFrameSurface.getHeight();
            n += 40;
            n += 12*octave;
            float volume = p.velocity.length() + p.freshness;
            float pan = 2*(p.position.x - mFrameSurface.getWidth()/2) / mFrameSurface.getWidth();
            message.addFloatArg(n);         // Note
            message.addFloatArg(volume);      // Volume
            message.addFloatArg(pan);
        }
        break;
            
        case 1:
        {
            // Trigger some glitchy samples
            //if(p.age % 4 != 0) return;
            // This is a NEW note for Max to track
            message.setAddress("/sampler");
            message.addIntArg(p.ID);
            p.hasNoteBeenSent = true;
            
            
            float offset = p.position.y / (float)mFrameSurface.getHeight();
            //float speed = (p.position.x - (float)mFrameSurface.getWidth()/2) / mFrameSurface.getWidth() * 10;
            float speed = p.position.distance(Vec2f(mFrameSurface.getWidth() / 2, mFrameSurface.getHeight()/2)) / 100.0f;
            //float speed = p.position.x / (float)mFrameSurface.getWidth() * 2;
            float length = 1- min(p.velocity.length() / 100.0f, 1.0f);
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
            if(p.hasNoteBeenSent || p.position.y < mFrameSurface.getHeight() * .9) return;
            
            // Only send if this particle hasn't triggered before.
            message.setAddress("/pop");
            //message.addIntArg(p.ID);
            p.hasNoteBeenSent = true;


            
            
            //int index = p.position.y / (float)mFrameSurface.getHeight() * 5;
            int index = 3;
            float pitch = 2*(p.position.x - mFrameSurface.getWidth()/2) / mFrameSurface.getWidth() * 10;
            //float pitch = 0;
            float pan = 2*(p.position.x - mFrameSurface.getWidth()/2) / mFrameSurface.getWidth();
            
            //float volume = p.velocity.length() / 3.0f + 0.1f;
            float volume = 1.0f;
            //length = .1;
            
            
            message.addIntArg(index);
            message.addFloatArg(pitch);
            message.addFloatArg(volume);
            message.addFloatArg(pan);
        }
        break;
      
    }
    oscSender.sendMessage(message);
    
}

cv::Mat ConfettiVisionApp::getHLSImage(Surface surface) {
    cv::Mat frame(toOcv(surface));
    cv::cvtColor(frame, frame, CV_RGB2HLS);

    return frame;
}

void ConfettiVisionApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0.2f, 0.2f, 0.2f ) );
    gl::enableAlphaBlending();
    
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
    
    // Draw the params control interface
	mParams->draw();
    
    drawDebugString(to_string(getAverageFps()), Vec2f(50, getWindowHeight() - 12));
}

void ConfettiVisionApp::keyDown(cinder::app::KeyEvent event) {
    if(event.getChar() == 'f') {
        setFullScreen( ! isFullScreen() );
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


void ConfettiVisionApp::drawDebugString(const std::string &str, const Vec2f &baseline) {
    // Retina resolution!
    debugTextureFont->drawString(str, baseline, gl::TextureFont::DrawOptions().scale( 0.5f ).pixelSnap( false ));
}

CINDER_APP_NATIVE( ConfettiVisionApp, RendererGl )
