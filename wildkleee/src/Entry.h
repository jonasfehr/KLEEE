//
//  Entry.h
//  wildkleee
//
//  Created by Jonas Fehr on 20/08/2019.
//

#ifndef Entry_h
#define Entry_h

#include "ContentBase.hpp"
#include "Zone.hpp"
#include "I_want_to_say_something.h"

class Entry : public I_want_to_say_something{
public:
    void setup(vector<shared_ptr<Zone>> &zones){
        this->pZones = &zones;
        
        I_want_to_say_something::setup();
        
        outIndex = 0;
        generateNew();
    }
    void generateNew(){
        int zIndex = ofRandom(pZones->size());
        currentZone = pZones->at(zIndex);
        
        processedWords.clear();
    
//        vector<unique_ptr<ConnectedWord>> newEntry;
        
        float zoneNL = currentZone->getNormLength();
        float sumNL = 0;
        float spacing = 0.5;
        
        for( ConnectedWord cW = getNextWord(); zoneNL > sumNL+cW.getNormLength(); cW = getNextWord()){
//            cout<<cW.word <<" - "<<cW.enumWord<< endl;
            ProcessedWord processedWord;
            for(auto & poly : cW.svgLoader.outlines){
                poly.translate(glm::vec2(sumNL, 0));
                processedWord.polys.push_back(poly);
            }
            sumNL += cW.getNormLength();
            sumNL+=spacing; // space between Words
            processedWord.word = cW.word;
            processedWords.push_back(processedWord);
        }
        sumNL-=spacing;
        
        if(processedWords.back().word == "TO"){
            processedWords.pop_back();
            sumNL -= words[2].getNormLength();
        }
        
        processedWords.push_back(*new ProcessedWord()); // to create break after an entry


        // Center
        float offset = (zoneNL-sumNL)/2;
        for(auto & pW : processedWords){
            for(auto & poly : pW.polys){
                poly.translate(glm::vec2(offset, 0));
            }
        
            // Normalize both axis
            for(auto & poly : pW.polys){
                poly.scale(1/zoneNL, 1);
            }
            
            
            // MAP to Zone and add Points
            for(auto & poly : pW.polys){
                for(auto & point : poly){
                    point = currentZone->map(point);
                }
                poly = poly.getResampledBySpacing(resSpacing.get());
                for(auto & point : poly){
                    pW.contentPoints.addPoint(point, 1.);
                }
                pW.contentPoints.closePointGroup();
            }
        }
    }
    
    void draw(ofRectangle rect){
        ofPushMatrix();
        ofTranslate(rect.getTopLeft());
        ofScale(rect.getWidth(), rect.getHeight());

//        ofScale(20, 20);
        ofSetColor(255);
        if(processedWords.size()>0){
            for(auto & poly : processedWords[outIndex].polys){
                poly.draw();
            }
        }
        ofPopMatrix();
    }
    
    void next(){
        outIndex++;
        if(outIndex>= processedWords.size()){
            generateNew();
            outIndex = 0;
        }
    }
    
    vector<vector<ofxIlda::Point>> getPointGroups(){
        return processedWords[outIndex].contentPoints.getPointGroups();
    }
    
    
    vector<shared_ptr<Zone>> *pZones;
    shared_ptr<Zone> currentZone;

    struct ProcessedWord{
        vector<ofPolyline> polys;
        ContentBase contentPoints;
        string word;
    };
    
    vector<ProcessedWord> processedWords;

    int outIndex;
    
    ofParameter<float> resSpacing{"resSpacing", 0.005, 0.001, 0.01};
    ofParameterGroup parameters{"Entry", resSpacing};
};

#endif /* Entry_h */
