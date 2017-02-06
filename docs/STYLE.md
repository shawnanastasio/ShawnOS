ShawnOS Kernel Style Guidelines
===============================


For consistency's sake, all code in the ShawnOS kernel should adhere to a few basic
code style guidelines. Most of these rules apply to code written in C, but some of them
can be applied to assembly as well. These should be followed as best as possible.


Indentation Style
------------
K&R-style indentation is mandatory for all code blocks in the ShawnOS kernel.
```
// Good
void my_function() {
    if (something) {
        do_something();
    }
}


// Bad
void my_function()
{
    if (something)
    {
        do_something();
    }
}
```
Also, all code indentations in the ShawnOS kernel consist of 4 spaces, as opposed to tab
characters (\t).


Typedefs
--------
The use of typedefs to shorten struct names is encouraged, but not mandatory.
Struct typedefs should be suffixed with `_t`, such as in the following example.
```
struct my_struct {
    void *my_pointer;
};
typedef struct my_struct my_struct_t;
```
Separate typedefs for structs are not necessary, and you may combine it all into one
statement if you so choose.


For integers, creating your own typedef usually serves no purpose and abstracts code
for no good reason. That means that things like this should be avoided:
```
typedef unsigned long long my_setting_type_t;
```


Integer Types
-------------
Usage of standard C integer types alone in the kernel is __strictly__ forbidden and developers are instead
encouraged to use the integer typedefs included in C99's `stdint.h`, like the following:
```
// Good
uint64_t my_good_var = 1;


// BAD!
unsigned long long my_bad_var = 1;
```
The C99 typedefs are superior in that they clearly represent the size of the integer and
aren't subject to change across platforms. While this technically could have fallen under
the previous typedef section, I felt it was important enough to get it's own section.


Spacing
-------
All function calls should not have a space before the parenthesis:
```
// Good
my_function(1, 1);


// Bad
my_function (1,1);
```


As for C keywords, the same generally applies, except for statements that can start
code blocks, such as `if, for, while, do, switch, case`. For these statements, you
should use a space before the parenthesis.
```
for (;;) {
    do_something();
}
```


Pointers
--------
When using `*` to declare or dereference pointers, the `*` should be adjacent to the
variable name:
```
// Good
uint32_t *my_pointer = 0x1000;


// Bad
uint32_t* my_pointer = 0x1000;


// Also bad
uint32_t * my_pointer = 0x1000;
```


Comments
--------
All multi-line comments should follow the following format:
```
/**
 * This is a good multi-line comment
 * It's so nice
 */
```
They should not look like any of these:
```
/** This is a bad multi-line comment
 * It's so bad
 */

/*
This is another bad
multi-line comment
*/
```


Single-line comments should use the C99 (`//`) comment style:
```
// This is a single-line comment
```


Include Guards
---------------
Instead of the traditional-style include guard, header files in the ShawnOS kernel
should instead use the `#pragma once` preprocessor directive.
```
#pragma once

void my_func();
```
The traditional-style include guards look like this and should not be used:
```
#ifdef MY_HEADER
#define MY_HEADER

void my_func();

#endif // MY_HEADER
```
