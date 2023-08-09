#include "OPTCacheSimulator.h"

OPTCacheSimulator::OPTCacheSimulator(ULONG &cacheSize, ULONG &cacheLineSize, ULONG &associativity) {
    this->cacheSize = cacheSize;
    this->cacheLineSize = cacheLineSize;
    this->associativity = associativity;
    this->setSize = cacheSize / (cacheLineSize * associativity);
    this->offsetSize = 0;
    while (cacheLineSize > 1) {
	cacheLineSize >>= 1;
	this->offsetSize++;
    }
    this->indexSize = 0;
    ULONG temp = this->setSize;
    while (temp > 1) {
	temp >>= 1;
	this->indexSize++;
    }
    this->tagSize = sizeof(ADDR) * 8 - this->indexSize - this->offsetSize;
    this->totalCounter = 0;
    this->missCounter = 0;
    this->evictionCounter = 0;
#if 0
    //#ifdef OPT_CACHE_DEBUG
    cout << "Cache configuration" << endl;
    cout << "\tsize\t\t\t" << this->cacheSize / (1024*1024) << "MB" << endl;
    cout << "\tline size\t\t" << this->cacheLineSize << "B" << endl;
    cout << "\tassociativity\t\t" << this->associativity << endl;
    cout << "\tthe number of sets\t" << this->setSize << endl;
    cout << "\taddress size\t\t" << sizeof(ADDR) * 8 << endl;
    cout << "\toffset size\t\t" << this->offsetSize << endl;
    cout << "\tindex size\t\t" << this->indexSize << endl;
    cout << "\ttag size\t\t" << this->tagSize << endl;
#endif

    // initialize the cache sets
    cacheSets = new CacheSet[setSize];
}

void OPTCacheSimulator::doSimulation(string &inputFileName) {
    ifstream inputFile(inputFileName.c_str());
    char line[256];

    if (inputFile.is_open()) {
	while (!inputFile.eof()) {
	  inputFile.getline(line, 256);
	  if ((line[0] == '0') && (line[1] == 'x')) {
	    //cout << line << endl;
	    istringstream iss(line);
	    vector<string> tokens;
	    copy(istream_iterator<string>(iss),
		 istream_iterator<string>(),
		 back_inserter<vector<string> >(tokens));
	    //cout << tokens.size() << "\t" << endl;
	    OPTAccess *anAccess;
	    //cout << tokens.size() << endl;
	    if (tokens.size() == 3) {
	      totalCounter++;
#ifdef OPT_CACHE_DEBUG
	      //cout << "--------------------------" << endl;
	      cout << "No." << totalCounter << "\t" << tokens[0] << "\t" << tokens[2] << "\t";
	      //cout << tokens[0] << "\t" << tokens[1] << "\t" << tokens[2] << endl;
#endif
	      ADDR addr;
	      istringstream issAddr(tokens[0]);
	      issAddr >> std::hex >> addr;
	      addr = (ADDR)((((ULONG)addr) >> offsetSize) << offsetSize);
	      ULONG refID = (ULONG)atol(tokens[1].c_str());
	      ULONGLONG tdis = (ULONGLONG)atol(tokens[2].c_str());
	      if (tdis != (ULONGLONG)INFINITE)
		anAccess = new OPTAccess(addr, refID, totalCounter+tdis);
	      else
		anAccess = new OPTAccess(addr, refID, INFINITE);
	    } else if (tokens.size() == 0) {
	      break;// the last line which is empty
	    } else {
	      cerr << "invalid input format" << endl;
	      exit(1);
	    }
	    //ADDR victim = doAccess(*anAccess);
	    doAccess(*anAccess);
	    //ADDR victim = 0;
	    //doAccess(*anAccess);
	    //cout << "victim: " << victim <<endl;
#ifdef OPT_CACHE_DEBUG
	    map<ADDR,CacheBlock*>::iterator it = hashTable.begin();
	    map<ADDR,CacheBlock*>::iterator end = hashTable.end();
	    for (;it!=end;it++) {
	      if ((*it).second->nextTime == INFINITE)
		cout << (*it).first << " => -1" << "\t";
	      else
		cout << (*it).first << " => " << (*it).second->nextTime << "\t";
	    }
	    cout << endl;
#endif	    
	    delete(anAccess);
	  }
	}
	inputFile.close();
    } else {
      cerr << "error: unable to open input file\t" << inputFileName << endl;
	exit(1);
    }

    //cout << "the total number of accesses: " << totalCounter << endl;
    //cout << "the total number of misses: " << missCounter << endl;

    ULONG logCacheSize = -1;
    ULONG temp = cacheSize;
    while (temp != 0) {
      logCacheSize++;
      temp >>= 1;
    }
    logCacheSize -= 6;
    cout << logCacheSize << "\t" << totalCounter << "\t" << missCounter << "\t" << evictionCounter << endl;
#if 0
    ULONG i;
    for (i=0;i<setSize;i++) {
      cout << i << ": " << cacheSets[i].totalCounter << "\t" << cacheSets[i].missCounter << endl;
    }
#endif
}

void OPTCacheSimulator::doAccess(OPTAccess &acc) {    
    ADDR addr = acc.addr;
    ULONG index = getIndex(addr);
    ADDR tag = (ADDR)getTag(addr);
    CacheSet *cacheSet = &cacheSets[index];
    //ADDR value = INVALID_ADDR;

    //cout << "No." << totalCounter << ":\t" << acc.addr << "\t" << acc.nextTime << endl;

    //check whether the addr is already in cache
    map<ADDR,CacheBlock*>::iterator it = cacheSet->hashTable.find(tag);
    map<ADDR,CacheBlock*>::iterator end =  cacheSet->hashTable.end();
    cacheSet->totalCounter++;
    if (it != end) {
	//it is a hit
#ifdef OPT_CACHE_DEBUG
      cout << "hit\t";
#endif
	CacheBlock *node = it->second;
	node->nextTime = acc.nextTime;
	cacheSet->adjustNodes(node);
    } else {
	//it is a miss
#ifdef OPT_CACHE_DEBUG
      cout << "miss\t";
#endif
	missCounter++;
	cacheSet->missCounter++;

	CacheBlock *newNode = new CacheBlock(tag, acc.nextTime);
	if (cacheSet->size < associativity) {
	    //no need to do eviction
	    //insert the new node (from bottom)
	    //cout << "\tdoing insertion" << endl;
	    cacheSet->insertNode(newNode);
	    cacheSet->hashTable.insert(pair<ADDR,CacheBlock*>(tag, newNode));
	} else {
	    //need to do eviction and the root with the greatest time distance is the victim
	    //delete the root node
	    //cout << "\tdoing eviction" << endl;
	    evictionCounter++;
	    CacheBlock *oldRoot = cacheSet->deleteRoot();
	    //value = oldRoot->addr;
	    //cout << "\t" << value << " is evicted" << endl;
	    //insert the new node (from bottom)
	    //cout << "\tdoing insertion" << endl;
	    cacheSet->insertNode(newNode);
	    it = cacheSet->hashTable.find(oldRoot->addr);
	    cacheSet->hashTable.erase(it);
	    cacheSet->hashTable.insert(pair<ADDR,CacheBlock*>(tag,newNode));
	    delete(oldRoot);
	}
    }
#ifdef OPT_CACHE_DEBUG
    //cacheSet->output();
#endif
    //return value;
}

void CacheSet::swapNodes(CacheBlock* oldChild, CacheBlock* oldParent) {
    //cout << "swaping\t" << oldChild->addr << "\t" << oldParent->addr << endl;
    CacheBlock *grandparent = oldParent->parent;
    CacheBlock *leftChild = oldChild->left;
    CacheBlock *rightChild = oldChild->right;
    if (leftChild != NULL)
	leftChild->parent = oldParent;
    if (rightChild != NULL)
	rightChild->parent = oldParent;
    oldParent->parent = oldChild;
    if (oldChild == oldParent->left) {
	CacheBlock *temp = oldChild->left;
	oldChild->left = oldParent;
	oldParent->left = temp;
	temp = oldChild->right;
	oldChild->right = oldParent->right;
	oldParent->right = temp;
	if (oldChild->right != NULL)
	    oldChild->right->parent = oldChild;
    }
    else if (oldChild == oldParent->right) {
	CacheBlock *temp = oldChild->right;
	oldChild->right = oldParent;
	oldParent->right = temp;
	temp = oldChild->left;
	oldChild->left = oldParent->left;
	oldParent->left = temp;
	if (oldChild->left != NULL)
	    oldChild->left->parent = oldChild;
    }
    oldChild->parent = grandparent;
    if (grandparent != NULL) {
	if (oldParent == grandparent->left)
	    grandparent->left = oldChild;
	else if (oldParent == grandparent->right)
	    grandparent->right = oldChild;
    }
}

void CacheSet::adjustNodes(CacheBlock* node) {
    if ((node->parent != NULL) && (node->nextTime > node->parent->nextTime)) {
	//move upward
	while ((node->parent != NULL) && (node->nextTime > node->parent->nextTime)) {
	    swapNodes(node, node->parent);
	}
	if (node->parent == NULL)
	    root = node;
    } else if (((node->right != NULL) && (node->nextTime < node->right->nextTime)) 
	       || ((node->left != NULL) && (node->nextTime < node->left->nextTime))) {
	while (((node->right != NULL) && (node->nextTime < node->right->nextTime))
	       || ((node->left != NULL) && (node->nextTime < node->left->nextTime))) {
	    if ((node->right != NULL) && (node->nextTime < node->right->nextTime) && ((node->left == NULL) || ((node->left != NULL) && (node->left->nextTime < node->right->nextTime)))) {
		//move downward to right side
		CacheBlock *temp = node->right;
		swapNodes(temp, node);
		if (temp->parent ==NULL)
		    root = temp;
	    } else {
		//move downward to left side
		CacheBlock *temp = node->left;
		swapNodes(temp, node);
		if (temp->parent ==NULL)
		    root = temp;
	    }
	}
    }
}

CacheBlock * CacheSet::deleteRoot() {
    if (size == 0) {
	cerr << "The cache set has no data to evict!" << endl;
	exit(1);
    }

    CacheBlock *oldRoot = root;
    //cout << root->addr << " is being evicted" << endl;

    if (size == 1) {
	root = NULL;
	size--;
    } else {
	CacheBlock *temp = root;
	ULONG path = size--;
	ULONG counter = sizeof(ULONG)*8;
	
	while ((path == ((path<<1)>>1)) && (counter != 0)) {
	    path = path<<1;
	    counter--;
	}
	path = path<<1;
	counter--;
	
	ULONG oldPath;
	while (counter != 1) {
	    oldPath = path;
	    path = (path<<1)>>1;
	    if (oldPath == path)
		temp = temp->left;
	    else
		temp = temp->right;
	    path = path<<1;
	    counter--;
	}
	
	oldPath = path;
	path = (path<<1)>>1;
	CacheBlock *lastNode;
	if (oldPath == path) {
	    lastNode = temp->left;
	    if (temp != root)
		temp->left = NULL;
	} else {
	    lastNode = temp->right;
	    if (temp != root)
		temp->right = NULL;
	}
	CacheBlock *rootLeftChild = root->left;
	CacheBlock *rootRightChild = root->right;
	if (lastNode != rootLeftChild) {
	    lastNode->left = rootLeftChild;
	    if (rootLeftChild != NULL)
	      rootLeftChild->parent = lastNode;
	}
	else
	    lastNode->left = NULL;
	if (lastNode != rootRightChild) {
	    lastNode->right = rootRightChild;
	    if (rootRightChild != NULL)
	      rootRightChild->parent = lastNode;
	}
	else
	    lastNode->right = NULL;
	lastNode->parent = NULL;
	root = lastNode;

	adjustNodes(root);
    }
    //output();
    return oldRoot;
}

void CacheSet::insertNode(CacheBlock* node){
    if (root == NULL) {
	root = node;
	size++;
	return;
    }

    CacheBlock *temp = root;
    ULONG path = ++size;
    ULONG counter = sizeof(ULONG)*8;

    while ((path == ((path<<1)>>1)) && (counter != 0)) {
	path = path<<1;
	counter--;
    }
    path = path<<1;
    counter--;

    ULONG oldPath;
    while (counter != 1) {
	oldPath = path;
	path = (path<<1)>>1;
	if (oldPath == path)
	    temp = temp->left;
	else
	    temp = temp->right;
	path = path<<1;
	counter--;
    }
    
    oldPath = path;
    path = (path<<1)>>1;
    if (oldPath == path) {
	temp->left = node;
	node->parent = temp;
    } else {
	temp->right = node;
	node->parent = temp;
    }

    //output();

    adjustNodes(node);
}

void CacheSet::output() {
    cout << "size: " << size << endl;
    outputNode(root);
}

void CacheSet::outputNode(CacheBlock *node) {
    if (node != NULL) {
	cout << "node: " << node->addr;
	if (node->parent != NULL)
	    cout << "\tparent: " << node->parent->addr;
	else
	    cout << "\tparent: NULL";
	if (node->left != NULL)
	    cout << "\tleft child: " << node->left->addr;
	else
	    cout << "\tleft child: NULL";
	if (node->right != NULL)
	    cout << "\tright child: " << node->right->addr;
	else
	    cout << "\tright child: NULL";
	cout << endl;

	outputNode(node->left);
	outputNode(node->right);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
	cerr << "USAGE: cacheSize cacheLineSize associativity inputFileName" << endl;
	exit(1);
    }
    ULONG cacheSize = atol(argv[1]);
    ULONG cacheLineSize = atol(argv[2]);
    ULONG associativity = atol(argv[3]);
    string inputFileName(argv[4]);

    OPTCacheSimulator *myOPTCacheSimulator = new OPTCacheSimulator(cacheSize, cacheLineSize, associativity);
    myOPTCacheSimulator->doSimulation(inputFileName);

    return 0;
}
