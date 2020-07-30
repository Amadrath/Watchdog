#!/home/countzero/ChimeraTK/install/bin/watchdog-server

#< envPaths

## Register all support components
dbLoadDatabase("../dbd/ChimeraTK.dbd",0,0)
ChimeraTK_registerRecordDeviceDriver(pdbbase)

chimeraTKConfigureApplication("ChimeraTKApp", 100)

## Setup autosave
set_requestfile_path("../db/")
set_savefile_path("../autosave/")

## Load record instances
dbLoadRecords("../db/WatchdogServer-main.db", "SERVER=Tst")
dbLoadRecords("../db/WatchdogServer-filesystem.db", "SERVER=Tst, F=0")
dbLoadRecords("../db/WatchdogServer-filesystem.db", "SERVER=Tst, F=1")
dbLoadRecords("../db/WatchdogServer-network.db", "SERVER=Tst, F=0")
dbLoadRecords("../db/WatchdogServer-network.db", "SERVER=Tst, F=1")
dbLoadRecords("../db/WatchdogServer-proc.db", "SERVER=Tst, P=0")
dbLoadRecords("../db/WatchdogServer-proc.db", "SERVER=Tst, P=1")
dbLoadRecords("../db/WatchdogServer-proc.db", "SERVER=Tst, P=2")
dbLoadRecords("../db/WatchdogServer-proc.db", "SERVER=Tst, P=3")
dbLoadRecords("../db/WatchdogServer-proc.db", "SERVER=Tst, P=4")
dbLoadRecords("../db/WatchdogServer-proc.db", "SERVER=Tst, P=5")
dbLoadRecords("../db/WatchdogServer-proc.db", "SERVER=Tst, P=6")
dbLoadRecords("../db/WatchdogServer-proc.db", "SERVER=Tst, P=7")

save_restoreSet_DatedBackupFiles(0)
save_restoreSet_Debug(0)
save_restoreSet_NumSeqFiles(3)
save_restoreSet_SeqPeriodInSeconds(3600)
#set_pass0_restoreFile("WatchdogServer-main.sav", "SERVER=Tst")
set_pass1_restoreFile("WatchdogServer-main.sav", "SERVER=Tst")
#set_pass0_restoreFile("Processes.sav", "SERVER=Tst")
set_pass1_restoreFile("Processes.sav", "SERVER=Tst")

iocInit()

#postEvent 1

create_monitor_set("WatchdogServer-main.req", 60, "SERVER=Tst")
create_monitor_set("Processes.req", 60, "SERVER=Tst")
