#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_USERNAME 80        // Max number of characters for each username in text file
#define MAX_PASSWORD 80        // Max number of characters for each password in text file
#define MAX_NUMBER_OF_CHAR 500 // Max number of characters for lines
#define MAX_NAME 80            // Max number of characters for names
#define MAX_EMAIL 80           // Max number of characters for emails

typedef enum
{
    January = 1,
    February,
    March,
    April,
    May,
    June,
    July,
    August,
    September,
    October,
    November,
    December
} allMonths;

typedef struct
{
    int Day;
    allMonths Month;
    int Year;
} Date;

typedef struct
{
    char account_number[11];
    char name[MAX_NAME];
    char email[MAX_EMAIL];
    char mobile[12];
    float balance;
    Date dateOpened;
} account;

typedef struct
{
    int acc_no;
    char type[25]; // The maximum string would have 25 characters
    float amount;
    Date date;
} Transaction;

// Global Variables
int login_successful;     // When login is successfull, the flag is set to one
account *global_accounts; // Pointer to struct initialized by the load function
int accounts_counter;     // Used to count how many accounts are there
time_t time_now;          // A variable of time_type
struct tm *ptr_to_time;   // Pointer to tm struct to hold the address of the time variable
char date_opened[30];     // String to store the appropriate date format

void menu(void);
void menuModify(void);
void login(char username[], char password[]);
void load(void);
void save(void);
void getCurrentTime(void);

// Sorting functions
int compareByName(const void *a, const void *b);
int compareByDate(const void *a, const void *b);
int compareByBalance(const void *a, const void *b);
void sortByName(void);
void sortByDate(void);
void sortByBalance(void);
void printAccounts(void);
void printAccount(int index);

const char *getMonthName(allMonths month);

// Search functions
void queryAccount(char *account_number);
void advancedSearch(char *search_keyword);
account *searchForAccount(char account_number[]);

// Validation functions
int isFloat(const char *str);
int isNumeric(const char *str);
int mobileNumberIsValid(char mobile_number[]);
int emailIsValid(char email[]);
int accountNumberIsValid(char account_number[]);
int accountNumberExists(const char *fileAddress, char account_number[]);
int nameIsValid(char name[]);

// Changes occur in the text files, then reload again the data into structs
void add(char account_number[], char name[], char email[], float balance, char mobileNumber[]);
void delete(char account_number[]);
void modify(char account_number[], char name[], char email[], float balance, char mobile_number[]);

// Changes occur in the program structs, then save changes in the text files
void withdraw(char account_number[], float amount_to_withdraw);
void deposit(char account_number[], float amount_to_deposit);
void transfer(char from_account_number[], char to_account_number[], float amount_to_transfer);

int confirm();
void validateFile(FILE *fptr);

void report(char account_number[]);

int main()
{
    menu();
    return 0;
}

void getCurrentTime(void)
{
    // gets the current time as a time_t value
    time(&time_now);

    // sets the time structure to the current local time
    ptr_to_time = localtime(&time_now);
}

// strcmp compares between accounts names
int compareByName(const void *a, const void *b)
{
    return strcmp(((account *)a)->name, ((account *)b)->name);
}

// memcmp compares between accounts date and return integer based on comparision
int compareByDate(const void *a, const void *b)
{
    // return memcmp(&((account *)a)->dateOpened, &((account *)b)->dateOpened, sizeof(Date));
    if (((account *)a)->dateOpened.Year > ((account *)b)->dateOpened.Year)
    {
        return 1;
    }
    else if (((account *)a)->dateOpened.Year == ((account *)b)->dateOpened.Year)
    {
        if (((account *)a)->dateOpened.Month > ((account *)b)->dateOpened.Month)
        {
            return 1;
        }
        else if (((account *)a)->dateOpened.Month == ((account *)b)->dateOpened.Month)
        {
            return 0;
        }
        else
            return -1;
    }
    else
        return -1;
}

// Function Compares Balance of accounts
// Ternary operation returns -1,0,1
int compareByBalance(const void *a, const void *b)
{
    return (((account *)a)->balance > ((account *)b)->balance) - (((account *)a)->balance < ((account *)b)->balance);
}

// Sort function uses return from strcmp and qsort sorts them
void sortByName(void)
{
    qsort(global_accounts, accounts_counter, sizeof(account), compareByName);
}

// Sort function uses return from memcmp and qsort sorts them
void sortByDate(void)
{
    qsort(global_accounts, accounts_counter, sizeof(account), compareByDate);
}

// Sort function uses return from ternary operator and qsort sorts them
void sortByBalance(void)
{
    qsort(global_accounts, accounts_counter, sizeof(account), compareByBalance);
}

// Convert month enumeration to a string with month's name
const char *getMonthName(allMonths month)
{
    switch (month)
    {
    case January:
        return "January";
    case February:
        return "February";
    case March:
        return "March";
    case April:
        return "April";
    case May:
        return "May";
    case June:
        return "June";
    case July:
        return "July";
    case August:
        return "August";
    case September:
        return "September";
    case October:
        return "October";
    case November:
        return "November";
    case December:
        return "December";
    default:
        return "Invalid Month";
    }
}

void validateFile(FILE *fptr)
{
    if (fptr == NULL)
    {
        printf("Error opening file");
        exit(1); // Exits the whole program
    }
}

int confirm()
{
    // getchar();
    printf("Confirm? (Y/N): ");
    char c;
    while (1)
    {
        // The function will keep asking for input until the user presses Y or N (lowercase included)
        scanf("%c", &c);
        if (c == 'y' || c == 'Y')
        {
            return 1;
        }
        if (c == 'n' || c == 'N')
        {
            return 0;
        }
        printf("Invalid option, try again: ");
        getchar();
    }
}

void load(void)
{
    FILE *file = fopen("accounts.txt", "r");

    validateFile(file);

    // Count the number of accounts
    accounts_counter = 0;

    account temp_acc;

    while (fscanf(file, "%10[^,],%79[^,],%79[^,],%f,%14[^,],%d-%d\n",
                  temp_acc.account_number,
                  temp_acc.name,
                  temp_acc.email,
                  &temp_acc.balance,
                  temp_acc.mobile,
                  &temp_acc.dateOpened.Month,
                  &temp_acc.dateOpened.Year) == 7) // End of while condition
    {
        (accounts_counter)++;
        // printf("%d",accounts_counter);
    }

    // Allocates memory dynamically, reserving the right amount of structs
    // In case of reloading, the pointer is freed, then we reallocate the right amount
    free(global_accounts);
    global_accounts = (account *)malloc(accounts_counter * sizeof(account));

    // Counts number of accounts in the text file by using a while loop that continues as long as fscanf successfully read the specified pattern
    // A while loop that uses the pattern used in the file to count the accoutns, when it matches 7 elements on every line it counts 1

    rewind(file);
    char *token;
    char *month_string; // Pointer to tokenize each line
    char line[500];     // String to store the line extracted from file
    int i;              // A variable to count till the balance token (fourth token)
    int j = 0;

    // Starts a while loop to find the input number knowing it already
    // exists and is valid from calling the functions in the menu

    // Looping for each line
    while (fgets(line, sizeof(line), file) != NULL)
    {

        // strcpy(temp_line, line);
        // token = temp_line;
        token = strtok(line, ",");
        i = 0;
        while (token != NULL)
        {
            switch (i)
            {
            case 0:

                strcpy(global_accounts[j].account_number, token);
                global_accounts[j].account_number[10] = '\0';

                break;
            case 1:

                strcpy(global_accounts[j].name, token);

                break;
            case 2:

                strcpy(global_accounts[j].email, token);

                break;
            case 3:

                global_accounts[j].balance = atof(token);

                break;
            case 4:

                strcpy(global_accounts[j].mobile, token);

                break;
            case 5:

                month_string = token;
                token = strtok(month_string, "-");
                global_accounts[j].dateOpened.Month = (int)atof(token);
                token = strtok(NULL, "-");
                global_accounts[j].dateOpened.Year = (int)atof(token);

                break;
            }
            token = strtok(NULL, ",");
            i++;
        }
        j++;
    }

    fclose(file);
}

int accountNumberExists(const char *fileAddress, char account_number[])
{
    // Check if the value is found in the "accounts.txt" file
    int found = 0;

    // Declare a pointer as a token to separate the data
    char *token;

    // Declare a pointer to the file
    FILE *fileToRead = fopen(fileAddress, "r");
    validateFile(fileToRead);

    // Set a maximum value for the lines based on the text file
    char line[MAX_NUMBER_OF_CHAR];

    // Start a loop to iterate over each line of data until
    // fgets detecting the end of file thus the token points to NULL
    while (fgets(line, sizeof(line), fileToRead) != NULL)
    {
        token = strtok(line, ",");
        // compares the input value and the prexisting values in the file
        if (strcmp(account_number, token) == 0 && token != NULL)
        {
            // sets the flag to 1 to indicate the account number exists
            found = 1;
            break;
        }
    }
    // Close the file and returns the value of the flag
    fclose(fileToRead);
    return found;
}

int isNumeric(const char *str)
{ // Checks if the passed string is numeric
    while (*str)
    {
        if (!isdigit(*str))
        {
            return 0;
        }
        str++;
    }
    return 1;
}

int isFloat(const char *str)
{
    int dot_count = 0;
    while (*str)
    {
        if (*str == '.')
        {
            dot_count++;
            if (dot_count > 1)
            {
                return 0;
            }
        }
        if (!isdigit(*str) && (*str) != '.')
        {
            return 0;
        }
        str++;
    }
    return 1;
}

int accountNumberIsValid(char account_number[])
{
    // Check if the input account_number follows the specifications
    if (strlen(account_number) == 10 && strncmp(account_number, "9", 1) == 0 && isNumeric(account_number))
        return 1;
    else
    {
        printf("Error: Account number is not valid.\n");
        return 0;
    }
}

int nameIsValid(char name[])
{
    if (!isupper(name[0]))
    {
        printf("Error: Name must begin with upper case\n");
        return 0;
    }
    int i = 0;
    while (name[i] != '\0')
    {
        // If it's not a letter and not a space, then it's an invalid character
        if (!isalpha(name[i]) && name[i] != ' ')
        {
            printf("Error: Name must only contain letters\n");
            return 0;
        }
        i++;
    }
    return 1;
}

int emailIsValid(char email[])
{
    // Check if the input email follows the specifications

    // Initialize a variable to iterate over the string
    unsigned int i = 0;

    // initialize variables to count the number of '@'s and '.'s
    int atCount = 0;
    int dotCount = 0;

    // Flag to check for an existing dot after @
    int dot_after_at = 0;

    // Index
    unsigned int at_index;

    if (email[0] == '.' || email[strlen(email)] == '.')
    {
        return 0;
    }

    while (i < strlen(email))
    {
        if (email[i] == '@')
        {
            atCount++;
            if (email[i + 1] == '.') // if there's a dot right after the @ returns 0
            {
                return 0;
            }
            at_index = i; // Locating the index of @
        }

        if (email[i] == '.')
        {
            dotCount++;
            if (i > at_index) // A dot exists after the @ and not right after it
            {
                dot_after_at = 1;
            }
            if (email[i + 1] == '.') // if there's a 2 consecutive dots returns 0
            {
                return 0;
            }
        }

        i++;

        if (email[i] == ' ' || email[0] == '@')
        {
            return 0;
        }
    }

    if (atCount == 1 && dotCount >= 1 && dot_after_at == 1)
    {
        return 1;
    }
    return 0;
}

int mobileNumberIsValid(char mobile_number[])
{
    // Check if the input mobileNumber follow the specifications
    // Return 1 if valid and 0 if not
    if (strlen(mobile_number) == 11 && strncmp(mobile_number, "01", 2) == 0 && isNumeric(mobile_number))
        return 1;
    else
    {
        printf("Error: Mobile number is invalid.\n");
        return 0;
    }
}

account *searchForAccount(char account_number[])
{
    int i; // index to search for the account number

    // Call the function accountNumberExists, if 0 display an error message and
    // exit the function
    if (!accountNumberExists("accounts.txt", account_number))
    {
        printf("Error: account number doesn't exist.\n\n");
        return NULL;
    }

    // Loop over the array of loaded structs
    for (i = 0; i < accounts_counter; i++)
    {
        if (strcmp(account_number, (global_accounts + i)->account_number) == 0) // Found a match
        {
            return global_accounts + i; // returns a pointer to the found struct
        }
    }

    // In case of errors
    return NULL;
}

void queryAccount(char *account_number)
{
    for (int i = 0; i < accounts_counter; i++)
    {
        // For loop that compares account number with every loaded account
        if (strcmp(global_accounts[i].account_number, account_number) == 0)
        {
            // Display matched account's details
            printAccount(i);
            return;
        }
    }
}

void advancedSearch(char *search_keyword)
{
    int match_found = 0;
    printf("\nSearch results:\n\n");
    for (int i = 0; i < accounts_counter; i++)
    {
        if (strstr(global_accounts[i].name, search_keyword) != NULL) // Needle in the haystack method (?)
        {
            printAccount(i);
            match_found = 1;
        }
    }

    if (!match_found)
    {
        printf("No matches found for the keyword: %s\n", search_keyword);
    }
}

void add(char account_number[], char name[], char email[], float balance, char mobileNumber[])
{
    // call the function accountNumberIsValid() and
    // accountNumberExists in menu() before sending the parameters to the function
    // if exists print error message

    // takes the address of "accounts.txt" file and appends
    FILE *file_to_read = fopen("accounts.txt", "a");
    validateFile(file_to_read);

    // stores the input data in variables

    char tempAccNo[10];
    strcpy(tempAccNo, account_number);
    char tempName[80];
    strcpy(tempName, name);
    char tempEmail[100];
    strcpy(tempEmail, email);
    char tempMobileNo[11];
    strcpy(tempMobileNo, mobileNumber);

    // calls strftime() to set the time to month-year format
    strftime(date_opened, sizeof(date_opened), "%m-%Y", ptr_to_time);

    // calls the confirm function in a flag
    int confirm_flag = confirm();
    // if 1 print the data in the file else, exit the function without appending

    if (confirm_flag)
    {
        fprintf(file_to_read, "%s,", tempAccNo);
        fprintf(file_to_read, "%s,", tempName);
        fprintf(file_to_read, "%s,", tempEmail);
        fprintf(file_to_read, "%.2f,", balance);
        fprintf(file_to_read, "%s,", tempMobileNo);
        fprintf(file_to_read, "%s\n", date_opened);

        char temp_acc_num[10];
        strcpy(temp_acc_num, account_number);

        // creates a file named with the account number
        strcat(temp_acc_num, ".txt");

        FILE *account_number_file = fopen(temp_acc_num, "w");
        if (account_number_file == NULL)
        {
            fprintf(stderr, "Error creating file '%s'\n", temp_acc_num);
            return;
        }

        fclose(account_number_file);

        printf("Account added successfully.\n");
    }

    // closes the accounts.txt
    fclose(file_to_read);
    load();
}

void modify(char account_number[], char name[], char email[], float balance, char mobile_number[])
{
    // Call the function accountNumberIsValid(),
    // accountNumberExists ,emailIsValid() and mobileNumberIsValid()
    // in menu() before sending the parameters to the function
    // if any the value of any of them is 0 the parameter shouldn't change

    // Take the address of "accounts.txt" file and read it
    FILE *file_to_read = fopen("accounts.txt", "r");
    validateFile(file_to_read);

    // Create a new file and copy ALL the data including the modifications
    FILE *modified_file = fopen("modified_file.txt", "w");
    validateFile(modified_file);

    char line[MAX_NUMBER_OF_CHAR];      // String to store the line extracted from file
    char temp_line[MAX_NUMBER_OF_CHAR]; // Temporary line as the one above
    char *token;                        // Pointer to each token
    int i;                              // To iterate over each token per line

    // Looping for each line
    while (fgets(line, sizeof(line), file_to_read) != NULL)
    {
        strcpy(temp_line, line);
        token = strtok(line, ",");

        if (strcmp(account_number, token) == 0) // When finding matching account number
        {
            fprintf(modified_file, "%s,", token); // Print account number before entering the loop
            i = 0;
            token = strtok(NULL, ",");

            // Print each detail after tokenizing the string delimited by ","
            while (token != NULL)
            {
                switch (i)
                {
                case 0: // Convert to if ( i == 0 && strcmp())
                    fprintf(modified_file, "%s,", name);
                    break;
                case 1:
                    fprintf(modified_file, "%s,", email);
                    break;
                case 2:
                    fprintf(modified_file, "%.2f,", balance);
                    break;
                case 3:
                    fprintf(modified_file, "%s,", mobile_number);
                    break;
                case 4:
                    fprintf(modified_file, "%s", token); // Print date at the end
                    break;
                default:
                    break;
                }
                token = strtok(NULL, ",");
                i++;
            }
        }
        else
        {
            fprintf(modified_file, "%s", temp_line);
        }
    }
    fclose(file_to_read);
    fclose(modified_file);
    // Calls the save function, if 1:
    // Remove the old file
    // Rename the new file (give it the old file's name)

    if (confirm())
    {
        remove("accounts.txt");

        // Print an error message if renaming the file failed
        if (rename("modified_file.txt", "accounts.txt") != 0)
        {
            printf("Error renaming the file.\n");
            exit(1);
        }
        else
        {
            printf("Account modified successfully.\n");
        }
    }
    // Else exit function without deleting
    // Remove the created file
    else
    {
        remove("modified_file.txt");
    }
    load();
}

void delete(char account_number[]) // Should it also remove the 9210124543.txt file????? Yess I forgot
{
    // Call the function accountNumberIsValid() and
    // accountNumberExists in menu() before sending the parameters to the function
    // If does not exist or invalid, exit()

    // Takes the path of "accounts.txt" file and reads
    FILE *file_to_read = fopen("accounts.txt", "r");
    validateFile(file_to_read);

    // creates a new file and copies the data excluding the data to be deleted
    FILE *deleted_from_file = fopen("deleted_from_file.txt", "w");
    validateFile(deleted_from_file);

    char *all_data_token;               // Pointer to tokenize each line
    char *balance_token;                // Pointer to point on the balance
    char line[MAX_NUMBER_OF_CHAR];      // String to store the line extracted from file
    char temp_line[MAX_NUMBER_OF_CHAR]; // Temporary line as the one above
    int i;                              // A variable to count till the balance token (fourth token)

    // Starts a while loop to find the input number knowing it already
    // exists and is valid from calling the functions in the menu

    // Looping for each line
    while (fgets(line, sizeof(line), file_to_read) != NULL)
    {
        strcpy(temp_line, line);
        all_data_token = strtok(line, ",");

        if (strcmp(account_number, all_data_token) == 0) // When finding matching account number
        {
            // To reach 4th token which is balance
            i = 0;
            while (i < 4)
            {
                // After the 3rd \0 the pointer should be pointing to the account number
                // So we need to save the pointer first to the balance_token pointer
                // Then the next \0 will be right after the balance in the line, ready to be extracted
                if (i == 3)
                {
                    balance_token = all_data_token;
                }
                all_data_token = strtok(NULL, ",");

                i++;
            }

            double balance = atof(balance_token);

            if (balance != 0)
            {
                // Check that the account balance is not zero to display an error message
                printf("Error: Cannot be deleted. The balance associated with the account number must be 0.00\n");
                fclose(file_to_read);
                fclose(deleted_from_file);
                remove("deleted_from_file.txt");
                return;
            }
        }
        else
        {
            fprintf(deleted_from_file, "%s", temp_line);
        }
    }

    fclose(file_to_read);
    fclose(deleted_from_file);

    // Calls the confirm function, if 1:
    // Remove the old file
    // Rename the new file (give it the old file's name)
    if (confirm())
    {
        remove("accounts.txt");
        strcat(account_number, ".txt");
        remove(account_number);
        // print an error message if renaming the file failed
        if (rename("deleted_from_file.txt", "accounts.txt") != 0)
        {
            printf("Error renaming the file.\n");
            exit(1);
        }
        else
        {
            printf("Account deleted successfully.\n");
        }
    }
    // Exit function without deleting
    // Remove the created file
    else
    {
        remove("deleted_from_file.txt");
    }
    load();
}

void withdraw(char account_number[], float amount_to_withdraw)
{
    account *acc = searchForAccount(account_number);
    // Calls the searchForAccount function, acquiring the pointer to the desired struct of our account

    // Allocate 14 bytes of memory for the account number (10) and the extension .txt (4) string
    // Then append the .txt extension to the account number string
    char *account_file_string = malloc(15 * sizeof(char));
    strcpy(account_file_string, account_number);
    strcat(account_file_string, ".txt");

    if (acc == NULL)
    {
        // Display error message
        printf("Error retrieving data.\n");
        return;
    }

    FILE *account_file = fopen(account_file_string, "a");
    validateFile(account_file);

    // Check for value not exceeding the limit
    if (amount_to_withdraw <= 10000)
    {
        // Check if there's enough balance to withdraw
        if (acc->balance >= amount_to_withdraw)
        {
            if (confirm())
            {
                // Update the balance after withdrawal
                acc->balance -= amount_to_withdraw;

                printf("Amount withdrawn successfully\n");

                strftime(date_opened, sizeof(date_opened), "%d/%m/%Y", ptr_to_time);

                // Appends the .txt file with the transaction
                fprintf(account_file, "%s %.2f Withdrawn\n", date_opened, amount_to_withdraw);

                // Confirm the new file and save changes
                save();
            }
            else
            {
                printf("Withdrawal canceled\n");
            }
        }
        else
        {
            printf("Insufficient funds\n");
        }
    }
    else
    {
        printf("Amount exceeds withdrawal limit\n");
    }

    printf("\n");

    free(account_file_string);
    fclose(account_file);
}

void deposit(char account_number[], float amount_to_deposit)
{
    // Calls the searchForAccount function, acquiring the pointer to the desired struct of our account
    account *acc = searchForAccount(account_number);

    // Allocate 14 bytes of memory for the account number (10) and the extension .txt (4) string
    // Then append the .txt extension to the account number string

    char *account_file_string = malloc(15 * sizeof(char));
    strcpy(account_file_string, account_number);
    strcat(account_file_string, ".txt");

    if (acc == NULL)
    {
        // Display error message
        printf("Error retrieving data");
        return;
    }

    FILE *account_file = fopen(account_file_string, "a");

    // Check for value not exceeding the limit
    if (amount_to_deposit <= 10000)
    {
        if (confirm())
        {
            // Update the balance after deposit
            acc->balance += amount_to_deposit;
            printf("Amount deposited successfully\n");

            strftime(date_opened, sizeof(date_opened), "%d/%m/%Y", ptr_to_time);

            // Appends the .txt file with the transaction
            fprintf(account_file, "%s %.2f Deposited\n", date_opened, amount_to_deposit);

            save(); // Confirm the new file and save changes
        }
        else
        {
            printf("Deposit canceled\n");
        }
    }
    else
    {
        printf("Amount exceeds deposit limit\n");
    }

    printf("\n");

    free(account_file_string);
    fclose(account_file);
}

void transfer(char from_account_number[], char to_account_number[], float amount_to_transfer)
{
    // Calls the searchForAccount function twice for both accounts
    account *from_acc = searchForAccount(from_account_number);
    account *to_acc = searchForAccount(to_account_number);

    // Allocate 14 bytes of memory for the account number (10) and the extension .txt (4) string
    // Then append the .txt extension to the account number string

    char *from_account_file_string = malloc(14 * sizeof(char));
    from_account_file_string = strcat(from_account_number, ".txt");

    // Repeat the same for the other account number
    char *to_account_file_string = malloc(14 * sizeof(char));
    to_account_file_string = strcat(to_account_number, ".txt");

    if (from_acc == NULL || to_acc == NULL)
    {
        // Display error message
        printf("Error retrieving data");
        return;
    }

    // Opening account files
    FILE *from_account_file = fopen(from_account_file_string, "a");
    FILE *to_account_file = fopen(to_account_file_string, "a");

    // Subtract the value entered from the balance of the first account using a pointer
    // to struct, adding the same value to the other account
    if (from_acc->balance >= amount_to_transfer)
    {
        if (confirm())
        {
            // Update the balance after transfer
            from_acc->balance -= amount_to_transfer;
            to_acc->balance += amount_to_transfer;

            printf("Amount transferred successfully\n");

            strftime(date_opened, sizeof(date_opened), "%d/%m/%Y", ptr_to_time);

            // Appends the .txt file with the transaction details
            fprintf(from_account_file, "%s %.2f Transfered\n", date_opened, amount_to_transfer);
            fprintf(to_account_file, "%s %.2f Received", date_opened, amount_to_transfer);

            save(); // Confirm the new file and save changes
        }
        else
        {
            printf("Transfer Canceled\n");
        }
    }
    else
    {
        // Display error message and close files
        printf("Error: insufficient balance to transfer.\n\n");
    }

    printf("\n");

    fclose(from_account_file);
    fclose(to_account_file);
}

void printAccounts(void)
{
    printf("\nAccount Details:\n\n");
    for (int i = 0; i < accounts_counter; i++)
    {
        // printf("Account %d:\n", i + 1);
        printAccount(i);
    }

    // exit the function
}

void printAccount(int index)
{
    printf("\nAccount Number: %s\n", global_accounts[index].account_number);
    printf("Name: %s\n", global_accounts[index].name);
    printf("Email: %s\n", global_accounts[index].email);
    printf("Balance: $%.2f\n", global_accounts[index].balance);
    printf("Mobile: %s\n", global_accounts[index].mobile);
    char temp_month[20];
    strcpy(temp_month, getMonthName(global_accounts[index].dateOpened.Month));
    printf("Date Opened: %s %d\n", temp_month, global_accounts[index].dateOpened.Year);
    printf("\n");
}

void login(char username[], char password[])
{
    char line[MAX_USERNAME + MAX_PASSWORD + 1]; // To store the line and make the pointer move to the next file
    char text_username[MAX_USERNAME];           // Username extracted from text file
    char text_password[MAX_PASSWORD];           // Password extracted from text file
    int found = 0;                              // Flag when there's a matching username

    FILE *users_file = fopen("users.txt", "r");

    validateFile(users_file);

    // Use a "do while" loop so that the condition makes it move to the following line
    do
    {
        fscanf(users_file, "%s", text_username); // Scans the first string in the line

        // printf("Comparing %s\n", text_userna me); // Used for debugging

        if (strcmp(username, text_username) == 0) // Found a match
        {
            found = 1;

            // Now time to check the password right next to the username, so fscanf will scan it
            fscanf(users_file, "%s", text_password);

            if (strcmp(password, text_password) == 0)
            {
                // login successful, setting the login flag to 1
                printf("Logged in successfully!\n\n");
                login_successful = 1;
            }
            else
            {
                printf("Wrong password.\n\n");
            }
            fclose(users_file);
            return; // Exiting function
        }

    } while (fgets(line, sizeof(line), users_file) != NULL); // Moves the pointer to the next line until it reaches NULL

    if (!found)
    {
        printf("Username doesn't exist\n\n");
    }
    fclose(users_file);
}

void save(void)
{
    // Overwrite the accounts.txt with the updated version
    FILE *file = fopen("accounts.txt", "w");

    validateFile(file);

    for (int i = 0; i < accounts_counter; i++)
    {
        fprintf(file, "%s,%s,%s,%.2f,%s,%d-%d\n", global_accounts[i].account_number, global_accounts[i].name, global_accounts[i].email, global_accounts[i].balance,
                global_accounts[i].mobile, global_accounts[i].dateOpened.Month, global_accounts[i].dateOpened.Year);
    }

    fclose(file);
    printf("Changes saved successfully!\n");
}

void report(char account_number[])
{
    char filename[15];
    sprintf(filename, "%s.txt", account_number); // sprintf stores array of characters and values in array buffer

    FILE *file = fopen(filename, "r");
    validateFile(file);

    int lines_to_print = 5; // Variable to store the number of lines to print
    int line_count = 0;     // Variable to count the total number of lines
    int lines_to_skip = 0;  // Variable to count the number of lines to skip

    char temp_char;
    while ((temp_char = fgetc(file)) != EOF) // While loop to count the number of lines till the end of file
    {
        if (temp_char == '\n')
        {
            line_count++;
        }
    }
    rewind(file);

    if (line_count == 0)
    {
        printf("No transaction history detected.\n\n");
        return;
    }
    else
    {
        printf("Transaction History for Account %s:\n\n", account_number);
        printf("Date\t\tAmount\t\tTransaction Type\n");
    }

    if (line_count > 5)
    {
        lines_to_skip = line_count - lines_to_print;
    }
    else
    {
        lines_to_skip = 0;
    }

    for (int j = 0; j < lines_to_skip; j++)
    {
        while ((temp_char = fgetc(file)) != '\n')
            ;
    }

    Transaction transaction;
    while (fscanf(file, "%d/%d/%d %f %s \n", &transaction.date.Day, &transaction.date.Month, &transaction.date.Year, &transaction.amount, transaction.type) == 5)
    {
        printf("%d/%d/%d\t%.2f\t\t%s\n", transaction.date.Day, transaction.date.Month, transaction.date.Year, transaction.amount, transaction.type);
    }

    printf("\n");

    fclose(file);
}

void menuModify(void)
{
    char input_acc_number[10];
    char input_name[MAX_NAME];
    char input_email[MAX_EMAIL];
    char input_mobile[11];
    char input_amount_string[11];
    float input_balance;

    printf("Enter account number to be modified: ");
    scanf("%s", input_acc_number);
    if (!accountNumberIsValid(input_acc_number))
    {
        return;
    }
    if (!accountNumberExists("accounts.txt", input_acc_number))
    {
        printf("Error : Account number does not exist\n\n");
        return;
    }

    // Validate first and retur if invalid
    account *input_modified_account = searchForAccount(input_acc_number);

    // Initialize function parameters from the program struct
    strcpy(input_name, input_modified_account->name);
    strcpy(input_email, input_modified_account->email);
    strcpy(input_mobile, input_modified_account->mobile);
    input_balance = input_modified_account->balance;

    // The function arguments are initialized from the program struct
    // The user is prompted to modify the desired values

    // Display modification options
    char option[2];
    int modify_option;
    int cancel_flag = 0;

    do
    {
        printf("1. Name\n");
        printf("2. Email\n");
        printf("3. Balance\n");
        printf("4. Mobile\n");
        printf("5. Done\n");
        printf("0. Cancel\n");

        printf("Enter modification option: ");
        scanf("%s", option);

        // Clear input buffer after getting input with scanf
        while (getchar() != '\n')
            ;

        if (isNumeric(option))
        {
            modify_option = (int)atof(option);

            switch (modify_option)
            {
            case 1: // Name

                printf("Enter new name: ");
                scanf("\n");
                gets(input_name);
                if (!nameIsValid(input_name))
                {
                    strcpy(input_name, input_modified_account->name);
                }

                break;

            case 2: // Email

                printf("Enter new email: ");
                scanf("%s", input_email);

                if (!emailIsValid(input_email))
                {
                    strcpy(input_email, input_modified_account->email);
                }

                break;

            case 3: // Balance

                printf("Enter new balance: ");
                scanf("%s", input_amount_string);
                if (!isFloat(input_amount_string))
                {
                    printf("Error: New balance should be a number\n\n");
                }
                else
                {
                    input_balance = atof(input_amount_string);
                }

                break;

            case 4: // Mobile

                printf("Enter new mobile number: ");
                scanf("%s", input_mobile);

                if (!mobileNumberIsValid(input_mobile))
                {
                    strcpy(input_mobile, input_modified_account->mobile);
                }

                break;

            case 5: // Done

                break;

            case 0: // Cancel

                cancel_flag = 1;
                break;

            default:

                printf("Invalid option, try again.\n\n");

                break;
            }
        }
        else
        {
            printf("Error: input wasn't a number\n\n");
        }

    } while (modify_option != 5 && cancel_flag == 0);

    if (cancel_flag == 0)
    {
        modify(input_acc_number, input_name, input_email, input_balance, input_mobile);
    }
}

void menu(void)
{
    // Start a do while loop to prompt the user to choose login or quit
    // Declare characters for login and quit commands then switch
    // If the user chooses to login, call the load function, enter another do while
    // Loop until the user enters the character that quits the program
    // Display the functions for the user to choose from and call them using switch cases
    // If the user chooses quit, exit the function and it will return to main
    // to automatically shutdown the program with the return statement

    char option[2];
    int input_option;
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];

    do
    {
        printf("1. LOGIN\n");
        printf("0. QUIT\n\n");

        printf("Enter option: ");
        scanf("%s", option);

        if (isNumeric(option))
        {
            input_option = (int)atof(option);

            switch (input_option)
            {
            case 1: // Login

                // Prompting for username and password, and sending them to the login function

                printf("Enter username: ");
                scanf("%s", username);
                printf("Enter password: ");
                scanf("%s", password);
                login(username, password);
                break;

            case 0: // Quit

                printf("Exiting...");
                return; // Return to main function

            default:

                printf("Invalid option, try again.\n\n");
                break;
            }
        }
        else
        {
            printf("Error: input wasn't a number\n\n");
        }

        while (getchar() != '\n')
            ; // Clear input buffer

        // Breaking out of the loop if the user chooses to quit or he logged in
        // successfully, gaining more options in the 2nd do while loop

    } while (login_successful != 1);

    /************************************ LOGIN SUCCESSFUL MENU ************************************/

    if (login_successful == 1)
    {
        // FILE *accounts_file = fopen("accounts.txt", "r");

        load(); // Loads the accounts.txt file

        // Input variables for any function
        char input_acc_number[11];
        char input_acc_number2[11];
        char input_name[MAX_NAME];
        char input_email[MAX_EMAIL];
        char input_mobile[11];
        char input_amount_string[11];
        float input_balance;
        float input_transaction;
        int sort_option;

        // Big menu when login is successfull
        do
        {
            printf("1. ADD\n");
            printf("2. DELETE\n");
            printf("3. MODIFY\n");
            printf("4. SEARCH\n");
            printf("5. ADVANCED SEARCH\n");
            printf("6. WITHDRAW\n");
            printf("7. DEPOSIT\n");
            printf("8. TRANSFER\n");
            printf("9. REPORT\n");
            printf("10. PRINT\n");
            printf("0. QUIT\n\n");

            printf("Enter option: ");
            scanf("%s", option);

            if (isNumeric(option))
            {
                input_option = (int)atof(option);

                switch (input_option)
                {
                case 1: // Add

                    printf("Enter Account Number: ");
                    scanf("%s", input_acc_number);

                    if (!accountNumberIsValid(input_acc_number))
                    {
                        break;
                    }
                    if (accountNumberExists("accounts.txt", input_acc_number))
                    {
                        printf("Error: Account number already exists\n\n");
                        break;
                    }

                    printf("Enter Name: ");
                    scanf("\n");
                    gets(input_name);
                    if (!nameIsValid(input_name))
                    {
                        break;
                    }

                    printf("Enter Email: ");
                    scanf("%s", input_email);
                    if (!emailIsValid(input_email))
                    {
                        printf("Error: Email is invalid\n\n");
                        break;
                    }

                    printf("Enter Balance: ");
                    scanf("%s", input_amount_string);
                    if (!isFloat(input_amount_string))
                    {
                        printf("Error: Balance should be a number\n\n");
                        break;
                    }
                    else
                    {
                        input_balance = atof(input_amount_string);
                    }

                    printf("Enter Mobile: ");
                    scanf("%s", input_mobile);
                    if (!mobileNumberIsValid(input_mobile))
                    {
                        break;
                    }

                    getCurrentTime();

                    getchar();

                    add(input_acc_number, input_name, input_email, input_balance, input_mobile);
                    load();

                    break;

                case 2: // Delete

                    printf("Enter account number to be deleted: ");
                    scanf("%s", input_acc_number);

                    if (!accountNumberIsValid(input_acc_number))
                    {
                        break;
                    }
                    if (!accountNumberExists("accounts.txt", input_acc_number))
                    {
                        printf("Error : Account number does not exist\n\n");
                        break;
                    }

                    getchar();

                    delete (input_acc_number);

                    break;

                case 3: // Modify

                    menuModify();

                    break;

                case 4: // Search

                    printf("Enter account number to search for: ");
                    scanf("%s", input_acc_number);

                    if (!accountNumberIsValid(input_acc_number))
                    {
                        break;
                    }
                    if (!accountNumberExists("accounts.txt", input_acc_number))
                    {
                        printf("Error : Account number does not exist\n\n");
                        break;
                    }

                    queryAccount(input_acc_number);

                    break;

                case 5: // Advanced search

                    printf("Enter keyword: ");
                    scanf("%s", input_name); // Serves as a keyword

                    advancedSearch(input_name);

                    break;

                case 6: // Withdraw

                    printf("Enter account number to withdraw from: ");
                    scanf("%s", input_acc_number);

                    if (!accountNumberIsValid(input_acc_number))
                    {
                        break;
                    }
                    if (!accountNumberExists("accounts.txt", input_acc_number))
                    {
                        printf("Error: Account number does not exist\n\n");
                        break;
                    }

                    printf("Enter amount to be withdrawn: ");
                    scanf("%s", input_amount_string);
                    if (!isFloat(input_amount_string))
                    {
                        printf("Error: Amount to be withdrawn should be a number\n\n");
                        break;
                    }
                    else
                    {
                        input_transaction = atof(input_amount_string);
                    }

                    getCurrentTime();

                    getchar();

                    withdraw(input_acc_number, input_transaction);

                    break;

                case 7: // Deposit

                    printf("Enter account number to deposit to: ");
                    scanf("%s", input_acc_number);

                    if (!accountNumberIsValid(input_acc_number))
                    {
                        break;
                    }
                    if (!accountNumberExists("accounts.txt", input_acc_number))
                    {
                        printf("Error : Account number does not exist\n\n");
                        break;
                    }

                    printf("Enter amount to be deposited: ");
                    scanf("%s", input_amount_string);
                    if (!isFloat(input_amount_string))
                    {
                        printf("Error: Amount to be deposited should be a number\n\n");
                        break;
                    }
                    else
                    {
                        input_transaction = atof(input_amount_string);
                    }

                    getCurrentTime();

                    getchar();

                    deposit(input_acc_number, input_transaction);

                    break;

                case 8: // Transfer

                    printf("Enter account number to transfer from: ");
                    scanf("%s", input_acc_number);

                    if (!accountNumberIsValid(input_acc_number))
                    {
                        break;
                    }
                    if (!accountNumberExists("accounts.txt", input_acc_number))
                    {
                        printf("Error : Account number does not exist\n\n");
                        break;
                    }

                    printf("Enter account number to transfer to: ");
                    scanf("%s", input_acc_number2);

                    if (!accountNumberIsValid(input_acc_number2))
                    {
                        break;
                    }
                    if (!accountNumberExists("accounts.txt", input_acc_number2))
                    {
                        printf("Error : Account number does not exist\n\n");
                        break;
                    }

                    printf("Enter amount to be transfered: ");
                    scanf("%s", input_amount_string);
                    if (!isFloat(input_amount_string))
                    {
                        printf("Error: Amount to be transfered should be a number\n\n");
                        break;
                    }
                    else
                    {
                        input_transaction = atof(input_amount_string);
                    }

                    getCurrentTime();

                    getchar();

                    transfer(input_acc_number, input_acc_number2, input_transaction);

                    break;

                case 9: // Report

                    printf("Enter account number for the requested report: ");
                    scanf("%s", input_acc_number);

                    if (!accountNumberIsValid(input_acc_number))
                    {
                        break;
                    }
                    if (!accountNumberExists("accounts.txt", input_acc_number))
                    {
                        printf("Error : Account number does not exist.\n");
                        break;
                    }

                    report(input_acc_number);

                    break;

                case 10: // Print

                    printf("Select the print option:\n");
                    printf("1. Sort by name\n");
                    printf("2. Sort by date\n");
                    printf("3. Sort by balance\n");
                    scanf("%d", &sort_option);

                    switch (sort_option)
                    {
                    case 1:
                        sortByName();
                        printAccounts();
                        break;
                    case 2:
                        sortByDate();
                        printAccounts();
                        break;
                    case 3:
                        sortByBalance();
                        printAccounts();
                        break;
                    default:
                        printf("Invalid option.\n");
                        break;
                    }

                    break;

                case 0: // Quit
                    printf("Exiting...");
                    break;

                default:
                    printf("Invalid option, try again.\n\n");
                    break;
                }
            }
            else
            {
                printf("Error: input wasn't a number\n\n");
            }

            while (getchar() != '\n')
                ; // Clear input buffer

        } while (input_option != 0);
    }
}