#include "StdAfx.h"
#include "DocHistoryMgr.h"

void CDocHistory::Do(void)
{
	const_iterator hist_iter = begin();
	for(; hist_iter != end(); hist_iter++)
	{
		hist_iter->first->Do();
	}
}

void CDocHistory::Undo(void)
{
	const_reverse_iterator hist_iter = rbegin();
	for(; hist_iter != rend(); hist_iter++)
	{
		hist_iter->second->Do();
	}
}
