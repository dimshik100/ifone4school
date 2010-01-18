#pragma once
#include "Types.h"
#include "DynamicListC.h"

// For shorter and clearer code.
#define ItemSize (long)sizeof(Contact)
#define DbItemSize (long)sizeof(DbInfo)
// Calculate an item's offset in the database file.
#define getOffset(num_of_items) ((long)(DbItemSize + (ItemSize * (num_of_items))))

int createAccount(TCHAR* user, TCHAR* pass);
int getAccountCredentials(TCHAR* user, TCHAR* pass);
int addContact(Contact* contact);
DynamicListC getContactList();
DynamicListC getTrashList();
int emptyTrashList();
int search(Contact* query, DynamicListC* pList); // returns number of items found.
int deleteContact(int contactIndex); // deletes the contact by index to trash
int recoverContact(int contactIndex); // recovers the contact by index from trash to contact list
int editContact(Contact* contact); 
int checkDataIntegrity();
int getMemoryInfo(int *count, int *total, DbType source);
void dupContact(Contact *dst, Contact *src);

// Extension functions for easier/faster management of the contact list
int			 addContactToLocalList(Contact *contact);
void		 freeContactListLocal();
DynamicListC getContactListLocal();
DynamicListC getContactListFromFile();
DynamicListC getContactListInitiated();