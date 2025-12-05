#include "stdafx.h"

_NT_BEGIN

NTSTATUS ReadFromFile(_In_ PCWSTR lpFileName, _Out_ void** ppv, _Out_ ULONG* pcb, _In_ ULONG cbExtra = 0)
{
	UNICODE_STRING ObjectName;

	NTSTATUS status = RtlDosPathNameToNtPathName_U_WithStatus(lpFileName, &ObjectName, 0, 0);

	if (0 <= status)
	{
		HANDLE hFile;
		IO_STATUS_BLOCK iosb;
		OBJECT_ATTRIBUTES oa = { sizeof(oa), 0, &ObjectName, OBJ_CASE_INSENSITIVE };

		status = NtOpenFile(&hFile, FILE_GENERIC_READ, &oa, &iosb,
			FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);

		RtlFreeUnicodeString(&ObjectName);

		if (0 <= status)
		{
			FILE_STANDARD_INFORMATION fsi;

			if (0 <= (status = NtQueryInformationFile(hFile, &iosb, &fsi, sizeof(fsi), FileStandardInformation)))
			{
				if (fsi.EndOfFile.HighPart)
				{
					status = STATUS_FILE_TOO_LARGE;
				}
				else if (fsi.EndOfFile.LowPart)
				{
					if (PVOID pv = LocalAlloc(LMEM_FIXED, fsi.EndOfFile.LowPart + cbExtra))
					{
						if (0 > (status = NtReadFile(hFile, 0, 0, 0, &iosb, pv, fsi.EndOfFile.LowPart, 0, 0)))
						{
							LocalFree(pv);
						}
						else
						{
							*ppv = pv;
							*pcb = (ULONG)iosb.Information;
						}
					}
					else
					{
						status = STATUS_NO_MEMORY;
					}
				}
				else
				{
					status = STATUS_BUFFER_ALL_ZEROS;
				}
			}

			NtClose(hFile);
		}
	}

	return status;
}

_NT_END