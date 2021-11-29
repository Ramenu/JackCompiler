
UPDATE 1.1: Added support for compiling OS classes like Array, Math, String, etc.

This is my version of the nand2tetris JackCompiler. It is probably not the best one out there, and its features are
very primitive, and inefficient. But it works nonetheless.

You may notice that the compilationengine.cpp file is verbose and bloated with lots of redundant code. I intentionally made it
like that, because earlier when I was developing the compiler, I had a hard time understanding what was happening and where
it was happening. I then redid it from scratch, and even though it is much more messy and probably difficult to read for
most people, it helped me a lot. And it is not like I was working on this project with other people anyways, if I did, I
would have been more mindful of this.

Most of the compiler was written according to the API. There were a few subroutines that I decided not to implement, because
I didn't feel that they were necessary. 

If you run the compiler and have some errors in your code, it will probably not recognize them. Most of the time, the
program will crash before it even outputs an errors. And even if the error is outputted, it will be very vague. Still, it has 
an understanding of simple syntax, so if you're missing a semicolon it will probably recognize it, if the statement/declaration
did not have any errors, that is.

Also, if you're on WINDOWS the error colour codes are ASCI. Which command prompt does not have, for some reason. At the moment,
I'm not sure if there is a way to change the settings for that. In the future, I may release an updated version with that fixed
(probably not) but just something to keep in mind.

The total time to make the compiler took a span of 3 weeks, the first two weeks to create the parser, and the third to write
the VM code for it.


HOW DOES IT WORK?
-----------------

1) Go in your terminal

2) Go to the directory of where the JackCompiler is located

3) Type in "JackCompiler ./Filename/directory" <- Keep in mind that only files with a .jack extension will be compiled.

4) If compilation is successful you should see a .vm file for each .jack file that was compiled, if you notice that one
   or more files are missing that means that something went wrong during compilation.

Do not try to execute a .vm file if there was a compilation error, it will most likely not work. Though you can try anyways.

HOW TO EXECUTE?
---------------

1) Load the VMEmulator (can be downloaded from nand2tetris.org)

2) Load the file/directory into the VMEmulator (If there are multiple vm files, load the entire directory into it)

3) At the top, you'll see a text showing "Animate:", turn it from "Program Flow" to "No Animation" if you want to actually use the
   program.

4) Click the "Run" button (should look like a fast forward icon)

5) Done! :)


