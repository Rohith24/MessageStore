#include "stdafx.h"
#include<string.h>
#include<stdlib.h>
void print_users(FILE *f,int *csock){
	char buffer[1024] = "";
	char buff[10] = "";
	fseek(f, bitvectorsize*sizeof(int), 0);
	for (int i = 0; i < 20; i++){
		struct UserMsg user;
		fread(&user, sizeof(struct UserMsg), 1, f);
		sprintf(buff,"%d\t%s\n", user.id, user.username);
		strcat(buffer, buff);
	}
	strcat(buffer, "\nchoose user:");
	send_data(csock, buffer,Buff_SIZE);
}

void create_category(FILE *f, int catogry, int userid, int catid,int *csock){
	char buffer[1024] = "";
	char buff[10] = "";
	//printf("%d\t", ftell(f));
	fseek(f, catogry, 0);
	//printf("%d\n", ftell(f));
	struct Category_inode ctg;
	ctg.userid = userid;
	ctg.category_id = catid*userid;
	send_data(csock, "\nEnter catogiry name:",Buff_SIZE);
	//printf("\nEnter catogiry name:");
	recv_data(csock, ctg.name, 1024);
	//scanf("%s", ctg.name);
	for (int i = 0; i < 10; i++){
		ctg.messages[i] = 0;
	}
	ctg.msg_single_indirect_block = NULL;
	ctg.double_indirect_block = NULL;
	fwrite(&ctg, sizeof(struct Category_inode), 1, f);
	fflush(f);
}

void print_catogiries(FILE *f, struct UserMsg user, int current_user_id,int *csock){
	char buffer[1024] = "";
	char buff[10] = "";
	for (int i = 0; i < 5; i++){
		if (user.ctgs[i] != NULL){
			struct Category_inode cat;
			fseek(f, (long)user.ctgs[i], 0);
			fread(&cat, sizeof(struct Category_inode), 1, f);
			sprintf(buff,"\n%d\t%s", cat.category_id, cat.name);
			strcat(buffer, buff);
			//printf("\n%s", cat.name);
		}
	}
	send_data(csock, buffer, Buff_SIZE);
}

int menu_catogiries(FILE *f, struct UserMsg user, int current_user_id,int *csock){
	while (1)
	{
		send_data(csock, "\n1-View Catogiries\n2-Create Carogiry\n", Buff_SIZE);
		//printf("\n1-View Catogiries\n2-Create Carogiry\n");
		int option;
		char convert[1024];
		recv_data(csock, convert, 1024);
		option = atoi(convert);
		switch (option)
		{
		case 1:
			//send_data(csock, "\nThe catogiries are:");
			print_catogiries(f, user, current_user_id, csock);
			return 1;
		case 2:
			for (int i = 0; i < 5; i++){
				if (user.ctgs[i] == NULL){
					fseek(f, ((bitvectorsize*sizeof(int)) + (current_user_id - 1)* sizeof(struct UserMsg)), 0);
					//typecast 
					user.ctgs[i] =(Category_inode *)((bitvectorsize*sizeof(int)+20 * sizeof(struct UserMsg)) + ((user.id - 1) * 5 * sizeof(struct Category_inode)) + (i*sizeof(struct Category_inode)));
					fwrite(&user, sizeof(struct UserMsg), 1, f);
					create_category(f, (int)user.ctgs[i], user.id, i + 1,csock);
					print_catogiries(f, user, current_user_id,csock);
					break;
				}
			}
			return 2;
		default:
			break;
		}
	}
}

int search_category(FILE *f, struct UserMsg user, int catid){
	struct Category_inode cat;
	for (int i = 0; i < 5; i++){
		if (user.ctgs[i] != 0){
			struct Category_inode cat;
			fseek(f, (int)user.ctgs[i], 0);
			fread(&cat, sizeof(struct Category_inode), 1, f);
			if (cat.category_id == (catid*user.id)){
				return (int)user.ctgs[i];
			}
		}
	}
	return NULL;
}

void print_messages(FILE *f, struct Category_inode cat, int current_user_id,int *csock){
	struct Message_inode msg;
	int j = 1;
	char buff[1024]="",buffer[1024]="";
	strcat(buffer, "\n\nMessages are:");
	//printf("\n\nMessages are:");
	for (int i = 0; i < 10; i++){
		if (cat.messages[i] != NULL){
			fseek(f, cat.messages[i], 0);
			//printf("%ld", ftell(f));
			fread(&msg, sizeof(struct Message_inode), 1, f);
			sprintf(buff,"\n%d:\nUser%d says:\t%s", j++, msg.userid, msg.msgtext);
			strcat(buffer, buff);
		}
	}
	strcat(buffer, "\nEnter msg number to view msg. Enter 0 to Exit\n");
	send_data(csock, buffer, Buff_SIZE);
	/*if (cat.msg_single_indirect_block != 0){
		do{
			printf("\n\tNext->");
			printf("\nEnter choice(n-Next,e-exit:");
			char choice;
			if (choice == 'n'){
				int *msgsib;
				fseek(f, cat.msg_single_indirect_block, 0);
				fread(msgsib, sizeof(int), 1, f);
				for (int i = 0; i < 10; i++)
				{
					if (msgsib[i] != NULL){
						fseek(f, msgsib[i], 0);
						fread(&msg, sizeof(struct Message_inode), 1, f);
						printf("\n%d:\nUser%d says:\t%s", j++, msg.userid, msg.msgtext);
					}
				}
			}
			else if (choice == 'e')
			{
				return;
			}
		} while (1);
	}*/
}

void create_message(FILE *f, struct Category_inode *cat, int userid,int *csock){
	struct Message_inode msg;
	msg.userid = userid;
	send_data(csock, "\nEnter Msg text:", Buff_SIZE);
	//scanf("%s", msg.msgtext);
	fflush(stdin);
	//scanf("%[^\n]s", msg.msgtext);
	//gets(msg.msgtext);
	recv_data(csock, msg.msgtext, 1024);
	for (int i = 0; i < 10; i++){
		msg.replys[i] = 0;
	}
	msg.single_indirect_block = NULL;
	msg.double_indirect_block = NULL;
	fwrite(&msg, sizeof(struct Message_inode), 1, f);
}

int get_free_space(FILE *f){
	int bitvector[bitvectorsize];
	fseek(f, 0, 0);
	fread(bitvector, sizeof(int), bitvectorsize, f);
	int freespace;
	for (int i = 0; i < bitvectorsize; i++){
		if (bitvector[i] == 0)
		{
			bitvector[i] = 1;
			freespace = messegestartoffset + (i*sizeof(struct Message_inode));
			break;
		}
	}
	fseek(f, 0, 0);
	fwrite(bitvector, sizeof(int), bitvectorsize, f);
	fflush(f);
	return freespace;
}

int get_free_space_relpy(FILE *f){
	int bitvector[bitvectorsize];
	fseek(f, 0, 0);
	fread(bitvector, sizeof(int), bitvectorsize, f);
	int freespace;
	for (int i = 640; i < bitvectorsize; i++){
		if (bitvector[i] == 0)
		{
			bitvector[i] = 1;
			freespace = replystartoffset + (i*sizeof(struct reply));
			break;
		}
	}
	fseek(f, 0, 0);
	fwrite(bitvector, sizeof(int), bitvectorsize, f);
	fflush(f);
	return freespace;
}

int menu_messages(FILE *f, struct Category_inode cat, int current_user_id, int offset,int *csock){
	char buffer[1024] = "";
	char buff[1024] = "";
	while (1)
	{
		send_data(csock, "\n1-View Messages\n2-Create Message:\n", Buff_SIZE);
		recv_data(csock, buff, 1024);
		int option;
		option = atoi(buff);
		//scanf("%d", &option);
		switch (option)
		{
		case 1:
			//send_data(csock, "\nThe Messages are:\n");
			//printf("\nThe Messages are:\n");
			//print_messages(f, cat, current_user_id,csock);
			return 1;
		case 2:
			for (int i = 0; i < 10; i++){
				if (cat.messages[i] == NULL){
					int space = get_free_space(f);
					cat.messages[i] = space;
					fseek(f, offset, 0);
					fwrite(&cat, sizeof(struct Category_inode), 1, f);
					fseek(f, space, 0);
					create_message(f, &cat, current_user_id,csock);
					break;
				}
			}
			return 2;
		default:
			break;
		}
	}
}

void delete_msg(FILE *f, int msgoffset, struct Category_inode cat, int offset){
	int bitvector[bitvectorsize];
	fseek(f, 0, 0);
	fread(bitvector, sizeof(int), bitvectorsize, f);
	bitvector[(msgoffset - messegestartoffset) / sizeof(struct Message_inode)] = 0;
	for (int i = 0; i < 5; i++){
		if (cat.messages[i] == msgoffset){
			struct Message_inode msg;
			fseek(f, msgoffset, 0);
			fread(&msg, sizeof(struct Message_inode), 1, f);
			for (int j = 0; j < 10; j++){
				msg.replys[j] = 0;
			}
			cat.messages[i] = NULL;
			break;
		}
	}
	fseek(f, 0, 0);
	fwrite(bitvector, sizeof(int), bitvectorsize, f);
	fseek(f, offset, 0);
	fwrite(&cat, sizeof(struct Category_inode), 1, f);
	fflush(f);
}

void print_replys(FILE *f, struct Message_inode msg, int current_user_id, int msgoffset,int *csock){
	struct reply rly;
	char buff[1024] = "";
	char buffer[1024] = "";
	int j = 1;
	for (int i = 0; i < 10; i++){
		if (msg.replys[i] != NULL){
			fseek(f, msg.replys[i], 0);
			//printf("%ld", ftell(f));
			fread(&rly, sizeof(struct reply), 1, f);
			sprintf(buff,"\n%d:\nUser%d\t%s", j++, rly.userid, rly.rpltext);
			strcat(buffer, buff);
		}
	}
	//printf("\n\n1-reply\n2-back");
	strcat(buffer, "\n\n1-reply\n2-back");
	send_data(csock, buffer, Buff_SIZE);
	recv_data(csock, buff, 1024);
	int option=atoi(buff);
	//scanf("%d", &option);
	if (option == 1){
		struct reply rly;
		rly.userid = current_user_id;
		send_data(csock, "\nEnter reply text:", Buff_SIZE);
		//printf("\nEnter reply text:");
		fflush(stdin);
		//scanf("%s", rly.rpltext);
		//scanf("%[^\n]s", rly.rpltext);
		//gets(rly.rpltext);
		recv_data(csock, rly.rpltext, 1024);
		for (int i = 0; i < 10; i++){
			if (msg.replys[i] == NULL){
				int space = get_free_space_relpy(f);
				msg.replys[i] = space;
				fseek(f, msgoffset, 0);
				fwrite(&msg, sizeof(struct Category_inode), 1, f);
				fseek(f, space, 0);
				fwrite(&rly, sizeof(struct reply), 1, f);
				fflush(f);
				break;
			}
		}
		print_replys(f, msg, current_user_id, msgoffset,csock);
	}
}

void messages(FILE *f, int user_id, int current_user_id, struct Category_inode cat, int catogiry,int *csock){
	int menuoption = 1;
	char convert[1024];
	if (user_id == current_user_id)
		menuoption = menu_messages(f, cat, current_user_id, catogiry,csock);
	if (menuoption == 1)
	{
		print_messages(f, cat, current_user_id, csock);
		//printf("\nEnter msg number to view msg. Enter 0 to Exit\n");
		int option;
		recv_data(csock, convert, 1024);
		option = atoi(convert);
		//scanf("%d", &option);
		if (option == 0)
			return;
		struct Message_inode msg;
		int j = 1;
		int msgoffset;
		for (int i = 0; i < 10; i++){
			if (cat.messages[i] != NULL){
				if (option == j){
					msgoffset = cat.messages[i];
					fseek(f, cat.messages[i], 0);
					//printf("%ld", ftell(f));
					fread(&msg, sizeof(struct Message_inode), 1, f);
					break;
				}
				j++;
			}
		}
		//send_data(csock, "");
		char buff[1024] = "";
		char buffer[1024] = "";
		sprintf(buff,"\n\nThe message:\t%d:\t%s\nEnter a key to continue\n", msg.userid, msg.msgtext);
		send_data(csock, buff, Buff_SIZE);
		recv_data(csock, buff, 1024);
		print_replys(f, msg, current_user_id, msgoffset,csock);
	 	strcpy(buff,"");
		strcpy(buffer,"\nChoose reply number to delete reply or -1-to exit:");
		//printf("\nChoose reply number to delete reply or -1-to exit:");
		send_data(csock, buffer, Buff_SIZE);
		recv_data(csock, buff, 1024);
		option = atoi(buff);
		//scanf("%d", &option); 
		if (option == 0)
			delete_msg(f, msgoffset, cat, catogiry);
		else if (option == -1)
		{
			return;
		}
	}
	else
	{
		char buff[1024];
		send_data(csock, "\n\n--No messages--", Buff_SIZE);
		recv_data(csock, buff, 1024);
	}
}

void message(int *csock, int current_user_id){
	FILE *f = fopen("MessageSystem.bin", "r+b");
	int userid;
	int catid, menuoption = 1, count = 0;
	struct UserMsg user;
	struct Category_inode cat;
	//int current_user_id;
	char buff[Buff_SIZE];
	int *catogiries = (int *)malloc(sizeof(20));
	//print_users(f,csock);
	//send_data(csock, "\nEnter your userid:");
	char convert[1024];
	//recv_data(csock, convert, 1024);
	current_user_id = atoi(convert);
	//scanf("%d", &current_user_id);
	char replybuf[1024] = "\Message Store Menu:\n\n1-search by usernames\n2-search by catogiries\n0-Exit\n\nchoose option:";
	send_data(csock, replybuf, Buff_SIZE);
	recv_data(csock, buff, Buff_SIZE);
	int switchon = atoi(buff);
	switch (switchon)
	{
	case 1:
		print_users(f,csock);
		//send_data(csock, "\nchoose user:");
		recv_data(csock, convert, 1024);
		userid = atoi(convert);
		fseek(f, (bitvectorsize*sizeof(int)+(userid - 1) * sizeof(struct UserMsg)), 0);
		fread(&user, sizeof(struct UserMsg), 1, f);
		if (user.id == current_user_id)
			menuoption = menu_catogiries(f, user, current_user_id,csock);
		else{
			print_catogiries(f, user, current_user_id,csock);
		}
		int catogiry;
		if (menuoption == 1){
			//printf("\nchoose catogiry id:");
			recv_data(csock, convert, 1024);
			//scanf("%d", &catid);
			catid = atoi(convert);
			catogiry = search_category(f, user, catid);
			if (catogiry != NULL){
				fseek(f, catogiry, 0);
				fread(&cat, sizeof(struct Category_inode), 1, f);
			}
			else
			{
				send_data(csock, "\nCatogiry Not found\nEnter a key to continue:",Buff_SIZE);
				//printf("\nCatogiry Not found\nEnter a key to continue:");
				recv_data(csock, buff, Buff_SIZE);
				strcpy(buff,"");
				break;
			}
			system("cls");
			messages(f, user.id, current_user_id, cat, catogiry,csock);
		}
		break;
	case 2:
		for (int i = 0; i < 20; i++){
			fseek(f, (bitvectorsize*sizeof(int)+(i* sizeof(struct UserMsg))), 0);
			fread(&user, sizeof(struct UserMsg), 1, f);
			print_catogiries(f, user, current_user_id,csock);
		}
		//printf("\nchoose catogiry:");
		recv_data(csock, buff, 1024);
		catid = atoi(buff);
		//scanf("%d", &catid);
		system("cls");
		count = 0;
		for (int i = 0; i < 20; i++){
			fseek(f, (bitvectorsize*sizeof(int)+(i* sizeof(struct UserMsg))), 0);
			fread(&user, sizeof(struct UserMsg), 1, f);
			catogiry = search_category(f, user, catid);
			if (catogiry != NULL){
				fseek(f, catogiry, 0);
				fread(&cat, sizeof(struct Category_inode), 1, f);
				count++;
				break;
			}
		}
		if (count>0){
			system("cls");
			messages(f, user.id, current_user_id, cat, catogiry,csock);

		}
		else
		{
			//printf("\nCatogiry Not found..");
			break;
		}
		break;
	default:
		break;
	}
}