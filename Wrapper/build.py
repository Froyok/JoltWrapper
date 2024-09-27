# https://www.ibm.com/docs/en/zvm/7.2?topic=application-building-simple-c-dll
# https://www.codementor.io/@a_hathon/building-and-using-dlls-in-c-d7rrd4caz
# https://www.cprogramming.com/tutorial/shared-libraries-linux-gcc.html
# https://nachtimwald.com/2019/04/05/cross-platform-thread-wrapper

import subprocess
import platform
import shutil
import os

import lib_common as libc

GCC_Command = {
	False : "gcc",

	# https://archlinux.org/packages/extra/x86_64/mingw-w64-gcc/
	# https://stackoverflow.com/questions/43658560/i-installed-mingw32-pthreads-w32-but-i-still-cannot-compile-stdmutex
	True : "x86_64-w64-mingw32-gcc"
}
Library_Extension = {
	False : ".so",
	True : ".dll"
}

def BuildLinux( ForWindows ):
	WorkingDirectory = libc.JoinPath( libc.SCRIPT_FOLDER, "jolt_wrapper" )

	FormatGCC = GCC_Command[ForWindows]
	Extension = Library_Extension[ForWindows]

	print( FormatGCC, Extension )

	# PIC (Position Independent Code)
	# ---
	# Needed for code that will be loaded at a memory
	# address without modification.
	# Used for creating shared libraries
	print( " - Generate PIC..." )
	Command = '{} -c -Wall -fPIC main.c'.format( FormatGCC )
	Command = subprocess.call( Command, shell=True, cwd=WorkingDirectory )

	# Build library .so from PIC
	print( " - Generate library..." )
	Command = '{} -shared -o jolt_wrapper{} main.o'.format( FormatGCC, Extension )
	subprocess.call( Command, shell=True, cwd=WorkingDirectory )

	# Copy to love folder
	print( " - Copy to update library..." )
	SourcePath = libc.JoinPath(
		WorkingDirectory,
		"jolt_wrapper{}".format( Extension )
	)
	TargetPath = libc.JoinPath(
		libc.GAME_PATH,
		"bin/lib/linux/libjolt_wrapper{}".format( Extension )
	)
	shutil.copyfile( SourcePath, TargetPath )

	print( "Done." )

def BuildWindows() :
	raise Exception( "Windows compiling not implemented" )

def Run() :
	if platform.system().lower() == "linux" :
		BuildLinux( False )
		BuildLinux( True )
	else :
		BuildWindows()
