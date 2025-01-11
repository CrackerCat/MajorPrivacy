#pragma once

#include "../Library/Status.h"
#include "AccessRule.h"

class CAccessManager: public QObject
{
	Q_OBJECT
public:
	CAccessManager(QObject* parent = nullptr);

	STATUS Update();

	bool UpdateAllAccessRules();
	bool UpdateAccessRule(const QFlexGuid& Guid);
	void RemoveAccessRule(const QFlexGuid& Guid);

	QSet<QFlexGuid> GetAccessRuleIDs() const;
	QList<CAccessRulePtr> GetAccessRules() const { return m_AccessRules.values(); }
	//QList<CAccessRulePtr> GetAccessRulesFor(const QList<const class CAccessItem*>& Nodes);
	QList<CAccessRulePtr> GetAccessRules(const QSet<QFlexGuid> &AccessRuleIDs);

	STATUS SetAccessRule(const CAccessRulePtr& pRule);
	RESULT(CAccessRulePtr) GetAccessRule(const QFlexGuid& Guid);
	STATUS DelAccessRule(const CAccessRulePtr& pRule);

protected:

	void AddAccessRule(const CAccessRulePtr& pRule);
	void RemoveAccessRule(const CAccessRulePtr& pRule);

	QMap<QFlexGuid, CAccessRulePtr>			m_AccessRules;
};

