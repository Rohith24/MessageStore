#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#define USER_SIZE 32
#define USER_OFFSET 4
typedef struct user{
	int userid;
	char name[28];
}User;

void addnewUser(FILE *fp){
	int no;
	fseek(fp, 0, 0);
	int count=fread(&no, sizeof(int), 1, fp);
	if (count == 0)
		no = 0;
	User u;
	memset(&u, 0, sizeof(User));
	u.userid = no + 1;
	printf("Enter user name:");
	scanf("%s", u.name);
	fseek(fp, (no*USER_SIZE) + USER_OFFSET, 0);
	fwrite(&u, sizeof(User), 1, fp);
	fflush(fp);
	no++;
	fseek(fp, 0, 0);
	fwrite(&no, sizeof(int), 1, fp);
	fflush(fp);
}