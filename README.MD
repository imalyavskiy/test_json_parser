DESCRIPTION
===========================================
This is a take to implement a JSON object/array that is fully template (de-)serializable and its instances are capable to be passed
between separate binary modules forming in self turn a process executable code where each of them built with static 'runtime library'
usage. I.e. in case if modules built with static 'runtime library' ech of them have its own memory manager instance that means that 
each of them have their own dynamic memory pool. This causes some inconfinience when some std container allocates memory within one
module but frees in another - this causes error.

The parser was made from syntactical digrams taken at json.org. Uses state machine to perform parsing.

And a lot of C++17 template magic...

HOT TO RUN
-------------------------------------------
- get repo
- init submodules
- run prepare.bat it will generate necessary files and then build GTest and GMock
- open json_lib.sln
- build and run the json_test project
- take a look to 'Project Properties'->'Configuration Properties'->'Debugging'->'Command arguments'

**Have fun!**