#include "xclass.h"
#include "object.h"
namespace X
{
namespace AST
{
bool XClass::Call(Runtime* rt,
	void* pContext,
	std::vector<Value>& params, 
	std::unordered_map<std::string, AST::Value>& kwParams,
	Value& retValue)
{
	Data::XClassObject* obj = new Data::XClassObject(this);
	if (m_constructor)
	{
		m_constructor->Call(rt,obj,params, kwParams,retValue);
	}
	retValue = Value(obj);
	return true;
}
void XClass::ScopeLayout()
{
	Scope* pMyScope = GetScope();
	if (pMyScope)
	{
		std::string strName(m_Name.s, m_Name.size);
		m_Index = pMyScope->AddOrGet(strName, false);
	}
}
XClass* XClass::FindBase(Runtime* rt, std::string& strName)
{
	XClass* pBase = nullptr;
	auto* pScope = GetScope();
	while (pScope != nullptr)
	{
		int idx = pScope->AddOrGet(strName, true);
		if (idx != -1)
		{
			Value v0;
			if (pScope->Get(rt,nullptr,idx, v0))
			{
				if (v0.IsObject())
				{
					Data::Object* pBaseObj = (Data::Object*)v0.GetObject();
					auto* pXBaseObj = dynamic_cast<Data::XClassObject*>(pBaseObj);
					if (pXBaseObj)
					{
						pBase = pXBaseObj->GetClassObj();
						break;
					}
				}
			}
		}
		pScope = pScope->GetParentScope();
	}
	return pBase;
}
bool XClass::Set(Runtime* rt, void* pContext, int idx, Value& v)
{
	if (pContext)
	{
		Data::XClassObject* pObj = (Data::XClassObject*)pContext;
		pObj->GetStack()->Set(idx, v);
	}
	else if (m_stackFrame)
	{
		m_stackFrame->Set(idx, v);
	}
	return true;
}
bool XClass::Get(Runtime* rt,void* pContext, int idx, Value& v, LValue* lValue)
{
	if (pContext)
	{
		Data::XClassObject* pObj = (Data::XClassObject*)pContext;
		pObj->GetStack()->Get(idx, v, lValue);
	}
	else if (m_stackFrame)
	{
		m_stackFrame->Get(idx, v, lValue);
	}
	return true;
}
bool XClass::Run(Runtime* rt,void* pContext,Value& v, LValue* lValue)
{
	m_stackFrame = new StackFrame(this);
	m_stackFrame->SetVarCount((int)m_Vars.size());
	for (auto it : m_tempMemberList)
	{
		Value& v0 = it.second;
		bool bSet = false;
		if (v0.IsObject())
		{
			Data::Object* pObj = (Data::Object*)v0.GetObject();
			if (pObj->GetType() == Data::Type::Expr)
			{
				Data::Expr* pExpr = dynamic_cast<Data::Expr*>(pObj);
				if (pExpr)
				{
					AST::Expression*  pExpression = pExpr->Get();
					if (pExpression)
					{
						Value v1;
						if (pExpression->Run(rt,pContext,v1))
						{
							Set(rt,pContext,it.first, v1);
							bSet = true;
						}
					}
				}
			}
		}
		if (!bSet)
		{
			Set(rt,pContext,it.first, v0);
		}
	}
	m_tempMemberList.clear();

	Data::XClassObject* cls = new Data::XClassObject(this);
	Value v0(cls);
	bool bOK = m_scope->Set(rt,pContext,m_Index, v0);
	v = v0;

	//prcoess base classes
	if (Params)
	{
		auto& list = Params->GetList();
		for (auto i : list)
		{
			std::string strVarName;
			if (i->m_type == ObType::Var)
			{
				Var* varName = dynamic_cast<Var*>(i);
				String& szName = varName->GetName();
				strVarName = std::string(szName.s, szName.size);
				auto* pBaseClass = FindBase(rt,strVarName);
				if (pBaseClass)
				{
					m_bases.push_back(pBaseClass);
				}
			}
		}
	}
	return bOK;
}

void XClass::Add(Expression* item)
{
	switch (item->m_type)
	{
	case ObType::Param:
	{
		Param* param = dynamic_cast<Param*>(item);
		std::string strVarName;
		std::string strVarType;//TODO: deal with type
		Value defaultValue;
		if (param->Parse(strVarName, strVarType, defaultValue))
		{
			int idx = AddOrGet(strVarName,false);
			m_tempMemberList.push_back(std::make_pair(idx, defaultValue));
		}
	}
		break;
	case ObType::Func:
	{
		Func* func = dynamic_cast<Func*>(item);
		String& funcName = func->GetNameStr();
		std::string strName(funcName.s, funcName.size);
		int idx = AddOrGet(strName, false);
		Data::Function* f = new Data::Function(func);
		Value funcObj(f);
		m_tempMemberList.push_back(std::make_pair(idx, funcObj));
		if (strName == "constructor")//TODO: add class name also can be constructor
		{
			m_constructor = func;
		}
	}
		break;
	default:
		break;
	}

	item->SetParent(this);
	item->ScopeLayout();
}
}
}