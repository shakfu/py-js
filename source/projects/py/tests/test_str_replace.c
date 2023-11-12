// C program to search and replace  all occurrences of a word with other word. 
// https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

// Function to replace a string with another string 
// char* str_replace(const char* s, const char* old, const char* new) 
// { 
// 	char* result; 
// 	int i, cnt = 0; 
// 	int new_len = strlen(new); 
// 	int old_len = strlen(old); 

// 	// Counting the number of times old word occurs in the string 
// 	for (i = 0; s[i] != '\0'; i++) { 
// 		if (strstr(&s[i], old) == &s[i]) { 
// 			cnt++; 

// 			// Jumping to index after the old word. 
// 			i += old_len - 1; 
// 		} 
// 	} 

// 	// Making new string of enough length 
// 	result = (char*)malloc(i + cnt * (new_len - old_len) + 1); 

// 	i = 0; 
// 	while (*s) { 
// 		// compare the substring with the result 
// 		if (strstr(s, old) == s) { 
// 			strcpy(&result[i], new); 
// 			i += new_len; 
// 			s += old_len; 
// 		} 
// 		else
// 			result[i++] = *s++; 
// 	} 

// 	result[i] = '\0'; 
// 	return result; 
// } 



// Function to replace a string with another string 
char* str_replace(const char* s, const char* old, const char* new) 
{ 
	char* result; 
	int i, cnt = 0; 
	int new_len = strlen(new); 
	int old_len = strlen(old); 

	// Counting the number of times old word occurs in the string 
	for (i = 0; s[i] != '\0'; i++) { 
		if (strstr(&s[i], old) == &s[i]) { 
			cnt++; 

			// Jumping to index after the old word. 
			i += old_len - 1; 
		} 
	} 

	// Making new string of enough length
	size_t maxlen = i + cnt * (new_len - old_len) + 1;
	result = (char*)malloc(maxlen);

	i = 0; 
	while (*s) { 
		// compare the substring with the result 
		if (strstr(s, old) == s) { 
			strncpy(&result[i], new, maxlen);
			i += new_len; 
			s += old_len; 
		} 
		else
			result[i++] = *s++; 
	} 

	result[i] = '\0'; 
	return result; 
} 


// Driver Program 
int main() 
{ 
	char str1[] = "xxforxx xx for xx"; 
	char old1[] = "xx"; 
	char new1[] = "Geeks"; 

	char* result = NULL; 

	// old string 
	printf("Old string: %s\n", str1); 

	result = str_replace(str1, old1, new1); 
	printf("New String: %s\n", result); 

	char str2[] = "hello\\ there\\ world"; 
	char old2[] = "\\"; 
	char new2[] = ""; 

	printf("Old string: %s\n", str2); 

	result = str_replace(str2, old2, new2); 
	printf("New String: %s\n", result); 

	free(result); 
	return 0; 
} 
