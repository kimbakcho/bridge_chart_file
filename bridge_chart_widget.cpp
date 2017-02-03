#include "bridge_chart_widget.h"
#include "ui_bridge_chart_widget.h"
send_email_data::send_email_data()
{
    for(int i=0;i<sizeof(match_case);i++){
        match_case[i] = false;
    }
}

bool send_email_data::check_send_email()
{
    for(int i=0;i<sizeof(match_case);i++){
        if(match_case[i]){
            return true;
        }
    }
    return false;

}
bridge_chart_widget::bridge_chart_widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::bridge_chart_widget)
{
    ui->setupUi(this);
    main_chart = new bridge_chart();
    main_chartview = new bridge_chartview(main_chart);
    ui->chart_layout->addWidget(main_chartview);
    KSQ_chart = new bridge_chart();
    KSQ_chartview = new bridge_chartview(KSQ_chart);
    ui->KSQ_layout->addWidget(KSQ_chartview);
    charttimer.setInterval(10);
    dbcurrentdatetime.setDate(QDate(2017,02,02));
    dbcurrentdatetime.setTime(QTime(0,0,0));
    ms_mes_db = QSqlDatabase::addDatabase("QODBC3","bridge_chart_ms_mes_db");
    ms_mes_db.setDatabaseName("DRIVER={FreeTDS};Server=10.20.10.221;Database=MESDB;Uid=fabview;Port=1433;Pwd=fabview");
    if(!ms_mes_db.open()){
        qDebug()<<ms_mes_db.lastError().text();
    }else {
    }
    connect(&charttimer,SIGNAL(timeout()),this,SLOT(chart_timer_timeout()));
    run_count = 0;



}

QString bridge_chart_widget::make_image_file(QDateTime currentdatetime, QWidget *pictuerwidget,QString pickture_name)
{
    QTimer::singleShot(100,this,SLOT(nonblock_timer_timeout()));
    nonblock.exec();
    QDir savedir = QDir::home();
    savedir.cd("bridge_chart_file");
    if(!savedir.exists(currentdatetime.toString("yyyy_MM_dd_hh_mm_ss"))){
          savedir.mkdir(currentdatetime.toString("yyyy_MM_dd_hh_mm_ss"));
    }
    savedir.cd(currentdatetime.toString("yyyy_MM_dd_hh_mm_ss"));
    QPixmap picture(pictuerwidget->size());
    pictuerwidget->render(&picture);
    picture.save(savedir.absolutePath()+QString("/%1.png").arg(pickture_name),"PNG");
    QString path =QString("http://fabsv.wisol.co.kr/bridge_chart_file/%1/%2.png").arg(currentdatetime.toString("yyyy_MM_dd_hh_mm_ss")).arg(pickture_name);
    QString tag = QString("<img src=\"%1\" alt=\"%2\" height=\"%3\" width=\"%4\">").arg(path).arg(pickture_name).arg(picture.height()).arg(picture.width());


    return tag;
}



bridge_chart_widget::~bridge_chart_widget()
{
    delete ui;
}

void bridge_chart_widget::chart_timer_timeout()
{
    QSqlQuery main_query(ms_mes_db);

    QDateTime search_start_time;
    QDateTime search_end_time;
    search_start_time.setDate(QDate::currentDate().addDays(-7));
    search_start_time.setTime(QTime(0,0,0));
    search_end_time.setDate(QDate::currentDate().addDays(1));
    search_end_time.setTime(QTime(0,0,0));
    run_count++;
    ui->LA_statue->setText(QString("run count = %1").arg(run_count));
    QString query_txt1 = QString("SELECT TOP 1 * from  [MESDB].[dbo].[NM_EDC_LOTS] where COLLECTION_ID = 'Bridge_Thickness'  AND CHARACTER_ID = 'Bridge_Thickness' order by TX_DTTM desc");

    main_query.exec(query_txt1);
    if(main_query.next()){
        QDateTime temp_db_datetime;
        temp_db_datetime = QDateTime::fromString(main_query.value("TX_DTTM").toString(),"yyyyMMddhhmmss");
        int diff = temp_db_datetime.secsTo(dbcurrentdatetime);
        if(diff == 0){
            return;
        }else {
            dbcurrentdatetime = temp_db_datetime;
        }
    }

    QString query_txt2 = QString("SELECT a.TX_USER_NAME,a.TX_DTTM,a.MATERIAL_ID,a.LOT_ID,a.ROUTE_ID,"
                                "a.OPERATION_ID,A.EQUIPMENT_ID,c.EQUIPMENT_NAME,a.VALUE1,a.COLLECTION_ID,"
                                "b.TARGET_VALUE,b.UPPER_SPEC_LIMIT,b.LOWER_SPEC_LIMIT,b.UPPER_WARN_LIMIT,b.LOWER_WARN_LIMIT  "
                                "FROM [MESDB].[dbo].[NM_EDC_LOTS] a,[MESDB].[dbo].[NM_COLLECTION_CHARACTERS] b,[MESDB].[dbo].[NM_EQUIPMENT] c "
                                "where a.COLLECTION_ID = 'Bridge_Thickness'  AND a.CHARACTER_ID = 'Bridge_Thickness' AND  a.CHARACTER_ID = b.CHARACTER_ID AND a.COLLECTION_ID = b.COLLECTION_ID AND a.COLLECTION_VERSION = b.COLLECTION_VERSION AND a.EQUIPMENT_ID = c.EQUIPMENT_ID AND "
                                "(TX_DTTM between '%1' AND '%2')  order by TX_DTTM asc").arg(search_start_time.toString("yyyyMMddhhmmss")).arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));

    main_query.exec(query_txt2);
    main_chart->removeAllSeries();
    if(main_chart->axisX() != NULL){
        main_chart->removeAxis(main_chart->axisX());
    }
    if(main_chart->axisY() != NULL){
        main_chart->removeAxis(main_chart->axisY());
    }
    QPen temp_pen;
    value_series = new QLineSeries;
    value_series->setName("value");
    value_series->setColor(QColor("#141414"));
    temp_pen = value_series->pen();
    temp_pen.setWidthF(2);
    value_series->setPen(temp_pen);

    CL_series = new QLineSeries;
    CL_series->setColor(QColor("#47ff53"));
    CL_series->setName("CL");

    USL_series[0] = new QLineSeries;
    USL_series[0]->setName("A");
    USL_series[0]->setColor(QColor("#ff0004"));
    USL_series[1] = new QLineSeries;
    USL_series[1]->setName("B");
    USL_series[1]->setColor(QColor("#ff636d"));
    USL_series[2] = new QLineSeries;
    USL_series[2]->setName("C");
    USL_series[2]->setColor(QColor("#ffc2cb"));
    LSL_series[0] = new QLineSeries;
    LSL_series[0]->setName("A");
    LSL_series[0]->setColor(QColor("#ff0004"));
    LSL_series[1] = new QLineSeries;
    LSL_series[1]->setName("B");
    LSL_series[1]->setColor(QColor("#ff636d"));
    LSL_series[2] = new QLineSeries;
    LSL_series[2]->setName("C");
    LSL_series[2]->setColor(QColor("#ffc2cb"));

    qreal setp = 0 ;
    value_series->setPointsVisible(true);

    while(main_query.next()){
        QDateTime search_time = QDateTime::fromString(main_query.value("TX_DTTM").toString(),QString("yyyyMMddHHmmss"));

        value_series->append(search_time.toMSecsSinceEpoch(),main_query.value("VALUE1").toInt());
        int diff = main_query.value("UPPER_SPEC_LIMIT").toInt() - main_query.value("LOWER_SPEC_LIMIT").toInt();
        setp =  (qreal)diff/6.0;
        USL_series[0]->append(search_time.toMSecsSinceEpoch(),main_query.value("UPPER_SPEC_LIMIT").toInt());
        USL_series[1]->append(search_time.toMSecsSinceEpoch(),main_query.value("UPPER_SPEC_LIMIT").toDouble()-setp*1);
        USL_series[2]->append(search_time.toMSecsSinceEpoch(),main_query.value("UPPER_SPEC_LIMIT").toDouble()-setp*2);
        CL_series->append(search_time.toMSecsSinceEpoch(),main_query.value("TARGET_VALUE").toInt());
        LSL_series[2]->append(search_time.toMSecsSinceEpoch(),main_query.value("LOWER_SPEC_LIMIT").toInt()+setp*2);
        LSL_series[1]->append(search_time.toMSecsSinceEpoch(),main_query.value("LOWER_SPEC_LIMIT").toInt()+setp*1);
        LSL_series[0]->append(search_time.toMSecsSinceEpoch(),main_query.value("LOWER_SPEC_LIMIT").toInt());
    }
    int item_count = CL_series->points().size();

    //case 1
//     value_series->append(CL_series->points().at(item_count-3).x(),13240);
//     value_series->append(CL_series->points().at(item_count-2).x(),13260);
//     value_series->append(CL_series->points().at(item_count-1).x(),15530);

     //case 2

//        value_series->append(CL_series->points().at(item_count-12).x(),13210);
//        value_series->append(CL_series->points().at(item_count-11).x(),13210);
//        value_series->append(CL_series->points().at(item_count-10).x(),13210);
//        value_series->append(CL_series->points().at(item_count-9).x(),13210);
//        value_series->append(CL_series->points().at(item_count-8).x(),13210);
//        value_series->append(CL_series->points().at(item_count-7).x(),13210);
//        value_series->append(CL_series->points().at(item_count-6).x(),13210);
//        value_series->append(CL_series->points().at(item_count-5).x(),13210);
//        value_series->append(CL_series->points().at(item_count-4).x(),13210);
//        value_series->append(CL_series->points().at(item_count-3).x(),13210);
//        value_series->append(CL_series->points().at(item_count-2).x(),13210);
//        value_series->append(CL_series->points().at(item_count-1).x(),13210);

//    case 3
//        value_series->append(CL_series->points().at(item_count-10).x(),13210);
//        value_series->append(CL_series->points().at(item_count-9).x(),13210);
//        value_series->append(CL_series->points().at(item_count-8).x(),13210);
//        value_series->append(CL_series->points().at(item_count-7).x(),13210);
//        value_series->append(CL_series->points().at(item_count-6).x(),13210);
//        value_series->append(CL_series->points().at(item_count-5).x(),13220);
//        value_series->append(CL_series->points().at(item_count-4).x(),13230);
//        value_series->append(CL_series->points().at(item_count-3).x(),13240);
//        value_series->append(CL_series->points().at(item_count-2).x(),13250);
//        value_series->append(CL_series->points().at(item_count-1).x(),13260);
//       case 4
//        value_series->append(CL_series->points().at(item_count-15).x(),14000);
//        value_series->append(CL_series->points().at(item_count-14).x(),13000);
//        value_series->append(CL_series->points().at(item_count-13).x(),14000);
//        value_series->append(CL_series->points().at(item_count-12).x(),13000);
//        value_series->append(CL_series->points().at(item_count-11).x(),14000);
//        value_series->append(CL_series->points().at(item_count-10).x(),13000);
//        value_series->append(CL_series->points().at(item_count-9).x(),14000);
//        value_series->append(CL_series->points().at(item_count-8).x(),13000);
//        value_series->append(CL_series->points().at(item_count-7).x(),14000);
//        value_series->append(CL_series->points().at(item_count-6).x(),13000);
//        value_series->append(CL_series->points().at(item_count-5).x(),14000);
//        value_series->append(CL_series->points().at(item_count-4).x(),13000);
//        value_series->append(CL_series->points().at(item_count-3).x(),14000);
//        value_series->append(CL_series->points().at(item_count-2).x(),13000);
//        value_series->append(CL_series->points().at(item_count-1).x(),14000);
//           case 5
//            value_series->append(CL_series->points().at(item_count-5).x(),14000);
//            value_series->append(CL_series->points().at(item_count-4).x(),13000);
//            value_series->append(CL_series->points().at(item_count-3).x(),14500);
//            value_series->append(CL_series->points().at(item_count-2).x(),14500);
//            value_series->append(CL_series->points().at(item_count-1).x(),13600);
//               case 6
//                value_series->append(CL_series->points().at(item_count-5).x(),14500);
//                value_series->append(CL_series->points().at(item_count-4).x(),13000);
//                value_series->append(CL_series->points().at(item_count-3).x(),14500);
//                value_series->append(CL_series->points().at(item_count-2).x(),14500);
//                value_series->append(CL_series->points().at(item_count-1).x(),14500);

//   case 7
//    value_series->append(CL_series->points().at(item_count-15).x(),13250);
//    value_series->append(CL_series->points().at(item_count-14).x(),13240);
//    value_series->append(CL_series->points().at(item_count-13).x(),13230);
//    value_series->append(CL_series->points().at(item_count-12).x(),13210);
//    value_series->append(CL_series->points().at(item_count-11).x(),13260);
//    value_series->append(CL_series->points().at(item_count-10).x(),13270);
//    value_series->append(CL_series->points().at(item_count-9).x(),13250);
//    value_series->append(CL_series->points().at(item_count-8).x(),13250);
//    value_series->append(CL_series->points().at(item_count-7).x(),13250);
//    value_series->append(CL_series->points().at(item_count-6).x(),13240);
//    value_series->append(CL_series->points().at(item_count-5).x(),13250);
//    value_series->append(CL_series->points().at(item_count-4).x(),13250);
//    value_series->append(CL_series->points().at(item_count-3).x(),13240);
//    value_series->append(CL_series->points().at(item_count-2).x(),13260);
//    value_series->append(CL_series->points().at(item_count-1).x(),13220);
    //   case 8

//        value_series->append(CL_series->points().at(item_count-6).x(),12000);
//        value_series->append(CL_series->points().at(item_count-5).x(),14500);
//        value_series->append(CL_series->points().at(item_count-4).x(),14500);
//        value_series->append(CL_series->points().at(item_count-3).x(),14500);
//        value_series->append(CL_series->points().at(item_count-2).x(),14500);
//        value_series->append(CL_series->points().at(item_count-1).x(),12000);

    main_chart->addSeries(value_series);
    main_chart->addSeries(USL_series[0]);
    main_chart->addSeries(USL_series[1]);
    main_chart->addSeries(USL_series[2]);
    main_chart->addSeries(LSL_series[0]);
    main_chart->addSeries(LSL_series[1]);
    main_chart->addSeries(LSL_series[2]);
    main_chart->addSeries(CL_series);
    axisX = new QDateTimeAxis;
    axisX->setTickCount(9);
    axisX->setFormat("MM-dd HH:mm:ss");
    axisX->setTitleText("Date");
    axisX->setVisible(true);
    axisX->setTitleVisible(true);
    axisX->setRange(search_start_time,search_end_time);
    main_chart->addAxis(axisX, Qt::AlignBottom);
    axisY = new QValueAxis;
    axisY->setRange(LSL_series[0]->at(0).y()-(setp),USL_series[0]->at(0).y()+(setp));
//    axisY->setLabelFormat("%i");
    main_chart->addAxis(axisY,Qt::AlignLeft);
    value_series->attachAxis(axisX);
    value_series->attachAxis(axisY);
    USL_series[0]->attachAxis(axisX);
    USL_series[0]->attachAxis(axisY);
    USL_series[1]->attachAxis(axisX);
    USL_series[1]->attachAxis(axisY);
    USL_series[2]->attachAxis(axisX);
    USL_series[2]->attachAxis(axisY);
    LSL_series[0]->attachAxis(axisX);
    LSL_series[0]->attachAxis(axisY);
    LSL_series[1]->attachAxis(axisX);
    LSL_series[1]->attachAxis(axisY);
    LSL_series[2]->attachAxis(axisX);
    LSL_series[2]->attachAxis(axisY);
    CL_series->attachAxis(axisX);
    CL_series->attachAxis(axisY);

    QDateTime current_datetime = QDateTime::currentDateTime();

    send_email_data *email_data = new send_email_data;
    //KSQCase 1
    if((USL_series[0]->points().last().y() < value_series->points().last().y()) ||
       (LSL_series[0]->points().last().y() > value_series->points().last().y()) ){
        KSQ_chart_draw(4);
        email_data->match_case[0] = true;

        email_data->filenames<<make_image_file(current_datetime,ui->KSQ_widget,"KSQ1");
        main_query.last();
        QDateTime point_time = QDateTime::fromString(main_query.value("TX_DTTM").toString(),"yyyyMMddhhmmss");
        email_data->filenames<<" <p> case KSQ 1 mactch</p><br> \n <p>1점이 영역 A를 넘고 있다.</p> <br> \n";
        QString content = QString(" <p> [%1] %2 / %3 / %4 / <span style=color:blue>측정값 = %5</span>  <span style=color:red>(A = %6)</span>   <span style=color:blue>(A = %7)</span></p>  <br> \n")
                .arg(point_time.toString("MM-dd hh:mm:ss")).arg(main_query.value("EQUIPMENT_NAME").toString()).arg(main_query.value("MATERIAL_ID").toString()).arg(main_query.value("LOT_ID").toString())
                .arg(value_series->points().last().y()).arg(USL_series[0]->points().last().y()).arg(LSL_series[0]->points().last().y());
        email_data->filenames<<content;
        qDebug()<<"case KSQ 1 mactch";
    }

    //KSQCase 2
    if(value_series->points().size()>=9){
        bool KSQ2_flag1 = true;
        for(int i=1;i<=9;i++){
            int item_size = value_series->points().size();
            if(value_series->points().at(item_size-i).y()>CL_series->points().at(item_size-i).y()){
                KSQ2_flag1 = false;
                break;
            }
        }
        bool KSQ2_flag2 = true;
        for(int i=1;i<=9;i++){
            int item_size = value_series->points().size();
            if(value_series->points().at(item_size-i).y()<CL_series->points().at(item_size-i).y()){
                KSQ2_flag2 = false;
                break;
            }
        }
        if(KSQ2_flag1||KSQ2_flag2){
            KSQ_chart_draw(12);
            email_data->match_case[1] = true;

            email_data->filenames<<make_image_file(current_datetime,ui->KSQ_widget,"KSQ2");
            email_data->filenames<<" <p> case KSQ 2 mactch </p> <br> \n <p> 9점이 중심선에 대하여 같은 쪽에 있다. </p> <br> \n";
            QString content;
            for(int i=1;i<=9;i++){
                int item_size = value_series->points().size();
                if(i == 1){
                    main_query.last();
                }else {
                    main_query.previous();
                }
                QDateTime point_time = QDateTime::fromString(main_query.value("TX_DTTM").toString(),"yyyyMMddhhmmss");
                content.append(QString(" <p> [%1] %2 / %3 / %4 / <span style=color:blue>측정값 = %5</span> (CL = %6) </p> <br> \n")
                               .arg(point_time.toString("MM-dd hh:mm:ss")).arg(main_query.value("EQUIPMENT_NAME").toString()).arg(main_query.value("MATERIAL_ID").toString()).arg(main_query.value("LOT_ID").toString())
                               .arg(value_series->points().at(item_size-i).y()).arg(CL_series->points().at(item_size-i).y()));
            }
            email_data->filenames<<content;
            qDebug()<<"case KSQ 2 mactch";

        }
    }

    //KSQCase 3
    if(value_series->points().size()>=6){
        bool KSQ3_flag1 = true;
        for(int i=1;i<=5;i++){
            int item_size = value_series->points().size();
            if(value_series->points().at(item_size-i-1).y()>value_series->points().at(item_size-i).y()){

            }else {
                KSQ3_flag1 = false;
                break;
            }
        }
        bool KSQ3_flag2 = true;
        for(int i=1;i<=5;i++){
            int item_size = value_series->points().size();
            if(value_series->points().at(item_size-i-1).y()<value_series->points().at(item_size-i).y()){

            }else {
                KSQ3_flag2 = false;
                break;
            }
        }
        if(KSQ3_flag1 || KSQ3_flag2){
            KSQ_chart_draw(7);
            email_data->match_case[2] = true;

            email_data->filenames<<make_image_file(current_datetime,ui->KSQ_widget,"KSQ3");
            email_data->filenames<<" <p> case KSQ 3 mactch </p> <br> \n<p> 6점이 연속적으로 증가 또는 감소하고 있다. </p> <br> \n";
            QString content;
            for(int i=1;i<=6;i++){
                int item_size = value_series->points().size();
                if(i == 1){
                    main_query.last();
                }else {
                    main_query.previous();
                }
                QDateTime point_time = QDateTime::fromString(main_query.value("TX_DTTM").toString(),"yyyyMMddhhmmss");
                content.append(QString(" <p> [%1] %2 / %3 / %4 / <span style=color:blue>측정값 = %5</span> </p> <br> \n")
                               .arg(point_time.toString("MM-dd hh:mm:ss")).arg(main_query.value("EQUIPMENT_NAME").toString()).arg(main_query.value("MATERIAL_ID").toString()).arg(main_query.value("LOT_ID").toString())
                               .arg(value_series->points().at(item_size-i).y()));
            }
            email_data->filenames<<content;
            qDebug()<<"case KSQ 3 mactch";
        }
    }

    //KSQCase 4
    if(value_series->points().size()>=14){
        int up_flag = 0;
        int down_flag = 0;
        int KSQ4_flag = true;
        for(int i=1;i<=13;i++){
            int item_size = value_series->points().size();
            if(value_series->points().at(item_size-i-1).y()>value_series->points().at(item_size-i).y()){
                down_flag=0;
                up_flag++;
            }else if(value_series->points().at(item_size-i-1).y()<value_series->points().at(item_size-i).y()){
                up_flag = 0;
                down_flag++;
            }
            if((up_flag>=2) || (down_flag>=2)){
                KSQ4_flag = false;
                break;
            }
        }
        if(KSQ4_flag){
            KSQ_chart_draw(14);
            email_data->match_case[3] = true;

            email_data->filenames<<make_image_file(current_datetime,ui->KSQ_widget,"KSQ4");
            email_data->filenames<<" <p> case KSQ 4 mactch </p> <br> \n <p> 14점이 교대로 증감하고 있다. </p> <br> \n";
            QString content;
            for(int i=1;i<=14;i++){
                int item_size = value_series->points().size();
                if(i == 1){
                    main_query.last();
                }else {
                    main_query.previous();
                }
                QDateTime point_time = QDateTime::fromString(main_query.value("TX_DTTM").toString(),"yyyyMMddhhmmss");
                content.append(QString(" <p>[%1] %2 / %3 / %4 / <span style=color:blue>측정값 = %5</span> </p> <br> \n")
                               .arg(point_time.toString("MM-dd hh:mm:ss")).arg(main_query.value("EQUIPMENT_NAME").toString()).arg(main_query.value("MATERIAL_ID").toString()).arg(main_query.value("LOT_ID").toString())
                               .arg(value_series->points().at(item_size-i).y()));
            }
            email_data->filenames<<content;

            qDebug()<<"case KSQ 4 mactch";

        }

    }
    //KSQCase 5
    if(value_series->points().size()>=3){
        int match_A_count = 0;
        for(int i=1;i<=3;i++){
            int item_size = value_series->points().size();
            if( (value_series->points().at(item_size-i).y() > USL_series[1]->points().at(item_size-i).y()) &&
                    (value_series->points().at(item_size-i).y() < USL_series[0]->points().at(item_size-i).y()) ){
                match_A_count++;
            }else if ((value_series->points().at(item_size-i).y() < LSL_series[1]->points().at(item_size-i).y()) &&
                      (value_series->points().at(item_size-i).y() > LSL_series[0]->points().at(item_size-i).y()) ){

                match_A_count++;
            }
        }
        if(match_A_count >= 2){
            KSQ_chart_draw(5);
            email_data->match_case[4] = true;

            email_data->filenames<<make_image_file(current_datetime,ui->KSQ_widget,"KSQ5");
            email_data->filenames<<" <p> case KSQ 5 mactch </p> <br> \n <p> 연속하는 3점 중 2점이 A영역에 있다. </p> <br> \n";
            QString content;
            for(int i=1;i<=3;i++){
                int item_size = value_series->points().size();
                if(i == 1){
                    main_query.last();
                }else {
                    main_query.previous();
                }
                QDateTime point_time = QDateTime::fromString(main_query.value("TX_DTTM").toString(),"yyyyMMddhhmmss");
                content.append(QString(" <p> [%1] %2 / %3 / %4 / <span style=color:blue>측정값 = %5</span> <span style=color:red> (A = %6 ~ %7) </span> <span style=color:blue> (A = %8 ~ %9) </span> </p> <br> \n")
                               .arg(point_time.toString("MM-dd hh:mm:ss")).arg(main_query.value("EQUIPMENT_NAME").toString()).arg(main_query.value("MATERIAL_ID").toString()).arg(main_query.value("LOT_ID").toString())
                               .arg(value_series->points().at(item_size-i).y()).arg(USL_series[1]->points().at(item_size-i).y()).arg(USL_series[0]->points().at(item_size-i).y()).arg(LSL_series[1]->points().at(item_size-i).y()).arg(LSL_series[0]->points().at(item_size-i).y()));
            }
            email_data->filenames<<content;

            qDebug()<<"case KSQ 5 mactch";
        }
    }
    //KSQCase 6
    if(value_series->points().size()>=5){
        int match_B_up_count = 0;
        for(int i=1;i<=5;i++){
            int item_size = value_series->points().size();
            if( value_series->points().at(item_size-i).y() > USL_series[2]->points().at(item_size-i).y()){
                match_B_up_count++;
            }else if (value_series->points().at(item_size-i).y() < LSL_series[2]->points().at(item_size-i).y() ){

                match_B_up_count++;
            }
        }
        if(match_B_up_count >= 4){
            KSQ_chart_draw(5);
            email_data->match_case[5] = true;

            email_data->filenames<<make_image_file(current_datetime,ui->KSQ_widget,"KSQ6");
            email_data->filenames<<" <p> case KSQ 6 mactch </p> <br> \n <p> 연속하는 5점중 4점이 B 영역 또는 그것을 넘는 영역에 있다. </p> <br> \n";
            QString content;
            for(int i=1;i<=5;i++){
                int item_size = value_series->points().size();
                if(i == 1){
                    main_query.last();
                }else {
                    main_query.previous();
                }
                QDateTime point_time = QDateTime::fromString(main_query.value("TX_DTTM").toString(),"yyyyMMddhhmmss");
                content.append(QString(" <p>  [%1] %2 / %3 / %4 / <span style=color:blue>측정값 = %5</span> <span style=color:red> (B = %6) </span> <span style=color:blue>(B = %7) </span> </p> <br> \n")
                               .arg(point_time.toString("MM-dd hh:mm:ss")).arg(main_query.value("EQUIPMENT_NAME").toString()).arg(main_query.value("MATERIAL_ID").toString()).arg(main_query.value("LOT_ID").toString())
                               .arg(value_series->points().at(item_size-i).y()).arg(USL_series[2]->points().at(item_size-i).y()).arg(LSL_series[2]->points().at(item_size-i).y()));
            }
            email_data->filenames<<content;
            qDebug()<<"case KSQ 6 mactch";
        }
    }

    //KSQCase 7
    if(value_series->points().size()>=15){
        int match_C_count = 0;
        for(int i=1;i<=15;i++){
            int item_size = value_series->points().size();
            if( (value_series->points().at(item_size-i).y() < USL_series[2]->points().at(item_size-i).y()) &&
                    (value_series->points().at(item_size-i).y() > LSL_series[2]->points().at(item_size-i).y()) ){
                match_C_count++;
            }
        }
        if(match_C_count >= 15){
            KSQ_chart_draw(20);
            email_data->match_case[6] = true;

            email_data->filenames<<make_image_file(current_datetime,ui->KSQ_widget,"KSQ7");
            email_data->filenames<<" <p> case KSQ 7 mactch </p> <br> \n  <p> 연속하는 15점이 C영역에 존재한다. </p> <br> \n";
            QString content;
            for(int i=1;i<=15;i++){
                int item_size = value_series->points().size();
                if(i == 1){
                    main_query.last();
                }else {
                    main_query.previous();
                }
                QDateTime point_time = QDateTime::fromString(main_query.value("TX_DTTM").toString(),"yyyyMMddhhmmss");
                content.append(QString(" <p> [%1] %2 / %3 / %4 / <span style=color:blue>측정값 = %5</span> (-C~+C = %6 ~ %7) </p> <br> \n")
                               .arg(point_time.toString("MM-dd hh:mm:ss")).arg(main_query.value("EQUIPMENT_NAME").toString()).arg(main_query.value("MATERIAL_ID").toString()).arg(main_query.value("LOT_ID").toString())
                               .arg(value_series->points().at(item_size-i).y()).arg(LSL_series[2]->points().at(item_size-i).y()).arg(USL_series[2]->points().at(item_size-i).y()));
            }
            email_data->filenames<<content;
            qDebug()<<"case KSQ 7 mactch";

        }
    }

    //KSQCase 8
    if(value_series->points().size()>=5){
        int match_miss_C_count = 0;
        for(int i=1;i<=5;i++){
            int item_size = value_series->points().size();
            if( (value_series->points().at(item_size-i).y() > USL_series[2]->points().at(item_size-i).y()) ||
                    (value_series->points().at(item_size-i).y() < LSL_series[2]->points().at(item_size-i).y()) ){
                match_miss_C_count++;
            }
        }
        if(match_miss_C_count >= 5){
            KSQ_chart_draw(8);
            email_data->match_case[7] = true;

            email_data->filenames<<make_image_file(current_datetime,ui->KSQ_widget,"KSQ8");
            email_data->filenames<<" <p> case KSQ 8 mactch </p ><br> \n <p>연속하는 5점이 상하 관계 없이 C 영역을 넘는 영역에 있다.</p> <br> \n";
            QString content;
            for(int i=1;i<=5;i++){
                int item_size = value_series->points().size();
                if(i == 1){
                    main_query.last();
                }else {
                    main_query.previous();
                }
                QDateTime point_time = QDateTime::fromString(main_query.value("TX_DTTM").toString(),"yyyyMMddhhmmss");
                content.append(QString(" <p> [%1] %2 / %3 / %4 / <span style=color:blue>측정값 = %5</span> 측정값 %5 <span style=color:red>(+C = %6) </span> <span style=color:blue>(-C = %7) </span> </p> <br> \n")
                               .arg(point_time.toString("MM-dd hh:mm:ss")).arg(main_query.value("EQUIPMENT_NAME").toString()).arg(main_query.value("MATERIAL_ID").toString()).arg(main_query.value("LOT_ID").toString())
                               .arg(value_series->points().at(item_size-i).y()).arg(USL_series[2]->points().at(item_size-i).y()).arg(LSL_series[2]->points().at(item_size-i).y()));
            }
            email_data->filenames<<content;
            qDebug()<<"case KSQ 8 mactch";
        }
    }
    if(email_data->check_send_email()){
        email_data->filenames<<make_image_file(current_datetime,main_chartview,"total");
        email_data->filenames<<"<p>total image </p><br>";
        email_send(email_data);
    }

    ui->LA_current_search_time->setText(dbcurrentdatetime.toString("yyyy-MM-dd hh:mm:ss"));

    charttimer.setInterval(5000);
}

void bridge_chart_widget::nonblock_timer_timeout()
{
    nonblock.exit();
}
void bridge_chart_widget::email_send(send_email_data *email_data)
{
    QStringList email_list;
    QStringList email_name;
    QString email_content;
    my_db = QSqlDatabase::addDatabase("QMYSQL",QString("%1_bridge_chart_my_db").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh_mm_ss")));
    my_db.setHostName("fabsv.wisol.co.kr");
    my_db.setUserName("EIS");
    my_db.setPassword("wisolfab!");
    my_db.setDatabaseName("FAB");
    if(!my_db.open()){
        qDebug()<<"open false";
    }
    SmtpClient smtp("mx.info.wisol.co.kr", 25, SmtpClient::TcpConnection);

    MimeMessage message;

    message.setHeaderEncoding(MimePart::QuotedPrintable);

    QSqlQuery query(my_db);
    query.exec("select * from bridge_tickness_email_list");
    while(query.next()){
        EmailAddress *to = new EmailAddress(query.value("email").toString(), query.value("user_name").toString());
        message.addRecipient(to);
    }

    query.exec("select * from bridge_tickness_email_content");
    if(query.next()){
        email_content = query.value("html_content").toString();
    }
    QString sorce_content;
    for(int i=0;i<sizeof(email_data->match_case);i++){
        if(email_data->match_case[i]){
            sorce_content.append(email_data->filenames.takeFirst());
            sorce_content.append(email_data->filenames.takeFirst());
            sorce_content.append(email_data->filenames.takeFirst());
        }
    }
    //total imgae
    sorce_content.append(email_data->filenames.takeFirst());
    sorce_content.append(email_data->filenames.takeFirst());

    email_content = email_content.replace("bridge_conent",sorce_content);

    EmailAddress sender("automail@wisol.co.kr", "automail");
    message.setSender(&sender);

    message.setSubject("FAB bridge 두께 KS Q 판정 mail 입니다.");

    MimeHtml html;

    html.setHtml(email_content);

    message.addPart(&html);

//    qDebug()<<email_content;

    if (!smtp.connectToHost()) {
        qDebug() << "Failed to connect to host!" << endl;
        return ;
    }else {
        if (!smtp.sendMail(message)) {
            qDebug() << "Failed to send mail!" << endl;
        }
    }
    smtp.quit();
    delete email_data;

}

void bridge_chart_widget::KSQ_chart_draw(int count)
{

    QList <QPointF> values = value_series->points();
    QList <QPointF>  CL_series_values;
    QList <QPointF>  USL_series_values[3];
    QList <QPointF>  LSL_series_values[3];
    CL_series_values =CL_series->points();
    USL_series_values[0] = USL_series[0]->points();
    USL_series_values[1] = USL_series[1]->points();
    USL_series_values[2] = USL_series[2]->points();
    LSL_series_values[0] = LSL_series[0]->points();
    LSL_series_values[1] = LSL_series[1]->points();
    LSL_series_values[2] = LSL_series[2]->points();
    KSQ_chart->removeAllSeries();
    if(KSQ_chart->axisX() != NULL){
        KSQ_chart->removeAxis(KSQ_chart->axisX());
    }
    if(KSQ_chart->axisY() != NULL){
        KSQ_chart->removeAxis(KSQ_chart->axisY());
    }
    QPen temp_pen;
    KSQ_value_series = new QLineSeries;
    KSQ_value_series->setName("value");
    KSQ_value_series->setColor(QColor("#141414"));
    temp_pen = KSQ_value_series->pen();
    temp_pen.setWidthF(2);
    KSQ_value_series->setPen(temp_pen);

    KSQ_CL_series = new QLineSeries;
    KSQ_CL_series->setColor(QColor("#47ff53"));
    KSQ_CL_series->setName("CL");

    KSQ_USL_series[0] = new QLineSeries;
    KSQ_USL_series[0]->setName("A");
    KSQ_USL_series[0]->setColor(QColor("#ff0004"));
    KSQ_USL_series[1] = new QLineSeries;
    KSQ_USL_series[1]->setName("B");
    KSQ_USL_series[1]->setColor(QColor("#ff636d"));
    KSQ_USL_series[2] = new QLineSeries;
    KSQ_USL_series[2]->setName("C");
    KSQ_USL_series[2]->setColor(QColor("#ffc2cb"));
    KSQ_LSL_series[0] = new QLineSeries;
    KSQ_LSL_series[0]->setName("A");
    KSQ_LSL_series[0]->setColor(QColor("#ff0004"));
    KSQ_LSL_series[1] = new QLineSeries;
    KSQ_LSL_series[1]->setName("B");
    KSQ_LSL_series[1]->setColor(QColor("#ff636d"));
    KSQ_LSL_series[2] = new QLineSeries;
    KSQ_LSL_series[2]->setName("C");
    KSQ_LSL_series[2]->setColor(QColor("#ffc2cb"));

    KSQ_value_series->setPointsVisible(true);



    int i=0;
    while(!values.isEmpty()){
          if(i > count){
            break;
          }
          KSQ_value_series->append(values.back().x(),values.back().y());
          values.pop_back();
          KSQ_USL_series[0]->append(USL_series_values[0].back().x(),USL_series_values[0].back().y());
          USL_series_values[0].pop_back();
          KSQ_USL_series[1]->append(USL_series_values[1].back().x(),USL_series_values[1].back().y());
          USL_series_values[1].pop_back();
          KSQ_USL_series[2]->append(USL_series_values[2].back().x(),USL_series_values[2].back().y());
          USL_series_values[2].pop_back();
          KSQ_LSL_series[0]->append(LSL_series_values[0].back().x(),LSL_series_values[0].back().y());
          LSL_series_values[0].pop_back();
          KSQ_LSL_series[1]->append(LSL_series_values[1].back().x(),LSL_series_values[1].back().y());
          LSL_series_values[1].pop_back();
          KSQ_LSL_series[2]->append(LSL_series_values[2].back().x(),LSL_series_values[2].back().y());
          LSL_series_values[2].pop_back();
          KSQ_CL_series->append(CL_series_values.back().x(),CL_series_values.back().y());
          CL_series_values.pop_back();
          i++;
    }
    KSQ_chart->addSeries(KSQ_value_series);
    KSQ_chart->addSeries(KSQ_USL_series[0]);
    KSQ_chart->addSeries(KSQ_USL_series[1]);
    KSQ_chart->addSeries(KSQ_USL_series[2]);
    KSQ_chart->addSeries(KSQ_LSL_series[0]);
    KSQ_chart->addSeries(KSQ_LSL_series[1]);
    KSQ_chart->addSeries(KSQ_LSL_series[2]);
    KSQ_chart->addSeries(KSQ_CL_series);

    KSQ_axisX = new QDateTimeAxis;
    KSQ_axisX->setTickCount(5);
    KSQ_axisX->setFormat("MM-dd HH:mm:ss");
    KSQ_axisX->setTitleText("Date");
    KSQ_axisX->setVisible(true);
    KSQ_axisX->setTitleVisible(true);
    KSQ_chart->addAxis(KSQ_axisX, Qt::AlignBottom);
    KSQ_axisY = new QValueAxis;
    KSQ_axisY->setRange(axisY->min(),axisY->max());
    KSQ_chart->addAxis(KSQ_axisY,Qt::AlignLeft);

    KSQ_value_series->attachAxis(KSQ_axisX);
    KSQ_value_series->attachAxis(KSQ_axisY);
    KSQ_USL_series[0]->attachAxis(KSQ_axisX);
    KSQ_USL_series[0]->attachAxis(KSQ_axisY);
    KSQ_USL_series[1]->attachAxis(KSQ_axisX);
    KSQ_USL_series[1]->attachAxis(KSQ_axisY);
    KSQ_USL_series[2]->attachAxis(KSQ_axisX);
    KSQ_USL_series[2]->attachAxis(KSQ_axisY);
    KSQ_LSL_series[0]->attachAxis(KSQ_axisX);
    KSQ_LSL_series[0]->attachAxis(KSQ_axisY);
    KSQ_LSL_series[1]->attachAxis(KSQ_axisX);
    KSQ_LSL_series[1]->attachAxis(KSQ_axisY);
    KSQ_LSL_series[2]->attachAxis(KSQ_axisX);
    KSQ_LSL_series[2]->attachAxis(KSQ_axisY);
    KSQ_CL_series->attachAxis(KSQ_axisX);
    KSQ_CL_series->attachAxis(KSQ_axisY);



}



void bridge_chart_widget::on_chart_timer_stop_btn_clicked()
{
    charttimer.stop();
}

void bridge_chart_widget::on_chart_timer_start_btn_clicked()
{
    charttimer.start();
}

void bridge_chart_widget::on_zoom_reset_btn_clicked()
{
    main_chart->zoomReset();
}


