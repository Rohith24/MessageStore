#include "stdafx.h"
#define bitvectorsize 1280
#define messegestartoffset 32000
#define replystartoffset 512000
struct Message_inode
{
	char msgtext[128];
	int userid;
	int replys[10];
	int *single_indirect_block;
	int *double_indirect_block;
};
struct single_indirect_block{
	int directblocks[32];
};

struct Category_inode
{
	int category_id;
	char name[32];
	int userid;
	int messages[10];
	int msg_single_indirect_block;
	int double_indirect_block;
};
struct reply
{
	char rpltext[128];
	int userid;
};

struct UserMsg{
	int id;
	char username[32];
	struct Category_inode *ctgs[5];
};

void message(int *csock, int current_user_id);