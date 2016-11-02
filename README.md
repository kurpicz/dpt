# Distributed Patricia Trie - DPT

[![Build Status](https://travis-ci.org/kurpicz/dpt.svg?branch=master)](https://travis-ci.org/kurpicz/dpt)

## What is it?
The Distributed Patricia Trie (DPT) is a distributed full-text index implemented in C++.
A detailed description can be found [here](https://doi.org/10.4230/LIPIcs.CPM.2016.26) ([arXiv preprint](https://arxiv.org/abs/1610.03332)).

    @inproceedings{DBLP:conf/alenex/0001K017,
        author    = {Johannes Fischer and
                     Florian Kurpicz and
                     Peter Sanders},
        title     = {Engineering a Distributed Full-Text Index},
        booktitle = {Proceedings of the Ninteenth Workshop on Algorithm Engineering and
               Experiments, {ALENEX}},
        pages     = {120--134},
        year      = {2017},
        doi       = {10.1137/1.9781611974768.10},
    }

## How to get it?
First clone this repository, then build all executables.
```sh
git clone https://github.com/kurpicz/dpt.git
cd dpt
git submodule update --init
mkdir build
cd build
cmake ..
make
```

## Dependencies 
- [SDSL](https://github.com/simongog/sdsl-lite): Giving us access to a variety of succinct data structures.

When cloning this project, you will also clone [googletest](https://github.com/google/googletest).