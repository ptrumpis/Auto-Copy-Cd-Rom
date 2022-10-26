#ifndef CINIFILE_H
#define CINIFILE_H

#define INIT_BUFFSIZE   256
#define MAX_BUFFSIZE    65536


class CINIFile
{
public:
	CINIFile();
	CINIFile(LPCTSTR lpFileName);
	~CINIFile();

	void SetFileName(LPCTSTR lpFileName);
	BOOL IsSectionPresent(LPCTSTR lpSection) const;

	BOOL GetFirstSectionName(LPTSTR lpBuffer, LPDWORD lpSize);
	BOOL GetNextSectionName(LPTSTR lpBuffer, LPDWORD lpSize);

	BOOL GetFirstSectionKey(LPCTSTR lpSection, LPTSTR lpKey, LPDWORD lpKeyBuffSize, LPTSTR lpValue, LPDWORD lpValueBuffSize);
	BOOL GetNextSectionKey(LPCTSTR lpSection, LPTSTR lpKey, LPDWORD lpKeyBuffSize, LPTSTR lpValue, LPDWORD lpValueBuffSize);

	BOOL ReadValue(LPCTSTR lpSection, LPCTSTR lpKeyName, int* pValue) const;
	BOOL WriteValue(LPCTSTR lpSection, LPCTSTR lpKeyName, int nValue, BOOL bSigned = TRUE) const;
	BOOL ReadString(LPCTSTR lpSection, LPCTSTR lpKeyName, LPTSTR lpBuffer, DWORD dwSize) const;
	BOOL WriteString(LPCTSTR lpSection, LPCTSTR lpKeyName, LPCTSTR lpString) const;
	BOOL ReadStruct(LPCTSTR lpSection, LPCTSTR lpKeyName, LPVOID lpStruct, DWORD dwSize) const;
	BOOL WriteStruct(LPCTSTR lpSection, LPCTSTR lpKeyName, LPVOID lpStruct, DWORD dwSize) const;

private:
	LPTSTR m_lpSectionNames;
	LPTSTR m_lpSectionPointer;
	DWORD m_dwSectionNames;
	LPTSTR m_lpSection;
	LPTSTR m_lpKeyPointer;
	DWORD m_dwSection;
	TCHAR m_lpFileName[MAX_PATH+1];
};

#endif
