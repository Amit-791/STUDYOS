#ifndef UTILS_H
#define UTILS_H

#include <vector>
using namespace std;

//Merge Sort implementation (Sorting Algorithm ).
template <typename T, typename Compare>
void mergeSortHelper(vector<T>& arr, vector<T>& temp, int left, int right, Compare comp) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSortHelper(arr, temp, left, mid, comp);
    mergeSortHelper(arr, temp, mid + 1, right, comp); // comp(a, b)  return true if a come before b.

    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        if (comp(arr[i], arr[j])) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];
    for (int x = left; x <= right; x++) arr[x] = temp[x];
}

template <typename T, typename Compare>
void mergeSort(vector<T>& arr, Compare comp) {
    if (arr.empty()) return;
    vector<T> temp(arr.size());
    mergeSortHelper(arr, temp, 0, (int)arr.size() - 1, comp);
}

#endif