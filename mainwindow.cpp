#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QSettings>
#include <QMessageBox>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QDateTime>

#include "./src_libmodbus/modbus.h"

#define PRILUKI
//#define GGPZ


#ifdef PRILUKI
#define SERVERFLAG 83
#define SERVERFLAG1UVR2NORD 23
#define SERVERFLAG4UVR 33
#define SERVERFLAGGAZ 47
#endif

#ifdef GGPZ
#define SERVERFLAG 84
#define SERVERFLAG1UVR2NORD 24
#define SERVERFLAG4UVR 34
#define SERVERFLAGGAZ 48
#endif



struct uzel
{
    QString IP_addr;
    QString OdbcName;
    QString TableName;
    QString text;
    QString type;  //"vosn","1uvr2nord","4uvr","gaz"
} uzels[]=
{
#ifdef PRILUKI

    "172.16.223.2","fire_lelaki","LELAKI_SOU","waiting queue...","vosn",

    "172.16.57.69","fire_ggpz_gned","GGPZ_GNED_SOU","waiting queue...","vosn",
    "172.16.57.70","fire_ggpz_lelaki_vhod","GGPZ_LELAKI_VHOD_SOU","waiting queue...","vosn",
//    "172.16.57.72","fire_ggpz_lelaki_vihod","GGPZ_LELAKI_VIHOD_SOU","waiting queue...","vosn",

    "172.16.57.75","fire_ggpz_z_os","GGPZ_Z_OS_SOU","waiting queue...","vosn",
    "172.16.57.76","fire_ggpz_z_ups","GGPZ_Z_UPS_SOU","waiting queue...","vosn",
    "172.16.57.78","fire_ggpz_z_ksu","GGPZ_Z_KSU_SOU","waiting queue...","1uvr2nord",
    "172.16.57.20","fire_gzu1_gned","GZU1_GNED_SOU","waiting queue...","vosn",
    "172.16.57.74","fire_ggpz_4vov","GGPZ_4VOV_SOU","waiting queue...","4uvr",
    "172.16.48.100","fire_yaroshiv","YAROSHIV_SOU","waiting queue...","vosn",
    "172.16.48.102","fire_talal_z_yaroshiv","TALAL_Z_YAROSHIV_SOU","waiting queue...","vosn",
    "172.16.57.72","fire_ggpz_gaz","GGPZ_GAZ","waiting queue...","gaz",

#endif

#ifdef GGPZ

        "172.16.223.2","fire_lelaki","LELAKI_SOU","waiting queue...","vosn",

        "172.16.57.69","fire_ggpz_gned","GGPZ_GNED_SOU","waiting queue...","vosn",
        "172.16.57.70","fire_ggpz_lelaki_vhod","GGPZ_LELAKI_VHOD_SOU","waiting queue...","vosn",
    //    "172.16.57.72","fire_ggpz_lelaki_vihod","GGPZ_LELAKI_VIHOD_SOU","waiting queue...","vosn",

        "172.16.57.75","fire_ggpz_z_os","GGPZ_Z_OS_SOU","waiting queue...","vosn",
        "172.16.57.76","fire_ggpz_z_ups","GGPZ_Z_UPS_SOU","waiting queue...","vosn",
        "172.16.57.78","fire_ggpz_z_ksu","GGPZ_Z_KSU_SOU","waiting queue...","1uvr2nord",
        "172.16.57.20","fire_gzu1_gned","GZU1_GNED_SOU","waiting queue...","vosn",
        "172.16.57.74","fire_ggpz_4vov","GGPZ_4VOV_SOU","waiting queue...","4uvr",
        "172.16.48.100","fire_yaroshiv","YAROSHIV_SOU","waiting queue...","vosn",
        "172.16.48.102","fire_talal_z_yaroshiv","TALAL_Z_YAROSHIV_SOU","waiting queue...","vosn",
        "172.16.57.72","fire_ggpz_gaz","GGPZ_GAZ","waiting queue...","gaz",
#endif



};
//==========================================================================

ThreadPollObjects pollThread;
bool SaveToDB();

//==========================================================================
QString GetNextName()
{
static int counter=1;
QString res;
res.sprintf("%u",counter);
counter=(counter+1) % 1000000;
return res;
}
//===========================================================================

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pushButtonRead,SIGNAL(clicked()),this,SLOT(PushButtonRead()));
    connect(&pollThread,SIGNAL(textchange(int,QString)),this,SLOT(TextChanged(int,QString)));
    connect(&pollThread,SIGNAL(insert(QString)),this,SLOT(InsertChanged(QString)));
    connect(ui->pushButtonClose,SIGNAL(clicked()),this,SLOT(close()));

    for(int i=0;i<sizeof(uzels)/sizeof(uzel);++i)
    {
        ui->listWidget->addItem(uzels[i].IP_addr+":modbus -> ODBC:" + uzels[i].OdbcName+", TABLE:"+uzels[i].TableName+ " === " + uzels[i].text);
    }

    pollThread.start(QThread::InheritPriority);

}
//===========================================================================
MainWindow::~MainWindow()
{
    delete ui;
}
//===========================================================================
void MainWindow::PushButtonRead()
{




}
//==========================================================================
void MainWindow::TextChanged(int iUzel, QString newText)
{


 uzels[iUzel].text=newText;
 ui->listWidget->item(iUzel)->setText( uzels[iUzel].IP_addr+":modbus -> ODBC:" + uzels[iUzel].OdbcName+", TABLE:"+uzels[iUzel].TableName+ " === " + uzels[iUzel].text +
                                       "   " + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
}


//==========================================================================
void ThreadPollObjects::run()
{

for(;;)
{


    for(int i=0;i<sizeof(uzels)/sizeof(uzel);++i)
    {

        if (uzels[i].type=="vosn") PollVosn(i);
        if (uzels[i].type=="1uvr2nord") Poll1Uvr2Nord(i);
        if (uzels[i].type=="4uvr") Poll4Uvr(i);
        if (uzels[i].type=="gaz") PollGaz(i);

        if (CheckThreadStop()) return;
    }

    for(int i=0;i<60;++i) //1 min delay
    {
    if (CheckThreadStop()) return;
    Sleep(1000);
    }

}//for(;;)


}
//===============================================================================
void MainWindow::InsertChanged(QString Text)
{
  //  ui->textEdit->setText(Text);
    ui->textEdit->append(Text+'\n');

}
//===============================================================================
void ThreadPollObjects::PollVosn(int i)
{

    modbus_t *mb;
    uint16_t tab_reg[100];


    uint16_t year;
    uint16_t month;
    uint16_t day;
    uint16_t hour;
    float GustPlastVod;
    float GustSepNaf;
    float GustGazuSU;
    float Gazovmist;
    float ObjomVilnGazu;
    float DiametrTrubopr;
    float KoeffZ;

    //masstotal
    float mass_total;

    //средних, делить на cycle_count
    float d_rFlowMM_avg;
    float d_rTemp_avg;
    float d_rTisk_avg;
    float d_rObvodn_opos_avg;
    float d_rDens_avg;

    //накопленные
    float d_rMasRid_MM_SOU;
    float d_rMasNaf_MM_SOU;
    float d_rObjemRidRU_MM_SOU;
    float d_rObjemNafRU_MM_SOU;  //d_rObjemGazonasZnevodnNafRU_MM_SOU
    float d_rObjemNafSU_MM_SOU;

    //накопленные - added for PIP-VSN
    float d_rObvodn_avg; //added for PIP-VSN
    float d_rMasRid_MM_SOU_PIP;
    float d_rMasNaf_MM_SOU_PIP;
    float d_rObjemRidRU_MM_SOU_PIP;
    float d_rObjemNafRU_MM_SOU_PIP;   //d_rObjemGazonasZnevodnNafRU_MM_SOU_PIP
    float d_rObjemNafSU_MM_SOU_PIP;

    //added for NORD1

    float d_rFlowNORD1_avg;
    // NORD1+SOU
    float d_rObjemRid_NORD1_SOU;
    float d_rObjemRidRU_NORD1_SOU;
    float d_rMassNaf_NORD1_SOU;
    float d_rObjomNafSU_NORD1_SOU;
    float d_rObjomNafGnRU_NORD1_SOU;


    //added for NORD1+PIP
    float d_rObjemRid_NORD1_SOU_PIP;
    float d_rObjemRidRU_NORD1_SOU_PIP;
    float d_rMassNaf_NORD1_SOU_PIP;
    float d_rObjomNafSU_NORD1_SOU_PIP;
    float d_rObjomNafGnRU_NORD1_SOU_PIP;


    uint16_t alarmsCode;
    uint16_t alarmsTimeSec;
    uint16_t WhichKoeffSaved;

    uint16_t flagCitect;
    uint16_t flagWeintek;
    uint16_t flagServerPRILUKI;
    uint16_t flagServerGGPZ;

    if (uzels[i].IP_addr.isEmpty())
    {
        //QMessageBox::information(this,"Configuration message","Your configuration is incorrect - no IP address!!!",QMessageBox::Ok);
        emit textchange(i, "ERROR-No IP adress!!!");
        return;
    }

    mb = modbus_new_tcp(uzels[i].IP_addr.toStdString().c_str(), 502);

    if (modbus_connect(mb)!=0 )
    {
        //QMessageBox::information(this,"Test","Зв'язок з об'єктом відсутній!!!",QMessageBox::Ok);
        emit textchange(i, "No connection with source");
    }
    else //connect OK
    {
        modbus_set_slave(mb, 1);
        // Read 85 registers from the address from 40301
        int res=modbus_read_registers(mb, 300, 85, tab_reg);

        if (res!=85)
        {
            emit textchange(i, "ERROR: modbus read error");

        }
        else //read OK
        {

            year=tab_reg[0];
            month=tab_reg[1];
            day=tab_reg[2];
            hour=tab_reg[3];
            GustPlastVod=modbus_get_float(&tab_reg[4]);
            GustSepNaf=modbus_get_float(&tab_reg[6]);
            GustGazuSU=modbus_get_float(&tab_reg[8]);
            Gazovmist=modbus_get_float(&tab_reg[10]);
            ObjomVilnGazu=modbus_get_float(&tab_reg[12]);
            DiametrTrubopr=modbus_get_float(&tab_reg[14]);
            KoeffZ=modbus_get_float(&tab_reg[16]);
            //masstotal
            mass_total=modbus_get_float(&tab_reg[18]);

            //средних, делить на cycle_count
            d_rFlowMM_avg=modbus_get_float(&tab_reg[20]);
            d_rTemp_avg=modbus_get_float(&tab_reg[22]);
            d_rTisk_avg=modbus_get_float(&tab_reg[24]);
            d_rObvodn_opos_avg=modbus_get_float(&tab_reg[26]);
            d_rDens_avg=modbus_get_float(&tab_reg[28]);

            //накопленные
            d_rMasRid_MM_SOU=modbus_get_float(&tab_reg[30]);
            d_rMasNaf_MM_SOU=modbus_get_float(&tab_reg[32]);
            d_rObjemRidRU_MM_SOU=modbus_get_float(&tab_reg[34]);
            d_rObjemNafRU_MM_SOU=modbus_get_float(&tab_reg[36]);  //d_rObjemGazonasZnevodnNafRU_MM_SOU
            d_rObjemNafSU_MM_SOU=modbus_get_float(&tab_reg[38]);

            //накопленные - added for PIP-VSN
            d_rObvodn_avg=modbus_get_float(&tab_reg[40]); //added for PIP-VSN
            d_rMasRid_MM_SOU_PIP=modbus_get_float(&tab_reg[42]);
            d_rMasNaf_MM_SOU_PIP=modbus_get_float(&tab_reg[44]);
            d_rObjemRidRU_MM_SOU_PIP=modbus_get_float(&tab_reg[46]);
            d_rObjemNafRU_MM_SOU_PIP=modbus_get_float(&tab_reg[48]);   //d_rObjemGazonasZnevodnNafRU_MM_SOU_PIP
            d_rObjemNafSU_MM_SOU_PIP=modbus_get_float(&tab_reg[50]);

            //added for NORD1

            d_rFlowNORD1_avg=modbus_get_float(&tab_reg[52]);
            // NORD1+SOU
            d_rObjemRid_NORD1_SOU=modbus_get_float(&tab_reg[54]);
            d_rObjemRidRU_NORD1_SOU=modbus_get_float(&tab_reg[56]);
            d_rMassNaf_NORD1_SOU=modbus_get_float(&tab_reg[58]);
            d_rObjomNafSU_NORD1_SOU=modbus_get_float(&tab_reg[60]);
            d_rObjomNafGnRU_NORD1_SOU=modbus_get_float(&tab_reg[62]);


            //added for NORD1+PIP
            d_rObjemRid_NORD1_SOU_PIP=modbus_get_float(&tab_reg[64]);
            d_rObjemRidRU_NORD1_SOU_PIP=modbus_get_float(&tab_reg[66]);
            d_rMassNaf_NORD1_SOU_PIP=modbus_get_float(&tab_reg[68]);
            d_rObjomNafSU_NORD1_SOU_PIP=modbus_get_float(&tab_reg[70]);
            d_rObjomNafGnRU_NORD1_SOU_PIP=modbus_get_float(&tab_reg[72]);  //40373


            //rezerv1 =modbus_get_float(&tab_reg[74]); 40375
            //rezerv2 =modbus_get_float(&tab_reg[76]); 40377


            alarmsCode=tab_reg[78];
            alarmsTimeSec=tab_reg[79];
            WhichKoeffSaved=tab_reg[80];

            flagCitect=tab_reg[81];
            flagWeintek=tab_reg[82];
            flagServerPRILUKI=tab_reg[83];  //Priluki server flag defined 83  (40384)
            flagServerGGPZ=tab_reg[84];     //ggpz defined 84                  (40385)


            if (tab_reg[SERVERFLAG]==1)
            {

               //QString tmp;

               //tmp.sprintf("%i %i %i %i\n%.2f %.2f %.2f %.2f %.2f %.2f %.2f\n%i %i %i",year,month,day,hour,
        //GustPlastVod,GustSepNaf,GustGazuSU,Gazovmist,ObjomVilnGazu,DiametrTrubopr,KoeffZ,flagCitect,flagWeintek,flagServer);
                //QMessageBox::information(this,"Test",tmp,QMessageBox::Ok);
        // reset flag flagServer


                    // insert to db
                    QString connectionName=GetNextName();

                        {  // start of the block where the db object lives

                            QSqlDatabase db = QSqlDatabase::addDatabase("QODBC",connectionName);
                                  db.setDatabaseName(uzels[i].OdbcName);
                                  db.setUserName("sysdba");//user);
                                  db.setPassword("784523");//pass);
                            if (db.open())
                            {

                                QSqlQuery sqlQuery(db);
                                QString query;

                                tab_reg[SERVERFLAG]=0;
                                if (modbus_write_registers(mb, 300+SERVERFLAG, 1, &tab_reg[SERVERFLAG])==1)  //flag reset OK
                                {

                                    query.sprintf(QString("INSERT INTO " + uzels[i].TableName + "(" +
                                                  "DT, TEMP, TISK, OBVODN_OPOS, DENS, OBVODN, " +
                                                  "GUSTPLASTVOD, GUSTSEPNAF, GUSTGAZUSU, GAZOVMIST, OBJOMVILNGAZU, DIAMETRTRUBOPR, KOEFFZ, "+
                                                  "FLOWMM, MASSARIDMM, OBJEMRIDRUMM, OBJEMNAFMM, MASSANAFMM, "+
                                                  "MASSARIDMM_PIP, OBJEMRIDRUMM_PIP, OBJEMNAFMM_PIP, MASSANAFMM_PIP, "+
                                                  "FLOWNORD1, OBJEMRIDNORD1, OBJEMNAFSUNORD1, MASSANAFNORD1, OBJEMRIDRUNORD1, "+
                                                  "OBJEMRIDNORD1_PIP, OBJEMRIDRUNORD1_PIP, MASSANAFNORD1_PIP, OBJEMNAFSUNORD1_PIP, "+    //, LEVEL_E2, OBJEM_E2, DELTA_E2)
                                                  "ALARMSCODE , ALARMSTIMESEC, WHICHKOEFFSAVED) "
                                                  "VALUES ("+
                                                  "'%i.%i.%i %i:00:00', %f, %f, %f, %f, %f, "+
                                                  "%f, %f, %f, %f, %f, %f, %f, " +
                                                  "%f, %f, %f, %f, %f, "
                                                  "%f, %f, %f, %f, "
                                                  "%f, %f, %f, %f, %f, " +
                                                  "%f, %f, %f, %f,"
                                                  "%i, %i, %i)").toStdString().c_str(),
                                                  day,month,year,hour,d_rTemp_avg,d_rTisk_avg,d_rObvodn_opos_avg,d_rDens_avg,d_rObvodn_avg,
                                                  GustPlastVod,GustSepNaf,GustGazuSU,Gazovmist,ObjomVilnGazu,DiametrTrubopr,KoeffZ,
                                                  d_rFlowMM_avg,d_rMasRid_MM_SOU,d_rObjemRidRU_MM_SOU,d_rObjemNafSU_MM_SOU,d_rMasNaf_MM_SOU,
                                                  d_rMasRid_MM_SOU_PIP,d_rObjemRidRU_MM_SOU_PIP,d_rObjemNafSU_MM_SOU_PIP,d_rMasNaf_MM_SOU_PIP,
                                                  d_rFlowNORD1_avg,d_rObjemRid_NORD1_SOU,d_rObjomNafSU_NORD1_SOU,d_rMassNaf_NORD1_SOU,d_rObjemRidRU_NORD1_SOU,
                                                  d_rObjemRid_NORD1_SOU_PIP,d_rObjemRidRU_NORD1_SOU_PIP,d_rMassNaf_NORD1_SOU_PIP,d_rObjomNafSU_NORD1_SOU_PIP,
                                                  alarmsCode, alarmsTimeSec, WhichKoeffSaved
                                                  );

                                                  //  emit insert(query);

                                    sqlQuery.exec(query);

                                    if (sqlQuery.lastError().isValid())
                                    {
                                        //QMessageBox::critical(NULL, QObject::tr("Database Error"), sqlQuery.lastError().text());
                                        emit textchange(i,"ERROR: Database error on INSERT");
                                    }
                                    else
                                    {
                                        //QMessageBox::information(NULL, tr("Информация"), "Запись удалена успешно.");
                                        emit textchange(i,"INSERT OK");
                                    }

                                }
                                else
                                {
                                    emit textchange(i, "ERROR: cannot reset flag!!!");
                                }


                            }
                            else
                            {
                                //QMessageBox::critical(NULL, QObject::tr("Database Error"), db.lastError().text());
                                emit insert(db.lastError().databaseText());
                                emit insert(db.lastError().driverText());
                                emit textchange(i,"ERROR: Database not open");
                            }

                            db.close();
                        } // end of the block where the db object lives, it will be destroyed here

                        QSqlDatabase::removeDatabase(connectionName);

                    //insert to DB end




            }
            else
            {
                emit textchange(i, "No new data");
            }
        }
    }
    modbus_close(mb);
    modbus_free(mb);



}
//===============================================================================
void ThreadPollObjects::Poll1Uvr2Nord(int i)
{

    modbus_t *mb;
    uint16_t tab_reg[100];


    uint16_t year;
    uint16_t month;
    uint16_t day;
    uint16_t hour;
    //uvr
    float m_rUvrVolFlow_avg;
    float m_rUvrVolTotal;
    float d_rUvrObjem;
    //NORD 1
    float d_rNORD1_VolFlow_avg;
    float d_rNORD1_Objem;
    //NORD 2
    float d_rNORD2_VolFlow_avg;
    float d_rNORD2_Objem;


    uint16_t alarmsCode;
    uint16_t alarmsTimeSec;
    uint16_t WhichKoeffSaved;

    uint16_t flagCitect;
    uint16_t flagWeintek;
    uint16_t flagServerPRILUKI;
    uint16_t flagServerGGPZ;

    if (uzels[i].IP_addr.isEmpty())
    {
        //QMessageBox::information(this,"Configuration message","Your configuration is incorrect - no IP address!!!",QMessageBox::Ok);
        emit textchange(i, "ERROR-No IP adress!!!");
        return;
    }

    mb = modbus_new_tcp(uzels[i].IP_addr.toStdString().c_str(), 502);

    if (modbus_connect(mb)!=0 )
    {
        //QMessageBox::information(this,"Test","Зв'язок з об'єктом відсутній!!!",QMessageBox::Ok);
        emit textchange(i, "No connection with source");
    }
    else //connect OK
    {
        modbus_set_slave(mb, 1);
        // Read 25 registers from the address from 40301
        int res=modbus_read_registers(mb, 300, 25, tab_reg);

        if (res!=25)
        {
            emit textchange(i, "ERROR: modbus read error");

        }
        else //read OK
        {

            year=tab_reg[0];
            month=tab_reg[1];
            day=tab_reg[2];
            hour=tab_reg[3];

            //uvr
            m_rUvrVolFlow_avg=modbus_get_float(&tab_reg[4]);
            m_rUvrVolTotal=modbus_get_float(&tab_reg[6]);
            d_rUvrObjem=modbus_get_float(&tab_reg[8]);
            //NORD 1
            d_rNORD1_VolFlow_avg=modbus_get_float(&tab_reg[10]);
            d_rNORD1_Objem=modbus_get_float(&tab_reg[12]);
            //NORD 2
            d_rNORD2_VolFlow_avg=modbus_get_float(&tab_reg[14]);
            d_rNORD2_Objem=modbus_get_float(&tab_reg[16]);




            alarmsCode=tab_reg[18];
            alarmsTimeSec=tab_reg[19];
            WhichKoeffSaved=tab_reg[20];

            flagCitect=tab_reg[21];
            flagWeintek=tab_reg[22];
            flagServerPRILUKI=tab_reg[23];  //Priluki server flag defined 83  (40384)
            flagServerGGPZ=tab_reg[24];     //ggpz defined 84                  (40385)


            if (tab_reg[SERVERFLAG1UVR2NORD]==1)
            {

               //QString tmp;

               //tmp.sprintf("%i %i %i %i\n%.2f %.2f %.2f %.2f %.2f %.2f %.2f\n%i %i %i",year,month,day,hour,
        //GustPlastVod,GustSepNaf,GustGazuSU,Gazovmist,ObjomVilnGazu,DiametrTrubopr,KoeffZ,flagCitect,flagWeintek,flagServer);
                //QMessageBox::information(this,"Test",tmp,QMessageBox::Ok);
        // reset flag flagServer


                    // insert to db
                    QString connectionName=GetNextName();

                        {  // start of the block where the db object lives

                            QSqlDatabase db = QSqlDatabase::addDatabase("QODBC",connectionName);
                                  db.setDatabaseName(uzels[i].OdbcName);
                                  db.setUserName("sysdba");//user);
                                  db.setPassword("784523");//pass);
                            if (db.open())
                            {

                                QSqlQuery sqlQuery(db);
                                QString query;

                                tab_reg[SERVERFLAG1UVR2NORD]=0;
                                if (modbus_write_registers(mb, 300+SERVERFLAG1UVR2NORD, 1, &tab_reg[SERVERFLAG1UVR2NORD])==1)  //flag reset OK
                                {

                                    query.sprintf(QString("INSERT INTO " + uzels[i].TableName + "(" +
                                                  "DT, UVRVOLFLOW, UVRVOLTOTAL, UVROBJEM, " +
                                                  "NORD1_VOLFLOW, NORD1_OBJEM, NORD2_VOLFLOW, NORD2_OBJEM, "+
                                                  "ALARMSCODE , ALARMSTIMESEC, WHICHKOEFFSAVED) "
                                                  "VALUES ("+
                                                  "'%i.%i.%i %i:00:00', %f, %f, %f, "+
                                                  "%f, %f, %f, %f, " +
                                                  "%i, %i, %i)").toStdString().c_str(),
                                                  day,month,year,hour,m_rUvrVolFlow_avg,m_rUvrVolTotal,d_rUvrObjem,
                                                  d_rNORD1_VolFlow_avg,d_rNORD1_Objem,d_rNORD2_VolFlow_avg,d_rNORD2_Objem,
                                                  alarmsCode, alarmsTimeSec, WhichKoeffSaved
                                                  );

                                                  //  emit insert(query);

                                    sqlQuery.exec(query);

                                    if (sqlQuery.lastError().isValid())
                                    {
                                        //QMessageBox::critical(NULL, QObject::tr("Database Error"), sqlQuery.lastError().text());
                                        emit textchange(i,"ERROR: Database error on INSERT");
                                    }
                                    else
                                    {
                                        //QMessageBox::information(NULL, tr("Информация"), "Запись удалена успешно.");
                                        emit textchange(i,"INSERT OK");
                                    }

                                }
                                else
                                {
                                    emit textchange(i, "ERROR: cannot reset flag!!!");
                                }


                            }
                            else
                            {
                                //QMessageBox::critical(NULL, QObject::tr("Database Error"), db.lastError().text());
                                emit insert(db.lastError().databaseText());
                                emit insert(db.lastError().driverText());
                                emit textchange(i,"ERROR: Database not open");
                            }

                            db.close();
                        } // end of the block where the db object lives, it will be destroyed here

                        QSqlDatabase::removeDatabase(connectionName);

                    //insert to DB end




            }
            else
            {
                emit textchange(i, "No new data");
            }
        }
    }
    modbus_close(mb);
    modbus_free(mb);

}
//===============================================================================
void ThreadPollObjects::Poll4Uvr(int i)
{
    modbus_t *mb;
    uint16_t tab_reg[100];


    uint16_t year;
    uint16_t month;
    uint16_t day;
    uint16_t hour;
    //uvr 1
    float m_rUvr1_VolFlow_avg;
    float m_rUvr1_VolTotal;
    float d_rUvr1_Objem;

    //uvr 2
    float m_rUvr2_VolFlow_avg;
    float m_rUvr2_VolTotal;
    float d_rUvr2_Objem;

    //uvr 3
    float m_rUvr3_VolFlow_avg;
    float m_rUvr3_VolTotal;
    float d_rUvr3_Objem;

    //uvr 4
    float m_rUvr4_VolFlow_avg;
    float m_rUvr4_VolTotal;
    float d_rUvr4_Objem;


    uint16_t alarmsCode;
    uint16_t alarmsTimeSec;
    uint16_t WhichKoeffSaved;

    uint16_t flagCitect;
    uint16_t flagWeintek;
    uint16_t flagServerPRILUKI;
    uint16_t flagServerGGPZ;

    if (uzels[i].IP_addr.isEmpty())
    {
        //QMessageBox::information(this,"Configuration message","Your configuration is incorrect - no IP address!!!",QMessageBox::Ok);
        emit textchange(i, "ERROR-No IP adress!!!");
        return;
    }

    mb = modbus_new_tcp(uzels[i].IP_addr.toStdString().c_str(), 502);

    if (modbus_connect(mb)!=0 )
    {
        //QMessageBox::information(this,"Test","Зв'язок з об'єктом відсутній!!!",QMessageBox::Ok);
        emit textchange(i, "No connection with source");
    }
    else //connect OK
    {
        modbus_set_slave(mb, 1);
        // Read 35 registers from the address from 40301
        int res=modbus_read_registers(mb, 300, 35, tab_reg);

        if (res!=35)
        {
            emit textchange(i, "ERROR: modbus read error");

        }
        else //read OK
        {

            year=tab_reg[0];
            month=tab_reg[1];
            day=tab_reg[2];
            hour=tab_reg[3];

            //uvr 1
            m_rUvr1_VolFlow_avg=modbus_get_float(&tab_reg[4]);
            m_rUvr1_VolTotal=modbus_get_float(&tab_reg[6]);
            d_rUvr1_Objem=modbus_get_float(&tab_reg[8]);

            //uvr 2
            m_rUvr2_VolFlow_avg=modbus_get_float(&tab_reg[10]);
            m_rUvr2_VolTotal=modbus_get_float(&tab_reg[12]);
            d_rUvr2_Objem=modbus_get_float(&tab_reg[14]);

            //uvr 3
            m_rUvr3_VolFlow_avg=modbus_get_float(&tab_reg[16]);
            m_rUvr3_VolTotal=modbus_get_float(&tab_reg[18]);
            d_rUvr3_Objem=modbus_get_float(&tab_reg[20]);

            //uvr 4
            m_rUvr4_VolFlow_avg=modbus_get_float(&tab_reg[22]);
            m_rUvr4_VolTotal=modbus_get_float(&tab_reg[24]);
            d_rUvr4_Objem=modbus_get_float(&tab_reg[26]);



            alarmsCode=tab_reg[28];
            alarmsTimeSec=tab_reg[29];
            WhichKoeffSaved=tab_reg[30];

            flagCitect=tab_reg[31];
            flagWeintek=tab_reg[32];
            flagServerPRILUKI=tab_reg[33];  //Priluki server flag defined 83  (40384)
            flagServerGGPZ=tab_reg[34];     //ggpz defined 84                  (40385)


            if (tab_reg[SERVERFLAG4UVR]==1)
            {
               //QString tmp;

               //tmp.sprintf("%i %i %i %i\n%.2f %.2f %.2f %.2f %.2f %.2f %.2f\n%i %i %i",year,month,day,hour,
        //GustPlastVod,GustSepNaf,GustGazuSU,Gazovmist,ObjomVilnGazu,DiametrTrubopr,KoeffZ,flagCitect,flagWeintek,flagServer);
                //QMessageBox::information(this,"Test",tmp,QMessageBox::Ok);
        // reset flag flagServer


                    // insert to db
                    QString connectionName=GetNextName();

                        {  // start of the block where the db object lives

                            QSqlDatabase db = QSqlDatabase::addDatabase("QODBC",connectionName);
                                  db.setDatabaseName(uzels[i].OdbcName);
                                  db.setUserName("sysdba");//user);
                                  db.setPassword("784523");//pass);
                            if (db.open())
                            {

                                QSqlQuery sqlQuery(db);
                                QString query;

                                tab_reg[SERVERFLAG4UVR]=0;
                                if (modbus_write_registers(mb, 300+SERVERFLAG4UVR, 1, &tab_reg[SERVERFLAG4UVR])==1)  //flag reset OK
                                {

                                    query.sprintf(QString("INSERT INTO " + uzels[i].TableName + "(" +
                                                  "DT, UVR1_VOLFLOW, UVR1_VOLTOTAL, UVR1_OBJEM, " +
                                                  "UVR2_VOLFLOW, UVR2_VOLTOTAL, UVR2_OBJEM, " +
                                                  "UVR3_VOLFLOW, UVR3_VOLTOTAL, UVR3_OBJEM, " +
                                                  "UVR4_VOLFLOW, UVR4_VOLTOTAL, UVR4_OBJEM, " +
                                                  "ALARMSCODE , ALARMSTIMESEC, WHICHKOEFFSAVED) "
                                                  "VALUES ("+
                                                  "'%i.%i.%i %i:00:00', %f, %f, %f, "+
                                                  "%f, %f, %f, " +
                                                  "%f, %f, %f, " +
                                                  "%f, %f, %f, " +
                                                  "%i, %i, %i)").toStdString().c_str(),
                                                  day,month,year,hour,m_rUvr1_VolFlow_avg,m_rUvr1_VolTotal,d_rUvr1_Objem,
                                                  m_rUvr2_VolFlow_avg,m_rUvr2_VolTotal,d_rUvr2_Objem,
                                                  m_rUvr3_VolFlow_avg,m_rUvr3_VolTotal,d_rUvr3_Objem,
                                                  m_rUvr4_VolFlow_avg,m_rUvr4_VolTotal,d_rUvr4_Objem,
                                                  alarmsCode, alarmsTimeSec, WhichKoeffSaved
                                                  );

                                                  //  emit insert(query);

                                    sqlQuery.exec(query);

                                    if (sqlQuery.lastError().isValid())
                                    {
                                        //QMessageBox::critical(NULL, QObject::tr("Database Error"), sqlQuery.lastError().text());
                                        emit textchange(i,"ERROR: Database error on INSERT");
                                    }
                                    else
                                    {
                                        //QMessageBox::information(NULL, tr("Информация"), "Запись удалена успешно.");
                                        emit textchange(i,"INSERT OK");
                                    }

                                }
                                else
                                {
                                    emit textchange(i, "ERROR: cannot reset flag!!!");
                                }


                            }
                            else
                            {
                                //QMessageBox::critical(NULL, QObject::tr("Database Error"), db.lastError().text());
                                emit insert(db.lastError().databaseText());
                                emit insert(db.lastError().driverText());
                                emit textchange(i,"ERROR: Database not open");
                            }

                            db.close();
                        } // end of the block where the db object lives, it will be destroyed here

                        QSqlDatabase::removeDatabase(connectionName);

                    //insert to DB end




            }
            else
            {
                emit textchange(i, "No new data");
            }
        }
    }
    modbus_close(mb);
    modbus_free(mb);




}
//===============================================================================
void ThreadPollObjects::PollGaz(int i)
{

    modbus_t *mb;
    uint16_t tab_reg[100];


    uint16_t year;
    uint16_t month;
    uint16_t day;
    uint16_t hour;
    float plotn_nom;
    float N_N2;
    float N_CO2;
    float D_it_20;
    float d_su_20;
    float a0_su;
    float a1_su;
    float a2_su;
    float Ra;
    float Rn;
    float Interval;


    //средних, делить на cycle_count
    float d_rTemp_avg;
    float d_rTisk_avg;
    float d_rPerepad_avg;
    float d_rVolFlowSU_avg;


    //накопленные
    float d_rObjemGazuSU;
    float d_rMassaGazu;  //d_rObjemGazonasZnevodnNafRU_MM_SOU

    float rep_rezerv1;
    float rep_rezerv2;


    uint16_t alarmsCode;
    uint16_t alarmsTimeSec;
    uint16_t WhichKoeffSaved;

    uint16_t flagCitect;
    uint16_t flagWeintek;
    uint16_t flagServerPRILUKI;
    uint16_t flagServerGGPZ;

    if (uzels[i].IP_addr.isEmpty())
    {
        //QMessageBox::information(this,"Configuration message","Your configuration is incorrect - no IP address!!!",QMessageBox::Ok);
        emit textchange(i, "ERROR-No IP adress!!!");
        return;
    }

    mb = modbus_new_tcp(uzels[i].IP_addr.toStdString().c_str(), 502);

    if (modbus_connect(mb)!=0 )
    {
        //QMessageBox::information(this,"Test","Зв'язок з об'єктом відсутній!!!",QMessageBox::Ok);
        emit textchange(i, "No connection with source");
    }
    else //connect OK
    {
        modbus_set_slave(mb, 1);
        // Read 49 registers from the address from 40301
        int res=modbus_read_registers(mb, 300, 49, tab_reg);

        if (res!=49)
        {
            emit textchange(i, "ERROR: modbus read error");

        }
        else //read OK
        {

            year=tab_reg[0];
            month=tab_reg[1];
            day=tab_reg[2];
            hour=tab_reg[3];
            plotn_nom=modbus_get_float(&tab_reg[4]);
            N_N2=modbus_get_float(&tab_reg[6]);
            N_CO2=modbus_get_float(&tab_reg[8]);
            D_it_20=modbus_get_float(&tab_reg[10]);
            d_su_20=modbus_get_float(&tab_reg[12]);
            a0_su=modbus_get_float(&tab_reg[14]);
            a1_su=modbus_get_float(&tab_reg[16]);
            a2_su=modbus_get_float(&tab_reg[18]);


            Ra=modbus_get_float(&tab_reg[20]);
            Rn=modbus_get_float(&tab_reg[22]);
            Interval=modbus_get_float(&tab_reg[24]);
            //средних, делить на cycle_count
            d_rTemp_avg=modbus_get_float(&tab_reg[26]);
            d_rTisk_avg=modbus_get_float(&tab_reg[28]);
            d_rPerepad_avg=modbus_get_float(&tab_reg[30]);
            d_rVolFlowSU_avg=modbus_get_float(&tab_reg[32]);
            //накопленные
            d_rObjemGazuSU=modbus_get_float(&tab_reg[34]);
            d_rMassaGazu=modbus_get_float(&tab_reg[36]);
            //rep_rezerv1=modbus_get_float(&tab_reg[38]);
            //rep_rezerv2=modbus_get_float(&tab_reg[40]);

            alarmsCode=tab_reg[42];
            alarmsTimeSec=tab_reg[43];
            WhichKoeffSaved=tab_reg[44];

            flagCitect=tab_reg[45];
            flagWeintek=tab_reg[46];
            flagServerPRILUKI=tab_reg[47];  //Priluki server flag GAZ defined 47  (40348)
            flagServerGGPZ=tab_reg[48];     //ggpz GAZ defined 48                  (40349)


            if (tab_reg[SERVERFLAGGAZ]==1)
            {

               //QString tmp;

               //tmp.sprintf("%i %i %i %i\n%.2f %.2f %.2f %.2f %.2f %.2f %.2f\n%i %i %i",year,month,day,hour,
        //GustPlastVod,GustSepNaf,GustGazuSU,Gazovmist,ObjomVilnGazu,DiametrTrubopr,KoeffZ,flagCitect,flagWeintek,flagServer);
                //QMessageBox::information(this,"Test",tmp,QMessageBox::Ok);
        // reset flag flagServer


                    // insert to db
                    QString connectionName=GetNextName();

                        {  // start of the block where the db object lives

                            QSqlDatabase db = QSqlDatabase::addDatabase("QODBC",connectionName);
                                  db.setDatabaseName(uzels[i].OdbcName);
                                  db.setUserName("sysdba");//user);
                                  db.setPassword("784523");//pass);
                            if (db.open())
                            {

                                QSqlQuery sqlQuery(db);
                                QString query;

                                tab_reg[SERVERFLAGGAZ]=0;
                                if (modbus_write_registers(mb, 300+SERVERFLAGGAZ, 1, &tab_reg[SERVERFLAGGAZ])==1)  //flag reset OK
                                {

                                    query.sprintf(QString("INSERT INTO " + uzels[i].TableName + "(" +
                                                  "DT, TEMP, TISK, PEREPAD, VOLFLOWSU, " +
                                                  "PLOTN_NOM, N_N2, N_CO2, D_IT_20, D_SU_20, A0_SU, A1_SU, A2_SU, " +
                                                  "RA, RN, INTERVAL, OBJEMGAZUSU, MASSAGAZU, "+
                                                  "ALARMSCODE , ALARMSTIMESEC, WHICHKOEFFSAVED) "
                                                  "VALUES ("+
                                                  "'%i.%i.%i %i:00:00', %f, %f, %f, %f, "+
                                                  "%f, %f, %f, %f, %f, %f, %f, %f, " +
                                                  "%f, %f, %f, %f, %f, " +
                                                  "%i, %i, %i)").toStdString().c_str(),
                                                  day,month,year,hour,d_rTemp_avg,d_rTisk_avg,d_rPerepad_avg,d_rVolFlowSU_avg,
                                                  plotn_nom,N_N2,N_CO2,D_it_20,d_su_20,a0_su,a1_su,a2_su,
                                                  Ra,Rn,Interval,d_rObjemGazuSU,d_rMassaGazu,
                                                  alarmsCode, alarmsTimeSec, WhichKoeffSaved
                                                  );

                                                  //  emit insert(query);

                                    sqlQuery.exec(query);

                                    if (sqlQuery.lastError().isValid())
                                    {
                                        //QMessageBox::critical(NULL, QObject::tr("Database Error"), sqlQuery.lastError().text());
                                        emit textchange(i,"ERROR: Database error on INSERT");
                                    }
                                    else
                                    {
                                        //QMessageBox::information(NULL, tr("Информация"), "Запись удалена успешно.");
                                        emit textchange(i,"INSERT OK");
                                    }

                                }
                                else
                                {
                                    emit textchange(i, "ERROR: cannot reset flag!!!");
                                }


                            }
                            else
                            {
                                //QMessageBox::critical(NULL, QObject::tr("Database Error"), db.lastError().text());
                                emit insert(db.lastError().databaseText());
                                emit insert(db.lastError().driverText());
                                emit textchange(i,"ERROR: Database not open");
                            }

                            db.close();
                        } // end of the block where the db object lives, it will be destroyed here

                        QSqlDatabase::removeDatabase(connectionName);

                    //insert to DB end




            }
            else
            {
                emit textchange(i, "No new data");
            }
        }
    }
    modbus_close(mb);
    modbus_free(mb);



}
//================================================================================
/*
CREATE TABLE LELAKI_SOU
(
ID INTEGER,
DT TIMESTAMP,
TEMP FLOAT,
TISK FLOAT,
OBVODN_OPOS FLOAT,
DENS FLOAT,
OBVODN FLOAT,
GUSTPLASTVOD FLOAT,
GUSTSEPNAF FLOAT,
GUSTGAZUSU FLOAT,
GAZOVMIST FLOAT,
OBJOMVILNGAZU FLOAT,
DIAMETRTRUBOPR FLOAT,
KOEFFZ FLOAT,
FLOWMM FLOAT,
MASSARIDMM FLOAT,
OBJEMRIDRUMM FLOAT,
OBJEMNAFMM FLOAT,
MASSANAFMM FLOAT,
MASSARIDMM_PIP FLOAT,
OBJEMRIDRUMM_PIP FLOAT,
OBJEMNAFMM_PIP FLOAT,
MASSANAFMM_PIP FLOAT,
FLOWNORD1 FLOAT,
OBJEMRIDNORD1 FLOAT,
OBJEMNAFSUNORD1 FLOAT,
MASSANAFNORD1 FLOAT,
OBJEMRIDRUNORD1 FLOAT,
OBJEMRIDNORD1_PIP FLOAT,
OBJEMRIDRUNORD1_PIP FLOAT,
MASSANAFNORD1_PIP FLOAT,
OBJEMNAFSUNORD1_PIP FLOAT,
ALARMSCODE INTEGER,
ALARMSTIMESEC INTEGER,
WHICHKOEFFSAVED INTEGER,
PRIMARY KEY(ID)
);

+autoincrement on id
//======================================================================

///1UVR2NORD
///
CREATE TABLE GGPZ_Z_KSU_SOU
(
ID INTEGER,
DT TIMESTAMP,
UVRVOLFLOW FLOAT,
UVRVOLTOTAL FLOAT,
UVROBJEM FLOAT,
NORD1_VOLFLOW FLOAT,
NORD1_OBJEM FLOAT,
NORD2_VOLFLOW FLOAT,
NORD2_OBJEM FLOAT,
ALARMSCODE INTEGER,
ALARMSTIMESEC INTEGER,
WHICHKOEFFSAVED INTEGER,
PRIMARY KEY(ID)
);

+autoincrement on id


//======================================================================

///4UVR
///
CREATE TABLE GGPZ_4VOV_SOU
(
ID INTEGER,
DT TIMESTAMP,
UVR1_VOLFLOW FLOAT,
UVR1_VOLTOTAL FLOAT,
UVR1_OBJEM FLOAT,
UVR2_VOLFLOW FLOAT,
UVR2_VOLTOTAL FLOAT,
UVR2_OBJEM FLOAT,
UVR3_VOLFLOW FLOAT,
UVR3_VOLTOTAL FLOAT,
UVR3_OBJEM FLOAT,
UVR4_VOLFLOW FLOAT,
UVR4_VOLTOTAL FLOAT,
UVR4_OBJEM FLOAT,
ALARMSCODE INTEGER,
ALARMSTIMESEC INTEGER,
WHICHKOEFFSAVED INTEGER,
PRIMARY KEY(ID)
);

+autoincrement on id

//======================================================================

///GAZ
///
CREATE TABLE GGPZ_GAZ
(
ID INTEGER,
DT TIMESTAMP,
TEMP FLOAT,
TISK FLOAT,
PEREPAD FLOAT,
VOLFLOWSU FLOAT,
PLOTN_NOM FLOAT,
N_N2 FLOAT,
N_CO2 FLOAT,
D_IT_20 FLOAT,
D_SU_20 FLOAT,
A0_SU FLOAT,
A1_SU FLOAT,
A2_SU FLOAT,
RA FLOAT,
RN FLOAT,
INTERVAL FLOAT,
OBJEMGAZUSU FLOAT,
MASSAGAZU FLOAT,
ALARMSCODE INTEGER,
ALARMSTIMESEC INTEGER,
WHICHKOEFFSAVED INTEGER,
PRIMARY KEY(ID)
);

+autoincrement on id


*/

