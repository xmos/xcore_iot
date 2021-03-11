// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.


#ifndef HEAPSORT_H_
#define HEAPSORT_H_

/**
 *
 *
 * \param root
 */
#define left_child_index(root) (2 * (root) + 1)

/**
 *
 * \param left
 */
#define right_sibling_index(left) ((left) + 1)

/**
 *
 * \param data
 * \param first
 * \param last
 * \param compare
 * \param swap
 */
#define sift_down(data, first, last, compare, swap) \
do { \
    int root, child; \
\
    for (root = first; left_child_index(root) <= last; root = child) { \
\
        child = left_child_index(root); \
\
        if (right_sibling_index(child) <= last && compare(data, child, right_sibling_index(child)) < 0) { \
            child = right_sibling_index(child); \
        } \
\
        if (compare(data, root, child) < 0) { \
            swap(data, root, child); \
        } else { \
            break; \
        } \
    } \
} while(0)


/**
 *
 * \param data
 * \param first
 * \param last
 * \param compare
 * \param swap
 */
#define heapify(data, first, last, compare, swap) \
do { \
    int _i; \
\
    for (_i = first; _i >= 0; _i--) { \
        sift_down(data, _i, last, compare, swap); \
    } \
} while (0)

/**
 *
 * \param data
 * \param length
 * \param compare
 * \param swap
 */
#define heapsort(data, length, compare, swap) \
do { \
    int last = (length) - 1; \
\
    heapify(data, ((length)/2 - 1), last, compare, swap); \
\
    while (last > 0) { \
        swap(data, last, 0); \
        last--; \
        sift_down(data, 0, last, compare, swap); \
    } \
} while (0)

#endif /* HEAPSORT_H_ */
