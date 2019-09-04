//
//  I_want_to_say_something.h
//  wildkleee
//
//  Created by Jonas Fehr on 13/08/2019.
//

#ifndef I_want_to_say_something_h
#define I_want_to_say_something_h

#include "ConnectedWord.h"
#include "WeightedConnection.h"

enum EnumWords : int{
    I = 0,
    WANT,
    TO,
    SAY,
    SOMETHING,
    BREAK,
    REPEAT
};


class I_want_to_say_something{
public:
    
    void setup(){
        ConnectedWord wI("I", I);
//        wI.addConnection(*new WC(EnumWords::REPEAT, 0.1));
        wI.addConnection(*new WC(EnumWords::WANT, 0.5));
        wI.addConnection(*new WC(EnumWords::SAY, 0.4));
        wI.normalizeWeights();
        words.insert(pair<int, ConnectedWord>(EnumWords::I, wI));

        ConnectedWord wWANT("WANT", WANT);
//        wWANT.addConnection(*new WC(EnumWords::REPEAT, 0.1));
        wWANT.addConnection(*new WC(EnumWords::TO, 0.2));
        wWANT.addConnection(*new WC(EnumWords::SOMETHING, 0.5));
        wWANT.addConnection(*new WC(EnumWords::BREAK, 0.3));
        wWANT.normalizeWeights();
        words.insert(pair<int, ConnectedWord>(EnumWords::WANT, wWANT));

        ConnectedWord wTO("TO", TO);
//        wTO.addConnection(*new WC(EnumWords::REPEAT, 0.1));
        wTO.addConnection(*new WC(EnumWords::SAY, 0.5));
        wTO.addConnection(*new WC(EnumWords::WANT, 0.5));
        wTO.addConnection(*new WC(EnumWords::BREAK, 0.3));
        wTO.normalizeWeights();
        words.insert(pair<int, ConnectedWord>(EnumWords::TO, wTO));

        ConnectedWord wSAY("SAY", SAY);
//        wSAY.addConnection(*new WC(EnumWords::REPEAT, 0.1));
        wSAY.addConnection(*new WC(EnumWords::SOMETHING, 0.5));
        wSAY.addConnection(*new WC(EnumWords::I, 0.5));
        wSAY.addConnection(*new WC(EnumWords::TO, 0.2));
        wSAY.addConnection(*new WC(EnumWords::BREAK, 0.3));
        wSAY.normalizeWeights();
        words.insert(pair<int, ConnectedWord>(EnumWords::SAY, wSAY));

        ConnectedWord wSOMETHING("SOMETHING", SOMETHING);
//        wSOMETHING.addConnection(*new WC(EnumWords::REPEAT, 0.1));
        wSOMETHING.addConnection(*new WC(EnumWords::I, 0.3));
        wSOMETHING.addConnection(*new WC(EnumWords::TO, 0.2));
        wSOMETHING.addConnection(*new WC(EnumWords::BREAK, 0.3));
        wSOMETHING.normalizeWeights();
        words.insert(pair<int, ConnectedWord>(EnumWords::SOMETHING, wSOMETHING));

        ConnectedWord wBREAK("\n", BREAK);
        wBREAK.addConnection(*new WC(EnumWords::I, 0.8));
        wBREAK.addConnection(*new WC(EnumWords::WANT, 0.5));
        wBREAK.addConnection(*new WC(EnumWords::TO, 0.2));
        wBREAK.addConnection(*new WC(EnumWords::SAY, 0.5));
        wBREAK.addConnection(*new WC(EnumWords::SOMETHING, 0.5));
        wBREAK.normalizeWeights();
        words.insert(pair<int, ConnectedWord>(EnumWords::BREAK, wBREAK));

        currentWord = EnumWords::BREAK;
    }
    
    string getNextString(){
        next();
        return getString();
    }
    
    string getString(){
        return words[currentWord].getString();
    }
    
    void next(){
        float rndF = ofRandom(0., 1.);
        for(auto & c : words[currentWord].getConnections()){
            if(c.from < rndF && c.to > rndF){
                if(words[c.connectedWord].enumWord == REPEAT){
                    currentWord = currentWord;
                }else {
                    currentWord = words[c.connectedWord].enumWord;
                }
            }
        }
    }
    
    void reset(){
        currentWord = EnumWords::BREAK;
    }
    
    
    ConnectedWord getNextWord(){
        next();
        return getWord();
    }
    
    ConnectedWord getWord(){
        return words[currentWord];
    }

    void changePath(string path){
        for(auto & word : words){
            word.second.setPath(path);
        }
    }
    
    
    int currentWord;
    
    map<int, ConnectedWord> words;
    
    string path;
};




#endif /* I_want_to_say_something_h */
