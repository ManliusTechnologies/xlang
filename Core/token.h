#pragma once

#include "def.h"

namespace XPython {
enum LastCharType
{
	LCT_None,
	LCT_Space,
	LCT_Str,//with ''
	LCT_Str2,//with " "
	LCT_Ops,
	LCT_Alpha,//digit and alphabet
	LCT_EOS//End of Stream
};
struct CoreContext
{
	short* kwTree;

	char* src_code;
	int src_code_size;

	//Cursor
	char* spos;//source pos
	short curNode;//offset in kwTree
	LastCharType lct;
	char* token_start;
};
enum TokenIndex
{
	TokenID = -10,
	TokenNum = -11,
	TokenStr = -12,
	TokenEOS = -13
};

#pragma pack(push,1)
struct node
{
	short index;
	unsigned char c;
	unsigned char child_cnt;
};
#pragma pack(pop)

class Token
{
	CoreContext _context;
	inline char GetChar()
	{
		return *_context.spos++;
	}
	inline void ResetToRoot()
	{
		_context.curNode = 0;
	}
	inline short GetLastMatchedNodeIndex()
	{
		node* pNode = (node*)(_context.kwTree + _context.curNode);
		return pNode->index;
	}
	bool MatchInTree(char c);
	inline bool InStr(char c, char* str)
	{
		bool bYes = false;
		char* p = str;
		while (*p)
		{
			if (*p == c)
			{
				bYes = true;
				break;
			}
			p++;
		}
		return bYes;
	}
	short Scan(String& id);

public:
	Token(short* kwTree)
	{
		_context.kwTree = kwTree;
		_context.lct = LCT_None;
		_context.curNode = 0;
		_context.token_start = nil;
	}
	void SetStream(char* code, int size)
	{
		_context.src_code = code;
		_context.src_code_size = size;
		_context.spos = _context.src_code;
	}
	short Get(String& id)
	{
		short retIndex = -1;
		while (retIndex == -1)
		{
			retIndex = Scan(id);
		}
		if (retIndex == TokenID)
		{

		}
		return retIndex;
	}
};
}