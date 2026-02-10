#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

struct ListNode{
	int x;
	int y;
	struct ListNode* next;
};

struct ListNode* listHead;

int addNode(int x, int y){
	struct ListNode* current = listHead;
	while(current->next != NULL){
		current = current->next;
	}
	struct ListNode* new = (struct ListNode *)malloc(sizeof(struct ListNode));
	new->x = x;
	new->y = y;
	new->next = NULL;
	current->next = new;
	return 1;
}


printList(){
	struct ListNode* current = listHead;
	do{
		printf("%d, %d\n", current->x, current->y);
		current = current->next;
	}while(current->next != NULL);
	printf("%d, %d\n", current->x, current->y);
}

freeAll(){
	struct ListNode* current = listHead;
	do{
		struct ListNode* temp = current;
		int point = (int)temp;
		current = current->next;
		free(temp);
		printf("freed node %d\n", point);
	}while(current->next != NULL);	
	free(current);
}

struct ListNode* getLast(struct ListNode* L){
	while(L->next != NULL){
		L = L->next;
	}
	return L;
}

int remove(int x, int y){
	struct ListNode* prev = listHead;
	struct ListNode* current = listHead->next;
	int found = 0;
	while(current->next != NULL){
		if(current->x == x && current->y == y){
			prev->next = current->next;
			current->next = NULL;
			found = 1;
		}else{
			prev = prev->next;
			current = current->next;
		}
	}
	if(found == 1){
		free(current);
	}
	return found;
}

int main(){
	listHead = (struct ListNode*)malloc(sizeof(struct ListNode));
	listHead->next = NULL;
	listHead->x = 1;
	listHead->y = 1;
	addNode(3, 2);
	if(addNode(1, 2) == 1){
		printList();	
	}else{
		printf("list addition failed\n");
	}
	if(remove(3, 2) == 1){
		printf("Removed node 3, 2\n");
		printList();
	}else{
		printf("No node with values 3, 2 found\n");
	}
	freeAll();
	printf("Hello World\n");
	return 1;
}