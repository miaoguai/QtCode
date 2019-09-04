#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSslSocket>
#include <QMainWindow>
#include <QtConcurrent>
#include <Everything.h>
#include <QtWebEngineWidgets>

#pragma execution_character_set("utf-8")

#define List_all_url 1615447

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static QList<QFileInfo> Search(QString str,bool reg = false);

    static QString getSearchErrorString();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void requestFinished_QIP_rows_post(QNetworkReply* reply);
    void requestFinished_QIP_result_post(QNetworkReply* reply);

    void requestFinished_KMP_result_get(QNetworkReply* reply);
    void requestFinished_KMP_htmlNums_get(QNetworkReply* reply);
    void requestFinished_KMP_docxNums_get(QNetworkReply* reply);
    void requestFinished_KMP_download_get(QNetworkReply *reply);

    void KMP_fileserver_preview(const QUrl);

    void ftp_research_Slot(QString strReadLine,QByteArray byte_urlEncoded,int count);

    void KMP_preview_loadfinish(bool);
    void KMP_anchorClickedSlot(const QUrl);
    void ForE_anchorClickedSlot(const QUrl);
    void slot_cookieAdded(const QNetworkCookie &cookie);
    void updateDataReadProgress(qint64 bytesSent, qint64 bytesTotal);

    void on_pushButton_clicked(void);

    void on_checkBox_3_clicked();

    void on_checkBox_4_clicked();

signals:
    void ftp_signal(QString strReadLine,QByteArray byte_urlEncoded,int count);

private:
    Ui::MainWindow *ui;

    bool http_stat(QNetworkReply* reply);

    void get(int get_id,QUrl url,QString content_type);
    void post(int post_id,QUrl url,QString content_type,QString rows,QString keyword);    

    void ftp_research(QString keyword);

    int flag_KMP_preview_webloadfinish = 0,flag_KMP_preview_downloadfinish = 0,flag_ui_flush = 0,flag_ftp_search_finish = 0;

    QFile *file;
    QString QIP_rows,KMP_docxTitle,KMP_htmlNums,KMP_docxNums,KMP_Cookie;

    QStackedLayout *layout = new QStackedLayout();
    QWebEngineView *KMP_webView = new QWebEngineView();
};

#endif // MAINWINDOW_H
