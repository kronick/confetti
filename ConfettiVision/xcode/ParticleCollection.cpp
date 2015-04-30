//
//  ParticleCollection.cpp
//  ConfettiVision
//
//  Created by Sam Kronick on 4/23/15.
//
//

#include "ParticleCollection.h"
using namespace std;



ParticleCollection::ParticleCollection(const char * name) {
    this->name = string(name);
    this->particles = vector<ParticleRef>();
    
    // Choose a random debug color
    cv::RNG rng(time(NULL));
    setDebugColor(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
    
}

void ParticleCollection::setDebugColor(int r, int g, int b) {
    this->debugColor = cv::Scalar(r, g, b);
}

void ParticleCollection::findParticles(const cv::Mat &frame) {
    // TODO:
    // [x] Set up n HLS ranges to mask video & identify each color of confetti
    // [x] Run a blob detector on each (either with SimpleBlobDetector or manually with findContours)
    // [x] Calculate center of each blob
    // [ ] Find nearest neighbor in previous frame for each blob-- try to correspond to continuous motion
    // [ ] If no neighbor in previous frame is found, create new point (trigger new musical event)
    // [ ] Also trigger events for points that disappear for ~2 frames and remove them from the list
    // [ ] Maybe point management/event triggering should happen elsewhere instead?
    
    cv::Mat mask;
    
    // Threshold the incoming frame
    cv::inRange(frame,
                cv::Scalar(this->threshold.h_min, this->threshold.l_min, this->threshold.s_min),
                cv::Scalar(this->threshold.h_max, this->threshold.l_max, this->threshold.s_max),
                mask);
    
    // Optional erod & blur to make things look a bit nicer
    cv::dilate(mask, mask, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5)));
    //cv::medianBlur(mask, mask, 5);
    
    // Find contour polygons
    vector<vector<cv::Point> > contours;
    vector<cv::Point> contour_centers;
    cv::findContours(mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    
    // Delete the ones that are too big or small
    for(auto contour = contours.begin(); contour != contours.end();) {
        double area = cv::contourArea(*contour);
        if(area > particleAreaMax || area < particleAreaMin) {
            contour = contours.erase(contour);
        }
        else {
            ++contour;
        }
    }
    
    // Calculate centers of mass
    for(auto &contour : contours) {
        cv::Rect bounds = cv::boundingRect(contour);
        contour_centers.push_back(cv::Point(bounds.x + bounds.width/2, bounds.y + bounds.height/2));
    }
    
    // Run through list of particles and find correspondences
    // ------------------------------------------------------
    // First update the old particle positions & lifetimes
    for(auto p = this->particles.begin(); p != this->particles.end();) {
        (*p)->update();
        
        // Clean out dead particles
        if((*p)->isAlive()) {
            ++p;
        }
        else {
            p = this->particles.erase(p);
        }
    }
    
    movedParticles = vector<ParticleRef>();
    newParticles = vector<ParticleRef>();
    for(auto &found_point : contour_centers) {
        ci::Vec2f found_vec(found_point.x, found_point.y);
        bool match_found = false;
        for(auto &p : this->particles) {
            // IF this particle hasn't already had a match since its last update
            // AND it is within the threshold distance of the current test particle
            // THEN update its position
            if(p->freshness < 1 && p->position.distanceSquared(found_vec) < this->particleSeparationThreshold) {
                p->setPosition(found_vec);
                this->movedParticles.push_back(p);
                match_found = true;
                break;
            }
        }
        
        if(!match_found) {
            // Create a new particle
            ParticleRef p = Particle::create(found_vec);
            this->particles.push_back(p);
            this->newParticles.push_back(p);
            
        }
        
    }

    
    if(this->debugOn) {
        // Create debug view
        this->debugView = cv::Mat::zeros(480, 640, CV_8UC3);
//        frame.copyTo(this->debugView, mask);
//        cv::cvtColor(this->debugView, this->debugView, CV_HLS2RGB);

//        for(auto &point : contour_centers) {
//            cv::circle(debugView, point, 3, this->debugColor, -1);
//        }
//        
//        for(int i = 0; i< contours.size(); i++) {
//                cv::drawContours( debugView, contours, i, this->debugColor, 3);
//        }

        for(auto &p : this->particles) {
//            cv::circle(debugView, cv::Point(p->position.x, p->position.y), 8 * p->freshness, this->debugColor, -1);
            if(p->isActive()) {
                cv::circle(debugView, cv::Point(p->position.x, p->position.y), .05 * p->age, this->debugColor, -1);
                //cv::line(debugView, cv::Point(p->position.x, p->position.y), cv::Point(p->position.x - p->velocity.x*10, p->position.y - p->velocity.y*10), cv::Scalar(255,255,255), 1, CV_AA);
                for(int i=0; i<p->positionHistory.size() - 1; i++) {
                    cv::line(debugView, cv::Point(p->positionHistory[i].x, p->positionHistory[i].y), cv::Point(p->positionHistory[i+1].x, p->positionHistory[i+1].y), cv::Scalar(255,255,255), 1, CV_AA);
                }
            }
        }
        
        
    }
    
}


int ParticleCollection::getActiveParticleCount() {
    int n = 0;
    for(auto &p : particles) {
        if(p->isActive()) n++;
    }
    return n;
}