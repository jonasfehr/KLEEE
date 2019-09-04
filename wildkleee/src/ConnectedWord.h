//
//  ConnectedWord.h
//  wildkleee
//
//  Created by Jonas Fehr on 13/08/2019.
//

#ifndef ConnectedWord_h
#define ConnectedWord_h

#include "WeightedConnection.h"
#include "SvgLoader.h"


class ConnectedWord{
public:
    ConnectedWord(){};
    
    ConnectedWord(string word, int enumWord, string path = "fonts/debug/"){
        this->word = word;
        this->enumWord = enumWord;
        if(word == "\n" ) return;
        setPath(path);
        isDirty = true;
    }
    
    float getNormLength(){
        if(isDirty) calculateNormLength();
        return normLength;
    }
    
    void calculateNormLength(){
        ofRectangle boundingBox = svgLoader.getBoundingBox();
        normLength = boundingBox.getWidth()/boundingBox.getHeight();
    }
    
    
    
    void addConnection(WC connection){
        connections.push_back(connection);
    }
    
    void normalizeWeights(){
        float sumWeights = 0;
        for(auto & c : connections){
            sumWeights += c.weight;
        }
        for(auto & c : connections){
            c.weight = c.weight/sumWeights;
        }
        calculateWeightRanges();
    }
    
    void calculateWeightRanges(){
        connections[0].from = 0;
        connections[0].to = connections[0].weight;
        for(int i = 1; i < connections.size(); i++){
            connections[i].from = connections[i-1].to;
            connections[i].to = connections[i].from + connections[i].weight;
        }
    }
    
    void setPath(string path){
        this->path = path;
        if(word == "\n") return;
        this->svgLoader.setup(path+word+".svg");
        isDirty = true;
    }
    
    string getString(){
        return word;
    }
    
    vector<WC> & getConnections(){ return connections; }
    
    string word;
    int enumWord;
    vector<WC> connections;
    
    SvgLoader svgLoader;
    
    float normLength;
    bool isDirty;
    
    string path;
};

#endif /* Word_h */
