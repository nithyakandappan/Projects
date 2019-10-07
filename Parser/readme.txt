This program takes a single input and splits it into words by any non-alphabetical character.
I use a a struct called Node and a few methods to accomplish this:
    print - this sets a pointer to the last node, and then traverses back to the
            front and prints each one along the way
    compareStrings - this method compares two strings and returns 1 if string1
                      is greater (meaning it comes after string2 alphabetically),
                      returns 2 if string2 is greater, and returns 3 if the strings
                      are equal
    addToken - adds a token to the appropriate place in the linked list
    
