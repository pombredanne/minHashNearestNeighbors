/**
 Copyright 2015 Joachim Wolff
 Master Thesis
 Tutors: Milad Miladi, Fabrizio Costa
 Winter semester 2015/2016

 Chair of Bioinformatics
 Department of Computer Science
 Faculty of Engineering
 Albert-Ludwig-University Freiburg im Breisgau
**/

#include <iostream>
#include <iterator>
#include <algorithm>
#include <utility>

#ifdef OPENMP
#include <omp.h>
#endif

#include "inverseIndex.h"


class sort_map {
  public:
    size_t key;
    size_t val;
};

bool mapSortDescByValue(const sort_map& a, const sort_map& b) {
        return a.val > b.val;
};

InverseIndex::inverseIndex(size_t pNumberOfHashFunctions, size_t pBlockSize,
                    size_t pNumberOfCores, size_t pChunkSize,
                    size_t pMaxBinSize,
                    size_t pSizeOfNeighborhood, size_t pMinimalBlocksInCommon,
                    size_t pExcessFactor, size_t pMaximalNumberOfHashCollisions) {
    
    numberOfHashFunctions = pNumberOfHashFunctions;
    blockSize = pBlockSize;
    numberOfCores = pNumberOfCores;
    chunkSize = pChunkSize;
    maxBinSize = pMaxBinSize;
    sizeOfNeighborhood = pSizeOfNeighborhood;
    minimalBlocksInCommon = pMinimalBlocksInCommon;
    excessFactor = pExcessFactor;
    maximalNumberOfHashCollisions = pMaximalNumberOfHashCollisions;
    inverseIndex = new std::vector<umapVector >();
    signatureStorage = new umap_pair_vector();
    size_t inverseIndexSize = ceil(((float) numberOfHashFunctions / (float) blockSize)+1);
    inverseIndex->resize(inverseIndexSize);
}

InverseIndex::~InverseIndex() {
    delete signatureStorage;
    delete inverseIndex;
}
 // compute the signature for one instance
vsize_t InverseIndex::computeSignature(const vsize_t& featureVector) {

    vsize_t signatureHash;
    signatureHash.reserve(numberOfHashFunctions);

    for(size_t j = 0; j < numberOfHashFunctions; ++j) {
        size_t minHashValue = MAX_VALUE;
        for (size_t i = 0; i < featureVector.size(); ++i) {
            size_t hashValue = _size_tHashSimple((featureVector[i] +1) * (j+1) * A, MAX_VALUE);
            if (hashValue < minHashValue) {
                minHashValue = hashValue;
            }
        }
        signatureHash[j] = minHashValue;
    }
    // reduce number of hash values by a factor of blockSize
    size_t k = 0;
    vsize_t signature;
    signature.reserve((numberOfHashFunctions / blockSize) + 1);
    while (k < (numberOfHashFunctions)) {
        // use computed hash value as a seed for the next computation
        size_t signatureBlockValue = signatureHash[k];
        for (size_t j = 0; j < blockSize; ++j) {
            signatureBlockValue = _size_tHashSimple((signatureHash[k+j]) * signatureBlockValue * A, MAX_VALUE);
        }
        signature.push_back(signatureBlockValue);
        k += blockSize; 
    }
    return signature;
}
umap_pair_vector* InverseIndex::computeSignatureMap(const umapVector& instanceFeatureVector) {
    mDoubleElementsQuery = 0;
    const size_t sizeOfInstances = instanceFeatureVector.size();
    umap_pair_vector* instanceSignature = new umap_pair_vector();
    (*instanceSignature).reserve(sizeOfInstances);
    if (chunkSize <= 0) {
        chunkSize = ceil(instanceFeatureVector.size() / static_cast<float>(numberOfCores));
    }
#ifdef OPENMP
    omp_set_dynamic(0);
#endif


#pragma omp parallel for schedule(static, chunkSize) num_threads(numberOfCores)
    for(size_t index = 0; index < instanceFeatureVector.size(); ++index) {

        auto instanceId = instanceFeatureVector.begin();
        std::advance(instanceId, index);
        
        // compute unique id
        size_t signatureId = 0;
        for (auto itFeatures = instanceId->second.begin(); itFeatures != instanceId->second.end(); ++itFeatures) {
                signatureId = _size_tHashSimple((*itFeatures +1) * (signatureId+1) * A, MAX_VALUE);
        }
        auto signatureIt = (*signatureStorage).find(signatureId);
        if (signatureIt != (*signatureStorage).end() && !(instanceSignature->find(signatureId) == instanceSignature->end())) {
#pragma omp critical
            {
                instanceSignature->operator[](signatureId) = (*signatureStorage)[signatureId];
                mDoubleElementsQuery += (*signatureStorage)[signatureId].first.size();
            }
            continue;
        }
        // for every hash function: compute the hash values of all features and take the minimum of these
        // as the hash value for one hash function --> h_j(x) = argmin (x_i of x) f_j(x_i)
        vsize_t signature = computeSignature(instanceId->second);
#pragma omp critical
        {
            if (instanceSignature->find(signatureId) == instanceSignature->end()) {
                vsize_t doubleInstanceVector(1);
                doubleInstanceVector[0] = instanceId->first;
                instanceSignature->operator[](signatureId) = std::make_pair (doubleInstanceVector, signature);
            } else {
                instanceSignature->operator[](signatureId).first.push_back(instanceId->first);
                mDoubleElementsQuery += 1;
            }
        }
    }
    return instanceSignature;
}
void InverseIndex::fit(const umapVector* instanceFeatureVector) {
    mDoubleElementsStorage = 0;
    size_t inverseIndexSize = ceil(((float) numberOfHashFunctions / (float) blockSize)+1);
    mInverseIndexUmapVector->resize(inverseIndexSize);

    if (chunkSize <= 0) {
        chunkSize = ceil(instanceFeatureVector->size() / static_cast<float>(numberOfCores));
    }
#ifdef OPENMP
    omp_set_dynamic(0);
#endif

#pragma omp parallel for schedule(static, chunkSize) num_threads(numberOfCores)
    for(size_t index = 0; index < instanceFeatureVector->size(); ++index) {

        auto instanceId = instanceFeatureVector->begin();
        std::advance(instanceId, index);

        vmSize_tSize_t hashStorage;
        vsize_t signatureHash(numberOfHashFunctions);
        // for every hash function: compute the hash values of all features and take the minimum of these
        // as the hash value for one hash function --> h_j(x) = argmin (x_i of x) f_j(x_i)
        vsize_t signature = computeSignature(instanceId->second);
        size_t signatureId = 0;
        for (auto itFeatures = instanceId->second.begin(); itFeatures != instanceId->second.end(); ++itFeatures) {
            signatureId = _size_tHashSimple((*itFeatures +1) * (signatureId+1) * A, MAX_VALUE);
        }


        // insert in inverse index
#pragma omp critical
        {    
            if (signatureStorage->find(signatureId) == signatureStorage->end()) {
                vsize_t doubleInstanceVector(1);
                doubleInstanceVector[0] = instanceId->first;
                signatureStorage->operator[](signatureId) = std::make_pair (doubleInstanceVector, signature);
            } else {
                 signatureStorage->operator[](signatureId).first.push_back(instanceId->first);
                 mDoubleElementsStorage += 1;
            }
            for (size_t j = 0; j < signature.size(); ++j) {
                auto itHashValue_InstanceVector = mInverseIndexUmapVector->operator[](j).find(signature[j]);
                // if for hash function h_i() the given hash values is already stored
                if (itHashValue_InstanceVector != mInverseIndexUmapVector->operator[](j).end()) {
                    // insert the instance id if not too many collisions (maxBinSize)
                    if (itHashValue_InstanceVector->second.size() < maxBinSize) {
                        // insert only if there wasn't any collisions in the past
                        if (itHashValue_InstanceVector->second.size() > 0) {
                            itHashValue_InstanceVector->second.push_back(instanceId->first);
                        }
                    } else { 
                        // too many collisions: delete stored ids. empty vector is interpreted as an error code 
                        // for too many collisions
                        itHashValue_InstanceVector->second.clear();
                    }
                } else {
                    // given hash value for the specific hash function was not avaible: insert new hash value
                    vsize_t instanceIdVector;
                    instanceIdVector.push_back(instanceId->first);
                    mInverseIndexUmapVector->operator[](j)[signature[j]] = instanceIdVector;
                }
            }
        }
    }
}

neighborhood InverseIndex::kneighbors(const umap_pair_vector* signaturesMap) {

    size_t doubleElements = 0;
#ifdef OPENMP
    omp_set_dynamic(0);
#endif
    vvsize_t* neighbors = new vvsize_t();
    vvfloat* distances = new vvsize_t();
    neighbors->resize(signaturesMap->size()+doubleElements);
    distances->resize(signaturesMap->size()+doubleElements);
    if (chunkSize <= 0) {
        chunkSize = ceil(mInverseIndexUmapVector->size() / static_cast<float>(numberOfCores));
    }

#pragma omp parallel for schedule(static, chunkSize) num_threads(numberOfCores)
    for (size_t i = 0; i < signaturesMap->size(); ++i) {
        umap_pair_vector::const_iterator instanceId = signaturesMap->begin();
        std::advance(instanceId, i); 
        
        std::unordered_map<size_t, size_t> neighborhood;
        const vsize_t signature = instanceId->second.second;
        for (size_t j = 0; j < signature.size(); ++j) {
            size_t hashID = signature[j];
            if (hashID != 0 && hashID != MAX_VALUE) {
                size_t collisionSize = 0;
                umapVector::const_iterator instances = mInverseIndexUmapVector->at(j).find(hashID);
                if (instances != mInverseIndexUmapVector->at(j).end()) {
                    collisionSize = instances->second.size();
                } else { 
                    continue;
                }

                if (collisionSize < maxBinSize && collisionSize > 0) {
                    for (size_t k = 0; k < instances->second.size(); ++k) {
                        neighborhood[instances->second.at(k)] += 1;
                    }
                }
            }
        }

        std::vector< sort_map > neighborhoodVectorForSorting;
        
        for (auto it = neighborhood.begin(); it != neighborhood.end(); ++it) {
            sort_map mapForSorting;
            mapForSorting.key = (*it).first;
            mapForSorting.val = (*it).second;
            neighborhoodVectorForSorting.push_back(mapForSorting);
        }
        std::sort(neighborhoodVectorForSorting.begin(), neighborhoodVectorForSorting.end(), mapSortDescByValue);
        vsize_t neighborhoodVector;
        std::vector<float> distanceVector;

        size_t sizeOfNeighborhoodAdjusted = std::min(static_cast<size_t>(sizeOfNeighborhood * excessFactor), neighborhoodVectorForSorting.size());
        size_t count = 0;

        for (auto it = neighborhoodVectorForSorting.begin();
                it != neighborhoodVectorForSorting.end(); ++it) {
            neighborhoodVector.push_back((*it).key);
            distanceVector.push_back(1 - ((*it).val / maximalNumberOfHashCollisions));
            ++count;
            if (count == sizeOfNeighborhoodAdjusted) {
                break;
            }
        }

#pragma omp critical
        {
            for (size_t j = 0; j < instanceId->second.first.size(); ++j) {
                (*neighbors)[instanceId->second.first[j]] = neighborhoodVector;
                (*distances)[instanceId->second.first[j]] = distanceVector;
            }
        }
    }
    neighborhood neighborhood_;
    neighborhood_.neighbors = neighbors;
    neighborhood_.distances = distances
    return _neighborhood;
}