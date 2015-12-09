#include "inverseIndexStorageBloomierFilter.h"

InverseIndexStorageBloomierFilter::InverseIndexStorageBloomierFilter(const size_t pSizeOfInverseIndex, const size_t pMaxBinSize, const size_t pMaximalFeatures) {
	mInverseIndex = new std::vector<BloomierFilter* > (pSizeOfInverseIndex);
	mMaxBinSize = pMaxBinSize;
	mM = 3500;
	mK = 2;
	mQ = 8;
}
InverseIndexStorageBloomierFilter::~InverseIndexStorageBloomierFilter() {
	
}
size_t InverseIndexStorageBloomierFilter::size() const{
	return mInverseIndex->size();
}
vsize_t* InverseIndexStorageBloomierFilter::getElement(size_t pVectorId, size_t pHashValue) {
    return (*mInverseIndex)[pVectorId]->get(pHashValue); 
}
void InverseIndexStorageBloomierFilter::insert(size_t pVectorId, size_t pHashValue, size_t pInstance) {
    if ((*mInverseIndex)[pVectorId] == NULL) {
        (*mInverseIndex)[pVectorId] = new BloomierFilter(mM, mK, mQ, 100, mMaxBinSize);
    }
    (*mInverseIndex)[pVectorId]->set(pHashValue, pInstance);
}