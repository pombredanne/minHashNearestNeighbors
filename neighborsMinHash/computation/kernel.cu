/**
 Copyright 2016 Joachim Wolff
 Master Thesis
 Tutor: Fabrizio Costa, Milad Miladi
 Winter semester 2015/2016

 Chair of Bioinformatics
 Department of Computer Science
 Faculty of Engineering
 Albert-Ludwigs-University Freiburg im Breisgau
**/

#include <stdio.h>
#include "sparseMatrix.h"
// #include <math.h>
#include "kernel.h"

__device__ size_t computeHashValueCuda(size_t key, size_t aModulo) {
    // source:  Thomas Wang: Integer Hash Functions, 1997 / 2007 
    // https://gist.github.com/badboy/6267743
    key = key * A;
    key = ~key + (key << 15);
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057;
    key = key ^ (key >> 16);
    return key % aModulo;
}

__global__ void fitCuda(const size_t* pFeatureIdList, const size_t* pSizeOfInstanceList,
                    const size_t pNumberOfHashFunctions, const size_t pMaxNnz,
                    size_t* pComputedSignatures, 
                    const size_t pNumberOfInstances, const size_t pStartInstance, 
                    const size_t pBlockSize, const size_t pShingleSize) {
    // extern __shared__ size_t signature[];  // pNumberOfHashFunctions
    int instanceId = blockIdx.x + pStartInstance;
    size_t minHashValue = MAX_VALUE;
    size_t hashValue = 0;
    
    int featureId = blockIdx.x * pMaxNnz;
    int hashFunctionId = threadIdx.x;
    size_t sizeOfInstance;// = pSizeOfInstanceList[blockId];
    // printf("pStartInstance: %i", pStartInstance);
    // printf("pNumberOfInstances: %i", pNumberOfInstances);
    // printf()
    // if (blockIdx.x == 0 && threadIdx.x == 0) {
    //     printf("Grid dim: %i" , gridDim.x); 
    //     printf("BLock id: %i" , blockIdx.x);
    //     printf("ffpp");
    // }
    while (instanceId < pNumberOfInstances) {
        
        sizeOfInstance = pSizeOfInstanceList[instanceId];
        while (hashFunctionId < pNumberOfHashFunctions && featureId < pNumberOfInstances*pMaxNnz) {
            for (size_t i = 0; i < sizeOfInstance; ++i) {
                hashValue = computeHashValueCuda((pFeatureIdList[featureId + i]+1) * (hashFunctionId+1), MAX_VALUE);
                if (hashValue < minHashValue) {
                    minHashValue = hashValue;
                }
            }
            // printf("%i ", (instanceId-pStartInstance)*pNumberOfHashFunctions + hashFunctionId);
            pComputedSignatures[(instanceId-pStartInstance)*pNumberOfHashFunctions + hashFunctionId] = minHashValue;
            hashFunctionId += blockDim.x;
        }
        instanceId += gridDim.x;
        featureId = instanceId * pMaxNnz;
        minHashValue = MAX_VALUE;
        hashFunctionId = threadIdx.x;
        // __syncthreads();
    }
}



__global__ void queryCuda(size_t* pHitsPerInstance, size_t* pSizePerInstance,
                            size_t pNeighborhoodSize,
                            size_t* pNeighborhood,
                            float* pDistances) {
    
}

__global__ void euclidianDistanceCuda(size_t* pFeatureIds, size_t* pSizeOfInstanceList,
                                    float* pFeatureValues, size_t pMaxNnz,
                                    size_t* pPossibleInstances, size_t* pSizePerInstance, size_t pMaxCandidates,                             
                                    size_t* pHitsToBeReturned, float* pValuesToBeReturned) {
    // extern __shared__ float euclidianDistance [];
    // size_t queryId;
    // size_t instanceId;
    // float queryValue;
    // float instanceValue;
    // float value;
    // size_t accessIdQuery;
    // size_t accessIdInstance;
    // size_t queryCount = 0;
    
    // // while (queryCount < )
    
    // queryId = pFeatureIds[accessIdQuery];
    // instanceId = pFeatureIds[accessIdInstance];
    // queryValue = pFeatureValues[accessIdQuery];
    // instanceValue = pFeatureValues[accessIdInstance];
    
    // if (queryId == instanceId) {
    //     value = queryValue - instanceValue;
    // } else if (queryId < instanceId) {
    //     value = -instanceValue;
    // } else {
    //     value = -queryValue;
    // }
    // euclidianDistance = powf(value, 2);
    
}

__global__ void cosineSimilarityCuda() {
    
}