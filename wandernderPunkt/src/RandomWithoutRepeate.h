//
//  RandomWithoutRepeate.h
//  wandernderPunkt
//
//  Created by Jonas Fehr on 03/09/2018.
//

#ifndef RandomWithoutRepeate_h
#define RandomWithoutRepeate_h


class RandomWithoutRepeate{
public:
    
    void add(){
        int newNumber = array.size();
        array.push_back(newNumber);
        std::random_shuffle ( array.begin(), array.end() );
    }
    
    int getNext(){
        index++;
        if(index>=array.size()){
            std::random_shuffle ( array.begin(), array.end() );
            index = 0;
        }
        return array[ index ];
    }
private:
    int index = 0;
    vector<int> array;
};


#endif /* RandomWithoutRepeate_h */
