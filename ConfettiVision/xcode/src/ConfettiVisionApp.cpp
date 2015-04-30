#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/qtime/QuickTime.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ConfettiVisionApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    void fileDrop( FileDropEvent event );
    void loadMovieFile( const fs::path &path );
    
    gl::Texture         mFrameTexture;
    qtime::MovieGlRef   mMovie;
};

void ConfettiVisionApp::setup()
{
    fs::path moviePath = getOpenFilePath();
    if (!moviePath.empty()) {
        loadMovieFile(moviePath);
    }
}

void ConfettiVisionApp::mouseDown( MouseEvent event )
{
}

void ConfettiVisionApp::update()
{
    if(mMovie) {
        mFrameTexture = mMovie->getTexture();
    }
}

void ConfettiVisionApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 1.0f, 0 ) );
}

void ConfettiVisionApp::fileDrop(FileDropEvent event)
{
    loadMovieFile(event.getFile(0));
}

void ConfettiVisionApp::loadMovieFile(const fs::path &path)
{
    try {
        mMovie = qtime::MovieGl::create(path);
        mMovie->setLoop();
        mMovie->play();
    }
    catch(...) {
        console() << "Unable to load movie file!" << std::endl;
        mMovie->reset();
    }
    
    mFrameTexture.reset();
}

CINDER_APP_NATIVE( ConfettiVisionApp, RendererGl )
