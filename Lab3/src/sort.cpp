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
	int depth;
};

typedef struct quicksort_params qs_param_t;

static
int
pick_pivot(int* array, int size){
	if (size < 3)
		return 0;
	int mid = size/2;
	if (array[size-1] < array[0]){
		std::swap(array[0], array[size-1]);
	}
	if (array[mid] < array[0]){
		std::swap(array[mid], array[0]);
	}
	if (array[size-1] < array[mid])
		std::swap(array[mid], array[size-1]);

	return mid;
}

static
void
insertion_sort(int* array, int count){
	int i, j, tmp;

	for (i= 0; i < count; i++){
		j = i;

		while (j > 0 && array[j] < array[j-1]){
			tmp = array[j];
			array[j] = array[j-1];
			array[j-1] = tmp;
			j--;
		}
	}
}


// A very simple quicksort implementation
// * Recursion until array size is 1
// * Bad pivot picking
// * Not in place
static
void*
simple_quicksort(void* args)
{
	int pivot, pivot_count, i;
	int *left, *right;
	size_t left_size = 0, right_size = 0;

		qs_param_t* params = (qs_param_t*)args;
	int size = params->size;
	int* array = params->array;
	pivot_count = 0;

	// This is a bad threshold. Better have a higher value
	// And use a non-recursive sort, such as insert sort
	// then tune the threshold value
	if(size > 5000)
	{
		pivot = array[pick_pivot(array, size)];

		left = (int*)malloc(size * sizeof(int));
		right = (int*)malloc(size * sizeof(int));

		// Split
		for(i = 0; i < size; i++)
		{
			if(array[i] < pivot)
			{
				left[left_size] = array[i];
				left_size++;
			}
			else if(array[i] > pivot)
			{
				right[right_size] = array[i];
				right_size++;
			}
			else
			{
				pivot_count++;
			}
		}
		qs_param_t* left_p = (qs_param_t*)malloc(sizeof(qs_param_t));
		qs_param_t* right_p =  (qs_param_t*)malloc(sizeof(qs_param_t));
		left_p->size = left_size;
		right_p->size = right_size;

		left_p->array = left;
		right_p->array = right;

		simple_quicksort(left_p);
		simple_quicksort(right_p);

		// Merge
		memcpy(array, left, left_size * sizeof(int));
		for(i = left_size; i < left_size + pivot_count; i++)
		{
			array[i] = pivot;
		}
		memcpy(array + left_size + pivot_count, right, right_size * sizeof(int));

		// Free
		free(left);
		free(right);
	}
	else
	{
		insertion_sort(array,size);
	}
}
static
void*
parallel_quicksort(void* args)
{
	int pivot, pivot_count, i;
	int *left, *right;
	size_t left_size = 0, right_size = 0;

	qs_param_t* params = (qs_param_t*)args;
	pivot_count = 0;

	int size = params->size;
	int* array = params->array;
	// This is a bad threshold. Better have a higher value
	// And use a non-recursive sort, such as insert sort
	// then tune the threshold value
	if(size > 5000)
	{
		// Bad, bad way to pick a pivot
		// Better take a sample and pick
		// it median value.
		pivot = array[pick_pivot(array, size)];

		left = (int*)malloc(size * sizeof(int));
		right = (int*)malloc(size * sizeof(int));

		// Split
		for(i = 0; i < size; i++)
		{
			if(array[i] < pivot)
			{
				left[left_size] = array[i];
				left_size++;
			}
			else if(array[i] > pivot)
			{
				right[right_size] = array[i];
				right_size++;
			}
			else
			{
				pivot_count++;
			}
		}

		qs_param_t* left_p = (qs_param_t*)malloc(sizeof(qs_param_t));
		qs_param_t* right_p =  (qs_param_t*)malloc(sizeof(qs_param_t));
		left_p->size = left_size;
		right_p->size = right_size;

		left_p->array = left;
		right_p->array = right;
		left_p->depth = params->depth + 1;
		right_p->depth = params->depth + 1;

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
		if (params->depth == 2){
			pthread_create(&thread1, NULL, parallel_quicksort, (void*)left_p);
			parallel_quicksort((void*)right_p);
		}else{
			simple_quicksort((void*)left_p);
			simple_quicksort((void*)right_p);
		}

		pthread_join( thread1, NULL);
		#endif






		// Merge
		memcpy(array, left, left_size * sizeof(int));
		for(i = left_size; i < left_size + pivot_count; i++)
		{
			array[i] = pivot;
		}
		memcpy(array + left_size + pivot_count, right, right_size * sizeof(int));

		// Free
		free(left);
		free(right);
		free(left_p);
		free(right_p);
	}
	else
	{
		insertion_sort(array, size);
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
	//cxx_sort(array, size);

	// Note: you are NOT allowed to demonstrate code that uses C or C++ standard sequential or parallel sort or merge
	// routines (qsort, std::sort, std::merge, etc). It's more interesting to learn by writing it yourself.


	qs_param_t* params =  (qs_param_t*)malloc(sizeof(qs_param_t));
	params->size = size;
	params->array = array;
	params->depth = 1;
	// Reproduce this structure here and there in your code to compile sequential or parallel versions of your code.
#if NB_THREADS < 2
	simple_quicksort(params);
	// Some sequential-specific sorting code
#else
	parallel_quicksort(params);

	// Some parallel sorting-related code
#endif // #if NB_THREADS
}
