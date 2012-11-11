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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//#define dprintf printf
#define dprintf

#define CHECK_MSG_AMOUNT  100

#define MSG_WORK_REQUEST 1000
#define MSG_WORK_SENT    1001
#define MSG_WORK_NOWORK  1002
#define MSG_TOKEN        1003
#define MSG_FINISH       1004

int fieldWidth = 6;
int fieldHeight = 5;

#define TOKEN_COLOR_WHITE 0
#define TOKEN_COLOR_BLACK 1

int bestResult = -1;

long long steps = 0;

int myProcessRank;
int numProcesses;

bool waitingForWork = false;

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
            
            //print();
            
            int result = getNumberOfGaps();
            
            if (bestResult == -1 || result < bestResult) {
                bestResult = result;
                //std::cout << "best result so far " << bestResult << std::endl;
            }
        }
        return false;
    }
    
    unsigned int getWidth() {
        return width;
    }
    
    unsigned int getHeight() {
        return height;
    }
    
    char* getArray() {
        return array;
    }
    
    void setArray(char* array) {
        
        if (this->array) {
            delete[] this->array;
        }
        
        this->array = array;
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
        
        dprintf("p%d: did start sending work to %d\n", myProcessRank, destination);
        int bufferSize = field->getWidth() * field->getHeight() + sizeof(int);
        char* buffer = new char[bufferSize];
        
        int position = 0;
        MPI_Pack(&index, 1, MPI_INT, buffer, bufferSize, &position, MPI_COMM_WORLD);
        MPI_Pack(field->getArray(), (field->getWidth() * field->getHeight()), MPI_CHAR, buffer, bufferSize, &position, MPI_COMM_WORLD);
        
        /*for (unsigned int i=0; i<bufferSize; i++) {
            dprintf("%d", buffer[i]);
        }
        dprintf("\n");*/
        
        MPI_Send(buffer, position, MPI_PACKED, destination, MSG_WORK_SENT, MPI_COMM_WORLD);
        
        delete[] buffer;
        
        dprintf("p%d: did finish sending work to %d\n", myProcessRank, destination);
    }
    
    void receive() {
        
        dprintf("p%d: did start receiving work\n", myProcessRank);
        
        field = new Field(fieldWidth, fieldHeight);
        
        int bufferSize = field->getWidth() * field->getHeight() + sizeof(int);
        char* buffer = new char[bufferSize];
        
        int position = 0;
        
        MPI_Status status;
        MPI_Recv(buffer, bufferSize, MPI_PACKED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        /*for (unsigned int i=0; i<bufferSize; i++) {
            dprintf("%d", buffer[i]);
        }
        dprintf("\n");*/
        
        MPI_Unpack(buffer, bufferSize, &position, &index, 1, MPI_INT, MPI_COMM_WORLD);
        
        //dprintf("p%d: did unpack index %d\n", myProcessRank, index);
        
        char* tmpArray = new char[field->getWidth() * field->getHeight()];
        MPI_Unpack(buffer, bufferSize, &position, tmpArray, (field->getWidth() * field->getHeight()), MPI_CHAR, MPI_COMM_WORLD);
        
        //dprintf("a\n");
        
        field->setArray(tmpArray);
        
        dprintf("p%d: did finish receiving work\n", myProcessRank);
    }
    
    Field* field;
    int index;
};

//-----------------------------------------------

void sendWorkRequest() {
    
    if (waitingForWork || bestResult==0) {
        return;
    }
    
    int destination = myProcessRank;
    
    while (destination==myProcessRank) {
        
        destination = rand()%numProcesses;
        if (destination!=myProcessRank) {
            
            int dummy=0;
            MPI_Send(&dummy, 1, MPI_INT, destination, MSG_WORK_REQUEST, MPI_COMM_WORLD);
        }
    }
    
    waitingForWork = true;
}

void receiveDummy() {
    
    int dummy;
    MPI_Status status;
    MPI_Recv(&dummy, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
}

void printBuffer(char* buffer, int bufferSize) {
    
    for (unsigned int i=0; i<bufferSize; i++) {
        dprintf("%d", buffer[i]);
    }
    dprintf("\n");
}

void sendToken(int destination, char tokenColor) {
    
    dprintf("p%d: sending token %d, best result %d\n", myProcessRank, tokenColor, bestResult);
    
    int bufferSize = sizeof(char) + sizeof(int);
    char* buffer = new char[bufferSize];
    
    int position = 0;
    MPI_Pack(&tokenColor, 1, MPI_CHAR, buffer, bufferSize, &position, MPI_COMM_WORLD);
    MPI_Pack(&bestResult, 1, MPI_INT, buffer, bufferSize, &position, MPI_COMM_WORLD);
    
    printBuffer(buffer, bufferSize);
    
    MPI_Send(buffer, position, MPI_PACKED, destination, MSG_TOKEN, MPI_COMM_WORLD);
    
    delete[] buffer;
}

void processUsingStack2(Field field) {
    
    std::list<Item*> stack;
    long long counter=0;
    std::list<int> workRequests;
    
    if (myProcessRank==0) {
        stack.push_back(new Item(new Field(field), 0));
        
        dprintf("p%d: sending black token to %d\n", myProcessRank, 1);
        char tokenColor = TOKEN_COLOR_BLACK;
        sendToken(1, tokenColor);
    }
    
    ///////
    
    while (true) {
        
        counter++;
        if ((counter % CHECK_MSG_AMOUNT)==0)
        {
            //dprintf("p%d: is checking new messages\n", myProcessRank);
            
            counter=1;
            int flag = 0;
            MPI_Status status;
            
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            if (flag)
            {
                if (status.MPI_TAG == MSG_WORK_REQUEST) {
                    
                    dprintf("p%d: did receive work request\n", myProcessRank);
                    receiveDummy();
                    workRequests.push_back(status.MPI_SOURCE);
                }
                else if (status.MPI_TAG == MSG_WORK_SENT) {
                    
                    dprintf("p%d: did receive work\n", myProcessRank);
                    Item* item = new Item();
                    item->receive();
                    stack.push_back(item);
                    waitingForWork = false;
                }
                else if (status.MPI_TAG == MSG_WORK_NOWORK) {
                 
                    dprintf("p%d: did receive no work\n", myProcessRank);
                    sendWorkRequest();
                }
                else if (status.MPI_TAG == MSG_TOKEN) {
                    
                    int position = 0;
                    int bufferSize = sizeof(char) + sizeof(int);
                    char* buffer = new char[bufferSize];
                    
                    char tokenColor;
                    int receivedBestResult;
                    MPI_Recv(buffer, bufferSize, MPI_PACKED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    
                    printBuffer(buffer, bufferSize);
                    MPI_Unpack(buffer, bufferSize, &position, &tokenColor, 1, MPI_CHAR, MPI_COMM_WORLD);
                    MPI_Unpack(buffer, bufferSize, &position, &receivedBestResult, 1, MPI_INT, MPI_COMM_WORLD);
                    
                    dprintf("p%d: received token: %d, best result %d\n", myProcessRank, tokenColor, receivedBestResult);
                    
                    if (tokenColor == TOKEN_COLOR_WHITE) {
                        dprintf("p%d: did receive WHITE token from %d\n", myProcessRank, status.MPI_SOURCE);
                    }
                    else {
                        dprintf("p%d: did receive BLACK token from %d\n", myProcessRank, status.MPI_SOURCE);
                    }
                    
                    if (bestResult==-1 && receivedBestResult!=-1) {
                        bestResult = receivedBestResult;
                    }
                    else if (bestResult!=-1 && receivedBestResult!=-1 && bestResult>receivedBestResult) {
                        bestResult = receivedBestResult;
                    }
                    
                    if (bestResult==0) {
                        dprintf("p%d: recognized best result 0\n", myProcessRank);
                        stack.clear();
                    }
                    
                    if (myProcessRank==0) {
                        
                        if (bestResult==0 || tokenColor==TOKEN_COLOR_WHITE) {
                            
                            dprintf("p%d: FINITO\n", myProcessRank);
                            
                            for (int i=1; i<numProcesses; i++) {
                                
                                dprintf("p%d: sending finish to %d\n", myProcessRank, i);
                                int dummy=0;
                                MPI_Send(&dummy, 1, MPI_INT, i, MSG_FINISH, MPI_COMM_WORLD);
                            }
                            
                            return;
                        }
                        else {
                            
                            if (stack.size()) {
                                dprintf("p%d: sending black token to %d\n", myProcessRank, 1);
                                tokenColor = TOKEN_COLOR_BLACK;
                                sendToken(1, tokenColor);
                            }
                            else {
                                dprintf("p%d: sending white token to %d\n", myProcessRank, 1);
                                tokenColor = TOKEN_COLOR_WHITE;
                                sendToken(1, tokenColor);
                            }
                        }
                        
                        /*if (stack.size()) {
                            dprintf("p%d: sending black token\n", myProcessRank);
                            tokenColor = TOKEN_COLOR_BLACK;
                            sendToken(1, tokenColor);
                        }
                        else if (tokenColor==TOKEN_COLOR_BLACK) {
                            dprintf("p%d: sending white token\n", myProcessRank);
                            tokenColor = TOKEN_COLOR_WHITE;
                            sendToken(1, tokenColor);
                        }
                        else {
                            dprintf("p%d: FINITO\n", myProcessRank);
                            
                            for (int i=1; i<numProcesses; i++) {
                                
                                dprintf("p%d: sending finish to %d\n", myProcessRank, i);
                                int dummy=0;
                                MPI_Send(&dummy, 1, MPI_INT, i, MSG_FINISH, MPI_COMM_WORLD);
                            }
                            
                            return;
                        }*/
                    }
                    else {
                        
                        if (tokenColor == TOKEN_COLOR_BLACK) {
                            dprintf("p%d: BLACK token\n", myProcessRank);
                        }
                        else {
                            dprintf("p%d: WHITE token\n", myProcessRank);
                        }
                        
                        if (tokenColor == TOKEN_COLOR_WHITE) {
                            if (stack.size()!=0) {
                                tokenColor = TOKEN_COLOR_BLACK;
                            }
                        }
                        
                        int destination = (myProcessRank+1)%numProcesses;
                        
                        if (tokenColor==TOKEN_COLOR_BLACK) {
                            dprintf("p%d: sending black token to %d\n", myProcessRank, destination);
                        }
                        else {
                            dprintf("p%d: sending white token to %d\n", myProcessRank, destination);
                        }
                        
                        sendToken(destination, tokenColor);
                    }

                }
                else if (status.MPI_TAG == MSG_FINISH) {
                    dprintf("p%d: did receive finish\n", myProcessRank);
                    receiveDummy();
                    return;
                }
              
            }
            
        }
        
        ////////
        
        if (stack.size()) {
            
            if (workRequests.size()) {
                
                std::list<Item*>::const_iterator iterator;
                for (iterator = stack.begin(); iterator != stack.end(); ++iterator) {
                    
                    Item* pItem = *iterator;
                    
                    if (pItem->index>3) {
                        continue;
                    }
                    
                    int destination = workRequests.front();
                    
                    pItem->index++;
                    pItem->send(destination);
                    pItem->index=5;
                    
                    workRequests.pop_front();
                    break;
                }
            }
            
            
            Item* pItem = stack.back();
            Field* pField = pItem->field;
            
            /*if (myProcessRank==1) {
                dprintf("p%d: is processing stack of size %d\n", myProcessRank, (int)stack.size());
                dprintf("p%d: rect id %d\n", myProcessRank, pItem->index);
            }*/
            
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
                /*if (workRequests.size()) {
                    
                    int destination = workRequests.front();
                    pItem->send(destination);
                    workRequests.pop_front();
                    delete pNewField;
                    pItem->index = 5;
                }
                else {
                    
                    if(pNewField->addRect(*pRectToTry)) {
                        
                        stack.push_back(new Item(pNewField, 0));
                    }
                    else {
                        delete pNewField;
                    }
                }*/
                
                
                if(pNewField->addRect(*pRectToTry)) {
                    
                    stack.push_back(new Item(pNewField, 0));
                }
                else {
                    delete pNewField;
                }
                
                
                pItem->index++;
                delete pRectToTry;
            }
            
            //empty stack? request work and send no work to all work requests
            if (stack.size()==0) {
                
                dprintf("p%d: no more work\n", myProcessRank);
                
                while (workRequests.size()) {
                    
                    int destination = workRequests.front();
                    
                    dprintf("p%d: is sending no work to %d\n", myProcessRank, destination);
                    
                    //send no work
                    int dummy=0;
                    MPI_Send(&dummy, 1, MPI_INT, destination, MSG_WORK_NOWORK, MPI_COMM_WORLD);
                    
                    workRequests.pop_front();
                }
                
                //request work
                sendWorkRequest();
            }
            
        }
        else {
            //dprintf("p%d: no work\n", myProcessRank);
            sendWorkRequest();
        }

        
    }
    
}

//-----------------------------------------------

int main(int argc, char * argv[])
{
    
    if (argc>1) {
        fieldWidth = atoi(argv[1]);
        fieldHeight = atoi(argv[2]);
    }
    
    MPI_Init(&argc, &argv);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    double tStart, tEnd;
    
    tStart = MPI_Wtime ();
    
    /* find out process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &myProcessRank);
    
    /* find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
    
    srand(myProcessRank);
    
    printf("%d processes, mine is %d\n", numProcesses, myProcessRank);
    
    Field field = Field(fieldWidth, fieldHeight);
    
    processUsingStack2(field);
    
    tEnd = MPI_Wtime();
    
    printf("p%d: best result %d\n", myProcessRank, bestResult);
    printf("p%d: time %fs\n", myProcessRank , tEnd-tStart);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    MPI_Finalize();
    
    return 0;
}

