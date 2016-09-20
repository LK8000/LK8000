# Memory-Mapped File C++ Library Tutorial and Reference


## Purpose

This is a library, for the C++98 language and its successive versions,
to handle files as arrays of bytes, exploiting system calls provided
by POSIX-compliant operating systems and by Microsoft Windows.


## Contents

This package is made of one HTML documentation file,
and two C++ files:

* `README.md`: This document.
* `memory_mapped_file.hpp`: Header file, to be included by every source file
  that needs to read or to write a memory-mapped file.
* `memory_mapped_file.cpp`: Implementation file, to be compiled separately
  and to be linked into the executable.

Only POSIX-compliant operating systems (like Unix, Linux, and Mac OS X)
and Microsoft Windows are supported.


## Tutorial


### Example

First of all, a complete example of use of the library is presented.
The following program just copies a file.
Put in an empty directory the two library files `memory_mapped_file.hpp` and
`memory_mapped_file.cpp`, and a new file named `example.cpp`,
having the following contents:

    #include "memory_mapped_file.hpp"
    #include <iostream> // for std::cout and std::endl
    #include <algorithm> // for std::copy

    int CopyFile(char const* source, char const* dest, bool overwrite)
    {
        // Create a read-only memory-mapped-file for reading the source file.
        memory_mapped_file::read_only_mmf source_mf(source);

        // Check that the file has been opened.
        if (! source_mf.is_open()) return 1;

        // Check that the contents of the file has been mapped into memory.
        if (! source_mf.data()) return 2;

        // Create a writable memory-mapped-file for writing
        // the destination file, with the option to overwrite it or not,
        // if such file already exists.
        memory_mapped_file::writable_mmf dest_mf(dest,
            overwrite ? memory_mapped_file::if_exists_truncate :
            memory_mapped_file::if_exists_fail,
            memory_mapped_file::if_doesnt_exist_create);

        // Check that the file has been opened.
        if (! dest_mf.is_open()) return 3;

        // Map into memory a (new) portion of the file,
        // as large as the source file.
        dest_mf.map(0, source_mf.file_size());

        // Check that the contents of the file has been mapped into memory.
        if (! dest_mf.data()) return 4;

        // Check that the source buffer has the same size
        // of the destination buffer. It cannot be otherwise.
        if (source_mf.mapped_size() != dest_mf.mapped_size()) return 5;

        // Check that the source file has the same size
        // of the destination file. It cannot be otherwise.
        if (source_mf.file_size() != dest_mf.file_size()) return 6;

        // Copy the source buffer to the destination buffer.
        std::copy(source_mf.data(), source_mf.data() + source_mf.mapped_size(),
            dest_mf.data());
        return 0;
    }

    int main()
    {
        using namespace std;

        // Copy the first file, overwriting the second file,
        // if it already exists.
        // It should always print 0, meaning success.
        cout << CopyFile("memory_mapped_file.hpp", "copy.tmp", true) << endl;

        // Copy the first file to the second file,
        // but only if the second file does not already exist.
        // It should always print 3, meaning failure to open the second file,
        // as here the second file already exists.
        cout << CopyFile("memory_mapped_file.hpp", "copy.tmp", false) << endl;
    }

To compile and run the example, in a POSIX environment with GCC installed,
from a shell, type:

    g++ example.cpp memory_mapped_file.cpp -o example

and then

    ./example

Instead, in a Windows environment with Visual C++ installed,
from a command prompt, type:

    cl /nologo /EHsc example.cpp memory_mapped_file.cpp /Feexample.exe

and then

    example.exe

In both environments the program should print,
even if it is run several times:

    0
    3

and should create a file named `copy.tmp`, identical
to the file `memory_mapped_file.hpp`.

The behavior of this example is explained in the comments embedded
in the file `example.cpp`.

But now we'll do a step-by-step tutorial.


### Set-up

In this tutorial, we'll write several versions of a file named `tutorial.cpp`.
The first version has the following contents:

    #include "memory_mapped_file.hpp"
    using namespace memory_mapped_file;
    #include <fstream>
    #include <iostream>
    #include <string>
    using namespace std;

    char const pathname[] = "a.txt";

    void create_file()
    {
        ofstream f(pathname);
        f << "Hello, world!";
        f.close();
    }

    int main()
    {
    }

To compile it using using GCC, type the following command line:

    g++ memory_mapped_file.cpp tutorial.cpp -o tutorial

To compile it using Visual C++, type the following command line:

    cl /nologo /EHsc memory_mapped_file.cpp tutorial.cpp /Fetutorial.exe

The `create_file` function creates in the current directory a file
named `a.txt`, containing only the 13 bytes `Hello, world!`.

Of course, this program does nothing, as it has an empty `main` function.

The following versions will change only the body of the `main` function.


### Opening and closing a file

Write the following contents for the `main` function
of the `tutorial.cpp` file:

        cout << boolalpha;
        create_file();
        read_only_mmf mmf;
        cout << mmf.is_open() << endl;
        cout << mmf.file_handle() << endl;
        cout << mmf.file_size() << " " << mmf.offset() << " "
            << mmf.mapped_size() << endl;
        mmf.open(pathname, false);
        cout << mmf.is_open() << endl;
        cout << mmf.file_handle() << endl;
        cout << mmf.file_size() << " " << mmf.offset() << " "
            << mmf.mapped_size() << endl;
        mmf.close();
        cout << mmf.is_open() << endl;
        cout << mmf.file_handle() << endl;
        cout << mmf.file_size() << " " << mmf.offset() <<  " "
            << mmf.mapped_size() << endl;

When it is run, it should print:

    false
    <OS dependent invalid value>
    0 0 0
    true
    <OS dependent valid value>
    13 0 0
    false
    <OS dependent invalid value>
    0 0 0

The first statement ensures that `bool` expressions are printed as
`true` or `false`.

The second statement ensures that there is a data file to use.

The third statement defines and initializes an object owning
a memory-mapped-file for reading it, without specifying which file to use.

Therefore, no file is opened, and so the call to `is_open`
returns `false`, the call to `file_handle` returns an invalid file handle,
the call to `file_size`, `offset`, and `mapped_size` return `0`.

Then the call to the `open` member function tries to open
the specified file (searching it from the current directory),
without mapping its contents in the address space of the process.

Presumably it finds such file and opens it, and therefore
the next call to `is_open` should return `true`,
and the call to `file_handle` should return an operating-system-dependent
value of a handle for the underlying file.
Such handle can be used, for example, to lock the file for exclusive use,
using operating system calls.

The call to `file_size` should return the length of the opened file, i.e. `13`,
but, as the file is still not mapped to memory,
the calls to `offset` and `mapped_size` still return `0`.

Then the underlying file is explicitly closed, by calling `close`,
restoring the file to the condition before the opening of the file.


### Failing to open a file

Write the following contents for the `main` function
of the `tutorial.cpp` file:

        cout << boolalpha;
        read_only_mmf mmf;
        mmf.open("x", false);
        cout << mmf.is_open() << endl;
        cout << mmf.file_handle() << endl;
        cout << mmf.file_size() << endl;
        cout << mmf.offset() << endl;
        cout << mmf.mapped_size() << endl;

When it is run, assuming the current directory does not contain
a file named `x`, it should print:

    false
    <OS dependent invalid value>
    0
    0
    0

Now the `open` call fails, as the specified file cannot be opened.


### Mapping and un-mapping

Now replace all the contents of the `main` function with the following lines:

        cout << boolalpha;
        create_file();
        read_only_mmf mmf;
        mmf.open(pathname, false);
        mmf.map(2, 6);
        cout << mmf.is_open() << endl;
        cout << mmf.file_size() << endl;
        cout << mmf.offset() << endl;
        cout << mmf.mapped_size() << endl;
        mmf.unmap();
        cout << mmf.is_open() << endl;
        cout << mmf.file_size() << endl;
        cout << mmf.offset() << endl;
        cout << mmf.mapped_size() << endl;

When it is run, it should print:

    true
    13
    2
    6
    true
    13
    0
    0

The call to the `map` member function creates a mapping
to memory of the file contents from the offset specified by the first argument,
for the length specified by the second argument.
This appears also by the ensuing calls to `offset` and `mapped_size`, that
return `2` and `6`, respectively.

Then, by calling the `unmap` member function, the mapping is undone.


### More on mapping

Now replace all the contents of the `main` function with the following lines:

        create_file();
        read_only_mmf mmf;
        mmf.open(pathname, false);
        mmf.map(2);
        cout << mmf.offset() << endl;
        cout << mmf.mapped_size() << endl;
        mmf.map();
        cout << mmf.offset() << endl;
        cout << mmf.mapped_size() << endl;
        mmf.map(10, 10000);
        cout << mmf.offset() << endl;
        cout << mmf.mapped_size() << endl;

When it is run, it should print:

    2
    11
    0
    13
    10
    3

First, notice that `map` is called three times and `unmap` is never called.
Actually `unmap` is implicitly called
by the `map` function and by the destructor.

Then, notice that `map` may have only one argument or no arguments,
as they have both the default value `0`.

Then, notice that the `0` value for the second argument of `map` doesn't mean
that the mapping will have zero length (that is impossible),
but that it will extend up to the length of the file, if possible.

At last, notice that even if the specified offset plus the specified length
exceeds the length of the file, the mapping extends anyway up to the length
of the file, as shown by the last printed line.


### Implicit opening and mapping

Now replace all the contents of the `main` function with the following lines:

        cout << boolalpha;
        create_file();
        read_only_mmf mmf1(pathname, false);
        cout << mmf1.is_open() << endl;
        cout << mmf1.file_size() << endl;
        cout << mmf1.offset() << endl;
        cout << mmf1.mapped_size() << endl;
        
        read_only_mmf mmf2;
        mmf2.open(pathname);
        cout << mmf2.is_open() << endl;
        cout << mmf2.file_size() << endl;
        cout << mmf2.offset() << endl;
        cout << mmf2.mapped_size() << endl;

        read_only_mmf mmf3(pathname);
        cout << mmf3.is_open() << endl;
        cout << mmf3.file_size() << endl;
        cout << mmf3.offset() << endl;
        cout << mmf3.mapped_size() << endl;

When it is run, it should print:

    true
    13
    0
    0
    true
    13
    0
    13
    true
    13
    0
    13

When the object `mmf1` is constructed, it gets two arguments that
cause to open to specified file, but not to map its contents to memory.
This avoids to call separately the constructor and the `open` function.

When the object `mmf2` is opened, its contents is implicitly entirely
mapped to memory.
This avoids to call separately the `open` and `map` functions.

When the object `mmf3` is constructed, it is implicitly opened,
and its contents is implicitly entirely mapped to memory.
This avoids to call separately the constructor, and the `open`
and `map` functions.


### Reading the file contents

Now replace all the contents of the `main` function with the following lines:

        create_file();
        read_only_mmf mmf(pathname);
        cout << string(mmf.data(), 8) << endl;
        mmf.map(2);
        cout << string(mmf.data(), 8) << endl;
        mmf.map(3000);
        cout << (size_t)mmf.data() << endl;

When it is run, it should print:

    Hello, w
    llo, wor
    0

If the mapping succeeds, every ensuing calls to `data` return
a pointer to the mapped buffer starting at the specified offset.

But the `map` function may fail, for several reasons.
For example, no mapping is possible if the file is not opened,
or if the specified offset is equal to or greater than the file size,
or if there is not enough address space to map all the specified range.

If the `map` call fails, every ensuing call to `data` returns a null pointer;
therefore, every time you try to map a file, implicitly or by calling `map`,
you should check the value returned by `data` or by `mapped_size`,
before dereferencing the pointer returned by `data`.


### Read-only access

Now replace all the contents of the `main` function with the following lines:

        create_file();
        read_only_mmf mmf(pathname);
        cout << mmf.data()[0];
        mmf.data()[0] = 'a';

When it is compiled, a syntax error should occurs in the last statement,
as the call to `data` returns a `const` address.
However, if such const-ness is bypassed using a cast,
the program will compile, but that statement will generate a run-time error,
as the operating system has marked such address range as _read-only_.


### Opening for writing

Up to now, only the `read_only_mmf` class has been used.
Such class disallows to change the file.

Memory mapped files may be used also for changing the contents of files,
as the following examples show.

Now replace all the contents of the `main` function with the following lines:

        cout << boolalpha;
        create_file();
        writable_mmf mmf(pathname, if_exists_map_all, if_doesnt_exist_fail);
        cout << mmf.is_open() << endl;
        cout << mmf.file_size() << endl;
        cout << mmf.offset() << endl;
        cout << mmf.mapped_size() << endl;
        cout << string(mmf.data(), 8) << endl;

When it is run, it should print:

    true
    13
    0
    13
    Hello, w

It means that the file has been opened, its size is 13 bytes,
all the file has been mapped, and its first `8` characters are `Hello, w`.

As it appears, this object can be used just like a read-only memory mapped
file, except for the constructor.


### Read/write mapping mode

We have already seen one usage instance of the `writable_mmf` class.
There are several other ways to create such object,
described in the reference.
The four most typical ones are used in the program
whose `main` function has the following contents:

        cout << boolalpha;

        // 1. Fail or create
        { // 1.1. File exists
            create_file();
            writable_mmf mmf(pathname,
                if_exists_fail, if_doesnt_exist_create);
            cout << mmf.is_open() << endl;
        }
        { // 1.2. File doesn't exist
            remove(pathname);
            writable_mmf mmf(pathname,
                if_exists_fail, if_doesnt_exist_create);
            cout << mmf.is_open() << " " << mmf.file_size() << endl;
        }

        // 2. Truncate or create
        { // 2.1. File exists
            create_file();
            writable_mmf mmf(pathname,
                if_exists_truncate, if_doesnt_exist_create);
            cout << mmf.is_open() << " " << mmf.file_size() << endl;
        }
        { // 2.2. File doesn't exist
            remove(pathname);
            writable_mmf mmf(pathname,
                if_exists_truncate, if_doesnt_exist_create);
            cout << mmf.is_open() << " " << mmf.file_size() << endl;
        }

        // 3. Just open or fail
        { // 3.1. File exists
            create_file();
            writable_mmf mmf(pathname,
                if_exists_just_open, if_doesnt_exist_fail);
            cout << mmf.is_open() << " " << mmf.file_size() << " "
                << mmf.mapped_size() << endl;
        }
        { // 3.2. File doesn't exist
            remove(pathname);
            writable_mmf mmf(pathname,
                if_exists_just_open, if_doesnt_exist_fail);
            cout << mmf.is_open() << endl;
        }

        // 4. Map all or fail
        { // 4.1. File exists
            create_file();
            writable_mmf mmf(pathname,
                if_exists_map_all, if_doesnt_exist_fail);
            cout << mmf.is_open() << " " << mmf.file_size() << " "
                << mmf.mapped_size() << endl;
        }
        { // 4.2. File doesn't exist
            remove(pathname);
            writable_mmf mmf(pathname,
                if_exists_map_all, if_doesnt_exist_fail);
            cout << mmf.is_open() << endl;
        }

When it is run, it should print:

    false
    true 0
    true 0
    true 0
    true 13 0
    false
    true 13 13
    false

There are 4 cases, each one with two sub-cases:
in the first one the file already exists,
and in the second one the file doesn't exist yet.

In case 1.1., the file exists, and there is the option `if_exists_fail`;
therefore such file is not opened, and `is_open` returns `false`.

In case 1.2., the file does not exist, and there is the option
`if_doesnt_exist_create`;
therefore such file is created empty, and so `is_open` returns `true`,
and `file_size` returns `0`.

Case 1 ("Fail or create") is useful when it is needed to create a file,
without overwriting an existing file.

In case 2.1., the file exists, and there is the option `if_exists_truncate`;
therefore such file is opened and truncated,
and so `is_open` returns `true`, and `file_size` returns `0`.

In case 2.2., the file does not exist, and there is the option
`if_doesnt_exist_create`;
therefore such file is created empty, and so `is_open` returns `true`,
and `file_size` returns `0`.

Case 2 ("Truncate or create") is useful when it is needed to create a file,
even overwriting an existing file.

In case 3.1., the file exists, and there is the option `if_exists_just_open`;
therefore such file is opened and not truncated nor mapped,
and so `is_open` returns `true`, `file_size` returns `13`,
and `mapped_size` returns `0`.

In case 3.2., the file does not exist, and there is the option
`if_doesnt_exist_fail`;
therefore such file is not opened, and so `is_open` returns `false`.

Case 3 ("Just open or fail") is useful when it is needed
to change a very large existing file, to be mapped piece-wise.

In case 4.1., the file exists, and there is the option `if_exists_map_all`;
therefore such file is opened and not truncated, and it is mapped all,
and so `is_open` returns `true`, `file_size` returns `13`,
and `mapped_size` returns `13`.

In case 4.2., the file does not exist, and there is the option
`if_doesnt_exist_fail`;
therefore such file is not opened, and so `is_open` returns `false`.

Case 4 ("Map all or fail") is useful when it is needed
to change a not-so-large existing file, to be handled as a single string.

Of course, when a file is to be created or changed, it is required to use
the `writable_mmf` class.
Instead, when a file is only to be read, also the `read_only_mmf` classes
could be used.
Nevertheless, using `read_only_mmf` has the following advantages:

* **Reading read-only files or files shared only for reading**:
  `writable_mmf` objects require to open the specified file
  in read/write mode, and the operating system prevents such operation
  on files marked as *read-only* or shared only for reading.
  The only way to read a read-only file or a file shared only for reading
  is to use a `read_only_mmf` object.
* **Simpler API**: As `read_only_mmf` cannot change to specified file,
  it has fewer features, and therefore it is simpler to learn and use.
* **No risk of accidental change**: As `writable_mmf` objects
  open the specified file in read/write mode, a logically erroneous operation
  or an undefined behavior operation could apply unwanted changes
  to the contents of such file.
  As `read_only_mmf` objects open the specified file in read-only mode,
  the operating system prevents any subsequent attempt to change it.
* **Possibly more efficient**: Operating systems may use more efficient
  buffering algorithms for read-only files, used by `read_only_mmf` objects,
  than for read/write files, used by `writable_mmf` objects.


### Read/write access

Up to now, in our examples, no file has been changed by a memory-mapped-file.

Now replace all the contents of the `main` function with the following lines:

        create_file();
        {
            writable_mmf mmf(pathname,
                if_exists_map_all, if_doesnt_exist_fail);

            cout << string(mmf.data(), 6) << endl;
            mmf.data()[1] = 'X';
            cout << string(mmf.data(), 6) << endl;
        }
        {
            read_only_mmf mmf(pathname);
            cout << string(mmf.data(), 6) << endl;
        }

It should print:

    Hello,
    HXllo,
    HXllo,

This means that the `data` function of the `writable_mmf` class,
returns a non-`const` address of a buffer,
and when some bytes of such buffer are changed,
such changes are immediately visible to the process, and are also
saved to the file sometime no later than when the current scope is closed.


### Flushing writes

Such effective write to the file is handled by the operating system,
and, for efficiency reasons, usually it does not happen immediately,
as it is shown by replacing all the contents of the `main` function
with the following lines:

        create_file();
        writable_mmf mmf(pathname,
            if_exists_map_all, if_doesnt_exist_fail);
        mmf.data()[0] = 'X';
        mmf.flush();
        mmf.data()[1] = 'Y';
        cin.get();

This program writes a letter "X" as the first byte of the file,
and calls the `flush` function to ensure it is written to the file.
Then it writes a letter "Y" as the second byte of the file,
and waits for user input.
If you now press the reset button of your computer,
or remove any electric power supply, preventing the operating system to save
this second byte to safe storage, and then restart your computer,
and look into file "a.txt", you should find it has the following contents:

    Xello, world!"

As you can see, the 'X' character has been written to the file,
thanks to the call to `flush`, but the `Y` character is not.

This brutal procedure is necessary to show the effect,
because if you terminate the process in any other way,
the operating system is still able to save the buffer to persistent storage.

The `flush` call, albeit more efficient than closing and reopening the file,
is rather inefficient, though, because it writes data to physical storage,
and so it should be used only when data consistency is required
even in case of a power failure or an operating system crash.


# Reference


## Introduction

To support several operating systems, the source files contain several code
portions under conditional compilation.
If the `_WIN32` macro is defined, then the Microsoft Windows API is used;
otherwise the POSIX API is used, allowing compilation for Linux, Unix,
MAC OS X, and other POSIX-compliant operating systems.

The header file contains only the namespace `memory_mapped_file`,
containing the definition of the following items:

* The `mmf_granularity` function: It allows to get operating system allocation
  granularity (for advanced uses).
* The `base_mmf` abstract class: It contains what is common
  between the other classes. It cannot be instantiated.
* The `read_only_mmf` class: It is used to access an already existing
  file only for reading it.
* The `writable_mmf` class: It is used to access an already existing
  or a not yet existing file for both reading and writing it.
* The `mmf_exists_mode` enumeration: Options for creating a writable
  memory-mapped-file based on an already existing file.
* The `mmf_doesnt_exist_mode` enumeration: Options for creating a writable
  memory-mapped-file based on a not yet existing file.


## The `mmf_granularity` function

Scope: namespace `memory_mapped_file`.

Operating systems do not allow to map files to memory starting from every
specified byte. They require that the offset be a multiple of a number,
named _granularity_, that is dependent on the operating system,
and typically may vary from 4 KiB to 64 KiB.

To avoid bothering users with such technicality, this library takes care
of mapping memory internally from the nearest boundary.
For example, if the granularity is 65536, and for a 10 MB long file a mapping
is requested from 500000 to 800000,
the memory actually mapped by the operating system is from 458752
to a number somewhat greater than 800000,
but the `offset` function will return 500000,
and the `mapped_size` function will return 300000.

To allow the user to take granularity into account,
there is a global function named `mmf_granularity`,
that returns such granularity size.

It is called like in the following statement:

    unsigned int granularity = memory_mapped_file::mmf_granularity();


## The `base_mmf` class

Scope: namespace `memory_mapped_file`.

Abstract class representing a memory-mapped-file.
It is the base class of `read_only_mmf` and `writable_mmf`,
and therefore it gathers the features common to both classes.

The possible states of the instances of this class are:

1. File not opened.
1. File opened but not mapped.
1. File opened and mapped.

The constructor sets the object in one of the three possible states,
as it can fail to open the underlying file or not even try to open it
(state 1), successfully open the file but fail to map it or not even try
to map it (state 2), or successfully open and map the file (state 3).

An object in state 1 (file not opened) can pass to another state,
by calling successfully the `open` function. If the value
of the second argument of the call is `false`, the mapping is not even
attempted.
If the value of the second argument of the call is `true` or is missing,
the mapping is attempted, but it may fail.

An object in state 2 (file not mapped) can pass to state 1 (file not opened)
by calling the `close` function, and it can pass to state 3 (file mapped)
by calling the `map` function.

An object in state 3 (file mapped) can pass to state 1 (file not opened)
by calling the `close` function, and it can pass to state 2 (file not mapped)
by calling the `unmap` function.

### The function `base_mmf()`

Scope: class `base_mmf`.

It is the only constructor of its class.
As this class is abstract, it can be called
only by the constructor of the derived classes.

### The function `~base_mmf()`

Scope: class `base_mmf`.

It is the destructor.
It releases every resources previously allocated by the object.

### The function `size_t offset() const`

Scope: class `base_mmf`.

It returns the distance in bytes
of the beginning of the portion of the file currently mapped to memory
from the beginning of the file. Therefore, it returns `0` (zero)
when the mapping starts at the beginning of the file.
It returns `0` also when the file hasn't been opened successfully,
or when the file has been opened, but it hasn't been mapped successfully.
therefore it cannot be used to discern if the file is open or not,
nor to discern if the file is mapped or not.

### The function `size_t mapped_size() const`

Scope: class `base_mmf`.

It returns the size in bytes of the portion of the file currently
mapped to memory.
It returns `0` when the file hasn't been opened successfully,
or when the file has been opened, but it hasn't been mapped successfully.
A mapping cannot have zero length, therefore this call can be used
to discern if the file is mapped or not.

### The function `size_t file_size() const`

Scope: class `base_mmf`.

It returns the whole size of the underlying opened file.
Of course, it returns `0` when the opened file has zero-length,
but it returns `0` also when the file hasn't been opened successfully;
therefore it cannot be used to discern if the file is open or not.

### The function `void unmap()`

Scope: class `base_mmf`.

It cancels the current mapping.
It is called implicitly at the beginning of the `map` function,
and by the destructor.
It is always assumed successful.

### The function `void close()`

Scope: class `base_mmf`.

It closes the currently open file.
It is called implicitly at the beginning of the `open` function,
and by the destructor.
It is always assumed successful.

### The function `bool is_open() const`

Scope: class `base_mmf`.

It returns `true` if and only if the underlying file has been opened
successfully.

### The type name `HANDLE`

Scope: class `base_mmf`.

Such name represents the operating-system-dependent type
of the handle of a file.
For POSIX systems, it is `int`; for Microsoft Windows, it is `void *`.

### The function `HANDLE file_handle() const`

Scope: class `base_mmf`.

It returns the operating-system-dependent handle used internally to access
the file.
It may be used to perform operating-system-dependent operations,
not defined by this library, like file locking.
If the file is open it returns a valid handle, while if the file
is not open it returns an invalid handle, therefore it can be used
to discern if a file is open or not, but using
an operating-system-dependent value.

## The `read_only_mmf` class

Scope: namespace `memory_mapped_file`.

The instances of this class encapsulate memory-mapped-files
that access a file only for reading it.
Internally it opens the underlying file only for reading it.

This class is derived from the class `base_mmf`.
Therefore the documentation of such class should be read
to see the inherited features.

It has several advantages with respect to the `writable_mmf` class,
whose instances are capable of changing a file. They are:

* **May be the only way**.
  If the operating system prevents any change to the underlying file
  by the current user, any attempt to open such file for reading/writing,
  like `writable_mmf` objects always do, will fail.
* **It's simpler to use**.
  There is no need to specify what to do if the file does not exist,
  as obviously it cannot be opened.
  It is simpler to specify what to do if the file exists,
  as it cannot be truncated, and it is senseless to fail.
* **It's safer to use**.
  There is no risk of modifying accidentally the file.
  Typically any attempt to change the file will cause a compilation error;
  but if the code can be compiled, it will cause a run-time error
  by the operating system.
* **It's more efficient**.
  Operating systems usually use more efficient buffering algorithms
  for read-only files, than for read/write files.

### `explicit read_only_mmf(char const* pathname, bool map_all = true)`

Scope: class `read_only_mmf`.

It is the only constructor of its class.

The `pathname` argument is the relative or absolute pathname
of the underlying file, specified according the operating system syntax.

The `map_all` argument specifies if, in case the file could be successfully
opened, such file should also be entirely mapped to memory or not.

By default, it is mapped, as it is the most convenient thing to do.
Instead, if it is needed to map the file later, or if it is needed
to map the file a piece at a time, the value of second argument
should be `false`.

### `void open(char const* pathname, bool map_all = true)`

Scope: class `read_only_mmf`.

It tries to open a file to be used for a read-only mapping to memory,
and optionally to map to memory the contents of that file.

The `pathname` argument is the relative or absolute pathname
of the underlying file, specified according the operating system syntax.

The `map_all` argument specifies if, in case the file could be successfully
opened, such file should also be entirely mapped to memory or not.

By default, it is mapped, as it is the most convenient thing to do.
Instead, if it is needed to map the file later, or if it is needed
to map the file a piece at a time, the value of second argument
should be `false`.

If the `open` function is called when the file is already open,
it is closed first.
Therefore, it useless to call `close` just before calling `open`.


### `char const* data() const`

Scope: class `read_only_mmf`.

It returns the address of the beginning of the read-only memory buffer mapped
to a portion of the file.

If and only if the file is not mapped,
it returns `0` (i.e. `nullptr`).
Therefore this call can be used to discern if the file is mapped or not.

Of course, it is undefined behavior both dereferencing the null pointer,
and accessing the referenced buffer before the beginning or after the end.

### `void map(size_t offset = 0, size_t size = 0)`

Scope: class `read_only_mmf`.

It tries to create a mapping between a portion of the file and a memory buffer.

The `offset` argument specifies the distance of the beginning of the mapped
portion from the beginning of the file. By default it is `0` (zero),
meaning that the mapping starts at the beginning of the file.

The `size` argument specifies the length of the required mapped portion
of the file. If `offset + size` is greater than the length of the file,
the mapping extends up to the end of the file.
For example, if a file is 500 bytes long, and `object` is of type
`read_only_mmf`, the following statement:

    object.map(100, 130);

maps into memory the 130 bytes from position 100 included
to position 230 excluded, counting from 0.

And the following statement:

    object.map(100, 750);

maps the 400 bytes from position 100 included to position 500 excluded.

By default, the `size` argument is `0`, meaning a request to map
the file up to its end.

The `map` function fails in the following cases:

* the underlying file is not open (so that `is_open` returns `false`);
* the specified offset is equal to or greater than the file size;
* there is is not enough address space to map all the specified range;
* the operating system refuses to map the file to memory for some other reason.

If the `map` function is successful,

* the `offset` function returns the same value passed as the `offset`
  argument;
* the `mapped_size` function returns the size of the mapped portion,
  that is not greater than the value of the `size` argument
  (except when it is zero);
* the `data` function returns a non-null value, that is a valid memory address.

Instead, if the `map` function fails, the `offset`,
the `mapped_size`, and the `data` functions return `0`.

If the `map` function is called when the file is already mapped,
it is unmapped first.
Therefore, it useless to call `unmap` just before calling `map`.


## The `mmf_exists_mode` enumeration

Scope: namespace `memory_mapped_file`.

This enumeration specifies what to do when a `writable_mmf` object
is created and the underlying file already exists.

There are four cases:

* `if_exists_fail`: the file is not opened,
  so that if later `is_open` is called, it will return `false`,
  and of course if `mapped_size` and `data` are called, they will return `0`.

* `if_exists_just_open`: the file is opened but not mapped,
  so that if the open is successful and later `is_open` is called,
  it will return `true`, but `mapped_size` and `data` will however return `0`.

* `if_exists_map_all`: the file is opened and mapped all to memory,
  so that if the open is successful and later `is_open` is called,
  it will return `true`, and if the map is also successful and later
  `mapped_size` and `data` are called they will return non-null values.

* `if_exists_truncate`: the file is opened and truncated,
  so that if the open is successful and later `is_open` is called,
  it will return `true`, but `mapped_size` and `data` will however return `0`.


## The `mmf_doesnt_exist_mode` enumeration

Scope: namespace `memory_mapped_file`.

This enumeration specifies what to do when a `writable_mmf` object
is created and the underlying file does not exist yet.

There are only two cases:

* `if_doesnt_exist_fail`: the file is not created,
  so that if later `is_open` is called, it will return `false`,
  and of course if `mapped_size` and `data` are called, they will return `0`.
* `if_doesnt_exist_create`, the file is created empty,
  so that if the creation is successful and later `is_open` is called,
  it will return `true`, but `mapped_size` and `data` will however return `0`.


## The `writable_mmf` class

Scope: namespace `memory_mapped_file`.

The instances of this class encapsulate memory-mapped-files
that access a file for reading or writing it.
Internally it opens the underlying file for reading or writing it.

This class is derived from the `base_mmf` class.
Therefore see the documentation of such class to see the inherited features.

In addition, this class is rather similar to the `read_only_mmf` class,
therefore here only the difference from such class are specified.


### `explicit writable_mmf(char const* pathname, mmf_exists_mode exists_mode, mmf_doesnt_exist_mode doesnt_exist_mode)`

Scope: class `writable_mmf`.

It is the only constructor of this class.

For more information, look at the description of the constructors
of `base_mmf` and of `read_only_mmf`,
and at the description of the enumerations `mmf_exists_mode`
and `mmf_doesnt_exist_mode`.

The `exists_mode` argument has four possible values,
and the `doesnt_exist_mode` has two possible values,
and therefore there are the following eight possible construction cases:

* `writable_mmf(pathname, if_exists_fail, if_doesnt_exist_fail)`:
  Fail always. Of course it is senseless.
* `writable_mmf(pathname, if_exists_fail, if_doesnt_exist_create)`:
  Fail or create. To be used to copy a file without overwriting
  an existing file.
* `writable_mmf(pathname, if_exists_just_open, if_doesnt_exist_fail)`:
  Open or fail. Similar to `read_only_mmf(pathname, false)`.
* `writable_mmf(pathname, if_exists_just_open, if_doesnt_exist_create)`:
  Open or create. To be used to modify a file piece-wise,
  by creating it if not yet existing.
* `writable_mmf(pathname, if_exists_map_all, if_doesnt_exist_fail)`:
  Map or fail. Similar to `read_only_mmf(pathname)`.
* `writable_mmf(pathname, if_exists_map_all, if_doesnt_exist_create)`:
  Map or create. To be used to modify a file as a whole,
  by creating it if not yet existing.
* `writable_mmf(pathname, if_exists_truncate, if_doesnt_exist_fail)`:
  Truncate or fail. To be used to overwrite an existing file. Rarely useful.
* `writable_mmf(pathname, if_exists_truncate, if_doesnt_exist_create)`:
  Truncate or create. To be used to copy a file even overwriting
  an existing file.

### `char* data()`

Scope: class `writable_mmf`.

### `void open(char const* pathname, mmf_exists_mode exists_mode = if_exists_fail, mmf_doesnt_exist_mode doesnt_exist_mode = if_doesnt_exist_create)`

Scope: class `writable_mmf`.

It tries to open a file to be used for a read-write mapping to memory,
and optionally to map to memory the contents of that file.

It is similar to the function with the same name of the `read_only_mmf` class,
and to the constructor of this class.

### `void map(size_t offset = 0, size_t size = 0)`

Scope: class `writable_mmf`.

It is has the same syntax and semantics of the function with the same name
of the `read_only_mmf` class.

### `bool flush()`

Scope: class `writable_mmf`.

It copies all the changes to the file system, ensuring that
they are persistent.

Actually, when a byte is modified in a memory-mapped file, that change
may be applied much later to the underlying file, possibly only when
the memory-mapped file is closed.

To ensure that every previous change is actually applied to the storage device,
it is possible to close and reopen the memory-mapped file.
The `flush` operation achieves the same effect much more efficiently.

If returns `true` if the operation succeeds, otherwise `false`.
