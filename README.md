# NCSK-D-Tree
An implementation of cache sensitive N-arry K-D Tree(NCSK-D Tree for short) for cs227
-------------------
It features several new aspects
* The tree does not need to be binary. Each node has arbitrary number of children.
* It features `cache sensitive` implementation. The idea of `cache sensitive` is brought from this paper[Making B+Trees Cache Conscious in Main Memory](https://www.cs.duke.edu/courses/spring03/cps216/papers/rao-ross-2000.pdf). To avoid penalized by resolving cache misses during pointer chasing in the tree, each parent maintains only one pointer pointing to a contiguous memory within which all its children nodes are packed together.

