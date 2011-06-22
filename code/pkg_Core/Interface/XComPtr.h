/*! \file XComPtr.h
 *  \brief Define autoptr class: Cx_Interface, Cx_Ptr
 *  \author Zhang Yun Gui, X3 C++ PluginFramework
 *  \date   2010.10.19
 */
#ifndef X3_CORE_XCOMPTR_H_
#define X3_CORE_XCOMPTR_H_

#include "Ix_Object.h"

// Create an object with the class id.
// It is implemented in XModuleImpl.h or XComCreator.h
int x3CreateObject(const X3CLSID& clsid, Ix_Object** ppv);

// Return the current module handle for making a distinction between plugins.
// It is implemented in XModuleImpl.h or XComCreator.h
HMODULE x3GetModuleHandle();

#if defined(_MSC_VER) && !defined(_CPPRTTI)
#error must enable RTTI (compiled with /GR)
#endif

class Cx_Ptr;

//! An autoptr class with a specified interface.
/*! x3CreateObject() is implemented in XModuleImpl.h or XComCreator.h.
    \ingroup _GROUP_PLUGIN_CORE_
    \see Cx_Ptr, x3CreateObject
*/
template <class IF_Type>
class Cx_Interface
{
    typedef Cx_Interface<IF_Type> thisClass;
public:
    Cx_Interface() : m_pInterface(NULL), m_pObj(NULL)
    {
    }

    Cx_Interface(const Cx_Ptr& src);

#if defined(_MSC_VER) && _MSC_VER > 1200 || defined(__GNUC__) // not VC60

    Cx_Interface(const thisClass& src)
        : m_pInterface(src.m_pInterface), m_pObj(copyi(src.m_pObj))
    {
    }

    Cx_Interface(IF_Type* pInterface, bool addref = true)
        : m_pInterface(pInterface)
        , m_pObj(copyi(dynamic_cast<Ix_Object*>(pInterface), addref))
    {
    }

#endif // _MSC_VER

    template <class IF_Type2>
    explicit Cx_Interface(IF_Type2* pInterface, bool addref = true)
        : m_pInterface(dynamic_cast<IF_Type*>(pInterface))
        , m_pObj(copyi(dynamic_cast<Ix_Object*>(pInterface), addref))
    {
    }

    template <class IF_Type2>
    explicit Cx_Interface(const Cx_Interface<IF_Type2>& pIF)
        : m_pInterface(dynamic_cast<IF_Type*>(pIF.P()))
        , m_pObj(copyi(dynamic_cast<Ix_Object*>(pIF.P())))
    {
    }

    explicit Cx_Interface(const X3CLSID& clsid) : m_pInterface(NULL), m_pObj(NULL)
    {
        if (0 == x3CreateObject(clsid, &m_pObj))
        {
            m_pInterface = dynamic_cast<IF_Type*>(m_pObj);
            if (NULL == m_pInterface)
            {
                m_pObj->Release(x3GetModuleHandle());
                m_pObj = NULL;
            }
        }
    }

    ~Cx_Interface()
    {
        Unload();
    }


    inline IF_Type* P() const
    {
        return m_pInterface;
    }

    inline IF_Type* operator->() const
    {
        return m_pInterface;
    }

    template <class IF_Type2>
    thisClass& operator=(IF_Type2* pInterface)
    {
        Unload();

        if (pInterface)
        {
            m_pInterface = dynamic_cast<IF_Type*>(pInterface);
            if (m_pInterface)
            {
                m_pObj = dynamic_cast<Ix_Object*>(m_pInterface);
                //ASSERT(m_pObj != NULL);
                m_pObj->AddRef(x3GetModuleHandle());
            }
        }

        return *this;
    }

    template <class IF_Type2>
    thisClass& operator=(const Cx_Interface<IF_Type2>& pIF)
    {
        return operator=(pIF.P());
    }

    thisClass& operator=(const thisClass& src)
    {
        if (this != &src)
        {
            Load(src.m_pInterface);
        }

        return *this;
    }

    thisClass& operator=(const Cx_Ptr& src);

    thisClass& operator=(const int)
    {
        //ASSERT(0 == nul);
        Unload();
        return *this;
    }

    bool operator==(const int) const
    {
        //ASSERT(0 == nul);
        return m_pInterface == NULL;
    }

    bool operator!=(const int) const
    {
        //ASSERT(0 == nul);
        return m_pInterface != NULL;
    }

    bool operator==(const IF_Type* pInterface) const
    {
        return m_pInterface == pInterface;
    }

    bool operator!=(const IF_Type* pInterface) const
    {
        return m_pInterface != pInterface;
    }

    bool operator==(const thisClass& src) const
    {
        return m_pInterface == src.m_pInterface;
    }

    bool operator!=(const thisClass& src) const
    {
        return m_pInterface != src.m_pInterface;
    }

    inline operator bool() const
    {
        return m_pInterface != NULL;
    }

    inline bool operator!() const
    {
        return NULL == m_pInterface;
    }

    inline bool IsNull() const
    {
        return NULL == m_pInterface;
    }

    inline bool IsNotNull() const
    {
        return m_pInterface != NULL;
    }


    void Release()
    {
        Unload();
    }

    IF_Type* DetachInterface()
    {
        IF_Type* pIF = m_pInterface;
        m_pInterface = NULL;
        m_pObj = NULL;
        return pIF;
    }

    bool AttachInterface(IF_Type* pIF)
    {
        Unload();
        m_pInterface = pIF;
        m_pObj = dynamic_cast<Ix_Object*>(m_pInterface);
        return (m_pObj != NULL || NULL == m_pInterface);
    }

    bool AttachInterface(Ix_Object* pIF)
    {
        Unload();

        if (pIF)
        {
            m_pInterface = dynamic_cast<IF_Type*>(pIF);
            if (m_pInterface)
            {
                m_pObj = dynamic_cast<Ix_Object*>(m_pInterface);
                //ASSERT(m_pObj != NULL);
            }
            else
            {
                pIF->Release(x3GetModuleHandle());
                pIF = NULL;
                return false;
            }
        }

        return true;
    }

    bool Create(const X3CLSID& clsid)
    {
        Unload();

        if (0 == x3CreateObject(clsid, &m_pObj))
        {
            m_pInterface = dynamic_cast<IF_Type*>(m_pObj);
            if (NULL == m_pInterface)
            {
                m_pObj->Release(x3GetModuleHandle());
                m_pObj = NULL;
            }
        }

        return m_pInterface != NULL;
    }

private:
    void Unload()
    {
        if (m_pObj)
        {
            m_pObj->Release(x3GetModuleHandle());
            m_pObj = NULL;
            m_pInterface = NULL;
        }
    }

    void Load(IF_Type* pIF)
    {
        Ix_Object* pObj = dynamic_cast<Ix_Object*>(pIF);
        if (m_pObj != pObj)
        {
            if (pObj)
            {
                pObj->AddRef(x3GetModuleHandle());
            }
            if (m_pObj)
            {
                m_pObj->Release(x3GetModuleHandle());
            }
            m_pObj = pObj;
            m_pInterface = pIF;
        }
    }

    static Ix_Object* copyi(Ix_Object* p, bool addref = true)
    {
        if (p && addref)
        {
            p->AddRef(x3GetModuleHandle());
        }
        return p;
    }

private:
    IF_Type*    m_pInterface;
    Ix_Object*  m_pObj;
};

//! An autoptr class with the Ix_Object interface.
/*! x3CreateObject() is implemented in XModuleImpl.h or XComCreator.h.
    \ingroup _GROUP_PLUGIN_CORE_
    \see Ix_Object, Cx_Interface, x3CreateObject
*/
class Cx_Ptr
{
public:
    Cx_Ptr() : m_pInterface(NULL)
    {
    }

    Cx_Ptr(const Cx_Ptr& src) : m_pInterface(src.m_pInterface)
    {
        if (m_pInterface)
        {
            m_pInterface->AddRef(x3GetModuleHandle());
        }
    }

    template <class IF_Type>
    explicit Cx_Ptr(IF_Type* pInterface) : m_pInterface(NULL)
    {
        m_pInterface = dynamic_cast<Ix_Object*>(pInterface);
        if (m_pInterface)
        {
            m_pInterface->AddRef(x3GetModuleHandle());
        }
    }

    template <class IF_Type>
    explicit Cx_Ptr(const Cx_Interface<IF_Type>& pIF) : m_pInterface(NULL)
    {
        m_pInterface = dynamic_cast<Ix_Object*>(pIF.P());
        if (m_pInterface)
        {
            m_pInterface->AddRef(x3GetModuleHandle());
        }
    }

    explicit Cx_Ptr(const X3CLSID& clsid) : m_pInterface(NULL)
    {
        x3CreateObject(clsid, &m_pInterface);
    }

    ~Cx_Ptr()
    {
        Unload();
    }


    Ix_Object* P() const
    {
        return m_pInterface;
    }

    template <class IF_Type>
    Cx_Ptr& operator=(IF_Type* pInterface)
    {
        Unload();

        m_pInterface = dynamic_cast<Ix_Object*>(pInterface);
        if (m_pInterface)
        {
            m_pInterface->AddRef(x3GetModuleHandle());
        }

        return *this;
    }

    template <class IF_Type>
    Cx_Ptr& operator=(const Cx_Interface<IF_Type>& pIF)
    {
        return operator=(pIF.P());
    }

    Cx_Ptr& operator=(const Cx_Ptr& src)
    {
        if (this != &src)
        {
            Load(src.m_pInterface);
        }

        return *this;
    }

    Cx_Ptr& operator=(const int)
    {
        //ASSERT(0 == nul);
        Unload();
        return *this;
    }

    bool operator==(const int) const
    {
        //ASSERT(0 == nul);
        return m_pInterface == NULL;
    }

    bool operator!=(const int) const
    {
        //ASSERT(0 == nul);
        return m_pInterface != NULL;
    }

    bool operator==(const Ix_Object* pInterface) const
    {
        return m_pInterface == pInterface;
    }

    bool operator!=(const Ix_Object* pInterface) const
    {
        return m_pInterface != pInterface;
    }

    bool operator==(const Cx_Ptr& src) const
    {
        return m_pInterface == src.m_pInterface;
    }

    bool operator!=(const Cx_Ptr& src) const
    {
        return m_pInterface != src.m_pInterface;
    }

    inline operator bool() const
    {
        return m_pInterface != NULL;
    }

    inline bool operator!() const
    {
        return NULL == m_pInterface;
    }

    inline bool IsNull() const
    {
        return NULL == m_pInterface;
    }

    inline bool IsNotNull() const
    {
        return m_pInterface != NULL;
    }


    void Release()
    {
        Unload();
    }

    Ix_Object* DetachInterface()
    {
        Ix_Object* pIF = m_pInterface;
        m_pInterface = NULL;
        return pIF;
    }

    void AttachInterface(Ix_Object* pIF)
    {
        Unload();
        m_pInterface = pIF;
    }

    bool Create(const X3CLSID& clsid)
    {
        Unload();
        return 0 == x3CreateObject(clsid, &m_pInterface);
    }

private:
    void Unload()
    {
        if (m_pInterface)
        {
            m_pInterface->Release(x3GetModuleHandle());
            m_pInterface = NULL;
        }
    }

    void Load(Ix_Object* pIF)
    {
        if (m_pInterface != pIF)
        {
            if (pIF)
            {
                pIF->AddRef(x3GetModuleHandle());
            }
            if (m_pInterface)
            {
                m_pInterface->Release(x3GetModuleHandle());
            }
            m_pInterface = pIF;
        }
    }

private:
    Ix_Object*  m_pInterface;
};

// Inlines for Cx_Interface
//
template <class IF_Type> inline
Cx_Interface<IF_Type>::Cx_Interface(const Cx_Ptr& src) : m_pInterface(NULL), m_pObj(NULL)
{
    if (src.P())
    {
        m_pInterface = dynamic_cast<IF_Type*>(src.P());
        if (m_pInterface)
        {
            m_pObj = src.P();
            m_pObj->AddRef(x3GetModuleHandle());
        }
    }
}

template <class IF_Type> inline
Cx_Interface<IF_Type>& Cx_Interface<IF_Type>::operator=(const Cx_Ptr& src)
{
    Unload();

    if (src.P())
    {
        m_pInterface = dynamic_cast<IF_Type*>(src.P());
        if (m_pInterface)
        {
            m_pObj = src.P();
            m_pObj->AddRef(x3GetModuleHandle());
        }
    }

    return *this;
}

#endif // X3_CORE_XCOMPTR_H_
