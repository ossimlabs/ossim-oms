PROJECT(ossim-oms)
Message("################## Setting up C OMS library #########################")

INCLUDE(OssimVersion)
INCLUDE(OssimUtilities)
INCLUDE(OssimCommonVariables)

cmake_minimum_required(VERSION 2.8)

subdirs(coms)

OSSIM_ADD_COMMON_MAKE_UNINSTALL()
