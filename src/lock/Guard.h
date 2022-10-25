#ifndef __H__GUARD__H__
#define __H__GUARD__H__

typedef enum emLockType
{
	EM_LOCKTYPE_MUTEX = 1,
	EM_LOCKTYPE_RW_RD = 2,
	EM_LOCKTYPE_RW_WR = 3
}emLockType;

template <class LOCK>
class Guard
{
public:
	
	Guard(LOCK& Lock, emLockType emType = EM_LOCKTYPE_MUTEX);
	Guard(LOCK& Lock, bool block, emLockType emType = EM_LOCKTYPE_MUTEX);
	~Guard(void);

	int acquire(void);
	int tryacquire(void);

	int release(void);
	void disown(void);

	bool locked(void) const;
	int remove(void);

protected:
	Guard(LOCK* Lock, emLockType emType = EM_LOCKTYPE_MUTEX): m_Lock(Lock), m_nRntCode(0), m_emType(emType){}

	LOCK* m_Lock;
	int m_nRntCode;
	emLockType m_emType;
};

template <class LOCK>
Guard<LOCK>::Guard(LOCK& Lock, emLockType emType)
:m_Lock(&Lock), m_nRntCode(-1), m_emType(emType)
{
	this->acquire();
}

template <class LOCK>
Guard<LOCK>::Guard(LOCK& Lock, bool block, emLockType emType)
:m_Lock(&Lock), m_nRntCode(-1), m_emType(emType)
{
	if (block)
		this->acquire();
	else
		this->tryacquire();
}

template <class LOCK>
Guard<LOCK>::~Guard(void)
{
	this->release();
}

template <class LOCK>  int
Guard<LOCK>::acquire(void)
{
	this->m_nRntCode = -1;
	if (EM_LOCKTYPE_MUTEX == m_emType)
	{
		this->m_nRntCode = this->m_Lock->acquire();
	}
	else if (EM_LOCKTYPE_RW_RD == m_emType)
	{
		this->m_nRntCode = this->m_Lock->acquire_read();
	}
	else if (EM_LOCKTYPE_RW_WR == m_emType)
	{
		this->m_nRntCode = this->m_Lock->acquire_write();
	}
	
	return this->m_nRntCode;
}

template <class LOCK>  int
Guard<LOCK>::tryacquire(void)
{
	this->m_nRntCode = -1;
	if (EM_LOCKTYPE_MUTEX == m_emType)
	{
		this->m_nRntCode = this->m_Lock->tryacquire();
	}
	else if (EM_LOCKTYPE_RW_RD == m_emType)
	{
		this->m_nRntCode = this->m_Lock->tryacquire_read();
	}
	else if (EM_LOCKTYPE_RW_WR == m_emType)
	{
		this->m_nRntCode = this->m_Lock->tryacquire_write();
	}

	return this->m_nRntCode;
}

template <class LOCK>  int
Guard<LOCK>::release(void)
{
	if (this->m_nRntCode == -1)
		return -1;
	else
	{
		this->m_nRntCode = -1;
		return this->m_Lock->release();
	}
}

template <class LOCK>  bool
Guard<LOCK>::locked(void) const
{
	return this->m_nRntCode != -1;
}

template <class LOCK>  int
Guard<LOCK>::remove(void)
{
	return this->m_Lock->remove();
}

template <class LOCK>  void
Guard<LOCK>::disown(void)
{
	this->m_nRntCode = -1;
}

#endif
