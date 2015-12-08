#include <unordered_map>
#include <cmath>
#include "orderAndMatchFinder.h"

OrderAndMatchFinder::OrderAndMatchFinder(size_t pModulo, size_t pNumberOfElements, BloomierHash* pBloomierHash) {
    mModulo = pModulo;
    mNumberOfElements = pNumberOfElements;
    mPiVector = new vsize_t();
    mTauVector = new vsize_t();
    mBloomierHash = pBloomierHash;
    size_t sizeOfBloomFilter = ceil(mModulo/ 8.0);
    mBloomFilterHashesSeen = new bitVector(sizeOfBloomFilter);
    mBloomFilterNonSingeltons = new bitVector(sizeOfBloomFilter);
    mBloomFilterInstance = new bitVector(sizeOfBloomFilter);
    mBloomFilterInstanceDifferentSeed = new bitVector(sizeOfBloomFilter);
    mSeeds = new std::unordered_map<size_t, size_t>;
    mBloomFilterSeed = 42;
    mHash = new Hash();
}
OrderAndMatchFinder::~OrderAndMatchFinder() {
    delete mPiVector;
    delete mTauVector;
    delete mSeeds;
    delete mHash;
    delete mBloomFilterHashesSeen;
    delete mBloomFilterNonSingeltons;
    delete mBloomFilterInstance;
    delete mBloomFilterInstanceDifferentSeed;
}

size_t OrderAndMatchFinder::getSeed(size_t pKey) {
    unsigned char index = floor(pKey / 8.0);
    unsigned char value = 1 << (pKey % 8);
    // std::cout << "value: " << value << std::endl;
    unsigned char valueSeenBefor = (*mBloomFilterInstance)[index] & value;
    if (valueSeenBefor == value) {
        // value seen and not using default seed
        unsigned char differentSeed = (*mBloomFilterInstanceDifferentSeed)[index] & value;
        if (differentSeed == value) {
            return (*mSeeds)[pKey];
        }
        // value seen but default seed
        return MAX_VALUE - 1;
    }
    // value never seen but default seed
    return MAX_VALUE;

}
void OrderAndMatchFinder::findMatch(vsize_t* pSubset) {
    vsize_t* piVector = new vsize_t();
    vsize_t* tauVector = new vsize_t();
    int singeltonValue;
    unsigned char index = 0;
    unsigned char value = 0;
    for (size_t i = 0; i < pSubset->size(); ++i) {
        singeltonValue = tweak((*pSubset)[i], pSubset, mBloomierHash->getHashSeed());
        index = floor((*pSubset)[i] / 8.0);
        value = 1 << ((*pSubset)[i] % 8);
        (*mBloomFilterInstance)[index] = (*mBloomFilterInstance)[index] | value;
        if (singeltonValue != -1) {
            piVector->push_back((*pSubset)[i]);
            tauVector->push_back(singeltonValue);
        }
    }
    if (piVector->size() == tauVector->size()) {
        for (size_t i = 0; i < piVector->size(); ++i) {
            mPiVector->push_back((*piVector)[i]);
            mTauVector->push_back((*tauVector)[i]);        
        }
    }
    delete piVector;
    delete tauVector;
}

void OrderAndMatchFinder::find(vsize_t* pSubset) {
    this->computeNonSingeltons(pSubset);
    this->findMatch(pSubset);
}
vsize_t* OrderAndMatchFinder::getPiVector() {
    return mPiVector;
}
vsize_t* OrderAndMatchFinder::getTauVector() {
    return mTauVector;
}
int OrderAndMatchFinder::tweak (size_t pKey, vsize_t* pSubset, size_t pSeed) {
    int singelton = -1;
    size_t i = 0;
    size_t j = 0;
    unsigned char value = 0;
    unsigned char valueSeen = 0;
    unsigned char index = 0;
    vsize_t* neighbors;
    
    while (singelton == -1) {
        pSeed = pSeed+i;
        neighbors = mBloomierHash->getKNeighbors(pKey, pSeed);
        j = 0;
        for (auto it = neighbors->begin(); it != neighbors->end(); ++it) {
            index = floor((*it) / 8.0);
            value = 1 << ((*it) % 8);
            valueSeen = (*mBloomFilterHashesSeen)[index] & value;
            if (value != valueSeen) {
                (*mBloomFilterNonSingeltons)[index] = (*mBloomFilterNonSingeltons)[index] | value;
                if (mBloomierHash->getHashSeed() != pSeed) {
                    index = floor(pKey / 8.0);
                    value = 1 << (pKey % 8);
                    (*mBloomFilterInstanceDifferentSeed)[index] = (*mBloomFilterInstanceDifferentSeed)[index] | value;
                    (*mSeeds)[pKey] = pSeed;
                }
                singelton = j;
                break;
            }
            ++j;
        }
        delete neighbors;
        ++i;
    }
    return singelton;
}

void OrderAndMatchFinder::computeNonSingeltons(vsize_t* pKeyValues, size_t pSeed) {
    vsize_t* neighbors;
    unsigned char value;
    unsigned char valueSeen;
    unsigned char index;
    for (auto it = pKeyValues->begin(); it != pKeyValues->end(); ++it) {
        neighbors = mBloomierHash->getKNeighbors((*it), pSeed);
        for (auto itNeighbors = neighbors->begin(); itNeighbors != neighbors->end(); ++itNeighbors) {
            index = floor((*itNeighbors) / 8.0);
            value = 1 << ((*itNeighbors) % 8);
            valueSeen = (*mBloomFilterHashesSeen)[index] & value;
            if (valueSeen != value) {
                (*mBloomFilterNonSingeltons)[index] = (*mBloomFilterNonSingeltons)[index] | value;
            }
        }
        for (auto itNeighbors = neighbors->begin(); itNeighbors != neighbors->end(); ++itNeighbors){
            index = floor((*itNeighbors) / 8.0);
            value = 1 << ((*itNeighbors) % 8);
            (*mBloomFilterHashesSeen)[index] = (*mBloomFilterHashesSeen)[index] | value;
        }
        delete neighbors;
    } 
}