#include <iostream>
#include <string>
#include <map>
#include <iomanip>
#include <fstream>
#include <vector>
#include <sstream>
#include <iterator>
#include <stdlib.h>

using namespace std;

//#define DEBUG

#define ULONG unsigned long
#define ULONGLONG unsigned long long
#define ADDR void *
#define INFINITE (unsigned long long)-1
#define INVALID_ADDR (void *)-1

class OPTAccess {
 public:
    ADDR addr;
    ULONG refID;
    ULONGLONG nextTime;
    OPTAccess(ADDR addr, ULONG refID, ULONGLONG nextTime) {
        this->addr = addr;
        this->refID = refID;
	this->nextTime = nextTime;
    }
};

class CacheBlock {
 public:
    ADDR addr;
    ULONGLONG nextTime;
    CacheBlock *left;
    CacheBlock *right;
    CacheBlock *parent;
    CacheBlock(ADDR addr, ULONGLONG nextTime) {
	this->addr = addr;
	this->nextTime = nextTime;
	this->left = NULL;
	this->right = NULL;
	this->parent = NULL;
    }
};

class CacheSet {
 public:
    CacheSet() {
	this->size = 0;
	this->root = NULL;
	this->totalCounter = 0;
	this->missCounter = 0;
    }
    ULONG size;
    void adjustNodes(CacheBlock* node);
    CacheBlock * deleteRoot();
    void insertNode(CacheBlock* node);
    void output();
    //private:
    map<ADDR, CacheBlock*> hashTable;
    CacheBlock *root;
    void swapNodes(CacheBlock* child, CacheBlock* parent);
    void outputNode(CacheBlock* node);
    ULONG totalCounter;
    ULONG missCounter;
};

class OPTCacheSimulator {
 public:
    OPTCacheSimulator(ULONG &cacheSize, ULONG &cacheLineSize, ULONG &associativity);
    void doSimulation(string &inputFileName);
    //private:
    CacheSet* cacheSets;
    ULONGLONG totalCounter;
    ULONGLONG missCounter;
    ULONGLONG evictionCounter;
    ULONG cacheSize;
    ULONG cacheLineSize;
    ULONG associativity;
    ULONG setSize;
    ULONG tagSize;
    ULONG indexSize;
    ULONG offsetSize;
    inline ULONG getTag(ADDR addr) {
        if (tagSize == 0)
            return 0;
        return (ULONG)addr >> (indexSize + offsetSize);
    }
    inline ULONG getIndex(ADDR addr) {
        if (indexSize == 0)
            return 0;
        return ((ULONG)addr << tagSize) >> (tagSize + offsetSize);
    }
    inline ULONG getOffset(ADDR addr) {
        if (offsetSize == 0)
            return 0;
        return ((ULONG)addr << (tagSize + offsetSize)) >> (tagSize + offsetSize);
    }
    //ADDR doAccess(OPTAccess &acc);
    void doAccess(OPTAccess &acc);
};
