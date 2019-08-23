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
    void KMP_fileserver_download(const QUrl url);

    void anchorClickedSlot(const QUrl);
    void slot_cookieAdded(const QNetworkCookie &cookie);
    void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);

    //void ftp_research_Slot(QString strReadLine,QString byte_urlEncoded,int count);

    void on_pushButton_clicked(void);

signals:
    void ftp_signal(QString strReadLine,QString byte_urlEncoded,int count);

private:
    Ui::MainWindow *ui;

    bool http_stat(QNetworkReply* reply);

    void post(int post_id,QUrl url,QString content_type,QString rows,QString keyword);
    void get(int get_id,QUrl url,QString content_type,QString keyword);

    //    void KMP_fileserver_download(const QUrl& url);
    //    void encode_method(QString response);
    //    void http_download(QString tag,QString title);
    //    void ftp_research(QString keyword);

    //    int flag_KMP_fileserver,flag_ftp,ui_flush_finish;

    QFile *file;
    QString KMP_Cookie;
    QString QIP_rows,KMP_target;
    QWebEngineView *KMP_webView = new QWebEngineView();
    QStackedLayout *layout = new QStackedLayout();
};

#endif // MAINWINDOW_H
