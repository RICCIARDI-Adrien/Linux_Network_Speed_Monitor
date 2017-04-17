/** @file Main.c
 * Display network interfaces transmission and reception speed.
 * @author Adrien RICCIARDI
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Network interface name (like eth0). */
#define INTERFACE_NAME_LENGTH 16
/** Maximum amount of interfaces to monitor in the same time. */
#define INTERFACES_MAXIMUM_COUNT 32

/** The file to parse to get network statistics. */
#define NETWORK_STATISTICS_FILE_NAME "/proc/net/dev"

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
		// TODO add a limit for the interface name string
		sscanf(String_Line, "%s %llu %*u %*u %*u %*u %*u %*u %*u %llu", Interfaces_Statistics[i].String_Interface_Name, &Interfaces_Statistics[i].Current_Received_Bytes_Count, &Interfaces_Statistics[i].Current_Transmitted_Bytes_Count);
		
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

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(void)
{
	int Interfaces_Count, i;
	float Reception_Rate, Transmission_Rate;
	
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
			
		// Display interfaces statistics
		for (i = 0; i < Interfaces_Count; i++)
		{
			// Compute rates
			Reception_Rate = Interfaces_Statistics[i].Current_Received_Bytes_Count - Interfaces_Statistics[i].Previous_Received_Bytes_Count;
			Transmission_Rate = Interfaces_Statistics[i].Current_Transmitted_Bytes_Count - Interfaces_Statistics[i].Previous_Transmitted_Bytes_Count;
			
			// Update previous rate value
			Interfaces_Statistics[i].Previous_Received_Bytes_Count = Interfaces_Statistics[i].Current_Received_Bytes_Count;
			Interfaces_Statistics[i].Previous_Transmitted_Bytes_Count = Interfaces_Statistics[i].Current_Transmitted_Bytes_Count;
			
			printf("%s \t: RX = %.1f Kbit/s (%.1f Kbyte/s), TX = %.1f Kbit/s (%.1f Kbyte/s)\n", Interfaces_Statistics[i].String_Interface_Name, Reception_Rate * 8.f / 1000.f, Reception_Rate / 1000.f, Transmission_Rate * 8.f / 1000.f, Transmission_Rate / 1000.f);
		}
		printf("Hit Ctrl+C to exit.\n");
		
		// Wait one second
		usleep(1000000);
	}
	
	return EXIT_SUCCESS;
}
