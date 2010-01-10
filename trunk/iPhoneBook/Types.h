#pragma once

#define MAX_ITEMCOUNT 1000

#define MAX_FNAME 16
#define MAX_LNAME 21
#define MAX_SKYPE 21
#define MAX_EMAIL 41
#define MAX_PHONE 16
#define MAX_SITE 51
#define MAX_COUNTRY 16
#define MAX_CITY 21
#define MAX_STREET 21
#define MAX_NUMBER 11
#define MAX_GROUPNAME 21

#define MAX_USERNAME 21
#define MAX_PASSWORD 21

typedef struct group 
{
	int groupIndex;
	TCHAR groupName [MAX_GROUPNAME];
}Group;

typedef struct dbInfo
{
	unsigned int itemCount;
	int NumberOfGroups;
}DbInfo;

typedef struct address
{
	TCHAR country[MAX_COUNTRY];
	TCHAR city[MAX_CITY];
	TCHAR street[MAX_STREET];
	TCHAR number[MAX_NUMBER];
}Address;


typedef struct contact
{
    TCHAR firstName[MAX_FNAME];
    TCHAR lastName[MAX_LNAME];
	TCHAR skypeName[MAX_SKYPE];
	TCHAR email[MAX_EMAIL];
	TCHAR phone[MAX_PHONE];
	int age;
	TCHAR webSite[MAX_SITE];
	Address address;
	int group; //index of group
	unsigned int index;
} Contact;

typedef enum { db_PhoneBook, db_Trash } DbType;