// Returns the length of given string without the nullpoint terminator.
int str_length(char *str);

// Returns 1 if both strings contains same characters in the
// same order.
int str_is_equal(char *str1, char *str2);

// Allocates memory for the new string, and returns pointer to it.
char *str_new(char *str);

// Appends string, by taking in the input string str and append.
// Memory in the location where str points to is freed, so it
// can be replaced with the return value.
char *str_append(char *str, char *append);
