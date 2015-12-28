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

TARGET = Target('CryCommon', TargetTypes.Dummy)

VCXPROJ = 'CryCommon.vcxproj'

REMOVE_SOURCES_C = []
REMOVE_SOURCES_H = []
REMOVE_SOURCES_CPP = []
