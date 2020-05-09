#ifndef STDAFX_H
#define STDAFX_H

#ifdef _WIN32
#pragma once
#endif

///NODEFAULTLIB /DYNAMICBASE:NO /MANIFEST:NO /MERGE:.rdata=.text

#define GAME_DLL
#define CLIENT_DLL

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define _CRT_SECURE_NO_WARNINGS

#include "targetver.h"


// Windows Header Files:
#include <windows.h>
#include <iomanip>
#else
#include <string>
#include <cstring>
#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#endif


#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>

#endif
