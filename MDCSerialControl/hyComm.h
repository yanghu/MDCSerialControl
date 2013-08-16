#pragma once

HANDLE CommInitial(TCHAR *szPortName);

//thread procedures
DWORD WINAPI ReaderThread(LPVOID pParam);
DWORD WINAPI WriterThread(LPVOID pParam);

