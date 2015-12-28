#############################################################################
## Crytek Source File
## Copyright (C) 2013, Crytek Studios
##
## Creator: Leander Beernaert
## Date: October 17, 2013
## Description: Makeomatic based build system
#############################################################################

from makeomatic.makeomatic import*
from makeomatic.defines import*
from makeomatic.export import*
from project_utils import*
import globals
import os

TARGET = Target('CryInput', TargetTypes.Module)

TARGET.flags.compile[BuildConfigs.Common].add_includes(os.path.join('%CODEROOT%','CryEngine',TARGET.name),os.path.join('%CODEROOT%','CryEngine','CryCommon'),os.path.join('%CODEROOT%','SDKs','boost'))



if not globals.ENV.is_dedicated:
    TARGET.flags.compile[BuildConfigs.Common].add_includes('%LIB%INC%sdl2%')
    if globals.PLATFORM.name == "android":
        TARGET.flags.link[BuildConfigs.Common].add_ldlibs('%LIB%LIB%sdl2%')

expand_sources(TARGET)
VCXPROJ = 'CryInput.vcxproj'

REMOVE_SOURCES_C = []
REMOVE_SOURCES_H = []
REMOVE_SOURCES_CPP = []
