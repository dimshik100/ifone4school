#include "stdafx.h"
#include "PhoneBook.h"
#include <limits.h>
#include <windows.h>

//Disable the deprecated function warnings
#pragma warning(disable : 4996)

#define Db_Name TEXT("pb.dat")
#define Trash_Name TEXT("trash.dat")
#define Cfg_Name TEXT("cfg.dat")

// Help functions
int addContactActual(Contact* contact, TCHAR *dbName);
DynamicListC transferDatabaseToList(TCHAR *dbName);
int deleteContactActual (int contactIndex, TCHAR *dbName, TCHAR *TrashName);
int searchDb (Contact* query, DynamicListC* pList, TCHAR *dbName);
int getMemoryInfoActual(int *count, int *total, TCHAR *dbName);
long getFileLen(FILE *file);
void encryptDecryptCredentials(TCHAR *dst, TCHAR *src, int Len);
DynamicListC *getCurrentContactList(int fromFile);

int createAccount (TCHAR* user, TCHAR* pass)
{
	FILE *stream;
	DbInfo dbInfo;
	TCHAR username[MAX_USERNAME], password[MAX_PASSWORD];
	int ret = 1;

	// Save username and password to config file.
	stream = _tfopen(Cfg_Name, TEXT("wb"));
	if (stream)
	{
		encryptDecryptCredentials(username, user, MAX_USERNAME);
		encryptDecryptCredentials(password, pass, MAX_PASSWORD);		
		fwrite(username, sizeof(TCHAR)*MAX_USERNAME, 1, stream);
		fwrite(password, sizeof(TCHAR)*MAX_PASSWORD, 1, stream);
		fclose(stream);
	}
	else
		ret = 0;

	// Initiate our database; create a new file and add a header.
	stream = _tfopen(Db_Name, TEXT("wb"));
	if (stream)
	{
		dbInfo.itemCount = 0;
		dbInfo.NumberOfGroups = 0;
		fwrite(&dbInfo, DbItemSize, 1, stream);
		fclose(stream);
	}
	else
		ret = 0;

	// Initiate our trash; create a new file and add a header.
	stream = _tfopen(Trash_Name, TEXT("wb"));
	if (stream)
	{
		dbInfo.itemCount = 0;
		dbInfo.NumberOfGroups = 0;
		fwrite(&dbInfo, DbItemSize, 1, stream);
		fclose(stream);
	}
	else
		ret = 0;

	return ret; 
}

int getAccountCredentials(TCHAR* user, TCHAR* pass)
{
	FILE *stream;
	TCHAR username[MAX_USERNAME], password[MAX_PASSWORD];
	int ret = 1;

	// Read username and password from config file.
	stream = _tfopen(Cfg_Name, TEXT("rb"));
	if (stream)
	{
		// Make sure we record success or failure from both read operations.
		ret = (int)fread(username, sizeof(TCHAR)*MAX_USERNAME, 1, stream);
		ret &= (int)fread(password, sizeof(TCHAR)*MAX_PASSWORD, 1, stream); // ret = ret & (int)fread...
		// Decrypt password
		encryptDecryptCredentials(user, username, MAX_USERNAME);
		encryptDecryptCredentials(pass, password, MAX_PASSWORD);		
		fclose(stream);
	}
	else
		ret = 0;

	return ret;
}

int addContact(Contact* contact)
{
	return addContactActual(contact, Db_Name);
}

int addContactActual(Contact* contact, TCHAR *dbName)
{
	FILE *stream;
	DbInfo dbInfo;
	int ret = 0;

	// We may receive a NULL from other functions, so make sure we don't cause an error.
	if (!dbName)
		return 0;

	// Open file for read/write in binary
	stream = _tfopen(dbName, TEXT("r+b"));
	// Make sure file is open and contact actually exists.
	if (stream && contact)
	{
		// Load database header.
		fread(&dbInfo, DbItemSize, 1, stream);

		// Make sure we still have room in the database to fit more items
		if (dbInfo.itemCount < MAX_ITEMCOUNT)
		{		
			contact->index = dbInfo.itemCount;
			// Write new item to end of file.
			fseek(stream, 0, SEEK_END);
			// Did function succeed?
			ret = (int)fwrite(contact, ItemSize, 1, stream);
			// Update database header
			if (ret)
			{
				dbInfo.itemCount++;
				fseek(stream, 0, SEEK_SET);
				fwrite(&dbInfo, DbItemSize, 1, stream);
			}
		}
		// Close file.
		fclose(stream);
	}

	return ret;
}

DynamicListC getContactList()
{
	return transferDatabaseToList(Db_Name);
}

DynamicListC getTrashList()
{
	return transferDatabaseToList(Trash_Name);
}

DynamicListC transferDatabaseToList(TCHAR *dbName)
{
	DynamicListC list = NULL;
	DbInfo dbInfo;
	Contact item, *newItem;
	FILE *stream;

	stream = _tfopen(dbName, TEXT("rb"));
	if (stream)
	{
		// Load database header.
		if (fread(&dbInfo, DbItemSize, 1, stream))
		{
			listInit(&list);
			// Find the contact we're deleting in the file.
			while(fread(&item, ItemSize, 1, stream))
			{
				newItem = (Contact*)malloc(sizeof(Contact));
				dupContact(newItem, &item);
				listInsertAfterEnd(list, &newItem);
			}
		}
		fclose(stream);
	}

	return list;
}

int emptyTrashList()
{
	FILE *stream;
	DbInfo dbInfo;
	int ret = 0;

	// Initiate our trash; create a new file and add a header.
	stream = _tfopen(Trash_Name, TEXT("wb"));
	if (stream)
	{
		dbInfo.itemCount = 0;
		dbInfo.NumberOfGroups = 0;
		ret = (int)fwrite(&dbInfo, DbItemSize, 1, stream);
		fclose(stream);
	}

	return ret;
}

int search(Contact* query, DynamicListC* pList)
{
	return searchDb(query, pList, Db_Name);
}

int searchDb (Contact* query, DynamicListC* pList, TCHAR *dbName)
{
	int flag, listCount = 0;
	FILE *stream;
	DbInfo dbInfo;
	Contact item, *newItem;

	// Set list as NULL in case function fails.
	*pList = NULL;
	stream = _tfopen(dbName, TEXT("rb"));
	if (stream)
	{
		listInit(pList);
		// Load database header.
		fread(&dbInfo, DbItemSize, 1, stream);
		// Find the contact we're deleting in the file.
		while(fread(&item, ItemSize, 1, stream))
		{
			// Reset search flag.
			flag = 0;

			if (_tcslen(query->firstName) > 0)
			{
				if (!_tcsicmp(query->firstName, item.firstName))
					flag = 1;
				else
					continue;
			}
			if (_tcslen(query->lastName) > 0)
			{
				if (!_tcsicmp(query->lastName, item.lastName))
					flag = 1;
				else
					continue;
			}
			if (_tcslen(query->phone) > 0)
			{
				if (!_tcsicmp(query->phone, item.phone))
					flag = 1;
				else
					continue;
			}

			if (flag)
			{
				newItem = (Contact*)malloc(sizeof(Contact));
				dupContact(newItem, &item);
				if (listInsertAfterEnd(*pList, &newItem))
					listCount++;
			}
		}
		// Close file.
		fclose(stream);
	}

	return listCount; 
}

int deleteContact (int contactIndex)
{
	return deleteContactActual(contactIndex, Db_Name, Trash_Name);
}

int deleteContactActual (int contactIndex, TCHAR *dbName, TCHAR *backupName)
{
	int ret = 0;
	Contact item;
	DbInfo dbInfo;
	FILE *stream;
	HANDLE hFile;

	// Make sure that the contact actually exists, if yes, try to open the file.
	stream = _tfopen(dbName, TEXT("r+b"));
	if (stream)
	{
		// Backup deleted item in trash
		fseek(stream, getOffset(contactIndex), SEEK_SET);
		fread(&item, ItemSize, 1, stream);
		addContactActual(&item, backupName);
		// Load database header.
		fseek(stream, 0, SEEK_SET);
		fread(&dbInfo, DbItemSize, 1, stream);
		// Read last item
		fseek(stream, -ItemSize, SEEK_END);
		fread(&item, ItemSize, 1, stream);
		// To keep list sorted by index, last item gets the index of the deleted item
		item.index = contactIndex;
		// Overwrite the deleted item with the updated last item.
		fseek(stream, getOffset(contactIndex), SEEK_SET);
		ret = (int)fwrite(&item, ItemSize, 1, stream);
		if (ret)
		{
			dbInfo.itemCount--;
			fseek(stream, 0, SEEK_SET);
			fwrite(&dbInfo, DbItemSize, 1, stream);
		}
		fclose(stream);
		
		// Cut end of file.
		hFile = CreateFile(dbName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (ret && hFile != INVALID_HANDLE_VALUE)
		{
			SetFilePointer(hFile, -ItemSize, 0, FILE_END);
			SetEndOfFile(hFile);
		}
		CloseHandle(hFile);
	}

	return ret; 
}

int recoverContact (int contactIndex) // recovers the contact by index from trash to contact list
{
	int ret = 0;
	Contact item;
	FILE *stream;

	// Open file for read/write in binary
	stream = _tfopen(Trash_Name, TEXT("r+b"));
	// Make sure file is open and contact actually exists.
	if (stream)
	{
		// Go to beginning of contact item.
		fseek(stream, getOffset(contactIndex), SEEK_SET);
		fread(&item, ItemSize, 1, stream);
		fclose(stream);
		if (addContactActual(&item, Db_Name))
			ret = deleteContactActual(contactIndex, Trash_Name, NULL);
	}

	return ret;
}

int editContact (Contact* contact) 
{
	int ret = 0;
	FILE *stream;

	// Open file for read/write in binary
	stream = _tfopen(Db_Name, TEXT("r+b"));
	// Make sure file is open and contact actually exists.
	if (stream && contact)
	{
		// Go to beginning of contact item.
		fseek(stream, getOffset(contact->index), SEEK_SET);
		ret = (int)fwrite(contact, ItemSize, 1, stream);
		// Close file.
		fclose(stream);
	}

	return ret; 
}

int getMemoryInfo(int *count, int *total, DbType source)
{
	int ret = 0;

	switch (source)
	{
	case db_PhoneBook:
		{
			ret = getMemoryInfoActual(count, total, Db_Name);
		}
		break;
	case db_Trash:
		{
			ret = getMemoryInfoActual(count, total, Trash_Name);
		}
		break;
	default:
		break;
	}

	return ret;
}

int getMemoryInfoActual(int *count, int *total, TCHAR *dbName)
{
	int ret = 0;
	FILE *stream;
	DbInfo dbInfo;

	stream = _tfopen(dbName, TEXT("rb"));
	// Does file exist?
	if (stream)
	{
		ret = (int)fread(&dbInfo, DbItemSize, 1, stream);
		*count = dbInfo.itemCount;
		*total = MAX_ITEMCOUNT;
		fclose(stream);
	}

	return ret;
}

int checkDataIntegrity()
{
	// Assumes maximum filename length is 19 bytes.
	TCHAR DBFileNames[2][20];
	int i, ret = 1;
	FILE *stream;
	DbInfo dbInfo;

	stream = _tfopen(Cfg_Name, TEXT("rb"));
	// Does file exist?
	if (stream)
	{
		// If username\password file is not the correct length, then file is corrupted.
		if (getFileLen(stream) != (MAX_USERNAME + MAX_PASSWORD) * sizeof(TCHAR))
			ret = 0;
		fclose(stream);
	}
	else
		ret = 0;

	// Copy database names to array to reuse the code by For loop.
	_tcscpy(DBFileNames[0], Db_Name);
	_tcscpy(DBFileNames[1], Trash_Name);
	// Loop through both DB files.
	for (i = 0; i < 2 && ret; i++)
	{
		// Does file exist?
		stream = _tfopen(DBFileNames[i], TEXT("rb"));
		if (stream)
		{
			// Attempt to read the DB header. If fread() fails, file is corrupted or empty.
			if (fread(&dbInfo, DbItemSize, 1, stream))
			{
				// Compare size of file to the expected size of file, if it doesn't match, file is corrupted.
				if (getFileLen(stream) != getOffset(dbInfo.itemCount))
					ret = 0;
			}
			else
				ret = 0;
			fclose(stream);
		}
		else
			ret = 0;
	}

	return ret;
}

long getFileLen(FILE *file)
{
	long OrigPos, FileLen;

	// Get current position in file.
	OrigPos = ftell(file);
	// Retrieve the file's length in bytes.
	fseek(file, 0, SEEK_END);
	FileLen = ftell(file);
	// Set file pointer back to the original position.
	fseek(file, OrigPos, SEEK_SET);

	return FileLen;
}

void dupContact(Contact *dst, Contact *src)
{
	if (dst && src)
		memcpy(dst, src, ItemSize);
}

// XOR Enc/Decryption algorithm
void encryptDecryptCredentials(TCHAR *dst, TCHAR *src, int Len)
{
	TCHAR key = 85;
	int i = 0;

	for (i = 0; i < Len; i++)
		dst[i] = src[i] ^ key;
}

/*
***************************Extension functions***************************
*/

// Adds a contact to local list if one is loaded.
int addContactToLocalList(Contact *contact)
{
	int ret = FALSE;
	Contact *newContact;
	DynamicListC list = *getCurrentContactList(FALSE);
	if (list)
	{
		newContact = (Contact*)malloc(sizeof(Contact));
		dupContact(newContact, contact);
		ret = listInsertAfterEnd(list, &newContact);
	}
	return ret;
}

// Deletes the current list from memory. 
// This forces getContactListInitiated() to return the list from the file.
void freeContactListLocal()
{
	DynamicListC *contactListPtr = getCurrentContactList(FALSE);
	if (*contactListPtr)
		listFree(contactListPtr);
}

// Retrieves the list currently held in memory.
DynamicListC getContactListLocal()
{
	return *getCurrentContactList(FALSE);
}

// Retrieves the list from database.
DynamicListC getContactListFromFile()
{
	return *getCurrentContactList(TRUE);
}

// Retrieves an initiated list, with memory list having the higher priority.
DynamicListC getContactListInitiated()
{
	DynamicListC list = *getCurrentContactList(FALSE);
	if (!list)
		list = *getCurrentContactList(TRUE);
	return list;
}

// Local function that actually does the work.
DynamicListC *getCurrentContactList(int fromFile)
{
	static DynamicListC contactList = NULL;

	if (fromFile)
	{	
		if (contactList)
			listFree(&contactList);
		contactList = getContactList();
	}

	return &contactList;
}