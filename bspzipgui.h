#ifndef BSPZIPGUI_H
#define BSPZIPGUI_H

#include <QDialog>
#include <QDir>
#include <QProcess>
#include <QProgressDialog>
#include <QString>

#include "ui_bspzipgui.h"

class BspZipGui : public QDialog
{
    Q_OBJECT

public:
    BspZipGui(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~BspZipGui();

public slots:
    void on_browse_bspzip_clicked();
    void on_browse_bsp_file_clicked();
    void on_browse_data_folder_clicked();

    void on_embed_clicked();
    void on_extract_clicked();
    
    void onBspZipProcessFinished(int exitCode, QProcess::ExitStatus exitStatus );

private:
    void log(const QString &logstr);
    void getDataFolderFiles(const QDir &dir, QStringList *file_list);
    bool filesExist(const QStringList &files);
    void disableAllButtons();
    void enableAllButtons();
    QStringList data_folder_files_;
    QProcess    *bspzip_process_;
    //QProgressDialog *progress_dialog_;

    Ui::BspZipGuiClass ui;
};

#endif // BSPZIPGUI_H
