/*
 * openEL_linkedList.h
 *
 *  Created on: 2018/05/18
 *      Author: OpenEL-WG
 */

#ifndef OPENEL_LINKEDLIST_H_
#define OPENEL_LINKEDLIST_H_

#include <stdint.h>

#define HalLinkedList_add( pBgn,pAdd) HalLinkedList_add__( &((pBgn)->linkedList),&((pAdd)->linkedList)  )
void * HalLinkedList_add__(HAL_LINKED_LIST_T *pNodeBgn,HAL_LINKED_LIST_T *pNodeAdd);

#define HalLinkedList_remove( pBgn,pRm ) HalLinkedList_remove__( &((pBgn)->linkedList),&((pRm)->linkedList)  )
void * HalLinkedList_remove__(HAL_LINKED_LIST_T *pNodeBgn,HAL_LINKED_LIST_T *pNodeRemove);

#define HalLinkedList_getNext( pArg ) (void *)((pArg)->linkedList.pNext)

#endif /* OPENEL_LINKEDLIST_H_*/
