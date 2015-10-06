# Copyright 2015 Joachim Wolff
# Master Project
# Tutors: Milad Miladi, Fabrizio Costa
# Summer semester 2015
#
# Chair of Bioinformatics
# Department of Computer Science
# Faculty of Engineering
# Albert-Ludwig-University Freiburg im Breisgau
__author__ = 'joachimwolff'

import math
import logging
from multiprocessing import Manager
from _hashUtility import computeInverseIndex
from _hashUtility import computeSignature
from _hashUtility import computeNeighborhood
from _hashUtility import computePartialFit


logger = logging.getLogger(__name__)

class InverseIndex():
    """The inverse index computes and stores all hash collisions.
    Parameters
    ----------
    number_of_hash_functions : int, optional (default = 400)
        Number of hash functions to compute the minHash value. With more hash function the error rate decreases
        and the probalbility of hash collisions increases.
    max_bin_size : int, optional (default = 50)
        Number of maximal accepted hash collisions.
    number_of_nearest_neighbors : int, optional (default = 5)
        Number of nearest neighbors that should be returned.
    minimal_blocks_in_common : int, optional (default = 1)
        Number of minimal collisions two instances should have to be recognised.
    block_size : int, optional (default = 4)
        Factor to reduces the size of one signature. The size of a signature is given by:
        number_of_hash_functions / block_size. block_size have to be smaller than number_of_hash_functions
    excess_factor: int, optional (default = 5)
        Factor to increase the number of returned neighbors. Useful for the 'exact' algorithm; has no influence
        for the approximate solution.
    """
    def __init__(self, number_of_hash_functions=400,
                 max_bin_size = 50,
                 number_of_nearest_neighbors = 5,
                 minimal_blocks_in_common = 1,
                 block_size = 4,
                 excess_factor = 5, 
                 number_of_cores = -1,
                 chunk_size = 0):
        self._number_of_hash_functions = number_of_hash_functions
        self._max_bin_size = max_bin_size
        self._number_of_nearest_neighbors = number_of_nearest_neighbors
        self._minimal_blocks_in_common = minimal_blocks_in_common
        self._block_size = block_size
        if self._number_of_hash_functions < self._block_size:
            logger.error("Number of hash functions is smaller than the block size!")
        if int(math.ceil(self._number_of_hash_functions / float(self._block_size))) < 1:
            logger.error("The allowed number of hash collisions is smaller than 1. Please increase number_of_hash_functions"
                         "or decrease the block_size.")
        self._maximal_number_of_hash_collisions = int(math.ceil(self._number_of_hash_functions / float(self._block_size)))
        self._inverse_index = None
        self._signature_storage = None
        self._excess_factor = excess_factor
        self.MAX_VALUE = 2147483647
        self._number_of_cores = number_of_cores
        self._chunk_size = chunk_size
        self._index_elements = 0

    def fit(self, X, lazy_fitting=False):
        """Fits the given dataset to the minHash model.

        Parameters
        ----------
        X : csr_matrix
            The input matrix to compute the inverse index. If empty, no index will be created.
        """
        # get all ids of non-null features per instance and compute the inverse index in c++.
        self._index_elements = X.shape[0]
        instances, features = X.nonzero()
        # returns a pointer to the inverse index stored in c++
        index_storage= computeInverseIndex(self._number_of_hash_functions,
                                instances.tolist(), features.tolist(), self._block_size, self._max_bin_size,
                                                  self._number_of_cores, self._chunk_size, 1 if lazy_fitting else 0)
        self._inverse_index = index_storage[0]
        self._signature_storage = index_storage[1]

    def partial_fit(self, X):
        """Extends the inverse index build in 'fit'.

        Parameters
        ----------
        X : csr_matrix
            The input matrix to compute the inverse index. If empty, no index will not be extended.
        """
        # extend the inverse index with the given data
        instances, features = X.nonzero()
        for i in xrange(len(instances)):
            instances[i] += self._index_elements
        self._index_elements += X.shape[0]
        index_storage = computePartialFit(self._number_of_hash_functions,
                                instances.tolist(), features.tolist(), self._block_size, self._max_bin_size,
                                self._number_of_cores, self._chunk_size, self._inverse_index, self._signature_storage)
        self._inverse_index = index_storage[0]
        self._signature_storage = index_storage[1]

    def signature(self, instance_feature_list):
        """Computes the signature based on the minHash model for one instaces.

        Parameters
        ----------
        instance : csr_matrix
            The input row to compute the signature.
        """
        instances, features = instance_feature_list.nonzero()
        # compute the siganture in c++
        return computeSignature(self._number_of_hash_functions,instances.tolist(), features.tolist() ,
                                                self._block_size, self._number_of_cores, self._chunk_size, 
                                                self._signature_storage)



    def neighbors(self, instance_feature_list, size_of_neighborhood, lazy_fitting=False):
        """This function computes the neighborhood for a given instance.

        Parameters
        ----------
        signature : list
            List of hash values. Index of a value corresponds to a hash function.
        size_of_neighborhood : int
            Size of neighborhood. If less neighbors are av
        """

        # numberOfHashFunktions, instances_list, features_list, block_size,
        # number_of_cores, chunk_size, size_of_neighborhood, MAX_VALUE, minimalBlocksInCommon,
        # excessFactor, maxBinSize, inverseIndex
        # define non local variables and functions as locals to increase performance
        instances, features = instance_feature_list.nonzero()
        # compute the siganture in c++
        return computeNeighborhood(self._number_of_hash_functions,instances.tolist(), features.tolist() ,
                                                self._block_size, self._number_of_cores, self._chunk_size,
                                                self._number_of_nearest_neighbors, self.MAX_VALUE, 
                                                self._minimal_blocks_in_common, self._excess_factor,
                                                self._max_bin_size, self._maximal_number_of_hash_collisions,
                                                self._inverse_index, self._signature_storage, 1 if lazy_fitting else 0)


    def get_signature_list(self, instances):
        """Returns all signatures for the given sequences."""
        # siganture_list = [[]] * instances.shape[0]
        # for i in xrange(instances.shape[0]):
        #     siganture_list[i] = self.signature(instances[i])
        return self.signature(instances)