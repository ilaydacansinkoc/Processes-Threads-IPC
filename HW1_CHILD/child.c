/* Name: Ýlayda Cansýn Koç 
   ID:	 220201029
*/
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <string.h>

#define MAX_THREADS 4
#define FILE_NAME "market.txt"
typedef struct
{
	int day_number;
	int milk_number;
	int biscuit_number;
	int chips_number;
	int coke_number;
} SendingData;
SendingData g_Sending_Data;

char* read_file(int dayNumber);
DWORD WINAPI thread_function(LPVOID lpParam);
int count_occurrences(char * str, char * toSearch);
void error_handler(LPTSTR lpszFunction);
char PRODUCT_TYPE[][16] = { "MILK","BISCUIT","CHIPS","COKE" };

char* read_file(int dayNumber)
{
	char* day_data = (char*)malloc(1024 * sizeof(char));
	day_data[0] = '\0';
	char data_line[256];

	char starting_line[32], ending_line[32];
	sprintf(starting_line, "#START DAY %d#", dayNumber);
	sprintf(ending_line, "#END DAY %d#", dayNumber);
	FILE *fptr;

	if ((fptr = fopen(FILE_NAME, "r")) == NULL)
	{
		printf("Error! opening file");
		system("pause");
		exit(EXIT_FAILURE);
	}

	// reads text until newline
	int writeEnable = 0;
	while (fscanf(fptr, "%[^\n]\n", data_line) != EOF)
	{
		if (strcmp(ending_line, data_line) == 0)
			break;
		if (strcmp(starting_line, data_line) == 0)
			writeEnable = 1;

		if (writeEnable)
			sprintf(day_data, "%s%s\n", day_data, data_line);
	}
	fclose(fptr);
	return day_data;
}

int main(int arg, char *argv[])
{
	SecureZeroMemory(&g_Sending_Data, sizeof(SendingData));

	HANDLE hStdin, hStdout;
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	hStdin = GetStdHandle(STD_INPUT_HANDLE);

	if ((hStdout == INVALID_HANDLE_VALUE) || (hStdin == INVALID_HANDLE_VALUE))
		ExitProcess(EXIT_FAILURE);

	if (!ReadFile(hStdin, &g_Sending_Data.day_number, sizeof(int), NULL, NULL))
	{
		printf("Error while reading data from pipe.");
		system("pause");
		ExitProcess(EXIT_FAILURE);
	}

	DWORD   dwThreadIdArray[MAX_THREADS];
	HANDLE  hThreadArray[MAX_THREADS];

	for (int i = 0; i < MAX_THREADS; i++) {
		hThreadArray[i] = CreateThread(
			NULL,						// default security attributes
			0,							// use default stack size  
			thread_function,				// thread function name
			&PRODUCT_TYPE[i],			// argument to thread function 
			0,							// use default creation flags 
			&dwThreadIdArray[i]);		// returns the thread identifier 
		if (hThreadArray[i] == NULL)	// Check the return value for success.
		{
			error_handler(TEXT("CreateThread"));		// If CreateThread fails, terminate execution. 
			ExitProcess(EXIT_FAILURE);				// This will automatically clean up threads and memory. 
		}
	}

	// Wait until all threads have terminated.
	WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);

	if (!WriteFile(hStdout, &g_Sending_Data, sizeof(SendingData), NULL, NULL))
	{
		printf("unable to write pipe");
		system("pause");
		exit(EXIT_FAILURE);
	}

	// Close all thread handles and free memory allocations.
	for (int i = 0; i<MAX_THREADS; i++)
		CloseHandle(hThreadArray[i]);
	return 0;
}


DWORD WINAPI thread_function(LPVOID lpParam)
{
	char* productType = (char*)lpParam;
	char toBeSearchedPattern[16];
	sprintf(toBeSearchedPattern, "%s", productType);
	int productCount = count_occurrences(read_file(g_Sending_Data.day_number), toBeSearchedPattern);

	if ( (strcmp(productType, PRODUCT_TYPE[0]) == 0) )
		g_Sending_Data.milk_number = productCount;
	else if (strcmp(productType, PRODUCT_TYPE[1]) == 0)
		g_Sending_Data.biscuit_number = productCount;
	else if (strcmp(productType, PRODUCT_TYPE[2]) == 0)
		g_Sending_Data.chips_number = productCount;
	else if (strcmp(productType, PRODUCT_TYPE[3]) == 0)
		g_Sending_Data.coke_number = productCount;

	return EXIT_SUCCESS;
}
int count_occurrences(char * str, char * toSearch)
{
	int i, j, found, count;
	int stringLen, searchLen;
	stringLen = strlen(str);      // length of string
	searchLen = strlen(toSearch); // length of word to be searched

	count = 0;

	for (i = 0; i <= stringLen - searchLen; i++)
	{
		found = 1;
		for (j = 0; j<searchLen; j++)
		{
			if (str[i + j] != toSearch[j])
			{
				found = 0;
				break;
			}
		}

		if (found == 1)
			count++;
	}

	return count;
	return 0;
}
void error_handler(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code.

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message.

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	// Free error-handling buffer allocations.

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}