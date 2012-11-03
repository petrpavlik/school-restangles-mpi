//
//  main.cpp
//  POO
//
//  Created by Petr Pavlik on 10/3/12.
//  Copyright (c) 2012 Petr Pavlik. All rights reserved.
//

#include <iostream>
#include <list>
#include "mpi.h"

#define CHECK_MSG_AMOUNT  100

#define MSG_WORK_REQUEST 1000
#define MSG_WORK_SENT    1001
#define MSG_WORK_NOWORK  1002
#define MSG_TOKEN        1003
#define MSG_FINISH       1004

int bestResult = -1;

long long steps = 0;

//-----------------------------------------------

struct Rect
{
	/// Constructor.
    Rect(int width, int height) : x(0), y(0), width(width), height(height)
	{
	}
    
    Rect() : x(0), y(0), width(0), height(0)
	{
	}
    
	int right() const
	{
		return x + width - 1;
	}
	
	int bottom() const
	{
		return y + height - 1;
	}
	
	int x; ///< X coordinate of the top-left corner.
	int y; ///< Y coordinate of the top-left corner.
	int width; ///< Width of the rectangle.
	int height; ///< Height of the rectangle.
};

//-----------------------------------------------

//represents a field
struct Field
{
    
public:
    
    Field(unsigned int width, unsigned int height) {
        
        this->width = width;
        this->height = height;
        
        array = new char[width*height];
        memset(array, 0, width*height*sizeof(char));
    }
    
    ~Field() {
        
        if (array) {
            delete[] array;
        }
    }
    
    Field(const Field& other) {
        width = other.width;
        height = other.height;
        array = new char[width*height];
        memcpy(array, other.array, width*height*sizeof(char));
    }
    
    bool addRect(Rect rect) {
        
        steps++;
        
        if (bestResult==0) {
            return false; //we already have the best result we can get
        }
        
        //How many gaps were skipped. Is this number greater than bestResult?
        int numSkippedGaps = 0;
        
        for (int y=0; y<height; y++) {
            
            for (int x=0; x<width; x++) {
                
                //find first free cell in this row
                if (arrayValueAtPosition(x, y)==0) {
                    
                    rect.x = x;
                    rect.y = y;
                    
                    //can the rect fit here?
                    if (tryToFitRect(rect)) {
                        return true;
                    }
                    
                    numSkippedGaps++;
                    //std::cout << "skipping gap at " << x << " y " << y << std::endl;
                    
                    if (bestResult!=-1 && numSkippedGaps>=bestResult) {
                        //we can cut this off
                        //std::cout << "cutting " << numSkippedGaps << std::endl;
                        //print();
                        return false;
                    }
                    
                }
            }
        }
        
        //cannot fit it anywhere
        if (containsRequiredRects()) {
            
            print();
            
            int result = getNumberOfGaps();
            
            if (bestResult == -1 || result < bestResult) {
                bestResult = result;
                std::cout << "best result so far " << bestResult << std::endl;
            }
        }
        return false;
    }
    
private:
    
    bool tryToFitRect(Rect rect) {
        
        //don't waste time if the rect exceeds boundaries of the field
        if (rect.right()>=width || rect.bottom()>=height) {
            return false;
        }
        
        //try to find a taken cell
        for (int y=rect.y; y<=rect.bottom(); y++) {
            
            for (int x=rect.x; x<=rect.right(); x++) {
                
                //have we found a taken cell?
                if (arrayValueAtPosition(x, y)!=0) {
                    return false; //then we cannot fit it
                }
            }
        }
        
        //fit the rect
        for (int y=rect.y; y<=rect.bottom(); y++) {
            
            for (int x=rect.x; x<=rect.right(); x++) {
                setArrayValueAtPosition(x, y, rect.width*10+rect.height);
            }
        }
        
        return true;
    }
    
    char arrayValueAtPosition(int x, int y) {
        return array[y*width+x];
    }
    
    void setArrayValueAtPosition(int x, int y, char value) {
        array[y*width+x] = value;
    }
    
    void print() {
        
        for (int y=0; y<height; y++) {
            
            for (int x=0; x<width; x++) {
                
                std::cout << (int)arrayValueAtPosition(x, y) << "\t";
            }
            
            std::cout << std::endl;
        }
        
        std::cout << std::endl;
        
    }
    
    bool containsRequiredRects() {
        
        bool has3x3 = false;
        bool has2x4 = false;
        bool has4x2 = false;
        bool has1x5 = false;
        bool has5x1 = false;
        
        for (int y=0; y<height; y++) {
            
            for (int x=0; x<width; x++) {
                
                if (arrayValueAtPosition(x, y) == 33) {
                    has3x3 = true;
                }
                else if (arrayValueAtPosition(x, y) == 24) {
                    has2x4 = true;
                }
                else if (arrayValueAtPosition(x, y) == 42) {
                    has4x2 = true;
                }
                else if (arrayValueAtPosition(x, y) == 15) {
                    has1x5 = true;
                }
                else if (arrayValueAtPosition(x, y) == 51) {
                    has5x1 = true;
                }
            }
            
        }
        
        if (has3x3 && (has2x4 || has4x2) && (has1x5 || has5x1)) {
            return true;
        }
        
        return false;
    }
    
    int getNumberOfGaps() {
        
        int numGaps = 0;
        
        for (int y=0; y<height; y++) {
            
            for (int x=0; x<width; x++) {
                
                if (arrayValueAtPosition(x, y) == 0) {
                    numGaps++;
                }
            }
            
        }
        
        return numGaps;
    }
    
    unsigned int width;
    unsigned int height;
    
    char* array;
};

//-----------------------------------------------

struct Item
{
    Item() {
        field = NULL;
        index = 0;
    }
    
    Item(Field* field, int index) {
        
        this->field = field;
        this->index = index;
    }
    
    ~Item() {
        
        if (field) {
            delete field;
        }
    }
    
    void send(int destination) {
        
        int bufferSize = field->width * field->height + sizeof(int);
        char* buffer = new char[bufferSize];
        
        int position = 0;
        MPI_Pack(&index, 1, MPI_INT, buffer, bufferSize, &position, MPI_COMM_WORLD);
        MPI_Pack(field->array, (field->width * field->height), MPI_CHAR, buffer, bufferSize, &position, MPI_COMM_WORLD);
        
        MPI_Send(buffer, position, MPI_PACKED, destination, MSG_WORK_SENT, MPI_COMM_WORLD);
        
        delete[] buffer;
    }
    
    void receive() {
        
        int bufferSize = field->width * field->height + sizeof(int);
        char* buffer = new char[bufferSize];
        
        int position = 0;
        
        MPI_Recv(buffer, bufferSize, MPI_PACKED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        MPI_Unpack(buffer, bufferSize, &position, &index, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(buffer, bufferSize, &position, field->array, (field->width * field->height), MPI_CHAR, MPI_COMM_WORLD);
        
        delete[] buffer;
    }
    
    Field* field;
    int index;
};

//-----------------------------------------------

void processUsingStack2(Field field) {
    
    std::list<Item*> stack;
    long long counter=0;
    std::list<int> workRequests;
    
    stack.push_back(new Item(new Field(field), 0));
    
    while (stack.size()) {
        
        counter++;
        if ((counter % CHECK_MSG_AMOUNT)==0)
        {
            int flag = 0;
            MPI_Status status;
            
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            if (flag)
            {
                //prisla zprava, je treba ji obslouzit
                //v promenne status je tag (status.MPI_TAG), cislo odesilatele (status.MPI_SOURCE)
                //a pripadne cislo chyby (status.MPI_ERROR)
                swith (status.MPI_TAG)
                {
                case MSG_WORK_REQUEST : // zadost o praci, prijmout a dopovedet
                    // zaslat rozdeleny zasobnik a nebo odmitnuti MSG_WORK_NOWORK
                    workRequests.push_back(status.MPI_SOURCE);
                    break;
                case MSG_WORK_SENT : // prisel rozdeleny zasobnik, prijmout
                    // deserializovat a spustit vypocet
                    break;
                case MSG_WORK_NOWORK : // odmitnuti zadosti o praci
                    // zkusit jiny proces
                    // a nebo se prepnout do pasivniho stavu a cekat na token
                    break
                case MSG_TOKEN : //ukoncovaci token, prijmout a nasledne preposlat
                    // - bily nebo cerny v zavislosti na stavu procesu
                    //MPI_Send(<#void *buf#>, <#int count#>, <#MPI_Datatype datatype#>, <#int dest#>, MSG_TOKEN, MPI_COMM_WORLD);
                    break;
                case MSG_FINISH : //konec vypoctu - proces 0 pomoci tokenu zjistil, ze jiz nikdo nema praci
                    //a rozeslal zpravu ukoncujici vypocet
                    //mam-li reseni, odeslu procesu 0
                    //nasledne ukoncim spoji cinnost
                    //jestlize se meri cas, nezapomen zavolat koncovou barieru MPI_Barrier (MPI_COMM_WORLD)
                    MPI_Finalize();
                    exit (0);
                    break;
                default : chyba("neznamy typ zpravy"); break;
                }
            }
        }
        
        Item* pItem = stack.back();
        Field* pField = pItem->field;
        
        Rect* pRectToTry = NULL;
        
        if (pItem->index==0) {
            pRectToTry = new Rect(3, 3);
        }
        else if(pItem->index==1) {
            pRectToTry = new Rect(2, 4);
        }
        else if(pItem->index==2) {
            pRectToTry = new Rect(4, 2);
        }
        else if(pItem->index==3) {
            pRectToTry = new Rect(1, 5);
        }
        else if(pItem->index==4) {
            pRectToTry = new Rect(5, 1);
        } 
        else {
            stack.pop_back();
            delete pItem;
        }
        
        
        if (pRectToTry) {
            
            Field* pNewField = new Field(*pField);
            
            //send this to somebody who has requested some work
            if (workRequests.size()) {
                
                int desctination = workRequests.front();
                pItem->send(desctination);
                workRequests.pop_front();
                delete pNewField;
            }
            else {
                
                if(pNewField->addRect(*pRectToTry)) {
                    
                    stack.push_back(new Item(pNewField, 0));
                }
                else {
                    delete pNewField;
                }
            }
            
            
            pItem->index++;
            delete pRectToTry;
        }
        
        //empty stack? request work and send no work to all work requests
        if (stack.size()==0) {
            
            for (unsigned int i=0; i<workRequests.size(); i+=) {
                
                //send no work
            }
            
            //request work
        }
        
    }
    
}

//-----------------------------------------------

int main(int argc, const char * argv[])
{
    
    int myProcessRank;
    int numProcesses;
    
    MPI_Init(&argc, &argv);
    
    /* find out process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &myProcessRank);
    
    /* find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
    
    Field field = Field(6, 5);
    
    processUsingStack2(field);
    
    std::cout << "num steps " << steps << std::endl;
    std::cout << "best result " << bestResult << std::endl;
    
    MPI_Finalize();
    MPI
    
    return 0;
}

