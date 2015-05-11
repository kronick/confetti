//
//  ParticleCollection.cpp
//  ConfettiVision
//
//  Created by Sam Kronick on 4/23/15.
//
//

#include "ParticleCollection.h"
using namespace std;



ParticleCollection::ParticleCollection(ParticleColor color) {
    this->color = color;
    this->particles = vector<ParticleRef>();
    
    // Choose a random debug color
    cv::RNG rng(time(NULL));
    setDebugColor(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
    
}

void ParticleCollection::setDebugColor(int r, int g, int b) {
    this->debugColor = cv::Scalar(r, g, b);
}

void ParticleCollection::resetParticles() {
    this->particles.clear();
    this->newParticles.clear();
    this->movedParticles.clear();
    this->largestParticleArea = 0;
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
    
    if(frame.rows == 0) return;
    
    vector<cv::Mat> channels;
 
    cv::split(frame, channels);
    
    cv::Mat mask = channels[0];
    
//    // Threshold the incoming frame
//    cv::inRange(frame,
//                cv::Scalar(this->threshold.h_min, this->threshold.l_min, this->threshold.s_min),
//                cv::Scalar(this->threshold.h_max, this->threshold.l_max, this->threshold.s_max),
//                mask);
    
    // Optional erod & blur to make things look a bit nicer
    cv::dilate(mask, mask, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5)));
    //cv::medianBlur(mask, mask, 5);
    
    // Find contour polygons
    vector<vector<cv::Point> > contours;
    vector< vector<ci::Vec2f> >contourVectors;
    vector<cv::Point> contour_centers;
    cv::findContours(mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    
    // Delete the ones that are too big or small
    this->largestParticleArea = 0;
    for(auto contour = contours.begin(); contour != contours.end();) {
        double area = cv::contourArea(*contour);
        this->largestParticleArea = max(this->largestParticleArea, (int)area);
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
        
        vector<ci::Vec2f> points;
        for(auto &p : contour) {
            points.push_back(ci::Vec2f(p.x, p.y));
        }
        
        contourVectors.push_back(points);
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

    int i =0;
    for(auto &found_point : contour_centers) {
        ci::Vec2f found_vec(found_point.x, found_point.y);
        bool match_found = false;
        float closestDistance = numeric_limits<float>::max();
        ParticleRef closestParticle;
        for(auto &p : this->particles) {
            // Find the closest particle within range that hasn't already been updated this cycle
            if(p->freshness < 1) {
                float distance = p->position.distanceSquared(found_vec);
                if(distance < this->particleSeparationThreshold && distance < closestDistance) {
                    closestDistance = distance;
                    closestParticle = p;
                    match_found = true;
                }
            }
        }
        if(match_found) {
            closestParticle->setPosition(found_vec);
            closestParticle->setContour(contourVectors[i]);
            this->movedParticles.push_back(closestParticle);
        }
        
        // Add new particle if it's not in the junk zone
        if(!match_found && found_point.y > this->deadZoneTop) {
            // Create a new particle
            ParticleRef p = Particle::create(found_vec, this->color);
            p->setContour(contourVectors[i]);
            this->particles.push_back(p);
            this->newParticles.push_back(p);
            
        }
        
        i++;
    }

    
    if(this->debugOn) {
        // Create debug view
        this->debugView = cv::Mat::zeros(480, 640, CV_8UC3);
        cv::rectangle(this->debugView, cv::Rect(0,0, 640,480), cv::Scalar(230,230,230), -1);
        frame.copyTo(this->debugView, mask);
        
        return;
    }
    
}


int ParticleCollection::getActiveParticleCount() {
    int n = 0;
    for(auto &p : particles) {
        if(p->isActive()) n++;
    }
    return n;
}