#ifndef _PZIP_LINKEDLIST_H_
#define _PZIP_LINKEDLIST_H_

struct zipped_elem
{
	char c;
	size_t cnt;
};

struct pzip_node
{
	struct zipped_elem elem;
	struct pzip_node *prev;
	struct pzip_node *next;
};

struct pzip_node *pzip_node_add_next(struct pzip_node *node, const struct zipped_elem *elem);
struct pzip_node *pzip_node_del_and_next(struct pzip_node *node);
struct pzip_node *pzip_node_add_prev(struct pzip_node *node, const struct zipped_elem *elem);
struct pzip_node *pzip_node_del_and_prev(struct pzip_node *node);
void pzip_node_print_nodes_moving_next(struct pzip_node *node);
void pzip_node_print_nodes_moving_prev(struct pzip_node *node);


#endif // _PZIP_LINKEDLIST_H_