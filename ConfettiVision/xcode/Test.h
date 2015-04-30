//
//  Test.h
//  ConfettiVision
//
//  Created by Sam Kronick on 4/24/15.
//
//

#pragma once
#include <memory>

class Test;
typedef std::shared_ptr<Test> TestRef;

class Test {
  public:
    // Smart Pointer Constructors
    static TestRef create() { return std::make_shared<Test>(); }

  public:
    // Constructors
    Test() {}
    
};