Neural Quake
======
A neural network AI for Quake, built as part of the INFT578 Software Development 4 course.


Building the Executable
------
This project was created in Visual Studio 2013 Community, and as such should be 
compiled with any of the VS2013 IDEs.

It is also recommended that the executable be compiled with either the 
'GL Debug' or 'GL Release' configurations. Other configurations work,
but debug graph rendering is incorrect and will have issues.


Running the AI
------
So you've got this far... How do you make the AI work? Simple. First, make sure 
you've loaded a single player map.

The recommended way to load the recommended map is to type 'map e1m1' into 
the in-game console. This will load the first map of the single player
campaign, 'e1m1'.

Then, accessing the same in-game console, type 'nq_start' into the console.
That's it! It should be running now. Simply close the console after it has
loaded and let the AI do its work.


Stopping the AI
------
In order to stop the AI, simply type 'nq_end' into the in-game console.
Optionally, a '<filename>' argument may be added after nq_end to save the 
generation's current data to a file when it ends. On that note...


Saving Data
------
If, at any point, you wish to save the current generation of the neural AI,
type 'nq_save <filename>' into the in-game console, where '<filename>' is the
name of the file to be saved. This file will be saved in the 'neural_backups'
folder in the base directory of the executable, if that directory exists.

###Important!
Do not include an extension; the file will save with the '.nq' extension.


Loading Data
------
Identical to saving data, except we're loading from the 'neural_backups' folder.
Just type the name of a .nq file that exists within this folder and Neural Quake
will attempt to load it as a generation of data.