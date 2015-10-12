#!/usr/bin/python
#! /usr/bin/python
# Copyright 2015 Joachim Wolff
# Master Project
# Tutors: Milad Miladi, Fabrizio Costa
# Summer semester 2015
#
# Chair of Bioinformatics
# Department of Computer Science
# Faculty of Engineering
# Albert-Ludwig-University Freiburg im Breisgau
__author__ = "Joachim Wolff"
__contact__ = "wolffj@informatik.uni-freiburg.de"
__copyright__ = "Copyright 2015, Joachim Wolff"
__credits__ = ["Milad Miladi", "Fabrizio Costa"]
__license__ = "No License"
__date__ = "05.08.2015"
__version_ = "0.1.dev"

from setuptools import setup, find_packages
from distutils.core import Extension
import platform
import sys

if "--openmp" in sys.argv:
    module1 = Extension('_hashUtility', sources = ['neighborsMinHash/src_c/hashUtility.cpp', 'neighborsMinHash/src_c/minHash.cpp' ], 
        depends=['neighborsMinHash/src_c/minHash.h'], define_macros=[('OPENMP', None)], extra_link_args = ["-lm", "-lrt","-lgomp"], 
        extra_compile_args=["-fopenmp", "-g", "-std=c++11"])

elif platform.system() == 'Darwin' or "--noopenmp" in sys.argv:
    module1 = Extension('_hashUtility', sources = ['neighborsMinHash/src_c/hashUtility.cpp', 'neighborsMinHash/src_c/minHash.cpp' ], 
        depends=['neighborsMinHash/src_c/minHash.h'], extra_compile_args=["-g", "-std=c++11"])

else:
    module1 = Extension('_hashUtility', sources = ['neighborsMinHash/src_c/hashUtility.cpp', 'neighborsMinHash/src_c/minHash.cpp' ], 
        depends=['neighborsMinHash/src_c/minHash.h'],  define_macros=[('OPENMP', None)], extra_link_args = ["-lm", "-lrt","-lgomp"],
         extra_compile_args=["-fopenmp", "-g", "-std=c++11"])

if "--openmp" in sys.argv:
    sys.argv.remove("--openmp")
if "--noopenmp" in sys.argv:
    sys.argv.remove("--noopenmp")


setup (name = 'neighborsMinHash',
        author = 'Joachim Wolff',
        author_email = 'wolffj@informatik.uni-freiburg.de',
        url='https://github.com/joachimwolff/minHashNearestNeighbors',
        license='LICENSE',
        description='An approximate computation of nearest neighbors based on locality sensitive hash functions.',
        long_description=open('README.md').read(),
        install_requires=[
        "numpy >= 1.8.0",
        "scipy >= 0.14.0",
        "scikit-learn >= 0.16.0",],
        ext_modules = [module1],
        packages=['neighborsMinHash',
                    'neighborsMinHash.neighbors',
                    'neighborsMinHash.computation',
		    'neighborsMinHash.util',
            'neighborsMinHash.clustering',
                ],
        platforms = "Linux, Mac OS X",
        version = '0.1.dev'
        )
