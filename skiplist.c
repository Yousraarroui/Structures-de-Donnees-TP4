#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "skiplist.h"
#include "rng.h"

struct s_Element
{
	int value;
	int level;
	struct s_Element** next;
	struct s_Element** previous;
};
typedef struct s_Element* Element;

struct s_SkipList
{
	RNG rng;
	int size;
	Element sentinel;
	int level;

};

Element element_create(int nblevels) {
	Element e = malloc(sizeof(struct s_Element));
	e->next = malloc(sizeof(Element) * nblevels);
	e->previous = malloc(sizeof(Element) * nblevels);
	e->level = nblevels;
	return e;
}

SkipList skiplist_create(int nblevels) {
	Element e = element_create(nblevels);
	SkipList d = malloc(sizeof(struct s_SkipList));
	d->level = nblevels;
	d->sentinel = e;
	d->size = 0;
	d->rng = rng_initialize(0);
	
	for (int i = 0; i < nblevels; i++)
	{
		e->next[i] = e;
		e->previous[i] = e;
	}
	

	return d;
}

void skiplist_delete(SkipList d) {
	Element e = d->sentinel->next[0]->next[0];
	
	for (;e!=d->sentinel;e = e->next[0])
	{
		free(e->previous[0]->next);
		free(e->previous[0]);
		free(e->previous);
	}
	free(e->previous[0]->next);
	free(e->previous[0]);
	free(e->previous);
	free(d->sentinel);
	free(d);
}

unsigned int skiplist_size(SkipList d){
	return d->size;
}

int skiplist_at(SkipList d, unsigned int i){
	assert(i < skiplist_size);
	Element e = d->sentinel->next[0];
	while (e!=d->sentinel && i>0)
	{
		e = e->next[0];
		i--;
	}
	return e->value;
}

void skiplist_map(SkipList d, ScanOperator f, void *userdata){
	Element e = d->sentinel->next[0];
	for (;e!=d->sentinel;e = e->next[0]){
		f(e->value,userdata);
	}
}

SkipList skiplist_insert(SkipList d, int value) {
	Element next = d->sentinel->next[0];

	// we search for the value right after our new element
	for (;next->value<value && next != d->sentinel;next = next->next[0]);
	if (next->value == value) return d;

	//we insert e at his position on level 0	
	Element e = element_create(rng_get_value(&(d->rng),d->level));
	e->value = value;
	e->previous[0] = next->previous[0];
	e->previous[0]->next[0] = e;
	next->previous[0] = e;
	e->next[0] = next;
	//we connect all levels to the closest elements of the right size
	for (int i = 1; i < e->level; i++)
	{
		Element current = e->next[i-1];
		//we get the next element at the ith level
		for (;i >= current->level;current = current->next[i-1]);

		e->previous[i] = current->previous[i];
		e->previous[i]->next[i] = e;	
		current->previous[i] = e;
		e->next[i] = current;	
		
	}
	d->size++;
	return d;
}

bool skiplist_search(SkipList d, int value, unsigned int *nb_operations){
	Element current = d->sentinel;
	for (int i = d->level-1; i >= 0; i--){
		while (current->next[i]->value <= value && current->next[i] != d->sentinel){
			current = current->next[i];
			++*nb_operations;
		if (current->value == value) return true;
		}
	}
	return false;
}

void print_e(int e,void* num){
	printf("%d ", e);
	(void)num;
}
struct  s_SkipListIterator{
    Element sentinel;
    SkipList d;
    Element current;
    Element (*next) (Element);
};

Element skiplist_next(Element e) {
    return e->next[0];
}

Element skiplist_previous(Element e) {
    return e->previous[0];
}


SkipListIterator skiplist_iterator_create(SkipList d, unsigned char direction) {
    SkipListIterator iterator = malloc(sizeof(struct s_SkipListIterator));
    iterator->d = d;
    iterator->sentinel = d->sentinel;
    if (direction == FORWARD_ITERATOR){
        iterator->current = *d->sentinel->next;
        iterator->next = skiplist_next;
    } else {
        iterator->current = *d->sentinel->previous;
        iterator->next = skiplist_previous;
    }
    return iterator;
}

void skiplist_iterator_delete(SkipListIterator it){
    free(it);    	
}

bool skiplist_iterator_end(SkipListIterator it){
    return it->current == it->sentinel;
}

SkipListIterator skiplist_iterator_begin(SkipListIterator it){
    it->current = it->next(it->sentinel);
    return it;
}

SkipListIterator skiplist_iterator_next(SkipListIterator it){
    it->current = it->next(it->current);
    return it;
}

int skiplist_iterator_value(SkipListIterator it){
    return it->current->value;
}