/** @file Main.c
 * Display network interfaces transmission and reception speed.
 * @author Adrien RICCIARDI
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//-------------------------------------------------------------------------------------------------
// Private constants and macros
//-------------------------------------------------------------------------------------------------
/** Network interface name (like eth0). */
#define INTERFACE_NAME_LENGTH 16
/** Maximum amount of interfaces to monitor in the same time. */
#define INTERFACES_MAXIMUM_COUNT 32

/** The file to parse to get network statistics. */
#define NETWORK_STATISTICS_FILE_NAME "/proc/net/dev"

/** First "stringification" macro expansion step.
 * @param X The constant value to convert to string.
 */
#define CONVERT_MACRO_TO_VALUE(X) #X

/** Second "stringification" macro expansion step.
 * @param X The constant value to convert to string.
 * @return The constant value converted to string.
 */
#define CONVERT_MACRO_TO_STRING(X) CONVERT_MACRO_TO_VALUE(X)

//-------------------------------------------------------------------------------------------------
// Private types
//-------------------------------------------------------------------------------------------------
/** An interface statistics. */
typedef struct
{
	char String_Interface_Name[INTERFACE_NAME_LENGTH]; //!< The interface name (like eth0, wlan0...).
	unsigned long long Previous_Received_Bytes_Count; //!< How many bytes were received last time the interface was monitored.
	unsigned long long Current_Received_Bytes_Count; //!< Up to date received bytes count value.
	unsigned long long Previous_Transmitted_Bytes_Count; //!< How many bytes were transmitted last time the interface was monitored.
	unsigned long long Current_Transmitted_Bytes_Count; //!< Up to date transmitted bytes count value.
} TInterfaceStatistics;

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** All found interfaces. */
static TInterfaceStatistics Interfaces_Statistics[INTERFACES_MAXIMUM_COUNT];

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Open the network statistics file and extract useful information.
 * @return -1 if an error occurred,
 * @return 0 or a positive number on success. It is the amount of interfaces found.
 */
static inline int GetInterfacesStatistics(void)
{
	static char String_Line[1024];
	FILE *Pointer_File;
	int Return_Value = -1, i, Interface_Name_Length;
	
	// Try to open the file
	Pointer_File = fopen(NETWORK_STATISTICS_FILE_NAME, "r");
	if (Pointer_File == NULL) goto Exit;
	
	// Bypass the beginning two lines (they contain the columns description)
	fgets(String_Line, sizeof(String_Line), Pointer_File);
	fgets(String_Line, sizeof(String_Line), Pointer_File);
	
	// Read all interface lines
	for (i = 0; i < INTERFACES_MAXIMUM_COUNT; i++)
	{
		// Read the next line
		if (fgets(String_Line, sizeof(String_Line), Pointer_File) == NULL) break;
		
		// Extract interface name and values ('*' character tells scanf() to discard a value)
		sscanf(String_Line, "%" CONVERT_MACRO_TO_STRING(INTERFACE_NAME_LENGTH) "s %llu %*u %*u %*u %*u %*u %*u %*u %llu", Interfaces_Statistics[i].String_Interface_Name, &Interfaces_Statistics[i].Current_Received_Bytes_Count, &Interfaces_Statistics[i].Current_Transmitted_Bytes_Count);
		
		// Remove the trailing ':' in the interface name
		Interface_Name_Length = strlen(Interfaces_Statistics[i].String_Interface_Name);
		Interfaces_Statistics[i].String_Interface_Name[Interface_Name_Length - 1] = 0;
	}
	
	// Return amount of interfaces
	Return_Value = i;
	
Exit:
	if (Pointer_File != NULL) fclose(Pointer_File);
	return Return_Value;
}

/** Compute a rate in an human-understandable way.
 * @param Previous_Bytes_Count How many bytes were received or sent by the interface last time (i.e., during the previous sampling loop).
 * @param Current_Bytes_Count How many bytes were received or sent by the interface now.
 * @param Pointer_Rate On output, contain the rate adapted to the appropriate prefix (giga, mega, kilo...).
 * @param Pointer_Multiple_Prefix On output, contain the rate prefix ('G' for giga, 'M' for mega...).
 */
static void ComputeRate(unsigned long long Previous_Bytes_Count, unsigned long long Current_Bytes_Count, float *Pointer_Rate, char *Pointer_Multiple_Prefix)
{
	float Rate;
	
	// Compute the rate in bytes/second
	Rate = Current_Bytes_Count - Previous_Bytes_Count;
	
	// Convert the rate to an human-readable form
	if (Rate >= 1000000000.f)
	{
		Rate /= 1000000000.f;
		*Pointer_Multiple_Prefix = 'G';
	}
	else if (Rate >= 1000000.f)
	{
		Rate /= 1000000.f;
		*Pointer_Multiple_Prefix = 'M';
	}
	else if (Rate >= 1000.f)
	{
		Rate /= 1000.f;
		*Pointer_Multiple_Prefix = 'K';
	}
	else *Pointer_Multiple_Prefix = ' ';
	
	*Pointer_Rate = Rate;
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(void)
{
	int Interfaces_Count, i;
	float Rate;
	char Multiple_Prefix;
	
	// Make sure previous transmission and reception values are zero
	memset(Interfaces_Statistics, 0, sizeof(Interfaces_Statistics));
	
	while (1)
	{
		// Get all interfaces statistics
		Interfaces_Count = GetInterfacesStatistics();
		
		// Did something wrong happen ?
		if (Interfaces_Count < 0)
		{
			printf("An error happened. Some debug might be added one day...\n");
			return EXIT_FAILURE;
		}
		
		// Clear screen
		printf("\033[2J");
		// Move cursor to left upper corner
		printf("\033[H");
		
		printf("Linux netspeed. (C) 2017 Adrien Ricciardi. Hit Ctrl+C to exit.\n");
		
		// Display interfaces statistics
		for (i = 0; i < Interfaces_Count; i++)
		{
			// Display interface rates
			// Interface name
			printf("%s \t:", Interfaces_Statistics[i].String_Interface_Name);
			// Reception rate in bit/s
			ComputeRate(Interfaces_Statistics[i].Previous_Received_Bytes_Count * 8, Interfaces_Statistics[i].Current_Received_Bytes_Count * 8, &Rate, &Multiple_Prefix);
			printf(" RX = %.1f %cbit/s", Rate, Multiple_Prefix);
			// Reception rate in byte/s
			ComputeRate(Interfaces_Statistics[i].Previous_Received_Bytes_Count, Interfaces_Statistics[i].Current_Received_Bytes_Count, &Rate, &Multiple_Prefix);
			printf(" (%.1f %cbyte/s)", Rate, Multiple_Prefix);
			// Transmission rate in bit/s
			ComputeRate(Interfaces_Statistics[i].Previous_Transmitted_Bytes_Count * 8, Interfaces_Statistics[i].Current_Transmitted_Bytes_Count * 8, &Rate, &Multiple_Prefix);
			printf(", TX = %.1f %cbit/s", Rate, Multiple_Prefix);
			// Transmission rate in byte/s
			ComputeRate(Interfaces_Statistics[i].Previous_Transmitted_Bytes_Count, Interfaces_Statistics[i].Current_Transmitted_Bytes_Count, &Rate, &Multiple_Prefix);
			printf(" (%.1f %cbyte/s)\n", Rate, Multiple_Prefix);
			
			// Update previous rate value
			Interfaces_Statistics[i].Previous_Received_Bytes_Count = Interfaces_Statistics[i].Current_Received_Bytes_Count;
			Interfaces_Statistics[i].Previous_Transmitted_Bytes_Count = Interfaces_Statistics[i].Current_Transmitted_Bytes_Count;

		}
		
		// Wait one second
		usleep(1000000);
	}
	
	return EXIT_SUCCESS;
}
