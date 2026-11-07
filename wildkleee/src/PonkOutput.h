//
//  PonkOutput.h
//  wildkleee
//
//  Sends the laser content to MadMapper via Ponk (ofxPonk):
//   - one stream per zone, in zone-local coordinates (placement / keystone is done in MadMapper)
//   - one combined stream with the already mapped output (same as what the RayComposer gets)
//

#pragma once

#include "ofMain.h"
#include "ofxPonk.h"

class PonkOutput {
public:
    struct Path {
        ofPolyline poly;
    };

    ofParameter<bool> enabled{"ponkEnabled", false};
    ofParameter<bool> sendPerZone{"ponkPerZone", true};
    ofParameter<bool> sendCombined{"ponkCombined", true};
    ofParameter<bool> flipY{"ponkFlipY", true};               // oF is y-down, MadMapper y-up
    ofParameter<string> destinationIP{"ponkIP", "127.0.0.1"};
    ofParameter<string> streamPrefix{"ponkPrefix", "wildkleee"};
    ofParameterGroup parameters{"Ponk", enabled, sendPerZone, sendCombined, flipY, destinationIP, streamPrefix};

    PonkOutput(){
        listener = parameters.parameterChangedE().newListener([this](ofAbstractParameter & p){
            if(p.getName() == destinationIP.getName() || p.getName() == streamPrefix.getName()){
                reconfigure();
            }
        });
    }

    // Call once per frame before adding content. zoneNames: all zones that currently exist;
    // each gets a stream (also when empty, so MadMapper keeps seeing it).
    void begin(const vector<string> & zoneNames){
        // remove streams of deleted zones
        for(auto it = zoneStreams.begin(); it != zoneStreams.end();){
            if(std::find(zoneNames.begin(), zoneNames.end(), it->first) == zoneNames.end()) it = zoneStreams.erase(it);
            else ++it;
        }
        // create streams for new zones
        for(auto & name : zoneNames){
            if(zoneStreams.count(name) == 0){
                zoneStreams[name] = make_unique<ofxPonk>();
                configure(*zoneStreams[name], streamName(name));
            }
        }
        if(!combined){
            combined = make_unique<ofxPonk>();
            configure(*combined, streamName("combined"));
        }

        combined->newFrame();
        for(auto & [name, ponk] : zoneStreams) ponk->newFrame();
    }

    // Zone-local path: x 0..1 along the zone, y 0..1 (text height), zoneNormLength = width/height of the zone.
    // Sent with preserved proportions: the zone is centred and its longer side spans -1..1.
    void addZoneLocal(const string & zoneName, const ofPolyline & poly, float zoneNormLength, const ofColor & color){
        auto it = zoneStreams.find(zoneName);
        if(it == zoneStreams.end() || zoneNormLength <= 0) return;
        const float w = zoneNormLength, h = 1.f;
        const float scale = 2.f / std::max(w, h);
        ofPolyline p;
        for(auto & v : poly.getVertices()){
            p.addVertex(toPonk({(v.x - 0.5f) * w * scale, (v.y - 0.5f) * h * scale}));
        }
        if(p.size() > 0) it->second->add(p, color);
    }

    // Mapped output path in 0..1 output space (as sent to ofxIlda).
    void addCombined(const ofPolyline & poly, const ofColor & color){
        ofPolyline p;
        for(auto & v : poly.getVertices()){
            p.addVertex(toPonk({v.x*2.f - 1.f, v.y*2.f - 1.f}));
        }
        if(p.size() > 0) combined->add(p, color);
    }

    void send(){
        if(!enabled) return;
        if(sendCombined && combined) combined->sendFrame();
        if(sendPerZone){
            for(auto & [name, ponk] : zoneStreams) ponk->sendFrame();
        }
    }

private:
    glm::vec3 toPonk(glm::vec2 p) const {
        if(flipY) p.y = -p.y;
        // the float format must stay within -1..1
        return glm::vec3(ofClamp(p.x, -1, 1), ofClamp(p.y, -1, 1), 0);
    }

    string streamName(const string & suffix) const {
        return streamPrefix.get() + "_" + suffix;
    }

    // stable ID per stream name, so MadMapper recognises the stream after a restart
    static int stableID(const string & s){
        uint32_t h = 2166136261u; // FNV-1a
        for(unsigned char c : s){ h ^= c; h *= 16777619u; }
        return static_cast<int>(h & 0x7fffffff);
    }

    void configure(ofxPonk & ponk, const string & name){
        ponk.setup(name, stableID(name), destinationIP.get(), PONK_DATA_FORMAT_XY_F32_RGB_U8);
    }

    void reconfigure(){
        for(auto & [name, ponk] : zoneStreams) configure(*ponk, streamName(name));
        if(combined) configure(*combined, streamName("combined"));
    }

    map<string, unique_ptr<ofxPonk>> zoneStreams;
    unique_ptr<ofxPonk> combined;
    ofEventListener listener;
};
