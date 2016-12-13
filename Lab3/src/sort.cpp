#include <cstdio>
#include <algorithm>

#include <string.h>

#include "sort.h"
	#include <pthread.h>

// These can be handy to debug your code through printf. Compile with CONFIG=DEBUG flags and spread debug(var)
// through your code to display values that may understand better why your code may not work. There are variants
// for strings (debug()), memory addresses (debug_addr()), integers (debug_int()) and buffer size (debug_size_t()).
// When you are done debugging, just clean your workspace (make clean) and compareile with CONFIG=RELEASE flags. When
// you demonstrate your lab, please cleanup all debug() statements you may use to faciliate the reading of your code.
#if defined DEBUG && DEBUG != 0
int *begin;
#define debug(var) printf("[%s:%s:%d] %s = \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, #var, var); fflush(NULL)
#define debug_addr(var) printf("[%s:%s:%d] %s = \"%p\"\n", __FILE__, __FUNCTION__, __LINE__, #var, var); fflush(NULL)
#define debug_int(var) printf("[%s:%s:%d] %s = \"%d\"\n", __FILE__, __FUNCTION__, __LINE__, #var, var); fflush(NULL)
#define debug_size_t(var) printf("[%s:%s:%d] %s = \"%zu\"\n", __FILE__, __FUNCTION__, __LINE__, #var, var); fflush(NULL)
#else
#define show(first, last)
#define show_ptr(first, last)
#define debug(var)
#define debug_addr(var)
#define debug_int(var)
#define debug_size_t(var)
#endif

// A C++ container class that translate int pointer
// into iterators with little constant penalty
template<typename T>
class DynArray
{
	typedef T& reference;
	typedef const T& const_reference;
	typedef T* iterator;
	typedef const T* const_iterator;
	typedef ptrdiff_t difference_type;
	typedef size_t size_type;

	public:
	DynArray(T* buffer, size_t size)
	{
		this->buffer = buffer;
		this->size = size;
	}

	iterator begin()
	{
		return buffer;
	}

	iterator end()
	{
		return buffer + size;
	}

	protected:
		T* buffer;
		size_t size;
};

static
void
cxx_sort(int *array, size_t size)
{
	DynArray<int> cppArray(array, size);
	std::sort(cppArray.begin(), cppArray.end());
}


struct quicksort_params {
	size_t size;
	int* array;
	int from;
	int to;
	int depth;
};

typedef struct quicksort_params qs_param_t;

static
void
insertion_sort(int* array, int from, int to){
	int i, j, tmp;
		
	for (i= from; i <= to; i++){
		j = i;

		while (j > 0 && array[j] < array[j-1]){
			tmp = array[j];
			array[j] = array[j-1];
			array[j-1] = tmp;
			j--;
		}
	}
}

static
int
pick_pivot(int* array, int from, int to){
	if (to-from < 3)
		return from;

	int mid = from + (to-from)/2;
	if (array[to] < array[from]){
		std::swap(array[from], array[to]);
	}
	if (array[mid] < array[from]){
		std::swap(array[mid], array[from]);
	}
	if (array[to] < array[mid])
		std::swap(array[mid], array[to]);

	return mid;
}

static
int
pick_pivot2(int* array, int from, int to){
    std::cout << array[1];

	if (to-from < 3)
		return from;

	int mid = from + (to-from)/2;
    
	//create new list
	int pivotSize = sqrtf(to-from)+1;
	int* smallList = (int*)malloc(sizeof(int) * pivotSize);

    int currI;
    while(currI < pivotSize){
        if (currI < pivotSize/3){
            smallList[currI] = array[from+currI];
        }else if(currI < 2*pivotSize/3){
            smallList[currI] = array[mid - pivotSize/3 - pivotSize/6 + currI];
        }else{
            smallList[currI] = array[to - currI + 2*pivotSize/3];
        }
        currI++;
    }

	insertion_sort(smallList, 0, currI-1);
	int pivot = smallList[currI/2];

	free(smallList);

	return pivot;
}


inline void swap(int* array, int a, int b) {
  int c = array[a];
  array[a] = array[b];
  array[b] = c;
}
static void printArray(qs_param_t* params) {
	int* array = params->array;

	for (int i=params->from; i <= params->to; i++){
		printf("%d ,", array[i]);
	}
	printf("\n");

}
static int partition(void* args)  {

  qs_param_t* params = (qs_param_t*)args;

  int* array = params->array;
  int size = params->size;
  int left, right;
  int last = params->to;
  int pivot_index = pick_pivot(array, params->from , params->to);
  int pivot = array[pivot_index];

	std::swap(array[pivot_index], array[last]);
  left = params->from;

  for(int i = params->from; i < last; i++) {

    if((i%2) ? array[i] <= pivot : array[i] < pivot)  {
			std::swap(array[i], array[left++]);
    }
  }
		std::swap(array[last], array[left]);


  return left;

}




// A very simple quicksort implementation
// * Recursion until array size is 1
// * Bad pivot picking
// * Not in place
static
void*
simple_quicksort(void* args)
{
  qs_param_t* params = (qs_param_t*)args;
	int size = params->size;
	int* array = params->array;

	if(size > 5000)
	{
		int mid = partition(args);

		// Split
		qs_param_t* left_p = (qs_param_t*)malloc(sizeof(qs_param_t));
		qs_param_t* right_p =  (qs_param_t*)malloc(sizeof(qs_param_t));
		left_p->size = mid - params->from;
		right_p->size = params->to - mid;

		left_p->array = array;
		right_p->array = array;

		left_p->from = params->from;
		left_p->to = mid-1;

		right_p->from = mid;
		right_p->to = params->to;

		simple_quicksort(left_p);
		simple_quicksort(right_p);

		// Free
		free(left_p);
		free(right_p);
	}
	else
	{
		insertion_sort(array,params->from, params->to);
	}
}
static
void*
parallel_quicksort(void* args)
{

	qs_param_t* params = (qs_param_t*)args;

	int size = params->size;
	int* array = params->array;

	if(size > 5000)
	{
		int mid = partition(args);

		// Split
		qs_param_t* left_p = (qs_param_t*)malloc(sizeof(qs_param_t));
		qs_param_t* right_p =  (qs_param_t*)malloc(sizeof(qs_param_t));

		left_p->size = mid - params->from;
		right_p->size = params->to - mid;

		left_p->array = array;
		right_p->array = array;
		left_p->depth = params->depth + 1;
		right_p->depth = params->depth + 1;

		left_p->from = params->from;
		left_p->to = mid-1;

		right_p->from = mid;
		right_p->to = params->to;

		pthread_t thread1, thread2;

		#if NB_THREADS == 2

		if (params->depth == 1){
			pthread_create(&thread1, NULL, parallel_quicksort, (void*)left_p);
			parallel_quicksort((void*)right_p);
		}else{
			simple_quicksort((void*)left_p);
			simple_quicksort((void*)right_p);
		}
		pthread_join( thread1, NULL);

		#elif NB_THREADS == 3
		if(params->depth > 2){
			simple_quicksort((void*)left_p);
			simple_quicksort((void*)right_p);
		}else{
			pthread_create(&thread1, NULL, parallel_quicksort, (void*)left_p);
			parallel_quicksort((void*)right_p);
		}

		pthread_join( thread1, NULL);

		#elif NB_THREADS == 4
		if (params->depth <= 2){
			pthread_create(&thread1, NULL, parallel_quicksort, (void*)left_p);
			parallel_quicksort((void*)right_p);
		}else{
			simple_quicksort((void*)left_p);
			simple_quicksort((void*)right_p);
		}

		pthread_join( thread1, NULL);
		#endif

		free(left_p);
		free(right_p);
	}
	else
	{
		insertion_sort(array, params->from, params->to);
	}
}


// This is used as sequential sort in the pipelined sort implementation with drake (see merge.c)
// to sort initial input data chunks before streaming merge operations.
void
sort(int* array, size_t size)
{
	// Do some sorting magic here. Just remember: if NB_THREADS == 0, then everything must be sequential
	// When this function returns, all data in array must be sorted from index 0 to size and not element
	// should be lost or duplicated.

	// Use preprocessor directives to influence the behavior of your implementation. For example NB_THREADS denotes
	// the number of threads to use and is defined at compareile time. NB_THREADS == 0 denotes a sequential version.
	// NB_THREADS == 1 is a parallel version using only one thread that can be useful to monitor the overhead
	// brought by addictional parallelization code.



	// This is to make the base skeleton to work. Replace it with your own implementation




	// Alternatively, use C++ sequential sort, just to see how fast it is
//	cxx_sort(array, size);
//	return;
	// Note: you are NOT allowed to demonstrate code that uses C or C++ standard sequential or parallel sort or merge
	// routines (qsort, std::sort, std::merge, etc). It's more interesting to learn by writing it yourself.


	qs_param_t* params =  (qs_param_t*)malloc(sizeof(qs_param_t));
	params->size = size;
	params->array = array;
	params->depth = 1;
	params->from = 0;
	params->to = size-1;
	// Reproduce this structure here and there in your code to compile sequential or parallel versions of your code.
#if NB_THREADS < 2
	simple_quicksort(params);
	// Some sequential-specific sorting code
#else
	parallel_quicksort(params);

	// Some parallel sorting-related code
#endif // #if NB_THREADS
}
