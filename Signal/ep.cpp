#include "stdafx.h"

_NT_BEGIN

#include "print.h"

NTSTATUS ReadFromFile(_In_ PCWSTR lpFileName, _Out_ void** ppv, _Out_ ULONG* pcb, _In_ ULONG cbExtra = 0);

static int __cdecl sort(const void* pa, const void* pb)
{
	LONG a = *(LONG*)pa, b = *(LONG*)pb;
	if (a < b) return -1;
	if (a > b) return +1;
	return 0;
}

ULONG RangeLength(_In_ PLONG Numbers, _In_ ULONG n, _Out_ PLONG pt, _Out_ PLONG pm)
{
	LONG a = *Numbers++ + 1, b;
	--n;

	enum { missing, twice };
	LONG f = (1 << missing) | (1 << twice);
	ULONG len = 1;
	do
	{
		if ((b = *Numbers++) == a)
		{
			continue;
		}

		if (b == a - 1)
		{
			if (!_bittestandreset(&f, twice))
			{
				break;
			}
			*pt = b;
			continue;
		}

		if (b == a + 1)
		{
			if (!_bittestandreset(&f, missing))
			{
				break;
			}
			*pm = a;
			continue;
		}

		break;

	} while (a = b + 1, len++, --n);

	return f ? 0 : len;
}

void PrintArr(_In_ PLONG Numbers, _In_ SIZE_T n)
{
	SIZE_T s = 1 + n * (_countof("4294967295, ") - 1);
	if (PWSTR buf = (PWSTR)_malloca(s * sizeof(wchar_t)))
	{
		PWSTR psz = buf;
		do
		{
			int len = swprintf_s(psz, s, L"%d, ", *Numbers++);
			if (0 >= len)
			{
				break;
			}
			psz += len, s -= len;
		} while (--n);

		PutChars(buf);

		_freea(buf);
	}
}

BOOL Challenge_14(_In_ PLONG Numbers, _In_ ULONG n, _Out_ PULONG ps)
{
	if (n < 3)
	{
		return FALSE;
	}
	qsort(Numbers, n, sizeof(LONG), sort);

	ULONG m = n, k, K = 0;
	LONG S = 0, * pn = 0, * _Numbers = Numbers, mis = 0, twi = 0, mm, tt;
	n -= 2;
	do
	{
		if (K < (k = RangeLength(Numbers, m, &tt, &mm)))
		{
			K = k, S = Numbers[0] + Numbers[k - 1], pn = Numbers, twi = tt, mis = mm;
		}

	} while (Numbers++, --m, --n);

	if (K)
	{
		*ps += S;

		DbgPrint("[ ");
		if (pn != _Numbers)
		{
			PrintArr(_Numbers, pn - _Numbers);
		}

		DbgPrint("( ");
		PrintArr(pn, K), pn += K;
		DbgPrint(") ");

		if (pn != (Numbers += 2))
		{
			PrintArr(pn, Numbers - pn);
		}

		DbgPrint("] : M = %d, D = %d, S = %d\n", mis, twi, S);

		return TRUE;
	}

	return FALSE;
}

void Challenge_14(_In_ PCWSTR lpFileName)
{
	PVOID buf = 0;
	ULONG cb;
	BOOL fOk = FALSE;
	ULONG nArrays = 0;
	NTSTATUS status;
	if (0 <= (status = ReadFromFile(lpFileName, &buf, &cb, 1)))
	{
		if (cb)
		{
			PSTR psz = (PSTR)buf, _psz;
			psz[cb] = 0;
			ULONG N = 0, s = 0;
		__0:
			if (PLONG Numbers = new LONG[N += 64])
			{
				for (;;)
				{
					switch (*psz)
					{
					default:
						psz++;
						continue;
					case 0:
						fOk = TRUE;
						goto __1;
					case '[':
						_psz = psz;
						BOOL bNext = TRUE;

						PLONG pl = Numbers;
						ULONG n = 0;

					__next:
						ULONG v = strtoul(psz + 1, &psz, 10);

						switch (*psz)
						{
						case ']':
							bNext = FALSE;
						case ',':
							if (n++ < N)
							{
								*pl++ = v;
							}
							if (bNext) goto __next;

							if (n++ < N)
							{
								*pl++ = v;
							}
							if (N < n)
							{
								delete[] Numbers;
								N = n;
								psz = _psz;
								goto __0;
							}
							if (!Challenge_14(Numbers, n, &s))
							{
								goto __1;
							}
							nArrays++, psz++;
							continue;

						default:
							goto __1;
						}
					}
				}
			__1:

				delete[] Numbers;

				DbgPrint("ok=%x, arrays=%u, sum=%d\n", fOk, nArrays, s);
			}
		}

		LocalFree(buf);
	}
	else
	{
		DbgPrint("open(\"%ws\")=%x\r\n", lpFileName, status);
		PrintError(status);
	}
}

void Challenge_14()
{
	PrintInfo pi;
	pi.Init();

	if (PWSTR lpFileName = wcschr(GetCommandLineW(), '*'))
	{
		if (PWSTR psz = wcschr(++lpFileName, '*'))
		{
			*psz = 0;
			Challenge_14(lpFileName);
			return;
		}
	}
	Challenge_14(L"Number Sequences");
}

void WINAPI ep(void*)
{
	Challenge_14();
	ExitProcess(0);
}

_NT_END

