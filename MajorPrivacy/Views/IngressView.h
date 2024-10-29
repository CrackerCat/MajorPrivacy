#pragma once

#include "../../MiscHelpers/Common/PanelView.h"
#include "../../MiscHelpers/Common/TreeviewEx.h"
#include "../Models/IngressModel.h"
#include "../Core/Programs/WindowsService.h"

class CIngressView : public CPanelViewEx<CIngressModel>
{
	Q_OBJECT

public:
	CIngressView(QWidget *parent = 0);
	virtual ~CIngressView();

	void					Sync(const QSet<CProgramFilePtr>& Programs, const QSet<CWindowsServicePtr>& Services, bool bAllPrograms);

protected:
	virtual void			OnMenu(const QPoint& Point) override;
	void					OnDoubleClicked(const QModelIndex& Index) override;

private slots:
	//void					OnResetColumns();
	//void					OnColumnsChanged();

protected:

	QToolBar*				m_pToolBar;
	QComboBox*				m_pCmbRole;

	QSet<CProgramFilePtr>					m_CurPrograms;
	QSet<CWindowsServicePtr>				m_CurServices;
	QMap<SIngressKey, SIngressItemPtr>		m_ParentMap;
	QMap<SIngressKey, SIngressItemPtr>		m_IngressMap;
	qint32									m_FilterRole = 0;
};