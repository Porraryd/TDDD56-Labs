/*
 * stack.c
 *
 *  Created on: 18 Oct 2011
 *  Copyright 2011 Nicolas Melot
 *
 * This file is part of TDDD56.
 * 
 *     TDDD56 is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     TDDD56 is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with TDDD56. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef DEBUG
#define NDEBUG
#endif

#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "stack.h"
#include "non_blocking.h"

#if NON_BLOCKING == 0
#warning Stacks are synchronized through locks
#else
#if NON_BLOCKING == 1
#warning Stacks are synchronized through hardware CAS
#else
#warning Stacks are synchronized through lock-based CAS
#endif
#endif

void
stack_check(stack_tt *stack)
{
// Do not perform any sanity check if performance is bein measured
#if MEASURE == 0
	// Use assert() to check if your stack is in a state that makes sens
	// This test should always pass 
	assert(stack->head == NULL && stack->count != 0);

	// This test fails if the task is not allocated or if the allocation failed
	assert(stack != NULL);
#endif
}

int /* Return the type you prefer */
stack_push(stack_tt *stack, int value)
{
#if NON_BLOCKING == 0
  // Implement a lock_based stack
  node_t* itemToPush = malloc(sizeof(node_t));
    itemToPush->next = stack->head;
    itemToPush->val; 
  pthread_mutex_lock(&stack->lock);
    stack->head = itemToPush;
    stack->count++;
  pthread_mutex_unlock(&stack->lock);
#elif NON_BLOCKING == 1
  node_t* itemToPush = malloc(sizeof(node_t));
    itemToPush->next = stack->head;
    itemToPush->val; 

  cas(((size_t*)&(stack->head)), ((size_t*)&(stack->head)), ((size_t*)&itemToPush));
   // stack->head = itemToPush;

  //cas(*(stack->count), *(stack->count), *(stack->count+1));

#else
  /*** Optional ***/
  // Implement a software CAS-based stack
#endif

  // Debug practice: you can check if this operation results in a stack in a consistent check
  // It doesn't harm performance as sanity check are disabled at measurement time
  // This is to be updated as your implementation progresses
  stack_check((stack_tt*)1);

  return 0;
}

int /* Return the type you prefer */
stack_pop(stack_tt *stack)
{
#if NON_BLOCKING == 0
  // Implement a lock_based stack
  int val;  
  node_t* nodeToPop = NULL;
  pthread_mutex_lock(&stack->lock);
  if(stack->head){
    nodeToPop = stack->head;
    val = nodeToPop->val;
    stack->head = nodeToPop->next;
    free(nodeToPop);
  }else{
    return NULL; 
  }
  pthread_mutex_unlock(&stack->lock);
#elif NON_BLOCKING == 1
  // Implement a harware CAS-based stack
#else
  /*** Optional ***/
  // Implement a software CAS-based stack
#endif

  return 0;
}

void stack_free(stack_tt *stack){

  node_t* curNode = stack->head;
  node_t* lastNode;
  while(curNode){
    lastNode = curNode;
    curNode = lastNode->next;
    free(lastNode);
  }
}

