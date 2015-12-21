#include "bloomierFilter.h"

BloomierFilter::BloomierFilter(const size_t pModulo, const size_t pNumberOfElements, const size_t pBitWidth, const size_t pHashSeed, const size_t pMaxBinSize){
	mModulo = pModulo;
	mNumberOfElements = pNumberOfElements;
    mHashSeed = pHashSeed;
	mBitVectorSize = ceil(pBitWidth / static_cast<float>(CHAR_BIT));
	mBloomierHash = new BloomierHash(pModulo, pNumberOfElements, mBitVectorSize, pHashSeed);
	mOrderAndMatchFinder = new OrderAndMatchFinder(pModulo, pNumberOfElements, mBloomierHash);
	mTable = new bloomierTable(pModulo);
	mValueTable = new vvsize_t_p(pModulo);
	for (size_t i = 0; i < pModulo; ++i) {
		(*mTable)[i] = new bitVector[mBitVectorSize];
	}
    mEncoder = new Encoder(mBitVectorSize);
	mPiIndex = 0; 
    mMaxBinSize = pMaxBinSize;
    // mStoredNeighbors = new std::unordered_map<size_t, vsize_t* >();
}

BloomierFilter::~BloomierFilter() {
    delete mEncoder;
    for (size_t i = 0; i < mTable->size(); ++i) {
		delete [] (*mTable)[i];
	}
    delete mTable;
    delete mValueTable;
    delete mOrderAndMatchFinder;
    delete mBloomierHash;
    // delete mStoredNeighbors; 
}
void BloomierFilter::xorOperation(bitVector* pValue, const bitVector* pMask, const vsize_t* pNeighbors) {
    for (size_t i = 0; i < mBitVectorSize; ++i) {
		pValue[i] = pMask[i];
	}
    // this->xorBitVector(pValue, pMask);
	for (auto it = pNeighbors->begin(); it != pNeighbors->end(); ++it) {
		if (*it < mTable->size()) {
			this->xorBitVector(pValue, (*mTable)[(*it)]);
		}
	}
}

void BloomierFilter::xorBitVector(bitVector* pResult, const bitVector* pInput) {
	// const size_t length = std::min(pResult->size(), pInput->size());
	for (size_t i = 0; i < mBitVectorSize; ++i) {
		pResult[i] = pResult[i] ^ pInput[i];
	}
}

const vsize_t* BloomierFilter::get(const size_t pKey) {
    
    unsigned char tries = 5;
    unsigned char i = 0;
    vsize_t* neighbors = new vsize_t(mNumberOfElements);
    const bitVector* mask = mBloomierHash->getMask(pKey);
    bitVector* valueToGet = new bitVector[mBitVectorSize];
    while (i < tries) {
        mBloomierHash->getKNeighbors(pKey, mBloomierHash->getHashSeed()+i, neighbors);
        this->xorOperation(valueToGet, mask, neighbors);
        const size_t h = mEncoder->decode(valueToGet, mBitVectorSize);
        if (h < neighbors->size()) {
            const size_t L = (*neighbors)[h];
            
            if (L < mValueTable->size()) {
                if ((*mValueTable)[L] != NULL) {
                    delete neighbors;
                    delete [] mask;
                    delete [] valueToGet;
                    return (*mValueTable)[L];
                }
            }
        }
        ++i;
    }
    delete neighbors;
    delete [] mask;
    delete [] valueToGet;
    return NULL;
    
    // bool valueSeenBefor = mOrderAndMatchFinder->getValueSeenBefor(pKey);
    // if (!valueSeenBefor) return NULL;
    
    // vsize_t* neighbors = new vsize_t(mNumberOfElements);
    // mBloomierHash->getKNeighbors(pKey, mOrderAndMatchFinder->getSeed(pKey), neighbors);
    // // std::cout << "KEY: " << pKey << std::endl;
    
    // // std::cout << "LINE: " << __LINE__ << std::endl;
    // if (neighbors == NULL) return NULL;
    // const bitVector* mask = mBloomierHash->getMask(pKey);
    
    // // std::cout << "LINE: " << __LINE__ << std::endl;
    // //  
	// bitVector* valueToGet = new bitVector[mBitVectorSize];
	// this->xorOperation(valueToGet, mask, neighbors);
	// const size_t h = mEncoder->decode(valueToGet, mBitVectorSize);
    // delete [] valueToGet;
    // delete [] mask;
    // // delete valueToGet;
	// if (h < neighbors->size()) {
	// 	const size_t L = (*neighbors)[h];
    //     delete neighbors;
	// 	if (L < mValueTable->size()) {
    // // std::cout << "LINE: " << __LINE__ << std::endl;
            
    //         if ((*mValueTable)[L] != NULL) return (*mValueTable)[L];
    // // std::cout << "LINE: " << __LINE__ << std::endl;
            
    //         return NULL;
	// 	}
	// }
    // delete neighbors;
    // // std::cout << "LINE: " << __LINE__ << std::endl;
    
	// return NULL;
}
bool BloomierFilter::set(const size_t pKey, const size_t pValue) {
    
    // bool valueSeenBevor = mOrderAndMatchFinder->getValueSeenBefor(pKey);
    // if (!valueSeenBevor) {
    //     this->create(pKey, pValue);
	// 	return true;
    // }
    unsigned char tries = 5;
    unsigned char i = 0;
    vsize_t* neighbors = new vsize_t(mNumberOfElements);
    const bitVector* mask = mBloomierHash->getMask(pKey);
    bitVector* valueToGet = new bitVector[mBitVectorSize];
    while (i < tries) {
        mBloomierHash->getKNeighbors(pKey, mBloomierHash->getHashSeed()+i, neighbors);
        this->xorOperation(valueToGet, mask, neighbors);
        const size_t h = mEncoder->decode(valueToGet, mBitVectorSize);
        if (h < neighbors->size()) {
            const size_t L = (*neighbors)[h];
            if (L < mValueTable->size()) {
                vsize_t* v = (*mValueTable)[L];
                if (v == NULL) {
                    ++i;
                    continue;
                }
                if (v->size() < mMaxBinSize) {
                    if (v->size() > 0) {
                        v->push_back(pValue);
                    }
                } else {
                    v->clear();
                }
                delete neighbors;
                delete [] mask;
                delete [] valueToGet;
                return true;
            }
        }
        ++i;
    }
    delete neighbors;
    delete [] mask;
    delete [] valueToGet;
    this->create(pKey, pValue);
    return true;
}

void BloomierFilter::create(const size_t pKey, const size_t pValue) {
	
    const vsize_t* neighbors = mOrderAndMatchFinder->findIndexAndReturnNeighborhood(pKey);
    // const vsize_t* piVector = mOrderAndMatchFinder->getPiVector();
	const vsize_t* tauVector = mOrderAndMatchFinder->getTauVector();
    // (*mStoredNeighbors)[pKey] = neighbors;
    const bitVector* mask = mBloomierHash->getMask(pKey);
    const size_t l = (*tauVector)[mPiIndex];
    const size_t L = (*neighbors)[l];

    const bitVector* encodeValue = mEncoder->encode(l);
    
	for (size_t i = 0; i < mBitVectorSize; ++i) {
		(*mTable)[L][i] = encodeValue[i] ^ mask[i];
	}

    // this->xorBitVector((*mTable)[L], encodeValue);
    // this->xorBitVector((*mTable)[L], mask);
    
    for (size_t j = 0; j < neighbors->size(); ++j) {
        if (j != l) {
            this->xorBitVector((*mTable)[L], (*mTable)[(*neighbors)[j]]);
        }
    }
    if ((*mValueTable)[L] == NULL) {
            (*mValueTable)[L] = new vsize_t(1);
            (*mValueTable)[L]->operator[](0) = pValue;
    }
    delete [] encodeValue;
    delete [] mask;
    delete neighbors;
    ++mPiIndex;
}