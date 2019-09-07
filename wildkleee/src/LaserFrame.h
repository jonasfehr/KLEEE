//
//  LaserFrame.h
//  wildkleee
//
//  Created by Jonas Fehr on 20/08/2019.
//

#ifndef LaserFrame_h
#define LaserFrame_h

#include "ContentBase.hpp"
#include "Zone.hpp"
#include "I_want_to_say_something.h"

class LaserFrame : public I_want_to_say_something{
public:
    void setup(vector<shared_ptr<Zone>> &zones, string path = "fonts/martina/"){
        this->pZones = &zones;
        
        I_want_to_say_something::setup();
        
        
        loadSvg(path);
        
        switchZone();
        createNewCombination();
        shortenIfTooLongForZone();
        
    }
    
    void switchZone(){
        int newIndex = ofRandom(pZones->size());
        float nL = svgContent[getKey()].getNormLength();
        float zoneNL = pZones->at(newIndex)->getNormLength();

        while(newIndex == zoneIndex || nL > zoneNL){ // not repeat, and current Word fits;
            newIndex = ofRandom(pZones->size());
            zoneNL = pZones->at(newIndex)->getNormLength();
        }
        zoneIndex = newIndex;
        pCurrentZone = pZones->at(zoneIndex);
    }
    
    void newFrame(){
        next();
        if(repeatIndex > 0) {
            if(isBreak()){
                repeatIndex -= 1;
                mutate(chanceToShorten, chanceToExtend);
                processedWords.clear();
                resetIndex();
                return;
            }
        } else {
            if(isBreak()){
                switchZone();
                createNewCombination();
                processedWords.clear();
                resetIndex();
                if(ofRandom(100) < chanceRepeat){
                    repeatIndex = minRepeat + ofRandom(maxRepeat-minRepeat);
                }
                return;
            }
        }
        
        processCurrentWord();
        
        while(processedWords.size()>numOfWords){
            processedWords.erase(processedWords.begin());
        }
        
    }
    
    void shortenIfTooLongForZone(){
        int indexToShorten = 0;
        float zoneNL = pCurrentZone->getNormLength();
        float sumNL = 0;

        for(auto & c : getCombinationKeys()){
            sumNL += svgContent[c].getNormLength();
            if(sumNL<zoneNL) indexToShorten++;
            sumNL += spacing;
        }
        
        shortenCombinationToIndex(indexToShorten);
    }
    
    void processCurrentWord(){
        ProcessedWord processedWord;
        float zoneNL = pCurrentZone->getNormLength();

        if(oneZoneOneWord){
            
            switchZone();
            zoneNL = pCurrentZone->getNormLength();

            float offset = ofRandom(zoneNL-svgContent[getKey()].getNormLength());
            for(auto & poly : svgContent[getKey()].outlines){
                ofPolyline p = poly;
                p.translate(glm::vec2(offset, 0)); // Center
                processedWord.polys.push_back(p);
            }
            
        }else{
            
            shortenIfTooLongForZone();
            
            float offset = 0;
            float totalLength = 0;
            
            for(int i = 0; i < getCombinationLength(); i++){
                float nL = svgContent[getKeyAtIndex(i)].getNormLength();
                if(i < getIndex()) offset += nL + spacing;
                totalLength += nL + spacing;
            }
            
            totalLength -= spacing;
            
            for(auto & poly : svgContent[getKey()].outlines){
                ofPolyline p = poly;
                p.translate(glm::vec2(offset, 0));
                processedWord.polys.push_back(p);
            }

            
            // Center
            float offsetC = (zoneNL-totalLength)/2;
            
            for(auto & poly : processedWord.polys){
                poly.translate(glm::vec2(offsetC, 0));
            }
        }
        
        
        // Normalize both axis
        for(auto & poly : processedWord.polys){
            poly.scale(1/zoneNL, 1);
        }
        
        
        // MAP to Zone and add Points
        for(auto & poly : processedWord.polys){
            for(auto & point : poly){
                point = pCurrentZone->map(point);
            }
            poly = poly.getResampledBySpacing(resLaser.get());
            for(auto & point : poly){
                processedWord.contentBase.addPoint(point, 1.);
            }
            processedWord.contentBase.closePointGroup();
        }
        
        processedWords.push_back(processedWord);
    }
    
    void draw(ofRectangle rect){
        ofPushMatrix();
        ofTranslate(rect.getTopLeft());
        ofScale(rect.getWidth(), rect.getHeight());
        ofSetColor(255);
        for(auto & processedWord : processedWords){
            for(auto & poly : processedWord.polys){
                poly.draw();
            }
        }
        ofPopMatrix();
    }
    
    void loadSvg(string path){
        svgContent.clear();
        for (auto& [key, word] : words) {
            if(key == EnumWords::BREAK || word.getString() == "") continue;
            SvgLoader svgLoader;
            svgLoader.setup(path+word.getString()+".svg");
            svgContent.insert(pair<int, SvgLoader>(key, svgLoader));
        }
    }
    
    
    vector<vector<ofxIlda::Point>> getPointGroups(){
        vector<vector<ofxIlda::Point>> pGroups;
        for(auto & pW: processedWords){
            for(auto & pGroup : pW.contentBase.getPointGroups())
            pGroups.push_back(pGroup);
        }
        return pGroups;
    }
    
    
    vector<shared_ptr<Zone>> *pZones;
    shared_ptr<Zone> pCurrentZone;

    struct ProcessedWord{
        vector<ofPolyline> polys;
        ContentBase contentBase;
    };
    
    vector<ProcessedWord> processedWords;
    

    
    map<int, SvgLoader> svgContent;

    
    int zoneIndex;
    
    ofParameter<bool> oneZoneOneWord{"oneZoneOneWord", true};

    ofParameter<int> numOfWords{"numOfWords", 1, 1, 5};

    ofParameter<float> spacing{"spacing", 0.5, 0.001, 1};

    
    ofParameter<float> chanceRepeat{"chanceRepeat", 10, 0, 100};

    ofParameter<int> minRepeat{"minRepeat", 5, 0, 20};
    ofParameter<int> maxRepeat{"maxRepeat", 20, 0, 20};

    ofParameter<int> repeatIndex{"repeatIndex", 0, 0, 20};
    ofParameter<float> resLaser{"resLaser", 0.005, 0.001, 0.01};
    ofParameter<float> chanceToShorten{"chanceToShorten", 10, 0, 100};
    ofParameter<float> chanceToExtend{"chanceToExtend", 10, 0, 100};

    ofParameterGroup parameters{"LaserFrame", oneZoneOneWord, numOfWords, spacing, chanceRepeat, chanceToShorten, chanceToExtend, minRepeat, maxRepeat, repeatIndex,  resLaser};
    
    
};

#endif /* LaserFrame_h */
