#! /usr/bin/python
from bioinf_learn.neighbors import MinHash
from bioinf_learn.util import neighborhood_accuracy
import matplotlib.pyplot as plt
import numpy as np
from sklearn.neighbors import NearestNeighbors
from sklearn.metrics import accuracy_score
import time
import math
from collections import Counter
import gc
from hyperopt import fmin, tpe, hp, STATUS_OK, Trials
import os.path
import cPickle as pickle

def compute_score(error, memory, time, max_memory, max_time, alpha, beta):
    print "error1: ", error
    # if error > 0.1:
    #     return 5
    # if error > 0.15:
    #     return 10
    # if error > 0.2:
    #     return 20
    # if error > 0.25:
    #     return 30
    # if error > 0.3:
    #     return 40
    if memory == 0:
        return 10
    if time == 0:
        return 10
    # error = error/(float (max_error))
    # memory = math.log(memory/float(max_memory), 10)*alpha
    # time = math.log(time/float(max_time), 10)*beta
    memory = math.log(memory, 10)*alpha
    time = math.log(time, 10)*beta
    print "memory: ", memory
    print "time: ", time
    print "score: ", error + memory  + time
    return error #+ memory  + time

 

# define an objective function
def objective(args):
    number_of_hash_functions = args["number_of_hash_functions"]
    max_bin_size = args["max_bin_size"]
    shingle_size =args["shingle_size"]
    excess_factor =args["excess_factor"]
    # chunk_size = args["chunk_size"]
    prune_inverse_index =args["prune_inverse_index"]
    prune_inverse_index_after_instance =args["prune_inverse_index_after_instance"]
    removeHashFunctionWithLessEntriesAs = args["removeHashFunctionWithLessEntriesAs"]
    block_size =args["block_size"]
    shingle = args["shingle"]
    remove_value_with_least_sigificant_bit =args["remove_value_with_least_sigificant_bit"]
    minimal_blocks_in_common = args['minimal_blocks_in_common']
    # try:
    print "\n\n\n"        
    print "Values: "
    print "number_of_hash_functions ", number_of_hash_functions
    print "max_bin_size ", max_bin_size
    print "shingle_size ", shingle_size
    print "excess_factor ", excess_factor
    # print "chunk_size ",chunk_size
    print "prune_inverse_index ", prune_inverse_index
    print "prune_inverse_index_after_instance ", prune_inverse_index_after_instance
    print "removeHashFunctionWithLessEntriesAs ", removeHashFunctionWithLessEntriesAs
    print "block_size ", block_size
    print "shingle ", shingle
    print "remove_value_with_least_sigificant_bit ", remove_value_with_least_sigificant_bit
    print "\n\n\n"        
    
    print "Create minHash object"
    minHash = MinHash(n_neighbors=10, radius=1.0, fast=False, number_of_hash_functions=int(number_of_hash_functions),
                max_bin_size = int(max_bin_size), minimal_blocks_in_common = int(minimal_blocks_in_common),
                shingle_size = int(shingle_size),
                excess_factor = int(excess_factor),
                similarity=False, number_of_cores=4, 
                prune_inverse_index=int(prune_inverse_index),
                prune_inverse_index_after_instance=prune_inverse_index_after_instance,
                remove_hash_function_with_less_entries_as=int(removeHashFunctionWithLessEntriesAs), 
                 block_size = int(block_size), shingle=shingle,
                store_value_with_least_sigificant_bit=remove_value_with_least_sigificant_bit, 
                cpu_gpu_load_balancing=0)
    print "Create minHash object...Done"
                
    # print "Values: "
    # print "number_of_hash_functions ", number_of_hash_functions
    # print "max_bin_size ", max_bin_size
    # print "shingle_size ", shingle_size
    # print "excess_factor ", excess_factor
    # # print "chunk_size ",chunk_size
    # print "prune_inverse_index ", prune_inverse_index
    # print "prune_inverse_index_after_instance ", prune_inverse_index_after_instance
    # print "removeHashFunctionWithLessEntriesAs ", removeHashFunctionWithLessEntriesAs
    # print "block_size ", block_size
    # print "shingle ", shingle
    # print "remove_value_with_least_sigificant_bit ", remove_value_with_least_sigificant_bit
    print "fitting..."
    minHash.fit(datasetBursi)
    print "fitting...Done"
    print "Get Distribution..."
    distribution = minHash.get_distribution_of_inverse_index()
    print "Get Distribution...Done"
    memory = 0
    for j in distribution[0]:
        memory += j * distribution[0][j]
    
    
    print "Compute k-neihgbors..."    
    time_start = time.time()
    neighbors = minHash.kneighbors(n_neighbors = 10, return_distance=False)
    time_end = time.time() - time_start
    print neighbors
    print "Compute k-neihgbors...Done"    
    
    # accuracy = 0.0
    # for x, y in zip(neighbors, neighbors_sklearn):
    #     accuracy += accuracy_score(x, y)
    # accuracy = accuracy / float(len(neighbors))
    error = 1 - neighborhood_accuracy(neighbors, neighbors_sklearn)
    # print number_of_hash_functions
    # except:
    # print "Exception!" 
    # print "error: ", error-0.001
    
        # return 5
    # alpha = args["alpha"]
    # beta = args["beta"]
    return compute_score(error, memory, time_end, max_memory, max_time, 0.1, 0.23)
    # return error-0.001


# get data set  
if not os.path.isfile("bursiDataset"):
    graphs = gspan_to_eden( 'http://www.bioinf.uni-freiburg.de/~costa/bursi.gspan' )
    vectorizer = Vectorizer( r=2,d=5 )
    datasetBursi = vectorizer.transform( graphs )
    fileObject = open("bursiDataset",'wb')
    pickle.dump(datasetBursi,fileObject)
    fileObject.close()
else:
    fileObject = open("bursiDataset",'r')
    datasetBursi = pickle.load(fileObject)

# get values with out
minHash_org = MinHash(n_neighbors=10, radius=1.0, fast=False, number_of_hash_functions=800,
                 max_bin_size = 50, minimal_blocks_in_common = 1, shingle_size = 1, excess_factor = 1,
                 similarity=False, number_of_cores=4, chunk_size=1, 
                 prune_inverse_index=-1,
                 prune_inverse_index_after_instance=-1,
                 remove_hash_function_with_less_entries_as=-1, 
                 block_size = 1, shingle=0,
                 store_value_with_least_sigificant_bit=0, cpu_gpu_load_balancing=0)
minHash_org.fit(datasetBursi)
distribution = minHash_org.get_distribution_of_inverse_index()
# print distribution

max_memory = 0
for j in distribution[0]:
    max_memory += j * distribution[0][j]
time_start = time.time()
neighbors_org = minHash_org.kneighbors(return_distance=False)
max_time = time.time() - time_start

nearest_Neighbors = NearestNeighbors(n_jobs=4)
nearest_Neighbors.fit(datasetBursi)
neighbors_sklearn = nearest_Neighbors.kneighbors(n_neighbors=10, return_distance=False)

max_error = 0.0
# for x, y in zip(neighbors_org, neighbors_sklearn):
#     max_error += accuracy_score(x, y)
# max_error = max_error / float(len(neighbors_org))
max_error = 1 - neighborhood_accuracy(neighbors_org, neighbors_sklearn)
print "Max error: ", max_error
print "Max time: ", max_time
print "Max memory: ", max_memory
# define a search space
from hyperopt import hp
space = {
        'number_of_hash_functions': hp.uniform('number_of_hash_functions', 50, 1000),
        'max_bin_size': hp.uniform('max_bin_size', 10, 100),
        'shingle_size': hp.choice('shingle_size', [1, 2, 3, 4, 5]),
        'excess_factor': hp.uniform('excess_factor', 1, 15),
        # 'chunk_size': hp.uniform('chunk_size', 1, 20),
        'prune_inverse_index': hp.uniform('prune_inverse_index',-1, 100),
        'prune_inverse_index_after_instance':  hp.uniform('prune_inverse_index_after_instance',0.0, 1),
        'removeHashFunctionWithLessEntriesAs': hp.uniform('removeHashFunctionWithLessEntriesAs', -1, 1000),
        'block_size': hp.choice('block_size', [1, 2, 3, 4, 5]), 
        'shingle': hp.choice('shingle', [0,1]), 
        'remove_value_with_least_sigificant_bit': hp.choice('remove_value_with_least_sigificant_bit', [1, 2, 3, 4, 5]),
        'minimal_blocks_in_common': hp.uniform('minimal_blocks_in_common', 1, 15),
        # 'alpha':hp.uniform('alpha', 0, 1),
        # 'beta':hp.uniform('beta', 0,1),    
}

trials = Trials()
# minimize the objective over the space
from hyperopt import fmin, tpe
best = fmin(objective, space, algo=tpe.suggest, max_evals=100, trials=trials)

print best
print trials.results
# -> {'a': 1, 'c2': 0.01420615366247227}
# print hyperopt.space_eval(space, best)
# -> ('case 2', 0.01420615366247227}