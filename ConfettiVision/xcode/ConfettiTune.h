//
//  ConfettiTune.h
//  ConfettiVision
//
//  Created by Sam Kronick on 4/28/15.
//
//

#pragma once
#include <memory>
//#include "cinder/qtime/QuickTime.h"

class ConfettiTune;
typedef std::shared_ptr<ConfettiTune> ConfettiTuneRef;

class ConfettiTune {
  public:
    // Smart Pointer Constructors
    static ConfettiTuneRef create() { return std::make_shared<ConfettiTune>(); }

  public:
    // Constructors
    ConfettiTune() {}
    
  private:
    std::string                     movieFilename;
    std::string                     audioFilename;
    //cinder::qtime::MovieSurfaceRef  movie;

    
};