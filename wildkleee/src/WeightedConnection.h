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
    WC(int connectedWordEnum, float weight){
        this->connectedWordEnum = connectedWordEnum;
        this->weight = weight;
    }
    
    int getConnected(){ return connectedWordEnum; }
    
    int connectedWordEnum;
    float weight;
    float weightNormalised;

    float from;
    float to;
};


#endif /* WeightedConnection_h */
