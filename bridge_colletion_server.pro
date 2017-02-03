#-------------------------------------------------
#
# Project created by QtCreator 2017-01-31T11:30:19
#
#-------------------------------------------------

QT       += core gui sql charts network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = bridge_colletion_server
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    bridge_chart.cpp \
    bridge_chartview.cpp \
    bridge_chart_widget.cpp \
    smtp/emailaddress.cpp \
    smtp/mimeattachment.cpp \
    smtp/mimecontentformatter.cpp \
    smtp/mimefile.cpp \
    smtp/mimehtml.cpp \
    smtp/mimeinlinefile.cpp \
    smtp/mimemessage.cpp \
    smtp/mimemultipart.cpp \
    smtp/mimepart.cpp \
    smtp/mimetext.cpp \
    smtp/quotedprintable.cpp \
    smtp/smtpclient.cpp

HEADERS  += mainwindow.h \
    bridge_chart.h \
    bridge_chartview.h \
    bridge_chart_widget.h \
    smtp/emailaddress.h \
    smtp/mimeattachment.h \
    smtp/mimecontentformatter.h \
    smtp/mimefile.h \
    smtp/mimehtml.h \
    smtp/mimeinlinefile.h \
    smtp/mimemessage.h \
    smtp/mimemultipart.h \
    smtp/mimepart.h \
    smtp/mimetext.h \
    smtp/quotedprintable.h \
    smtp/smtpclient.h \
    smtp/smtpexports.h \
    smtp/SmtpMime

FORMS    += mainwindow.ui \
    bridge_chart_widget.ui \
    email_html.ui
