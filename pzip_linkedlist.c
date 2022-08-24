#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pzip_linkedlist.h"

// #define PRINTF_LINKEDLIST_ENABLED
#ifndef PRINTF_LINKEDLIST_ENABLED
#define PRINTF_LINKEDLIST(...)
#else
#define PRINTF_LINKEDLIST fprintf
#endif

struct pzip_node *pzip_node_add_next(struct pzip_node *node, const struct zipped_elem *elem)
{
	if (elem == NULL)
		return NULL;

	struct pzip_node *new_node = (struct pzip_node *) malloc(sizeof(struct pzip_node));

	if (new_node == NULL)
		return NULL;

	memcpy(&new_node->elem, elem, sizeof(*elem));

	if (node == NULL)
	{
		new_node->next = new_node->prev = NULL;
		return new_node;
	}

	new_node->next = node->next;
	new_node->prev = node;
	if (node->next)
		node->next->prev = new_node;
	node->next = new_node;

	return new_node;
}

struct pzip_node *pzip_node_del_and_next(struct pzip_node *node)
{
	if (node)
	{
		struct pzip_node *ret = node->next;

		if (node->next)
			node->next->prev = node->prev;
		if (node->prev)
			node->prev->next = node->next;

		free(node);
		return ret;
	}

	return NULL;
}

struct pzip_node *pzip_node_add_prev(struct pzip_node *node, const struct zipped_elem *elem)
{
	if (elem == NULL)
		return NULL;

	struct pzip_node *new_node = (struct pzip_node *) malloc(sizeof(struct pzip_node));

	if (new_node == NULL)
		return NULL;

	memcpy(&new_node->elem, elem, sizeof(*elem));

	new_node->prev = node->prev;
	new_node->next = node;
	if (node->prev)
		node->prev->next = new_node;
	node->prev = new_node;

	return new_node;
}

struct pzip_node *pzip_node_del_and_prev(struct pzip_node *node)
{
	if (node)
	{
		struct pzip_node *ret = node->prev;

		if (node->next)
			node->next->prev = node->prev;
		if (node->prev)
			node->prev->next = node->next;

		free(node);
		return ret;
	}

	return NULL;
}

struct pzip_node *pzip_node_join_nodes(struct pzip_node *node_no_next, struct pzip_node *node_no_prev)
{
	if (node_no_next == NULL || node_no_next->next != NULL)
		return NULL;

	if (node_no_prev == NULL || node_no_prev->prev != NULL)
		return NULL;

	if (node_no_next->elem.c == node_no_prev->elem.c)
	{
		node_no_next->elem.cnt += node_no_prev->elem.cnt;
		node_no_prev = pzip_node_del_and_next(node_no_prev);
	}

	node_no_next->next = node_no_prev;

	if (node_no_prev)
		node_no_prev->prev = node_no_next;

	return node_no_next;
}


// void test_pzip_node_join_nodes(int test_different_nodes, int test_2plus_linked_list_joining)
// {
//     struct zipped_elem elem[] = {{'a', 1}, {'b', 2}, {'c', 3}, {'c', 6}, {'d', 4}};
//     struct pzip_node *head1 = NULL;
//     struct pzip_node *tail1 = NULL;
//     struct pzip_node *head2 = NULL;
//     struct pzip_node *tail2 = NULL;

//     printf("\nTest different nodes: %s, Test 2+ linked list joining: %s\n\n",
//     		(test_different_nodes)?("True"):("False"),
//     		(test_2plus_linked_list_joining)?("True"):("False"));

//     head1 = tail1 = pzip_node_add_next(NULL, &elem[0]);
//     head1 = pzip_node_add_next(head1, &elem[1]);
//     head1 = pzip_node_add_next(head1, &elem[2]);
//     // tail -> ('a', 1) <--> ('b', 2) <--> ('c', 3) <- head

//     printf("\nlinked list 1 tail to head\n");
//     pzip_node_print_nodes_moving_next(tail1);
//     printf("\nlinked list 1 head to tail\n");
//     pzip_node_print_nodes_moving_prev(head1);
//     printf("\n");

//     head2 = tail2 = pzip_node_add_next(head2, &elem[3]);
// 	// tail -> ('d', 4) <- head

//     // false / true for same / differenet node(s) respectively
//     if (test_different_nodes) head2->elem.c = 'z'; // will change to: tail -> ('z', 6) <- head

//     // disable / enable for 1 / 2+ linked list nodes(s) respectively
//     if (test_2plus_linked_list_joining) head2 = pzip_node_add_next(head2, &elem[4]); // will change to: tail -> ('c/z', 6) <--> ('d', 4) <- head

//     printf("\nlinked list 2 tail to head\n");
//     pzip_node_print_nodes_moving_next(tail2);
//     printf("\nlinked list 2 head to tail\n");
//     pzip_node_print_nodes_moving_prev(head2);
//     printf("\n");

//     /*
//     ** After joining:
// 	** 1. test_different_nodes f test_2plus_linked_list_joining f: concat with same elem 1 node
// 	** 2. test_different_nodes f test_2plus_linked_list_joining t: concat with same elem 2+ node
// 	** 3. test_different_nodes t test_2plus_linked_list_joining f: concat with different elem 1 node
// 	** 4. test_different_nodes t test_2plus_linked_list_joining t: concat with different elem 2+ node (should be the same as 3)
// 	*/

// 	head1 = pzip_node_join_nodes(head1, tail2);
// 	printf("\nlinked list 1 tail to head after joining...\n");
// 	pzip_node_print_nodes_moving_next(tail1);

// 	// if concated with same elem and following linked-list had 1 node
// 	if ( !test_different_nodes && !test_2plus_linked_list_joining )
// 	{
// 		printf("\nNo reason to print 2nd linked list, it is completely deleted\n\n");
// 		printf("head1 %p head1 elem {%c, %d}\n", head1, head1->elem.c, head1->elem.cnt);
// 		//pzip_node_print_nodes_moving_prev(head1); // TODO: There is a bug here
// 		return;
// 	}

//     printf("\nlinked list 1 head to tail after joining...\n");
//     pzip_node_print_nodes_moving_prev(head2);
//     printf("\n");

//     printf("\n");
// }



void pzip_node_print_nodes_moving_next(struct pzip_node *node)
{
	PRINTF_LINKEDLIST(stderr, "\t");
	while(node)
	{
		PRINTF_LINKEDLIST(stderr, "('%c', %ld) ", node->elem.c, node->elem.cnt);
		node = node->next;
	}
}

void pzip_node_print_nodes_moving_prev(struct pzip_node *node)
{
	PRINTF_LINKEDLIST(stderr, "\t");
	while(node)
	{
		PRINTF_LINKEDLIST(stderr, "('%c', %ld) ", node->elem.c, node->elem.cnt);
		node = node->prev;
	}
}
