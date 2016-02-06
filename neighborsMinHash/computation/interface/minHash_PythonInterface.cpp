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
#include <Python.h>

#include "../minHash.h"
#ifdef CUDA
#include "../minHashCuda.h"
#endif
#include "../parsePythonToCpp.h"

static neighborhood* neighborhoodComputation(size_t pMinHashAddress, PyObject* pInstancesListObj,PyObject* pFeaturesListObj,PyObject* pDataListObj, 
                                                   size_t pMaxNumberOfInstances, size_t pMaxNumberOfFeatures, 
                                                   size_t pNneighbors, int pFast, int pSimilarity) {
    // std::cout << "20" << std::endl;
    SparseMatrixFloat* originalDataMatrix = NULL;
    if (pMaxNumberOfInstances != 0) {
        originalDataMatrix = parseRawData(pInstancesListObj, pFeaturesListObj, pDataListObj, 
                                                    pMaxNumberOfInstances, pMaxNumberOfFeatures);
    }
    MinHashBase* minHash = reinterpret_cast<MinHashBase* >(pMinHashAddress);

    // compute the k-nearest neighbors
    neighborhood* neighbors_ =  minHash->kneighbors(originalDataMatrix, pNneighbors, pFast, pSimilarity);
    if (originalDataMatrix != NULL) {
        delete originalDataMatrix;    
    } 
    return neighbors_;
}

static neighborhood* fitNeighborhoodComputation(size_t pMinHashAddress, PyObject* pInstancesListObj,PyObject* pFeaturesListObj,PyObject* pDataListObj, 
                                                   size_t pMaxNumberOfInstances, size_t pMaxNumberOfFeatures, 
                                                   size_t pNneighbors, int pFast, int pSimilarity) {
    SparseMatrixFloat* originalDataMatrix = parseRawData(pInstancesListObj, pFeaturesListObj, pDataListObj, 
                                                    pMaxNumberOfInstances, pMaxNumberOfFeatures);
    // get pointer to the minhash object
    MinHashBase* minHash = reinterpret_cast<MinHashBase* >(pMinHashAddress);
    minHash->set_mOriginalData(originalDataMatrix);

    minHash->fit(originalDataMatrix);
    SparseMatrixFloat* emptyMatrix = NULL;
    neighborhood* neighborhood_ = minHash->kneighbors(emptyMatrix, pNneighbors, pFast, pSimilarity);
    // delete emptyMatrix;
    return neighborhood_;
}

static PyObject* createObject(PyObject* self, PyObject* args) {
    size_t numberOfHashFunctions, shingleSize, numberOfCores, chunkSize,
    nNeighbors, minimalBlocksInCommon, maxBinSize,
    maximalNumberOfHashCollisions, excessFactor, bloomierFilter, hashAlgorithm,
     blockSize, shingle, removeValueWithLeastSigificantBit, cuda;
    int fast, similarity, pruneInverseIndex, removeHashFunctionWithLessEntriesAs;
    float pruneInverseIndexAfterInstance;
    
    if (!PyArg_ParseTuple(args, "kkkkkkkkkiikifikkkkk", &numberOfHashFunctions,
                        &shingleSize, &numberOfCores, &chunkSize, &nNeighbors,
                        &minimalBlocksInCommon, &maxBinSize,
                        &maximalNumberOfHashCollisions, &excessFactor, &fast, &similarity, &bloomierFilter,
                        &pruneInverseIndex,&pruneInverseIndexAfterInstance, &removeHashFunctionWithLessEntriesAs,
                        &hashAlgorithm, &blockSize, &shingle, &removeValueWithLeastSigificantBit, &cuda))
        return NULL;
        // std::cout << __LINE__ << std::endl;
    MinHashBase* minHash;
    minHash = NULL;
    if (cuda == 0) {
        printf("%i\n", __LINE__);
        minHash = new MinHash (numberOfHashFunctions, shingleSize, numberOfCores, chunkSize,
                        maxBinSize, nNeighbors, minimalBlocksInCommon, 
                        excessFactor, maximalNumberOfHashCollisions, fast, similarity, bloomierFilter, pruneInverseIndex,
                        pruneInverseIndexAfterInstance, removeHashFunctionWithLessEntriesAs, 
                        hashAlgorithm, blockSize, shingle, removeValueWithLeastSigificantBit);
        printf("%i\n", __LINE__);
                        
    } else if (cuda == 1) {
#ifdef CUDA
        printf("%i\n", __LINE__);

        minHash = new MinHashCuda(numberOfHashFunctions, shingleSize, numberOfCores, chunkSize,
                        maxBinSize, nNeighbors, minimalBlocksInCommon, 
                        excessFactor, maximalNumberOfHashCollisions, fast, similarity, bloomierFilter, pruneInverseIndex,
                        pruneInverseIndexAfterInstance, removeHashFunctionWithLessEntriesAs, 
                        hashAlgorithm, blockSize, shingle, removeValueWithLeastSigificantBit);
        printf("%i\n", __LINE__);
                        
#endif
        if (minHash == NULL) {
        printf("%i\n", __LINE__);
            
            minHash = new MinHash (numberOfHashFunctions, shingleSize, numberOfCores, chunkSize,
                        maxBinSize, nNeighbors, minimalBlocksInCommon, 
                        excessFactor, maximalNumberOfHashCollisions, fast, similarity, bloomierFilter, pruneInverseIndex,
                        pruneInverseIndexAfterInstance, removeHashFunctionWithLessEntriesAs, 
                        hashAlgorithm, blockSize, shingle, removeValueWithLeastSigificantBit);
        printf("%i\n", __LINE__);
                        
        }
    }
        // std::cout << __LINE__ << std::endl;
        printf("%i\n", __LINE__);

    size_t adressMinHashObject = reinterpret_cast<size_t>(minHash);
    PyObject* pointerToInverseIndex = Py_BuildValue("k", adressMinHashObject);
        printf("%i\n", __LINE__);
    
    return pointerToInverseIndex;
}
static PyObject* deleteObject(PyObject* self, PyObject* args) {
    size_t addressMinHashObject;

    if (!PyArg_ParseTuple(args, "k", &addressMinHashObject))
        return Py_BuildValue("i", 1);;

    MinHashBase* minHash = reinterpret_cast<MinHashBase* >(addressMinHashObject);
    delete minHash;
    return Py_BuildValue("i", 0);
}

static PyObject* fit(PyObject* self, PyObject* args) {
        printf("%i\n", __LINE__);
    
    size_t addressMinHashObject, maxNumberOfInstances, maxNumberOfFeatures;
    PyObject* instancesListObj, *featuresListObj, *dataListObj;

    if (!PyArg_ParseTuple(args, "O!O!O!kkk", 
                            &PyList_Type, &instancesListObj, 
                            &PyList_Type, &featuresListObj,
                            &PyList_Type, &dataListObj,
                            &maxNumberOfInstances,
                            &maxNumberOfFeatures,
                            &addressMinHashObject))
        return NULL;
        printf("%i\n", __LINE__);
    
    // parse from python list to a c++ map<size_t, vector<size_t> >
    // where key == instance id and vector<size_t> == non null feature ids
    SparseMatrixFloat* originalDataMatrix = parseRawData(instancesListObj, featuresListObj, dataListObj, 
                                                    maxNumberOfInstances, maxNumberOfFeatures);
    // std::cout << __LINE__ << std::endl;
        printf("%i\n", __LINE__);
 
    // get pointer to the minhash object
    MinHashCuda* minHash = reinterpret_cast<MinHashCuda* >(addressMinHashObject);
        // std::cout << __LINE__ << std::endl;
        printf("%i\n", __LINE__);

    minHash->set_mOriginalData(originalDataMatrix);
    // std::cout << __LINE__ << std::endl;
        printf("%i\n", __LINE__);

    minHash->fit(originalDataMatrix);
    // std::cout << __LINE__ << std::endl;
        printf("%i\n", __LINE__);

    addressMinHashObject = reinterpret_cast<size_t>(minHash);
    PyObject * pointerToInverseIndex = Py_BuildValue("k", addressMinHashObject);
        printf("%i\n", __LINE__);
    
    return pointerToInverseIndex;
}
static PyObject* partialFit(PyObject* self, PyObject* args) {
    return fit(self, args);
}
static PyObject* kneighbors(PyObject* self, PyObject* args) {
    // std::cout << __LINE__ << std::endl;
    
    size_t addressMinHashObject, nNeighbors, maxNumberOfInstances,
            maxNumberOfFeatures, returnDistance;
    int fast, similarity;
    PyObject* instancesListObj, *featuresListObj, *dataListObj;

    if (!PyArg_ParseTuple(args, "O!O!O!kkkkiik", 
                        &PyList_Type, &instancesListObj,
                        &PyList_Type, &featuresListObj,  
                        &PyList_Type, &dataListObj,
                        &maxNumberOfInstances,
                        &maxNumberOfFeatures,
                        &nNeighbors, &returnDistance,
                        &fast, &similarity, &addressMinHashObject))
        return NULL;

    // compute the k-nearest neighbors
    neighborhood* neighborhood_ = neighborhoodComputation(addressMinHashObject, instancesListObj, featuresListObj, dataListObj, 
                                                maxNumberOfInstances, maxNumberOfFeatures, nNeighbors, fast, similarity);

    size_t cutFirstValue = 0;
    if (PyList_Size(instancesListObj) == 0) {
        cutFirstValue = 1;
    }
    if (nNeighbors == 0) {
        MinHashBase* minHash = reinterpret_cast<MinHashBase* >(addressMinHashObject);
        nNeighbors = minHash->getNneighbors();
    }
    // std::cout << "140" << std::endl;

    return bringNeighborhoodInShape(neighborhood_, nNeighbors, cutFirstValue, returnDistance);
}

static PyObject* kneighborsGraph(PyObject* self, PyObject* args) {
    size_t addressMinHashObject, nNeighbors, maxNumberOfInstances,
            maxNumberOfFeatures, returnDistance, symmetric;
    int fast, similarity;
    PyObject* instancesListObj, *featuresListObj, *dataListObj;

    if (!PyArg_ParseTuple(args, "O!O!O!kkkkikik", 
                        &PyList_Type, &instancesListObj,
                        &PyList_Type, &featuresListObj,  
                        &PyList_Type, &dataListObj,
                        &maxNumberOfInstances,
                        &maxNumberOfFeatures,
                        &nNeighbors, &returnDistance,
                        &fast, &symmetric, &similarity, &addressMinHashObject))
        return NULL;
    // compute the k-nearest neighbors
    neighborhood* neighborhood_ = neighborhoodComputation(addressMinHashObject, instancesListObj, featuresListObj, dataListObj, 
                                                maxNumberOfInstances, maxNumberOfFeatures, nNeighbors, fast, similarity);
    if (nNeighbors == 0) {
        MinHashBase* minHash = reinterpret_cast<MinHashBase* >(addressMinHashObject);
        nNeighbors = minHash->getNneighbors();
    }
    return buildGraph(neighborhood_, nNeighbors, returnDistance, symmetric);
}
static PyObject* radiusNeighbors(PyObject* self, PyObject* args) {
    size_t addressMinHashObject, radius, maxNumberOfInstances,
             maxNumberOfFeatures, returnDistance, similarity;
    int fast;
    PyObject* instancesListObj, *featuresListObj, *dataListObj;

    if (!PyArg_ParseTuple(args, "O!O!O!kkkkiik", 
                        &PyList_Type, &instancesListObj,
                        &PyList_Type, &featuresListObj,  
                        &PyList_Type, &dataListObj,
                        &maxNumberOfInstances,
                        &maxNumberOfFeatures,
                        &radius, &returnDistance,
                        &fast,&similarity, &addressMinHashObject))
        return NULL;
    // compute the k-nearest neighbors
    neighborhood* neighborhood_ = neighborhoodComputation(addressMinHashObject, instancesListObj, featuresListObj, dataListObj, 
                                                maxNumberOfInstances, maxNumberOfFeatures, MAX_VALUE, fast, similarity);
    size_t cutFirstValue = 0;
    if (PyList_Size(instancesListObj) == 0) {
        cutFirstValue = 1;
    }
    return radiusNeighborhood(neighborhood_, radius, cutFirstValue, returnDistance); 
}
static PyObject* radiusNeighborsGraph(PyObject* self, PyObject* args) {
    size_t addressMinHashObject, radius, maxNumberOfInstances,
            maxNumberOfFeatures, returnDistance, symmetric;
    int fast, similarity;
    PyObject* instancesListObj, *featuresListObj, *dataListObj;

    if (!PyArg_ParseTuple(args, "O!O!O!kkkkikik", 
                        &PyList_Type, &instancesListObj,
                        &PyList_Type, &featuresListObj,  
                        &PyList_Type, &dataListObj,
                        &maxNumberOfInstances,
                        &maxNumberOfFeatures,
                        &radius, &returnDistance,
                        &fast, &symmetric, &similarity, &addressMinHashObject))
        return NULL;
    // compute the k-nearest neighbors
    neighborhood* neighborhood_ = neighborhoodComputation(addressMinHashObject, instancesListObj, featuresListObj, dataListObj, 
                                                maxNumberOfInstances, maxNumberOfFeatures, MAX_VALUE, fast, similarity);
    return radiusNeighborhoodGraph(neighborhood_, radius, returnDistance, symmetric); 
}
static PyObject* fitKneighbors(PyObject* self, PyObject* args) {
    size_t addressMinHashObject, maxNumberOfInstances, maxNumberOfFeatures,
            nNeighbors, returnDistance;
    int fast, similarity;
    PyObject* instancesListObj, *featuresListObj, *dataListObj;

    if (!PyArg_ParseTuple(args, "O!O!O!kkkkiik", 
                            &PyList_Type, &instancesListObj, 
                            &PyList_Type, &featuresListObj,
                            &PyList_Type, &dataListObj,
                            &maxNumberOfInstances,
                            &maxNumberOfFeatures,
                            &nNeighbors,
                            &returnDistance, &fast,
                            &similarity,
                            &addressMinHashObject))
        return NULL;

    neighborhood* neighborhood_ = fitNeighborhoodComputation(addressMinHashObject, instancesListObj, featuresListObj, dataListObj, 
                                                   maxNumberOfInstances, maxNumberOfFeatures, nNeighbors, fast, similarity);
    size_t cutFirstValue = 1;
    if (nNeighbors == 0) {
        MinHashBase* minHash = reinterpret_cast<MinHashBase* >(addressMinHashObject);
        nNeighbors = minHash->getNneighbors();
    }
    return bringNeighborhoodInShape(neighborhood_, nNeighbors, cutFirstValue, returnDistance);
}
static PyObject* fitKneighborsGraph(PyObject* self, PyObject* args) {
    size_t addressMinHashObject, maxNumberOfInstances, maxNumberOfFeatures,
            nNeighbors, returnDistance, symmetric;
    int fast, similarity;
    PyObject* instancesListObj, *featuresListObj, *dataListObj;

    if (!PyArg_ParseTuple(args, "O!O!O!kkkikik", 
                            &PyList_Type, &instancesListObj, 
                            &PyList_Type, &featuresListObj,
                            &PyList_Type, &dataListObj,
                            &maxNumberOfInstances,
                            &maxNumberOfFeatures,
                            &nNeighbors,
                            &returnDistance, &fast, &symmetric,
                            &similarity,
                            &addressMinHashObject))
        return NULL;

    neighborhood* neighborhood_ = fitNeighborhoodComputation(addressMinHashObject, instancesListObj, featuresListObj, dataListObj, 
                                                   maxNumberOfInstances, maxNumberOfFeatures, nNeighbors, fast, similarity);
    if (nNeighbors == 0) {
        MinHashBase* minHash = reinterpret_cast<MinHashBase* >(addressMinHashObject);
        nNeighbors = minHash->getNneighbors();
    }
    return buildGraph(neighborhood_, nNeighbors, returnDistance, symmetric);

}
static PyObject* fitRadiusNeighbors(PyObject* self, PyObject* args) {
    size_t addressMinHashObject, maxNumberOfInstances, maxNumberOfFeatures,
            radius, returnDistance;
    int fast, similarity;
    PyObject* instancesListObj, *featuresListObj, *dataListObj;

    if (!PyArg_ParseTuple(args, "O!O!O!kkkkiik", 
                            &PyList_Type, &instancesListObj, 
                            &PyList_Type, &featuresListObj,
                            &PyList_Type, &dataListObj,
                            &maxNumberOfInstances,
                            &maxNumberOfFeatures,
                            &radius, &returnDistance,
                            &fast, &similarity,
                            &addressMinHashObject))
        return NULL;

    neighborhood* neighborhood_ = fitNeighborhoodComputation(addressMinHashObject, instancesListObj, featuresListObj, dataListObj, 
                                                   maxNumberOfInstances, maxNumberOfFeatures, MAX_VALUE, fast, similarity); 
    size_t cutFirstValue = 1;
    return radiusNeighborhood(neighborhood_, radius, cutFirstValue, returnDistance); 
}
static PyObject* fitRadiusNeighborsGraph(PyObject* self, PyObject* args) {
    size_t addressMinHashObject, maxNumberOfInstances, maxNumberOfFeatures,
            radius, returnDistance, symmetric;
    int fast, similarity;
    PyObject* instancesListObj, *featuresListObj, *dataListObj;

    if (!PyArg_ParseTuple(args, "O!O!O!kkkkikik", 
                            &PyList_Type, &instancesListObj, 
                            &PyList_Type, &featuresListObj,
                            &PyList_Type, &dataListObj,
                            &maxNumberOfInstances,
                            &maxNumberOfFeatures,
                            &radius, &returnDistance, &fast,
                            &symmetric, &similarity,
                            &addressMinHashObject))
        return NULL;

    neighborhood* neighborhood_ = fitNeighborhoodComputation(addressMinHashObject, instancesListObj, featuresListObj, dataListObj, 
                                                   maxNumberOfInstances, maxNumberOfFeatures, MAX_VALUE, fast, similarity);
    return radiusNeighborhoodGraph(neighborhood_, radius, returnDistance, symmetric); 
}


static PyObject* getDistributionOfInverseIndex(PyObject* self, PyObject* args) {
    size_t addressMinHashObject;

    if (!PyArg_ParseTuple(args, "k", &addressMinHashObject))
        return NULL;

    MinHashBase* minHash = reinterpret_cast<MinHashBase* >(addressMinHashObject);

    distributionInverseIndex* distribution = minHash->getDistributionOfInverseIndex();

    return parseDistributionOfInverseIndex(distribution);
}
// definition of avaible functions for python and which function parsing fucntion in c++ should be called.
static PyMethodDef minHashFunctions[] = {
    {"fit", fit, METH_VARARGS, "Calculate the inverse index for the given instances."},
    {"partial_fit", partialFit, METH_VARARGS, "Extend the inverse index with the given instances."},
    {"kneighbors", kneighbors, METH_VARARGS, "Calculate k-nearest neighbors."},
    {"kneighbors_graph", kneighborsGraph, METH_VARARGS, "Calculate k-nearest neighbors as a graph."},
    {"radius_neighbors", radiusNeighbors, METH_VARARGS, "Calculate the neighbors inside a given radius."},
    {"radius_neighbors_graph", radiusNeighborsGraph, METH_VARARGS, "Calculate the neighbors inside a given radius as a graph."},
    {"fit_kneighbors", fitKneighbors, METH_VARARGS, "Fits and calculates k-nearest neighbors."},
    {"fit_kneighbors_graph", fitKneighborsGraph, METH_VARARGS, "Fits and calculates k-nearest neighbors as a graph."},
    {"fit_radius_neighbors", fitRadiusNeighbors, METH_VARARGS, "Fits and calculates the neighbors inside a given radius."},
    {"fit_radius_neighbors_graph", fitRadiusNeighborsGraph, METH_VARARGS, "Fits and calculates the neighbors inside a given radius as a graph."},
    {"create_object", createObject, METH_VARARGS, "Create the c++ object."},
    {"delete_object", deleteObject, METH_VARARGS, "Delete the c++ object by calling the destructor."},
    {"get_distribution_of_inverse_index", getDistributionOfInverseIndex, METH_VARARGS, "Get the distribution of the inverse index."},
    
    {NULL, NULL, 0, NULL}
};

// definition of the module for python
PyMODINIT_FUNC
init_minHash(void)
{
    (void) Py_InitModule("_minHash", minHashFunctions);
}