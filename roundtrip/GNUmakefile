# -*- makefile -*-
#----------------------------------------------------------------------------
#       GNU ACE Workspace
#
# $Id: GNUACEWorkspaceCreator.pm 2490 2015-03-20 20:50:04Z mitza $
#
# This file was generated by MPC.  Any changes made directly to
# this file will be lost the next time it is generated.
#
# MPC Command:
# /home/karlkw/OpenDDS-3.9/ACE_wrappers/bin/mwc.pl -type gnuace /home/karlkw/OpenDDS-3.9/DDS_TAOv2_all.mwc
#
#----------------------------------------------------------------------------

MAKEFILE = GNUmakefile

ifeq ($(findstring k,$(MAKEFLAGS)),k)
  KEEP_GOING = -
endif

include $(ACE_ROOT)/include/makeinclude/macros.GNU

all: RoundTrip_Idl RoundTrip_Ping RoundTrip_Pong

depend: RoundTrip_Idl-depend RoundTrip_Ping-depend RoundTrip_Pong-depend

REMAINING_TARGETS := $(filter-out all depend,$(TARGETS_NESTED:.nested=)) $(CUSTOM_TARGETS)

$(REMAINING_TARGETS):
	$(KEEP_GOING)@$(MAKE) -f GNUmakefile.RoundTrip_Idl $(@)
	$(KEEP_GOING)@$(MAKE) -f GNUmakefile.RoundTrip_Ping $(@)
	$(KEEP_GOING)@$(MAKE) -f GNUmakefile.RoundTrip_Pong $(@)

.PHONY: RoundTrip_Idl
RoundTrip_Idl:
	$(KEEP_GOING)@$(MAKE) -f GNUmakefile.RoundTrip_Idl all

.PHONY: RoundTrip_Idl-depend
RoundTrip_Idl-depend:
	$(KEEP_GOING)@$(MAKE) -f GNUmakefile.RoundTrip_Idl depend

.PHONY: RoundTrip_Ping
RoundTrip_Ping: RoundTrip_Idl
	$(KEEP_GOING)@$(MAKE) -f GNUmakefile.RoundTrip_Ping all

.PHONY: RoundTrip_Ping-depend
RoundTrip_Ping-depend:
	$(KEEP_GOING)@$(MAKE) -f GNUmakefile.RoundTrip_Ping depend

.PHONY: RoundTrip_Pong
RoundTrip_Pong: RoundTrip_Ping
	$(KEEP_GOING)@$(MAKE) -f GNUmakefile.RoundTrip_Pong all

.PHONY: RoundTrip_Pong-depend
RoundTrip_Pong-depend:
	$(KEEP_GOING)@$(MAKE) -f GNUmakefile.RoundTrip_Pong depend

project_name_list:
	@echo RoundTrip_Idl
	@echo RoundTrip_Ping
	@echo RoundTrip_Pong
