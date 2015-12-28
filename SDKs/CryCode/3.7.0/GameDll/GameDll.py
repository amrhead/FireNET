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

TARGET = Target('GameDll', TargetTypes.Module)


TARGET.flags.compile[BuildConfigs.Common].add_includes(os.path.join('%CODEROOT%',globals.GAME_VERSION,TARGET.name),os.path.join('%CODEROOT%','CryEngine','CryCommon'),os.path.join('%CODEROOT%','SDKs','boost'))

TARGET.flags.compile[BuildConfigs.Common].add_includes(os.path.join('%CODEROOT%','CryEngine','CryAction'),os.path.join('%CODEROOT%','GameSDK','GameDll'),os.path.join('%CODEROOT%','SDKs','CrispRMF-C++-3.0.0','CryCrispWrapper'))

VCXPROJ = 'GameDllSDK.vcxproj'

TARGET.flags.compile[BuildConfigs.Common].add_definitions("DISABLE_OM=1")

REMOVE_SOURCES_C = []
REMOVE_SOURCES_H = []
REMOVE_SOURCES_CPP = ['StdAfx.cpp','OrganicMotion/%', 'XboxOneLive/%', 'Network/MatchMakingUtils.cpp']
