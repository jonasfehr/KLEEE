//
//  WeightedConnection.h
//  wildkleee
//
//  Created by Jonas Fehr on 13/08/2019.
//

#ifndef WeightedConnection_h
#define WeightedConnection_h

class WC{ // weighted Connection
public:
    WC(int next, float w){
        this->connectedWord = next;
        this->weight = w;
    }
    
    int connectedWord;
    float weight;
    
    float from;
    float to;
};


#endif /* WeightedConnection_h */
