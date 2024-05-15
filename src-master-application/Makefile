## Makefile for iol-hat

## Release
ProjectName            :=iol-hat
ConfigurationName      :=Release
WorkspaceConfiguration :=Release
IntermediateDirectory  :=./build-$(WorkspaceConfiguration)/
OutDir                 :=$(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
MakeDirCommand         :=mkdir -p
LinkerName             :=/usr/bin/aarch64-linux-gnu-g++
SharedObjectLinkerName :=/usr/bin/aarch64-linux-gnu-g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputDirectory        := .
OutputFile             :=$(ProjectName)
Preprocessors          :=$(PreprocessorSwitch)NDEBUG 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :=$(IntermediateDirectory)/ObjectsList.txt
PCHCompileFlags        :=
LinkOptions            :=  -g  -pthread   -lrt
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)./src/ $(IncludeSwitch)./include $(IncludeSwitch)./src/ilink $(IncludeSwitch)./src/osal 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)gpiod 
ArLibs                 :=  "gpiod" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)./lib/ 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overridden using an environment variable
##
AR       := ar rcu
CXX      := g++
CC       := gcc
CXXFLAGS :=   -Wno-psabi  -Wall   -std=c++17 -g   -lrt   -pthread  -O2 $(Preprocessors)
CFLAGS   :=   -Wno-psabi  -Wall   -g   -pthread   -lrt   -O2 $(Preprocessors)
ASFLAGS  := 
AS       := as


##
## User defined environment variables
##
Objects0=$(IntermediateDirectory)/src_ilink_iolink_cm.c$(ObjectSuffix) $(IntermediateDirectory)/src_ilink_iolink_dl.c$(ObjectSuffix) $(IntermediateDirectory)/src_ilink_iolink_ds.c$(ObjectSuffix) $(IntermediateDirectory)/src_ilink_iolink_ode.c$(ObjectSuffix) $(IntermediateDirectory)/src_iolink_smi.cc$(ObjectSuffix) $(IntermediateDirectory)/src_iol_master_sample_app.cc$(ObjectSuffix) $(IntermediateDirectory)/src_osal_osal_log.c$(ObjectSuffix) $(IntermediateDirectory)/src_status.cc$(ObjectSuffix) $(IntermediateDirectory)/src_realtime.cc$(ObjectSuffix) $(IntermediateDirectory)/src_iolink_ifm.cc$(ObjectSuffix) \
	$(IntermediateDirectory)/src_server.cc$(ObjectSuffix) $(IntermediateDirectory)/src_iolink_handler.cc$(ObjectSuffix) $(IntermediateDirectory)/src_osal_osal_fileops.c$(ObjectSuffix) $(IntermediateDirectory)/src_osal_osal.c$(ObjectSuffix) $(IntermediateDirectory)/src_ilink_iolink_max14819_pl.c$(ObjectSuffix) $(IntermediateDirectory)/src_osal_osal_drv.c$(ObjectSuffix) $(IntermediateDirectory)/src_ilink_iolink_sm.c$(ObjectSuffix) $(IntermediateDirectory)/src_ilink_iolink_pde.c$(ObjectSuffix) $(IntermediateDirectory)/src_ilink_iolink_main.c$(ObjectSuffix) $(IntermediateDirectory)/src_ilink_iolink_pl_hw.c$(ObjectSuffix) \
	$(IntermediateDirectory)/src_ilink_iolink_al.c$(ObjectSuffix) $(IntermediateDirectory)/src_ilink_iolink_pl.c$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) "$(IntermediateDirectory)"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

PostBuild:
	@echo Executing Post Build commands ...
	/usr/bin/aarch64-linux-gnu-strip  -s $(OutputFile)
	@echo Done

MakeIntermediateDirs:
	@$(MakeDirCommand) "$(IntermediateDirectory)"
	@$(MakeDirCommand) "$(OutputDirectory)"

$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "$(IntermediateDirectory)"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/src_ilink_iolink_cm.c$(ObjectSuffix): src/ilink/iolink_cm.c $(IntermediateDirectory)/src_ilink_iolink_cm.c$(DependSuffix)
	$(CC) $(SourceSwitch) "./src/ilink/iolink_cm.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ilink_iolink_cm.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ilink_iolink_cm.c$(DependSuffix): src/ilink/iolink_cm.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ilink_iolink_cm.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ilink_iolink_cm.c$(DependSuffix) -MM src/ilink/iolink_cm.c

$(IntermediateDirectory)/src_ilink_iolink_cm.c$(PreprocessSuffix): src/ilink/iolink_cm.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ilink_iolink_cm.c$(PreprocessSuffix) src/ilink/iolink_cm.c

$(IntermediateDirectory)/src_ilink_iolink_dl.c$(ObjectSuffix): src/ilink/iolink_dl.c $(IntermediateDirectory)/src_ilink_iolink_dl.c$(DependSuffix)
	$(CC) $(SourceSwitch) "./src/ilink/iolink_dl.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ilink_iolink_dl.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ilink_iolink_dl.c$(DependSuffix): src/ilink/iolink_dl.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ilink_iolink_dl.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ilink_iolink_dl.c$(DependSuffix) -MM src/ilink/iolink_dl.c

$(IntermediateDirectory)/src_ilink_iolink_dl.c$(PreprocessSuffix): src/ilink/iolink_dl.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ilink_iolink_dl.c$(PreprocessSuffix) src/ilink/iolink_dl.c

$(IntermediateDirectory)/src_ilink_iolink_ds.c$(ObjectSuffix): src/ilink/iolink_ds.c $(IntermediateDirectory)/src_ilink_iolink_ds.c$(DependSuffix)
	$(CC) $(SourceSwitch) "./src/ilink/iolink_ds.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ilink_iolink_ds.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ilink_iolink_ds.c$(DependSuffix): src/ilink/iolink_ds.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ilink_iolink_ds.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ilink_iolink_ds.c$(DependSuffix) -MM src/ilink/iolink_ds.c

$(IntermediateDirectory)/src_ilink_iolink_ds.c$(PreprocessSuffix): src/ilink/iolink_ds.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ilink_iolink_ds.c$(PreprocessSuffix) src/ilink/iolink_ds.c

$(IntermediateDirectory)/src_ilink_iolink_ode.c$(ObjectSuffix): src/ilink/iolink_ode.c $(IntermediateDirectory)/src_ilink_iolink_ode.c$(DependSuffix)
	$(CC) $(SourceSwitch) "./src/ilink/iolink_ode.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ilink_iolink_ode.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ilink_iolink_ode.c$(DependSuffix): src/ilink/iolink_ode.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ilink_iolink_ode.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ilink_iolink_ode.c$(DependSuffix) -MM src/ilink/iolink_ode.c

$(IntermediateDirectory)/src_ilink_iolink_ode.c$(PreprocessSuffix): src/ilink/iolink_ode.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ilink_iolink_ode.c$(PreprocessSuffix) src/ilink/iolink_ode.c

$(IntermediateDirectory)/src_iolink_smi.cc$(ObjectSuffix): src/iolink_smi.cc $(IntermediateDirectory)/src_iolink_smi.cc$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "./src/iolink_smi.cc" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_iolink_smi.cc$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_iolink_smi.cc$(DependSuffix): src/iolink_smi.cc
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_iolink_smi.cc$(ObjectSuffix) -MF$(IntermediateDirectory)/src_iolink_smi.cc$(DependSuffix) -MM src/iolink_smi.cc

$(IntermediateDirectory)/src_iolink_smi.cc$(PreprocessSuffix): src/iolink_smi.cc
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_iolink_smi.cc$(PreprocessSuffix) src/iolink_smi.cc

$(IntermediateDirectory)/src_iol_master_sample_app.cc$(ObjectSuffix): src/iol_master_sample_app.cc $(IntermediateDirectory)/src_iol_master_sample_app.cc$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "./src/iol_master_sample_app.cc" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_iol_master_sample_app.cc$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_iol_master_sample_app.cc$(DependSuffix): src/iol_master_sample_app.cc
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_iol_master_sample_app.cc$(ObjectSuffix) -MF$(IntermediateDirectory)/src_iol_master_sample_app.cc$(DependSuffix) -MM src/iol_master_sample_app.cc

$(IntermediateDirectory)/src_iol_master_sample_app.cc$(PreprocessSuffix): src/iol_master_sample_app.cc
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_iol_master_sample_app.cc$(PreprocessSuffix) src/iol_master_sample_app.cc

$(IntermediateDirectory)/src_osal_osal_log.c$(ObjectSuffix): src/osal/osal_log.c $(IntermediateDirectory)/src_osal_osal_log.c$(DependSuffix)
	$(CC) $(SourceSwitch) "./src/osal/osal_log.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_osal_osal_log.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_osal_osal_log.c$(DependSuffix): src/osal/osal_log.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_osal_osal_log.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_osal_osal_log.c$(DependSuffix) -MM src/osal/osal_log.c

$(IntermediateDirectory)/src_osal_osal_log.c$(PreprocessSuffix): src/osal/osal_log.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_osal_osal_log.c$(PreprocessSuffix) src/osal/osal_log.c

$(IntermediateDirectory)/src_status.cc$(ObjectSuffix): src/status.cc $(IntermediateDirectory)/src_status.cc$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "./src/status.cc" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_status.cc$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_status.cc$(DependSuffix): src/status.cc
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_status.cc$(ObjectSuffix) -MF$(IntermediateDirectory)/src_status.cc$(DependSuffix) -MM src/status.cc

$(IntermediateDirectory)/src_status.cc$(PreprocessSuffix): src/status.cc
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_status.cc$(PreprocessSuffix) src/status.cc

$(IntermediateDirectory)/src_realtime.cc$(ObjectSuffix): src/realtime.cc $(IntermediateDirectory)/src_realtime.cc$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "./src/realtime.cc" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_realtime.cc$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_realtime.cc$(DependSuffix): src/realtime.cc
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_realtime.cc$(ObjectSuffix) -MF$(IntermediateDirectory)/src_realtime.cc$(DependSuffix) -MM src/realtime.cc

$(IntermediateDirectory)/src_realtime.cc$(PreprocessSuffix): src/realtime.cc
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_realtime.cc$(PreprocessSuffix) src/realtime.cc

$(IntermediateDirectory)/src_iolink_ifm.cc$(ObjectSuffix): src/iolink_ifm.cc $(IntermediateDirectory)/src_iolink_ifm.cc$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "./src/iolink_ifm.cc" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_iolink_ifm.cc$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_iolink_ifm.cc$(DependSuffix): src/iolink_ifm.cc
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_iolink_ifm.cc$(ObjectSuffix) -MF$(IntermediateDirectory)/src_iolink_ifm.cc$(DependSuffix) -MM src/iolink_ifm.cc

$(IntermediateDirectory)/src_iolink_ifm.cc$(PreprocessSuffix): src/iolink_ifm.cc
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_iolink_ifm.cc$(PreprocessSuffix) src/iolink_ifm.cc

$(IntermediateDirectory)/src_server.cc$(ObjectSuffix): src/server.cc $(IntermediateDirectory)/src_server.cc$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "./src/server.cc" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_server.cc$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_server.cc$(DependSuffix): src/server.cc
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_server.cc$(ObjectSuffix) -MF$(IntermediateDirectory)/src_server.cc$(DependSuffix) -MM src/server.cc

$(IntermediateDirectory)/src_server.cc$(PreprocessSuffix): src/server.cc
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_server.cc$(PreprocessSuffix) src/server.cc

$(IntermediateDirectory)/src_iolink_handler.cc$(ObjectSuffix): src/iolink_handler.cc $(IntermediateDirectory)/src_iolink_handler.cc$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "./src/iolink_handler.cc" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_iolink_handler.cc$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_iolink_handler.cc$(DependSuffix): src/iolink_handler.cc
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_iolink_handler.cc$(ObjectSuffix) -MF$(IntermediateDirectory)/src_iolink_handler.cc$(DependSuffix) -MM src/iolink_handler.cc

$(IntermediateDirectory)/src_iolink_handler.cc$(PreprocessSuffix): src/iolink_handler.cc
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_iolink_handler.cc$(PreprocessSuffix) src/iolink_handler.cc

$(IntermediateDirectory)/src_osal_osal_fileops.c$(ObjectSuffix): src/osal/osal_fileops.c $(IntermediateDirectory)/src_osal_osal_fileops.c$(DependSuffix)
	$(CC) $(SourceSwitch) "./src/osal/osal_fileops.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_osal_osal_fileops.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_osal_osal_fileops.c$(DependSuffix): src/osal/osal_fileops.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_osal_osal_fileops.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_osal_osal_fileops.c$(DependSuffix) -MM src/osal/osal_fileops.c

$(IntermediateDirectory)/src_osal_osal_fileops.c$(PreprocessSuffix): src/osal/osal_fileops.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_osal_osal_fileops.c$(PreprocessSuffix) src/osal/osal_fileops.c

$(IntermediateDirectory)/src_osal_osal.c$(ObjectSuffix): src/osal/osal.c $(IntermediateDirectory)/src_osal_osal.c$(DependSuffix)
	$(CC) $(SourceSwitch) "./src/osal/osal.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_osal_osal.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_osal_osal.c$(DependSuffix): src/osal/osal.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_osal_osal.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_osal_osal.c$(DependSuffix) -MM src/osal/osal.c

$(IntermediateDirectory)/src_osal_osal.c$(PreprocessSuffix): src/osal/osal.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_osal_osal.c$(PreprocessSuffix) src/osal/osal.c

$(IntermediateDirectory)/src_ilink_iolink_max14819_pl.c$(ObjectSuffix): src/ilink/iolink_max14819_pl.c $(IntermediateDirectory)/src_ilink_iolink_max14819_pl.c$(DependSuffix)
	$(CC) $(SourceSwitch) "./src/ilink/iolink_max14819_pl.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ilink_iolink_max14819_pl.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ilink_iolink_max14819_pl.c$(DependSuffix): src/ilink/iolink_max14819_pl.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ilink_iolink_max14819_pl.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ilink_iolink_max14819_pl.c$(DependSuffix) -MM src/ilink/iolink_max14819_pl.c

$(IntermediateDirectory)/src_ilink_iolink_max14819_pl.c$(PreprocessSuffix): src/ilink/iolink_max14819_pl.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ilink_iolink_max14819_pl.c$(PreprocessSuffix) src/ilink/iolink_max14819_pl.c

$(IntermediateDirectory)/src_osal_osal_drv.c$(ObjectSuffix): src/osal/osal_drv.c $(IntermediateDirectory)/src_osal_osal_drv.c$(DependSuffix)
	$(CC) $(SourceSwitch) "./src/osal/osal_drv.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_osal_osal_drv.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_osal_osal_drv.c$(DependSuffix): src/osal/osal_drv.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_osal_osal_drv.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_osal_osal_drv.c$(DependSuffix) -MM src/osal/osal_drv.c

$(IntermediateDirectory)/src_osal_osal_drv.c$(PreprocessSuffix): src/osal/osal_drv.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_osal_osal_drv.c$(PreprocessSuffix) src/osal/osal_drv.c

$(IntermediateDirectory)/src_ilink_iolink_sm.c$(ObjectSuffix): src/ilink/iolink_sm.c $(IntermediateDirectory)/src_ilink_iolink_sm.c$(DependSuffix)
	$(CC) $(SourceSwitch) "./src/ilink/iolink_sm.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ilink_iolink_sm.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ilink_iolink_sm.c$(DependSuffix): src/ilink/iolink_sm.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ilink_iolink_sm.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ilink_iolink_sm.c$(DependSuffix) -MM src/ilink/iolink_sm.c

$(IntermediateDirectory)/src_ilink_iolink_sm.c$(PreprocessSuffix): src/ilink/iolink_sm.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ilink_iolink_sm.c$(PreprocessSuffix) src/ilink/iolink_sm.c

$(IntermediateDirectory)/src_ilink_iolink_pde.c$(ObjectSuffix): src/ilink/iolink_pde.c $(IntermediateDirectory)/src_ilink_iolink_pde.c$(DependSuffix)
	$(CC) $(SourceSwitch) "./src/ilink/iolink_pde.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ilink_iolink_pde.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ilink_iolink_pde.c$(DependSuffix): src/ilink/iolink_pde.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ilink_iolink_pde.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ilink_iolink_pde.c$(DependSuffix) -MM src/ilink/iolink_pde.c

$(IntermediateDirectory)/src_ilink_iolink_pde.c$(PreprocessSuffix): src/ilink/iolink_pde.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ilink_iolink_pde.c$(PreprocessSuffix) src/ilink/iolink_pde.c

$(IntermediateDirectory)/src_ilink_iolink_main.c$(ObjectSuffix): src/ilink/iolink_main.c $(IntermediateDirectory)/src_ilink_iolink_main.c$(DependSuffix)
	$(CC) $(SourceSwitch) "./src/ilink/iolink_main.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ilink_iolink_main.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ilink_iolink_main.c$(DependSuffix): src/ilink/iolink_main.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ilink_iolink_main.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ilink_iolink_main.c$(DependSuffix) -MM src/ilink/iolink_main.c

$(IntermediateDirectory)/src_ilink_iolink_main.c$(PreprocessSuffix): src/ilink/iolink_main.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ilink_iolink_main.c$(PreprocessSuffix) src/ilink/iolink_main.c

$(IntermediateDirectory)/src_ilink_iolink_pl_hw.c$(ObjectSuffix): src/ilink/iolink_pl_hw.c $(IntermediateDirectory)/src_ilink_iolink_pl_hw.c$(DependSuffix)
	$(CC) $(SourceSwitch) "./src/ilink/iolink_pl_hw.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ilink_iolink_pl_hw.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ilink_iolink_pl_hw.c$(DependSuffix): src/ilink/iolink_pl_hw.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ilink_iolink_pl_hw.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ilink_iolink_pl_hw.c$(DependSuffix) -MM src/ilink/iolink_pl_hw.c

$(IntermediateDirectory)/src_ilink_iolink_pl_hw.c$(PreprocessSuffix): src/ilink/iolink_pl_hw.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ilink_iolink_pl_hw.c$(PreprocessSuffix) src/ilink/iolink_pl_hw.c

$(IntermediateDirectory)/src_ilink_iolink_al.c$(ObjectSuffix): src/ilink/iolink_al.c $(IntermediateDirectory)/src_ilink_iolink_al.c$(DependSuffix)
	$(CC) $(SourceSwitch) "./src/ilink/iolink_al.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ilink_iolink_al.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ilink_iolink_al.c$(DependSuffix): src/ilink/iolink_al.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ilink_iolink_al.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ilink_iolink_al.c$(DependSuffix) -MM src/ilink/iolink_al.c

$(IntermediateDirectory)/src_ilink_iolink_al.c$(PreprocessSuffix): src/ilink/iolink_al.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ilink_iolink_al.c$(PreprocessSuffix) src/ilink/iolink_al.c

$(IntermediateDirectory)/src_ilink_iolink_pl.c$(ObjectSuffix): src/ilink/iolink_pl.c $(IntermediateDirectory)/src_ilink_iolink_pl.c$(DependSuffix)
	$(CC) $(SourceSwitch) "./src/ilink/iolink_pl.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ilink_iolink_pl.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ilink_iolink_pl.c$(DependSuffix): src/ilink/iolink_pl.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ilink_iolink_pl.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ilink_iolink_pl.c$(DependSuffix) -MM src/ilink/iolink_pl.c

$(IntermediateDirectory)/src_ilink_iolink_pl.c$(PreprocessSuffix): src/ilink/iolink_pl.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ilink_iolink_pl.c$(PreprocessSuffix) src/ilink/iolink_pl.c


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


