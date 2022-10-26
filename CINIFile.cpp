#include "stdafx.h"
#include "CINIFile.h"

#ifndef CINIFILE_CPP
#define CINIFILE_CPP


CINIFile::CINIFile()
{
	m_lpSectionNames = NULL;
	m_lpSectionPointer = NULL;
	m_dwSectionNames = 0;

	m_lpSection = NULL;
	m_lpKeyPointer = NULL;
	m_dwSection = 0;

	m_lpFileName[0] = '\0';
}

CINIFile::CINIFile(LPCTSTR lpFileName)
{
	m_lpSectionNames = NULL;
	m_lpSectionPointer = NULL;
	m_dwSectionNames = 0;

	m_lpSection = NULL;
	m_lpKeyPointer = NULL;
	m_dwSection = 0;

	SetFileName(lpFileName);
}

CINIFile::~CINIFile()
{
	if(m_lpSectionNames)
		delete[] m_lpSectionNames;

	if(m_lpSection)
		delete[] m_lpSection;
}

void CINIFile::SetFileName(LPCTSTR lpFileName)
{
	lstrcpyn(m_lpFileName, lpFileName, MAX_PATH);
}

BOOL CINIFile::IsSectionPresent(LPCTSTR lpSection) const
{
	TCHAR buffer[256];
	buffer[0] = '\0';

	return (GetPrivateProfileString(lpSection, NULL, _T(""), buffer, 256, m_lpFileName) > 0);
}

BOOL CINIFile::GetFirstSectionName(LPTSTR lpBuffer, LPDWORD lpSize)
{
	if(m_lpSectionNames)
	{
		delete[] m_lpSectionNames;
		m_lpSectionNames = NULL;
	}

	m_lpSectionPointer = NULL;


	for(DWORD dwBuffSize=INIT_BUFFSIZE;dwBuffSize<=MAX_BUFFSIZE;)
	{
		m_lpSectionNames = new TCHAR[dwBuffSize];

		m_dwSectionNames = GetPrivateProfileSectionNames(m_lpSectionNames, dwBuffSize, m_lpFileName);
		if(m_dwSectionNames == 0)
		{
			// error
			// no sections?
			break;
		}
		else if(m_dwSectionNames == (dwBuffSize-2))
		{
			// buffer too small
			delete[] m_lpSectionNames;
			m_lpSectionNames = NULL;

			// create bigger one
			dwBuffSize = (dwBuffSize * 2);
			continue;
		}
		else
		{
			DWORD dwLen = lstrlen(m_lpSectionNames);

			if(dwLen+1 > (*lpSize))
			{
				(*lpSize) = dwLen;
				break;
			}

			(*lpSize) = dwLen;

			lstrcpy(lpBuffer, m_lpSectionNames);

			m_lpSectionPointer = m_lpSectionNames + dwLen + 1;

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CINIFile::GetNextSectionName(LPTSTR lpBuffer, LPDWORD lpSize)
{
	if(!m_lpSectionPointer)
		return FALSE;

	DWORD dwLen = lstrlen(m_lpSectionPointer);

	if(dwLen+1 > (*lpSize))
	{
		(*lpSize) = dwLen;
		return FALSE;
	}

	(*lpSize) = dwLen;

	lstrcpy(lpBuffer, m_lpSectionPointer);

	m_lpSectionPointer += (dwLen + 1);

    if((m_lpSectionPointer - m_lpSectionNames) >= m_dwSectionNames)
    {
        m_lpSectionPointer = NULL;
    }

	return TRUE;
}

BOOL CINIFile::GetFirstSectionKey(LPCTSTR lpSection, LPTSTR lpKey, LPDWORD lpKeyBuffSize, LPTSTR lpValue, LPDWORD lpValueBuffSize)
{
	if(m_lpSection)
	{
		delete[] m_lpSection;
		m_lpSection = NULL;
	}

	m_lpKeyPointer = NULL;


	for(DWORD dwBuffSize=INIT_BUFFSIZE;dwBuffSize<=MAX_BUFFSIZE;)
	{
		m_lpSection = new TCHAR[dwBuffSize];

		m_dwSection = GetPrivateProfileSection(lpSection, m_lpSection, dwBuffSize, m_lpFileName);
		if(m_dwSection == 0)
		{
			// error
			// no sections?
			break;
		}
		else if(m_dwSection == (dwBuffSize-2))
		{
			// buffer too small
			delete[] m_lpSection;
			m_lpSection = NULL;

			// create bigger one
			dwBuffSize = (dwBuffSize * 2);
			continue;
		}
		else
		{
			DWORD dwKeyValueLen = lstrlen(m_lpSection);

			for(DWORD i=0;i<dwKeyValueLen;i++)
			{
				if(m_lpSection[i] == '=')
				{
					m_lpSection[i] = '\0';

					LPTSTR lpKeyOffset = m_lpSection;
					DWORD dwKeyLen = lstrlen(lpKeyOffset);
					LPTSTR lpValueOffset = lpKeyOffset + dwKeyLen + 1;
					DWORD dwValueLen = lstrlen(lpValueOffset);

					if(dwKeyLen+1 > (*lpKeyBuffSize) || dwValueLen+1 > (*lpValueBuffSize))
					{
						if(dwKeyLen+1 > (*lpKeyBuffSize))
							(*lpKeyBuffSize) = dwKeyLen+1;

						if(dwValueLen+1 > (*lpValueBuffSize))
							(*lpValueBuffSize) = dwValueLen+1;

						break;
					}

					(*lpKeyBuffSize) = dwKeyLen;
					(*lpValueBuffSize) = dwValueLen;

					lstrcpy(lpKey, lpKeyOffset);
					lstrcpy(lpValue, lpValueOffset);

					m_lpKeyPointer = lpValueOffset + dwValueLen + 1;

					return TRUE;
				}
			}
			break;
		}
	}

	return FALSE;
}

BOOL CINIFile::GetNextSectionKey(LPCTSTR lpSection, LPTSTR lpKey, LPDWORD lpKeyBuffSize, LPTSTR lpValue, LPDWORD lpValueBuffSize)
{
	if(!m_lpKeyPointer)
		return FALSE;

	DWORD dwKeyValueLen = lstrlen(m_lpKeyPointer);

	for(DWORD i=0;i<dwKeyValueLen;i++)
	{
		if(m_lpKeyPointer[i] == '=')
		{
			m_lpKeyPointer[i] = '\0';

			LPTSTR lpKeyOffset = m_lpKeyPointer;
			DWORD dwKeyLen = lstrlen(lpKeyOffset);
			LPTSTR lpValueOffset = lpKeyOffset + dwKeyLen + 1;
			DWORD dwValueLen = lstrlen(lpValueOffset);

			if(dwKeyLen+1 > (*lpKeyBuffSize) || dwValueLen+1 > (*lpValueBuffSize))
			{
				if(dwKeyLen+1 > (*lpKeyBuffSize))
					(*lpKeyBuffSize) = dwKeyLen+1;

				if(dwValueLen+1 > (*lpValueBuffSize))
					(*lpValueBuffSize) = dwValueLen+1;

				break;
			}

			(*lpKeyBuffSize) = dwKeyLen;
			(*lpValueBuffSize) = dwValueLen;

			lstrcpy(lpKey, lpKeyOffset);
			lstrcpy(lpValue, lpValueOffset);

			m_lpKeyPointer = lpValueOffset + dwValueLen + 1;

            if((m_lpKeyPointer - m_lpSection) >= m_dwSection)
            {
                m_lpKeyPointer = NULL;
            }

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CINIFile::ReadValue(LPCTSTR lpSection, LPCTSTR lpKeyName, int* pValue) const
{
	TCHAR buffer[16];
	buffer[0] = '\0';

	GetPrivateProfileString(lpSection, lpKeyName, _T(""), buffer, 16, m_lpFileName);
	if(!lstrlen(buffer))
		return FALSE;

	*pValue = _ttoi(buffer);

	return TRUE;
}

BOOL CINIFile::WriteValue(LPCTSTR lpSection, LPCTSTR lpKeyName, int nValue, BOOL bSigned) const
{
	TCHAR format[3] = { '%', 'd', '\0' };
	TCHAR buffer[16];
	buffer[0] = '\0';

	if(!bSigned)
		format[1] = 'u';

	wsprintf(buffer, format, nValue);

	return WritePrivateProfileString(lpSection, lpKeyName, buffer, m_lpFileName);
}

BOOL CINIFile::ReadString(LPCTSTR lpSection, LPCTSTR lpKeyName, LPTSTR lpBuffer, DWORD dwSize) const
{
	GetPrivateProfileString(lpSection, lpKeyName, _T(""), lpBuffer, dwSize, m_lpFileName);
	if(!lstrlen(lpBuffer))
		return FALSE;

	return TRUE;
}

BOOL CINIFile::WriteString(LPCTSTR lpSection, LPCTSTR lpKeyName, LPCTSTR lpString) const
{
	return WritePrivateProfileString(lpSection, lpKeyName, lpString, m_lpFileName);
}

BOOL CINIFile::ReadStruct(LPCTSTR lpSection, LPCTSTR lpKeyName, LPVOID lpStruct, DWORD dwSize) const
{
	return GetPrivateProfileStruct(lpSection, lpKeyName, lpStruct, dwSize, m_lpFileName);
}

BOOL CINIFile::WriteStruct(LPCTSTR lpSection, LPCTSTR lpKeyName, LPVOID lpStruct, DWORD dwSize) const
{
	return WritePrivateProfileStruct(lpSection, lpKeyName, lpStruct, dwSize, m_lpFileName);
}

#endif
