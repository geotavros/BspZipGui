#include "bspzipgui.h"

#include <iostream>

#include <QFile>
#include <qfiledialog>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QTextStream>

BspZipGui::BspZipGui(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
    bspzip_process_(NULL)
    //progress_dialog_(NULL)
{
    ui.setupUi(this);

    this->setWindowFlags(Qt::Window);

    bspzip_process_ = new QProcess(this);
    bool b = connect(bspzip_process_, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onBspZipProcessFinished(int, QProcess::ExitStatus)));
    Q_ASSERT(b);

    //progress_dialog_ = new QProgressDialog(tr("Performing operation"),
    //                                       tr("Cancel"), 0, 100, this);
    //progress_dialog_->setMinimumDuration(0);

    QSettings settings("settings.ini", QSettings::IniFormat, this);
    QString bzip = tr("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Nuclear Dawn\\bin\\bspzip.exe");
    QString bsp =  tr("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Nuclear Dawn\\nucleardawn\\maps\\MyMap.bsp");
    QString data_folder = tr("C:\\work\\MyMapProject");
    ui.bspzip_path->setText(settings.value("BSPZIP_PATH", bzip).toString());
    ui.bspfile_path->setText(settings.value("BSP_FILE_PATH", bsp).toString());
    ui.data_folder_path->setText(settings.value("DATA_FOLDER_PATH", data_folder).toString());
    this->resize(settings.value("SIZE", QSize(this->width(), this->height())).toSize());
}

BspZipGui::~BspZipGui()
{
    QSettings settings("settings.ini", QSettings::IniFormat, this);
    settings.setValue("BSPZIP_PATH", ui.bspzip_path->text());
    settings.setValue("BSP_FILE_PATH", ui.bspfile_path->text());
    settings.setValue("DATA_FOLDER_PATH", ui.data_folder_path->text());
    settings.setValue("SIZE", this->size());
}

void BspZipGui::on_browse_bspzip_clicked() 
{
    QString default_dir_path;;
    QFileInfo fi(ui.bspzip_path->text());
    if (fi.exists())
    {
        default_dir_path = fi.absolutePath();
    }
    else 
    {
        default_dir_path = QDir::homePath();
    }

    QFileDialog d;
    QString path = d.getOpenFileName(this, tr("Select bspzip.exe file"), default_dir_path);

    if (!path.isEmpty())
        ui.bspzip_path->setText(QDir::toNativeSeparators(path));
}

void BspZipGui::on_browse_bsp_file_clicked() 
{
    QString default_dir_path;;
    QFileInfo fi(ui.bspfile_path->text());
    if (fi.exists())
    {
        default_dir_path = fi.absolutePath();
    }
    else 
    {
        default_dir_path = QDir::homePath();
    }

    QFileDialog d;
    QString path = d.getOpenFileName(this, tr("Select BSP file"), default_dir_path);

    if (!path.isEmpty())
        ui.bspfile_path->setText(QDir::toNativeSeparators(path));
}

void BspZipGui::on_browse_data_folder_clicked() 
{
    QString default_dir_path;;
    QFileInfo fi(ui.data_folder_path->text());
    if (fi.exists())
    {
        if (fi.isDir())
            default_dir_path = fi.absoluteFilePath();
        else
            default_dir_path = fi.absolutePath();
    }
    else 
    {
        default_dir_path = QDir::homePath();
    }

    QFileDialog d;
    QString path = d.getExistingDirectory(this, tr("Select folder where to extract BSP or which to embed in BSP"), default_dir_path);

    if (!path.isEmpty())
        ui.data_folder_path->setText(QDir::toNativeSeparators(path));
}

void BspZipGui::on_embed_clicked() 
{
    disableAllButtons();

    QStringList check_list;
    check_list << ui.bspzip_path->text() << ui.bspfile_path->text() << ui.data_folder_path->text();
    if (!filesExist(check_list))
    {
        enableAllButtons();
        return;
    }

    QFile bspzip_filelist_file(QDir::homePath() + "/bspzip_filelist.txt"); 
    log(tr("Opening file for writing: %1").arg( bspzip_filelist_file.fileName() ));
    if (!bspzip_filelist_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        log(tr("Failed to open file for writing: %1").arg(bspzip_filelist_file.fileName()));
        enableAllButtons();
        return;
    }

    QDir data_dir( ui.data_folder_path->text(), "", QDir::Name | QDir::IgnoreCase, QDir::AllDirs | QDir::Files|  QDir::NoDotAndDotDot );
    getDataFolderFiles(data_dir, &data_folder_files_);

    if (data_folder_files_.isEmpty())
    {
        log(tr("No files found for embedding in directory: %1 Aborting...").arg(data_dir.absolutePath()));
        enableAllButtons();
        return;
    }

    log("Adding file list into filelist.txt...");

    QTextStream out(&bspzip_filelist_file);
    QString absolute_file_path;
    Q_FOREACH(absolute_file_path, data_folder_files_)
    {
        absolute_file_path = QDir::toNativeSeparators(absolute_file_path);
        QString relative_file_path = absolute_file_path.mid(ui.data_folder_path->text().size() + 1);

        log(relative_file_path);
        log(absolute_file_path);

        // no quotes here as BSPZIP doesn't handle them
        out << relative_file_path << "\n";
        out << absolute_file_path << "\n";
    }
    data_folder_files_.clear(); // must be cleared! or we get bugs

    bspzip_filelist_file.close();
    log(tr("File bspzip_filelist.txt was successfully created"));

    QString bsp_file_path = QDir::toNativeSeparators(ui.bspfile_path->text());
    QString filelist_path = QDir::toNativeSeparators(bspzip_filelist_file.fileName());
    QStringList process_arguments;
    process_arguments << "-addorupdatelist";
    process_arguments << bsp_file_path;
    process_arguments << filelist_path;
    process_arguments << bsp_file_path;

//     if (bspzip_process_) 
//     {
//         delete bspzip_process_;
//     }

    log(tr("Starting BSPZIP.EXE with following arguments"));
    Q_FOREACH(QString argument, process_arguments)
    {
        log(argument);
    }

    bspzip_process_->start(ui.bspzip_path->text(), process_arguments);
}

void BspZipGui::on_extract_clicked() 
{
    QString bsp_file_path = QDir::toNativeSeparators(ui.bspfile_path->text());
    QString data_folder = QDir::toNativeSeparators(ui.data_folder_path->text());

    QStringList check_list;
    check_list << ui.bspzip_path->text() << bsp_file_path << data_folder;
    if (!filesExist(check_list))
    {
        return;
    }
    
    QStringList process_arguments;
    process_arguments << "-extractfiles";
    process_arguments << bsp_file_path; 
    process_arguments << data_folder; 

//     if (bspzip_process_) 
//     {
//         delete bspzip_process_;
//     }

//     bspzip_process_ = new QProcess(this);
//     bool b = connect(bspzip_process_, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onBspZipProcessFinished(int, QProcess::ExitStatus)));
//    Q_ASSERT(b);

    bspzip_process_->start(ui.bspzip_path->text(), process_arguments);
}

void BspZipGui::getDataFolderFiles(const QDir &dir, QStringList *file_list) 
{
    QFileInfoList files = dir.entryInfoList();

    //log(tr("In directory: %1 parsing...").arg(dir.absolutePath()));
    
    Q_FOREACH(QFileInfo file, files)
    {
        if (file.isDir())
        {
            QDir d( file.absoluteFilePath(), "", QDir::Name | QDir::IgnoreCase, QDir::AllDirs | QDir::Files|  QDir::NoDotAndDotDot );

            //QString logstr = tr("Found directory: %1 parsing...").arg(d.absolutePath());
            //log(logstr);
            
            getDataFolderFiles(d, file_list);
        }
        else 
        {
            (*file_list) << file.absoluteFilePath();
            
            //log(tr("Found file: %1").arg(file.absoluteFilePath()));
        }
    }
}

void BspZipGui::log(const QString &logstr) 
{
    ui.console->appendPlainText(logstr);
    //std::cout << logstr.toLatin1().constData() << std::endl;
}

void BspZipGui::onBspZipProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) 
{
    Q_ASSERT(bspzip_process_);
    QByteArray standard_output = bspzip_process_->readAllStandardOutput();
    QByteArray error_output = bspzip_process_->readAllStandardError();

    QString status = exitStatus == QProcess::NormalExit ? "Normal" : "Crashed";

    log(tr("BSPZIP.exe finished with status: %1, exit code: %2").arg(status).arg(exitCode));
    log(tr("BSPZIP.exe output: "));
    log(standard_output);
    if (error_output.size() > 0)
    {
        log(tr("BSPZIP.exe error output: "));
        log(error_output);
    }

    enableAllButtons();

    QMessageBox::information(
        this, tr("Operation Completed"),
        tr("Operation completed. See output window for details"),
        QMessageBox::Ok);
}

bool BspZipGui::filesExist(const QStringList &files) 
{
    Q_FOREACH(QString file_path, files)
    {
        QFileInfo fi(file_path);
        if (!fi.exists())
        {
            QMessageBox::warning(this, tr("File Not Found"), tr("The file doesn't exist: %1. Aborting ").arg(file_path));
            return false;
        }
    }
    return true;
}

void BspZipGui::disableAllButtons()
{
    ui.embed->setDisabled(true);
    ui.extract->setDisabled(true);
}

void BspZipGui::enableAllButtons()
{
    ui.embed->setEnabled(true);
    ui.extract->setEnabled(true);
}
