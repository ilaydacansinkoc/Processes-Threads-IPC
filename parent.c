/* Name: Ýlayda Cansýn Koç
   ID:	 220201029
*/

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>

#define NUMBER_OF_PROCESSES 7

char PRODUCT_TYPE[][16] = { "MILK","BISCUIT","CHIPS","COKE" };

typedef struct
{
	int day_number;
	int milk_number;
	int biscuit_number;
	int chips_number;
	int coke_number;
} ReceivingData;


int main(int argc, char *argv[])
{
	SECURITY_ATTRIBUTES sa[NUMBER_OF_PROCESSES];
	HANDLE hWritePipeForSending[NUMBER_OF_PROCESSES], hReadPipeForSending[NUMBER_OF_PROCESSES];
	HANDLE hWritePipeForReceiving[NUMBER_OF_PROCESSES], hReadPipeForReceiving[NUMBER_OF_PROCESSES];

	//Creating Pipes.
	for (int i = 0; i < NUMBER_OF_PROCESSES; i++)
	{
		SecureZeroMemory(&sa[i], sizeof(SECURITY_ATTRIBUTES));
		sa[i].bInheritHandle = TRUE;
		sa[i].lpSecurityDescriptor = NULL;
		sa[i].nLength = sizeof(SECURITY_ATTRIBUTES);

		if (!CreatePipe(&hReadPipeForSending[i], &hWritePipeForSending[i], &sa[i], 0) ||
			!CreatePipe(&hReadPipeForReceiving[i], &hWritePipeForReceiving[i], &sa[i], 0))
		{
			printf("unable to create pipe.\n");
			system("pause");
			exit(EXIT_FAILURE);
		}
	}

	STARTUPINFO si[NUMBER_OF_PROCESSES];
	PROCESS_INFORMATION pi[NUMBER_OF_PROCESSES];
	HANDLE process_handles[NUMBER_OF_PROCESSES];

	//Creating Child Processes.
	for (int i = 0; i < NUMBER_OF_PROCESSES; i++)
	{

		SecureZeroMemory(&si[i], sizeof(STARTUPINFO));
		SecureZeroMemory(&pi[i], sizeof(PROCESS_INFORMATION));
		si[i].cb = sizeof(STARTUPINFO);

		//Initialize your Pipe handles here
		si[i].hStdInput = hReadPipeForSending[i];
		si[i].hStdOutput = hWritePipeForReceiving[i];
		si[i].dwFlags = STARTF_USESTDHANDLES;

		// Start the child process. 
		if (!CreateProcess(NULL,   // No module name (use command line)
			"HW1_CHILD.exe",        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			TRUE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si[i],            // Pointer to STARTUPINFO structure
			&pi[i])           // Pointer to PROCESS_INFORMATION structure
			)
		{
			printf("CreateProcess failed (%d).\n", GetLastError());
			system("pause");
			exit(EXIT_FAILURE);
		}

		process_handles[i] = pi[i].hProcess;
	}

	//Writing to Pipe.
	for (int i = 0; i < NUMBER_OF_PROCESSES; i++)
	{
		int dayNumber = i + 1;
		if (!WriteFile(hWritePipeForSending[i], &dayNumber, sizeof(int), NULL, NULL))
		{
			printf("unable to write pipe");
			system("pause");
			exit(EXIT_FAILURE);
		}
	}

	// Wait until child process exits.
	WaitForMultipleObjects(NUMBER_OF_PROCESSES, process_handles, TRUE, INFINITE);

	//Read data that is sent from child processes' pipe.
	ReceivingData receivingData[NUMBER_OF_PROCESSES];
	for (int i = 0; i < NUMBER_OF_PROCESSES; i++)
	{
		if (!ReadFile(hReadPipeForReceiving[i], &receivingData[i], sizeof(ReceivingData), NULL, NULL))
		{
			printf("Error while reading data from pipe.");
			system("pause");
			ExitProcess(EXIT_FAILURE);
		}
	}


	//Parameters for question 1.
	int mostSoldItemIndex = -1;
	int mostSoldItemCount = -1;

	//Parameters for question 2.
	char mostSoldItemForEachDay[7][16];
	int mostSoldItemCountForEachDay[7];

	//Parameters for question 3-4.
	int totalNumOfMilk = 0;
	int totalNumOfBiscuit = 0;
	int totalNumOfChips = 0;
	int totalNumOfCoke = 0;


	for (int i = 0; i < NUMBER_OF_PROCESSES; i++)
	{

		//Question 2 - The most sold item for each day for given four items.
		int productTypeEnum = 1;
		int productCount = receivingData[i].milk_number;

		if (productCount < receivingData[i].biscuit_number)
		{
			productTypeEnum = 2;
			productCount = receivingData[i].biscuit_number;
		}
		if (productCount < receivingData[i].chips_number)
		{
			productTypeEnum = 3;
			productCount = receivingData[i].chips_number;
		}
		if (productCount < receivingData[i].coke_number)
		{
			productTypeEnum = 4;
			productCount = receivingData[i].coke_number;
		}

		sprintf(mostSoldItemForEachDay[i], "%s", PRODUCT_TYPE[productTypeEnum - 1]);
		mostSoldItemCountForEachDay[i] = productCount;
		printf("Day: %d\n", i + 1);
		printf("The most sold item --> Name: %s | Count: %d\n", mostSoldItemForEachDay[i], mostSoldItemCountForEachDay[i]);


		//Question 1 -	The most sold item in seven days for given four items.
		if (mostSoldItemCount < mostSoldItemCountForEachDay[i])
		{
			mostSoldItemCount = mostSoldItemCountForEachDay[i];
			mostSoldItemIndex = i;
		}

		//Question 3 - The total number of each item sold in seven days for given four items.
		totalNumOfMilk += receivingData[i].milk_number;
		totalNumOfBiscuit += receivingData[i].biscuit_number;
		totalNumOfChips += receivingData[i].chips_number;
		totalNumOfCoke += receivingData[i].coke_number;

		printf("-------------------------------\n");
		printf("Total count of MILK    ---> %d | \n", receivingData[i].milk_number);
		printf("Total count of BISCUIT ---> %d | \n", receivingData[i].biscuit_number);
		printf("Total count of CHIPS   ---> %d | \n", receivingData[i].chips_number);
		printf("Total count of COKE    ---> %d | \n", receivingData[i].coke_number);
		printf("-------------------------------\n\n");

	}
	//Question 4 - The total number of each item sold in each day for given four items.
	printf("The most sold item in seven days: %s | Count: %d\n-----------------------------------------\n",
		mostSoldItemForEachDay[mostSoldItemIndex],
		mostSoldItemCountForEachDay[mostSoldItemIndex]);
	printf("Total count of MILK in seven days:    %d | \n", totalNumOfMilk);
	printf("Total count of BISCUIT in seven days: %d | \n", totalNumOfBiscuit);
	printf("Total count of CHIPS in seven days:   %d | \n", totalNumOfChips);
	printf("Total count of COKE in seven days:    %d | \n", totalNumOfCoke);
	printf("-----------------------------------------\n");


	// Close process and thread handles. 
	for (int i = 0; i < NUMBER_OF_PROCESSES; i++)
	{
		CloseHandle(pi[i].hProcess);
		CloseHandle(pi[i].hThread);
	}
	system("pause");
	return EXIT_SUCCESS;
}