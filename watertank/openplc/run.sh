#!/bin/bash
SQL_SCRIPT="INSERT INTO Programs (Name, Description, File, Date_upload) VALUES ('PLC1', 'Descrizione', 'PLC1.st', strftime('%s', 'now'));"
#SQL_DEVICE="INSERT INTO Slave_dev (dev_name, dev_type, slave_id, ip_address, ip_port, di_start, di_size, coil_start, coil_size, ir_start, ir_size, hr_read_start, hr_read_size, hr_write_start, hr_write_size) VALUES ('Testdevice', 'TCP', 15, '127.0.0.1', 502, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);"
SQL_AUTOST="UPDATE Settings SET Value = 'true' WHERE Key = 'Start_run_mode';"

FILE=/home/OpenPLC_v3/started.txt

if ! [[ -f "$FILE" ]]; then
    

    #sudo rm /home/OpenPLC_v3/webserver/active_program
    #echo PLC.st >> /home/OpenPLC_v3/webserver/active_program

    #cp /home/OpenPLC_v3/scripts/mbconfig.cfg /home/OpenPLC_v3/webserver
    #sqlite3 /home/OpenPLC_v3/webserver/openplc.db "$SQL_DEVICE"

    sqlite3 /home/OpenPLC_v3/webserver/openplc.db "$SQL_AUTOST"
   
    #creazione file primo accesso eseguito
    echo "Don't remove this file" >> /home/OpenPLC_v3/started.txt
    
    #copia script e esecuzione query
    cp /home/OpenPLC_v3/scripts/PLC1.st /home/OpenPLC_v3/webserver/st_files
    sqlite3 /home/OpenPLC_v3/webserver/openplc.db "$SQL_SCRIPT"

    #cambio hardware e programma di start
   
    sudo rm /home/OpenPLC_v3/webserver/active_program
    sudo rm /home/OpenPLC_v3/webserver/scripts/openplc_driver
    echo PLC1.st >> /home/OpenPLC_v3/webserver/active_program
    echo simulink_linux >> /home/OpenPLC_v3/webserver/scripts/openplc_driver
    
fi 

sudo /home/OpenPLC_v3/start_openplc.sh