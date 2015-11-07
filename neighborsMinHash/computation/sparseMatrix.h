/**
 Copyright 2015 Joachim Wolff
 Master Thesis
 Tutor: Fabrizio Costa
 Winter semester 2015/2016

 Chair of Bioinformatics
 Department of Computer Science
 Faculty of Engineering
 Albert-Ludwig-University Freiburg im Breisgau
**/
#include <math.h>

#include <algorithm>
#include <iostream>
#include "typeDefinitionsBasic.h"

#ifndef SPARSE_MATRIX_H
#define SPARSE_MATRIX_H


class SparseMatrixFloat {

  private: 
    // stores only the pointer addresses to:
    //      if even index: size_t which is pointer for vsize
    //      if odd indeX: size_t which is pointer for vfloat
    size_t* mSparseMatrix;
    float*  mSparseMatrixValues;
    size_t* mSizesOfInstances;
    size_t mMaxNnz;
    size_t mNumberOfInstance;
  public:
    SparseMatrixFloat(size_t pNumberOfInstances, size_t pMaxNnz) {
        mSparseMatrix = new size_t [pNumberOfInstance * pMaxNnz];
        mSparseMatrixValues = new float [pNumberOfInstance * pMaxNnz];
        mSizesOfInstances = new size_t [pNumberOfInstance];
        mMaxNnz = pMaxNnz;
        mNumberOfInstances = pNumberOfInstances;
    };
    ~SparseMatrixFloat() {
        delete mSparseMatrix;
        delete mSparseMatrixValues;
        delete mSizesOfInstances;
    };
    size_t* getFeatureList() const {
        return mSparseMatrix;
    }
    size_t getNextElement(size_t pInstance, size_t pCounter) const {
        if (pCounter < mSizesOfInstances[pInstance]) {
            return mSparseMatrix[pInstance*mMaxNnz+pCounter];
        } else {
            return MAX_VALUE;
        }
    }
    size_t size() const {
        return mNumberOfInstances;
    }
    float* getSparseMatrixValues() const {
        return mSparseMatrixValues;
    }
    size_t getSizeOfInstance(size_t pInstance) const {
        return mSizesOfInstances[pInstance];
    }
    void insertElement(size_t pInstanceId, size_t pNnzCount, size_t pFeatureId, float pValue) {
        mSparseMatrix[pInstanceId*mMaxNnz + pNnzCount] = pFeatureId;
        mSparseMatrixValues[pInstanceId*mMaxNnz + pNnzCount] = pValue; 
    };

    void insertToSizesOfInstances(size_t pInstanceId, size_t pSizeOfInstance) {
        mSizesOfInstances[pInstanceId] = pSizeOfInstance;
    };

    std::vector<sortMapFloat>* euclidianDistance(const std::vector<int> pRowIdVector, const int pRowId, const size_t pNneighbors) const {
        std::vector<sortMapFloat>* returnValue = new std::vector<sortMapFloat>(pRowIdVector.size());
        size_t pointerToMatrixElement = 0;
        size_t pointerToVectorElement = 0;
        for (size_t i = 0; i < pRowIdVector.size(); ++i) {
            sortMapFloat element; 
            element.key = pRowIdVector[i];
            element.val = 0.0;
            while (pointerToMatrixElement < mSizesOfInstances[pRowIdVector[i]] || pointerToVectorElement < mSizesOfInstances[pRowId]) {
                if (mSparseMatrix[pRowIdVector[i]*mMaxNnz + pointerToMatrixElement] == mSparseMatrix[pRowId*mMaxNnz + pointerToVectorElement]) {
                    element.val += 
                    pow(mSparseMatrixValues[pRowIdVector[i]*mMaxNnz + pointerToMatrixElement] - mSparseMatrixValues[pRowId*mMaxNnz + pointerToVectorElement], 2);
                    ++pointerToMatrixElement;
                    ++pointerToVectorElement;
                } else if (mSparseMatrix[pRowIdVector[i]*mMaxNnz + pointerToMatrixElement] < mSparseMatrix[pRowId*mMaxNnz + pointerToVectorElement]) {
                    element.val += pow(mSparseMatrixValues[pRowIdVector[i]*mMaxNnz + pointerToMatrixElement], 2);
                    ++pointerToMatrixElement;
                } else {
                    element.val += pow(mSparseMatrixValues[pRowId*mMaxNnz + pointerToVectorElement], 2);
                    ++pointerToVectorElement;
                }
            }

            pointerToMatrixElement = 0;
            pointerToVectorElement = 0;
            element.val = sqrt(element.val);
            
            (*returnValue)[i] = element;
            
        }
        size_t numberOfElementsToSort = pNneighbors;
        if (pNneighbors > returnValue->size()) {
            numberOfElementsToSort = returnValue->size();
        }
        std::partial_sort(returnValue->begin(), returnValue->begin()+numberOfElementsToSort, returnValue->end(), mapSortAscByValueFloat);
        return returnValue;
    };
};
#endif // SPARSE_MATRIX_H
