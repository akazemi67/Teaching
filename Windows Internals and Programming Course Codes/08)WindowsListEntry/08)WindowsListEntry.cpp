#include <cstdio>
#include <cstring>

/**
 * Here I define LIST_ENTRY structure and related macros and functions
 *  for manipulating linked lists similar to what Windows has.
 *  
 * You can use source code of ReactOS for ideas, an open source operating system
 *  implementation based on the concepts in Windows XP/Server2003.
 * Definition of list macros can be found in this link:
 * https://doxygen.reactos.org/dd/df3/env__spec__w32_8h_source.html 
 */

typedef struct _LIST_ENTRY {
	struct _LIST_ENTRY* Flink;
	struct _LIST_ENTRY* Blink;
} LIST_ENTRY, *PLIST_ENTRY;

#define CONTAINING_RECORD(address, type, field) ( (type *) \
	( (char*)(address) - \
      (unsigned long)(&((type *)0)->field)) )

#define InitializeListHead(ListHead) (\
     (ListHead)->Flink = (ListHead)->Blink = (ListHead))

void RemoveEntryList(PLIST_ENTRY Entry) {
	PLIST_ENTRY prev = Entry->Blink;
	PLIST_ENTRY next = Entry->Flink;
	prev->Flink = next;
	next->Blink = prev;
}

void InsertHeadList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry) {
	PLIST_ENTRY FirstEntry = ListHead->Flink;
	Entry->Flink = FirstEntry;
	Entry->Blink = ListHead;
	ListHead->Flink = Entry;
	FirstEntry->Blink = Entry;
}

PLIST_ENTRY RemoveHeadList(PLIST_ENTRY ListHead) {
	if(ListHead->Flink == ListHead) 
		return nullptr;
	PLIST_ENTRY FirstEntry = ListHead->Flink;
	RemoveEntryList(FirstEntry);
	return FirstEntry;
}

void InsertTailList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry) {
	PLIST_ENTRY LastEntry = ListHead->Blink;
	Entry->Blink = LastEntry;
	Entry->Flink = ListHead;
	ListHead->Blink = Entry;
	LastEntry->Flink = Entry;
}

PLIST_ENTRY RemoveTailList(PLIST_ENTRY ListHead) {
	if (ListHead->Blink == ListHead)
		return nullptr;
	PLIST_ENTRY LastEntry = ListHead->Blink;
	RemoveEntryList(LastEntry);
	return LastEntry;
}

/**
 * Here I create a sample program that creates a linked list, so we can examine
 *  the construction of lists using WinDbg.
 */
PLIST_ENTRY MySampleListHead;

typedef struct _SampleData {
	int a;
	char ch1;
	char nodeId[10];
	int b;
	char ch2;
	LIST_ENTRY link;
	char str[25];
	int c;
} SampleData;

//extern "C"
SampleData *CreateNewNode(int num, char ch, const char *nId) {
	auto* data = new SampleData;
	//some random values
	data->a = num;
	data->b = num + 100;
	data->c = num + 500;
	data->ch1 = ch;
	data->ch2 = 32 + ch;
	strcpy_s<10>(data->nodeId, nId);
	strcpy_s<25>(data->str, "Here is nodeId: ");
	strcat_s<25>(data->str, nId);

	printf("%s link addr: 0x%p\n", data->nodeId, &data->link);
	return data;
}

void DeleteListItems() {
	PLIST_ENTRY entry;
	printf("\n\n****** Deleting list nodes. ******\n");
	while( (entry=RemoveHeadList(MySampleListHead))!=nullptr ) {
		SampleData* data = CONTAINING_RECORD(entry, SampleData, link);
		printf("Deleting nodeId: %s\n", data->nodeId);
		delete data;
	}
}

int main() {
	/*
	int b;
	int* a = (int*)234;
	SampleData* d = new SampleData;
	int val1 = (int) &d->link;//+0x18 == 24
	int value = (int) &((SampleData*)0)->link;
	*/
	SampleData* sd;

	MySampleListHead = new LIST_ENTRY;
	InitializeListHead(MySampleListHead);

	sd = CreateNewNode(10, 'A', "node-A");
	InsertHeadList(MySampleListHead, &sd->link);

	sd = CreateNewNode(20, 'B', "node-B");
	InsertHeadList(MySampleListHead, &sd->link);

	sd = CreateNewNode(30, 'C', "node-C");
	InsertTailList(MySampleListHead, &sd->link);

	DeleteListItems();
	delete MySampleListHead;
	return 0;
}
