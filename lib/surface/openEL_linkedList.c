/*
 * openEL_linkedList.c
 *
 *  Created on: 2018/05/18
 *      Author: OpenEL-WG
 */

#include "openEL.h"

void * HalLinkedList_add__(HAL_LINKED_LIST_T *pNodeBgn,HAL_LINKED_LIST_T *pNodeAdd) {
	HAL_LINKED_LIST_T *pNodeWk;

	if ( 0 == pNodeBgn ) {
		pNodeAdd->pNext = 0;
		return pNodeAdd;
	}
	pNodeWk = pNodeBgn;
	while ( 0 != pNodeWk->pNext ) {
		pNodeWk = pNodeWk->pNext;
	}
	pNodeAdd->pNext = 0;
	pNodeWk->pNext = pNodeAdd;
	return pNodeBgn;
}

void * HalLinkedList_remove__(HAL_LINKED_LIST_T *pNodeBgn,HAL_LINKED_LIST_T *pNodeRemove) {
	HAL_LINKED_LIST_T *pNodeWk;
	HAL_LINKED_LIST_T *pNodePrev;
	HAL_LINKED_LIST_T nodeDummy;

	pNodePrev = &nodeDummy;
	pNodePrev->pNext = pNodeBgn;
	pNodeWk = pNodeBgn;
	while ( 0 != pNodeWk ) {
		if( pNodeWk == pNodeRemove ) {
			pNodePrev->pNext = pNodeWk->pNext;
			return nodeDummy.pNext;
		}
		pNodePrev = pNodeWk;
		pNodeWk = pNodeWk->pNext;
	}
	return pNodeBgn;
}

