#pragma once
#include "Interfaces.h"
#include "EseRepository.h"

// Defined in EseDataAccess.h 
namespace Ese
{
	class EseTable;
	class EseDatabase;
	class EseInstance;
}

class DbTreeView : public CWindowImpl<DbTreeView, CTreeViewCtrl>, IDbObserver
{
public:
	DECLARE_WND_SUPERCLASS(nullptr, CTreeViewCtrl::GetWndClassName())

	BEGIN_MSG_MAP_EX(DbTreeView)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_DBLCLK, OnTreeDoubleClick)
		DEFAULT_REFLECTION_HANDLER()
		END_MSG_MAP()

	DbTreeView(EseRepository* eseRepository);
	~DbTreeView(void);

	LRESULT OnTreeDoubleClick(LPNMHDR pnmh) const;
	virtual void LoadEseRepository() override;

private:
	EseRepository* eseRepository_;
};
