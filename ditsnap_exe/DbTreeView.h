#pragma once
#include "Interfaces.h"
#include "EseDbManager.h"

// Defined in EseDataAccess.h 
namespace EseDataAccess
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

	DbTreeView(EseDbManager* eseDbManager);
	~DbTreeView(void);

	LRESULT OnTreeDoubleClick(LPNMHDR pnmh) const;
	virtual void LoadEseDbManager() override;

private:
	EseDbManager* eseDbManager_;
};
