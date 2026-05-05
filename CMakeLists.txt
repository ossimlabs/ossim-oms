cmake_minimum_required(VERSION 3.5)
PROJECT(ossim-oms)
Message("################## Setting up C OMS library #########################")

INCLUDE(OssimVersion)
INCLUDE(OssimUtilities)
INCLUDE(OssimCommonVariables)

subdirs(coms)

OSSIM_ADD_COMMON_MAKE_UNINSTALL()
