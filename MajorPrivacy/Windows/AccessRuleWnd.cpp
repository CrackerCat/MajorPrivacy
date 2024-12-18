#include "pch.h"
#include "AccessRuleWnd.h"
#include "../Core/PrivacyCore.h"
#include "../Core/Programs/ProgramManager.h"
#include "../Core/Access/AccessManager.h"
#include "../Library/API/PrivacyAPI.h"
#include "../Library/Helpers/NetUtil.h"
#include "../Library/Helpers/NtUtil.h"
#include "../MiscHelpers/Common/SettingsWidgets.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MajorPrivacy.h"
#include "../Core/Volumes/VolumeManager.h"

CAccessRuleWnd::CAccessRuleWnd(const CAccessRulePtr& pRule, QSet<CProgramItemPtr> Items, const QString& VolumeRoot, const QString& VolumeImage, QWidget* parent)
	: QDialog(parent)
{
	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::CustomizeWindowHint;
	//flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	//flags &= ~Qt::WindowMinMaxButtonsHint;
	//flags |= Qt::WindowMinimizeButtonHint;
	//flags &= ~Qt::WindowCloseButtonHint;
	flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	setWindowFlags(flags);

	ui.setupUi(this);


	m_pRule = pRule;
	bool bNew = m_pRule->m_Guid.isEmpty();

	setWindowTitle(bNew ? tr("Create Program Rule") : tr("Edit Program Rule"));

	foreach(auto pItem, Items) 
	{
		switch (pItem->GetID().GetType())
		{
		case EProgramType::eProgramFile:
		case EProgramType::eFilePattern:
		case EProgramType::eAppInstallation:
		case EProgramType::eAllPrograms:
			break;
		default:
			continue;
		}

		m_Items.append(pItem);
		ui.cmbProgram->addItem(pItem->GetNameEx());
	}

	if (bNew && m_pRule->m_Name.isEmpty()) {
		m_pRule->m_Name = tr("New Access Rule");
	}

	m_VolumeRoot = VolumeRoot;
	m_VolumeImage = VolumeImage;

	connect(ui.cmbProgram, SIGNAL(currentIndexChanged(int)), this, SLOT(OnProgramChanged()));


	connect(ui.buttonBox, SIGNAL(accepted()), SLOT(OnSaveAndClose()));
	connect(ui.buttonBox, SIGNAL(rejected()), SLOT(reject()));

	ui.cmbAction->addItem(tr("Allow"), (int)EAccessRuleType::eAllow);
	ui.cmbAction->addItem(tr("Read Only"), (int)EAccessRuleType::eAllowRO);
	ui.cmbAction->addItem(tr("Directory Listing"), (int)EAccessRuleType::eEnum);
	ui.cmbAction->addItem(tr("Protect"), (int)EAccessRuleType::eProtect);
	ui.cmbAction->addItem(tr("Block"), (int)EAccessRuleType::eBlock);
	

	//FixComboBoxEditing(ui.cmbGroup);


	ui.txtName->setText(m_pRule->m_Name);
	// todo: load groups
	//SetComboBoxValue(ui.cmbGroup, m_pRule->m_Grouping); // todo
	ui.txtInfo->setPlainText(m_pRule->m_Description);

	ui.cmbProgram->setEditable(true);
	ui.cmbProgram->lineEdit()->setReadOnly(true);
	CProgramItemPtr pItem;
	if (m_pRule->m_ProgramID.GetType() == EProgramType::eAllPrograms)
		pItem = theCore->ProgramManager()->GetAll();
	else if(!bNew)
		pItem = theCore->ProgramManager()->GetProgramByID(m_pRule->m_ProgramID);
	int Index = m_Items.indexOf(pItem);
	ui.cmbProgram->setCurrentIndex(Index);
	if(bNew)
		OnProgramChanged();
	else
		ui.txtProgPath->setText(m_pRule->GetProgramPath());

	if(!m_pRule->m_AccessPath.isEmpty())
		ui.txtPath->setText(m_pRule->m_AccessPath);
	else if(!m_VolumeRoot.isEmpty())
		ui.txtPath->setText(m_VolumeRoot + "\\");
	else if (!m_VolumeImage.isEmpty())
		ui.txtPath->setText(m_VolumeImage + "/");

	SetComboBoxValue(ui.cmbAction, (int)m_pRule->m_Type);
}

CAccessRuleWnd::~CAccessRuleWnd()
{
}

void CAccessRuleWnd::closeEvent(QCloseEvent *e)
{
	emit Closed();
	this->deleteLater();
}

void CAccessRuleWnd::OnSaveAndClose()
{
	if (!Save()) {
		QApplication::beep();
		return;
	}

	if (m_pRule->m_Guid.isEmpty())
	{
		QString NtPath = QString::fromStdWString(DosPathToNtPath(ui.txtPath->text().toStdWString()));

		theCore->VolumeManager()->Update();
		auto Volumes = theCore->VolumeManager()->List();
		foreach(const CVolumePtr& pVolume, Volumes) {
			if(pVolume->GetStatus() != CVolume::eMounted)
				continue;
			QString DevicePath = pVolume->GetDevicePath();
			if (NtPath.startsWith(DevicePath, Qt::CaseInsensitive)) {
				m_pRule->SetVolumeRule(true);
				m_pRule->m_AccessPath = NtPath;
				break;
			}
		}
	}

	STATUS Status = theCore->AccessManager()->SetAccessRule(m_pRule);	
	if (theGUI->CheckResults(QList<STATUS>() << Status, this))
		return;
	accept();
}

bool CAccessRuleWnd::Save()
{
	m_pRule->m_Name = ui.txtName->text();
	//m_pRule->m_Grouping = GetComboBoxValue(ui.cmbGroup).toString(); // todo
	m_pRule->m_Description = ui.txtInfo->toPlainText();

	/*int Index = ui.cmbProgram->currentIndex();
	if (Index == -1) 
		return false;*/

	/*CProgramItemPtr pItem = m_Items[Index];
	if (m_pRule->m_ProgramID != pItem->GetID()) {
		m_pRule->m_Path = pItem->GetPath();
	}*/
	m_pRule->m_ProgramID.SetPath(QString::fromStdWString(DosPathToNtPath(ui.txtProgPath->text().toStdWString())));
	m_pRule->m_ProgramPath = ui.txtProgPath->text();

	QString Path = ui.txtPath->text();
	if (!m_VolumeImage.isEmpty() && (Path.length() < m_VolumeImage.length() + 2 || !Path.startsWith(m_VolumeImage + "/", Qt::CaseInsensitive))
		&& (m_VolumeRoot.isEmpty() || Path.length() < m_VolumeRoot.length() + 2 || !Path.startsWith(m_VolumeRoot + "\\", Qt::CaseInsensitive))) {
		QMessageBox::information(this, "MajorPrivacy", tr("The path must be contained within the volume."));
		return false;
	}

	m_pRule->m_AccessPath = Path;

	m_pRule->m_Type = (EAccessRuleType)GetComboBoxValue(ui.cmbAction).toInt();

	return true;
}

void CAccessRuleWnd::OnProgramChanged()
{
	int Index = ui.cmbProgram->currentIndex();
	if (Index == -1) return;

	CProgramItemPtr pItem = m_Items[Index];
	//if (pItem) ui.cmbProgram->setCurrentText(pItem->GetName());
	CProgramID ID = pItem->GetID();
	ui.txtProgPath->setText(pItem->GetPath(EPathType::eWin32));
}
